#pragma once
#include <string>
#include <stdint.h>
#include "core/json_api.h"
#include <core/logger.h>

// Interfaz base que todos los plugins deben implementar
class DeviceIO_plugin
{
public:
    bool configured = false;
    jsonapi::json_obj _cfg;

    virtual ~DeviceIO_plugin() = default;

    virtual bool config(const std::string &cfg) = 0;
    virtual bool connect(const std::string &target) = 0;

    virtual ssize_t write(void *data, size_t size, const void *arg1 = nullptr) = 0;
    virtual ssize_t read(void *buffer, size_t max_size, const void *arg1 = nullptr) = 0;

    virtual bool command(uint32_t opcode, const void *arg = nullptr)
    {
        hh_logw("DeviceIO_plugin: Command(opcode) not defined, continuing with out errors. ");
        return true;
    };
    virtual bool command(std::string cmd, const void *arg0 = nullptr, const void *arg1 = nullptr)
    {
        hh_logw("DeviceIO_plugin: Command(cmd) not defined, continuing with out errors. ");
        return true;
    }

    virtual void disconnect() = 0;
};