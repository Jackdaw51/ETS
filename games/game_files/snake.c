#include "display/display.h"
#include "stddef.h"
#include "snake.h"
#include <stdio.h>


#define MAP_WIDTH 20 // Size of the game area in blocks
#define MAP_HEIGHT 16
#define MAP_SIZE (MAP_WIDTH * MAP_HEIGHT)
u8 rng = 1;

static inline joystick_t joystick_action(void) {
  #ifdef SIMULATION_PC
    if (IsKeyDown(KEY_ENTER)) return JS_BUTTON;
    if (IsKeyDown(KEY_W))     return JS_UP;
    if (IsKeyDown(KEY_A))     return JS_LEFT;
    if (IsKeyDown(KEY_S))     return JS_DOWN;
    if (IsKeyDown(KEY_D))     return JS_RIGHT;
    return JS_NONE;
  #else
    return get_joystick();
  #endif
}


int start_snake(int *max_length){
  *max_length = MAP_SIZE;
  joystick_t joystick;
  State game_state = PLAYING;
  Quad_Direction direction = Q_RIGHT; // By default it moves to the right

  u8 snake_x[MAP_SIZE]; // Work as circular buffers
  u8 snake_y[MAP_SIZE];
  u8 snake_tail_pos = 0;
  u8 snake_length = 3; // Create starting snake
  u8 i;
  for (i = 0; i < snake_length; i++){
    snake_x[i] = MAP_WIDTH/4 - 1 + i; // The snake is built in reverse wrt the array
    snake_y[i] = MAP_HEIGHT/2;
  }

  u8 food_x = 3*(MAP_WIDTH/4);
  u8 food_y = MAP_HEIGHT/2;
  boolean food_type = false; // false for the apple, true for the mouse

  
  set_palette(SNAKE_INDEX);
  set_screen_color(0); // Background color should be se to black

  TextureHandle food_texture = load_texture_from_sprite_p(apple_sprite.height, apple_sprite.width, apple_sprite.data, APPLE_INDEX);
  TextureHandle rat_texture = load_texture_from_sprite_p(rat_sprite.height, rat_sprite.width, rat_sprite.data, RAT_INDEX);
  TextureHandle snake_texture[14]; // 14 different snake sprites
  // Main pieces
  snake_texture[0] = load_texture_from_sprite(sn_horizontal_sprite.height, sn_horizontal_sprite.width, sn_horizontal_sprite.data);
  snake_texture[1] = load_texture_from_sprite(sn_vertical_sprite.height, sn_vertical_sprite.width, sn_vertical_sprite.data);
  // Turning
  snake_texture[2] = load_texture_from_sprite(sn_upleft_sprite.height, sn_upleft_sprite.width, sn_upleft_sprite.data);
  snake_texture[3] = load_texture_from_sprite(sn_upright_sprite.height, sn_upright_sprite.width, sn_upright_sprite.data);
  snake_texture[4] = load_texture_from_sprite(sn_downleft_sprite.height, sn_downleft_sprite.width, sn_downleft_sprite.data);
  snake_texture[5] = load_texture_from_sprite(sn_downright_sprite.height, sn_downright_sprite.width, sn_downright_sprite.data);
  // Heads
  snake_texture[6] = load_texture_from_sprite(sn_head_up_sprite.height, sn_head_up_sprite.width, sn_head_up_sprite.data);
  snake_texture[7] = load_texture_from_sprite(sn_head_down_sprite.height, sn_head_down_sprite.width, sn_head_down_sprite.data);
  snake_texture[8] = load_texture_from_sprite(sn_head_left_sprite.height, sn_head_left_sprite.width, sn_head_left_sprite.data);
  snake_texture[9] = load_texture_from_sprite(sn_head_right_sprite.height, sn_head_right_sprite.width, sn_head_right_sprite.data);
  // Tails
  snake_texture[10] = load_texture_from_sprite(sn_tail_up_sprite.height, sn_tail_up_sprite.width, sn_tail_up_sprite.data);
  snake_texture[11] = load_texture_from_sprite(sn_tail_down_sprite.height, sn_tail_down_sprite.width, sn_tail_down_sprite.data);
  snake_texture[12] = load_texture_from_sprite(sn_tail_left_sprite.height, sn_tail_left_sprite.width, sn_tail_left_sprite.data);
  snake_texture[13] = load_texture_from_sprite(sn_tail_right_sprite.height, sn_tail_right_sprite.width, sn_tail_right_sprite.data);
  printf("Snake game started!\n");
  fflush(stdout);
  // Run the game loop
  while(!window_should_close()){
    display_begin();
    printf("Game state: %d\n", game_state);
    fflush(stdout);

		switch (game_state) {
      case PLAYING:
        // Input comes from here
        joystick = joystick_action();
        switch(joystick) {
          case JS_UP:
            if (direction != Q_DOWN) {
              direction = Q_UP;
            }
            break;
          case JS_DOWN:
            if (direction != Q_UP) {
              direction = Q_DOWN;
            }
            break;
          case JS_LEFT:
            if (direction != Q_RIGHT) {
              direction = Q_LEFT;
            }
            break;
          case JS_RIGHT:
            if (direction != Q_LEFT) {
              direction = Q_RIGHT;
            }
            break;
          default:
            break;
        }

        // Physics and state changes go here
        u8 current_head = (snake_tail_pos + snake_length - 1) % MAP_SIZE;
        u8 new_head = (current_head + 1) % MAP_SIZE;

        // Based on joystick input, check if the snake can move
        // in that direction and apply the movement if possible
        if (direction == Q_UP &&
          snake_y[current_head] > 0 &&
          !check_crush(&snake_x[snake_tail_pos], &snake_y[snake_tail_pos],
            snake_length, snake_x[current_head], snake_y[current_head] - 1)
        ) {
          // Moves up
          snake_x[new_head] = snake_x[current_head];
          snake_y[new_head] = snake_y[current_head] - 1;

        } else if (direction == Q_DOWN &&
          snake_y[current_head % MAP_SIZE] < MAP_HEIGHT - 1 &&
          !check_crush(&snake_x[snake_tail_pos], &snake_y[snake_tail_pos],
            snake_length, snake_x[current_head % MAP_SIZE], snake_y[current_head % MAP_SIZE] + 1)
        ) {
          // Moves down
          snake_x[new_head] = snake_x[current_head];
          snake_y[new_head] = snake_y[current_head] + 1;

        } else if (direction == Q_LEFT &&
          snake_x[current_head % MAP_SIZE] > 0 &&
          !check_crush(&snake_x[snake_tail_pos], &snake_y[snake_tail_pos], snake_length,
            snake_x[current_head % MAP_SIZE] - 1, snake_y[current_head % MAP_SIZE])
        ) {
          // Moves left
          snake_x[new_head] = snake_x[current_head] - 1;
          snake_y[new_head] = snake_y[current_head];
          
        } else if (direction == Q_RIGHT &&
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
          if (!new_food(&food_x, &food_y, &food_type, &snake_x[snake_tail_pos], &snake_y[snake_tail_pos], snake_length)) {
            // No space left, player has won
            game_state = WIN;
          }
        } else if (game_state == PLAYING) {
          // Move tail forward, effectively removing last segment
          snake_tail_pos = (snake_tail_pos + 1) % MAP_SIZE;
        }
        

        break;

      case LOST:
        return snake_length - 3; // -3 due to it being initial length.
        break;
        
      case WIN:
        return snake_length - 3;
        break;

    }


    // Drawing goes after this line
    clear_screen();
    draw_texture(food_x*8, food_y*8, food_texture);
    draw_snake(snake_x, snake_y, snake_tail_pos, snake_length, snake_texture);
    if (game_state == PLAYING) {
      // Draw the food at new position
      if (food_type) {
        // 10% chance of being a rat
        draw_food(food_x, food_y, rat_texture);
      } else {
        draw_food(food_x, food_y, food_texture);
      }
    }
    display_end();
  }
}

boolean check_crush(u8* snake_x, u8* snake_y, u8 snake_length, u8 new_head_x, u8 new_head_y) {
    u8 i;
    for (i=0; i < snake_length-1; i++) {
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

boolean new_food(u8* food_x, u8* food_y, boolean* food_type, u8* snake_x, u8* snake_y, u8 snake_length) {
  // Generate a new position for the food that is not on the snake
  if (snake_length >= MAP_SIZE) {
    return false; // No space left, won the game
  }

  // Keep generating positions until a valid one is found
  boolean valid_position = false;
  while (!valid_position) {
    *food_x = rand8() % MAP_WIDTH;
    *food_y = rand8() % MAP_HEIGHT;
    valid_position = true;
    u8 i;
    for (i = 0; i < snake_length; i++) {
      if (snake_x[i] == *food_x && snake_y[i] == *food_y) {
        valid_position = false;
        break;
      }
    }
  }

  if (rand8() % 10 == 0) {
    food_type = true;
  } else {
    food_type = false;
  }

  return true;
}

void draw_snake(u8* snake_x, u8* snake_y, u8 snake_tail_pos, u8 snake_length, TextureHandle* snake_texture) {
  // Draw the snake based on its segments' positions
  u8 i;
  for (i = 0; i < snake_length; i++) {
    TextureHandle segment_texture;
    // Calculate current, next, and previous segment positions in circular buffer
    u8 current_pos = (snake_tail_pos + i) % MAP_SIZE;
    u8 next_pos = (snake_tail_pos + i + 1) % MAP_SIZE;
    u8 prev_pos = (snake_tail_pos + i - 1 + MAP_SIZE) % MAP_SIZE;

    // Determine which texture to use based on segment position
    if (i == 0) {
      // Tail segment
      if (snake_x[current_pos] == snake_x[next_pos]) {
        // Vertical tail
        if (snake_y[current_pos] < snake_y[next_pos]) {
          segment_texture = snake_texture[10]; // Tail up
        } else {
          segment_texture = snake_texture[11]; // Tail down
        }
      } else {
        // Horizontal tail
        if (snake_x[current_pos] < snake_x[next_pos]) {
          segment_texture = snake_texture[12]; // Tail left
        } else {
          segment_texture = snake_texture[13]; // Tail right
        }
      }
    } else if (i == snake_length - 1) {
      // Head segment
      if (snake_x[current_pos] == snake_x[prev_pos]) {
        // Vertical head
        if (snake_y[current_pos] < snake_y[prev_pos]) {
          segment_texture = snake_texture[6]; // Head up
        } else {
          segment_texture = snake_texture[7]; // Head down
        }
      } else {
        // Horizontal head
        if (snake_x[current_pos] < snake_x[prev_pos]) {
          segment_texture = snake_texture[8]; // Head left
        } else {
          segment_texture = snake_texture[9]; // Head right
        }
      }
    } else {
      // Body segment
      if (snake_x[current_pos] == snake_x[prev_pos]) {
        if (snake_x[current_pos] == snake_x[next_pos]) {
          // Vertical segment
          segment_texture = snake_texture[1]; // Vertical
        } else {
          // Turning segment
          if (snake_y[current_pos] < snake_y[prev_pos] && snake_x[current_pos] > snake_x[next_pos]) {
            segment_texture = snake_texture[2]; // Upleft
          } else if (snake_y[current_pos] < snake_y[prev_pos] && snake_x[current_pos] < snake_x[next_pos]) {
            segment_texture = snake_texture[3]; // Upright
          } else if (snake_y[current_pos] > snake_y[prev_pos] && snake_x[current_pos] > snake_x[next_pos]) {
            segment_texture = snake_texture[4]; // Downleft
          } else {
            segment_texture = snake_texture[5]; // Downright
          }
        }
        //else if (snake_y[current_pos] == snake_y[prev_pos]) {
      } else { // For how the snake is saved, we shouldn't need to check also the y coordinates
        if (snake_y[current_pos] == snake_y[next_pos]) {
          // Horizontal segment
          segment_texture = snake_texture[0]; // Horizontal
        } else {
          // Turning segment
          if (snake_x[current_pos] > snake_x[prev_pos] && snake_y[current_pos] < snake_y[next_pos]) {
            segment_texture = snake_texture[2]; // Rightdown
          } else if (snake_x[current_pos] < snake_x[prev_pos] && snake_y[current_pos] < snake_y[next_pos]) {
            segment_texture = snake_texture[3]; // Leftdown
          } else if (snake_x[current_pos] > snake_x[prev_pos] && snake_y[current_pos] > snake_y[next_pos]) {
            segment_texture = snake_texture[4]; // Rightup
          } else {
            segment_texture = snake_texture[5]; // Leftup
          }
        }
      }
    }

    draw_texture(snake_x[current_pos]*8, snake_y[current_pos]*8, segment_texture);
  }

}

void draw_food(u8 food_x, u8 food_y, TextureHandle food_texture) {
  draw_texture(food_x*8, food_y*8, food_texture);
}

u8 rand8(void) {
    rng ^= rng << 3;
    rng ^= rng >> 5;
    rng ^= rng << 1;
    return rng;
}
