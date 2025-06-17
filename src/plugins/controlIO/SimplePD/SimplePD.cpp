#include <iostream>
#include <string.h>
#include <vector>

#include <controlIO/controlIO_interface.h>

#include <math.h>
#include <chrono>

#include <core/core.h>
#include <core/logger.h>
#include <core/tasking.h>

#include <utils/utils_control.h>

#define PLUGIN_IO_NAME SimplePD
#define PLUGIN_IO_TYPE ControlIO_plugin

class PLUGIN_IO_NAME : public PLUGIN_IO_TYPE
{

    float kp, kd;
    std::chrono::nanoseconds ts;
    
    double pos0 = 0;
    double pos1 = 0;

    float N = 101.0f;
    double t_ms, vel;

    VariableTrace p0, p1, error;

public:
    virtual ~PLUGIN_IO_NAME() = default;

    virtual bool config(const std::string &cfg) override
    {
        CONTROL_IO_INIT_CONFIG(cfg);

        CONFIG_VALIDATE(_CFG_FIELD_GET(kp));
        CONFIG_VALIDATE(_CFG_FIELD_GET(kd));
        CONFIG_VALIDATE(_CFG_FIELD_GET(ts));

        set_logger();

        CONTROL_IO_FINISH_CONFIG_FULL;
    }

    void set_logger()
    {
        if (!_logger.enabled)
            return;

        auto &core = HH::Core::instance();

        this->_logger.file.emplace_back();
        this->_logger.file.back().set_file_name(core.config().logger.path + _logger.file_name + ".log");

        _logger.file[0].vars.push_back(RecordVariable{.name = "time", .format = "%.4f", ._ptr = &t_ms});
        _logger.file[0].vars.push_back(RecordVariable{.name = "pos0", .fnc = [this]()
                                                                      { return p0.v(); }});
        _logger.file[0].vars.push_back(RecordVariable{.name = "d_pos0", .fnc = [this]()
                                                                        { return p0.d(); }});
        _logger.file[0].vars.push_back(RecordVariable{.name = "pos1", .fnc = [this]()
                                                                      { return p1.v(); }});
        _logger.file[0].vars.push_back(RecordVariable{.name = "d_pos1", .fnc = [this]()
                                                                        { return p1.d(); }});
        _logger.file[0].vars.push_back(RecordVariable{.name = "error", .fnc = [this]()
                                                                       { return error.v(); }});
        _logger.file[0].vars.push_back(RecordVariable{.name = "vel", ._ptr = &vel});
    }

    void print_event(std::chrono::nanoseconds now)
    {
        static auto next_time = now;
        if (now >= next_time)
        {
            next_time += 1s;
            hh_logi("pos0: %8f\t pos1: %8f\t vel: %4f", pos0, pos1, vel);
        }
    }

    void send_control_signal(double u)
    {
        int32_t vel_int = static_cast<int32_t>(u);
        _actuators[0]->write(&vel_int, 0);
    }

    virtual int loop() override
    {
        // Do not modify — handles timing and required imports
        __CONTROL_IO_BEGIN_LOOP();

        t_ms = core.get_run_time_double(1ms);

        // Read sensors
        _sensors[0]->read(&pos0, 0);
        _sensors[1]->read(&pos1, 0);

        // For continuous-time controller: compute dt with lower bound
        __CONTROL_IO_GET_DT(1e-3);

        // Update tracked variables and compute their derivatives and integrals
        error.update(pos0 / N - pos1, dt);
        p0.update(pos0, dt);
        p1.update(pos1, dt);

        /* P.Y.C.H. — Put Your Controller (Probably Overengineered) Here */
        vel = kp * error.v() + kd * error.d();

        // Apply control signal to actuator
        send_control_signal(vel);

        // Finalize loop timing and logging
        __CONTROL_IO_END_LOOP();

        print_event(now);
        _logger.loop();

        return 0;
    };
};

__FINISH_PLUGIN_IO;
