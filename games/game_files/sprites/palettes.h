
#pragma once
#include <stdint.h>

#define MAX_PALETTES 16
// Indices used to recognize pallettes inside the palette array
#define BW_INDEX 0
#define RETRO_RBY_INDEX 1
#define OLIVE_GREEN_INDEX 2
#define SNAKE_INDEX 3
#define RAT_INDEX 4
#define APPLE_INDEX 5
#define PONG_CUSTOM_PALETTE_INDEX 6
#define DINO_CUSTOM_PALETTE_INDEX 7
#define SPACESHIP_PALETTE_INDEX 8
#define ALIEN_1_PALETTE_INDEX 9
#define ALIEN_2_PALETTE_INDEX 10
#define UFO_PALETTE_INDEX 11


typedef struct {
    uint8_t colors[3*4];
} Palette;

typedef struct {
    uint16_t colors[3];
} Palette565;

extern const Palette565 PaletteArray565[MAX_PALETTES];
extern const Palette PaletteArray[MAX_PALETTES];

