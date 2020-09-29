#ifndef _BT_H
#define _BT_H
#include <stdbool.h>
/* set up communications with BT module */
void bt_init(void);

/* send command cmd, then wait timeout milliseconds for response */
void bt_send_command(const char* cmd, const char* expected_response, unsigned int timeout);

void bt_send_packet(const char* packet, int length);

/* start beeping at frequency freq */
void bt_beep_start(int freq);

/* stop beeping */
void bt_beep_stop(void);

/* return true if bt connected to other device*/
bool bt_connected(void);

extern bool bt_present;

#ifdef TEST
void bt_write_str(const char* text);
void bt_read_str(char* buffer, unsigned int max_len);
#endif

#endif // _BT_H
