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
    _VirtualIO = 1,
    _CANOpenIO = 2,
    _SerialIO = 3
};

class SensorIO : public DeviceIO_plugin
{
    CommIO_plugin *comm = nullptr;
    std::string commIO_str;

    arg_CANOpen_receive arg_to_read;

    struct
    {
        bool enable = false;
        std::string to_eval = "";
        ExprtkParser parser;
        double x = 0.0;
        double t = 0.0;
        double y = 0.0;
        double off_y = 0.0;
    } eval;

    struct
    {
        CommIO_type comm_type = CommIO_type::_NO_TYPE;
        uint32_t CAN_ID;
        int nodeID;
        int tpdo;
        int lsb_byte;
        int n_bytes = 8;
        bool is_lbs = true;
        bool calibrate = true;
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
                else if (commIO_str.compare("VirtualIO") == 0)
                {
                    source.comm_type = CommIO_type::_VirtualIO;
                    source.ctype = "int32";
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

                if (_source.get("NodeID", &source.nodeID) &&
                    _source.get("tpdo", &source.tpdo) &&
                    _source.get("lsb_byte", &source.lsb_byte) &&
                    _source.get("ctype", &source.ctype) &&
                    _source.get("n_bytes", &source.n_bytes))
                {
                    arg_to_read = {.NodeID = source.nodeID,
                                   .TPDO_Index = source.tpdo,
                                   .lsb_byte = source.lsb_byte,
                                   .n_bytes = source.n_bytes};
                    configured = true;
                }
                else if (_source.get("CAN_ID", &source.CAN_ID) &&
                         _source.get("lsb_byte", &source.lsb_byte) &&
                         _source.get("ctype", &source.ctype) &&
                         _source.get("n_bytes", &source.n_bytes))
                {
                    arg_to_read = {.CAN_ID = source.CAN_ID,
                                   .lsb_byte = source.lsb_byte,
                                   .n_bytes = source.n_bytes};
                    configured = true;
                }
                else
                {
                    configured = false;
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
            case CommIO_type::_VirtualIO:
                if (_cfg.contain("eval"))
                {
                    configured = true;
                }
                break;
            default:
                configured = false;
                break;
            }
        }
        else
            return configured;

        source._type = parse_var_type(source.ctype);

        std::vector<std::string> eval_arr;
        if (configured && _cfg.get("eval", &eval_arr))
        {
            // hh_loge("Es array");
            eval.to_eval = "";
            for (size_t i = 0; i < eval_arr.size(); i++)
            {
                eval.to_eval += eval_arr[i] + (i + 1 == eval_arr.size() ? "" : " \n ");
            }
            eval.enable = true;
        }
        else
        {
            _cfg.get("eval", &eval.to_eval);
            eval.enable = true;
            // hh_loge("Es string");
        }

        if (configured && eval.enable)
        {
            // hh_loge("Expression: %s", eval.to_eval.c_str());

            double aux[10];
            eval.parser.setVariable("aux0", &aux[0]);
            eval.parser.setVariable("aux1", &aux[1]);
            eval.parser.setVariable("aux2", &aux[2]);
            eval.parser.setVariable("aux3", &aux[3]);
            eval.parser.setVariable("aux4", &aux[4]);
            eval.parser.setVariable("aux5", &aux[5]);

            eval.parser.setVariable("x", &eval.x);
            eval.parser.setVariable("t", &eval.t);
            eval.parser.setVariable("off_y", &eval.off_y);
            if (!eval.parser.setExpression(eval.to_eval + " - off_y"))
            {
                hh_loge("Failed to parse expression %s", eval.to_eval.c_str());
                hh_loge(">> %s", eval.parser.error().c_str());
                configured = false;
            }
        }

        _source.get("lsb", &source.is_lbs);
        _source.get("calibrate", &source.calibrate);

        return configured;
    }

    virtual bool connect(const std::string &target) override
    {
        if (!configured)
            return false;

        auto &core = HH::Core::instance();

        if (source.comm_type != CommIO_type::_VirtualIO)
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
        uint8_t tmp_buffer[source.n_bytes] = {0};
        ssize_t res = 0;

        switch (source.comm_type)
        {
        case CommIO_type::_CANOpenIO:
            res = comm->receive(tmp_buffer, source.n_bytes, static_cast<void *>(&arg_to_read));
            break;
        case CommIO_type::_SerialIO:
            res = comm->receive(tmp_buffer, source.n_bytes, &source.lsb_byte);
            break;
        case CommIO_type::_VirtualIO:
            // res = comm->receive(tmp_buffer, source.n_bytes, &source.lsb_byte);
            break;

        default:
            return -1;
            break;
        }

        if (!source.is_lbs)
        {
            std::reverse(tmp_buffer, tmp_buffer + source.n_bytes);
        }

        if (!eval.enable)
        {
            memcpy(buffer, tmp_buffer, source.n_bytes);
            return res;
        }

        if (convert_buffer_to_double(tmp_buffer, eval.x, source._type))
        {
            static auto &core = HH::Core::instance();

            eval.t = core.get_run_time_double(1s);
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
        if (source.calibrate == false)
            return true;

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
            if (source.calibrate == false)
            {
                eval.off_y = 0;
            }
            else
            {
                eval.off_y = moving_avg.back();
            }

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