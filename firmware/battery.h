#ifndef BATTERY_H
#define BATTERY_H
enum BAT_STATUS {
	DISCHARGING,
	CHARGING,
	CHARGED
};

enum BAT_STATUS get_bat_status();

double get_bat_charge();
#endif