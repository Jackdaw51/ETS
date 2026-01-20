// game_files/dino_runner.c
// Dino Runner - usa dino_state1 (run) e dino_state2 (duck)

#include <stdio.h>

#include "display/display.h"
#include "sprites/sprites.h"
#include "sprites/palettes.h"
#include "stddef.h"

// ------------------------------------------------------------
// LCD size
#define LCD_W 160
#define LCD_H 128

// Start condition (muovi mano abbastanza rispetto al baseline)
#define PROX_START_DELTA 8.0f

// Jump gesture (delta frame-to-frame) - meno sensibile
#define PROX_JUMP_DELTA  28.0f

// Duck: mano molto vicina (tuning)
#define PROX_DUCK_TH     750.0f

// Ground
#define GROUND_Y      (LCD_H - 18)

// Dino position (x fisso)
#define DINO_X        22

// Physics (frame-based) -> salto più "lungo"
#define GRAVITY       0.30f   // prima 0.35
#define JUMP_VY      -7.2f    // prima -6.5
#define VY_MAX        8.0f

// Speed (px/frame) - più lento
#define SPEED_START   1.30f
#define SPEED_MUL     1.002f
#define SPEED_MAX     4.50f

// Obstacles
#define MAX_OBS       6
#define SPAWN_MIN_FR  48
#define SPAWN_MAX_FR  95

// Jump cooldown (anti-rumore)
#define JUMP_COOLDOWN_FR 10

// Gameover
#define GAMEOVER_FRAMES 90

#define DINO_DUCK_H_PIX 12


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
// Simple RNG

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
// 7-seg digits (come pong_wall)

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

    if (m & (1<<6)) draw_rectangle_p(x + thick, y, w - 2*thick, thick, col, BW_INDEX);
    if (m & (1<<5)) draw_rectangle_p(x + w - thick, y + thick, thick, (h/2) - thick, col, BW_INDEX);
    if (m & (1<<4)) draw_rectangle_p(x + w - thick, y + (h/2), thick, (h/2) - thick, col, BW_INDEX);
    if (m & (1<<3)) draw_rectangle_p(x + thick, y + h - thick, w - 2*thick, thick, col, BW_INDEX);
    if (m & (1<<2)) draw_rectangle_p(x, y + (h/2), thick, (h/2) - thick, col, BW_INDEX);
    if (m & (1<<1)) draw_rectangle_p(x, y + thick, thick, (h/2) - thick, col, BW_INDEX);
    if (m & (1<<0)) draw_rectangle_p(x + thick, y + (h/2) - (thick/2), w - 2*thick, thick, col, BW_INDEX);
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
        // -------- CACTUS (a terra)
        o->type = OBS_CACTUS;
        o->w = rng_range_i32(6, 16);
        o->h = rng_range_i32(10, 20);
        o->x = (f32)(LCD_W + 2);
        o->y = (f32)(GROUND_Y - o->h);
    } else {
        // -------- BIRD (in aria)
        o->type = OBS_BIRD;
        o->w = 14;
        o->h = 8;
        o->x = (f32)(LCD_W + 2);

        // vogliamo che "duck" (12px) possa passare sotto
        // gap = GROUND_Y - (bird_bottom) >= DINO_DUCK_H_PIX + margin
        const i32 margin = 2;
        const i32 gap_ok = DINO_DUCK_H_PIX + margin;

        // y che rende duck necessario e sufficiente
        i32 y_duck = GROUND_Y - (gap_ok + o->h);

        // y più alto (duck non necessario, ma sempre ok)
        i32 y_high = y_duck - 10;

        // clamp per sicurezza (non andare sopra lo schermo)
        if (y_high < 0) y_high = 0;

        // mix: spesso uccello basso (duck richiesto), ogni tanto più alto
        if (rng_chance_percent(60)) o->y = (f32)y_duck;
        else                        o->y = (f32)y_high;
    }
}


static void despawn_if_offscreen(Obstacle* o) {
    if (o->active && (o->x + (f32)o->w < -2.0f)) o->active = 0;
}

// ------------------------------------------------------------
// GAME

int dino_runner_game(void) {
    display_init_lcd();
    // Palette di gioco (sfondo/terrain ecc.)
    set_palette(DINO_CUSTOM_PALETTE_INDEX);

    TextureHandle dino_run_tex  = load_texture_from_sprite_p(
    dino_state1_sprite.height, dino_state1_sprite.width, dino_state1_sprite.data,
    BW_INDEX
);

    TextureHandle dino_duck_tex = load_texture_from_sprite_p(
        dino_state2_sprite.height, dino_state2_sprite.width, dino_state2_sprite.data,
        BW_INDEX
    );

    set_screen_color(T_ONE);

    // Dimensioni reali (hitbox = sprite)
    const i32 DINO_RUN_W  = (i32)dino_state1_sprite.width;
    const i32 DINO_RUN_H  = (i32)dino_state1_sprite.height;
    const i32 DINO_DUCK_W = (i32)dino_state2_sprite.width;
    const i32 DINO_DUCK_H = (i32)dino_state2_sprite.height;

    GameState state = IDLE;

    Dino dino = (Dino){
        .y = (f32)(GROUND_Y - DINO_RUN_H),
        .vy = 0.0f,
        .on_ground = 1,
        .duck = 0
    };

    Obstacle obs[MAX_OBS];
    clear_obstacles(obs);

    i32 score = 0;

    f32 speed = SPEED_START;
    i32 spawn_cd = rng_range_i32(SPAWN_MIN_FR, SPAWN_MAX_FR);

    // proximity for START + delta jump
    f32 proximity = get_proximity();
    f32 prox0 = proximity;
    f32 prox_prev = proximity;

    i32 jump_cd = 0;
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

                dino.duck = 0;
                dino.on_ground = 1;
                dino.vy = 0.0f;
                dino.y = (f32)(GROUND_Y - DINO_RUN_H);

                clear_obstacles(obs);
                jump_cd = 0;
            }
        }
        else if (state == PLAYING) {
            if (jump_cd > 0) jump_cd--;

            // duck solo a terra
            dino.duck = (duck && dino.on_ground) ? 1 : 0;

            // dimensioni attuali (in base allo sprite)
            i32 cur_w = dino.duck ? DINO_DUCK_W : DINO_RUN_W;
            i32 cur_h = dino.duck ? DINO_DUCK_H : DINO_RUN_H;

            // Se passi in duck mentre sei a terra, riallinea la y al terreno
            if (dino.on_ground) {
                dino.y = (f32)(GROUND_Y - cur_h);
            }

            // jump solo se a terra e cooldown finito
            if (jump && dino.on_ground && jump_cd == 0) {
                dino.vy = JUMP_VY;
                dino.on_ground = 0;
                jump_cd = JUMP_COOLDOWN_FR;
            }

            // physics
            dino.vy += GRAVITY;
            dino.vy = clampf(dino.vy, -50.0f, VY_MAX);
            dino.y  += dino.vy;

            // FIX pavimento (int)
            i32 ground_top_i = GROUND_Y - cur_h;
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
            spawn_cd--;
            if (spawn_cd <= 0) {
                spawn_obstacle(obs);

                i32 min_fr = SPAWN_MIN_FR - (i32)((speed - SPEED_START) * 3.0f);
                i32 max_fr = SPAWN_MAX_FR - (i32)((speed - SPEED_START) * 4.0f);
                if (min_fr < 30) min_fr = 30;
                if (max_fr < 55) max_fr = 55;

                spawn_cd = rng_range_i32(min_fr, max_fr);
            }

            // move obstacles + collision
            i32 dino_yi = (i32)dino.y;
            for (int i = 0; i < MAX_OBS; i++) {
                if (!obs[i].active) continue;

                obs[i].x -= speed;
                despawn_if_offscreen(&obs[i]);

                if (obs[i].active) {
                    if (aabb_i32(DINO_X, dino_yi, cur_w, cur_h,
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
        draw_rectangle(0, GROUND_Y, LCD_W, 20, T_TWO);

        // dino sprite
        {
            if (state == PLAYING && dino.duck) {
                draw_texture((u8)DINO_X, (u8)((i32)dino.y), dino_duck_tex);
            } else {
                draw_texture((u8)DINO_X, (u8)((i32)dino.y), dino_run_tex);
            }
        }

        // obstacles (ancora rettangoli)
        for (int i = 0; i < MAX_OBS; i++) {
            if (!obs[i].active) continue;
            draw_rectangle((i32)obs[i].x, (i32)obs[i].y, obs[i].w, obs[i].h, T_TWO);
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
            draw_rectangle(LCD_W - 10, 2, 6, 2, T_TWO);
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
