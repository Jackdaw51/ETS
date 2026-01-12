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
