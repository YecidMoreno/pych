#pragma once

#include <stdint.h>

#define MAX_NODES 127
#define MAX_PDO 4
#define ID_SDO 0x600
#define ID_RPDO_BASE 0x200
#define ID_TPDO_BASE 0x180
#define ID_NMT 0x000
#define ID_SYNC 0x080

#define B1_PAYLOAD 0x2F
#define B2_PAYLOAD 0x22
#define B3_PAYLOAD 0x00
#define B4_PAYLOAD 0x22

typedef struct
{
    uint32_t CAN_ID = 0;
    int NodeID;
    int TPDO_Index;
    int byte_0 = 0;
    int n_bytes = 8;
    bool raw = false;
    bool reverse = false;
} arg_CANOpen_receive;

typedef struct
{
    int NodeID;
    int TPDO_Index;
} arg_CANOpen_send;

enum class CANOpenSpecial : uint8_t
{
    NMT = 1,
    SYNC = 2,
    RPDO = 3
};

struct canopen_cmd
{
    CANOpenSpecial type;
    uint8_t data[2]; // Para NMT: [cmd, node_id]
    uint32_t arg0=0;
    uint32_t arg1=0;
};

const canopen_cmd canopen_cmd_MNT{CANOpenSpecial::NMT, {0x01, 0x00}};
const canopen_cmd canopen_cmd_SYNC{CANOpenSpecial::SYNC, {0x00, 0x00}};

void print_can_message(struct can_frame &frame);