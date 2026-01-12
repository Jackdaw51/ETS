#pragma once 
#include <raylib.h>
#include <stdint.h>
#include "types.h"
#include "../sprites/palettes.h"

// ------------------------------------------------------------------
// color enum 

typedef enum TWOS_COLOURS {
	T_ONE = 0,
	T_TWO = 1,
	T_THREE = 2,
	T_TRANSPARENT = 3,
} TWOS_COLOURS;

// ------------------------------------------------------------------
// Opening and Closing the window
// lcd version simulates the screen size

void display_init_lcd();
void display_init(i32 width, i32 height);
void display_close(void);

// ------------------------------------------------------------------
// For raylib compatibility always put at the start and end of the drawing loop
void display_begin(void);
void display_end(void);
u8 window_should_close(void);

// ------------------------------------------------------------------
// primative drawing functions
void clear_screen(void);
void draw_pixel(i32 x, i32 y,TWOS_COLOURS color);
void draw_line(i32 start_x, i32 start_y,i32 end_x, i32 end_y,TWOS_COLOURS color); 

// This is the x and y of the top left corner
void draw_rectangle(i32 x, i32 y, i32 width, i32 height,TWOS_COLOURS color);
void draw_rectangle_outline(i32 x, i32 y, i32 width, i32 height,f32 thickness,TWOS_COLOURS color);

// ------------------------------------------------------------------
// Texture drawing functions

void set_palette(u8 palette_index);
TextureHandle load_texture_from_sprite(u8 height, u8 width, const u8* sprite);
void draw_texture(u8 x, u8 y, TextureHandle texture_index);

// ------------------------------------------------------------------
// function to get the value of the proximity sensor  
f32 get_proximity();


