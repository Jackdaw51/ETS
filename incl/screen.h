#pragma once
/*#include "../games/game_files/display/types.h"

void set_address_window(u8 top_left_x, u8 top_left_y,u8 bottom_right_x,u8 bottom_right_y);
void write_data(u8 data);


*/
#include <stdint.h>
void screen_init();
void LCD_FillColor(uint16_t);
void LCD_Init(void);
