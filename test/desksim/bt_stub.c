#include <stdbool.h>
#include "unity.h"
#include "bt_stub.h"
#include "mock_bt.h"

bool bt_present = true;


void bt_setup(void) {
    bt_init_Ignore();
    bt_connected_IgnoreAndReturn(false);
    bt_send_command_Ignore();
}
