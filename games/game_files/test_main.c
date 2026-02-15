#pragma once
#include "display/display.h"
#include "snake.h"
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
		break;
	  case 1:
		// game 1
		break;
	  case 2:
		// game 2
		break;
	  case 3:
		start_example();
		break;
	  case 4:
		//start_snake();
		int max_length;
		int result = start_snake(&max_length);
		break;
	}
  }

  display_close();
}

