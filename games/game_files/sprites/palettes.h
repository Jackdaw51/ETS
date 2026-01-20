#pragma once

#define MAX_PALETTES 10
#define BW_INDEX 0
#define RETRO_RBY_INDEX 1
#define OLIVE_GREEN_INDEX 2
#define PONG_CUSTOM_PALETTE_INDEX 3

#include <stdint.h>

typedef struct {
	uint8_t colors[3*4];	
} Palette;

#define PALETTE_ARR_LEN 4

extern const Palette PaletteArray[MAX_PALETTES];


