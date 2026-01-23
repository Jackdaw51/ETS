#pragma once
/*#include "../games/game_files/display/types.h"

void set_address_window(u8 top_left_x, u8 top_left_y,u8 bottom_right_x,u8 bottom_right_y);
void write_data(u8 data);


*/
#include <stdint.h>
void screen_init();
void LCD_FillColor(uint16_t);
void LCD_Init(void);

void set_address_window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void write_data_fast(uint8_t highByte, uint8_t lowByte);
void pin_init();
void pin_des();

