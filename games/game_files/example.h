#pragma once
#include "display/display.h"
#include "sprites/sprites.h"
#include "sprites/palettes.h"

typedef enum {
  UP,
  DOWN,
} Direction ;

typedef enum {
  FILLP,
  OUTLINEP,
  FILL,
  OUTLINE,
} BlockStyle;

typedef struct {
  u8 x;
  u8 y;

  u8 height;
  u8 width;

  Direction d;
  u8 animation_n;
  TextureHandle animation_pack[2][2];
} Character;

typedef struct {
  u8 tl_x;
  u8 tl_y;

  u8 height;
  u8 width;

  // only for outline
  u8 thickness;

  BlockStyle style;
  TWOS_COLOURS color;
  u8 p_index;
} Block;


void start_example(void);
void draw_character(Character* c);
void draw_block(Block b);
Block new_block(u8 x, u8 y, u8 width, u8 height, TWOS_COLOURS color);
Block new_block_p(u8 x, u8 y, u8 width, u8 height, TWOS_COLOURS color, u8 p_index);
Block new_block_outlined(u8 x, u8 y, u8 width, u8 height,u8 thickness, TWOS_COLOURS color);
Block new_block_outlined_p(u8 x, u8 y, u8 width, u8 height,u8 thickness, TWOS_COLOURS color, u8 p_index);
