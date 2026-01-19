#include "display/display.h"
#include "sprites/palettes.h"
#include "stddef.h"

// make left hand of screen choice box

#define CHOICE_X_OFFSET 3
#define CHOICE_Y_OFFSET 3

#define CHOICE_WIDTH 15
#define CHOICE_GAP 4 

#define NON_CHOICE_THICKNESS 2.0f

typedef enum {
	EASY,
	MEDIUM,
	HARD,
} Difficulty;

const u8 difficulty_to_choices[3] = {2,4,8};

void draw_choices(u8 choice,u8 n_choices, u8 choice_height){
	for(int i=0; i < n_choices; i++){
		if(i == choice){
			draw_rectangle_p(CHOICE_X_OFFSET,CHOICE_Y_OFFSET+i*(choice_height+CHOICE_GAP),CHOICE_WIDTH,choice_height,T_ONE,RETRO_RBY_INDEX);
		} else {
			draw_rectangle_outline_p(CHOICE_X_OFFSET,CHOICE_Y_OFFSET+i*(choice_height+CHOICE_GAP),CHOICE_WIDTH,choice_height,NON_CHOICE_THICKNESS,T_ONE,RETRO_RBY_INDEX);
		}
	}
};

static u8 rng = 1;   // must be non-zero

u8 rand8(void) {
    rng ^= rng << 3;
    rng ^= rng >> 5;
    rng ^= rng << 1;
    return rng;
}

uint8_t rand8_range(uint8_t max) {
    return ((uint16_t)rand8() * (max + 1)) >> 8;
}

int main(){
	display_init_lcd(); f32 proximity;
	set_palette(BW_INDEX);
	set_screen_color(T_TWO);

	Difficulty d = MEDIUM;
	
	u8 n_choices = difficulty_to_choices[0];
	u8 choice_height = (u8)((128 - 2*CHOICE_Y_OFFSET - (n_choices-1)*CHOICE_GAP)/(f32)n_choices);

	u8 choice = 0;

  while(!window_should_close()){
    display_begin();

		// Input comes from here
		proximity = get_proximity();

		choice = (proximity * n_choices)/ 1024;


		// Physics and state changes go here



		// Drawing goes after this line
		clear_screen();
		draw_choices(choice,n_choices,choice_height);

    display_end();
  }

  display_close();
}

