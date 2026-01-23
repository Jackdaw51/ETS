#include "palettes.h"

#define BW (Palette){\
	.colors = {\
		0, 0, 0, 255,\
		85, 85, 85, 255,\
		255, 255, 255, 255,\
	}\
}\

#define RETRO_RBY (Palette){\
	.colors = {\
		255, 77, 77, 255,\
    	51, 204, 204, 255,\
    	255, 204, 51, 255,\
	}\
}\

#define OLIVE_GREEN (Palette){\
	.colors = {\
    	48, 98, 48, 255,\
		15, 56, 15, 255,\
    	155, 188, 15, 255,\
	}\
}\

#define BW_565 (Palette565){\
    .colors = {\
        0x0000,\
        0x52AA,\
        0xFFFF,\
    }\
}\

#define RETRO_RBY_565 (Palette565){\
	.colors = {\
		0xFC89,\
		0x3679,\
		0xFE66,\
	}\
}\

#define OLIVE_GREEN_565 (Palette565){\
	.colors = {\
		0x3306,\
		0x09C1,\
		0x9DE1,\
	}\
}\

#define SNAKE_PALETTE (Palette){\
	.colors = {\
		0, 0, 0, 255,\
		16, 81, 33, 255,\
		25, 117, 49, 255,\
	}\
}\

#define SNAKE_PALETTE_565 (Palette565){\
	.colors = {\
		0x0000,\
		0x1284,\
		0x1ba6,\
	}\
}\

#define FIBONACCI_RAT_PALETTE (Palette){\
	.colors = {\
		0, 0, 0, 255,\
		132, 134, 132, 255,\
		247, 130, 255, 255,\
	}\
}\

#define FIBONACCI_RAT_PALETTE_565 (Palette565){\
	.colors = {\
		0x0000,\
		0x8430,\
		0xf41f,\
	}\
}\

#define APPLE_PALETTE (Palette){\
	.colors = {\
		0, 0, 0, 255,\
		182, 16, 16, 255,\
		16, 81, 33, 255,\
	}\
}\

#define APPLE_PALETTE_565 (Palette565){\
	.colors = {\
		0x0000,\
		0xb082,\
		0x1284,\
	}\
}\

const Palette PaletteArray[MAX_PALETTES] = { BW, RETRO_RBY, OLIVE_GREEN, SNAKE_PALETTE, FIBONACCI_RAT_PALETTE, APPLE_PALETTE };
const Palette565 PaletteArray565[MAX_PALETTES] = { BW_565, RETRO_RBY_565, OLIVE_GREEN_565, SNAKE_PALETTE_565, FIBONACCI_RAT_PALETTE_565, APPLE_PALETTE_565 };

	




