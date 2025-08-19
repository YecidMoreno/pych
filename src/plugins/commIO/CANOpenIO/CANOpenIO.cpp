#define PLUGIN_IO_NAME CANProtocol
#define PLUGIN_IO_TYPE CommIO_plugin

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

#include <core/type_utils.h>

#include <unordered_map>

#include <thread>
using namespace std::chrono_literals;

using namespace jsonapi;

pthread_t rx_thread;
static std::atomic<bool> tx_running{false};

struct map_frame
{
    can_frame frame = {0};
    uint64_t w_count = 0;
    uint64_t r_count = 0;
};

pthread_mutex_t lock_all_frames;
std::unordered_map<int32_t, map_frame> _all_frames;

void print_can_message(struct can_frame &frame)
{
    hh_logi("CAN ID: 0x%X DLC: %d Data: %02X %02X %02X %02X %02X %02X %02X %02X\n",
            frame.can_id & CAN_EFF_MASK,
            frame.can_dlc,
            frame.data[0], frame.data[1], frame.data[2], frame.data[3],
            frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
}
static void *can_receive_thread(void *arg)
{
    int sock = *(int *)arg;
    struct can_frame frame;

    auto &core = HH::Core::instance();
    while (core.get_state() == HH::AppState::RUNNING)
    {
        if (read(sock, &frame, sizeof(frame)) > 0)
        {
            uint32_t node_id_32 = frame.can_id & CAN_EFF_MASK;

            if (true)
            {
                pthread_mutex_lock(&lock_all_frames);
                _all_frames[node_id_32].frame = frame;
                _all_frames[node_id_32].w_count++;
                pthread_mutex_unlock(&lock_all_frames);
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
    std::string iface;

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

            configured = core.scheduler.add_to_task(_task._thread_name, _task);
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

    ssize_t send(void *data, size_t len, const void *arg1) override
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
            else if (cmd->type == CANOpenSpecial::RPDO)
            {
                frame.can_id = ID_RPDO_BASE + 0x100 * cmd->arg1 + cmd->arg0;
                frame.can_dlc = len;
                const uint8_t *ptr = static_cast<const uint8_t *>(data);
                for (size_t i = 0; i < len && i < 8; i++)
                {
                    frame.data[i] = ptr[i];
                }
            }

            return write(sock, &frame, sizeof(frame));
        }

        return write(sock, static_cast<struct can_frame *>(data), sizeof(struct can_frame));
    }

    ssize_t receive(void *buffer, size_t, const void *arg) override
    {
        const arg_CANOpen_receive *a = static_cast<const arg_CANOpen_receive *>(arg);

        uint32_t CAN_ID = a->CAN_ID;
        const int &node = a->NodeID;
        const int &index = a->TPDO_Index;
        const int &byte_0 = a->byte_0;
        const int &n_bytes = a->n_bytes;
        const bool &is_raw = a->raw;

        if (CAN_ID == 0U)
        {
            if (a && node <= MAX_NODES && index >= 0 && index <= MAX_PDO)
            {
                CAN_ID = ID_TPDO_BASE + node + 0x100 * (index);
            }
            else
            {
                return -1;
            }
        }
        ssize_t res = 0;
        map_frame &mf = _all_frames[CAN_ID];

        if (CAN_ID != 0U)
        {
            // if (_all_frames.find(CAN_ID) != _all_frames.end())
            // {
            if (!is_raw)
            {
                pthread_mutex_lock(&lock_all_frames);
                memcpy(buffer, &mf.frame.data + byte_0, n_bytes);

                if (a->reverse)
                {
                    reverse_bytes((uint8_t *)buffer, n_bytes);
                }

                res = mf.r_count == mf.w_count ? 0 : n_bytes;
                mf.r_count = mf.w_count;
                pthread_mutex_unlock(&lock_all_frames);
            }
            else
            {
                pthread_mutex_lock(&lock_all_frames);
                memcpy(buffer, &mf.frame, sizeof(can_frame));
                res = mf.r_count == mf.w_count ? 0 : sizeof(can_frame);
                pthread_mutex_unlock(&lock_all_frames);
            }

            return res;

            // }
            // return 0;
        }

        return -1;
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
    }

    ~CANProtocol()
    {
        disconnect();
        if (sock >= 0)
            close(sock);
    }


    std::string get_type() override
    {
        return TO_STR(PLUGIN_IO_NAME);
    }

};

__FINISH_PLUGIN_IO;