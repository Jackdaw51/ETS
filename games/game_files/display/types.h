
#pragma once
#include <stdint.h>

#define LCD_SCREEN_WIDTH 160
#define LCD_SCREEN_HEIGHT 128

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef float f32;

typedef u8 boolean;

typedef enum {
	LETTER,
	SPACE
} SymbolType;

typedef uint8_t TextureHandle;

typedef struct {
	SymbolType type;
	TextureHandle handle;
} BuilderElement;

typedef struct {
	u16 len;
	BuilderElement* handles;
} TextBuilder;

typedef enum TWOS_COLOURS {
	T_ONE = 0,
	T_TWO = 1,
	T_THREE = 2,
	T_TRANSPARENT = 3,
} TWOS_COLOURS;
