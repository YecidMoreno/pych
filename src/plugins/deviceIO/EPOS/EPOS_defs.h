#pragma once
#include <stdint.h>

namespace EPOS_CMD
{
    constexpr uint32_t SET_VELOCITY = 0x01;
    constexpr uint32_t SET_POSITION = 0x02;

    
    constexpr uint32_t SET_VELOCITY_MODE = 0x0100;
    constexpr uint32_t SET_CURRENT_MODE = 0x0102;

    constexpr uint32_t FAULT_RESET = 0x0101;

    constexpr uint32_t ENABLE_MOTORS = 0x0200;
}

typedef struct
{
    
} epos_read;
