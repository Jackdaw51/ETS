#include "menu.h"
#include "stdio.h"
#include "display/display.h"
#include "stddef.h"

#define NUMBER_OF_GAMES 4

u8 menu(){
	char* example_game_names[NUMBER_OF_GAMES] = {"COD","LEGEND OF ZELDA", "ROWAN", "SNAKE"};
	TextBuilder game_title_builders[NUMBER_OF_GAMES];

	u8 chosen = 0;

	while(!chosen){
		u8 game_choice_index = 0;
		u8 game_choice_index_old = 0;

		game_title_builders[0] = (TextBuilder){ .handles = (BuilderElement[3]){}, .len = 3};
		game_title_builders[1] = (TextBuilder){ .handles = (BuilderElement[15]){}, .len = 15};
		game_title_builders[2] = (TextBuilder){ .handles = (BuilderElement[5]){}, .len = 5};
		game_title_builders[3] = (TextBuilder){ .handles = (BuilderElement[5]){}, .len = 5};

		TextBuilder menu_text_builder = (TextBuilder){ .handles = (BuilderElement[4]){}, .len = 4};

		for(int i = 0; i < NUMBER_OF_GAMES; i++){
			load_text_p(example_game_names[i],&game_title_builders[i],RETRO_RBY_INDEX);
		}
		load_text("Menu",&menu_text_builder);
		set_screen_color(T_TWO);

		while(!window_should_close() && !chosen){
			get_proximity();
			display_begin();
			// Input comes from here
			// input can be up down left or right
			
			joystick_t action = get_joystick();

			if(action == JS_BUTTON){
				switch(game_choice_index){
					case 0:
						break;
					case 1:
						break;
					case 2:
						// Start Rowan
						chosen = 2;
						break;
					case 3:
						// Start snake
						chosen = 3;
						break;
				};
				printf("You chose: %s\n",example_game_names[game_choice_index]);
				fflush(stdout);
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
			clear_screen();

			draw_text_h_center(80,20,0,&menu_text_builder);
			draw_text_h_center(80,50,0,&game_title_builders[game_choice_index]);

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
