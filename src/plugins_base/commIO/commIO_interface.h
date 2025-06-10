#pragma once
#include <string>
#include <stdint.h>

// Interfaz base que todos los plugins deben implementar
class CommIO_plugin
{
public:
    virtual ~CommIO_plugin() = default;
    virtual bool config(const std::string &cfg) = 0;
    virtual bool connect(const std::string &target) = 0;
    virtual ssize_t send(void *data, size_t size, const void *arg1 = NULL) = 0;
    virtual ssize_t receive(void *buffer, size_t max_size, const void *arg1 = NULL) = 0;
    virtual bool command(uint32_t opcode, const void* arg = nullptr) = 0;
    virtual void disconnect() = 0;
};