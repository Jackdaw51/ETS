#pragma once

#define MAX_PALETTES 10
#define BW_INDEX 0
#define RETRO_RBY_INDEX 1
#define OLIVE_GREEN_INDEX 2

#include <stdint.h>

typedef struct {
	uint8_t colors[3*4];	
} Palette;

typedef struct {
	uint16_t colors[3];	
} Palette565;

#define PALETTE_ARR_LEN 3

extern const Palette565 PaletteArray565[MAX_PALETTES];
extern const Palette PaletteArray[MAX_PALETTES];


