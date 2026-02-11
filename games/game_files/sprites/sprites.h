#pragma once
#include <stdint.h>

typedef struct {
	uint8_t height;
	uint8_t width;
	const uint8_t* data;
} Sprite;





extern const Sprite mman_sprite;
extern const uint8_t mman[];

extern const Sprite potato_sprite;
extern const uint8_t potato[];


extern const Sprite wdf2_sprite;
extern const uint8_t wdf2[];

extern const Sprite wuf2_sprite;
extern const uint8_t wuf2[];

extern const Sprite wuf1_sprite;
extern const uint8_t wuf1[];

extern const Sprite wdf1_sprite;
extern const uint8_t wdf1[];

extern const Sprite dino_state1_sprite;
extern const uint8_t dino_state1[];

extern const Sprite dino_state2_sprite;
extern const uint8_t dino_state2[];

extern const Sprite spaceship_sprite;
extern const uint8_t spaceship[];

extern const Sprite alien_1_sprite;
extern const uint8_t alien_1[];

extern const Sprite alien_2_sprite;
extern const uint8_t alien_2[];

extern const Sprite ufo_bonus_sprite;
extern const uint8_t ufo_bonus[];
