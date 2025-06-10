#include <iostream>
#include <string.h>
#include <unordered_map>
#include <cstring>

#include "commIO/commIO_interface.h"
#include "deviceIO/deviceIO_interface.h"

#include "commIO/commIO_interface.h"
#include "src/plugins/commIO/CANOpenIO/CANOpenIO_defs.h"

#include <linux/can.h>

#include "core/json_api.h"
#include "core/core.h"
#include "core/logger.h"

#include <core/exprtk_wrapper.h>
#include <core/type_utils.h>

using namespace jsonapi;

enum class CommandOp : uint8_t
{
    START,
    STOP,
    RESET,
    READ,
    CALIBRATE,
    INVALID
};

CommandOp parse_command(const std::string &cmd)
{
    static const std::unordered_map<std::string, CommandOp> map = {
        {"start", CommandOp::START},
        {"stop", CommandOp::STOP},
        {"reset", CommandOp::RESET},
        {"calibrate", CommandOp::CALIBRATE},
        {"read", CommandOp::READ},
    };
    auto it = map.find(cmd);
    return (it != map.end()) ? it->second : CommandOp::INVALID;
}

enum class CommIO_type : uint8_t
{
    _NO_TYPE = 0,
    _CANOpenIO = 1,
    _SerialIO = 2
};

class SensorIO : public DeviceIO_plugin
{
    CommIO_plugin *comm = nullptr;
    std::string commIO_str;

    arg_CANOpen_receive arg_to_read;

    struct
    {
        bool enable = false;
        std::string to_eval;
        ExprtkParser parser;
        double x = 0.0;
        double y = 0.0;
        double off_y = 0.0;
    } eval;

    struct
    {
        CommIO_type comm_type = CommIO_type::_NO_TYPE;
        int nodeID;
        int tpdo;
        int lsb_byte;
        int n_bytes;
        std::string ctype;
        Var_type _type = Var_type::_NO_TYPE;
    } source;

public:
    virtual ~SensorIO() = default;

    virtual bool config(const std::string &cfg) override
    {
        configured = false;
        _cfg = json_obj::from_string(cfg);

        auto &core = HH::Core::instance();
        json_obj _source;

        if (_cfg.get("source", &_source))
        {
            if (_source.get("commIO", &commIO_str))
            {
                std::string type_comm_str = core.pm_commIO.node_type[commIO_str];

                if (type_comm_str.compare("CANOpenIO") == 0)
                {
                    source.comm_type = CommIO_type::_CANOpenIO;
                }
                else if (type_comm_str.compare("SerialIO") == 0)
                {
                    source.comm_type = CommIO_type::_SerialIO;
                }
            }
        }
        else
            return configured;

        if (source.comm_type != CommIO_type::_NO_TYPE)
        {
            switch (source.comm_type)
            {
            case CommIO_type::_CANOpenIO:
                if (_source.get("NodeID", &source.nodeID) &
                    _source.get("tpdo", &source.tpdo) &
                    _source.get("lsb_byte", &source.lsb_byte) &
                    _source.get("ctype", &source.ctype) &
                    _source.get("n_bytes", &source.n_bytes))
                {

                    arg_to_read = {.NodeID = source.nodeID,
                                   .TPDO_Index = source.tpdo,
                                   .lsb_byte = source.lsb_byte,
                                   .n_bytes = source.n_bytes};
                    configured = true;
                }
                break;
            case CommIO_type::_SerialIO:
                if (_source.get("lsb_byte", &source.lsb_byte) &
                    _source.get("n_bytes", &source.n_bytes) &
                    _source.get("ctype", &source.ctype))
                {
                    configured = true;
                }
                break;
            default:
                configured = false;
                break;
            }
        }

        source._type = parse_var_type(source.ctype);

        if (configured && _cfg.get("eval", &eval.to_eval))
        {
            eval.enable = true;
            eval.parser.setVariable("x", &eval.x);
            eval.parser.setVariable("off_y", &eval.off_y);
            if (!eval.parser.setExpression(eval.to_eval + " - off_y"))
            {
                hh_loge("Failed to parse expression %s", eval.to_eval.c_str());
                configured = false;
            }
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
        return -1;
    }

    virtual ssize_t read(void *buffer, size_t, const void *arg1 = nullptr) override
    {
        uint8_t tmp_buffer[32] = {0};
        ssize_t res = 0;

        switch (source.comm_type)
        {
        case CommIO_type::_CANOpenIO:
            res = comm->receive(tmp_buffer, source.n_bytes, static_cast<void *>(&arg_to_read));
            break;
        case CommIO_type::_SerialIO:
            res = comm->receive(tmp_buffer, source.n_bytes, &source.lsb_byte);
            break;

        default:
            return -1;
            break;
        }

        if (!eval.enable)
        {
            memcpy(buffer, tmp_buffer, source.n_bytes);
            return res;
        }

        if (convert_buffer_to_double(tmp_buffer, eval.x, source._type))
        {
            eval.y = eval.parser.evaluate();
            memcpy(buffer, &eval.y, sizeof(eval.y));
        }
        else
        {
            memcpy(buffer, tmp_buffer, source.n_bytes);
        }

        // hh_logi("tmp_buffer ptr: %p",(void *)&tmp_buffer[0]);
        // hh_logn("eval: %s = %f   x: %f", eval.to_eval.c_str(), eval.y, eval.x);

        return res;
    }

    std::vector<double> moving_avg;

    bool sensor_calibrate()
    {
        double val = 0.0;
        if (eval.enable)
        {
            switch (source.comm_type)
            {
            case CommIO_type::_CANOpenIO:
                comm->send(nullptr, 0, &canopen_cmd_SYNC);
                break;
            }

            this->read(&val, 0);
            if (moving_avg.size() < 1)
            {
                moving_avg.push_back(val);
            }
            {
                moving_avg.push_back((moving_avg.back() + val) / 2.0);
            }
        }

        return true;
    }

    virtual bool command(std::string cmd, const void *arg0, const void *arg1) override
    {
        switch (parse_command(cmd))
        {

        case CommandOp::CALIBRATE:
            return sensor_calibrate();
            break;
        case CommandOp::START:
            eval.off_y = moving_avg.back();
            break;
        case CommandOp::INVALID:
        default:
            return false;
        }

        return true;
    }
};

extern "C" DeviceIO_plugin *create() { return new SensorIO; }
extern "C" void destroy(DeviceIO_plugin *p) { delete p; }