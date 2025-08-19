#include "CoreRunner.h"
#include <core/core.h>

using namespace HH;

CoreRunner::CoreRunner(/* args */)
{
    // auto &core = Core::instance();
    // start();
    // core.request_state(AppState::IDLE);
}

CoreRunner::~CoreRunner()
{
}

bool CoreRunner::connect_all_devices()
{

    auto &core = Core::instance();
    core.request_state(AppState::RUNNING);

    for (auto &node : core.plugins.getNodes_T<CommIO_plugin>())
    {
        if (!node->connect(""))
        {
            hh_loge("[] CommIO connect False");
            return false;
        }
    }

    for (auto &node : core.plugins.getNodes_T<DeviceIO_plugin>())
    {
        if (!node->connect(""))
        {
            hh_loge("[] DeviceIO connect False");
            return false;
        }
    }

    std::this_thread::sleep_for(100ms);

    
    for (int i = 0; i < 10; i++)
    {
        for (auto &node : core.plugins.getNodes_T<DeviceIO_plugin>())
        {
            if (!node->command("calibrate"))
            {
                
                hh_loge("[] DeviceIO calibrate False");
                return false;
            }
        }

        std::this_thread::sleep_for(10ms);
    }


    for (auto &node : core.plugins.getNodes_T<DeviceIO_plugin>())
    {
        if (!node->command("start"))
        {
            hh_loge("[] DeviceIO start False");
            return false;
        }
    }

    return true;
}

bool CoreRunner::disconnect_all_devices()
{
    auto &core = Core::instance();
    core.request_state(AppState::STOPPING);

    for (auto &node : core.plugins.getNodes_T<ControlIO_plugin>())
    {
        node->command("END");
    }

    for (auto &node : core.plugins.getNodes_T<DeviceIO_plugin>())
    {
        node->disconnect();
    }

    for (auto &node : core.plugins.getNodes_T<CommIO_plugin>())
    {
        node->disconnect();
    }

    core.request_state(AppState::STOPPING);

    return true;
}

std::chrono::nanoseconds CoreRunner::get_run_time()
{
    using namespace std::chrono_literals;
    using namespace std::chrono;
    return steady_clock::now() - this->run_time_0;
}

double CoreRunner::get_run_time_double(std::chrono::nanoseconds units)
{
    using namespace std::chrono_literals;
    using namespace std::chrono;
    auto elapsed = steady_clock::now() - this->get_run_time_0();
    return double(elapsed.count()) / units.count();
}

std::chrono::steady_clock::time_point CoreRunner::get_run_time_0()
{
    return run_time_0;
}

void CoreRunner::start()
{
    this->run_time_0 = std::chrono::_V2::steady_clock::now();
}