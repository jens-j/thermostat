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

// opentherm 32 bit message structure
typedef struct message_s {
    bool        parity;
    MsgType     msgType;
    uint8_t     dataId;
    uint16_t    dataValue;
} message_t;

// receive error codes
enum ot_recv_error_t {ERR_NONE,
                      ERR_FIRST_EDGE, // first edge has wrong direction
                      ERR_EDGE_EARLY, // edge arrived too early for specifications
                      ERR_EDGE_LATE,  // edge arrived too late for specifications
                      ERR_TIMEOUT};   // communication timeout (too few bits received)

// receive error codes
#define OT_PARSE_ERR_NONE   0x0
#define OT_PARSE_ERR_START  0x1 // missing start bit
#define OT_PARSE_ERR_STOP   0x2 // missing stop bit
#define OT_PARSE_ERR_PARITY 0x4 // incorrectly set parity bit
#define OT_PARSE_ERR_SIZE   0x8 // too much bits

// opentherm message ids
enum ot_msg_t {READ_DATA,
               WRITE_DATA,
               INVALID_DATA,
               RESERVED,
               READ_ACK,
               DATA_INVALID,
               UNKNOWN_DATA_ID};

// message id strings for printings
const String OT_MSG_T_STR[8] = {"READ_DATA",        // master to slave
                                "WRITE_DATA",       // |
                                "INVALID_DATA",     // |
                                "RESERVED",         // |
                                "READ_ACK",         // slave to master
                                "WRITE_ACK",        // |
                                "DATA_INVALID",     // |
                                "UNKNOWN_DATA_ID"}; // |

// opentherm receive error strings
const String OT_RECV_ERROR_T_STR[8] = {"ERR_NONE",
                                       "ERR_FIRST_EDGE",
                                       "ERR_EDGE_EARLY",
                                       "ERR_EDGE_LATE",
                                       "ERR_TIMEOUT"};

// Opentherm interface class
// this class can send frames by bitbanging the ot interface.
// And external interrupt is used to receive replies.
//
class OpenTherm {

public:

    volatile bool recvFlag;                 // receive data available flag. can be polled to check for received messages
    volatile uint64_t recvData;             // receive data buffer
    volatile int recvErrorFlag;             // flag is set when a receive error is encountered
    volatile ot_recv_error_t recvErrorCode; // receive error buffer
    volatile int recvCount;                 // count received bits

    // constructor
    OpenTherm (int inPin,
               int outPin);

    // send a frame over the opentherm interface
    void sendFrame (uint32_t msgType, uint32_t dataId, uint32_t dataValue);

    // handle communication timeouts
    void wdtIsr ();

    // parse changes on the opentherm input
    void otIsr ();

    // parses a 34 bit frame to a message struct and returns true if the start,
    // stop and parity bits were set correctly
    static bool parseFrame (uint64_t buf, message_t *msg);

    // parse a 32 bit message into separate fields
    static message_t parseMessage (uint32_t msg);

    // pretty print a message
    static void printFrame (uint64_t buf);

    // calculate the positive parity bit value for a 32 bit word
    static uint32_t parity32 (uint32_t mag);

private:

    // pin configuration
    int inputPin_;
    int outputPin_;

    bool recvBusyFlag_;         // flag is active during the parsing of a message
    uint64_t recvBuffer_;       // buffer for incoming data
    int midBitFlag_;            // flag set after a mid cycle transition has occurred
    unsigned long recvTimeRef_; // [ms] timestamp of last observed ot level change
    unsigned long idleTimeRef_; // [ms] timestamp of the end of the last received frame

    // send a single machester encoded bit over the opentherm interface
    void sendMachesterBit(bool);

};

#endif OPENTHERM_H
