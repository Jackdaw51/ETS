#include "display/display.h"
#include "stddef.h"

#define MAP_WIDTH 20 // Size of the game area in blocks
#define MAP_HEIGHT 16
#define MAP_SIZE (MAP_WIDTH * MAP_HEIGHT)

boolean check_crush(u8* snake_x, u8* snake_y, u8 snake_length, u8 new_head_x, u8 new_head_y) {
    for (u8 i=0; i < snake_length-1; i++) {
        if (snake_x[i] == new_head_x && snake_y[i] == new_head_y) {
            return true;
        }
    }
    return false;
}

int main(){
  display_init_lcd();
  joystick_t joystick;

  u8 snake_x[MAP_SIZE]; // Work as circular buffers
  u8 snake_y[MAP_SIZE];
  u8 snake_start = 0;
  u8 snake_length = 3; // Create starting snake
  for (u8 i = 0; i < snake_length; i++){
    snake_x[i] = MAP_WIDTH/4 - 1 + i; // The snake is built in reverse wrt the array
    snake_y[i] = MAP_HEIGHT/2;
  }

  u8 food_x = 3*(MAP_WIDTH/4);
  u8 food_y = MAP_HEIGHT/2;

  while(!window_should_close()){
    display_begin();

		// Input comes from here
		joystick = get_joystick();


		// Physics and state changes go here
        u8 current_head = snake_start + snake_length - 1;
        if (joystick == JS_UP &&
          snake_y[current_head % MAP_SIZE] > 0 &&
          !check_crush(&snake_x[snake_start], &snake_y[snake_start], snake_length, snake_x[current_head % MAP_SIZE], snake_y[current_head % MAP_SIZE] - 1)
        ) {
            
        } else if (joystick == JS_DOWN &&
          snake_y[current_head % MAP_SIZE] < MAP_HEIGHT - 1 &&
          !check_crush(&snake_x[snake_start], &snake_y[snake_start], snake_length, snake_x[current_head % MAP_SIZE], snake_y[current_head % MAP_SIZE] + 1)
        ) {
             
        } else if (joystick == JS_LEFT &&
          snake_x[current_head % MAP_SIZE] > 0 &&
          !check_crush(&snake_x[snake_start], &snake_y[snake_start], snake_length, snake_x[current_head % MAP_SIZE] - 1, snake_y[current_head % MAP_SIZE])
        ) {
            
        } else if (joystick == JS_RIGHT && snake_x[current_head % MAP_SIZE] < MAP_WIDTH - 1) {
            
        } else {
            // Crushed into a wall, game over
        }


		// Drawing goes after this line
		clear_screen();



    display_end();
  }

  display_close();
}

