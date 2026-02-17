#include "display.h"
#include "raylib.h"
#include "stdio.h"
#include <math.h>

#define RAYGUI_IMPLEMENTATION

#define SLIDER_HEIGHT 20 
#define SLIDER_WIDTH 40 

#define UI_WIDTH SLIDER_WIDTH 

#define MAX_TEXTURES 1000

static i32 window_width = 160;
static i32 window_height = 128;

static u8 frame_buffer[160][128];

static TWOS_COLOURS screen_color = T_ONE;

static Palette* palette = &PaletteArray[0];
static u8 mapping_array[3] = {0,1,2};

static u8 h_space_spacing = 6; 
static u8 v_space_spacing = 8;


void set_palette(u8 palette_index){
	palette = &PaletteArray[palette_index];
};

void set_h_space_len(u8 len){
	h_space_spacing = len;
};

void set_v_space_len(u8 len){
	v_space_spacing = len;
};

void set_mapping_array(u8* map){
  mapping_array[0] = map[0];
  mapping_array[1] = map[1];
  mapping_array[2] = map[2];
}

Color twos_to_rl(TWOS_COLOURS color){
	if(color==T_TRANSPARENT){
		return (Color){0,0,0,0};
	} 

  int base = mapping_array[color];
	return (Color){
    palette->colors[(base*4)],
    palette->colors[(base*4)+1],
    palette->colors[(base*4)+2],
    palette->colors[(base*4)+3]
  };
}

u8 twos_to_u8(TWOS_COLOURS color){
	switch(color){
	case T_ONE:
		return 0;
	case T_TWO:
		return 85;
	case T_TRANSPARENT:
		return 0;
	case T_THREE:
		return 255;
	};
}


void display_init_lcd(){
  InitWindow(160+UI_WIDTH,128,"Simulation Window");
  SetTargetFPS(60);
};

void display_init(i32 width,i32 height){
  InitWindow(width+UI_WIDTH,height,"Simulation Window");
	window_width = width + UI_WIDTH;
	window_height = height;
  SetTargetFPS(60);
};

void display_close(){
  CloseWindow();
};

void set_screen_color(TWOS_COLOURS color){
  screen_color = color;
}

void clear_screen(){
  ClearBackground(twos_to_rl(screen_color)); 
}

void clear_screen_c(TWOS_COLOURS color){
  ClearBackground(twos_to_rl(color)); 
};

u8 window_should_close(){
  return WindowShouldClose() ? 1 : 0;
}

void display_begin(){
  BeginDrawing();
};

void display_end(){
  EndDrawing();
};

void draw_pixel(i32 x, i32 y,TWOS_COLOURS color){
  DrawPixel(x+UI_WIDTH,y,twos_to_rl(color));
}

static f32 slider_value = 0.0f;
static bool sliding = false;

float slider_y_to_value(int y,float maxValue){
	i32 half_slider_height = SLIDER_HEIGHT/2;
	float ratio = (float)(y-half_slider_height)/(window_height-SLIDER_HEIGHT);
	return ratio * maxValue;
}

void VerticalSlider(Rectangle bounds,float *value,float maxValue){
	// range of y for the slider is [0+slider_height/2,window_height-slider_height/2]
	i32 half_slider_height = SLIDER_HEIGHT/2;
	float ratio = *value/maxValue;
	i32 temp_max = window_height-SLIDER_HEIGHT;

	i32 slider_y = (i32)round(ratio*temp_max)-(i32)SLIDER_HEIGHT/2;

	// check if the left mouse button is clicked
	// check if the mouse is over the slider,
	Rectangle slider_rec = (Rectangle){0,slider_y+half_slider_height,SLIDER_WIDTH,SLIDER_HEIGHT};
	
	if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
		Vector2 mouse = GetMousePosition();
		if(CheckCollisionPointRec(mouse,slider_rec) || sliding){
			if(mouse.y > window_height-half_slider_height){
				slider_value = slider_y_to_value(window_height-half_slider_height, maxValue);
			}
			else if(mouse.y <= 0+half_slider_height){
				slider_value = slider_y_to_value(0+half_slider_height, maxValue);
			}
			else{
				sliding = true;
				slider_y = mouse.y;
				slider_value = slider_y_to_value(mouse.y, maxValue);
			}
		}
	} else {
		sliding = false;
	}

	Color slider_color = sliding ? DARKBLUE : BLUE;

	DrawRectangleRec(bounds,WHITE);
	DrawRectangleRec(slider_rec,slider_color);
	DrawRectangleLinesEx(slider_rec,3.0f,GRAY);
};

f32 get_proximity(){
  VerticalSlider((Rectangle){0,0,SLIDER_WIDTH,window_height},&slider_value,1023.0f);
  return slider_value;
};
void draw_rectangle_outline(i32 x, i32 y, i32 width, i32 height,u8 thickness,TWOS_COLOURS color){
	DrawRectangleLinesEx((Rectangle){x+UI_WIDTH,y,width,height},(f32)thickness,twos_to_rl(color));

};

void draw_rectangle_outline_p(i32 x, i32 y, i32 width, i32 height,u8 thickness,TWOS_COLOURS color, u8 p){
  Palette *old_palette = palette;
  set_palette(p);
	DrawRectangleLinesEx((Rectangle){x+UI_WIDTH,y,width,height},(f32)thickness,twos_to_rl(color));

  palette = old_palette;
};

void draw_rectangle(i32 x, i32 y, i32 width, i32 height,TWOS_COLOURS color){
	DrawRectangleRec((Rectangle){x+UI_WIDTH,y,width,height},twos_to_rl(color));
};

void draw_rectangle_p(i32 x, i32 y, i32 width, i32 height,TWOS_COLOURS color, u8 p){
  Palette *old_palette = palette;
  set_palette(p);
	DrawRectangleRec((Rectangle){x+UI_WIDTH,y,width,height},twos_to_rl(color));

  palette = old_palette;
};

static Texture2D texture_array[MAX_TEXTURES] = {};
static u8 current_index = 0;

TextureHandle load_texture_from_sprite_p(u8 height, u8 width, const u8* sprite, u8 p){

  Palette *old_palette = palette;
  set_palette(p);

  i32 pixel_count = height * width;
	Color remapped_image[pixel_count]; 

	u8 isPadded = width % 4;
	u8 rows = height;
	u8 cols = (u8)ceil(width/4.0f);

	i32 col_counter = 1;
	Color *remapped_image_ptr = remapped_image;

	for(i32 i = 0; i < rows*cols; i++){
		u8 batch = sprite[i];

		if((col_counter == cols) && isPadded){
			u8 non_padding = width % 4;


			for(u8 j = 0; j < non_padding; j++){
			  *remapped_image_ptr++ = twos_to_rl(((3 << ((3-j)*2)) & batch) >> ((3-j)*2));
			}
			col_counter = 1;
		} else {
			*remapped_image_ptr++ = twos_to_rl(((3 << 6) & batch) >> 6);
			*remapped_image_ptr++ = twos_to_rl(((3 << 4) & batch) >> 4);
			*remapped_image_ptr++ = twos_to_rl(((3 << 2) & batch) >> 2);
			*remapped_image_ptr++ = twos_to_rl(3 & batch);
			col_counter+= 1;
		}
	}


  Image rl_image = (Image){.format=PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, .height=height, .width=width, .mipmaps=1, .data=remapped_image};
	Texture2D rl_texture = LoadTextureFromImage(rl_image);

	texture_array[current_index] = rl_texture;
	current_index++;

  palette = old_palette;
	return current_index-1;
}

TextureHandle load_texture_from_sprite(u8 height, u8 width, const u8* sprite){
	i32 pixel_count = height * width;
	Color remapped_image[pixel_count]; 

	u8 isPadded = width % 4;
	u8 rows = height;
	u8 cols = (u8)ceil(width/4.0f);

	i32 col_counter = 1;
	Color *remapped_image_ptr = remapped_image;

	for(i32 i = 0; i < rows*cols; i++){
		u8 batch = sprite[i];

		if((col_counter == cols) && isPadded){
			u8 non_padding = width % 4;


			for(u8 j = 0; j < non_padding; j++){
			  *remapped_image_ptr++ = twos_to_rl(((3 << ((3-j)*2)) & batch) >> ((3-j)*2));
			}
			col_counter = 1;
		} else {
			*remapped_image_ptr++ = twos_to_rl(((3 << 6) & batch) >> 6);
			*remapped_image_ptr++ = twos_to_rl(((3 << 4) & batch) >> 4);
			*remapped_image_ptr++ = twos_to_rl(((3 << 2) & batch) >> 2);
			*remapped_image_ptr++ = twos_to_rl(3 & batch);
			col_counter+= 1;
		}
	}


  Image rl_image = (Image){.format=PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, .height=height, .width=width, .mipmaps=1, .data=remapped_image};
	Texture2D rl_texture = LoadTextureFromImage(rl_image);

	texture_array[current_index] = rl_texture;
	current_index++;
	return current_index-1;
}

void draw_texture(u8 x, u8 y,TextureHandle texture_index){
	DrawTexture(texture_array[texture_index],x+UI_WIDTH,y, WHITE);	
};

void draw_text_h(i32 x, i32 y,i32 extra_spacing, TextBuilder* builder){
	u8 x_new = x;
	for(int i = 0; i < builder->len;i++){
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
	for(int i = 0; i < builder->len;i++){
		BuilderElement el = builder->handles[i];
		if(el.type == SPACE){
			y_new += v_space_spacing;
		} else if(el.type == LETTER){
			draw_texture(x,y_new,el.handle);
			y_new += 8+extra_spacing;
		}
	};
}

void load_text(const char *text, TextBuilder* builder){
	const char *c = text;
	u8 counter = 0;
	while(*c != '\0' && counter < builder->len){
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
			printf("Invalid text string: %s",text); 
			return;
		}

		Sprite* s = text_lookup_table[index];
		builder->handles[counter++] = (BuilderElement){.type = LETTER ,.handle = load_texture_from_sprite(s->height,s->width, s->data)};

		c++;
	}
};

void load_text_p(const char *text,TextBuilder* builder,u8 p){
	const char *c = text;
	u8 counter = 0;
	while(*c != '\0' && counter < builder->len){
		u8 index = (u8)*c;
		
		if(index == ' '){
			builder->handles[counter++] = (BuilderElement){ .type = SPACE };
			c++;
			continue;
		}
		// map to 0-31
		if(index >= 'A' && index <= 'Z'){
			index = index - (u8)'A';
		}

		if(index >= 'a' && index <= 'z'){
			index = index - (u8)'a' + 26; // lowercase letters come after cap
		}

		Sprite* s = text_lookup_table[index];
		builder->handles[counter++] = (BuilderElement){.type = LETTER ,.handle = load_texture_from_sprite_p(s->height,s->width, s->data,p)};

		c++;
	}
};

joystick_t get_joystick(){
	if(IsKeyPressed(KEY_ENTER)){
		return JS_BUTTON;
	}
	if(IsKeyPressed(KEY_W)){
		return JS_UP;
	}
	if(IsKeyPressed(KEY_A)){
		return JS_LEFT;
	}
	if(IsKeyPressed(KEY_S)){
		return JS_DOWN;
	}
	if(IsKeyPressed(KEY_D)){
		return JS_RIGHT;
	}

	return JS_NONE;
};










