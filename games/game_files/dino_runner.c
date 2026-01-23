// game_files/dino_runner.c
// Dino Runner - usa dino_state1 (run) e dino_state2 (duck)
// Mondo: DINO_CUSTOM_PALETTE_INDEX (T_ONE sabbia, T_TWO verde, T_THREE nero)
// Dino: caricato in BW_INDEX (stabile/visibile)
// Score/UI: usa la palette corrente (quindi T_THREE = nero vero)

#include <stdio.h>

#include "display/display.h"
#include "sprites/sprites.h"
#include "sprites/palettes.h"
#include "stddef.h"

// ------------------------------------------------------------
// LCD size
#define LCD_W 160
#define LCD_H 128

// Start condition
#define PROX_START_DELTA 8.0f

// Jump gesture (delta frame-to-frame)
#define PROX_JUMP_DELTA  28.0f

// Duck threshold
#define PROX_DUCK_TH     750.0f

// Ground
#define GROUND_Y      (LCD_H - 18)

// Dino position (x fisso)
#define DINO_X        22

// Physics (frame-based)
#define GRAVITY       0.30f
#define JUMP_VY      -7.2f
#define VY_MAX        8.0f

// Speed
#define SPEED_START   1.30f
#define SPEED_MUL     1.002f
#define SPEED_MAX     4.50f

// Obstacles
#define MAX_OBS       6
#define SPAWN_MIN_FR  48
#define SPAWN_MAX_FR  95

// Jump cooldown
#define JUMP_COOLDOWN_FR 10

typedef enum {
    IDLE = 0,
    PLAYING = 1,
    GAMEOVER = 2
} GameState;

typedef enum {
    OBS_CACTUS = 0,
    OBS_BIRD   = 1
} ObsType;

// ------------------------------------------------------------
// Struct ottimizzati

typedef struct {
    u8 active;
    u8 type;      // ObsType in 1 byte
    f32 x;        // può diventare negativo
    u8  y;        // 0..127
    u8  w, h;     // dimensioni piccole
} Obstacle;

typedef struct {
    f32 y;
    f32 vy;
    u8  on_ground;
    u8  duck;
} Dino;

// ------------------------------------------------------------
// Helpers

static inline f32 clampf(f32 v, f32 lo, f32 hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}
static inline f32 absf(f32 v) { return (v < 0.0f) ? -v : v; }

static u8 count_digits_u32(u32 v) {
    if (v == 0) return 1;
    u8 n = 0;
    while (v > 0) { n++; v /= 10u; }
    return n;
}

// AABB: posizioni signed (i16) perché x può essere negativa, dimensioni u8
static u8 aabb_i16_u8(i16 ax, i16 ay, u8 aw, u8 ah,
                      i16 bx, i16 by, u8 bw, u8 bh) {
    i32 al = ax, ar = (i32)ax + (i32)aw;
    i32 at = ay, ab = (i32)ay + (i32)ah;

    i32 bl = bx, br = (i32)bx + (i32)bw;
    i32 bt = by, bb = (i32)by + (i32)bh;

    return (ar >= bl) && (al <= br) && (ab >= bt) && (at <= bb);
}

// ------------------------------------------------------------
// Simple RNG

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
// 7-seg digits (usa la palette CORRENTE)

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

    for (int i = n - 1; i >= 0; --i) {
        draw_seg_digit(x, y, s, thick, digits[i], col);
        x += (3*s + s);
    }
}

// ------------------------------------------------------------
// Game helpers

static void clear_obstacles(Obstacle obs[MAX_OBS]) {
    for (u8 i = 0; i < MAX_OBS; i++) obs[i].active = 0;
}

static void spawn_obstacle(Obstacle obs[MAX_OBS], u8 duck_h) {
    i8 idx = -1;
    for (u8 i = 0; i < MAX_OBS; i++) {
        if (!obs[i].active) { idx = (i8)i; break; }
    }
    if (idx < 0) return;

    Obstacle* o = &obs[(u8)idx];
    o->active = 1;

    if (rng_chance_percent_u8(78)) {
        // -------- CACTUS
        o->type = (u8)OBS_CACTUS;
        o->w = rng_range_u8(6, 16);
        o->h = rng_range_u8(10, 20);
        o->x = (f32)(LCD_W + 2);
        o->y = (u8)(GROUND_Y - (i32)o->h);
    } else {
        // -------- BIRD
        o->type = (u8)OBS_BIRD;
        o->w = 14;
        o->h = 8;
        o->x = (f32)(LCD_W + 2);

        const i16 margin = 2;
        i16 gap_ok = (i16)duck_h + margin;

        i16 y_duck = (i16)GROUND_Y - (gap_ok + (i16)o->h);
        i16 y_high = y_duck - 10;

        if (y_high < 0) y_high = 0;
        if (y_duck < 0) y_duck = 0;

        if (rng_chance_percent_u8(60)) o->y = (u8)y_duck;
        else                          o->y = (u8)y_high;
    }
}

static void despawn_if_offscreen(Obstacle* o) {
    if (o->active && (o->x + (f32)o->w < -2.0f)) o->active = 0;
}

// ------------------------------------------------------------
// GAME

int dino_runner_game(void) {
    display_init_lcd();

    // Mondo con palette dinosauro
    set_palette(DINO_CUSTOM_PALETTE_INDEX);
    set_screen_color(T_ONE); // sabbia

    // Dino caricato in BW (così non sparisce sulla sabbia)
    TextureHandle dino_run_tex  = load_texture_from_sprite_p(
        dino_state1_sprite.height, dino_state1_sprite.width, dino_state1_sprite.data,
        BW_INDEX
    );
    TextureHandle dino_duck_tex = load_texture_from_sprite_p(
        dino_state2_sprite.height, dino_state2_sprite.width, dino_state2_sprite.data,
        BW_INDEX
    );

    // Dimensioni reali (hitbox = sprite)
    const u8 DINO_RUN_W  = dino_state1_sprite.width;
    const u8 DINO_RUN_H  = dino_state1_sprite.height;
    const u8 DINO_DUCK_W = dino_state2_sprite.width;
    const u8 DINO_DUCK_H = dino_state2_sprite.height;

    u8 state = (u8)IDLE;

    Dino dino;
    dino.y = (f32)(GROUND_Y - (i32)DINO_RUN_H);
    dino.vy = 0.0f;
    dino.on_ground = 1;
    dino.duck = 0;

    Obstacle obs[MAX_OBS];
    clear_obstacles(obs);

    u32 score = 0;

    f32 speed = SPEED_START;
    u8 spawn_cd = rng_range_u8(SPAWN_MIN_FR, SPAWN_MAX_FR);

    f32 proximity = get_proximity();
    f32 prox0 = proximity;
    f32 prox_prev = proximity;

    u8 jump_cd = 0;

    while (!window_should_close()) {
        display_begin();

        // -------- INPUT
        proximity = get_proximity();
        f32 dprox = absf(proximity - prox_prev);
        prox_prev = proximity;

        u8 jump = (dprox >= PROX_JUMP_DELTA) ? 1 : 0;
        u8 duck = (proximity >= PROX_DUCK_TH) ? 1 : 0;

        // -------- UPDATE
        if (state == (u8)IDLE) {
            if (absf(proximity - prox0) >= PROX_START_DELTA || jump) {
                state = (u8)PLAYING;
                score = 0;
                speed = SPEED_START;
                spawn_cd = rng_range_u8(SPAWN_MIN_FR, SPAWN_MAX_FR);

                dino.duck = 0;
                dino.on_ground = 1;
                dino.vy = 0.0f;
                dino.y = (f32)(GROUND_Y - (i32)DINO_RUN_H);

                clear_obstacles(obs);
                jump_cd = 0;
            }
        }
        else if (state == (u8)PLAYING) {
            if (jump_cd > 0) jump_cd--;

            dino.duck = (duck && dino.on_ground) ? 1 : 0;

            u8 cur_w = dino.duck ? DINO_DUCK_W : DINO_RUN_W;
            u8 cur_h = dino.duck ? DINO_DUCK_H : DINO_RUN_H;

            if (dino.on_ground) {
                dino.y = (f32)(GROUND_Y - (i32)cur_h);
            }

            if (jump && dino.on_ground && jump_cd == 0) {
                dino.vy = JUMP_VY;
                dino.on_ground = 0;
                jump_cd = (u8)JUMP_COOLDOWN_FR;
            }

            // physics
            dino.vy += GRAVITY;
            dino.vy = clampf(dino.vy, -50.0f, VY_MAX);
            dino.y  += dino.vy;

            // clamp pavimento
            i16 ground_top_i = (i16)GROUND_Y - (i16)cur_h;
            if (dino.y >= (f32)ground_top_i) {
                dino.y = (f32)ground_top_i;
                dino.vy = 0.0f;
                dino.on_ground = 1;
            }

            // speed up
            speed *= SPEED_MUL;
            if (speed > SPEED_MAX) speed = SPEED_MAX;

            // score
            score += 1;

            // spawn
            if (spawn_cd > 0) spawn_cd--;
            if (spawn_cd == 0) {
                spawn_obstacle(obs, DINO_DUCK_H);

                i16 min_fr = (i16)SPAWN_MIN_FR - (i16)((speed - SPEED_START) * 3.0f);
                i16 max_fr = (i16)SPAWN_MAX_FR - (i16)((speed - SPEED_START) * 4.0f);
                if (min_fr < 30) min_fr = 30;
                if (max_fr < 55) max_fr = 55;
                if (max_fr < min_fr) max_fr = min_fr;

                spawn_cd = rng_range_u8((u8)min_fr, (u8)max_fr);
            }

            // move obstacles + collision
            i16 dino_yi = (i16)(i32)dino.y;

            for (u8 i = 0; i < MAX_OBS; i++) {
                if (!obs[i].active) continue;

                obs[i].x -= speed;
                despawn_if_offscreen(&obs[i]);

                if (obs[i].active) {
                    i16 ox = (i16)(i32)obs[i].x;
                    i16 oy = (i16)obs[i].y;

                    if (aabb_i16_u8((i16)DINO_X, dino_yi, cur_w, cur_h,
                                    ox, oy, obs[i].w, obs[i].h)) {
                        // Niente schermata gameover: esco subito
                        display_end();
                        display_close();
                        return (int)score;
                    }
                }
            }
        }

        // -------- DRAW (solo IDLE/PLAYING)
        clear_screen();

        // ground verde
        draw_rectangle(0, GROUND_Y, LCD_W, 20, T_TWO);

        // dino sprite
        {
            i16 dy = (i16)(i32)dino.y;
            if (dy < 0) dy = 0;
            if (dy > 255) dy = 255;

            if (state == (u8)PLAYING && dino.duck) {
                draw_texture((u8)DINO_X, (u8)dy, dino_duck_tex);
            } else {
                draw_texture((u8)DINO_X, (u8)dy, dino_run_tex);
            }
        }

        // obstacles verdi
        for (u8 i = 0; i < MAX_OBS; i++) {
            if (!obs[i].active) continue;
            draw_rectangle((i32)obs[i].x, (i32)obs[i].y, (i32)obs[i].w, (i32)obs[i].h, T_TWO);
        }

        // score top-right (nero)
        if (state == (u8)PLAYING) {
            i32 s = 3, thick = 2;
            u8 digits = count_digits_u32(score);
            i32 digit_w = 3 * s;
            i32 gap = s;
            i32 total_w = (i32)digits * digit_w + ((i32)digits - 1) * gap;

            i32 margin_r = 2;
            i32 score_x = LCD_W - margin_r - total_w;
            i32 score_y = 2;

            draw_int_7seg(score_x, score_y, s, thick, score, T_THREE);
        }

        display_end();
    }

    display_close();
    return (int)score;
}

// ------------------------------------------------------------
// MAIN DI TEST

int main(void) {
    int score = dino_runner_game();
    printf("Score: %d\n", score);
    return score;
}
