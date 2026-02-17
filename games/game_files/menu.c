#include "menu.h"
#include "stdio.h"
#include "display/display.h"
#include "stddef.h"

#define NUMBER_OF_GAMES 4

#define STEPS_PER_WHILE 2
#define WHILE_PER_AN_CHANGE 8

u8 menu(){
	char* example_game_names[NUMBER_OF_GAMES] = {"Pong","Dino","Snake","Invaders"};
	TextBuilder game_title_builders[NUMBER_OF_GAMES];

	u8 chosen = 0;
	u8 close = 0;

	TextureHandle wdf1_texture_handle = load_texture_from_sprite_p(wdf1_sprite.height,wdf1_sprite.width,wdf1_sprite.data,OLIVE_GREEN_INDEX);
  	TextureHandle wdf2_texture_handle = load_texture_from_sprite_p(wdf2_sprite.height,wdf2_sprite.width,wdf2_sprite.data,OLIVE_GREEN_INDEX);
  	TextureHandle wuf1_texture_handle = load_texture_from_sprite_p(wuf1_sprite.height,wuf1_sprite.width,wuf1_sprite.data,OLIVE_GREEN_INDEX);
  	TextureHandle wuf2_texture_handle = load_texture_from_sprite_p(wuf2_sprite.height,wuf2_sprite.width,wuf2_sprite.data,OLIVE_GREEN_INDEX);

  // Using a specified palette

	TextureHandle wdf1_texture_handle2 = load_texture_from_sprite_p(wdf1_sprite.height,wdf1_sprite.width,wdf1_sprite.data,RETRO_RBY_INDEX);
	TextureHandle wdf2_texture_handle2 = load_texture_from_sprite_p(wdf2_sprite.height,wdf2_sprite.width,wdf2_sprite.data,RETRO_RBY_INDEX);
	TextureHandle wuf1_texture_handle2 = load_texture_from_sprite_p(wuf1_sprite.height,wuf1_sprite.width,wuf1_sprite.data,RETRO_RBY_INDEX);
	TextureHandle wuf2_texture_handle2 = load_texture_from_sprite_p(wuf2_sprite.height,wuf2_sprite.width,wuf2_sprite.data,RETRO_RBY_INDEX);

	TextureHandle animation_pack[2][2] = {{wdf1_texture_handle,wdf2_texture_handle},{wuf1_texture_handle,wuf2_texture_handle}};
    TextureHandle animation_pack2[2][2] = {{wdf1_texture_handle2,wdf2_texture_handle2},{wuf1_texture_handle2,wuf2_texture_handle2}};

	u8 x1 = 0;
	u8 x2 = 160-wdf1_sprite.width;

	u8 y1 = 0;
	u8 y2 = 120-wdf1_sprite.height;

	u8 direction1 = 0;
	u8 direction2 = 1;

	u8 an1 = 0;
	u8 an2 = 1;

	u8 c = 0;

	while(!close){
		u8 game_choice_index = 0;
		u8 game_choice_index_old = 0;

		game_title_builders[0] = (TextBuilder){ .handles = (BuilderElement[4]){}, .len = 4};
		game_title_builders[1] = (TextBuilder){ .handles = (BuilderElement[4]){}, .len = 4};
		game_title_builders[2] = (TextBuilder){ .handles = (BuilderElement[5]){}, .len = 5};
		game_title_builders[3] = (TextBuilder){ .handles = (BuilderElement[8]){}, .len = 8};


		TextBuilder menu_text_builder = (TextBuilder){ .handles = (BuilderElement[4]){}, .len = 4};
		int i;
		for(i = 0; i < NUMBER_OF_GAMES; i++){
			load_text_p(example_game_names[i],&game_title_builders[i],BW_INDEX);
		}
		load_text_p("Menu",&menu_text_builder,RETRO_RBY_INDEX);
		set_screen_color(T_TWO);

		joystick_t old_action = JS_NONE;

		while(!window_should_close() && !close){
			get_proximity();
			display_begin();
			// Input comes from here
			// input can be up down left or right
			
			joystick_t action = get_joystick();
			if(action != old_action){
				if(action == JS_BUTTON){
					chosen = game_choice_index;
					close = !close;
				} else if(action == JS_UP){
					if(game_choice_index == 0){
						game_choice_index = NUMBER_OF_GAMES-1;
					} else {
						game_choice_index--;
					}
				} else if(action == JS_DOWN){
					if(game_choice_index == NUMBER_OF_GAMES-1){
						game_choice_index = 0;
					} else {
						game_choice_index++;
					}
				}
			}

			c++;
			if(c%WHILE_PER_AN_CHANGE == 0){
				an1 = !an1;
				an2 = !an2;
			}

			if(y1 + wdf1_sprite.height > 120 && direction1 == 0){
				direction1 = 1;
			}

			if(y2 + wdf1_sprite.height > 120 && direction2 == 0){
				direction2 = 1;
			}

			if(y1 - STEPS_PER_WHILE < 0 && direction1 == 1){
				direction1 = 0;
			}
			
			if(y2 - STEPS_PER_WHILE < 0 && direction2 == 1){
				direction2 = 0;
			}

			y1 += STEPS_PER_WHILE*(direction1 == 0 ? 1 : -1);
			y2 += STEPS_PER_WHILE*(direction2 == 0 ? 1 : -1);


			
			old_action = action;
			clear_screen();

			draw_text_h_center(60,30,1,&menu_text_builder);
			draw_text_h_center(60,65,1,&game_title_builders[game_choice_index]);
			//draw_texture(x1,y1,animation_pack[direction1][an1]);
			draw_texture(x2,y2,animation_pack2[direction2][an2]);

			display_end();
		}
	}
	return chosen;
}

void draw_text_h_center(u8 x,u8 y,u8 extra_spacing,TextBuilder* builder){
		u8 offset_x = 5 * builder->len;
		u8 offset_y = 10;
		draw_text_h(x-offset_x,y-offset_y,extra_spacing,builder);
};
