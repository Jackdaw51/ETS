#pragma once

#define MAX_PALETTES 10
#define BW_INDEX 0
#define RETRO_RBY_INDEX 1
#define OLIVE_GREEN_INDEX 2
#define PONG_CUSTOM_PALETTE_INDEX 3
#define DINO_CUSTOM_PALETTE_INDEX 4
#define SPACESHIP_PALETTE_INDEX 5
#define ALIEN_1_PALETTE_INDEX 6
#define ALIEN_2_PALETTE_INDEX 7
#define UFO_PALETTE_INDEX 8

#include <stdint.h>

typedef struct {
	uint8_t colors[3*4];	
} Palette;

#define PALETTE_ARR_LEN 9

extern const Palette PaletteArray[MAX_PALETTES];


