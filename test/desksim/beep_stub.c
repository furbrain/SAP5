#include <zmq.h>
#include "unity.h"
#include "beep_stub.h"
#include "zmqstubs.h"
#include "exception.h"
#include "mock_utils.h"

void beep_stub(double frequency, int duration, int numcalls) {
    struct message {
        char id;
        double f;
        int duration;
    };
    struct message msg;
    char buffer[100];
    msg.id = 0x07;
    msg.f = frequency;
    msg.duration = duration;
    zmq_send(requester, &msg, 20, 0);
    zmq_recv(requester, buffer, 100, 0);
    if (buffer[0]!=0x07 || buffer[1]!=0x00) {
        printf("failing %d %d",buffer[0], buffer[1]);
        THROW_WITH_REASON("Failed simulation", ERROR_SIMULATION_COMMS_FAILED);
    }

}

void beep_setup() {
    bt_and_beep_initialise_Ignore();
    beep_StubWithCallback(beep_stub);
}
