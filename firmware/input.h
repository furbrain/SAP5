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

/* setup features related to button presses etc*/
void input_init();

/* reset the timeout feature*/
void timeout_reset();

/* get most recent input*/
enum INPUT get_input();

#endif	/* INPUT_H */

