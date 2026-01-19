#include "display/display.h"
#include "example.h"
#include "menu.h"
#include "stddef.h"

int main(){
	display_init_lcd();
	u8 screen = 0;

  while(1){
		switch(screen){
		case 0:
			screen = menu() + 1;
		case 1:
			// game 1
		case 2:
			// game 2
		case 3:
			start_example();
		}
  }

  display_close();
}

