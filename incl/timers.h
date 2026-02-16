#pragma once
#include <msp432p401r.h>
#define CLOCK_SPEED 24000000
void init_A0(void);
void init_A1(void);
void init_A2(void);
void init_A3(void);
void sleep_ms(uint32_t ms);
extern volatile uint32_t game_tick;
