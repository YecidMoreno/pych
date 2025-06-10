#include <iostream>
#include <string.h>
#include <vector>

#include "controlIO/controlIO_interface.h"

#include <math.h>
#include <chrono>

#include "core/json_api.h"
#include "core/core.h"
#include "core/logger.h"
#include "core/tasking.h"

#include <utils/utils_control.h>


class SimplePD : public ControlIO_plugin
{

    float kp, kd;
    std::chrono::nanoseconds ts;
    
public:
    virtual ~SimplePD() = default;

    virtual bool config(const std::string &cfg) override
    {
        CONTROL_IO_INIT_CONFIG(cfg);

        CONFIG_VALIDATE(_CFG_FIELD_GET(kp));
        CONFIG_VALIDATE(_CFG_FIELD_GET(kd));
        CONFIG_VALIDATE(_CFG_FIELD_GET(ts));

        CONTROL_IO_FINISH_CONFIG_FULL;
    }

    double pos0 = 0;
    double pos1 = 0;

    float N = 101.0f;
    float vel = 0.0;

    VariableTrace error;

    virtual int loop() override
    {
        static auto &core = HH::Core::instance();

        static auto prev_time = core.get_run_time();
        auto now = core.get_run_time();

        _sensors[0]->read(&pos0, 0);
        _sensors[1]->read(&pos1, 0);

        double dt = (now - prev_time).count();
        dt = (dt > 1e-3) ? dt : 1e-3;

        error.update(pos0 / N - pos1, dt);

        double vel = kp * error.value() + kd * error.d();

        prev_time = now;

        int32_t vel_int = static_cast<int32_t>(vel);
        _actuators[0]->write(&vel_int, 0);

        static auto next_time = now;
        if (now >= next_time)
        {
            next_time += 1s;
            hh_logi("pos0: %8f\t pos1: %8f\t vel: %4f", pos0, pos1, vel);
            
        }

        return 0;
    };

};

extern "C" ControlIO_plugin *create() { return new SimplePD; }
extern "C" void destroy(ControlIO_plugin *p) { delete p; }