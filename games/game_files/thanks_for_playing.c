#include "thanks_for_playing.h"
#include "../../incl/timers.h"

void draw_text_h_centertp(u8 x,u8 y,u8 extra_spacing,TextBuilder* builder){
		u8 offset_x = 5 * builder->len;
		u8 offset_y = 10;
		draw_text_h(x-offset_x,y-offset_y,extra_spacing,builder);
};


int thanks(){
	display_init_lcd();
  f32 proximity;

    set_palette(RETRO_RBY_INDEX);
    
    TextBuilder title1_builder = { .handles = (BuilderElement[6]){}, .len = 6};
    load_text("Thanks",&title1_builder);

    TextBuilder title2_builder = { .handles = (BuilderElement[3]){}, .len = 3};
    load_text("For",&title2_builder);

    TextBuilder title3_builder = { .handles = (BuilderElement[7]){}, .len = 7};
    load_text("Playing",&title3_builder);

    set_palette(BW_INDEX);

    while(!window_should_close()){
        display_begin();

            // Input comes from here
            proximity = get_proximity();



            // Physics and state changes go here



            // Drawing goes after this line
            clear_screen();

            draw_text_h_centertp(80,30,0,&title1_builder);
            draw_text_h_centertp(80,50,0,&title2_builder);
            draw_text_h_centertp(80,70,0,&title3_builder);


            // sleep and return


        display_end();
        sleep_ms(2000);
                    break;

    }

    display_close();
}

