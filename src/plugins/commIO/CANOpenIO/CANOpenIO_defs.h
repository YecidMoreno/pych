#pragma once

#include <stdint.h>

#define MAX_NODES 24
#define ID_SDO 0x600
#define ID_RPDO_BASE 0x200
#define ID_TPDO_BASE 0x180
#define ID_NMT 0x000
#define ID_SYNC 0x080

#define B1_PAYLOAD 0x2F
#define B2_PAYLOAD 0x22
#define B3_PAYLOAD 0x00
#define B4_PAYLOAD 0x22


typedef struct {
    int NodeID;
    int TPDO_Index;
    int lsb_byte=0;
    int n_bytes=8;
} arg_CANOpen_receive;

typedef struct {
    int NodeID;
    int TPDO_Index;
} arg_CANOpen_send;

enum class CANOpenSpecial : uint8_t { NMT = 1, SYNC = 2 };

struct canopen_cmd {
    CANOpenSpecial type;
    uint8_t data[2];  // Para NMT: [cmd, node_id]
};

const canopen_cmd canopen_cmd_MNT{CANOpenSpecial::NMT, {0x01, 0x00}};
const canopen_cmd canopen_cmd_SYNC{CANOpenSpecial::SYNC, {0x00, 0x00}};