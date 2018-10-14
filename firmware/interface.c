#define USE_AND_OR
#include "config.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "interface.h"
#include "sensors.h"
#include "display.h"
#include "utils.h"
#include "measure.h"
#include "calibrate.h"
#include "mcc_generated_files/rtcc.h"
#include "mcc_generated_files/tmr2.h"
#include "mcc_generated_files/pin_manager.h"
#include "mcc_generated_files/interrupt_manager.h"

volatile enum ACTION last_click = NONE;

void beep(int a, int b) {
}

void set_date(int32_t a) {
    /* first display date */
    char text[18];
    int pos = 0;
    int pos_arr[] = {0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15};
    int done = false;
    struct tm dt;
    RTCC_TimeGet(&dt);
    display_clear_screen();
    while (true) {
        strcpy(text, "                ");
        text[pos_arr[pos]] = 'v';
        display_write_text(1, 0, text, &small_font, false);
        text[pos_arr[pos]] = '^';
        display_write_text(5, 0, text, &small_font, false);
        //sprintf(text,"%02X/%02X/%02X %02X:%02X X",1,2,3,4,5);
        sprintf(text, "%02d/%02d/%02d %02d:%02d X", dt.tm_mday, dt.tm_mon, dt.tm_year, dt.tm_hour, dt.tm_min);
        display_write_text(3, 0, text, &small_font, false);
        switch (get_action()) {
            case FLIP_LEFT:
                pos = (pos + 11 - 1) % 11;
                delay_ms(600);
                break;
            case FLIP_RIGHT:
                pos = (pos + 1) % 11;
                delay_ms(600);
                break;
            case FLIP_UP:
            case FLIP_DOWN:
                break;
            case SINGLE_CLICK:
            case DOUBLE_CLICK:
                return;
        }
        delay_ms(50);
    }
}

void set_time(int32_t a) {
}

/* a null-terminated list of menu_entries */


const struct menu_entry main_menu[] = {
    /* main menu */
    {-2, NULL, 0, NULL, 0},
    {0, "Measure", FUNCTION, measure, 0},
    {1, "Calibrate  >", 10, NULL, 0},
    {2, "Settings  >", 20, NULL, 0},
    {3, "Off", FUNCTION, sys_reset, 0},
    {4, NULL, 0, 0},

    /* calibrate menu */
    {10, "Quick", FUNCTION, quick_cal, 0},
    //{11,"Laser",FUNCTION,laser_cal, 0},
    //{12,"Align",FUNCTION,align_cal, 0},
    //{13,"Full",FUNCTION,full_cal, 0},
    {14, "Back", BACK, 0},
    {15, NULL, 10, 0},

    /* settings menu */
    {20, "Units  >", 30, NULL, 0},
    {21, "Function  >", 40, NULL, 0},
    {22, "Display  >", 50, NULL, 0},
    {23, "Timeout  >", 60, NULL, 0},
    {24, "Set  Date", FUNCTION, set_time, 0},
    {25, "Set  Time", FUNCTION, set_time, 0},
    {26, "Back", BACK, NULL, 0},
    {27, NULL, 20, 0},

    /* Units menu */
    {30, "Metric", FUNCTION, config_set_units, METRIC},
    {31, "Imperial", FUNCTION, config_set_units, IMPERIAL},
    {32, "Back", BACK, NULL, 0},
    {33, NULL, 30, NULL, 0},

    /* Function menu */
    {40, "Cartesian", FUNCTION, config_set_style, CARTESIAN},
    {41, "Polar", FUNCTION, config_set_style, POLAR},
    {42, "Grad", FUNCTION, config_set_style, GRAD},
    {43, "Back", BACK, 0},
    {44, NULL, 40, NULL, 0},

    /* Display menu */
    {50, "Day", FUNCTION, config_set_day, true},
    {51, "Night", FUNCTION, config_set_day, false},
    {52, "Back", BACK, NULL, 0},
    {53, NULL, 50, NULL, 0},
    
    /* Timeout menu*/
    {60, "30s", FUNCTION, config_set_timeout, 30},
    {61, "60s", FUNCTION, config_set_timeout, 60},
    {62, "2 min", FUNCTION, config_set_timeout, 120},
    {63, "5 min", FUNCTION, config_set_timeout, 300},
    {64, "10 mins", FUNCTION, config_set_timeout, 600},
    {65, "Back", BACK, NULL, 0},
    {66, NULL, 60, NULL, 0},
    /*end */
    {-1, NULL, -1, NULL, 0}
};


/* set up timer interrupts etc */
/* Timer 2 is our input poller counter */
/* timer 3 is click length counter */
/* timer 2 delay: 2ms  = */
#define CLICKS_PER_MS (FCY_PER_MS/256)
#define T2_DELAY (2*CLICKS_PER_MS)
uint32_t last_activity_counter = 0;

/* change notification interrupt, called every 2ms*/
void TMR2_CallBack(void) {
    static uint16_t state = 0x0001;
    last_activity_counter++;
    state = ((state << 1) | SWITCH_GetValue()) & 0x0fff;
    if (state == (SWITCH_ACTIVE_HIGH ? 0x07ff : 0x0800)) {
        /* we have just transitioned to a '1' and held it for 11 T2 cycles*/
        if (last_activity_counter > 200) {
            /* it's been more than a quarter second since the last press started */
            last_click = SINGLE_CLICK;
        } else {
            last_click = DOUBLE_CLICK;
        }
        last_activity_counter=0;
    }
    if (state == (SWITCH_ACTIVE_HIGH ? 0x0800 : 0x07ff)) {
        /* we have justtransitiioned to a '0' and held it for 11 T2 cycles */
        if (last_activity_counter > 1000) {
            last_click = LONG_CLICK;
        }
        last_activity_counter=0;
    }
    if (last_activity_counter>(config.timeout*500)) {
        sys_reset(0);
    }
}

void swipe_text(const char *text, bool left) {
    uint8_t my_buffer[4 * 128];
    int page;
    memset(my_buffer, 0, 128 * 4);
    for (page = 0; page < 4; page++) {
        //memset(&my_buffer[page*128],0xaa,128);
        render_text_to_page(&(my_buffer[page * 128]), page, 0, text, &large_font);
    }
    display_swipe_pages(day ? 0 : 2, my_buffer, 4, left);
}

void scroll_text(const char *text, bool up) {
    if (!day) {
        if (up) {
            display_clear_page(6);
            display_clear_page(7);
        } else {
            display_clear_page(0);
            display_clear_page(1);
        }
    }
    display_scroll_text(day ? 0 : 2, 0, text, &large_font, up);
}

/* get the offset of the menu_item with the specified index */

/* return -1 if index not found */
int16_t get_menu_item_offset(const struct menu_entry *menu, int16_t index) {
    int16_t i;
    for (i = 0; menu[i].index != -1; ++i) {
        if (menu[i].index == index) return i;
    }
    return -1;
}

bool valid_text(const struct menu_entry *menu, int16_t index) {
    if (menu[index].text) {
        if (strlen(menu[index].text)) return true;
    }
    return false;
}

/* return index of previous menu item, rotate onto last menu item if currently on first item */
int16_t get_previous_menu_item(const struct menu_entry *menu, int16_t index) {
    if (valid_text(menu,index-1)) return index - 1;
    while (valid_text(menu,index)) index++;
    return index;
}

/* return index of next menu item, rotating onto first menu item if currently on last item */
int16_t get_next_menu_item(const struct menu_entry *menu, int16_t index) {
    if (valid_text(menu,index + 1)) {
        return index + 1;
    } else {
        return get_menu_item_offset(menu, menu[index + 1].next_menu);
    }
}

void menu_set_entry(struct menu_entry *menu, int16_t index, char *text, int16_t next_menu, void (*action) (int), int32_t argument) {
    menu->index = index;
    strncpy(menu->text, text, 20);
    menu->next_menu = next_menu;
    menu->action = action;
    menu->argument = argument;
}

enum ACTION get_action() {
    struct COOKED_SENSORS sensors;
    enum ACTION temp;

    sensors_read_cooked(&sensors);
    /* look for "flip" movements */
    //debug("f%.2g",sensors.gyro[1]);
    if (sensors.gyro[1] > 30.0) {
        last_activity_counter = 0;
        return display_inverted ? FLIP_DOWN : FLIP_UP;
    }
    if (sensors.gyro[1]<-30.0) {
        last_activity_counter = 0;
        return display_inverted ? FLIP_UP : FLIP_DOWN;
    }
    if (sensors.gyro[0] > 30.0) {
        last_activity_counter = 0;
        return display_inverted ? FLIP_LEFT : FLIP_RIGHT;
    }
    if (sensors.gyro[0]<-30.0) {
        last_activity_counter = 0;
        return display_inverted ? FLIP_RIGHT : FLIP_LEFT;
    }
    /* check to see if display needs flipping */
    /* use 0.5g - gives a hysteresis of about +/- 30 degrees */
    if ((sensors.accel[0]<-0.5) && display_inverted) display_flip(false);
    if ((sensors.accel[0] > 0.5) && !display_inverted) display_flip(true);
    /* search for a click */
    if (last_click != NONE) {
        /* momentarily disable interrupts */
        INTERRUPT_GlobalDisable();
        temp = last_click;
        last_click = NONE;
        INTERRUPT_GlobalEnable();
        last_activity_counter = 0;
        return temp;
    }
    //nothing else found - so return NONE
    return NONE;
}


unsigned char reverse(unsigned char b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

void show_status() {
    char header[17];
    char footer[17] = "                "; //16 spaces
#define FOOTER_LENGTH 16
    int x;
    struct tm dt;
    /* batt icon 50% charge: 0x1f,0x20,0x2f*9,0x20*9,0x20,0x1f,0x04,0x03 */
    /* reverse bit order for second line */
    char bat_status[24];
    int charge;
    charge = battery_get_units();
    if (!day) {
        bat_status[0] = 0xf8;
        bat_status[1] = 0x04;
        memset(&bat_status[2], 0xf4, charge);
        memset(&bat_status[2 + charge], 0x04, 19 - charge);
        bat_status[21] = 0xf8;
        bat_status[22] = 0x20;
        bat_status[23] = 0xC0;
        RTCC_TimeGet(&dt);
        switch (config.length_units) {
            case METRIC:
                memcpy(footer, "Metric", 6);
                break;
            case IMPERIAL:
                memcpy(footer, "Imp.", 4);
                break;
        }
        switch (config.display_style) {
            case POLAR:
                memcpy(&footer[FOOTER_LENGTH - 5], "Polar", 5);
                break;
            case GRAD:
                memcpy(&footer[FOOTER_LENGTH - 5], "Grad.", 5);
                break;
            case CARTESIAN:
                memcpy(&footer[FOOTER_LENGTH - 5], "Cart.", 5);
                break;
        }
        snprintf(header, 17, "%02d:%02d        ", dt.tm_hour, dt.tm_min);
        display_write_text(0, 0, header, &small_font, false);
        display_write_text(6, 0, footer, &small_font, false);
        render_data_to_page(0, 104, bat_status, 24);
        for (x = 0; x < 24; ++x) {
            bat_status[x] = reverse(bat_status[x]);
        }
        render_data_to_page(1, 104, bat_status, 24);
    }
}

bool show_menu(const struct menu_entry *menu, int16_t index, bool first_time) {
    enum ACTION action;
    bool function_called = false;
    if (first_time) {
        scroll_text(menu[index].text, true);
    } else {
        swipe_text(menu[index].text, true);
    }
    while (true) {
        wdt_clear();
        delay_ms(50);
        show_status();
        action = get_action();
        switch (action) {
            case FLIP_DOWN:
                //index = get_previous_menu_item(menu, index);
                //scroll_text(menu[index].text, false);
                break;
            case FLIP_UP:
                index = get_next_menu_item(menu, index);
                scroll_text(menu[index].text, true);
                break;
                //case FLIP_RIGHT:
            case SINGLE_CLICK:
                beep(3600, 20);
                switch (menu[index].next_menu) {
                    case FUNCTION:
                        if (menu[index].action){
                            menu[index].action(menu[index].argument);
                            }
                        function_called = true;
                        break;
                    case BACK:
                        if (!first_time) return false;
                        break;
                    case INFO: //do nothing on click if INFO item.
                        break;
                    default:
                        function_called = show_menu(menu, get_menu_item_offset(menu, menu[index].next_menu), false);
                        break;
                }
                if (function_called) {
                    if (first_time) {
                        /* we have got back to the root screen */
                        index = FIRST_MENU_ITEM;
                        display_clear_screen();
                        scroll_text(menu[index].text, true);
                    } else {
                        return true;
                    }
                } else {
                    /* sub-menu: back selected */
                    swipe_text(menu[index].text, false);
                }
                break;
                //             case FLIP_LEFT:
                //                 if (!first_time) return false;
                //                 break;
            case DOUBLE_CLICK:
                //hibernate();
                break;
        }
        if (action != NONE) {
            show_status();
        }
    }
}
