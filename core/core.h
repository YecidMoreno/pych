#pragma once

#include <utils/plugin_loader.h>
#include <commIO/commIO_interface.h>
#include <deviceIO/deviceIO_interface.h>
#include <controlIO/controlIO_interface.h>
#include "json_api.h"
#include "core/logger.h"
#include <atomic>
#include <vector>
#include <thread>
#include <functional>
#include <chrono>

#include <core/tasking.h>
#include "core_defs.h"

namespace HH
{
    using namespace std::chrono_literals;
    using namespace std::chrono;

    class __attribute__((visibility("default"))) Core
    {

    private:
        Core();
        ~Core();

        std::atomic<AppState> _state;

        Core(const Core &) = delete;
        Core &operator=(const Core &) = delete;

        std::unordered_map<std::string, task_container_t> _tasks_handles;

        core_config _cfg;

    public:
        steady_clock::time_point run_time_0;

        AppState  get_state();

        bool connect_all_devices();
        bool disconnect_all_devices();

        PluginManager<CommIO_plugin> pm_commIO;
        PluginManager<DeviceIO_plugin> pm_deviceIO;
        PluginManager<ControlIO_plugin> pm_controlIO;

        bool load_json_project(std::string file_path);

        bool add_to_task(std::string _task_name, task_struct_t _t);
        bool run_tasks();
        bool stop_tasks();

        bool run_for_time(std::chrono::nanoseconds _time);

        std::chrono::nanoseconds get_run_time();
        double get_run_time_double(std::chrono::nanoseconds units = 1s);

        inline const core_config& config() const { return _cfg; }

        static Core &instance();
    };

}
