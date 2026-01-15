#pragma once 
#include "raylib.h"
#include <stdint.h>
#include "types.h"
#include "../sprites/palettes.h"

// When drawing 

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

void display_init_lcd(void);
void display_init(i32 width, i32 height);
void display_close(void);

// ------------------------------------------------------------------
// For raylib compatibility always put at the start and end of the drawing loop
void display_begin(void);
void display_end(void);
u8 window_should_close(void);

// ------------------------------------------------------------------
// primative drawing functions
void set_screen_color(TWOS_COLOURS color);

void clear_screen(void);
void clear_screen_c(TWOS_COLOURS color);
void draw_pixel(i32 x, i32 y,TWOS_COLOURS color);
void draw_line(i32 start_x, i32 start_y,i32 end_x, i32 end_y,TWOS_COLOURS color); 

// This is the x and y of the top left corner
void draw_rectangle(i32 x, i32 y, i32 width, i32 height,TWOS_COLOURS color);
void draw_rectangle_outline(i32 x, i32 y, i32 width, i32 height,f32 thickness,TWOS_COLOURS color);

// Drawing with a different palette to the base

void draw_rectangle_p(i32 x, i32 y, i32 width, i32 height,TWOS_COLOURS color, u8 p);
void draw_rectangle_outline_p(i32 x, i32 y, i32 width, i32 height,f32 thickness,TWOS_COLOURS color, u8 p);


// ------------------------------------------------------------------
// Texture drawing functions
// Note that textures will not change after being loaded,
// so if you want multiple colored versions you must load multiple

// Set mapping array is intended to be used when loading a sprite with a 
// different palette from the one it was drawn in

// It chooses which index of the original palette maps to which index of the new palette

// eg 
// Palette 1 BW {Black,DarkGray,White}
// Palette 2 RGB {R,G,B}

// If I want to map:
// DarkGray (1) to R (0)
// Black (0) to B (2)
// White (2) to G (1)
// I can do set_mapping_array(2,0,1) 
void set_mapping_array(u8* map);

// It helps to #define palette indexes in the palettes.h file so you don't have to remember them
void set_palette(u8 palette_index);
TextureHandle load_texture_from_sprite(u8 height, u8 width, const u8* sprite);
TextureHandle load_texture_from_sprite_p(u8 height, u8 width, const u8* sprite, u8 p);
void draw_texture(u8 x, u8 y, TextureHandle texture_index);

// ------------------------------------------------------------------
// function to get the value of the proximity sensor  
f32 get_proximity(void);


