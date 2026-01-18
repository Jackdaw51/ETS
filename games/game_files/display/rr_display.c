#include "display.h"
#include "../../../incl/screen.h"
#include "types.h"

#define CEIL_DIV(a,b) (((a) + (b) - 1) / (b))

// idea of this implementation
//
// for every draw call, send a rectangle to the screen
// order to do these in
//
//
// draw rectangle
// load sprite
// draw sprite


// problem transparency
//
// solution 
//
// row run solution, we scan through sprites, for contiguous non transparent pixels,
// we set address for these and draw them, then we skip transparent pixels and repeat
// 
//

// these maybe do nothing
// display_init_lcd
// display_close

// window should close just return false
//
// should do some checks for how where people are trying to draw and if it is outside the range


// kinda seperate
// get proximity
// get joystick 

#define MAX_TEXTURES 100

static TWOS_COLOURS screen_color = T_ONE;

static Palette565 *palette565 = &PaletteArray565[0];
static u8 mapping_array[3] = {0,1,2};

static Sprite texture_array[MAX_TEXTURES] = {};
static u8 texture_array_index = 0;

u16 twos_to_16(TWOS_COLOURS color){
	return palette565->colors[color];
};

void write_color(u8 low,u8 high){
	write_data(low);
	write_data(high);
}

// ------------------------------------------------------------------
// Opening and Closing the window
// lcd version simulates the screen size

void display_init_lcd(void){
	// send commands to turn
	// turn screen on
};

void display_init(i32 width, i32 height){}; // unused

void display_close(void){
	// send commands to turn screen off
};

// ------------------------------------------------------------------
// For raylib compatibility always put at the start and end of the drawing loop
void display_begin(void);
void display_end(void){
	// clean up static memory and reset variables
	for(int i = 0; i < MAX_TEXTURES; i++){
		texture_array[i] = (Sprite){};
	}

	texture_array_index = 0;
};
u8 window_should_close(void);

// ------------------------------------------------------------------
// primative drawing functions
void set_screen_color(TWOS_COLOURS color);

void clear_screen(void){
	set_address_window(0,0,160,128);
	u16 p_color = twos_to_16(screen_color);
	u8 color_high = (u8)(p_color >> 8);
	u8 color_low = (u8)(p_color & 255);

	for(int i = 0; i < 160*128; i++){
		write_color(color_low,color_high);	
	}
};
void clear_screen_c(TWOS_COLOURS color);
void draw_pixel(i32 x, i32 y,TWOS_COLOURS color){
	u16 p_color = twos_to_16(color);
	u8 color_high = (u8)(p_color >> 8);
	u8 color_low = (u8)(p_color & 255);

	set_address_window(x,y,x,y);	
	write_color(color_low,color_high);
};

// This is the x and y of the top left corner
void draw_rectangle(i32 x, i32 y, i32 width, i32 height,TWOS_COLOURS color){
	// get color
	u16 p_color = twos_to_16(color);
	u8 color_high = (u8)(p_color >> 8);
	u8 color_low = (u8)(p_color & 255);

	// setup address window
	set_address_window(x,y,x+width,y+height);	

	for(int i = 0; i < width*height; i++){
		write_color(color_low,color_high);
	};
};

// This is the x and y of the top left corner
void draw_rectangle_outline(i32 x, i32 y, i32 width,i32 height,u8 thickness,TWOS_COLOURS color){
	// do sanity checks
	// get color
	u16 p_color = twos_to_16(color);
	u8 color_high = (u8)(p_color >> 8);
	u8 color_low = (u8)(p_color & 255);

	// setup top address window
	set_address_window(x,y,x+width,y+thickness);	

	// draws the top lines
	for(int i = 0; i < width*thickness; i++){
		write_color(color_low,color_high);
	};

	// setup top address window
	set_address_window(x,(y+height)-thickness,x+width,y+height);	

	// draws the bottom lines
	for(int i = 0; i < width*thickness; i++){
		write_color(color_low,color_high);
	}

	// draw central lines
	
	for(int i = 0; i < height-2*thickness;i++){
		set_address_window(x,y+thickness+i,x+thickness,y+thickness+i);	
		
		for(int j = 0; j < thickness; j++){
			write_color(color_low,color_high);
		}

		set_address_window(x+width-thickness,y+thickness+i,x+thickness,y+thickness+i);	

		for(int j = 0; j < thickness; j++){
			write_color(color_low,color_high);
		}
	}
	// setup top address window
	set_address_window(x,(y+height)-thickness,x+width,y+height);	
};

// Drawing with a different palette to the base

void draw_rectangle_p(i32 x, i32 y, i32 width, i32 height,TWOS_COLOURS color, u8 p){
	Palette565 *old_palette = palette565;
	set_palette(p);

	draw_rectangle(x,y,width,height,color);

	palette565 = old_palette;

};
void draw_rectangle_outline_p(i32 x, i32 y, i32 width, i32 height,u8 thickness,TWOS_COLOURS color, u8 p){
	Palette565 *old_palette = palette565;
	set_palette(p);

	draw_rectangle_outline(x,y,width,height,thickness,color);

	palette565 = old_palette;
};


// buf handles length must be as long as string
void set_space_len(u8 len);
void load_text(const char *text,TextBuilder* builder);
void load_text_p(const char *text,TextBuilder* builder,u8 p);
void draw_text_h(i32 x, i32 y,i32 extra_spacing, TextBuilder* builder);
void draw_text_v(i32 x, i32 y,i32 extra_spacing, TextBuilder* builder);

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
void set_palette(u8 palette_index){
	palette565 = &PaletteArray565[palette_index];
};

TextureHandle load_texture_from_sprite(u8 height, u8 width, const u8* sprite){
	texture_array[texture_array_index] = (Sprite){ .width = width, .height = height, .data = sprite};
	return texture_array_index++;
};

TextureHandle load_texture_from_sprite_p(u8 height, u8 width, const u8* sprite, u8 p){
	Palette565 *old_palette = palette565;
	set_palette(p);

	load_texture_from_sprite(height,width,sprite);

	palette565 = old_palette;
};

void hit_transparent(u8 current_x,u8 y,u8 i, u8 nt_counter,u8 *nt_indexes){
	set_address_window(current_x,y+i,current_x+nt_counter-1,y+i);	

	for(int k = 0; k < nt_counter-1; k++){
		u16 p_color = twos_to_16(nt_indexes[k]);
		u8 color_high = (u8)(p_color >> 8);
		u8 color_low = (u8)(p_color & 255);

		write_color(color_low,color_high);
	}
}

void draw_texture(u8 x, u8 y, TextureHandle texture_index){
	Sprite *s = &texture_array[texture_index];

	u8 padding = s->width % 4;
	u8 cols = CEIL_DIV(s->width,4);

	for(int i = 0; i < s->height; i++){
		
		u8 nt_counter = 0;
		u8 nt_indexes[160] = {};

		u8 current_x = x;

		u8 batch = 0;
		for(int j = 0; j < cols; j++){
			u8 batch = s->data[i*cols+j];

			if((nt_indexes[nt_counter++] = ((3 << 6) & batch) >> 6) == T_TRANSPARENT){
				if(nt_counter != 1){
					hit_transparent(current_x,y,i,nt_counter,nt_indexes);
				}; 
			current_x += nt_counter; // this is probably wrong
			nt_counter = 0;
			};

			if((nt_indexes[nt_counter++] = ((3 << 4) & batch) >> 4) == T_TRANSPARENT){
				if(nt_counter != 1){
					hit_transparent(current_x,y,i,nt_counter,nt_indexes);
				}; 
			current_x += nt_counter; // this is probably wrong
			nt_counter = 0;
			};

			if((nt_indexes[nt_counter++] = ((3 << 2) & batch) >> 2) == T_TRANSPARENT){
				if(nt_counter != 1){
					hit_transparent(current_x,y,i,nt_counter,nt_indexes);
				}; 
				current_x += nt_counter; // this is probably wrong
				nt_counter = 0;
			};

			if((nt_indexes[nt_counter++] = 3 & batch) == T_TRANSPARENT){
				if(nt_counter != 1){
					hit_transparent(current_x,y,i,nt_counter,nt_indexes);
					nt_counter = 0;
				}; 
				current_x += nt_counter; // this is probably wrong
				nt_counter = 0;
			};
		}

		if(padding){
			for(u8 j = 0; j < padding; j++){
				if((nt_indexes[nt_counter++] = ((3 << ((3-j)*2)) & batch) >> ((3-j)*2)) == T_TRANSPARENT){
					if(nt_counter != 1){
						hit_transparent(current_x,y,i,nt_counter,nt_indexes);
					}; 
					current_x += nt_counter; // this is probably wrong
					nt_counter = 0;
				};
			}
		} 
	}
};

// ------------------------------------------------------------------
// function to get the value of the proximity sensor  
f32 get_proximity(void);
joystick_t get_joystick(void){
	return read_joystick();
};

