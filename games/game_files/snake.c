#include "display/display.h"
#include "stddef.h"

#define MAP_WIDTH 20 // Size of the game area in blocks
#define MAP_HEIGHT 16
#define MAP_SIZE (MAP_WIDTH * MAP_HEIGHT)

typedef enum {
	PLAYING,
	LOST,
	WIN,
} State;

boolean check_crush(u8* snake_x, u8* snake_y, u8 snake_length, u8 new_head_x, u8 new_head_y) {
    for (u8 i=0; i < snake_length-1; i++) {
        if (snake_x[i] == new_head_x && snake_y[i] == new_head_y) {
            return true;
        }
    }
    return false;
}

boolean has_eaten(u8 head_x, u8 head_y, u8 food_x, u8 food_y) {
  // True if they occupy the same position
  return (head_x == food_x && head_y == food_y);
}

boolean new_food(u8* food_x, u8* food_y, u8* snake_x, u8* snake_y, u8 snake_length) {
  // Generate a new position for the food that is not on the snake
  if (snake_length >= MAP_SIZE) {
    return false; // No space left, won the game
  }

  // Keep generating positions until a valid one is found
  boolean valid_position = false;
  while (!valid_position) {
    *food_x = rand() % MAP_WIDTH; // !!!PROBABLY SHOULD CHANGE THE RAND FUNCTION
    *food_y = rand() % MAP_HEIGHT;
    valid_position = true;
    for (u8 i = 0; i < snake_length; i++) {
      if (snake_x[i] == *food_x && snake_y[i] == *food_y) {
        valid_position = false;
        break;
      }
    }
  }

  return true;
}

int main(){
  display_init_lcd();
  joystick_t joystick;

  u8 snake_x[MAP_SIZE]; // Work as circular buffers
  u8 snake_y[MAP_SIZE];
  u8 snake_tail_pos = 0;
  u8 snake_length = 3; // Create starting snake
  for (u8 i = 0; i < snake_length; i++){
    snake_x[i] = MAP_WIDTH/4 - 1 + i; // The snake is built in reverse wrt the array
    snake_y[i] = MAP_HEIGHT/2;
  }

  u8 food_x = 3*(MAP_WIDTH/4);
  u8 food_y = MAP_HEIGHT/2;

  State game_state = PLAYING;

  while(!window_should_close()){
    display_begin();

		switch (game_state) {
      case PLAYING:
        // Input comes from here
        joystick = get_joystick();

        // Physics and state changes go here
        u8 current_head = (snake_tail_pos + snake_length - 1) % MAP_SIZE;
        u8 new_head = (current_head +1) % MAP_SIZE;

        // Based on joystick input, check if the snake can move
        // in that direction and apply the movement if possible
        if (joystick == JS_UP &&
          snake_y[current_head] > 0 &&
          !check_crush(&snake_x[snake_tail_pos], &snake_y[snake_tail_pos],
            snake_length, snake_x[current_head], snake_y[current_head] - 1)
        ) {
          // Moves up
          snake_x[new_head] = snake_x[current_head];
          snake_y[new_head] = snake_y[current_head] - 1;

        } else if (joystick == JS_DOWN &&
          snake_y[current_head % MAP_SIZE] < MAP_HEIGHT - 1 &&
          !check_crush(&snake_x[snake_tail_pos], &snake_y[snake_tail_pos],
            snake_length, snake_x[current_head % MAP_SIZE], snake_y[current_head % MAP_SIZE] + 1)
        ) {
          // Moves down
          snake_x[new_head] = snake_x[current_head];
          snake_y[new_head] = snake_y[current_head] + 1;

        } else if (joystick == JS_LEFT &&
          snake_x[current_head % MAP_SIZE] > 0 &&
          !check_crush(&snake_x[snake_tail_pos], &snake_y[snake_tail_pos], snake_length,
            snake_x[current_head % MAP_SIZE] - 1, snake_y[current_head % MAP_SIZE])
        ) {
          // Moves left
          snake_x[new_head] = snake_x[current_head] - 1;
          snake_y[new_head] = snake_y[current_head];
          
        } else if (joystick == JS_RIGHT &&
          snake_x[current_head % MAP_SIZE] < MAP_WIDTH - 1 &&
          !check_crush(&snake_x[snake_tail_pos], &snake_y[snake_tail_pos],
            snake_length, snake_x[current_head % MAP_SIZE] + 1, snake_y[current_head % MAP_SIZE])
        ) {
          // Moves right
          snake_x[new_head] = snake_x[current_head] + 1;
          snake_y[new_head] = snake_y[current_head];

        } else {
            // Crushed into a wall, game over
            game_state = LOST;
        }

        // If it has eaten food, increase length, otherwise move start forward
        if (has_eaten(snake_x[new_head], snake_y[new_head], food_x, food_y)) {
          snake_length++;
          // Move food to new position, if possible
          if (!new_food(&food_x, &food_y, &snake_x[snake_tail_pos], &snake_y[snake_tail_pos], snake_length)) {
            // No space left, player has won
            game_state = WIN;
          }
        } else {
          // Move tail forward, effectively removing last segment
          snake_tail_pos = (snake_tail_pos + 1) % MAP_SIZE;
        }

        // Drawing goes after this line
		    clear_screen();
      
      case LOST:
        // Drawing goes after this line
		    clear_screen();
        break;
        
      case WIN:
        // Drawing goes after this line
		    clear_screen();
        break;
    }

		



    display_end();
  }

  display_close();
}

