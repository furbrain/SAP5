#ifndef BATTERY_H
#define BATTERY_H
#define BATTERY_MIN_VOLTAGE 3.5
#define BATTERY_MAX_VOLTAGE 4.0
enum BAT_STATUS {
	DISCHARGING,
	CHARGING,
	CHARGED
};

enum BAT_STATUS battery_get_status(void);
#ifndef BOOTLOADER
double battery_get_voltage(void);

/* return a number between 0 and 19 reflecting the current charge state of the battery *
 * this assumes a linear relationship between voltage and charge, which is not correct,
 * but a useful approximation */
int battery_get_units(void);
#endif
#endif
