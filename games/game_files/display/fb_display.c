#include "display.h"
#include "pixel_mask_lut.h"

#define FRAME_WIDTH 160
#define FRAME_HEIGHT 128
#define MAX_TEXTURES 1000


#define BIT_PER_PIXEL 2
#define PIXEL_PER_BYTE 4

#define BIT_PER_PALETTE 4
#define PALETTE_PER_BYTE 2


#define GET_FRAME_ELEMENT(x,y) ((frame_buffer[(FRAME_WIDTH/PIXEL_PER_BYTE*y) + x/PIXEL_PER_BYTE] >> ((3-x%PIXEL_PER_BYTE)*2)) & 3)
#define SET_FRAME_ELEMENT(x,y,val)\
    do{\
        frame_buffer[(FRAME_WIDTH/PIXEL_PER_BYTE*y) + x/PIXEL_PER_BYTE] &= ~(3 << ((3-x%PIXEL_PER_BYTE)*2));\
        frame_buffer[(FRAME_WIDTH/PIXEL_PER_BYTE*y) + x/PIXEL_PER_BYTE] |= (val << ((3-x%PIXEL_PER_BYTE)*2));\
    }while(0)
#define GET_FRAME_PTR(x,y) &frame_buffer[(FRAME_WIDTH/PIXEL_PER_BYTE*y) + x/PIXEL_PER_BYTE]


#define GET_PALETTE_ELEMENT(x,y) ((palette_buffer[(FRAME_WIDTH/PALETTE_PER_BYTE*y) + x/PALETTE_PER_BYTE] >> ((1-x%PALETTE_PER_BYTE)*4)) & 15)
#define SET_PALETTE_ELEMENT(x,y,p)\
    do {\
        palette_buffer[(FRAME_WIDTH/PALETTE_PER_BYTE*y) + x/PALETTE_PER_BYTE] &= ~(15 << ((1-x%PALETTE_PER_BYTE)*4));\
        palette_buffer[(FRAME_WIDTH/PALETTE_PER_BYTE*y) + x/PALETTE_PER_BYTE] |= ((p << (1-x%PALETTE_PER_BYTE)*4));\
    }while(0)
#define GET_PALETTE_PTR(x,y) &palette_buffer[(FRAME_WIDTH/PALETTE_PER_BYTE*y) + x/PALETTE_PER_BYTE]

#define GET_COLOR(x,y) (PaletteArray565[GET_PALETTE_ELEMENT(x,y)].colors[GET_FRAME_ELEMENT(x,y)])
#define CEIL_DIV(a,b) (((a) + (b) - 1) / (b))

typedef struct {
	Sprite s;
	u8 p_index;
} FbTexture;

// Frame info
static u8 frame_buffer[5120];
static u8 palette_buffer[10240];

static u8 dma_buffer[1024];

// Palette info
static TWOS_COLOURS screen_color = T_ONE;
static u8 base_palette_index = BW_INDEX;
static FbTexture texture_array[MAX_TEXTURES] = {};
static u8 texture_array_index = 0;

static u8 h_space_spacing = 6;
static u8 v_space_spacing = 8;

// how to set a pixel in the frame buffer
// access the pixel

u16 twos_to_16(TWOS_COLOURS color){
	return PaletteArray565[base_palette_index].colors[color];
};

void draw_pixel(i32 x, i32 y,TWOS_COLOURS color){
    SET_FRAME_ELEMENT(x,y,color);
    SET_PALETTE_ELEMENT(x,y,base_palette_index);
};

void display_init_lcd(void){

};
void display_init(i32 width, i32 height);
void display_close(void){};

// ------------------------------------------------------------------
// For raylib compatibility always put at the start and end of the drawing loop
void display_begin(void){};
void display_end(void){
    // send everything
    DMA_send_frame(frame_buffer,palette_buffer);
};
u8 window_should_close(void){
    return 0;
}


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
void set_mapping_array(u8* map){};

// It helps to #define palette indexes in the palettes.h file so you don't have to remember them
void set_palette(u8 palette_index){
    base_palette_index = palette_index;
};
void set_screen_color(TWOS_COLOURS color){
    screen_color = color;
};

// Should not use these functions
void clear_screen(void){
    int i;

    u32 *f_fptr = (u32*)frame_buffer;
    u32 *p_fptr = (u32*)palette_buffer;

    u32 extended_pixel = (screen_color | screen_color << 2 | screen_color << 4 | screen_color << 6 | screen_color << 8 | screen_color << 10
                        | screen_color << 12 | screen_color << 14 | screen_color << 16 | screen_color << 18 | screen_color << 20 | screen_color << 22
                        | screen_color << 24 | screen_color << 26 | screen_color << 28 | screen_color << 30);

    u32 extended_palette = (base_palette_index | base_palette_index << 4 | base_palette_index << 8 | base_palette_index << 12 | base_palette_index << 16
                            | base_palette_index << 20 | base_palette_index << 24 | base_palette_index << 28);


    // clear_screen() snippet
    // Each byte represents 4 pixel and we need to fill all bytes with the same value 
    for(i = 0; i < FRAME_WIDTH*FRAME_HEIGHT/(PIXEL_PER_BYTE*4); i++){
        f_fptr[i] = extended_pixel;
    }

    for(i = 0; i < FRAME_WIDTH*FRAME_HEIGHT/(PALETTE_PER_BYTE*4); i++){
        p_fptr[i] = extended_palette;
    }
};
void clear_screen_c(TWOS_COLOURS color){};


// This is the x and y of the top left corner
void draw_rectangle(i32 x, i32 y, i32 width, i32 height,TWOS_COLOURS color){
	int i;
    int j;
    int n_y;

    if(width >= 48){
        u32 extended_pixel = (color | color << 2 | color << 4 | color << 6 | color << 8 | color << 10
                            | color << 12 | color << 14 | color << 16 | color << 18 | color << 20 | color << 22
                            | color << 24 | color << 26 | color << 28 | color << 30);

        u32 leftovers = width%16;

        for(i = 0; i < height; i++){
            n_y = y+i;
            u32* r_fptr = (u32*)GET_FRAME_PTR(x,n_y);
            for(j = 0; j < width/16; j++){
                r_fptr[j] = extended_pixel;
            }

            for(j = 0; j<leftovers; j++){
                int n_x = x+width-leftovers+j;
                SET_FRAME_ELEMENT(n_x,n_y,color);
                SET_PALETTE_ELEMENT(n_x,n_y,base_palette_index);
            }
        }
   } else if(width >= 24){
        u16 extended_pixel = (color | color << 2 | color << 4 | color << 6 | color << 8 | color << 10 | color << 12 | color << 14);
        u16 leftovers = width%8;

        for(i = 0; i < height; i++){
            n_y = y+i;
            u16* r_fptr = (u16*)GET_FRAME_PTR(x,n_y);
            for(j = 0; j < width/8; j++){
                r_fptr[j] = extended_pixel;
            }

            for(j = 0; j<leftovers; j++){
                int n_x = x+width-leftovers+j;
                SET_FRAME_ELEMENT(n_x,n_y,color);
                SET_PALETTE_ELEMENT(n_x,n_y,base_palette_index);
            }
        }
   }
   else if(width >= 12){ // can add longer if necessary
        u8 extended_pixel = (color | color << 2 | color << 4 | color << 6);
        u8 leftovers = width%4;

        for(i = 0; i < height; i++){
            n_y = y+i;
            u8* r_fptr = (u8*)GET_FRAME_PTR(x,n_y);
            for(j = 0; j < width/4; j++){
                r_fptr[j] = extended_pixel;
            }

            for(j = 0; j<leftovers; j++){
                int n_x = x+width-leftovers+j;
                SET_FRAME_ELEMENT(n_x,n_y,color);
                SET_PALETTE_ELEMENT(n_x,n_y,base_palette_index);
            }
        }
    } else {
        for(i = 0; i < height; i++){
            int n_y = y+i;
            for(j = 0; j < width; j++){
                int n_x = x+j;
                SET_FRAME_ELEMENT(n_x,n_y,color);
                SET_PALETTE_ELEMENT(n_x,n_y,base_palette_index);
            };
        };
    }
};

void draw_rectangle_outline(i32 x, i32 y, i32 width, i32 height,u8 thickness,TWOS_COLOURS color){
    // draws the top lines
	int i;
    int j;
	for(i = 0; i < thickness; i++){
        for(j = 0; j < width; j++){
            SET_FRAME_ELEMENT(x+j,y+i,color);
            SET_PALETTE_ELEMENT(x+j,y+i,base_palette_index);
        }
	};

	// draws the bottom lines
	for(i = 0; i < thickness; i++){
        for(j = 0; j < width; j++){
            SET_FRAME_ELEMENT(x+j,y+i,color);
            SET_PALETTE_ELEMENT(x+j,y+i,base_palette_index);
        }
    }

	// draw central lines
	for(i = 0; i < height-2*thickness;i++){
		for(j = 0; j < thickness; j++){
            SET_FRAME_ELEMENT(x+j,y+i,color);
            SET_PALETTE_ELEMENT(x+j,y+i,base_palette_index);
		}
		for(j = 0; j < thickness; j++){
			SET_FRAME_ELEMENT(x+j,y+i,color);
            SET_PALETTE_ELEMENT(x+j,y+i,base_palette_index);
		}
	}
};

// Drawing with a different palette to the base

void draw_rectangle_p(i32 x, i32 y, i32 width, i32 height,TWOS_COLOURS color, u8 p){
    u8 old_base_palette_index = base_palette_index;
    set_palette(p);

    draw_rectangle(x,y,width,height,color);

    base_palette_index = old_base_palette_index;
};
void draw_rectangle_outline_p(i32 x, i32 y, i32 width, i32 height,u8 thickness,TWOS_COLOURS color, u8 p){
    u8 old_base_palette_index = base_palette_index;
    set_palette(p);

    draw_rectangle_outline(x,y,width,height,thickness,color);

    base_palette_index = old_base_palette_index;
};

// buf handles length must be as long as string
void set_space_len(u8 len){};

void load_text(const char *text, TextBuilder* builder){
	const char *c = text;
	u8 counter = 0;
	while(*c != '\0'){
		u8 index = (u8)*c;

		// map to 0-31
		if(index == ' '){
			builder->handles[counter++] = (BuilderElement){ .type = SPACE };
			c++;
			continue;
		} else if(index >= 'A' && index <= 'Z'){
			index = index - (u8)'A';
		} else if (index >= 'a' && index <= 'z'){
			index = index - (u8)'a' + 26; // lowercase letters come after cap
		} else {
			return;
		}

		Sprite* s = text_lookup_table[index];
		builder->handles[counter++] = (BuilderElement){.type = LETTER ,.handle = load_texture_from_sprite(s->height,s->width, s->data)};

		c++;
	}
};

void load_text_p(const char *text,TextBuilder* builder,u8 p){
    u8 old_base_palette_index = base_palette_index;
    set_palette(p);

    load_text(text,builder);

    base_palette_index = old_base_palette_index;
};
void draw_text_h(i32 x, i32 y,i32 extra_spacing, TextBuilder* builder){
	u8 x_new = x;
	int i;
	for(i = 0; i < builder->len;i++){
		BuilderElement el = builder->handles[i];
		if(el.type == SPACE){
			x_new += h_space_spacing;
		} else if(el.type == LETTER){
			draw_texture(x_new,y,el.handle);
			x_new += 8+extra_spacing;
		}
	};
}

void draw_text_v(i32 x, i32 y,i32 extra_spacing, TextBuilder* builder){
   	// loop over handles and draw them with the spacing
	u8 y_new = y;
	int i;
	for(i = 0; i < builder->len;i++){
		BuilderElement el = builder->handles[i];
		if(el.type == SPACE){
			y_new += v_space_spacing;
		} else if(el.type == LETTER){
			draw_texture(x,y_new,el.handle);
			y_new += 8+extra_spacing;
		}
	};
};

// ------------------------------------------------------------------
// Texture drawing functions
// Note that textures will not change after being loaded,
// so if you want multiple colored versions you must load multiple


TextureHandle load_texture_from_sprite(u8 height, u8 width, const u8* sprite){
    texture_array[texture_array_index] = (FbTexture){ .p_index = base_palette_index,.s = (Sprite){ .width = width, .height = height, .data = sprite}};
	return texture_array_index++;
};

TextureHandle load_texture_from_sprite_p(u8 height, u8 width, const u8* sprite, u8 p){
    texture_array[texture_array_index] = (FbTexture){ .p_index = p, .s = (Sprite){ .width = width, .height = height, .data = sprite}};
	return texture_array_index++;
};
void draw_texture(u8 x, u8 y, TextureHandle texture_index){
    FbTexture t = texture_array[texture_index];
	Sprite *s = &t.s;

	u8 old_base_palette_index = base_palette_index;
	u8 fat_palette_index = (old_base_palette_index << 6 | old_base_palette_index << 4 | old_base_palette_index << 2 | old_base_palette_index);
    set_palette(t.p_index);

	u8 padding = s->width % 4;
	u8 cols = CEIL_DIV(s->width,4);
	int i;

	for(i = 0; i < s->height; i++){
		int j;
        int n_y = y+i;
		for(j = 0; j < cols; j++){
			u8 batch = s->data[i*cols+j];
			int n_x = x+j*4;
			//u8* frame_ptr = (u8*)GET_FRAME_PTR(n_x, n_y);
            //u8* palette_ptr = (u8*)GET_PALETTE_PTR(n_x, n_y);

			u8 mask = byte_to_mask_lut[batch];

			//batch &= mask;
			//fat_palette_index &= mask;


			//*frame_ptr &= ~mask;
			//*frame_ptr |= batch;

			//*palette_ptr
			// take this batch and get the mask -> update pointer

           u8 p1 = ((3 << 6) & batch) >> 6;
           u8 p2 = ((3 << 4) & batch) >> 4;
           u8 p3 = ((3 << 2) & batch) >> 2;
           u8 p4 = (3 & batch);

           int n_x1 = x+j*4;
           int n_x2 = x+j*4+1;
           int n_x3 = x+j*4+2;
           int n_x4 = x+j*4+3;

           u8* frame_ptr = GET_FRAME_PTR(n_x1, n_y);

			if(p1 != T_TRANSPARENT){
               SET_FRAME_ELEMENT(n_x1,n_y,p1);
               SET_PALETTE_ELEMENT(n_x1,n_y,base_palette_index);
           };

           if(p2 != T_TRANSPARENT){
               SET_FRAME_ELEMENT(n_x2,n_y,p2);
               SET_PALETTE_ELEMENT(n_x2,n_y,base_palette_index);
           };

           if(p3 != T_TRANSPARENT) {
               SET_FRAME_ELEMENT(n_x3,n_y,p3);
               SET_PALETTE_ELEMENT(n_x3,n_y,base_palette_index);
           };

           if(p4 != T_TRANSPARENT){
               SET_FRAME_ELEMENT(n_x4,n_y,p4);
               SET_PALETTE_ELEMENT(n_x4,n_y,base_palette_index);
           };
		}

		// if(padding){
		//     u8 k;
        //     u8 batch = s->data[i*cols+(cols-1)];
		// 	for(k = 0; k < padding; k++){
        //         u8 p = ((3 << ((3-k)*2)) & batch) >> ((3-k)*2);
        //         int n_x = x+cols*4+k;
        
        //         if(p != T_TRANSPARENT){
        //             SET_FRAME_ELEMENT(n_x,n_y,p);
        //             SET_PALETTE_ELEMENT(n_x,n_y,base_palette_index);
        //         }
		// 	}
		// }
	}

    base_palette_index = old_base_palette_index;
};

// ------------------------------------------------------------------
// function to get the value of the proximity sensor

static f32 proximity = 0;

f32 get_proximity(void){
    proximity += 50;
    if(proximity > 1023.0f){
        proximity = 0.0f;
    }

    return proximity;
    //return trigger_hcsr04();
};

joystick_t get_joystick(void){
	return read_joystick();
};


void set_render_update_rate(u16 rate){};
void set_physics_update_rate(u16 rate){};

u16 get_physics_ticks(void){};
u16 get_render_ticks(void){};

void decrement_physics_ticks(void){};
void decrement_render_ticks(void){};
