#include <stdint.h>
#ifndef JOYSTICK_H
#define JOYSTICK_H

typedef enum {
    JS_UP = 0,
    JS_DOWN,
    JS_LEFT,
    JS_RIGHT,
    JS_BUTTON,
    JS_NONE
} joystick_t;

joystick_t read_joystick();
void js_init(void);

#endif