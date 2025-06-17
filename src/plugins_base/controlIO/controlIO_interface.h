#pragma once
#include <string>
#include <stdint.h>

#include <core/json_api.h>
#include <core/core_defs.h>
#include <core/record_variables.h>
#include <core/json_api.h>
#include "deviceIO/deviceIO_interface.h"

// Interfaz base que todos los plugins deben implementar
class ControlIO_plugin
{
public:
    bool configured = false;
    jsonapi::json_obj _cfg;

    struct
    {
        bool enabled = false;
        std::string file_name;
        std::vector<RecordVariables> file;
        inline void loop()
        {
            if (!enabled)
                return;
            for (auto &f : file)
            {
                f.loop();
            }
        }
    } _logger;

    std::vector<DeviceIO_plugin *> _sensors;
    std::vector<DeviceIO_plugin *> _actuators;

    virtual ~ControlIO_plugin() = default;
    virtual bool config(const std::string &cfg) = 0;

    virtual int loop() = 0;

    virtual bool command(uint32_t opcode, const void *arg = nullptr) { return true; };
    virtual bool command(std::string cmd, const void *arg0 = nullptr, const void *arg1 = nullptr) { return false; }
};

#define CONTROL_IO_INIT_CONFIG(cfg)                 \
    using namespace jsonapi;                        \
    configured = true;                              \
    auto &core = HH::Core::instance();              \
    _cfg = json_obj::from_string(cfg);              \
    json_obj j_log;                                 \
    if (_cfg.get("log", &j_log))                    \
    {                                               \
        j_log.get("enable", &_logger.enabled);      \
        j_log.get("file_name", &_logger.file_name); \
    }

#define CONTROL_IO_LOAD_SENSORS_FROM_JSON_                       \
    std::vector<std::string> _cfg_sensors;                       \
    if (configured && _cfg.get("sensors", &_cfg_sensors))        \
    {                                                            \
        _sensors.clear();                                        \
        for (auto &name : _cfg_sensors)                          \
        {                                                        \
            _sensors.push_back(core.pm_deviceIO.get_node(name)); \
        }                                                        \
    }                                                            \
    else                                                         \
    {                                                            \
        configured = false;                                      \
    }

#define CONTROL_IO_LOAD_ACTUATORS_FROM_JSON_                       \
    std::vector<std::string> _cfg_actuators;                       \
    if (configured && _cfg.get("actuators", &_cfg_actuators))      \
    {                                                              \
        _actuators.clear();                                        \
                                                                   \
        for (auto &name : _cfg_actuators)                          \
        {                                                          \
            _actuators.push_back(core.pm_deviceIO.get_node(name)); \
        }                                                          \
    }                                                              \
    else                                                           \
    {                                                              \
        configured = false;                                        \
    }

#define CONTROL_IO_CONFIGURE_TASK_FROM_JSON_                      \
    task_struct_t _task;                                          \
    if (configured && load_task_from_json(cfg, _task))            \
    {                                                             \
        _task._callback = [this]()                                \
        { this->loop(); };                                        \
        configured = core.add_to_task(_task._thread_name, _task); \
    }                                                             \
    else                                                          \
    {                                                             \
        configured = false;                                       \
    }

#define CONTROL_IO_FINISH_CONFIG \
    return configured;

#define CONTROL_IO_FINISH_CONFIG_FULL     \
    CONTROL_IO_LOAD_SENSORS_FROM_JSON_;   \
    CONTROL_IO_LOAD_ACTUATORS_FROM_JSON_; \
    CONTROL_IO_CONFIGURE_TASK_FROM_JSON_; \
    return configured;

#define CONFIG_VALIDATE(ARG)                                                \
    do                                                                      \
    {                                                                       \
        if (configured)                                                     \
        {                                                                   \
            configured &= ARG;                                              \
            if (!configured)                                                \
            {                                                               \
                hh_logw("No se ha podido recuperar el valor de: %s", #ARG); \
            }                                                               \
        }                                                                   \
    } while (0);

#define __CONTROL_IO_BEGIN_LOOP()             \
    static auto &core = HH::Core::instance(); \
    using namespace std::chrono_literals;     \
    static auto prev_time = core.get_run_time();

#define __CONTROL_IO_END_LOOP() \
    prev_time = now;

#define __CONTROL_IO_GET_DT(DT)            \
    auto now = core.get_run_time();        \
    double dt = (now - prev_time).count(); \
    dt = (dt > DT) ? dt : DT;
