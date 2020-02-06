#define USE_AND_OR
#include "config.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "battery.h"
#include "interface.h"
#include "input.h"
#include "display.h"
#include "utils.h"
#include "measure.h"
#include "calibrate.h"
#include "menu.h"
#include "exception.h"
#include "debug.h"
#include "visualise.h"
#include "datetime.h"
#include "beep.h"
#include "laser.h"
#include "mcc_generated_files/rtcc.h"
#include "mcc_generated_files/tmr2.h"
#include "mcc_generated_files/pin_manager.h"
#include "mcc_generated_files/interrupt_manager.h"


DECLARE_MENU(timeout_menu, {
    {"30s", Action, {config_set_timeout}, 30},
    {"60s", Action, {config_set_timeout}, 60},
    {"2  min", Action, {config_set_timeout}, 120},
    {"5  min", Action, {config_set_timeout}, 300},
    {"10  mins", Action, {config_set_timeout}, 600},
    {"Back", Back, {NULL}, 0}
});

DECLARE_MENU(display_menu, {
    /* Display menu */
    {"Day", Action, {config_set_day}, true},
    {"Night", Action, {config_set_day}, false},
    {"Back", Back, {NULL}, 0}
});

DECLARE_MENU(style_menu, {
    {"Cartesian", Action, {config_set_style}, CARTESIAN},
    {"Polar", Action, {config_set_style}, POLAR},
    {"Grad", Action, {config_set_style}, GRAD},
    {"Back", Back, {NULL}, 0}
});

DECLARE_MENU(units_menu, {
    {"Metric", Action, {config_set_units}, METRIC},
    {"Imperial", Action, {config_set_units}, IMPERIAL},
    {"Back", Back, {NULL}, 0}
});

DECLARE_MENU(settings_menu, {    /* settings menu */
    {"Units  >", SubMenu, .submenu = &units_menu, 0},
    {"Style  >", SubMenu, .submenu = &style_menu, 0},
    {"Display  >", SubMenu, .submenu = &display_menu, 0},
    {"Timeout  >", SubMenu, .submenu = &timeout_menu, 0},
    {"Set  Date", Action, {datetime_set_date}, 0},
    {"Set  Time", Action, {datetime_set_time}, 0},
    {"Back", Back, {NULL}, 0},
});

DECLARE_MENU(main_menu, {
    {"Measure", Exit, {NULL}, 0},
    {"Calibrate  >", SubMenu, .submenu = &calibration_menu, 0},
    {"Settings  >", SubMenu, .submenu = &settings_menu, 0},
    {"Visualise", Action, {visualise_show_menu}, 0},
    {"Info  >", SubMenu, .submenu = &debug_menu, 0},
    {"Off", Action, {utils_turn_off}, 0}
});

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
            display_clear_page(6, true);
            display_clear_page(7, true);
        } else {
            display_clear_page(0, true);
            display_clear_page(1, true);
        }
    }
    display_scroll_text(day ? 0 : 2, 0, text, &large_font, up);
}



unsigned char reverse(unsigned char b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

void show_status() {
    const int FOOTER_LENGTH = 16;
    char header[17];
    char footer[17] = "                "; //16 spaces
    int x;
    struct tm dt;
    /* batt icon 50% charge: 0x1f,0x20,0x2f*9,0x20*9,0x20,0x1f,0x04,0x03 */
    /* reverse bit order for second line */
    unsigned char bat_status[24];
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
        display_write_text(0, 0, header, &small_font, false, true);
        display_write_text(6, 0, footer, &small_font, false, true);
        render_data_to_page(0, 104, bat_status, 24);
        for (x = 0; x < 24; ++x) {
            bat_status[x] = reverse(bat_status[x]);
        }
        render_data_to_page(1, 104, bat_status, 24);
    }
}

void show_menu(struct menu *menu) {
    laser_off();
    menu_initialise(menu);
    scroll_text(menu_get_text(menu), true);
    while (true) {
        wdt_clear();
        delay_ms(50);
        show_status();
        switch (get_input()) {
            case FLIP_DOWN:
                //index = get_previous_menu_item(menu, index);
                //scroll_text(menu_get_text(menu), false);
                break;
            case FLIP_UP:
                menu_next(menu);
                scroll_text(menu_get_text(menu), true);
                break;
                //case FLIP_RIGHT:
            case SINGLE_CLICK:
                beep(512, 20);
                switch (menu_action(menu)) {
                    case Action:
                    case Exit:
                        return;
                        break;
                    case Back:
                        swipe_text(menu_get_text(menu), false);
                        break;
                    case SubMenu: 
                        swipe_text(menu_get_text(menu), true);
                        break;
                    case Info: //do nothing on click if INFO item.
                        break;
                }
                break;
            case DOUBLE_CLICK:
                utils_turn_off(0);
                break;
            case LONG_CLICK:
                measure_requested=true;
                return;
                break;
            default:
                break;
        }
    }
}
