#include "plugin_loader.h"

#include "SerialIO_defs.h"
#include "commIO/commIO_interface.h"

#include <atomic>

#include "core/core.h"
#include "core/json_api.h"
#include "core/logger.h"

#include <thread>
using namespace std::chrono_literals;

using namespace jsonapi;

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <mutex>

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <cstring>
#include <string>

#define MAX_SERIAL_BUFFER 128

pthread_t rx_thread;
static std::atomic<bool> tx_running{false};

inline void default_tty(struct termios &tty)
{
    // struct termios  tty = {};

    memset(&tty, 0, sizeof tty);

    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
}

inline int available(int _fd)
{
    int nBytes = 0;
    if (ioctl(_fd, FIONREAD, &nBytes) < 0)
    {
        return 0;
    }

    return nBytes;
}

class SerialProtocol : public CommIO_plugin
{
private:
    json_obj _cfg;
    bool configured = false;

    int sock = -1;
    bool is_open = false;
    uint8_t buffer[MAX_SERIAL_BUFFER];

    struct
    {
        std::string iface;
        unsigned long baudrate = 115200;
        unsigned long timeout_sec = 0;
        unsigned long timeout_usec = 500000;
        std::string header = "@";
        std::string terminator = "\n";
        int n_Bytes = 34;

        bool isHeader = false, isTerminator = false;

    } _config;

public:
    void *serial_receive_thread()
    {
        // int sock = *(int *)arg;
        int nBytes;
        uint8_t tmp_buffer[MAX_SERIAL_BUFFER];
        bool isOK = false;
        char hex_str[3 * MAX_SERIAL_BUFFER + 1] = {0};
        size_t sync_index = 0;

        auto &core = HH::Core::instance();
        while (core.get_state() == HH::AppState::RUNNING)
        {
            uint8_t byte;
            ssize_t r = ::read(sock, &byte, 1);
            if (r <= 0)
                continue;

            // Desplazar ventana si ya se llenó
            if (sync_index >= _config.n_Bytes)
            {
                memmove(tmp_buffer, tmp_buffer + 1, _config.n_Bytes - 1);
                tmp_buffer[_config.n_Bytes - 1] = byte;
            }
            else
            {
                tmp_buffer[sync_index++] = byte;
                continue;
            }

            bool isOK = true;

            if (_config.isHeader)
                isOK = (tmp_buffer[0] == *_config.header.c_str());

            if (isOK && _config.isTerminator)
                isOK = (tmp_buffer[_config.n_Bytes - 1] == *_config.terminator.c_str());

            if (isOK)
            {
                memcpy(buffer, tmp_buffer, _config.n_Bytes);
                sync_index = 0;                                              // opcional: resetear para esperar otro frame completo
                std::this_thread::sleep_for(std::chrono::microseconds(100)); // limitar ritmo si deseas
            }
        }
        return nullptr;
    }

    static void *thread_entry(void *arg)
    {
        return static_cast<SerialProtocol *>(arg)->serial_receive_thread();
    }

    bool config(const std::string &cfg) override
    {
        _cfg = json_obj::from_string(cfg);
        configured = _cfg.get("iface", &_config.iface) &&
                     _cfg.get("baudrate", &_config.baudrate) &&
                     _cfg.get("timeout_sec", &_config.timeout_sec) &&
                     _cfg.get("timeout_usec", &_config.timeout_usec) &&
                     _cfg.get("header", &_config.header) &&
                     _cfg.get("terminator", &_config.terminator) &&
                     _cfg.get("n_Bytes", &_config.n_Bytes);

        _config.isHeader = _config.header.compare("") != 0;
        _config.isTerminator = _config.terminator.compare("") != 0;

        return configured;
    }

    bool connect(const std::string &) override
    {
        if (!configured)
            return false;

        if (is_open)
            return true;
        is_open = false;

        // timeout_s = _cfg.value("timeout_s", 0L);
        // timeout_us = _cfg.value("timeout_us", 100L);

        sock = ::open(_config.iface.c_str(), O_RDWR | O_NOCTTY | O_DSYNC);
        if (sock < 0)
        {
            return false;
        }

        struct termios tty;

        tcgetattr(sock, &tty);
        default_tty(tty);

        speed_t baud;
        switch (_config.baudrate)
        {
        case 9600:
            baud = B9600;
            break;
        case 19200:
            baud = B19200;
            break;
        case 38400:
            baud = B38400;
            break;
        case 57600:
            baud = B57600;
            break;
        case 115200:
            baud = B115200;
            break;
        case 230400:
            baud = B230400;
        case 1000000:
            baud = B1000000;
            break;
        default:
            return false; // o manejar error
        }

        cfsetispeed(&tty, baud);
        cfsetospeed(&tty, baud);

        // if (tcgetattr(_fd, &tty) != 0)
        // {
        //     return false;
        // }

        if (tcsetattr(sock, TCSANOW, &tty) != 0)
        {
            return false;
        }

        tcflush(sock, TCIOFLUSH);

        is_open = true;

        tx_running = true;
        // pthread_create(&rx_thread, nullptr, serial_receive_thread, &sock);
        pthread_create(&rx_thread, nullptr, thread_entry, this);

        hh_logi("%s connected", _config.iface.c_str());
        std::this_thread::sleep_for(100ms);

        return true;
    }

    ssize_t send(void *data, size_t length, const void *arg1) override
    {
        if (arg1)
        {
        }
        return ::write(sock, data, length);
        // return write(sock, static_cast<struct can_frame *>(data), sizeof(struct can_frame));
    }

    ssize_t receive(void *_buffer, size_t length, const void *arg) override
    {
        memcpy(_buffer, this->buffer + *(static_cast<const int *>(arg)), length);
        return 0;
    }
    virtual bool command(uint32_t opcode, const void *arg = nullptr)
    {
        return true;
    }

    void
    disconnect() override
    {
        tx_running = false;

        hh_logv("%s disconnected", iface.c_str());
    }

    SerialProtocol()
    {
    }

    ~SerialProtocol()
    {
        disconnect();
        // if (sock >= 0)
        //     close(sock);
    }
};

extern "C" CommIO_plugin *create() { return new SerialProtocol; }
extern "C" void destroy(CommIO_plugin *p) { delete p; }