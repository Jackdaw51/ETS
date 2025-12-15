#include <stdint.h>
#ifndef JOYSTICK_H
#define JOYSTICK_H

void read_joystick(uint16_t *x, uint16_t *y, uint8_t *button);
void js_init(void);

#endif