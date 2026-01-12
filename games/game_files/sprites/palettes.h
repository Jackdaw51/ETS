#pragma once

#define MAX_PALETTES 10

#include <stdint.h>

typedef struct {
	uint8_t colors[3*4];	
} Palette;

extern const Palette PaletteArray[MAX_PALETTES];
extern const Palette BW;
extern const Palette Retro_RBY;

