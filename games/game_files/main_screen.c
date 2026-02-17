#include "main_screen.h"

char* game_names[4] = {"Pong","Dino","Snake","Invaders"};

void game_loop(){
  display_init_lcd();
  u8 screen = 0;

  int max_length;
  int game_index;
  int score;

  while(1){
	switch(screen){
	  case 0: {
		screen = menu() + 1;
		break;
	  }
	  case 1:{
	  	score = pong_wall_game();
		game_index = 0;
		screen = 5;
		break;
	  }

	  case 2:{
	  	score = dino_runner_game();
		game_index = 1;
		screen = 5;
		break;
	  }
	  case 3: {
		score = start_snake(&max_length);
		game_index = 2;
		screen = 5;
		break;
	  }
	  case 4: {
		score = space_invaders_game();
		game_index = 3;
		screen = 5;
		break;
	  }
	  case 5: {
		int result = ask_upload();
		if(result == 0){
			screen = 6;
		} else {
			screen = 7;
		}
		break;
	  } 
	  case 6: {
		char name[13] = {0};
		sleep_ms(20);
		keyboard(name);
		char* game_name = game_names[game_index];

		char json_string[256] = {0};

		snprintf(
			json_string,
			256,
			"{\"game\": \"%s\",\"player\": \"%s\",\"score\": %d}",
			game_name,
			name,
			score
    	);

		transmitString(json_string);
		screen = 7;
		break;
	  }
	  case 7: {
		thanks();
		screen = 0;
		break;
	  }
	}
  }

  display_close();
}

