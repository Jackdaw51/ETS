#pragma once
#include <stdint.h>

#define MAX_PALETTES 16
// Indices used to recognize pallettes inside the palette array
#define BW_INDEX 0
#define RETRO_RBY_INDEX 1
#define OLIVE_GREEN_INDEX 2


typedef struct {
	uint8_t colors[3*4];
} Palette;

#define PALETTE_ARR_LEN 3

//#define PALETTE_ARR_LEN 3

extern const Palette565 PaletteArray565[MAX_PALETTES];
extern const Palette PaletteArray[MAX_PALETTES];
