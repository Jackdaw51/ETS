#pragma once
#include "display/types.h"

typedef enum {
	PLAYING,
	LOST,
	WIN,
} State;

typedef enum {
  Q_UP,
  Q_DOWN,
  Q_LEFT,
  Q_RIGHT
} Quad_Direction;

void start_snake(void);
boolean check_crush(u8* snake_x, u8* snake_y, u8 snake_length, u8 new_head_x, u8 new_head_y);
boolean has_eaten(u8 head_x, u8 head_y, u8 food_x, u8 food_y);
boolean new_food(u8* food_x, u8* food_y, u8* snake_x, u8* snake_y, u8 snake_length);
void draw_snake(u8* snake_x, u8* snake_y, u8 snake_tail_pos, u8 snake_length, TextureHandle* snake_texture);
void draw_food(u8 food_x, u8 food_y, TextureHandle food_texture);
