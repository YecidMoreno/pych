#define PLUGIN_IO_NAME ActuatorIO
#define PLUGIN_IO_TYPE DeviceIO_plugin

#include <iostream>
#include <string.h>

#include "commIO/commIO_interface.h"
#include "deviceIO/deviceIO_interface.h"

#include "commIO/commIO_interface.h"
#include "src/plugins/commIO/CANOpenIO/CANOpenIO_defs.h"

#include <linux/can.h>

#include "core/json_api.h"
#include "core/core.h"
#include "core/logger.h"

#include <core/type_utils.h>

using namespace jsonapi;

class PLUGIN_IO_NAME : public PLUGIN_IO_TYPE
{
    json_obj _source;
    CommIO_plugin *comm = nullptr;
    std::string commIO_str;

    arg_CANOpen_send arg_to_send;
    struct
    {
        int nodeID;
        uint32_t CAN_ID = 0;
        int rpdo;
        int lsb_byte;
        int n_bytes;
        bool lsb = true;
        int dlc = 8;
    } source;

    float max_value = 100.0;

    int32_t pos_val = 0;
    struct can_frame frameTx{};

    bool CANOpen_cfg, CAN_raw_cfg;

public:
    virtual ~PLUGIN_IO_NAME() = default;

    virtual bool config(const std::string &cfg) override
    {
        configured = false;
        _cfg = json_obj::from_string(cfg);

        if (!_cfg.get("source", &_source) || !_source.get("commIO", &commIO_str))
            return configured;

        auto &core = HH::Core::instance();
        // comm = core.pm_commIO.get_node(commIO_str);
        comm = core.plugins.get_node<CommIO_plugin>(commIO_str);

        if (comm->get_type().compare("CANProtocol") == 0)
        {
            CANOpen_cfg = _source.get("NodeID", &source.nodeID) &&
                          _source.get("rpdo", &source.rpdo) &&
                          _source.get("lsb_byte", &source.lsb_byte) &&
                          _source.get("n_bytes", &source.n_bytes) &&
                          _source.get("dlc", &source.dlc);

            CAN_raw_cfg = _source.get("CAN_ID", &source.CAN_ID) &&
                          _source.get("lsb_byte", &source.lsb_byte) &&
                          _source.get("n_bytes", &source.n_bytes) &&
                          _source.get("dlc", &source.dlc);

            configured = CANOpen_cfg || CAN_raw_cfg;
        }

        if (!_cfg.get("max_value", &max_value))
        {
            configured = false;
            hh_loge("The max value is Mandatory");
        }

        if (!configured)
            return configured;

        _source.get("lsb", &source.lsb);

        if (CANOpen_cfg)
        {
            frameTx.can_id = ID_RPDO_BASE + source.nodeID + source.rpdo * 0x100;
            frameTx.can_dlc = source.dlc;
            memset(frameTx.data, 0, 8);
        }

        if (CAN_raw_cfg)
        {
            frameTx.can_id = source.CAN_ID | CAN_EFF_FLAG;
            frameTx.can_dlc = source.dlc;
            memset(frameTx.data, 0, source.dlc);
        }

        return configured;
    }

    virtual bool connect(const std::string &target) override
    {
        if (!configured)
            return false;

        auto &core = HH::Core::instance();

        // comm = core.pm_commIO.get_node(commIO_str);
        comm = core.plugins.get_node<CommIO_plugin>(commIO_str);

        return true;
    }

    virtual void disconnect() override
    {
    }

    virtual ssize_t write(void *data, size_t size, const void *arg1 = nullptr) override
    {
        if (source.n_bytes == 2)
        {
            int16_t tmp16;
            memcpy(&tmp16, data, 2);
            pos_val = tmp16; // aquí ocurre la extensión de signo a 32 bits
        }
        else if (source.n_bytes == 4)
        {
            int32_t tmp32;
            memcpy(&tmp32, data, 4);
            pos_val = tmp32;
        }

        // memcpy(&pos_val, data, source.n_bytes);

        pos_val = pos_val > max_value ? max_value : pos_val;
        pos_val = pos_val < -max_value ? -max_value : pos_val;

        if (!source.lsb)
        {
            reverse_bytes((uint8_t *)&pos_val, source.n_bytes);
        }

        memcpy(frameTx.data + source.lsb_byte, &pos_val, source.n_bytes);
        comm->send(static_cast<void *>(&frameTx), 0);
        return -1;
    }

    virtual ssize_t read(void *buffer, size_t max_size, const void *arg1 = nullptr) override
    {

        return comm->receive(buffer, 0, static_cast<void *>(&arg_to_send));
    }

    virtual bool command(uint32_t opcode, const void *arg = nullptr) override
    {
        return false;
    }
};

__FINISH_PLUGIN_IO;
