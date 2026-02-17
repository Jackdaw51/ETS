#include "ask_upload.h"

void draw_text_h_centerau(u8 x,u8 y,u8 extra_spacing,TextBuilder* builder, u8 choice){
		u8 offset_x = 5 * builder->len;
		u8 offset_y = 10;
        if(choice == 0){
            draw_rectangle_p(x-offset_x-3,y-offset_y,32,25,T_THREE,OLIVE_GREEN_INDEX);
        }
		draw_text_h(x-offset_x,y-offset_y,extra_spacing,builder);
};

int ask_upload(){
	display_init_lcd();
    f32 proximity;

    set_palette(RETRO_RBY_INDEX);

    TextBuilder title_builder = { .handles = (BuilderElement[12]){}, .len = 12};
    load_text("Upload Score",&title_builder);

    TextBuilder yes_builder = { .handles = (BuilderElement[3]){}, .len = 3};
    load_text("YES",&yes_builder);

    TextBuilder no_builder = { .handles = (BuilderElement[2]){}, .len = 2};
    load_text("NO",&no_builder);

    set_palette(BW_INDEX);

    joystick_t old_input = JS_NONE;
    u8 choice = 0;

    while(!window_should_close()){
        display_begin();

            // Input comes from here
            proximity = get_proximity();
            joystick_t input = get_joystick();

            if(old_input != input){
                if(input == JS_LEFT || input == JS_RIGHT) choice = !choice;
                if(input == JS_BUTTON) return choice;
            }

            old_input = input;
            // Physics and state changes go here



            // Drawing goes after this line
        clear_screen();

        draw_text_h_centerau(90,30,0,&title_builder,1);
        
        draw_text_h_centerau(55,80,0,&yes_builder,choice);
        draw_text_h_centerau(105,80,0,&no_builder,!choice);

        display_end();
    }

    display_close();
}

