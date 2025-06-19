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

#define PLUGIN_IO_NAME SimplePD_1D
#define PLUGIN_IO_TYPE ControlIO_plugin

class PLUGIN_IO_NAME : public PLUGIN_IO_TYPE
{

    float kp, kd, ki;
    std::chrono::nanoseconds ts;

    double t_s, vel;
    double pos_d;

    VariableTrace p0, error, _dt;

public:
    virtual ~PLUGIN_IO_NAME() = default;

    virtual bool config(const std::string &cfg) override
    {
        CONTROL_IO_INIT_CONFIG(cfg);

        CONFIG_VALIDATE(_CFG_FIELD_GET(kp));
        CONFIG_VALIDATE(_CFG_FIELD_GET(kd));
        CONFIG_VALIDATE(_CFG_FIELD_GET(ki));
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

        _logger.file[0].vars.push_back(RecordVariable{.name = "time", .format = "%.4f", ._ptr = &t_s});
        _logger.file[0].vars.push_back(RecordVariable{.name = "pos", .fnc = [this]()
                                                                     { return p0.v(); }});
        _logger.file[0].vars.push_back(RecordVariable{.name = "pos_d", .fnc = [this]()
                                                                       { return pos_d; }});
        _logger.file[0].vars.push_back(RecordVariable{.name = "vel", .fnc = [this]()
                                                                     { return p0.d(); }});
        _logger.file[0].vars.push_back(RecordVariable{.name = "vel_d", .fnc = [this]()
                                                                       { return vel; }});
        _logger.file[0].vars.push_back(RecordVariable{.name = "error", .fnc = [this]()
                                                                       { return error.v(); }});
        _logger.file[0].vars.push_back(RecordVariable{.name = "dt", .fnc = [this]()
                                                                    { return _dt.v(); }});
    }

    void send_control_signal(double u)
    {
        int32_t vel_int = static_cast<int32_t>(u);
        _actuators[0]->write(&vel_int, 4);
    }

    virtual int loop() override
    {
        static LowPassFilter lpf(5.0);
        static double pos0 = 0;

        // Do not modify — handles timing and required imports
        __CONTROL_IO_BEGIN_LOOP();

        t_s = core.get_run_time_double(1s);

        // Read sensors
        _references[0]->read(&pos_d, 4);
        _sensors[0]->read(&pos0, 4);

        // For continuous-time controller: compute dt with lower bound
        __CONTROL_IO_GET_DT(1e-3);

        _dt.update(dt, dt);
        p0.update(pos0, dt);

        // pos_d = 90.0 * sin(2 * M_PI * .4f * t_s);
        // Update tracked variables and compute their derivatives and integrals

        error.update(pos_d - p0.v(), dt);

        /* P.Y.C.H. — Put Your Controller (Probably Overengineered) Here */
        vel = kp * error.v() + kd * error.d() + ki * error.i();
        // vel = 60.0;
        double filtered = lpf.update(vel, dt);
        vel = filtered;
        // Apply control signal to actuator
        // send_control_signal(vel);
        send_control_signal(vel * 21.0 * 9.0);

        hh_logi("t: %f\tpos: %f\tpos_d: %f\tvel_d: %f\t e: %f\n", t_s, p0.v(), pos_d, vel, error.v());

        // Finalize loop timing and logging
        __CONTROL_IO_END_LOOP();
        _logger.loop();

        return 0;
    };
};

__FINISH_PLUGIN_IO;
