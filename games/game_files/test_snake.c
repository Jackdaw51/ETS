#include "display/display.h"

void test_snake() {
  set_palette(SNAKE_INDEX);
  //set_mapping_array((u8[3]){0,1,2}); // This only needed to switch colors of the palette
  set_screen_color(0);

  TextureHandle food_texture = load_texture_from_sprite_p(apple_sprite.height, apple_sprite.width, apple_sprite.data, APPLE_INDEX);

  
  while(!window_should_close()){
    display_begin();
    clear_screen();
    draw_texture(10*8, 10*8, food_texture);

    display_end();
  }
}