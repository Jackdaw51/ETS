#include "display.h"
#include "types.h"
#include <stdio.h>
#include "raylib.h"
#include <math.h>

#define RAYGUI_IMPLEMENTATION

#define SLIDER_HEIGHT 20 
#define SLIDER_WIDTH 40 

#define UI_WIDTH SLIDER_WIDTH 

#define MAX_TEXTURES 100

static i32 window_width = 160;
static i32 window_height = 128;

static u8 frame_buffer[160][128];

Color twos_to_rl(TWOS_COLOURS color){
	switch(color){
	case T_BLACK:
		return BLACK;
	case T_DARK:
		return (Color){85,85,85,255};
	case T_TRANSPARENT:
		return (Color){0,0,0,0};
	case T_WHITE:
		return WHITE;
	};
}

u8 twos_to_u8(TWOS_COLOURS color){
	switch(color){
	case T_BLACK:
		return 0;
	case T_DARK:
		return 85;
	case T_TRANSPARENT:
		return 0;
	case T_WHITE:
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

void clear_screen(){
  ClearBackground(BLACK); 
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
void draw_rectangle_outline(i32 x, i32 y, i32 width, i32 height,f32 thickness,TWOS_COLOURS color){
	DrawRectangleLinesEx((Rectangle){x+UI_WIDTH,y,width,height},thickness,twos_to_rl(color));

};
void draw_rectangle(i32 x, i32 y, i32 width, i32 height,TWOS_COLOURS color){
	DrawRectangleRec((Rectangle){x+UI_WIDTH,y,width,height},twos_to_rl(color));
};

void draw_line(int startPosX,int startPosY,int endPosX,int endPosY,TWOS_COLOURS color){
	DrawLine(startPosX+UI_WIDTH,startPosY,endPosX+UI_WIDTH,endPosY,twos_to_rl(color));
}

static Texture2D texture_array[MAX_TEXTURES] = {};
static u8 current_index = 0;

TextureHandle load_texture_from_sprite(u8 height, u8 width, const u8* sprite){
	i32 pixel_count = height * width;
	Color remapped_image[pixel_count]; 

	u8 isPadded = height % 4;
	u8 rows = height;
	printf("Number of rows = %d,",rows);
	u8 cols = (u8)ceil(width/4);
	printf("Number of cols = %d\n",cols);

	i32 row_counter = 1;
	Color *remapped_image_ptr = remapped_image;

	for(i32 i = 0; i < rows*cols; i++){
		u8 batch = sprite[i];

		if(row_counter == rows && isPadded){
			u8 non_padding = height % 4;

			for(i32 j = 0; j < non_padding; j++){
				*remapped_image_ptr++ = twos_to_rl(((3 << ((3-j)*2)) & batch) >> ((3-j)*2));
			}

			row_counter = 1;
		} else {

			*remapped_image_ptr++ = twos_to_rl(((3 << 6) & batch) >> 6);
			*remapped_image_ptr++ = twos_to_rl(((3 << 4) & batch) >> 4);
			*remapped_image_ptr++ = twos_to_rl(((3 << 2) & batch) >> 2);
			*remapped_image_ptr++ = twos_to_rl(3 & batch);

			row_counter+= 1;
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









