
#include <commIO/commIO_interface.h>
#include "deviceIO/deviceIO_interface.h"

#include "src/plugins/commIO/CANOpenIO/CANOpenIO_defs.h"

#include "core/core.h"
#include "core/json_api.h"
#include "core/logger.h"

#include "EPOS_defs.h"
#include "linux/can.h"

#include <thread>

using namespace jsonapi;
using namespace std::chrono_literals;

class EPOS : public DeviceIO_plugin
{
    json_obj _source;
    CommIO_plugin *comm = nullptr;

    int NodeID = 0;
    std::string commIO_str;

public:
    virtual ~EPOS() = default;

    virtual bool config(const std::string &cfg) override
    {
        configured = false;
        _cfg = json_obj::from_string(cfg);

        if (_cfg.get("source", &_source))
        {
            if (_source.get("commIO", &commIO_str))
            {
                if (_source.get("NodeID", &NodeID))
                {
                    configured = true;
                }
            }
        }
        else
        {
        }

        if (configured)
        {
            hh_logi("EPOS configurada");

            //     comm = reinterpret_cast<CommIO_plugin *>(com_aux);
            //     comm->send(nullptr, 0, &canopen_cmd_SYNC);
        }
        else
        {
            hh_logw("No fue posible inicializar la EPOS");
        }

        return configured;
    }

    virtual bool connect(const std::string &target) override
    {
        if (!configured)
            return false;

        hh_logv("commIO_str: %s", commIO_str.c_str());

        auto &core = HH::Core::instance();
        hh_logv("EPOS ptr core: %p", (void *)&core);

        comm = core.pm_commIO.get_node(commIO_str);

        this->command(EPOS_CMD::FAULT_RESET);
        this->command(EPOS_CMD::SET_VELOCITY_MODE);
        this->command(EPOS_CMD::ENABLE_MOTORS);

        comm->send(nullptr, 0, &canopen_cmd_SYNC);

        return true;
    }

    virtual void disconnect() override
    {
        this->command(EPOS_CMD::FAULT_RESET);
    }

    virtual ssize_t write(void *data, size_t size, const void *arg1 = nullptr) override
    {
        return -1;
    }

    virtual ssize_t read(void *buffer, size_t max_size, const void *arg1 = nullptr) override
    {
        return -1;
    }

    void send_FAULT_RESET()
    {
        struct can_frame frame;
        frame.data[0] = B2_PAYLOAD;

        frame.can_id = ID_SDO + NodeID;
        frame.can_dlc = 8;

        frame.data[0] = B4_PAYLOAD; // WRITE 2 bytes
        frame.data[1] = 0x40;       // Index 0x6040 LSB
        frame.data[2] = 0x60;       // Index 0x6040 MSB
        frame.data[3] = 0x00;       // Subindex 0x00

        frame.data[6] = 0x00;
        frame.data[7] = 0x00;

        frame.data[4] = 0x00;
        frame.data[5] = 0x00;
        comm->send(static_cast<void *>(&frame), sizeof(frame));
        std::this_thread::sleep_for(100ms);

        frame.data[4] = 0x80;
        frame.data[5] = 0x00;
        comm->send(static_cast<void *>(&frame), sizeof(frame));
    }

    void send_VELOCITY_MODE()
    {
        struct can_frame frame;
        frame.data[0] = B1_PAYLOAD;

        frame.can_id = ID_SDO + NodeID;
        frame.can_dlc = 8;

        frame.data[0] = B4_PAYLOAD; // WRITE 2 bytes
        frame.data[1] = 0x60;       // Index 0x6060 LSB
        frame.data[2] = 0x60;       // Index 0x6060 MSB
        frame.data[3] = 0x00;       // Subindex 0x00

        frame.data[4] = -2;
        comm->send(static_cast<void *>(&frame), sizeof(frame));
    }

    void send_ENABLE_MOTOR()
    {
        struct can_frame frame;
        frame.data[0] = B2_PAYLOAD;

        frame.can_id = ID_SDO + NodeID;
        frame.can_dlc = 8;

        frame.data[0] = B4_PAYLOAD; // WRITE 2 bytes
        frame.data[1] = 0x40;       // Index 0x6040 LSB
        frame.data[2] = 0x60;       // Index 0x6040 MSB
        frame.data[3] = 0x00;       // Subindex 0x00

        frame.data[4] = 0b00000000;
        frame.data[5] = 0x80;
        frame.data[6] = 0x00;
        frame.data[7] = 0x00;

        frame.data[4] = 0b00000000;
        frame.data[5] = 0x80;
        comm->send(static_cast<void *>(&frame), sizeof(frame));

        std::this_thread::sleep_for(50ms);

        frame.data[4] = 0x06;
        frame.data[5] = 0x00;
        comm->send(static_cast<void *>(&frame), sizeof(frame));

        std::this_thread::sleep_for(50ms);

        frame.data[4] = 0x07;
        frame.data[5] = 0x00;
        comm->send(static_cast<void *>(&frame), sizeof(frame));

        std::this_thread::sleep_for(50ms);

        frame.data[4] = 0x0f;
        frame.data[5] = 0x00;
        comm->send(static_cast<void *>(&frame), sizeof(frame));
    }

    virtual bool command(uint32_t opcode, const void *arg = nullptr) override
    {

        switch (opcode)
        {
        case EPOS_CMD::FAULT_RESET:
            send_FAULT_RESET();
            return true;
            break;

        case EPOS_CMD::SET_VELOCITY_MODE:
            send_VELOCITY_MODE();
            return true;
            break;
        case EPOS_CMD::ENABLE_MOTORS:
            send_ENABLE_MOTOR();
            return true;
            break;

        default:
            break;
        }
        return false;
    }
};

extern "C" DeviceIO_plugin *create() { return new EPOS; }
extern "C" void destroy(DeviceIO_plugin *p) { delete p; }