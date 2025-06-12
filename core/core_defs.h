#pragma once
#include <string>

namespace HH
{
    enum class AppState
    {
        IDLE,
        RUNNING,
        STOPPING,
        CLOSING
    };

    struct logger_core_config
    {
        bool enable = false;
        std::string path = "logs";
    };

    struct core_config
    {
        logger_core_config logger;
    };

};