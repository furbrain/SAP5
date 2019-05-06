#ifndef _BT_H
#define _BT_H

void bt_init(void);

void bt_enter_command_mode(void);

void bt_send_command(const char* cmd, unsigned int timeout);

void bt_leave_command_mode(void);


#ifdef TEST
void bt_write_str(const char* text);
void bt_read_str(char* buffer, unsigned int max_len);
#endif

#endif // _BT_H
