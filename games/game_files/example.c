#include "display/display.h"
#include "sprites/sprites.h"
#include "sprites/palettes.h"
#include "stddef.h"
#include <math.h>

i32 proximity_to_y(f32 proximity){
	return (i32)round(proximity * 0.1251);
};

int main(){
	display_init_lcd();
  f32 proximity;
	i32 sprite_y = 0;
  
	set_palette(1);
	
	TextureHandle potato_texture_handle = load_texture_from_sprite(potato_sprite.height,potato_sprite.width,potato_sprite.data);
	TextureHandle mman_texture_handle = load_texture_from_sprite(mman_sprite.height,mman_sprite.width,mman_sprite.data);

  while(!window_should_close()){
    display_begin();
		proximity = get_proximity();

		i32 linear_y = proximity_to_y(proximity);

		if (linear_y >= 128-potato_sprite.height){
			sprite_y = 128-potato_sprite.height;
		} else {
			sprite_y = proximity_to_y(proximity);
		}

    clear_screen();

		draw_rectangle(40,60,20,30,T_TWO);
		draw_texture(40,sprite_y,mman_texture_handle);
		draw_texture(100,sprite_y,potato_texture_handle);

    display_end();
  }

  display_close();
}

