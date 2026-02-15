#include "keyboard.h"
#include "display/display.h"
#include "display/types.h"
#include "sprites/sprites.h"

typedef enum {
    UPPER,
    LOWER,
} Case;

int main(){
	display_init_lcd();
  f32 proximity;

  TextureHandle lower_a = load_texture_from_sprite(lower_a_sprite.height, lower_a_sprite.width, lower_a_sprite.data);
  TextureHandle lower_b = load_texture_from_sprite(lower_b_sprite.height, lower_b_sprite.width, lower_b_sprite.data);
  TextureHandle lower_c = load_texture_from_sprite(lower_c_sprite.height, lower_c_sprite.width, lower_c_sprite.data);
  TextureHandle lower_d = load_texture_from_sprite(lower_d_sprite.height, lower_d_sprite.width, lower_d_sprite.data);
  TextureHandle lower_e = load_texture_from_sprite(lower_e_sprite.height, lower_e_sprite.width, lower_e_sprite.data);
  TextureHandle lower_f = load_texture_from_sprite(lower_f_sprite.height, lower_f_sprite.width, lower_f_sprite.data);
  TextureHandle lower_g = load_texture_from_sprite(lower_g_sprite.height, lower_g_sprite.width, lower_g_sprite.data);
  TextureHandle lower_h = load_texture_from_sprite(lower_h_sprite.height, lower_h_sprite.width, lower_h_sprite.data);
  TextureHandle lower_i = load_texture_from_sprite(lower_i_sprite.height, lower_i_sprite.width, lower_i_sprite.data);
  TextureHandle lower_j = load_texture_from_sprite(lower_j_sprite.height, lower_j_sprite.width, lower_j_sprite.data);
  TextureHandle lower_k = load_texture_from_sprite(lower_k_sprite.height, lower_k_sprite.width, lower_k_sprite.data);
  TextureHandle lower_l = load_texture_from_sprite(lower_l_sprite.height, lower_l_sprite.width, lower_l_sprite.data);
  TextureHandle lower_m = load_texture_from_sprite(lower_m_sprite.height, lower_m_sprite.width, lower_m_sprite.data);
  TextureHandle lower_n = load_texture_from_sprite(lower_n_sprite.height, lower_n_sprite.width, lower_n_sprite.data);
  TextureHandle lower_o = load_texture_from_sprite(lower_o_sprite.height, lower_o_sprite.width, lower_o_sprite.data);
  TextureHandle lower_p = load_texture_from_sprite(lower_p_sprite.height, lower_p_sprite.width, lower_p_sprite.data);
  TextureHandle lower_q = load_texture_from_sprite(lower_q_sprite.height, lower_q_sprite.width, lower_q_sprite.data);
  TextureHandle lower_r = load_texture_from_sprite(lower_r_sprite.height, lower_r_sprite.width, lower_r_sprite.data);
  TextureHandle lower_s = load_texture_from_sprite(lower_s_sprite.height, lower_s_sprite.width, lower_s_sprite.data);
  TextureHandle lower_t = load_texture_from_sprite(lower_t_sprite.height, lower_t_sprite.width, lower_t_sprite.data);
  TextureHandle lower_u = load_texture_from_sprite(lower_u_sprite.height, lower_u_sprite.width, lower_u_sprite.data);
  TextureHandle lower_v = load_texture_from_sprite(lower_v_sprite.height, lower_v_sprite.width, lower_v_sprite.data);
  TextureHandle lower_w = load_texture_from_sprite(lower_w_sprite.height, lower_w_sprite.width, lower_w_sprite.data);
  TextureHandle lower_x = load_texture_from_sprite(lower_x_sprite.height, lower_x_sprite.width, lower_x_sprite.data);
  TextureHandle lower_y = load_texture_from_sprite(lower_y_sprite.height, lower_y_sprite.width, lower_y_sprite.data);
  TextureHandle lower_z = load_texture_from_sprite(lower_z_sprite.height, lower_z_sprite.width, lower_z_sprite.data);

  TextureHandle upper_A = load_texture_from_sprite(upper_A_sprite.height, upper_A_sprite.width, upper_A_sprite.data);
  TextureHandle upper_B = load_texture_from_sprite(upper_B_sprite.height, upper_B_sprite.width, upper_B_sprite.data);
  TextureHandle upper_C = load_texture_from_sprite(upper_C_sprite.height, upper_C_sprite.width, upper_C_sprite.data);
  TextureHandle upper_D = load_texture_from_sprite(upper_D_sprite.height, upper_D_sprite.width, upper_D_sprite.data);
  TextureHandle upper_E = load_texture_from_sprite(upper_E_sprite.height, upper_E_sprite.width, upper_E_sprite.data);
  TextureHandle upper_F = load_texture_from_sprite(upper_F_sprite.height, upper_F_sprite.width, upper_F_sprite.data);
  TextureHandle upper_G = load_texture_from_sprite(upper_G_sprite.height, upper_G_sprite.width, upper_G_sprite.data);
  TextureHandle upper_H = load_texture_from_sprite(upper_H_sprite.height, upper_H_sprite.width, upper_H_sprite.data);
  TextureHandle upper_I = load_texture_from_sprite(upper_I_sprite.height, upper_I_sprite.width, upper_I_sprite.data);
  TextureHandle upper_J = load_texture_from_sprite(upper_J_sprite.height, upper_J_sprite.width, upper_J_sprite.data);
  TextureHandle upper_K = load_texture_from_sprite(upper_K_sprite.height, upper_K_sprite.width, upper_K_sprite.data);
  TextureHandle upper_L = load_texture_from_sprite(upper_L_sprite.height, upper_L_sprite.width, upper_L_sprite.data);
  TextureHandle upper_M = load_texture_from_sprite(upper_M_sprite.height, upper_M_sprite.width, upper_M_sprite.data);
  TextureHandle upper_N = load_texture_from_sprite(upper_N_sprite.height, upper_N_sprite.width, upper_N_sprite.data);
  TextureHandle upper_O = load_texture_from_sprite(upper_O_sprite.height, upper_O_sprite.width, upper_O_sprite.data);
  TextureHandle upper_P = load_texture_from_sprite(upper_P_sprite.height, upper_P_sprite.width, upper_P_sprite.data);
  TextureHandle upper_Q = load_texture_from_sprite(upper_Q_sprite.height, upper_Q_sprite.width, upper_Q_sprite.data);
  TextureHandle upper_R = load_texture_from_sprite(upper_R_sprite.height, upper_R_sprite.width, upper_R_sprite.data);
  TextureHandle upper_S = load_texture_from_sprite(upper_S_sprite.height, upper_S_sprite.width, upper_S_sprite.data);
  TextureHandle upper_T = load_texture_from_sprite(upper_T_sprite.height, upper_T_sprite.width, upper_T_sprite.data);
  TextureHandle upper_U = load_texture_from_sprite(upper_U_sprite.height, upper_U_sprite.width, upper_U_sprite.data);
  TextureHandle upper_V = load_texture_from_sprite(upper_V_sprite.height, upper_V_sprite.width, upper_V_sprite.data);
  TextureHandle upper_W = load_texture_from_sprite(upper_W_sprite.height, upper_W_sprite.width, upper_W_sprite.data);
  TextureHandle upper_X = load_texture_from_sprite(upper_X_sprite.height, upper_X_sprite.width, upper_X_sprite.data);
  TextureHandle upper_Y = load_texture_from_sprite(upper_Y_sprite.height, upper_Y_sprite.width, upper_Y_sprite.data);
  TextureHandle upper_Z = load_texture_from_sprite(upper_Z_sprite.height, upper_Z_sprite.width, upper_Z_sprite.data);

  TextureHandle lowercase_letters[26] = {
      lower_a, lower_b, lower_c, lower_d, lower_e, lower_f,
      lower_g, lower_h, lower_i, lower_j, lower_k, lower_l,
      lower_m, lower_n, lower_o, lower_p, lower_q, lower_r,
      lower_s, lower_t, lower_u, lower_v, lower_w, lower_x,
      lower_y, lower_z
  };

  TextureHandle uppercase_letters[26] = {
      upper_A, upper_B, upper_C, upper_D, upper_E, upper_F,
      upper_G, upper_H, upper_I, upper_J, upper_K, upper_L,
      upper_M, upper_N, upper_O, upper_P, upper_Q, upper_R,
      upper_S, upper_T, upper_U, upper_V, upper_W, upper_X,
      upper_Y, upper_Z
  };

  int i;
  int j;

  int letter_width = lower_a_sprite.width;
  int letter_height = lower_a_sprite.height;

  int letter_spacing = 10;
  int letter_spacing_v = 5;

  Case c = LOWER;
  TextureHandle* symbols[2] = {lowercase_letters,uppercase_letters};

  while(!window_should_close()){
    display_begin();

		// Input comes from here
		proximity = get_proximity();
		joystick_t input = get_joystick();

		switch(input){

		};


		// Physics and state changes go here




		// Drawing goes after this line
	clear_screen();


	for(i = 0; i < 6; i++){
	    for(j = 0; j < 4; j++){
			draw_texture(i*(letter_width+letter_spacing),j*(letter_height+letter_spacing_v),symbols[c][i*6+j]);
		}
	}

    display_end();
  }

  display_close();
}
