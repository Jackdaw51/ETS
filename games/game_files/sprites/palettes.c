#include "palettes.h"

#define BW (Palette){\ 
	.colors = {\
		0,0,0,255,\
		85,85,85,255,\
		255,255,255,255,\
	}\
}\

#define RETRO_RBY (Palette){\
	.colors = {\
		255, 77, 77,255,\  
    51, 204, 204,255,\    
    255, 204, 51,255,\    
	}\
}\

#define OLIVE_GREEN (Palette){\
	.colors = {\
    48,98,48,255,\
		15,56,15,255,\  
    155,188,15,255\    
	}\
}\

#define PONG_CUSTOM_PALETTE (Palette) {\
.colors = {\
200, 195, 185, 255,\
215,  90, 175, 255,\
90, 130, 185, 255  \
}\
}\

#define DINO_CUSTOM_PALETTE (Palette) { \
.colors = { \
232, 214, 170, 255, \
76, 125,  72, 255, \
0,   0,   0, 255  \
} \
}



const Palette PaletteArray[MAX_PALETTES] = { BW,RETRO_RBY, OLIVE_GREEN, PONG_CUSTOM_PALETTE, DINO_CUSTOM_PALETTE };

	




