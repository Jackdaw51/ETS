#include "main_screen.h"

char* game_names[3] = {"Pong","Dino","Snake"};

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
		screen = 4;
		break;
	  }

	  case 2:{
	  	score = dino_runner_game();
		screen = 4;
		break;
	  }
	  case 3: {
		score = start_snake(&max_length);
		screen = 4;
		break;
	  }
	  case 4: {
		int result = ask_upload();
		if(result == 0){
			screen = 5;
		} else {
			screen = 6;
		}
		break;
	  } 
	  case 5: {
		char* name = keyboard();
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
		screen = 6;
		break;
	  }
	  case 6: {
		thanks();
		screen = 0;
		break;
	  }
	}
  }

  display_close();
}

