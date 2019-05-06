#ifndef _BT_H
#define _BT_H

/* set up communications with BT module */
void bt_init(void);

/* enter command mode */
void bt_enter_command_mode(void);

/* send command cmd, then wait timeout milliseconds for response */
void bt_send_command(const char* cmd, unsigned int timeout);

/* leave command mode */
void bt_leave_command_mode(void);

/* start beeping at frequency freq */
void bt_beep(double freq);

/* stop beeping */
void bt_beep_off(void);

/* reset firmware on BT module to default */
void bt_reset(void);

/* advertise BT module */
void bt_advertise(void);

#ifdef TEST
void bt_write_str(const char* text);
void bt_read_str(char* buffer, unsigned int max_len);
#endif

#endif // _BT_H
