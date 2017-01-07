#include "Arduino.h"
#include <stdint.h>


#define ID_STATUS               0
#define ID_SETPOINT             1
#define ID_FAULT                5 
#define ID_SLAVE_CONFIG         3
#define ID_OT_VERSION_SLAVE     125
#define ID_SLAVE_VERSION        127
#define ID_MODULATION_LEVEL     17
#define ID_WATER_PRESSURE       18
#define ID_DHW_FLOW_RATE        19
#define ID_ROOM_TEMP            24
#define ID_BOILER_WATER_TEMP    25
#define ID_DHW_TEMP             26
#define ID_RETURN_WATER_TEMP    28


// receive error code
enum ErrorCode {ERR_NONE, 
                ERR_FIRST_EDGE, 
                ERR_SECOND_EDGE, 
                ERR_EDGE_EARLY, 
                ERR_EDGE_LATE, 
                ERR_TIMEOUT};

enum MsgType   {READ_DATA,
                WRITE_DATA};

const String MSG_TYPE[8] = {"READ_DATA",
                            "WRITE_DATA",
                            "INVALID_DATA",
                            "RESERVED",
                            "READ_ACK",
                            "WRITE_ACK",
                            "DATA_INVALID",
                            "UNKNOWN_DATA_ID"};



class OpenTherm {

public:
    
    volatile bool recvFlag;           // receive data available flag. can be polled to check for received messages
    volatile uint64_t recvData;       // receive data buffer
    volatile ErrorCode recvErrorCode; // receive error buffer
    volatile int recvErrorFlag;   // flag is set when a receive error is encountered 
    volatile int recvCount;       // count received bits

    // constructor
    OpenTherm(int, int, int);

    // send a frame over the opentherm interface
    void sendFrame(uint32_t, uint32_t, uint32_t);

    // handle communication timeouts
    void wdtIsr();

    // parse changes on the opentherm input
    void recvIsr();

    static void printMsg(uint64_t);

private:

    // pin configuration
    int inputPin;
    int outputPin;
    int interruptNumber;

    bool recvBusyFlag;   // flag is active during the parsing of a message
    uint64_t recvBuffer; // buffer for incoming data
    int midBitFlag;      // flag set after a mid cycle transition has occurred
    unsigned long recvTimeRef; 

    // send a single machester encoded bit over the opentherm interface
    void sendMachesterBit(int);

    // calculate the positive parity bit value for a 32 bit word
    uint32_t parity32(uint32_t);
    
};
