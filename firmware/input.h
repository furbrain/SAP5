/* 
 * File:   input.h
 * Author: phil
 *
 * Created on 18 August 2019, 18:05
 */

#ifndef INPUT_H
#define	INPUT_H


enum INPUT {
    NONE = 0,
    FLIP_DOWN,
    FLIP_UP,
    FLIP_LEFT,
    FLIP_RIGHT,
    SINGLE_CLICK,
    LONG_CLICK,
    DOUBLE_CLICK,
    SHAKE
};

/* setup input detection*/
void input_init(void);

/* reset the timeout feature*/
void timeout_reset(void);

/* get most recent input*/
enum INPUT get_input(void);

/* get most recent input*/
enum INPUT get_clicks(void);

#endif	/* INPUT_H */

