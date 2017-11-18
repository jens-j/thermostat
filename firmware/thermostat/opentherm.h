#ifndef OPENTHERM_H
#define OPENTHERM_H

#include <stdint.h>
#include "Arduino.h"
#include "common.h"

// MSG_ID codes
#define ID_STATUS                   0   // 8.8 master status, slave status
#define ID_CONTROL_SETPOINT         1   // [C] write-only
#define ID_SLAVE_CONFIG             3   // read-only
#define ID_FAULT                    5
#define ID_MODULATION_LEVEL         17
#define ID_WATER_PRESSURE           18  // not supported
#define ID_DHW_FLOW_RATE            19  // [l/m] 8.8 fixed point
#define ID_BOILER_WATER_TEMP        25  // [C] 8.8 fixed point, read-only
#define ID_DHW_TEMP                 26  // [C] 8.8 fixed point, read-only
#define ID_RETURN_WATER_TEMP        28  // not supported
#define ID_DHW_SETPOINT_BOUNDS      48  // [C] 8.8 max, min
#define ID_MAX_CH_SETPOINT_BOUNDS   49  // [C] 8.8 max, min
#define ID_DHW_SETPOINT             56  // [C] 8.8 fixed point, read-write
#define ID_MAX_CH_SETPOINT          57  // [C] 8.8 fixed point, read-write
#define ID_BURNER_STARTS            116
#define ID_CH_PUMP_STARTS           117
#define ID_DHW_PUMP_STARTS          118
#define ID_DHW_BURNER_STARTS        119
#define ID_BURNER_OP_HOURS          120
#define ID_CH_PUMP_OP_HOURS         121
#define ID_DHW_PUMP_OP_HOURS        122
#define ID_DHW_BURNER_OP_HOURS      123
#define ID_OT_VERSION_SLAVE         125 // not supported
#define ID_SLAVE_VERSION            127 // not supported

// opentherm timing constants
#define T_SLAVE_RESP                800  // [ms]
#define T_MASTER_IDLE               100  // [ms]
#define T_KEEPALIVE                 1150 // [ms]

typedef struct message_s {
    bool        parity;
    MsgType     msgType;
    uint8_t     dataId;
    uint16_t    dataValue;
} message_t;

// receive error code
enum ErrorCode {ERR_NONE,
                ERR_FIRST_EDGE,
                ERR_EDGE_EARLY,
                ERR_EDGE_LATE,
                ERR_TIMEOUT};

enum MsgType   {READ_DATA,
                WRITE_DATA,
                INVALID_DATA,
                RESERVED,
                READ_ACK,
                DATA_INVALID,
                UNKNOWN_DATA_ID};

const String MSG_TYPE[8] = {"READ_DATA",        // master to slave
                            "WRITE_DATA",       // |
                            "INVALID_DATA",     // |
                            "RESERVED",
                            "READ_ACK",         // slave to master
                            "WRITE_ACK",        // |
                            "DATA_INVALID",     // |
                            "UNKNOWN_DATA_ID"}; // |



class OpenTherm {

public:

    volatile bool recvFlag;           // receive data available flag. can be polled to check for received messages
    volatile uint64_t recvData;       // receive data buffer
    volatile message_t recvMsg;       // receive message struct
    volatile ErrorCode recvErrorCode; // receive error buffer
    volatile int recvErrorFlag;       // flag is set when a receive error is encountered
    volatile int recvCount;           // count received bits

    // constructor
    OpenTherm (int inPin,
               int outPin);

    // send a frame over the opentherm interface
    void sendFrame (uint32_t msgType, uint32_t dataId, uint32_t dataValue);

    // handle communication timeouts
    void wdtIsr ();

    // parse changes on the opentherm input
    void otIsr ();

    static message_t parseMessage (uint64_t);

    // pretty print a message
    static void printMsg (message_t);

    // calculate the positive parity bit value for a 32 bit word
    static uint32_t parity32 (uint32_t);

private:

    // pin configuration
    int inputPin;
    int outputPin;

    bool recvBusyFlag;   // flag is active during the parsing of a message
    uint64_t recvBuffer; // buffer for incoming data
    int midBitFlag;      // flag set after a mid cycle transition has occurred
    unsigned long recvTimeRef;

    // send a single machester encoded bit over the opentherm interface
    void sendMachesterBit(bool);

};

#endif OPENTHERM_H
