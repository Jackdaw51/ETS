<<<<<<< Updated upstream
<<<<<<< Updated upstream
#include "display/display.h"
#include "sprites/sprites.h"
#include "sprites/palettes.h"
=======
>>>>>>> Stashed changes
#include "example.h"

=======
#include "example.h"
#include "../../incl/timers.h"
>>>>>>> Stashed changes
i32 proximity_to_y(f32 proximity, u8 maxY){
  f32 ratio = (f32)maxY/1023.0f;
	return (i32)(proximity * ratio + 0.5f);
};

<<<<<<< Updated upstream
<<<<<<< Updated upstream
int main(){
	display_init_lcd();
=======
=======
>>>>>>> Stashed changes
void m_example(){
    display_init_lcd();
    start_example();
    display_close();
}

//int main(){
//s  m_example();
//}

void start_example(){
<<<<<<< Updated upstream
>>>>>>> Stashed changes
=======
>>>>>>> Stashed changes
  f32 proximity;
  f32 proximity_old;
	i32 sprite_y = 0;
	i32 old_sprite_y = 0;

  // Using the base palette when generating sprites

  TextureHandle wdf1_texture_handle = load_texture_from_sprite(wdf1_sprite.height,wdf1_sprite.width,wdf1_sprite.data);
  TextureHandle wdf2_texture_handle = load_texture_from_sprite(wdf2_sprite.height,wdf2_sprite.width,wdf2_sprite.data);
  TextureHandle wuf1_texture_handle = load_texture_from_sprite(wuf1_sprite.height,wuf1_sprite.width,wuf1_sprite.data);
  TextureHandle wuf2_texture_handle = load_texture_from_sprite(wuf2_sprite.height,wuf2_sprite.width,wuf2_sprite.data);

  // Using a specified palette

  TextureHandle wdf1_texture_handle2 = load_texture_from_sprite_p(wdf1_sprite.height,wdf1_sprite.width,wdf1_sprite.data,RETRO_RBY_INDEX);
  TextureHandle wdf2_texture_handle2 = load_texture_from_sprite_p(wdf2_sprite.height,wdf2_sprite.width,wdf2_sprite.data,RETRO_RBY_INDEX);
  TextureHandle wuf1_texture_handle2 = load_texture_from_sprite_p(wuf1_sprite.height,wuf1_sprite.width,wuf1_sprite.data,RETRO_RBY_INDEX);
  TextureHandle wuf2_texture_handle2 = load_texture_from_sprite_p(wuf2_sprite.height,wuf2_sprite.width,wuf2_sprite.data,RETRO_RBY_INDEX);

  // game data

  Block world_blocks[20]; // 20 is max blocks can be any number
  u8 world_blocks_len = 0;

  Character p1 = (Character){
  .x = 0,
  .height = wdf1_sprite.height,
  .width = wdf1_sprite.width,

  .d = DOWN,
  .animation_pack = {{wdf1_texture_handle,wdf2_texture_handle},{wuf1_texture_handle,wuf2_texture_handle}},
  };

  Character p2 = (Character){
  .x = 150-wdf1_sprite.width,
  .height = wdf1_sprite.height,
  .width = wdf1_sprite.width,

  .d = UP,
  .animation_pack = {{wdf1_texture_handle2,wdf2_texture_handle2},{wuf1_texture_handle2,wuf2_texture_handle2}},
  };

  i32 counter = 0;
  i32 counter2 = 0;
  i32 direction = 0;

  u8 max_height = 128-wdf1_sprite.height;

<<<<<<< Updated upstream
<<<<<<< Updated upstream
  // Setting the color map and palette
  //u8 map[3] = {2,1,0};
  //set_mapping_array(map);
  set_screen_color(1);
	set_palette(RETRO_RBY_INDEX);
  // Selecting which color is the background

  // Setting up static world
  // note that to make blocks move you would put this inside the loop and clear the world_blocks array at the start of the loop
=======
  set_screen_color(T_ONE);
  set_palette(OLIVE_GREEN_INDEX);
>>>>>>> Stashed changes
=======
  set_screen_color(T_ONE);
  set_palette(OLIVE_GREEN_INDEX);
>>>>>>> Stashed changes

  world_blocks[world_blocks_len++] = new_block(0,70,30,30,T_THREE);
  world_blocks[world_blocks_len++] = new_block(0,100,24,20,T_THREE);
  world_blocks[world_blocks_len++] = new_block_outlined_p(110,10,30,30,8,T_TWO,BW_INDEX);
  world_blocks[world_blocks_len++] = new_block_p(125,40,15,30,T_TWO,BW_INDEX);

  while(!window_should_close()){
    display_begin();

    // INPUT
		proximity = get_proximity();

    if(counter2 == 3){
      counter = (counter==0 ? 1 : 0);
      counter2 = 0;
    }

    if(proximity != proximity_old){
      counter2++;
      if(proximity > proximity_old) direction = 0;
      if(proximity < proximity_old) direction = 1;
    } 

    sprite_y = proximity_to_y(proximity,max_height);

    p2.prev_y = p2.y;
    p2.y = 128-sprite_y-p1.height;
    p2.d = 1-direction;
    p2.animation_n = counter%2;

    p1.prev_y = p1.y;
    p1.y = sprite_y;
    p1.d = direction;
    p1.animation_n = counter%2;

    // DRAWING
    clear_screen();

<<<<<<< Updated upstream
<<<<<<< Updated upstream
    for(int i = 0; i < world_blocks_len; i++){
      //draw_block(world_blocks[i]);
    }

    draw_texture(74,48,R_handle);
    draw_texture(84,48,O_handle);
    draw_texture(94,48,W_handle);
    draw_texture(104,48,A_handle);
    draw_texture(114,48,N_handle);
=======
    int i;
    for(i = 0; i < world_blocks_len; i++){
      //draw_block(world_blocks[i]);
    }

    draw_rectangle(0,0,20,50,T_THREE);
    draw_rectangle(60,50,50,40,T_TWO);
	  //draw_text_h(74,p1.y,1,&rowan_builder);
>>>>>>> Stashed changes
=======
    int i;
    for(i = 0; i < world_blocks_len; i++){
      //draw_block(world_blocks[i]);
    }

    draw_rectangle(0,0,20,50,T_THREE);
    draw_rectangle(60,50,50,40,T_TWO);
	  //draw_text_h(74,p1.y,1,&rowan_builder);
>>>>>>> Stashed changes
    draw_character(&p1);
    draw_character(&p2);

    proximity_old = proximity;
    display_end();
    sleep_ms(10);
  }

  display_close();
}


void draw_character(Character* c){
  //draw_rectangle(c->x,c->prev_y,c->width,c->height,T_THREE);
  draw_texture(c->x,c->y,c->animation_pack[c->d][c->animation_n]);
}

Block new_block_p(u8 x, u8 y, u8 width, u8 height, TWOS_COLOURS color, u8 p_index){
  return (Block){
    .tl_x = x,
    .tl_y = y,
    .width = width,
    .height = height,
    .color = color,
    .p_index = p_index,
    .style = FILLP,
  };
};

Block new_block(u8 x, u8 y, u8 width, u8 height, TWOS_COLOURS color){
  return (Block){
    .tl_x = x,
    .tl_y = y,
    .width = width,
    .height = height,
    .color = color,
    .style = FILL,
  };
};

Block new_block_outlined_p(u8 x, u8 y, u8 width, u8 height,u8 thickness, TWOS_COLOURS color, u8 p_index){
  return (Block){
    .tl_x = x,
    .tl_y = y,
    .width = width,
    .height = height,
    .thickness = thickness,
    .color = color,
    .p_index = p_index,
    .style = OUTLINEP,
  };
};

Block new_block_outlined(u8 x, u8 y, u8 width, u8 height,u8 thickness, TWOS_COLOURS color){
  return (Block){
    .tl_x = x,
    .tl_y = y,
    .width = width,
    .height = height,
    .thickness = thickness,
    .color = color,
    .style = OUTLINE,
  };
};

 
void draw_block(Block b){
  switch(b.style){
    case FILLP:
      draw_rectangle_p(b.tl_x,b.tl_y,b.width,b.height,b.color,b.p_index);
      break;
    case OUTLINEP:
      draw_rectangle_outline_p(b.tl_x,b.tl_y,b.width,b.height,b.thickness,b.color,b.p_index);
      break;
    case FILL:
      draw_rectangle(b.tl_x,b.tl_y,b.width,b.height,b.color);
      break;
    case OUTLINE:
      draw_rectangle_outline(b.tl_x,b.tl_y,b.width,b.height,b.thickness,b.color);
      break;
  }
}
