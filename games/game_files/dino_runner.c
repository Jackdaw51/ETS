// game_files/dino_runner.c

#include <stdio.h>

#include "display/display.h"
#include "sprites/palettes.h"
#include "stddef.h"

// ------------------------------------------------------------
// LCD size
#define LCD_W 160
#define LCD_H 128

// Proximity range
#define PROX_MAX 1023.0f

// Start condition (come pong)
#define PROX_START_DELTA 8.0f

// Jump gesture: variazione rapida di proximity (tuning)
#define PROX_JUMP_DELTA  18.0f

// Duck: mano molto vicina (tuning)
#define PROX_DUCK_TH     750.0f

// Ground
#define GROUND_Y      (LCD_H - 18)

// Dino
#define DINO_X        22
#define DINO_W        10
#define DINO_H        14
#define DINO_DUCK_H    9

// Physics (frame-based)
#define GRAVITY       0.35f   // px/frame^2
#define JUMP_VY      -6.5f    // px/frame
#define VY_MAX        8.0f

// Speed (px/frame)
#define SPEED_START   2.0f
#define SPEED_MUL     1.004f
#define SPEED_MAX     7.0f

// Obstacles
#define MAX_OBS       6
#define SPAWN_MIN_FR  42
#define SPAWN_MAX_FR  85

// Gameover
#define GAMEOVER_FRAMES 90

typedef enum {
    IDLE = 0,
    PLAYING = 1,
    GAMEOVER = 2
} GameState;

typedef enum {
    OBS_CACTUS = 0,
    OBS_BIRD   = 1
} ObsType;

typedef struct {
    u8 active;
    ObsType type;
    f32 x, y;
    i32 w, h;
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

static i32 count_digits_i32(i32 v) {
    if (v <= 0) return 1;
    i32 n = 0;
    while (v > 0) { n++; v /= 10; }
    return n;
}

static u8 aabb_i32(i32 ax, i32 ay, i32 aw, i32 ah,
                   i32 bx, i32 by, i32 bw, i32 bh) {
    i32 al = ax, ar = ax + aw;
    i32 at = ay, ab = ay + ah;

    i32 bl = bx, br = bx + bw;
    i32 bt = by, bb = by + bh;

    return (ar >= bl) && (al <= br) && (ab >= bt) && (at <= bb);
}

// ------------------------------------------------------------
// Simple RNG (no stdlib)

static u32 rng_state = 123456789u;

static u32 rng_u32(void) {
    rng_state = rng_state * 1664525u + 1013904223u;
    return rng_state;
}

static i32 rng_range_i32(i32 lo, i32 hi) {
    if (hi <= lo) return lo;
    u32 r = rng_u32();
    i32 span = (hi - lo) + 1;
    return lo + (i32)(r % (u32)span);
}

static u8 rng_chance_percent(i32 pct) {
    i32 v = (i32)(rng_u32() % 100u);
    return (v < pct) ? 1 : 0;
}

// ------------------------------------------------------------
// 7-seg digits (stesso del pong)

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

    if (m & (1<<6)) draw_rectangle_p(x + thick, y, w - 2*thick, thick, col, BW_INDEX);                 // A
    if (m & (1<<5)) draw_rectangle_p(x + w - thick, y + thick, thick, (h/2) - thick, col, BW_INDEX);  // B
    if (m & (1<<4)) draw_rectangle_p(x + w - thick, y + (h/2), thick, (h/2) - thick, col, BW_INDEX);  // C
    if (m & (1<<3)) draw_rectangle_p(x + thick, y + h - thick, w - 2*thick, thick, col, BW_INDEX);     // D
    if (m & (1<<2)) draw_rectangle_p(x, y + (h/2), thick, (h/2) - thick, col, BW_INDEX);               // E
    if (m & (1<<1)) draw_rectangle_p(x, y + thick, thick, (h/2) - thick, col, BW_INDEX);               // F
    if (m & (1<<0)) draw_rectangle_p(x + thick, y + (h/2) - (thick/2), w - 2*thick, thick, col, BW_INDEX); // G
}

static void draw_int_7seg(i32 x, i32 y, i32 s, i32 thick, i32 value, TWOS_COLOURS col) {
    if (value < 0) value = 0;

    int digits[8];
    int n = 0;

    if (value == 0) digits[n++] = 0;
    else {
        while (value > 0 && n < 8) {
            digits[n++] = value % 10;
            value /= 10;
        }
    }

    for (int i = n - 1; i >= 0; --i) {
        draw_seg_digit(x, y, s, thick, digits[i], col);
        x += (3*s + s);
    }
}

// ------------------------------------------------------------
// Game helpers

static void reset_dino(Dino* d) {
    d->duck = 0;
    d->on_ground = 1;
    d->vy = 0.0f;
    d->y = (f32)(GROUND_Y - DINO_H);
}

static void clear_obstacles(Obstacle obs[MAX_OBS]) {
    for (int i = 0; i < MAX_OBS; i++) obs[i].active = 0;
}

static void spawn_obstacle(Obstacle obs[MAX_OBS]) {
    int idx = -1;
    for (int i = 0; i < MAX_OBS; i++) {
        if (!obs[i].active) { idx = i; break; }
    }
    if (idx < 0) return;

    Obstacle* o = &obs[idx];
    o->active = 1;

    if (rng_chance_percent(78)) {
        o->type = OBS_CACTUS;
        o->w = rng_range_i32(6, 16);
        o->h = rng_range_i32(10, 20);
        o->x = (f32)(LCD_W + 2);
        o->y = (f32)(GROUND_Y - o->h);
    } else {
        o->type = OBS_BIRD;
        o->w = 14;
        o->h = 8;
        o->x = (f32)(LCD_W + 2);

        // due altezze
        if (rng_chance_percent(50)) o->y = (f32)(GROUND_Y - 18);
        else                        o->y = (f32)(GROUND_Y - 26);
    }
}

static void despawn_if_offscreen(Obstacle* o) {
    if (o->active && (o->x + (f32)o->w < -2.0f)) o->active = 0;
}

// ------------------------------------------------------------
// GAME

int dino_runner_game(void) {
    display_init_lcd();
    set_palette(PONG_CUSTOM_PALETTE_INDEX);  // cambia se vuoi
    set_screen_color(T_ONE);

    GameState state = IDLE;

    Dino dino;
    reset_dino(&dino);

    Obstacle obs[MAX_OBS];
    clear_obstacles(obs);

    i32 score = 0;

    f32 speed = SPEED_START;
    i32 spawn_cd = rng_range_i32(SPAWN_MIN_FR, SPAWN_MAX_FR);

    f32 proximity = get_proximity();
    f32 prox0 = proximity;
    f32 prox_prev = proximity;

    i32 gameover_cd = 0;

    while (!window_should_close()) {
        display_begin();

        // -------- INPUT
        proximity = get_proximity();
        f32 dprox = absf(proximity - prox_prev);
        prox_prev = proximity;

        u8 jump = (dprox >= PROX_JUMP_DELTA) ? 1 : 0;
        u8 duck = (proximity >= PROX_DUCK_TH) ? 1 : 0;

        // -------- UPDATE
        if (state == IDLE) {
            if (absf(proximity - prox0) >= PROX_START_DELTA || jump) {
                state = PLAYING;
                score = 0;
                speed = SPEED_START;
                spawn_cd = rng_range_i32(SPAWN_MIN_FR, SPAWN_MAX_FR);

                reset_dino(&dino);
                clear_obstacles(obs);
            }
        }
        else if (state == PLAYING) {
            // duck solo a terra
            dino.duck = (duck && dino.on_ground) ? 1 : 0;

            // jump solo se a terra
            if (jump && dino.on_ground) {
                dino.vy = JUMP_VY;
                dino.on_ground = 0;
            }

            // fisica
            dino.vy += GRAVITY;
            dino.vy = clampf(dino.vy, -50.0f, VY_MAX);
            dino.y  += dino.vy;

            i32 dino_h = dino.duck ? DINO_DUCK_H : DINO_H;
            f32 ground_top = (f32)(GROUND_Y - dino_h);

            if (dino.y >= ground_top) {
                dino.y = ground_top;
                dino.vy = 0.0f;
                dino.on_ground = 1;
            }

            // speed up
            speed *= SPEED_MUL;
            if (speed > SPEED_MAX) speed = SPEED_MAX;

            // score
            score += 1;

            // spawn
            spawn_cd--;
            if (spawn_cd <= 0) {
                spawn_obstacle(obs);

                i32 min_fr = SPAWN_MIN_FR - (i32)((speed - SPEED_START) * 3.0f);
                i32 max_fr = SPAWN_MAX_FR - (i32)((speed - SPEED_START) * 4.0f);
                if (min_fr < 26) min_fr = 26;
                if (max_fr < 45) max_fr = 45;

                spawn_cd = rng_range_i32(min_fr, max_fr);
            }

            // move obstacles + collision
            i32 dino_yi = (i32)dino.y;
            i32 dino_hi = dino.duck ? DINO_DUCK_H : DINO_H;

            for (int i = 0; i < MAX_OBS; i++) {
                if (!obs[i].active) continue;

                obs[i].x -= speed;
                despawn_if_offscreen(&obs[i]);

                if (obs[i].active) {
                    if (aabb_i32(DINO_X, dino_yi, DINO_W, dino_hi,
                                 (i32)obs[i].x, (i32)obs[i].y, obs[i].w, obs[i].h)) {
                        state = GAMEOVER;
                        gameover_cd = GAMEOVER_FRAMES;
                        break;
                    }
                }
            }
        }
        else { // GAMEOVER
            if (gameover_cd > 0) gameover_cd--;
            else {
                display_end();
                break;
            }
        }

        // -------- DRAW
        clear_screen();

        // ground
        draw_rectangle(0, GROUND_Y, LCD_W, 2, T_THREE);

        // dino
        {
            i32 dh = (state == PLAYING && dino.duck) ? DINO_DUCK_H : DINO_H;
            if (state == IDLE) dh = DINO_H;
            draw_rectangle(DINO_X, (i32)dino.y, DINO_W, dh, T_TWO);
        }

        // obstacles
        for (int i = 0; i < MAX_OBS; i++) {
            if (!obs[i].active) continue;
            draw_rectangle((i32)obs[i].x, (i32)obs[i].y, obs[i].w, obs[i].h, T_THREE);
        }

        // score top-right (7seg)
        if (state != GAMEOVER) {
            i32 s = 3, thick = 2;
            i32 digits = count_digits_i32(score);
            i32 digit_w = 3 * s;
            i32 gap = s;
            i32 total_w = digits * digit_w + (digits - 1) * gap;

            i32 margin_r = 2;
            i32 score_x = LCD_W - margin_r - total_w;
            i32 score_y = 2;

            draw_int_7seg(score_x, score_y, s, thick, score, T_ONE);
        }

        // overlays
        if (state == IDLE) {
            draw_rectangle(LCD_W - 10, 2, 6, 2, T_THREE);
        }
        if (state == GAMEOVER) {
            draw_rectangle_outline_p(12, 32, LCD_W - 24, 64, 2.0f, T_ONE, BW_INDEX);

            i32 s = 6, thick = 3;
            i32 digits = count_digits_i32(score);
            i32 digit_w = 3 * s;
            i32 gap = s;
            i32 total_w = digits * digit_w + (digits - 1) * gap;

            i32 box_x = 12;
            i32 box_w = LCD_W - 24;

            i32 score_x = box_x + (box_w - total_w) / 2;
            i32 score_y = 50;

            draw_int_7seg(score_x, score_y, s, thick, score, T_ONE);
        }

        display_end();
    }

    display_close();
    return score;
}

// ------------------------------------------------------------
// MAIN DI TEST

int main(void) {
    int score = dino_runner_game();
    printf("Score: %d\n", score);
    return score;
}
