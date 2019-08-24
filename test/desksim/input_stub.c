#include <zmq.h>
#include "unity.h"
#include "input_stub.h"
#include "zmqstubs.h"
#include "exception.h"
#include "mock_input.h"

enum INPUT input_stub(int numcalls) {
    char buffer[100];
    enum INPUT response;
    buffer[0] = 0x08;
    zmq_send(requester, buffer, 1, 0);
    zmq_recv(requester, buffer, 100, 0);
    if (buffer[0]!=0x08 || buffer[1]==0xFF) {
        printf("failing %d %d",buffer[0], buffer[1]);
        THROW_WITH_REASON("Failed simulation", ERROR_SIMULATION_COMMS_FAILED);
    } else {
        response = buffer[1];
    }
    return response;
}

void input_setup() {
    input_init_Ignore();
    timeout_reset_Ignore();
    get_input_StubWithCallback(input_stub);
}

