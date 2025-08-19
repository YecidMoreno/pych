/*

Add runner class:
    connect
    disconnect
    ...

*/
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

#include "core_defs.h"

#include <plugin/PluginService.h>
#include <CoreRunner/CoreRunner.h>
#include <CoreScheduler/CoreScheduler.h>

namespace HH
{
    using namespace std::chrono_literals;
    using namespace std::chrono;

    class __attribute__((visibility("default"))) Core
    {

    private:
        Core();
        ~Core();

        static Core *this_instance;

        static std::atomic<AppState> _state;

        Core(const Core &) = delete;
        Core &operator=(const Core &) = delete;

        core_config _cfg;

    public:
        AppState get_state();

        PluginService plugins;
        CoreRunner runner;
        CoreScheduler scheduler;

        bool load_json_project(std::string file_path);

        inline const core_config &config() const { return _cfg; }

        static Core &instance();
        static void destroy();
        static void request_state(HH::AppState newState);
        static void request_shutdown();
    };

}
