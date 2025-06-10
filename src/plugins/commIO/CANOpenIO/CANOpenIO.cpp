#include "plugin_loader.h"

#include "CANOpenIO_defs.h"
#include "commIO/commIO_interface.h"
#include "controlIO/controlIO_interface.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <time.h>
#include <cstdint>
#include <atomic>
#include <array>

#include "core/json_api.h"
#include "core/core.h"
#include "core/logger.h"

#include <thread>
using namespace std::chrono_literals;

using namespace jsonapi;

constexpr int NUM_PDO = 4; 

typedef struct
{
    pthread_mutex_t lock;
    std::array<struct can_frame, NUM_PDO> TPDO;
    std::array<struct can_frame, NUM_PDO> RPDO;
    std::array<uint64_t, NUM_PDO> TPDO_counter;
} NodeDO;

static NodeDO nodes[MAX_NODES];
pthread_t rx_thread;
static std::atomic<bool> tx_running{false};

static void *can_receive_thread(void *arg)
{
    int sock = *(int *)arg;
    struct can_frame frame;

    auto &core = HH::Core::instance();
    while (core.get_state() == HH::AppState::RUNNING)
    {
        if (read(sock, &frame, sizeof(frame)) > 0)
        {
            uint8_t node_id = frame.can_id & 0x7F;
            if (node_id >= MAX_NODES)
                continue;

            NodeDO *m = &nodes[node_id];
            uint16_t offset = (frame.can_id & 0x780) - ID_TPDO_BASE;
            int index = offset / 0x100;
            if (index >= 0 && index < NUM_PDO)
            {
                pthread_mutex_lock(&m->lock);
                memcpy(&m->TPDO[index], &frame, sizeof(frame));
                ++m->TPDO_counter[index];
                pthread_mutex_unlock(&m->lock);
            }
        }
    }
    return nullptr;
}

class CANProtocol : public CommIO_plugin
{
private:
    json_obj _cfg;
    int sock = -1;
    bool configured = false;
    std::string iface;

    std::array<std::array<uint64_t, NUM_PDO>, MAX_NODES> last_read_counter{};

public:
    bool config(const std::string &cfg) override
    {
        _cfg = json_obj::from_string(cfg);
        configured = _cfg.get("iface", &iface);

        auto &core = HH::Core::instance();

        task_struct_t _task;
        if (configured && load_task_from_json(cfg, _task))
        {
            _task._callback = [this]()
            { this->send(nullptr, 0, &canopen_cmd_SYNC); };

            configured = core.add_to_task(_task._thread_name, _task);
        }
        else
        {
            configured = false;
        }

        return configured;
    }

    bool connect(const std::string &) override
    {
        if (!configured)
            return false;

        sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (sock < 0)
            return false;

        struct ifreq ifr;
        strcpy(ifr.ifr_name, iface.c_str());
        if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0)
            return false;

        struct sockaddr_can addr = {.can_family = AF_CAN, .can_ifindex = ifr.ifr_ifindex};
        if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
            return false;

        tx_running = true;
        pthread_create(&rx_thread, nullptr, can_receive_thread, &sock);

        if (configured)
        {
            this->send(nullptr, 0, &canopen_cmd_MNT);
            std::this_thread::sleep_for(100ms);
            this->send(nullptr, 0, &canopen_cmd_SYNC);
        }

        hh_logv("%s connected", iface.c_str());
        std::this_thread::sleep_for(100ms);

        return true;
    }

    ssize_t send(void *data, size_t, const void *arg1) override
    {
        if (arg1)
        {
            const canopen_cmd *cmd = static_cast<const canopen_cmd *>(arg1);
            struct can_frame frame{};

            if (cmd->type == CANOpenSpecial::NMT)
            {
                frame.can_id = ID_NMT;
                frame.can_dlc = 2;
                frame.data[0] = cmd->data[0];
                frame.data[1] = cmd->data[1];
            }
            else if (cmd->type == CANOpenSpecial::SYNC)
            {
                frame.can_id = ID_SYNC;
                frame.can_dlc = 0;
            }

            return write(sock, &frame, sizeof(frame));
        }

        return write(sock, static_cast<struct can_frame *>(data), sizeof(struct can_frame));
    }

    ssize_t receive(void *buffer, size_t, const void *arg) override
    {
        const arg_CANOpen_receive *a = static_cast<const arg_CANOpen_receive *>(arg);
        if (!a || a->NodeID >= MAX_NODES || a->TPDO_Index < 0 || a->TPDO_Index >= NUM_PDO)
            return -1;

        int node = a->NodeID;
        int index = a->TPDO_Index;
        int lsb_byte = a->lsb_byte;
        int n_bytes = a->n_bytes;

        bool is_new = false;
        pthread_mutex_lock(&nodes[node].lock);
        memcpy(buffer, &nodes[node].TPDO[index].data + lsb_byte, n_bytes);

        is_new = (nodes[node].TPDO_counter[index] != last_read_counter[node][index]);
        if (is_new)
        {
            last_read_counter[node][index] = nodes[node].TPDO_counter[index];
        }
        pthread_mutex_unlock(&nodes[node].lock);

        return is_new ? sizeof(struct can_frame) : 0;
    }

    virtual bool command(uint32_t opcode, const void *arg = nullptr)
    {
        return true;
    }

    void
    disconnect() override
    {
        tx_running = false;

        hh_logv("%s disconnected", iface.c_str());
    }

    CANProtocol()
    {
        for (int i = 0; i < MAX_NODES; ++i)
        {
            for (int j = 0; j < NUM_PDO; ++j)
            {
                nodes[i].RPDO[j].can_id = ID_RPDO_BASE + 0x100 * j + i;
                nodes[i].TPDO[j].can_id = ID_TPDO_BASE + 0x100 * j + i;
            }
        }
    }

    ~CANProtocol()
    {
        disconnect();
        if (sock >= 0)
            close(sock);
    }
};

extern "C" CommIO_plugin *create() { return new CANProtocol; }
extern "C" void destroy(CommIO_plugin *p) { delete p; }