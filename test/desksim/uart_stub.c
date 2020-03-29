#include <zmq.h>
#include <string.h>
#include "unity.h"
#include "input_stub.h"
#include "zmqstubs.h"
#include "exception.h"
#include "mock_uart1.h"

unsigned int UART1_WriteBuffer_Stub( const uint8_t *buf , const unsigned int numbytes, int numcalls ) {
    char buffer[100];
    buffer[0] = 0x03;
    memcpy(&buffer[1],buf, numbytes);
    zmq_send(requester, buffer, numbytes + 1, 0);
    zmq_recv(requester, buffer, 100, 0);
    if (buffer[0]!=0x03) {
        printf("failing %d %d",buffer[0], buffer[1]);
        THROW_WITH_REASON("Failed simulation", ERROR_SIMULATION_COMMS_FAILED);
    }
    return buffer[1];
}

void UART1_Write_Stub( const uint8_t byte, int numcalls) {
    char buffer[100];
    buffer[0] = 0x03;
    buffer[1] = byte;
    zmq_send(requester, buffer, 2, 0);
    zmq_recv(requester, buffer, 100, 0);
    if (buffer[0]!=0x03) {
        printf("failing %d %d",buffer[0], buffer[1]);
        THROW_WITH_REASON("Failed simulation", ERROR_SIMULATION_COMMS_FAILED);
    }
}

unsigned int UART1_ReadBuffer_Stub( uint8_t *buf,  const unsigned int numbytes, int numcalls) {
    char buffer[100];
    int count;
    buffer[0] = 0x04;
    zmq_send(requester, buffer, 1, 0);
    count = zmq_recv(requester, buffer, 100, 0);
    memcpy(buf, &buffer[1], count-1);
    if (buffer[0]!=0x04) {
        printf("failing %d %d",buffer[0], buffer[1]);
        THROW_WITH_REASON("Failed simulation", ERROR_SIMULATION_COMMS_FAILED);
    }
    return count-1;
}
unsigned int UART1_ReceiveBufferSizeGet_Stub(int numcalls) {
    char buffer[100];
    int count;
    buffer[0] = 0x05;
    zmq_send(requester, buffer, 1, 0);
    zmq_recv(requester, buffer, 100, 0);
    if (buffer[0]!=0x05) {
        printf("failing %d %d",buffer[0], buffer[1]);
        THROW_WITH_REASON("Failed simulation", ERROR_SIMULATION_COMMS_FAILED);
    }
    return buffer[1];
}
void uart1_setup() {
    UART1_Initialize_Ignore();
    UART1_ReceiveBufferClear_Ignore();
    UART1_ReadBuffer_StubWithCallback(UART1_ReadBuffer_Stub);
    UART1_WriteBuffer_StubWithCallback(UART1_WriteBuffer_Stub);
    UART1_Write_StubWithCallback(UART1_Write_Stub);
    UART1_ReceiveBufferSizeGet_StubWithCallback(UART1_ReceiveBufferSizeGet_Stub);
}

