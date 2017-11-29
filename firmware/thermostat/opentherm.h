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
#define ID_REMOTE_PARAMETER         6   // 8.8 transfer, ro/rw for DHW stpoint [0] and max CH setpoint [1]
#define ID_MODULATION_LEVEL         17
#define ID_WATER_PRESSURE           18  // not supported
#define ID_DHW_FLOW_RATE            19  // [l/m] 8.8 fixed point
#define ID_BOILER_WATER_TEMP        25  // [C] 8.8 fixed point, read-only
#define ID_DHW_TEMP                 26  // [C] 8.8 fixed point, read-only
#define ID_RETURN_WATER_TEMP        28  // not supported
#define ID_DHW_SETPOINT_BOUNDS      48  // [C] 8.8 max, min. read-only
#define ID_MAX_CH_SETPOINT_BOUNDS   49  // [C] 8.8 max, min. read-only
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

// opentherm 32 bit message structure
typedef struct message_s {
    bool        parity;
    ot_msg_t    msgType;
    uint8_t     dataId;
    uint16_t    dataValue;
} message_t;

// receive error codes
enum ot_recv_error_t {OT_RECV_ERR_NONE,
                      OT_RECV_ERR_FIRST_EDGE, // first edge has wrong direction
                      OT_RECV_ERR_EDGE_EARLY, // edge arrived too early for specifications
                      OT_RECV_ERR_EDGE_LATE,  // edge arrived too late for specifications
                      OT_RECV_ERR_INCOMPLETE, // inclomplete message received (wdt timeout without other errors occurring)
                      OT_RECV_ERR_TIMEOUT};   // nothing received before the 800 ms response time elapsed


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
const String OT_RECV_ERROR_T_STR[6] = {"ERR_NONE",
                                       "ERR_FIRST_EDGE",
                                       "ERR_EDGE_EARLY",
                                       "ERR_EDGE_LATE",
                                       "OT_RECV_ERR_INCOMPLETE",
                                       "ERR_TIMEOUT"};

// Opentherm interface class
// this class can send frames by bitbanging the ot interface.
// An external interrupt is used to receive replies.
class OpenTherm {

public:

    // constructor
    OpenTherm (int inPin,
               int outPin);

    // send a frame over the opentherm interface
    void sendFrame (uint32_t msgType, uint32_t dataId, uint32_t dataValue);

    // receive the reply to a read or write request. return the recv error code.
    ot_recv_error_t recvReply(uint64_t *frameBuf, int *n);

    // handle communication timeouts
    void wdtIsr ();

    // parse changes on the opentherm input
    void otIsr ();

    // parses a 34 bit frame to a message struct and returns true if formatted correctly
    static uint8_t parseFrame (uint64_t frameBuf, message_t *msg);

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

    // opentherm receive isr interface buffer
    volatile bool recvFlag_;                 // receive data available flag. can be polled to check for received messages
    volatile uint64_t recvData_;             // receive data buffer
    volatile ot_recv_error_t recvErrorCode_; // receive error buffer
    volatile int recvCount_;                 // count received bits

    // isr internal variables
    bool recvBusyFlag_;         // flag is active during the parsing of a message. used to identify the first edge
    uint64_t recvBuffer_;       // buffer for incoming data
    int midBitFlag_;            // fla is set after a mid cycle transition has occurred
    volatile int recvErrorFlag_;// flag is set when a receive error is encountered. used to ignore the rest of the frame
    unsigned long recvTimeRef_; // [ms] timestamp of last observed ot level change
    unsigned long idleTimeRef_; // [ms] timestamp of the end of the last received frame

    void setRecvError_(ot_recv_error_t); // report an receive error to the class interface
    void sendMachesterBit_(bool);        // send a single machester encoded bit over the opentherm interface

};

#endif OPENTHERM_H
