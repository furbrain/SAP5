#include "unity.h"
#include <zmq.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "mock_i2c1.h"

void static *context;
void static *requester;
bool static connected = false;

static void
s_dump (void *socket)
{
    int rc;

    zmq_msg_t message;
    rc = zmq_msg_init (&message);
    assert (rc == 0);

    puts ("----------------------------------------");
    //  Process all parts of the message
    do {
        int size = zmq_msg_recv (&message, socket, 0);
        assert (size >= 0);

        //  Dump the message as text or binary
        char *data = (char*)zmq_msg_data (&message);
        assert (data != 0);
        int is_text = 1;
        int char_nbr;
        for (char_nbr = 0; char_nbr < size; char_nbr++) {
            if ((unsigned char) data [char_nbr] < 32
                || (unsigned char) data [char_nbr] > 126) {
                is_text = 0;
            }
        }

        printf ("[%03d] ", size);
        for (char_nbr = 0; char_nbr < size; char_nbr++) {
            if (is_text) {
                printf ("%c", data [char_nbr]);
            } else {
                printf ("%02X", (unsigned char) data [char_nbr]);
            }
        }
        printf ("\n");
    } while (zmq_msg_more (&message));

    rc = zmq_msg_close (&message);
    assert (rc == 0);
}



void zmq_setup() {
    if (!connected) {
        connected = true;
        context = zmq_ctx_new();
        requester = zmq_socket (context, ZMQ_REQ);
        zmq_connect (requester, "tcp://localhost:5555");
    }
}

void i2c_write_stub(uint8_t *data, uint8_t length, uint16_t address, I2C1_MESSAGE_STATUS *status, int numcalls) {
    char buffer[300];
    buffer[0] = 0x01;
    buffer[1] = (char)address*2;
    memcpy(buffer+2, data, length);
    zmq_send (requester, buffer, length+2, 0);
    s_dump(requester);
}

void i2c_setup() {
    zmq_setup();
    I2C1_Initialize_Ignore();
    I2C1_MasterWrite_StubWithCallback(i2c_write_stub);  
}
