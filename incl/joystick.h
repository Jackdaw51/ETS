#include <stdint.h>
#ifndef JOYSTICK_H
#define JOYSTICK_H

typedef enum {
    JS_DOWN = 0,
    JS_UP,
    JS_RIGHT,
    JS_LEFT,
    JS_BUTTON,
    JS_NONE
} joystick_t;

joystick_t read_joystick();
void js_init(void);

#endif
