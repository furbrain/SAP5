#include <zmq.h>
#include <string.h>
#include <time.h>
#include "unity.h"
#include "time_stub.h"
#include "zmqstubs.h"
#include "exception.h"
#include "mock_rtcc.h"

bool RTCC_TimeGet_stub(struct tm *currentTime, int numcalls) {
    char buffer[100];
    time_t now;
    buffer[0] = 0x09;
    zmq_send(requester, buffer, 1, 0);
    zmq_recv(requester, buffer, 100, 0);
    if (buffer[0]!=0x09 || buffer[1]==0xFF) {
        printf("failing %d %d",buffer[0], buffer[1]);
        THROW_WITH_REASON("Failed simulation", ERROR_SIMULATION_COMMS_FAILED);
    } else {
        memcpy(&now, &buffer[1], sizeof(time_t));
        memcpy(currentTime, localtime(&now), sizeof (struct tm));
    }
    return true;
}

void time_setup() {
    RTCC_TimeGet_StubWithCallback(RTCC_TimeGet_stub);
}

