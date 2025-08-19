#include "core.h"
#include "logger.h"
#include <csignal>
#include <thread>
#include <chrono>

#include <filesystem>

using namespace std::chrono_literals;

using namespace HH;
using namespace jsonapi;

Core *Core::this_instance = nullptr;
std::atomic<AppState> Core::_state = AppState::IDLE;

Core &Core::instance()
{
    if (!this_instance)
        this_instance = new Core();
    return *this_instance;
}

void Core::destroy()
{
    delete this_instance;
    this_instance = nullptr;
}

void Core::request_shutdown()
{
    _state = AppState::STOPPING;
    if (Core::this_instance)
    {
        auto &core = Core::instance();
        hh_logw("Core::request_shutdown at [%f]", core.runner.get_run_time_double(1ms));
    }
}

void Core::request_state(HH::AppState newState)
{
    _state = newState;
}

void handle_sigint(int)
{
    Core::request_shutdown();
    hh_logw("Ctrl + C, evoked");
}

Core::Core()
{
    hh_log_init("hh");
    hh_logn(" ");
    hh_logn("**********************************************");
    hh_logn("**********      Core Inited            *******");
    hh_logn("**********************************************");
    hh_logn(" ");

    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGHUP, &sa, nullptr);

    _state = AppState::IDLE;
};

Core::~Core()
{

    this->runner.disconnect_all_devices();
    this->scheduler.stop();
    std::this_thread::sleep_for(400ms);
    hh_logi("Finish Core");
    hh_log_close();
};

bool Core::load_json_project(std::string file_path)
{
    std::vector<std::string> pluginPath = {""};
    json_obj j = json_obj::from_file(file_path);

    json_obj j_plugins, j_p_commIO, j_p_deviceIO, j_p_controlIO;
    if (!j.get("plugins", &j_plugins) &
        !j_plugins.get("commIO", &j_p_commIO) &
        !j_plugins.get("deviceIO", &j_p_deviceIO) &
        !j_plugins.get("controlIO", &j_p_controlIO))
    {
        hh_loge("Key NOT found: plugins");
        return false;
    }

    if (!plugins.register_all_plugins_from_json(j_plugins))
        return false;

    json_obj j_log;
    if (j.get("log", &j_log))
    {
        j_log.get("enable", &_cfg.logger.enable);
        j_log.get("path", &_cfg.logger.path);
        namespace fs = std::filesystem;
        fs::create_directories(_cfg.logger.path);
    }

    if (!plugins.add_nodes_from_json(j))
        return false;

    hh_logn("Json file loaded with sucess");
    return true;
}


AppState Core::get_state()
{
    return _state;
}
