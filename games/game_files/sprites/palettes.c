#include "palettes.h"

const Palette BW = { 
	.colors = {
		0,0,0,255,
		85,85,85,255,
		255,255,255,255,
	}
};

const Palette Retro_RBY = {
	.colors = {
		255, 77, 77,255,    // Red
    51, 204, 204,255,   // Green 
    255, 204, 51,255,    // Yellow
	}
};

const Palette PaletteArray[MAX_PALETTES] = { BW,Retro_RBY};

	




