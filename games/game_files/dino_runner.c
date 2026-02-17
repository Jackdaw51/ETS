#include "dino_runner.h"

// ------------------------------------------------------------
// LCD size
#define LCD_W 160
#define LCD_H 128

// Ground
#define GROUND_Y      (LCD_H - 18)

// Dino position (x fixed)
#define DINO_X        22

// Obstacles
#define MAX_OBS       6
#define SPAWN_MIN_FR  80
#define SPAWN_MAX_FR  110

// Jump cooldown
#define JUMP_COOLDOWN_FR 10

// Walk animation speed (frames per step)
#define WALK_ANIM_PERIOD 7

// ------------------------------------------------------------
// HITBOX TUNING
#define HIT_PAD_L  3
#define HIT_PAD_R  3
#define HIT_PAD_T  2
#define HIT_PAD_B  1

#define OBS_PAD_L  1
#define OBS_PAD_R  1
#define OBS_PAD_T  1
#define OBS_PAD_B  1

// Debug: draw rectangles hitbox (0 = off, 1 = on)
#define DEBUG_HITBOX 0

// ------------------------------------------------------------
// FIXED POINT Q8.8  (SAFE: no shift of negatives)
#define FP_SHIFT 8
#define FP_ONE   (1 << FP_SHIFT)

// NB: TO_FP with multiplication => avoid UB on negatives
#define TO_FP_I32(x)     ((i32)(x) * (i32)FP_ONE)
#define FP_TO_I16(xfp)   ((i16)((i32)(xfp) / (i32)FP_ONE))  // robusto anche per negativi

// physics:
#define GRAVITY_FP   ((i16)77)
#define JUMP_VY_FP   ((i16)-1843)
#define VY_MAX_FP    ((i16)2048)

// Speed
#define SPEED_START_FP ((i16)350)
#define SPEED_MAX_FP   ((i16)1280)
#define SPEED_INC_DIV  300

// ------------------------------------------------------------
// clouds on backgrounds
#define CLOUD_COUNT          3
#define CLOUD_Y_MIN          10
#define CLOUD_Y_MAX          50
#define CLOUD_RESPAWN_PAD_X  12      // extra margin when it respawns
#define CLOUD_GAP_MIN        40
#define CLOUD_GAP_MAX        90
#define CLOUD_PARALLAX_DIV   3       // more higher = more slower clouds (speed/3)
#define CLOUD_IDLE_STEP_FP   (FP_ONE / 2) // 0.5 px/frame when you are in IDLE

typedef enum {
    IDLE = 0,
    PLAYING = 1,
    GAMEOVER = 2
} GameState;

typedef enum {
    OBS_SMALL_CACTUS = 0,
    OBS_BIG_CACTUS   = 1,
    OBS_BIRD         = 2
} ObsType;

typedef struct {
    u8  active;
    u8  type;      // ObsType in 1 byte
    i32 x_fp;      // Q8.8 (i32 necessary)
    u8  y;         // 0..127
    u8  w, h;      // small dimensions
} Obstacle;

typedef struct {
    i32 y_fp;      // Q8.8 (i32 for overflow's sicurity)
    i16 vy_fp;     // Q8.8
    u8  on_ground;
    u8  duck;
} Dino;

typedef struct {
    i32 x_fp;      // Q8.8
    u8  y;         // px
} Cloud;

// ------------------------------------------------------------
// Helpers

// ------------------------------------------------------------
// Joystick helper
// PC: WASD/ENTER in HOLD -> continuous input
// MSP432: uses get_joystick()
static inline joystick_t joystick_action(void) {
#ifdef SIMULATION_PC
    if (IsKeyDown(KEY_ENTER)) return JS_BUTTON;
    if (IsKeyDown(KEY_W))     return JS_UP;
    if (IsKeyDown(KEY_A))     return JS_LEFT;
    if (IsKeyDown(KEY_S))     return JS_DOWN;
    if (IsKeyDown(KEY_D))     return JS_RIGHT;
    return JS_NONE;
#else
    return get_joystick();
#endif
}



static inline i16 clamp_i16(i16 v, i16 lo, i16 hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static u8 count_digits_u32(u32 v) {
    if (v == 0) return 1;
    u8 n = 0;
    while (v > 0) { n++; v /= 10u; }
    return n;
}

// AABB: posizioni signed (i16), dimensioni u8
static u8 aabb_i16_u8(i16 ax, i16 ay, u8 aw, u8 ah,
                      i16 bx, i16 by, u8 bw, u8 bh) {
    i16 al = ax, ar = (i16)(ax + (i16)aw);
    i16 at = ay, ab = (i16)(ay + (i16)ah);

    i16 bl = bx, br = (i16)(bx + (i16)bw);
    i16 bt = by, bb = (i16)(by + (i16)bh);

    return (ar >= bl) && (al <= br) && (ab >= bt) && (at <= bb);
}

// ------------------------------------------------------------
// RNG

static u32 rng_state = 123456789u;

static u32 rng_u32(void) {
    rng_state = rng_state * 1664525u + 1013904223u;
    return rng_state;
}

static u8 rng_range_u8(u8 lo, u8 hi) {
    if (hi <= lo) return lo;
    u32 r = rng_u32();
    u32 span = (u32)(hi - lo) + 1u;
    return (u8)(lo + (u8)(r % span));
}

static u8 rng_chance_percent_u8(u8 pct) {
    u8 v = (u8)(rng_u32() % 100u);
    return (v < pct) ? 1 : 0;
}

// ------------------------------------------------------------
// 7-seg

static void draw_seg_digit(i32 x, i32 y, i32 s, i32 thick, int d, TWOS_COLOURS col) {
    static const u8 mask[10] = {
        0b1111110, // 0
        0b0110000, // 1
        0b1101101, // 2
        0b1111001, // 3
        0b0110011, // 4
        0b1011011, // 5
        0b1011111, // 6
        0b1110000, // 7
        0b1111111, // 8
        0b1111011  // 9
    };

    if (d < 0 || d > 9) return;
    u8 m = mask[d];

    i32 w = 3 * s;
    i32 h = 5 * s;

    if (m & (1<<6)) draw_rectangle(x + thick, y, w - 2*thick, thick, col);
    if (m & (1<<5)) draw_rectangle(x + w - thick, y + thick, thick, (h/2) - thick, col);
    if (m & (1<<4)) draw_rectangle(x + w - thick, y + (h/2), thick, (h/2) - thick, col);
    if (m & (1<<3)) draw_rectangle(x + thick, y + h - thick, w - 2*thick, thick, col);
    if (m & (1<<2)) draw_rectangle(x, y + (h/2), thick, (h/2) - thick, col);
    if (m & (1<<1)) draw_rectangle(x, y + thick, thick, (h/2) - thick, col);
    if (m & (1<<0)) draw_rectangle(x + thick, y + (h/2) - (thick/2), w - 2*thick, thick, col);
}

static void draw_int_7seg(i32 x, i32 y, i32 s, i32 thick, u32 value, TWOS_COLOURS col) {
    int digits[10];
    int n = 0;

    if (value == 0) digits[n++] = 0;
    else {
        while (value > 0 && n < 10) {
            digits[n++] = (int)(value % 10u);
            value /= 10u;
        }
    }
    int i;
    for (i = n - 1; i >= 0; --i) {
        draw_seg_digit(x, y, s, thick, digits[i], col);
        x += (3*s + s);
    }
}

// ------------------------------------------------------------
// Obstacles

static void clear_obstacles(Obstacle obs[MAX_OBS]) {
    u8 i;
    for (i = 0; i < MAX_OBS; i++) obs[i].active = 0;
}

static void spawn_obstacle(Obstacle obs[MAX_OBS], u8 duck_h) {
    i8 idx = -1;
    u8 i;
    for (i = 0; i < MAX_OBS; i++) {
        if (!obs[i].active) { idx = (i8)i; break; }
    }
    if (idx < 0) return;

    Obstacle* o = &obs[(u8)idx];
    o->active = 1;

    // choice for obstacle: 70% cactus (small/big), 30% bird
    u8 r = (u8)(rng_u32() % 100u);

    if (r < 70) {
        // cactus
        if (rng_chance_percent_u8(55)) {
            o->type = (u8)OBS_SMALL_CACTUS;
            o->w = small_cactus_sprite.width;
            o->h = small_cactus_sprite.height;
        } else {
            o->type = (u8)OBS_BIG_CACTUS;
            o->w = big_cactus_sprite.width;
            o->h = big_cactus_sprite.height;
        }

        o->x_fp = TO_FP_I32(LCD_W + 2);
        o->y = (u8)(GROUND_Y - (i16)o->h);

    } else {
        // bird
        o->type = (u8)OBS_BIRD;
        o->w = bird_sprite.width;
        o->h = bird_sprite.height;
        o->x_fp = TO_FP_I32(LCD_W + 2);

        const i16 margin = 2;
        i16 gap_ok = (i16)duck_h + margin;

        i16 y_duck = (i16)GROUND_Y - (gap_ok + (i16)o->h);
        i16 y_high = (i16)(y_duck - 10);

        if (y_high < 0) y_high = 0;
        if (y_duck < 0) y_duck = 0;

        if (rng_chance_percent_u8(60)) o->y = (u8)y_duck;
        else                          o->y = (u8)y_high;
    }
}

static void despawn_if_offscreen(Obstacle* o) {
    if (!o->active) return;
    i32 right_fp = o->x_fp + ((i32)o->w * (i32)FP_ONE);
    if (right_fp < TO_FP_I32(-2)) o->active = 0;
}

// ------------------------------------------------------------
// Clouds

static void init_clouds(Cloud c[CLOUD_COUNT]) {
    i16 x = 10;
    u8 i;
    for (i = 0; i < CLOUD_COUNT; i++) {
        u8 y = rng_range_u8((u8)CLOUD_Y_MIN, (u8)CLOUD_Y_MAX);
        u8 gap = rng_range_u8((u8)CLOUD_GAP_MIN, (u8)CLOUD_GAP_MAX);
        c[i].x_fp = TO_FP_I32(x);
        c[i].y = y;
        x += (i16)(cloud_sprite.width + gap);
    }
}

static void update_clouds(Cloud c[CLOUD_COUNT], i16 bg_step_fp) {
    // bg_step_fp = how much the background "flows" (Q8.8)
    u8 i;
    for (i = 0; i < CLOUD_COUNT; i++) {
        c[i].x_fp -= (i32)bg_step_fp;

        // if it exits on the left, it respawns on the right with a random gap
        if (c[i].x_fp + TO_FP_I32((i16)cloud_sprite.width) < TO_FP_I32(-2)) {
            // find actual x_max
            i32 max_x = c[0].x_fp;
            u8 k;
            for (k = 1; k < CLOUD_COUNT; k++) {
                if (c[k].x_fp > max_x) max_x = c[k].x_fp;
            }

            u8 gap = rng_range_u8((u8)CLOUD_GAP_MIN, (u8)CLOUD_GAP_MAX);
            c[i].x_fp = max_x + TO_FP_I32((i16)cloud_sprite.width + (i16)gap + (i16)CLOUD_RESPAWN_PAD_X);
            c[i].y = rng_range_u8((u8)CLOUD_Y_MIN, (u8)CLOUD_Y_MAX);
        }
    }
}

// ------------------------------------------------------------
// GAME

int dino_runner_game(void) {
    display_init_lcd();

    set_palette(DINO_CUSTOM_PALETTE_INDEX);
    set_screen_color(T_ONE); // sabbia

    // --- Dino sprites (BW) ---
    TextureHandle dino_idle_jump_tex = load_texture_from_sprite_p(
        dino_state1_sprite.height, dino_state1_sprite.width, dino_state1_sprite.data,
        BW_INDEX
    );
    TextureHandle dino_duck_tex = load_texture_from_sprite_p(
        dino_state2_sprite.height, dino_state2_sprite.width, dino_state2_sprite.data,
        BW_INDEX
    );
    TextureHandle dino_walk_a_tex = load_texture_from_sprite_p(
        dino_state3_sprite.height, dino_state3_sprite.width, dino_state3_sprite.data,
        BW_INDEX
    );
    TextureHandle dino_walk_b_tex = load_texture_from_sprite_p(
        dino_state4_sprite.height, dino_state4_sprite.width, dino_state4_sprite.data,
        BW_INDEX
    );

    // --- Obstacles sprites ---
    TextureHandle small_cactus_tex = load_texture_from_sprite_p(
        small_cactus_sprite.height, small_cactus_sprite.width, small_cactus_sprite.data,
        DINO_CUSTOM_PALETTE_INDEX
    );
    TextureHandle big_cactus_tex = load_texture_from_sprite_p(
        big_cactus_sprite.height, big_cactus_sprite.width, big_cactus_sprite.data,
        DINO_CUSTOM_PALETTE_INDEX
    );
    TextureHandle bird_tex = load_texture_from_sprite_p(
        bird_sprite.height, bird_sprite.width, bird_sprite.data,
        DINO_CUSTOM_PALETTE_INDEX
    );

    // --- Clouds sprite ---
    TextureHandle cloud_tex = load_texture_from_sprite_p(
        cloud_sprite.height, cloud_sprite.width, cloud_sprite.data,
        BW_INDEX
    );

    // Hitbox base
    const u8 DINO_RUN_W  = dino_state1_sprite.width;
    const u8 DINO_RUN_H  = dino_state1_sprite.height;
    const u8 DINO_DUCK_W = dino_state2_sprite.width;
    const u8 DINO_DUCK_H = dino_state2_sprite.height;

    u8 state = (u8)IDLE;

    Dino dino;
    dino.y_fp = TO_FP_I32(GROUND_Y - (i16)DINO_RUN_H);
    dino.vy_fp = 0;
    dino.on_ground = 1;
    dino.duck = 0;

    Obstacle obs[MAX_OBS];
    clear_obstacles(obs);

    // clouds
    Cloud clouds[CLOUD_COUNT];
    init_clouds(clouds);

    u32 score = 0;

    i16 speed_fp = SPEED_START_FP;
    u8  spawn_cd = rng_range_u8(SPAWN_MIN_FR, SPAWN_MAX_FR);


    u8 jump_cd = 0;

    // walk anim
    u8 walk_tick = 0;
    u8 walk_frame = 0;

    u8 i;
    while (!window_should_close()) {
        display_begin();

        // -------- INPUT (joystick)
        joystick_t a = joystick_action();

        // HOLD behavior:
        // - UP = jump
        // - DOWN = duck
        u8 jump = (a == JS_UP || a == JS_BUTTON) ? 1u : 0u;
        u8 duck = (a == JS_DOWN) ? 1u : 0u;


        // -------- UPDATE
        if (state == (u8)IDLE) {
            walk_tick = 0;
            walk_frame = 0;

            // slow clouds in IDLE
            update_clouds(clouds, (i16)CLOUD_IDLE_STEP_FP);

            if (a != JS_NONE || jump) {

                state = (u8)PLAYING;
                score = 0;
                speed_fp = SPEED_START_FP;
                spawn_cd = rng_range_u8(SPAWN_MIN_FR, SPAWN_MAX_FR);

                dino.duck = 0;
                dino.on_ground = 1;
                dino.vy_fp = 0;
                dino.y_fp = TO_FP_I32(GROUND_Y - (i16)DINO_RUN_H);

                clear_obstacles(obs);
                jump_cd = 0;
            }
        }
        else if (state == (u8)PLAYING) {
            if (jump_cd > 0) jump_cd--;

            dino.duck = (duck && dino.on_ground) ? 1u : 0u;

            u8 cur_w = dino.duck ? DINO_DUCK_W : DINO_RUN_W;
            u8 cur_h = dino.duck ? DINO_DUCK_H : DINO_RUN_H;

            if (dino.on_ground) {
                dino.y_fp = TO_FP_I32(GROUND_Y - (i16)cur_h);
            }

            // jump
            if (jump && dino.on_ground && (jump_cd == 0)) {
                dino.vy_fp = JUMP_VY_FP;
                dino.on_ground = 0;
                jump_cd = (u8)JUMP_COOLDOWN_FR;
            }

            // physics
            dino.vy_fp = (i16)(dino.vy_fp + GRAVITY_FP);
            dino.vy_fp = clamp_i16(dino.vy_fp, (i16)-32760, VY_MAX_FP);
            dino.y_fp  = (i32)(dino.y_fp + (i32)dino.vy_fp);

            i32 ground_top_fp = TO_FP_I32(GROUND_Y - (i16)cur_h);
            if (dino.y_fp >= ground_top_fp) {
                dino.y_fp = ground_top_fp;
                dino.vy_fp = 0;
                dino.on_ground = 1;
            }

            // walk anim only when it runs
            if (dino.on_ground && !dino.duck) {
                walk_tick++;
                if (walk_tick >= (u8)WALK_ANIM_PERIOD) {
                    walk_tick = 0;
                    walk_frame ^= 1u;
                }
            } else {
                walk_tick = 0;
                walk_frame = 0;
            }

            // speed + score
            speed_fp = (i16)(speed_fp + (i16)(speed_fp / SPEED_INC_DIV));
            if (speed_fp > SPEED_MAX_FP) speed_fp = SPEED_MAX_FP;
            score += 1u;

            // cloude (slower than obstacles)
            i16 cloud_step_fp = (i16)(speed_fp / (i16)CLOUD_PARALLAX_DIV);
            if (cloud_step_fp < 1) cloud_step_fp = 1;
            update_clouds(clouds, cloud_step_fp);

            // spawn
            if (spawn_cd > 0) spawn_cd--;
            if (spawn_cd == 0) {
                spawn_obstacle(obs, DINO_DUCK_H);

                i16 delta_fp = (i16)(speed_fp - SPEED_START_FP);
                // approx in px (Q8.8 -> int)
                i16 delta_px = (i16)((delta_fp + (FP_ONE/2)) >> FP_SHIFT);

                i16 min_fr = (i16)SPAWN_MIN_FR - (i16)(delta_px * 3);
                i16 max_fr = (i16)SPAWN_MAX_FR - (i16)(delta_px * 4);

                if (min_fr < 30) min_fr = 30;
                if (max_fr < 55) max_fr = 55;
                if (max_fr < min_fr) max_fr = min_fr;

                spawn_cd = rng_range_u8((u8)min_fr, (u8)max_fr);
            }

            // collision + move obstacles
            i16 dino_yi = FP_TO_I16(dino.y_fp);

            i16 dino_x_hit = (i16)DINO_X + (i16)HIT_PAD_L;
            i16 dino_y_hit = dino_yi + (i16)HIT_PAD_T;

            u8 dino_w_hit = cur_w;
            u8 dino_h_hit = cur_h;

            if (dino_w_hit > (u8)(HIT_PAD_L + HIT_PAD_R)) dino_w_hit = (u8)(dino_w_hit - (HIT_PAD_L + HIT_PAD_R));
            if (dino_h_hit > (u8)(HIT_PAD_T + HIT_PAD_B)) dino_h_hit = (u8)(dino_h_hit - (HIT_PAD_T + HIT_PAD_B));
            for (i = 0; i < MAX_OBS; i++) {
                if (!obs[i].active) continue;

                obs[i].x_fp -= (i32)speed_fp;
                despawn_if_offscreen(&obs[i]);
                if (!obs[i].active) continue;

                i16 ox = FP_TO_I16(obs[i].x_fp);
                i16 oy = (i16)obs[i].y;

                i16 ox_hit = (i16)(ox + (i16)OBS_PAD_L);
                i16 oy_hit = (i16)(oy + (i16)OBS_PAD_T);

                u8 ow_hit = obs[i].w;
                u8 oh_hit = obs[i].h;

                if (ow_hit > (u8)(OBS_PAD_L + OBS_PAD_R)) ow_hit = (u8)(ow_hit - (OBS_PAD_L + OBS_PAD_R));
                if (oh_hit > (u8)(OBS_PAD_T + OBS_PAD_B)) oh_hit = (u8)(oh_hit - (OBS_PAD_T + OBS_PAD_B));

#if DEBUG_HITBOX
                draw_rectangle_outline((i32)dino_x_hit, (i32)dino_y_hit, (i32)dino_w_hit, (i32)dino_h_hit, 1.0f, T_THREE);
                draw_rectangle_outline((i32)ox_hit, (i32)oy_hit, (i32)ow_hit, (i32)oh_hit, 1.0f, T_THREE);
#endif

                if (aabb_i16_u8(dino_x_hit, dino_y_hit, dino_w_hit, dino_h_hit,
                                ox_hit, oy_hit, ow_hit, oh_hit)) {
                    display_end();
                    display_close();
                    return (int)score;
                }
            }
        }

        // -------- DRAW
        clear_screen();

        // draw clouds

        for (i = 0; i < CLOUD_COUNT; i++) {
            i16 cx = FP_TO_I16(clouds[i].x_fp);
            if (cx > -40 && cx < 220) { // piccolo culling grezzo
                draw_texture((u8)cx, clouds[i].y, cloud_tex);
            }
        }

        draw_rectangle(0, GROUND_Y, LCD_W, 20, T_TWO);

        // texture dino
        TextureHandle dino_tex = dino_idle_jump_tex;
        if (state == (u8)IDLE) {
            dino_tex = dino_idle_jump_tex; // state1
        } else {
            if (!dino.on_ground) dino_tex = dino_idle_jump_tex;      // jump -> state1
            else if (dino.duck)  dino_tex = dino_duck_tex;           // duck -> state2
            else                 dino_tex = (walk_frame == 0) ? dino_walk_a_tex : dino_walk_b_tex; // walk -> 3/4
        }

        {
            i16 dy = FP_TO_I16(dino.y_fp);
            if (dy < 0) dy = 0;
            if (dy > 255) dy = 255;
            draw_texture((u8)DINO_X, (u8)dy, dino_tex);
        }

        // draw obstacles as textures
        for (i = 0; i < MAX_OBS; i++) {
            if (!obs[i].active) continue;

            i16 ox = FP_TO_I16(obs[i].x_fp);
            u8  oy = obs[i].y;

            TextureHandle t = small_cactus_tex;
            if (obs[i].type == (u8)OBS_BIG_CACTUS) t = big_cactus_tex;
            else if (obs[i].type == (u8)OBS_BIRD)  t = bird_tex;

            draw_texture((u8)ox, oy, t);
        }

        // score
        if (state == (u8)PLAYING) {
            i32 s = 3, thick = 2;
            u8 digits = count_digits_u32(score);
            i32 digit_w = 3 * s;
            i32 gap = s;
            i32 total_w = (i32)digits * digit_w + ((i32)digits - 1) * gap;

            i32 margin_r = 2;
            i32 score_x = LCD_W - margin_r - total_w;
            i32 score_y = 2;

            draw_int_7seg(score_x, score_y, s, thick, score, T_THREE); // nero
        }

        display_end();
    }

    display_close();
    return (int)score;
}
