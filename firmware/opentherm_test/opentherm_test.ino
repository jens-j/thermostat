#include "common.h"
#include "opentherm.h"

OpenTherm ot = OpenTherm(OT_INPUT_PIN, OT_OUTPUT_PIN);


void setup() {

    Serial.begin(115200);
    delay(2000);
}


void loop() {

    int i;
    ot_recv_error_t recvError;
    uint64_t frameBuf;
    int recvCount;
    char cBuf[100];
    int ids[] = {ID_DHW_TEMP, ID_DHW_FLOW_RATE, ID_BOILER_WATER_TEMP, ID_DHW_SETPOINT_BOUNDS, ID_DHW_SETPOINT,
                 ID_SLAVE_CONFIG, ID_REMOTE_PARAMETER, ID_STATUS, ID_DHW_BURNER_STARTS, ID_BURNER_OP_HOURS};

    for (i = 0; i < sizeof(ids) / 2; i++) {

        ot.sendFrame(READ_DATA, ids[i], 0);
        recvError = ot.recvReply(&frameBuf, &recvCount);

        if (recvError != OT_RECV_ERR_NONE) {
            Serial.print("receive error: ");
            Serial.println(OT_RECV_ERROR_T_STR[recvError]);
        }

        ot.printFrame(frameBuf);

        delay(1000);
    }
}
