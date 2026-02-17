#include "keyboard.h"
#include "display/display.h"
#include "display/types.h"
#include "sprites/sprites.h"
#include <stdio.h>

typedef enum {
    LOWER,
    UPPER,
} Case;

#define LETTER_X_OFFSET 10
#define LETTER_Y_OFFSET 7

#define NAME_SPACE_HEIGHT 20

void draw_letters(u8 choice_x,u8 choice_y,u8 rows, TextureHandle lc_letters[][6]){
  int i;
  int j;
  
  u8 total_letters = 28;
  u8 columns = (total_letters/rows)+1;
  u8 leftovers = total_letters%rows;

  u8 height = lower_a_sprite.height;
  u8 width = lower_a_sprite.width;

  u8 vertical_spacing = (128-NAME_SPACE_HEIGHT-(rows*height)-2*LETTER_Y_OFFSET)/(rows-1);
  u8 horizontal_spacing = (160-(columns*width)-2*LETTER_X_OFFSET)/(columns-1);

  u8 vertical_offset = vertical_spacing+height;
  u8 horizontal_offset = horizontal_spacing+width;

  u8 counter = 0;

  for(j = 0; j < rows; j++){
    for(i = 0; i < columns; i++){
        if(counter < 28){
          if(i == choice_x && j == choice_y){
            // the chosen letter
            // print box with a certain size
            draw_rectangle_p(LETTER_X_OFFSET+horizontal_offset*i-5,LETTER_Y_OFFSET+NAME_SPACE_HEIGHT+(vertical_offset*j)-2,width+10,height+2,T_THREE,OLIVE_GREEN_INDEX);
            draw_texture(LETTER_X_OFFSET+horizontal_offset*i,LETTER_Y_OFFSET+NAME_SPACE_HEIGHT+(vertical_offset*j),lc_letters[j][i]);
          } else {
            // the unchosen letters
            draw_texture(LETTER_X_OFFSET+horizontal_offset*i,LETTER_Y_OFFSET+NAME_SPACE_HEIGHT+(vertical_offset*j),lc_letters[j][i]);
            //draw_rectangle(LETTER_X_OFFSET+horizontal_offset*i,LETTER_Y_OFFSET+vertical_offset,width,height,T_TWO);
          }
        }
        counter++;
      }
  }
};

void draw_text_h_centers(u8 x,u8 y,u8 extra_spacing,TextBuilder* builder){
		u8 offset_x = 5 * builder->len;
		u8 offset_y = 10;
		draw_text_h(x-offset_x,y-offset_y,extra_spacing,builder);
};

void keyboard(char* buffer){
	display_init_lcd();
  f32 proximity;
  set_palette(RETRO_RBY_INDEX);

  char name[13] = {0};
  u8 name_ptr = 0;

  TextureHandle lower_a = load_texture_from_sprite(lower_a_sprite.height, lower_a_sprite.width, lower_a_sprite.data);
  TextureHandle lower_b = load_texture_from_sprite(lower_b_sprite.height, lower_b_sprite.width, lower_b_sprite.data);
  TextureHandle lower_c = load_texture_from_sprite(lower_c_sprite.height, lower_c_sprite.width, lower_c_sprite.data);
  TextureHandle lower_d = load_texture_from_sprite(lower_d_sprite.height, lower_d_sprite.width, lower_d_sprite.data);
  TextureHandle lower_e = load_texture_from_sprite(lower_e_sprite.height, lower_e_sprite.width, lower_e_sprite.data);
  TextureHandle lower_f = load_texture_from_sprite(lower_f_sprite.height, lower_f_sprite.width, lower_f_sprite.data);
  TextureHandle lower_g = load_texture_from_sprite(lower_g_sprite.height, lower_g_sprite.width, lower_g_sprite.data);
  TextureHandle lower_h = load_texture_from_sprite(lower_h_sprite.height, lower_h_sprite.width, lower_h_sprite.data);
  TextureHandle lower_i = load_texture_from_sprite(lower_i_sprite.height, lower_i_sprite.width, lower_i_sprite.data);
  TextureHandle lower_j = load_texture_from_sprite(lower_j_sprite.height, lower_j_sprite.width, lower_j_sprite.data);
  TextureHandle lower_k = load_texture_from_sprite(lower_k_sprite.height, lower_k_sprite.width, lower_k_sprite.data);
  TextureHandle lower_l = load_texture_from_sprite(lower_l_sprite.height, lower_l_sprite.width, lower_l_sprite.data);
  TextureHandle lower_m = load_texture_from_sprite(lower_m_sprite.height, lower_m_sprite.width, lower_m_sprite.data);
  TextureHandle lower_n = load_texture_from_sprite(lower_n_sprite.height, lower_n_sprite.width, lower_n_sprite.data);
  TextureHandle lower_o = load_texture_from_sprite(lower_o_sprite.height, lower_o_sprite.width, lower_o_sprite.data);
  TextureHandle lower_p = load_texture_from_sprite(lower_p_sprite.height, lower_p_sprite.width, lower_p_sprite.data);
  TextureHandle lower_q = load_texture_from_sprite(lower_q_sprite.height, lower_q_sprite.width, lower_q_sprite.data);
  TextureHandle lower_r = load_texture_from_sprite(lower_r_sprite.height, lower_r_sprite.width, lower_r_sprite.data);
  TextureHandle lower_s = load_texture_from_sprite(lower_s_sprite.height, lower_s_sprite.width, lower_s_sprite.data);
  TextureHandle lower_t = load_texture_from_sprite(lower_t_sprite.height, lower_t_sprite.width, lower_t_sprite.data);
  TextureHandle lower_u = load_texture_from_sprite(lower_u_sprite.height, lower_u_sprite.width, lower_u_sprite.data);
  TextureHandle lower_v = load_texture_from_sprite(lower_v_sprite.height, lower_v_sprite.width, lower_v_sprite.data);
  TextureHandle lower_w = load_texture_from_sprite(lower_w_sprite.height, lower_w_sprite.width, lower_w_sprite.data);
  TextureHandle lower_x = load_texture_from_sprite(lower_x_sprite.height, lower_x_sprite.width, lower_x_sprite.data);
  TextureHandle lower_y = load_texture_from_sprite(lower_y_sprite.height, lower_y_sprite.width, lower_y_sprite.data);
  TextureHandle lower_z = load_texture_from_sprite(lower_z_sprite.height, lower_z_sprite.width, lower_z_sprite.data);

  TextureHandle tick = load_texture_from_sprite(tick_sprite.height, tick_sprite.width, tick_sprite.data);
  TextureHandle canc = load_texture_from_sprite(canc_sprite.height, canc_sprite.width, canc_sprite.data);

  TextureHandle lowercase_letters[5][6] = {
      {lower_a, lower_b, lower_c, lower_d, lower_e, lower_f},
      {lower_g, lower_h, lower_i, lower_j, lower_k, lower_l},
      {lower_m, lower_n, lower_o, lower_p, lower_q, lower_r},
      {lower_s, lower_t, lower_u, lower_v, lower_w, lower_x},
      {lower_y, lower_z, canc, tick }
  };

  char letter_as_char[52] = {
    'a','b','c','d','e','f','g','h','i','j','k','l','m',
    'n','o','p','q','r','s','t','u','v','w','x','y','z',
    'A','B','C','D','E','F','G','H','I','J','K','L','M',
    'N','O','P','Q','R','S','T','U','V','W','X','Y','Z'
  };


  int i;
  int j;

  int letter_width = lower_a_sprite.width;
  int letter_height = lower_a_sprite.height;

  int letter_spacing = 10;
  int letter_spacing_v = 5;
  int rows = 5;
  int leftovers = 28%5;

  int columns = (28/5)+1;

  int choice_x = 0;
  int choice_y = 0;


  TextBuilder name_builder = { .handles = (BuilderElement[12]){}, .len = 4};
  load_text("name",&name_builder);

  set_palette(BW_INDEX);
  joystick_t old_input;

  while(!window_should_close()){
    display_begin();

		// Input comes from here
		proximity = get_proximity();
		joystick_t input = get_joystick();

    if(input != old_input){
      switch(input){
      case JS_LEFT:
        if(choice_x > 0){
          choice_x--;
        }
        break;
      case JS_RIGHT:
        if(choice_x < columns){
          if(!(choice_y == rows-1 && choice_x > leftovers-1)){
            choice_x++;
          }
        }
        break;
      case JS_DOWN:
        if(choice_y < rows-1){
          if(choice_y == rows - 2 && choice_x > leftovers){
            choice_x = leftovers;
          }
          choice_y++;
        }
        break;
      case JS_UP:
        if(choice_y > 0){
          choice_y--;
        }
        break;
      case JS_BUTTON:
        if(choice_y == rows-1 && choice_x == 2){
          name_ptr--;
          name[name_ptr] = '\0';
          name_builder.len = name_ptr;
          load_text_p(name,&name_builder,RETRO_RBY_INDEX);
        } else if(choice_y == rows-1 && choice_x == 3){
          for(i = 0; i < name_ptr; i++){
            buffer[i] = name[i];
          }
          return;
        } else if(name_ptr < 12){
          u8 index = choice_x + choice_y*6;
          name[name_ptr] = letter_as_char[index];
          printf("Name: %s\n",name);
          fflush(stdout);
          name_ptr += 1;
          name_builder.len = name_ptr;
          load_text_p(name,&name_builder,RETRO_RBY_INDEX);
        }
    
        break;
      };
    }

    old_input = input;

	  clear_screen();

    draw_text_h_centers(160/2,lower_a_sprite.height-4,1,&name_builder);
    draw_letters(choice_x,choice_y,rows,lowercase_letters);

    display_end();
  }

  display_close();
}
