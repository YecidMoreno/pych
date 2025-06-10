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

using namespace jsonapi;

class ActuatorIO : public DeviceIO_plugin
{
    json_obj _source;
    CommIO_plugin *comm = nullptr;
    std::string commIO_str;

    arg_CANOpen_send arg_to_send;
    struct
    {
        int nodeID;
        int rpdo;
        int lsb_byte;
        int n_bytes;
    } source;

    float max_value = 100.0;

    int32_t pos_val = 0;
    struct can_frame frameTx{};

public:
    virtual ~ActuatorIO() = default;

    virtual bool config(const std::string &cfg) override
    {
        configured = false;
        _cfg = json_obj::from_string(cfg);

    

        if (_cfg.get("source", &_source))
        {
            if (_source.get("NodeID", &source.nodeID) &
                _source.get("rpdo", &source.rpdo) &
                _source.get("lsb_byte", &source.lsb_byte) &
                _source.get("n_bytes", &source.n_bytes) &
                _source.get("commIO", &commIO_str))
            {
                configured = true;
            }
        }

     

        if (configured)
        {
            if (!_cfg.get("max_value", &max_value)){
                configured = false;
            }
        
            // struct can_frame frameTx{};
            frameTx.can_id = ID_RPDO_BASE+source.nodeID + source.rpdo*0x100;
            frameTx.can_dlc=8;
            memset(frameTx.data,0,8);
            // frameTx.can_id = 
            

            // arg_to_send = {.NodeID = source.nodeID,
            //                .TPDO_Index = source.tpdo,
            //                .lsb_byte = source.lsb_byte,
            //                .n_bytes = source.n_bytes};

            // comm = reinterpret_cast<CommIO_plugin *>(com_aux);
            // comm->send(nullptr, 0, &canopen_cmd_SYNC);
        }

        return configured;
    }

    virtual bool connect(const std::string &target) override
    {
        if (!configured)
            return false;

        auto &core = HH::Core::instance();

        comm = core.pm_commIO.get_node(commIO_str);

        return true;
    }

    virtual void disconnect() override
    {
    }

    virtual ssize_t write(void *data, size_t size, const void *arg1 = nullptr) override
    {
        memcpy(&pos_val,data,4);

        pos_val = pos_val > max_value? max_value : pos_val;
        pos_val = pos_val < -max_value? -max_value : pos_val;

        memcpy(frameTx.data+source.lsb_byte,&pos_val,4);
        comm->send( static_cast<void * >(&frameTx), 0);
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

extern "C" DeviceIO_plugin *create() { return new ActuatorIO; }
extern "C" void destroy(DeviceIO_plugin *p) { delete p; }