#include "core.h"
#include "logger.h"
#include <csignal>
#include <thread>
#include <chrono>
#include <core/tasking.h>

using namespace std::chrono_literals;

using namespace HH;
using namespace jsonapi;

Core &Core::instance()
{
    static Core instance; // inicialización segura en C++11+
    return instance;
}

void handle_sigint(int)
{
    auto &core = Core::instance();
    core.disconnect_all_devices();
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

    _state = AppState::IDLE;
};

Core::~Core()
{
    this->disconnect_all_devices();
    this->stop_tasks();
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

    if (j_plugins.get("path", &pluginPath))
    {
        hh_logn("pluginPath: %s", pluginPath[0].c_str());
    }

    /*
        Adicionando plugins de comunicacion
    */
    hh_logn("commIO:");
    for (const auto &key : j_p_commIO.keys())
    {
        std::string value;
        if (j_p_commIO.get(key, &value))
        {
            hh_logn("  %s -> %s\n", key.c_str(), value.c_str());
            bool res = false;
            for (const auto &pp : pluginPath)
            {
                res = pm_commIO.register_plugin(key, pp + value);
                if (res)
                    break;
            }
            if (!res)
            {
                throw std::runtime_error("No se ha encontrado la biblioteca " + value);
            }
        }
    }

    /*
        Adicionando plugins de disposivos
    */
    hh_logn("deviceIO:");
    for (const auto &key : j_p_deviceIO.keys())
    {
        std::string value;
        if (j_p_deviceIO.get(key, &value))
        {
            hh_logn("  %s -> %s\n", key.c_str(), value.c_str());
            bool res = false;

            for (const auto &pp : pluginPath)
            {
                res = pm_deviceIO.register_plugin(key, pp + value);
                if (res)
                    break;
            }
            if (!res)
            {
                throw std::runtime_error("No se ha encontrado la biblioteca " + value);
            }
        }
    }

    /*
        Adicionando plugins de control
    */
    hh_logn("controlIO:");
    for (const auto &key : j_p_controlIO.keys())
    {
        std::string value;
        if (j_p_controlIO.get(key, &value))
        {
            hh_logn("  %s -> %s\n", key.c_str(), value.c_str());
            bool res = false;

            for (const auto &pp : pluginPath)
            {
                res = pm_controlIO.register_plugin(key, pp + value);
                if (res)
                    break;
            }
            if (!res)
            {
                throw std::runtime_error("No se ha encontrado la biblioteca " + value);
            }
        }
        else
        {
            hh_loge("No se ha podido capturar la key: %s\n", key.c_str());
        }
    }

    json_obj j_commIO;
    if (!j.get("commIO", &j_commIO))
    {
        hh_loge("Key NOT found: commIO\n");
        return false;
    }

    /*
        Registrando nodos de comunicacion
    */
    for (const auto &key : j_commIO.keys())
    {
        json_obj value;
        json_obj value_cfg;
        std::string type;
        if (!j_commIO.get(key, &value))
        {
            continue;
        }

        if (!value.get("config", &value_cfg) & !value.get("type", &type))
        {
            continue;
        }

        hh_logn("commIO: %s -> %s\n", key.c_str(), type.c_str());
        pm_commIO.create_node(key, type);
        if (!pm_commIO.get_node(key)->config(value.get_object_str("config")))
        {
            hh_loge("commIO: %s -> %s NOT configured\n", key.c_str(), type.c_str());
        }
    }

    json_obj j_deviceIO;
    if (!j.get("deviceIO", &j_deviceIO))
    {
        hh_loge("Key NOT found: deviceIO\n");
        return false;
    }

    for (const auto &key : j_deviceIO.keys())
    {
        json_obj value;
        json_obj value_cfg;
        std::string type;
        if (!j_deviceIO.get(key, &value))
        {
            continue;
        }

        if (!value.get("config", &value_cfg) & !value.get("type", &type))
        {
            continue;
        }

        hh_logn("deviceIO: %s -> %s\n", key.c_str(), type.c_str());
        pm_deviceIO.create_node(key, type);
        if (!pm_deviceIO.get_node(key)->config(value.get_object_str("config")))
        {
            hh_loge("deviceIO: %s -> %s NOT configured\n", key.c_str(), type.c_str());
        }
    }

    json_obj j_controlIO;
    if (!j.get("controlIO", &j_controlIO))
    {
        hh_loge("Key NOT found: controlIO\n");
        return false;
    }
    for (const auto &key : j_controlIO.keys())
    {
        json_obj value;
        json_obj value_cfg;
        std::string type;
        if (!j_controlIO.get(key, &value))
        {
            continue;
        }

        if (!value.get("config", &value_cfg) & !value.get("type", &type))
        {
            continue;
        }

        hh_logn("controlIO: %s -> %s\n", key.c_str(), type.c_str());
        pm_controlIO.create_node(key, type);
        if (!pm_controlIO.get_node(key)->config(value.get_object_str("config")))
        {
            hh_loge("controlIO: %s -> %s NOT configured\n", key.c_str(), type.c_str());
        }
    }

    hh_logn("Json file loaded with sucess");
    return true;
}

std::chrono::nanoseconds Core::get_run_time()
{
    using namespace std::chrono_literals;
    using namespace std::chrono;
    return steady_clock::now() - this->run_time_0;
}

bool Core::connect_all_devices()
{
    _state = AppState::RUNNING;

    for (auto &k : this->pm_commIO.getNodes())
    {
        this->pm_commIO.get_node(k)->connect("");
    }

    for (auto &k : this->pm_deviceIO.getNodes())
    {
        this->pm_deviceIO.get_node(k)->connect("");
    }

    std::this_thread::sleep_for(100ms);

    for (int i = 0; i < 10; i++)
    {
        for (auto &d : this->pm_deviceIO.getNodes())
        {
            auto n = this->pm_deviceIO.get_node(d);
            n->command("calibrate");
        }

        std::this_thread::sleep_for(10ms);
    }

    for (auto &d : this->pm_deviceIO.getNodes())
    {
        auto n = this->pm_deviceIO.get_node(d);
        n->command("start");
    }

    return true;
}

bool Core::disconnect_all_devices()
{
    _state = AppState::STOPPING;

    for (auto &k : this->pm_deviceIO.getNodes())
    {
        this->pm_deviceIO.get_node(k)->disconnect();
    }

    for (auto &k : this->pm_commIO.getNodes())
    {
        this->pm_commIO.get_node(k)->disconnect();
    }

    _state = AppState::STOPPING;

    return true;
}

AppState  Core::get_state()
{
    return _state;
}
std::unordered_map<std::string, task_container_t> _tasks_handles;

bool Core::add_to_task(std::string _task_name, task_struct_t _t)
{
    if (_tasks_handles.find(_task_name) == _tasks_handles.end())
    {
        task_container_t _c;
        _c.task.clear();
        _c._loop = _taks_main_h;
        _c._thr = std::thread();
        _tasks_handles.emplace(_task_name, std::move(_c));
    }

    if (_tasks_handles.find(_task_name) != _tasks_handles.end())
    {
        _tasks_handles[_task_name].task.push_back(std::move(_t));
        return true;
    }
    else
    {
        return false;
    }
    return true;
}

bool Core::run_tasks()
{
    this->run_time_0 = std::chrono::_V2::steady_clock::now();

    for (auto &[name, val] : this->_tasks_handles)
    {

        val._thr = std::thread([&val]()
                               { val._loop(val.task); });
    }
    return true;
}

bool Core::stop_tasks()
{
    for (auto &[name, val] : this->_tasks_handles)
    {

        if (val._thr.joinable())
        {
            val._thr.join();
        }
    }
    return true;
}

bool Core::run_for_time(std::chrono::nanoseconds _time)
{
    this->run_tasks();

    auto t0 = steady_clock::now();

    while (this->get_state() == HH::AppState::RUNNING)
    {
        if (steady_clock::now() - t0 > _time)
        {
            return true;
            break;
        }

        std::this_thread::sleep_for(500ms);
    }

    return false;
}
