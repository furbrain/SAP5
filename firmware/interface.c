#define USE_AND_OR
#include "config.h"
#include <libpic30.h>
#include <stdint.h>
#include <timer.h>
#include <ports.h>
#include <stdio.h>
#include <string.h>
#include <Rtcc.h>
#include <math.h>
#include "interface.h"
#include "sensors.h"
#include "display.h"
#include "power.h"
#include "calibrate.h"
#include "debug.h"
#include "battery.h"
#include "measure.h"

struct menu_entry {
	int16_t index;
	const char* text;
	int16_t next_menu; /* start of next menu (or this menu if text NULL */
	void (*action) (void); /*action to perform when selected */
};


volatile enum ACTION last_click = NONE;
void set_date() {
		/* first display date */
	char text[18];
	int pos = 0;
	int pos_arr[] = {0,1,3,4,6,7,9,10,12,13,15};
	int done = false;
	rtccTimeDate dt;
	RtccReadTimeDate(&dt);
	display_clear_screen();
	while (true) {
		strcpy(text,"                ");
		text[pos_arr[pos]] = 'v';
		display_write_text(1,0,text,&small_font,false);
		text[pos_arr[pos]] = '^';
		display_write_text(5,0,text,&small_font,false);
		//sprintf(text,"%02X/%02X/%02X %02X:%02X X",1,2,3,4,5);
		sprintf(text,"%02d/%02d/%02d %02d:%02d X",dt.f.mday,dt.f.mon,dt.f.year,dt.f.hour,dt.f.min);
		display_write_text(3,0,text,&small_font,false);
		switch (get_action()) {
			case FLIP_LEFT:
				beep(3600,20);   
				pos = (pos+11-1)%11;
				delay_ms(600);
				break;
			case FLIP_RIGHT:
				beep(3600,20);   
				pos = (pos+1)%11;
				delay_ms(600);
				break;
			case FLIP_UP:
			case FLIP_DOWN:
				beep(3600,20);   
				break;
			case SINGLE_CLICK:
			case DOUBLE_CLICK:
				beep(3600,20);   
				return;
		}
		delay_ms(50);
	}
}
void set_time() {}

/* a null-terminated list of menu_entries */
#define FUNCTION -1
#define BACK -2


const struct menu_entry menu_items[] = {
	/* main menu */
	{-2,NULL,0,NULL},
	{0,"Measure",FUNCTION,measure},
	{1,"Calibrate  >",10,NULL},
	{2,"Settings  >",20,NULL},
	{3,"Off",-1,hibernate},
	{4,NULL,0,NULL},
	
	/* calibrate menu */
	{10,"Quick",FUNCTION,quick_cal},
	{11,"Laser",FUNCTION,laser_cal},
	{12,"Align",FUNCTION,align_cal},
	{13,"Full",FUNCTION,full_cal},
	{14,"Back",BACK,NULL},
	{15,NULL,10,NULL},
	
	/* settings menu */
	{20,"Units  >",30,NULL},
	{21,"Function  >",40,NULL},
	{22,"Display  >",50,NULL},
	{23,"Set  Date",FUNCTION,set_date},
	{24,"Set  Time",FUNCTION,set_time},
	{25,"Back",BACK,NULL},
	{26,NULL,20,NULL},
	
	/* Units menu */
	{30,"Metric",FUNCTION,set_metric},
	{31,"Imperial",FUNCTION,set_imperial},
	{32,"Back",BACK,NULL},
	{33,NULL,30,NULL},
	
	/* Function menu */
	{40,"Cartesian",FUNCTION,set_cartesian},
	{41,"Polar",FUNCTION,set_polar},
	{42,"Grad",FUNCTION,set_grad},
	{43,"Back",BACK,NULL},
	{44,NULL,40,NULL},
	
	/* Display menu */
	{50,"Day",FUNCTION,set_day},
	{51,"Night",FUNCTION,set_night},
	{52,"Back",BACK,NULL},
	{53,NULL,50,NULL},
	
	/*end */
	{-1,NULL,-1,NULL}
};

/* set up timer interrupts etc */
/* Timer 2 is our input poller counter */
/* timer 3 is click length counter */
/* timer 2 delay: 2ms  = */
#define CLICKS_PER_MS (FCY_PER_MS/256)
#define T2_DELAY (2*CLICKS_PER_MS)

void interface_init() {
    OpenTimer2(T2_ON | T2_IDLE_CON | T2_PS_1_256 | T2_32BIT_MODE_OFF,T2_DELAY);
    OpenTimer3(T3_ON | T3_IDLE_CON | T3_PS_1_256 ,1000*CLICKS_PER_MS);
    // enable CN23 interrupt
    ConfigIntTimer2(T2_INT_ON | T2_INT_PRIOR_3);
    WriteTimer2(0);
    WriteTimer3(0);
}

/* change notification interrupt */
void __attribute__ ((interrupt,no_auto_psv,shadow)) _T2Interrupt() {
    static uint16_t state = 0x0001;
    T2_Clear_Intr_Status_Bit;
    state = ((state << 1) | PORT_BUTTON) & 0x0fff;
    if (state == (INT0_ACTIVE_HIGH?0x07ff:0x0800)) {
        /* we have just transitioned to a '1' and held it for 11 T2 cycles*/
        if (IFS0bits.T3IF || (ReadTimer3()>(400*CLICKS_PER_MS))) {
            /* it's been more than a quarter second since the last press started */
            last_click = SINGLE_CLICK;
        } else {
            last_click = DOUBLE_CLICK;
        }
        T3_Clear_Intr_Status_Bit;
        WriteTimer3(0);
   }
    if (state == (INT0_ACTIVE_HIGH?0x0800:0x07ff)) {
        /* we have justtransitiioned to a '0' and held it for 11 T2 cycles */
         if (IFS0bits.T3IF) {
            last_click = LONG_CLICK;
        }        
    }
    WriteTimer2(0);
}

void swipe_text(uint8_t index, bool left) {
    uint8_t my_buffer[4*128];
    int page;
    memset(my_buffer,0,128*4);
    for (page=0; page < 4; page++) {
        //memset(&my_buffer[page*128],0xaa,128);
        render_text_to_page(&(my_buffer[page*128]),page,0,menu_items[index].text,&large_font);
    }
    display_swipe_pages(day?0:2,my_buffer,4,left);
}

void scroll_text(uint8_t index, bool up) {
    if (!day) {
        if (up) {
            display_clear_page(6);
            display_clear_page(7);
        } else {
            display_clear_page(0);
            display_clear_page(1);
        }
    }
    display_scroll_text(day?0:2,0,menu_items[index].text,&large_font,up);
}

/* get the offset of the menu_item with the specified index */
/* return -1 if index not found */
int16_t get_menu_item_offset(int16_t index) {
	int16_t i;
	for (i = 0; menu_items[i].index != -1; ++i) {
		if (menu_items[i].index==index) return i;
	}
	return -1;
}

/* return index of previous menu item, rotate onto last menu item if currently on first item */
int16_t get_previous_menu_item(int16_t index) {
	if (menu_items[index-1].text) return index-1;
	while (menu_items[index+1].text) index++;
	return index;
}

/* return index of next menu item, rotating onto first menu item if currently on last item */
int16_t get_next_menu_item(int16_t index) {
	if (menu_items[index+1].text) {
		return index+1;
	} else {
		return get_menu_item_offset(menu_items[index+1].next_menu);
	}
}

enum ACTION get_action() {
	struct COOKED_SENSORS sensors;
	enum ACTION temp;
	
	sensors_read_cooked(&sensors,false);
	/* look for "flip" movements */
	//debug("f%.2g",sensors.gyro[1]);
	if (sensors.gyro[1]>80.0) {
		return display_inverted? FLIP_DOWN : FLIP_UP;
	}
	if (sensors.gyro[1]<-80.0) {
		return display_inverted? FLIP_UP : FLIP_DOWN;
	}
	if (sensors.gyro[0]>80.0) {
		return display_inverted? FLIP_LEFT : FLIP_RIGHT;
	}
	if (sensors.gyro[0]<-80.0) {
		return display_inverted? FLIP_RIGHT : FLIP_LEFT;
	}
	/* check to see if display needs flipping */
	/* use 0.5g - gives a hysteresis of about +/- 30 degrees */
	if ((sensors.accel[0]<-0.5) && display_inverted) display_flip(false);
	if ((sensors.accel[0]>0.5) && !display_inverted) display_flip(true);
	/* search for a click */
	if (last_click != NONE) {
		/* momentarily disable interrupts */
		__builtin_disi(0x0100);
		temp = last_click;
		last_click = NONE;
		__builtin_disi(0x0000);
		return temp;
	}
	//nothing else found - so return NONE
	return NONE;
}

uint8_t bcdtobyte(uint8_t x) {
	return (x & 0xf) + (10* (x>>4));
}

unsigned char reverse(unsigned char b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

void show_status(){
	char header[17];
	char footer[17] = "                "; //16 spaces
	#define FOOTER_LENGTH 16
	int x;
	rtccTime time;
	rtccDate date;
	/* batt icon 50% charge: 0x1f,0x20,0x2f*9,0x20*9,0x20,0x1f,0x04,0x03 */
	/* reverse bit order for second line */
	char bat_status[24];
	uint8_t charge;
	double bat_charge;
	bat_charge = get_bat_charge();
	bat_charge -= 3.6; //our minimum acceptable voltage
	bat_charge /= 0.6; //3.6+0.6 = 4.2V - ideal max voltage
	bat_charge *= 19; //there are 19 possible status levels
	bat_charge = bat_charge<0?0:bat_charge; //clip max and min values
	bat_charge = bat_charge>19?19:bat_charge;
	charge =bat_charge;
	if (!day) {
	    bat_status[0] = 0xf8;
	    bat_status[1] = 0x04;
	    memset(&bat_status[2],0xf4,charge);
	    memset(&bat_status[2+charge],0x04,19-charge);
	    bat_status[21] = 0xf8;
	    bat_status[22] = 0x20;
	    bat_status[23] = 0xC0;
	    RtccReadDate(&date);
	    RtccReadTime(&time);
	    switch (config.length_units) {
	        case METRIC:
	            memcpy(footer,"Metric",6);
	            break;
	        case IMPERIAL:
	            memcpy(footer,"Imp.",4);
	            break;
	    }
	    switch (config.display_style) {
	        case POLAR:
	            memcpy(&footer[FOOTER_LENGTH-5],"Polar",5);
	            break;
	        case GRAD:
	            memcpy(&footer[FOOTER_LENGTH-5],"Grad.",5);
	            break;
	        case CARTESIAN:
	            memcpy(&footer[FOOTER_LENGTH-5],"Cart.",5);
	            break;
	    }
	    snprintf(header,17,"%02d:%02d        ",
		    bcdtobyte(time.f.hour),	bcdtobyte(time.f.min));
	    display_write_text(0,0,header,&small_font,false);
	    display_write_text(6,0,footer,&small_font,false);
	    render_data_to_page(0,104,bat_status,24);
	    for (x=0; x< 24; ++x) {
	        bat_status[x] = reverse(bat_status[x]);
	    }
	    render_data_to_page(1,104,bat_status,24);
    }
}

bool show_menu(int16_t index, bool first_time) {
    enum ACTION action;
    bool result;
    if (first_time) {
        scroll_text(index,true);
    } else {
        swipe_text(index,true);
    }
    while(true) {
        delay_ms(50);
	show_status();
        action = get_action();
        switch(action){
            case FLIP_DOWN:
                index = get_previous_menu_item(index);
                scroll_text(index,false);
                break;
            case FLIP_UP:
                index = get_next_menu_item(index);
                scroll_text(index,true);
                break;
            //case FLIP_RIGHT:
            case SINGLE_CLICK:
		beep(3600,20);   
		switch (menu_items[index].next_menu) {
			case FUNCTION:
				menu_items[index].action();
				result = true;
				break;
			case BACK:
				if (!first_time) return false;
				break;
			default:
				result = show_menu(get_menu_item_offset(menu_items[index].next_menu),false);
				break;
		}
                if (result) {
                    if (first_time) {
			/* we have got back to the root screen */
                        index = FIRST_MENU_ITEM;
                        display_clear_screen();
                        scroll_text(index,true);
                    } else {
                        return true;   
                    }
                } else {
		    /* sub-menu: back selected */	
                    swipe_text(index,false);
                }
                break;
//             case FLIP_LEFT:
//                 if (!first_time) return false;
//                 break;
			case DOUBLE_CLICK:
				hibernate();
				break;
            }   
        if (action!=NONE) {
		show_status();
		beep(3600,20);   
		delay_ms(300);
	    }
    }
}
