#include "display/display.h"
#include "stddef.h"

int main(){
	display_init_lcd();
  f32 proximity;

  while(!window_should_close()){
    display_begin();

		// Input comes from here
		proximity = get_proximity();



		// Physics and state changes go here



		// Drawing goes after this line
		clear_screen();



    display_end();
  }

  display_close();
}

