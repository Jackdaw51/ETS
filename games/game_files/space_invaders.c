#include <stdio.h>

#include "display/display.h"
#include "display/types.h"
#include "sprites/palettes.h"
#include "sprites/sprites.h"

// ------------------------------------------------------------
// LCD size
#define LCD_W 160
#define LCD_H 128

// Input (proximity)
#define PROX_MAX           1023.0f
#define PROX_START_DELTA   8.0f

// Player movement: aumenta la "corsa" ai bordi (aiuta se non raggiungi 0/1023 reali)
#define SHIP_RANGE_EXTRA   10.0f   // px extra virtuali a sx/dx (più = più facile arrivare ai margini)

// Auto-fire
#define AUTO_FIRE_PERIOD_FR  8
#define MAX_PB               3

// Waves
#define WAVE_PAUSE_FR        12

// Wave entry (drop from above)
#define WAVE_ENTRY_STEP_PX    2
#define WAVE_ENTRY_EXTRA_PAD 10

// Lives
#define START_LIVES          3
#define RESPAWN_PAUSE_FR     22
#define RESPAWN_INVULN_FR    50

// Player (coerente coi tuoi sprite)
#define SHIP_W   20
#define SHIP_H    8
#define SHIP_Y   (LCD_H - 10)

// Player bullet
#define PB_W 2
#define PB_H 5
#define PB_VY (-4.5f)

// Aliens grid
#define A_ROWS 5
#define A_COLS 10
#define A_W    12
#define A_H     8
#define A_GAPX  4
#define A_GAPY  4
#define A_LEFT  4
#define A_TOP   12
#define A_DROP   6

// Alien movement pacing (frames per step)
#define A_STEP_FR_START  28
#define A_STEP_FR_MIN     7

// Zig-zag: quante volte rimbalzano ai bordi prima di fare un drop
#define EDGE_BOUNCES_BEFORE_DROP  2

// Alien bullets
#define MAX_AB 2
#define AB_W 2
#define AB_H 5
#define AB_VY (2.8f)

// Lives UI (mini ship icons)
#define LIFE_ICON_W   8
#define LIFE_ICON_H   4
#define LIFE_ICON_GAP 2
#define LIFE_MARGIN_L 2
#define LIFE_MARGIN_T 1

// ------------------------------------------------------------
// UFO bonus
#define UFO_Y                10
#define UFO_SPAWN_MIN_FR      240
#define UFO_SPAWN_MAX_FR      420
#define UFO_POINTS            120

// ------------------------------------------------------------
// Combo dots
#define COMBO_MAX_LVL         6
#define COMBO_DOT_W           2
#define COMBO_DOT_H           2
#define COMBO_DOT_GAP         2
#define COMBO_DOT_Y           1

// ------------------------------------------------------------
// Palette choices
#define PAL_UI      BW_INDEX
#define PAL_SHIP    RETRO_RBY_INDEX
#define PAL_ALIEN1  OLIVE_GREEN_INDEX
#define PAL_ALIEN2  PONG_CUSTOM_PALETTE_INDEX
#define PAL_UFO     RETRO_RBY_INDEX

// ------------------------------------------------------------
// EXPLOSIONS (NUOVO)
#define EXP_FRAMES        10   // durata esplosione
#define EXP_THICK         2    // spessore "croce"
#define EXP_MAX_R         7    // raggio max (visivo)
#define UFO_EXP_FRAMES   12


// ------------------------------------------------------------
// Score helpers
static u8 count_digits_u32(u32 v) {
    if (v == 0) return 1;
    u8 n = 0;
    while (v > 0) { n++; v /= 10u; }
    return n;
}

// 7-seg (stile dino_runner)
static void draw_seg_digit(i32 x, i32 y, i32 s, i32 thick, int d, TWOS_COLOURS col) {
    static const u8 mask[10] = {
        0b1111110, 0b0110000, 0b1101101, 0b1111001, 0b0110011,
        0b1011011, 0b1011111, 0b1110000, 0b1111111, 0b1111011
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
// Helpers
static inline f32 clampf(f32 v, f32 lo, f32 hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}
static inline f32 absf(f32 v) { return (v < 0.0f) ? -v : v; }

static u8 aabb_i32(i32 ax, i32 ay, i32 aw, i32 ah,
                   i32 bx, i32 by, i32 bw, i32 bh) {
    i32 al = ax, ar = ax + aw;
    i32 at = ay, ab = ay + ah;
    i32 bl = bx, br = bx + bw;
    i32 bt = by, bb = by + bh;
    return (ar >= bl) && (al <= br) && (ab >= bt) && (at <= bb);
}

// RNG (LCG)
static u32 rng_state = 2463534242u;
static u32 rng_u32(void) {
    rng_state = rng_state * 1664525u + 1013904223u;
    return rng_state;
}
static u8 rng_range_u8(u8 lo, u8 hi) {
    if (hi <= lo) return lo;
    u32 span = (u32)(hi - lo) + 1u;
    return (u8)(lo + (u8)(rng_u32() % span));
}
static u8 rng_chance_percent_u8(u8 pct) {
    return ((u8)(rng_u32() % 100u) < pct) ? 1 : 0;
}

// ------------------------------------------------------------
// Data
typedef enum { IDLE=0, PLAYING=1 } GameState;

typedef struct {
    u8 active;
    f32 x, y;
    f32 vy;
} Bullet;

typedef struct {
    u8 alive;
    u8 kind;   // 0 = alien_1, 1 = alien_2
    i16 x, y;
} Alien;

typedef struct {
    u8 active;
    f32 x, y;
    f32 vx;
} Ufo;

static i16 alien_target_y(u8 r) {
    return (i16)(A_TOP + (i16)r * (i16)(A_H + A_GAPY));
}

// pattern semplice waves
static u8 alien_spawn_alive(u8 pattern, u8 r, u8 c) {
    if (pattern == 0) return 1;
    if (pattern == 1) {
        if (r > 0 && (c % 3u) == 1u) return 0;
        return 1;
    }
    // pattern 2
    if (r < 2 && c >= 4 && c <= 5) return 0;
    return 1;
}

static void init_aliens_drop(Alien a[A_ROWS][A_COLS], u8 pattern) {
    i16 total_h = (i16)(A_ROWS * (A_H + A_GAPY));
    i16 entry_offset = (i16)(A_TOP + total_h + WAVE_ENTRY_EXTRA_PAD);

    i16 x_shift = 0;
    if (pattern == 1) x_shift = 2;
    if (pattern == 2) x_shift = -2;

    for (u8 r = 0; r < A_ROWS; r++) {
        for (u8 c = 0; c < A_COLS; c++) {
            u8 alive = alien_spawn_alive(pattern, r, c);
            a[r][c].alive = alive;
            a[r][c].kind  = (u8)(r & 1u);
            a[r][c].x = (i16)(A_LEFT + x_shift + (i16)c * (i16)(A_W + A_GAPX));
            a[r][c].y = (i16)(alien_target_y(r) - entry_offset);
        }
    }
}

static u16 count_aliens_alive(Alien a[A_ROWS][A_COLS]) {
    u16 n = 0;
    for (u8 r = 0; r < A_ROWS; r++)
        for (u8 c = 0; c < A_COLS; c++)
            if (a[r][c].alive) n++;
    return n;
}

static u8 find_shooter_in_col(Alien a[A_ROWS][A_COLS], u8 col, i16 *out_x, i16 *out_y) {
    for (i8 r = (i8)A_ROWS - 1; r >= 0; --r) {
        if (a[(u8)r][col].alive) {
            *out_x = a[(u8)r][col].x;
            *out_y = a[(u8)r][col].y;
            return 1;
        }
    }
    return 0;
}

static void clear_player_bullets(Bullet pb[MAX_PB]) {
    for (u8 i = 0; i < MAX_PB; i++) pb[i].active = 0;
}

static void draw_life_icon(i32 x, i32 y) {
    draw_rectangle_p(x, y, LIFE_ICON_W, LIFE_ICON_H, T_THREE, PAL_SHIP);
    if (y > 0) draw_rectangle_p(x + 2, y - 1, LIFE_ICON_W - 4, 1, T_THREE, PAL_SHIP);
}

static void draw_lives(u8 lives) {
    i32 y = (LIFE_MARGIN_T < 1) ? 1 : LIFE_MARGIN_T;
    i32 x_left = LIFE_MARGIN_L;

    for (u8 i = 0; i < lives; i++) {
        i32 x = x_left + (i32)i * (LIFE_ICON_W + LIFE_ICON_GAP);
        draw_life_icon(x, y);
    }
}

static void draw_combo_dots(u8 combo_lvl) {
    if (combo_lvl <= 1) return;

    u8 dots = (u8)(combo_lvl - 1);
    if (dots > 10) dots = 10;

    i32 total_w = (i32)dots * COMBO_DOT_W + (i32)(dots - 1) * COMBO_DOT_GAP;
    i32 x0 = (LCD_W - total_w) / 2;
    i32 y0 = COMBO_DOT_Y;

    for (u8 i = 0; i < dots; i++) {
        i32 x = x0 + (i32)i * (COMBO_DOT_W + COMBO_DOT_GAP);
        draw_rectangle(x, y0, COMBO_DOT_W, COMBO_DOT_H, T_THREE);
    }
}

// ------------------------------------------------------------
// Mapping ship_x with extra range (important fix)
static f32 map_ship_x(f32 proximity) {
    f32 t = proximity / PROX_MAX;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    // range virtuale più largo, poi clampiamo allo schermo
    f32 span = (f32)(LCD_W - SHIP_W) + 2.0f * SHIP_RANGE_EXTRA;
    f32 x = t * span - SHIP_RANGE_EXTRA;
    return clampf(x, 0.0f, (f32)(LCD_W - SHIP_W));
}

// ------------------------------------------------------------
// EXPLOSION DRAW (NUOVO)
static void draw_explosion_at(i32 ax, i32 ay, u8 pal, u8 timer_left) {
    if (timer_left == 0) return;

    // age: 0..EXP_FRAMES-1
    u8 age = (u8)(EXP_FRAMES - timer_left);
    i32 r = 1 + (i32)((age * EXP_MAX_R) / (EXP_FRAMES - 1));
    if (r < 1) r = 1;
    if (r > EXP_MAX_R) r = EXP_MAX_R;

    // centro dell’alieno
    i32 cx = ax + (A_W / 2);
    i32 cy = ay + (A_H / 2);

    // un po' di "flash": alterna T_THREE/T_TWO
    TWOS_COLOURS col = ((age & 1u) == 0u) ? T_THREE : T_TWO;

    // croce orizzontale + verticale (spessore EXP_THICK)
    draw_rectangle_p(cx - r, cy - (EXP_THICK / 2), 2*r + 1, EXP_THICK, col, pal);
    draw_rectangle_p(cx - (EXP_THICK / 2), cy - r, EXP_THICK, 2*r + 1, col, pal);

    // piccoli "spark" diagonali ai 4 angoli
    draw_rectangle_p(cx - r, cy - r, 1, 1, col, pal);
    draw_rectangle_p(cx + r, cy - r, 1, 1, col, pal);
    draw_rectangle_p(cx - r, cy + r, 1, 1, col, pal);
    draw_rectangle_p(cx + r, cy + r, 1, 1, col, pal);
}

// ------------------------------------------------------------
// GAME
int space_invaders_game(void) {
    display_init_lcd();

    // UI in BW, sfondo nero
    set_palette(PAL_UI);
    set_screen_color(T_ONE);

    TextureHandle ship_tex = load_texture_from_sprite_p(
        spaceship_sprite.height, spaceship_sprite.width, spaceship_sprite.data, SPACESHIP_PALETTE_INDEX
    );
    TextureHandle alien1_tex = load_texture_from_sprite_p(
        alien_1_sprite.height, alien_1_sprite.width, alien_1_sprite.data, ALIEN_1_PALETTE_INDEX
    );
    TextureHandle alien2_tex = load_texture_from_sprite_p(
        alien_2_sprite.height, alien_2_sprite.width, alien_2_sprite.data, ALIEN_2_PALETTE_INDEX
    );
    TextureHandle ufo_tex = load_texture_from_sprite_p(
        ufo_bonus_sprite.height, ufo_bonus_sprite.width, ufo_bonus_sprite.data, UFO_PALETTE_INDEX
    );

    const i32 UFO_W = (i32)ufo_bonus_sprite.width;
    const i32 UFO_H = (i32)ufo_bonus_sprite.height;

    GameState state = IDLE;

    f32 proximity = get_proximity();
    f32 prox0 = proximity;

    f32 ship_x = (LCD_W - SHIP_W) / 2.0f;

    Bullet pb[MAX_PB];
    clear_player_bullets(pb);

    Bullet ab[MAX_AB] = {0};

    Alien aliens[A_ROWS][A_COLS];
    u8 pattern = 0;
    init_aliens_drop(aliens, pattern);

    // EXPLOSION TIMERS (NUOVO)
    static u8 expl_fr[A_ROWS][A_COLS] = {0};

    u8 wave_entering = 1;
    u8 a_dir = 1;
    u8 a_step_cd = A_STEP_FR_START;

    u8 edge_bounces = 0;

    u32 score = 0;
    u32 level = 1; // non disegnato, ma usato per speed

    u8 auto_fire_cd = AUTO_FIRE_PERIOD_FR;
    u8 wave_pause = 0;

    u8 lives = START_LIVES;
    u8 invuln = 0;

    // combo
    u8 combo_lvl = 1;
    u8 combo_hits = 0;

    // UFO
    Ufo ufo = {0};
    u8 ufo_expl = 0;
    i32 ufo_expl_x = 0;
    i32 ufo_expl_y = 0;
    u16 ufo_cd = (u16)rng_range_u8((u8)(UFO_SPAWN_MIN_FR/2), (u8)(UFO_SPAWN_MAX_FR/2));
    ufo_cd *= 2;

    while (!window_should_close()) {
        display_begin();

        // ---------- INPUT ----------
        proximity = get_proximity();
        ship_x = map_ship_x(proximity);

        // ---------- UPDATE ----------
        if (state == IDLE) {
            if (absf(proximity - prox0) >= PROX_START_DELTA) {
                state = PLAYING;

                score = 0;
                level = 1;
                lives = START_LIVES;
                invuln = 0;

                combo_lvl = 1;
                combo_hits = 0;

                clear_player_bullets(pb);
                for (u8 i = 0; i < MAX_AB; i++) ab[i].active = 0;

                pattern = (u8)((level - 1u) % 3u);
                init_aliens_drop(aliens, pattern);
                wave_entering = 1;

                // reset esplosioni (NUOVO)
                for (u8 r = 0; r < A_ROWS; r++)
                    for (u8 c = 0; c < A_COLS; c++)
                        expl_fr[r][c] = 0;

                a_dir = 1;
                a_step_cd = A_STEP_FR_START;
                edge_bounces = 0;

                auto_fire_cd = AUTO_FIRE_PERIOD_FR;
                wave_pause = 0;

                ufo.active = 0;
                ufo_expl = 0;

                ufo_cd = (u16)rng_range_u8((u8)(UFO_SPAWN_MIN_FR/2), (u8)(UFO_SPAWN_MAX_FR/2));
                ufo_cd *= 2;
            }
        } else { // PLAYING
            if (invuln > 0) invuln--;

            // tick esplosioni (NUOVO)
            for (u8 r = 0; r < A_ROWS; r++)
                for (u8 c = 0; c < A_COLS; c++)
                    if (expl_fr[r][c] > 0) expl_fr[r][c]--;
                    if (ufo_expl > 0) ufo_expl--;


            if (wave_pause > 0) {
                wave_pause--;
            } else {
                // -------- UFO spawn / move --------
                if (!wave_entering) {
                    if (!ufo.active) {
                        if (ufo_cd > 0) ufo_cd--;
                        if (ufo_cd == 0) {
                            ufo.active = 1;
                            ufo.y = (f32)UFO_Y;

                            u8 left_to_right = rng_chance_percent_u8(50);
                            f32 spd = 1.2f + 0.06f * (f32)(level - 1u);
                            if (spd > 2.2f) spd = 2.2f;

                            if (left_to_right) {
                                ufo.x = (f32)(-UFO_W - 2);
                                ufo.vx = spd;
                            } else {
                                ufo.x = (f32)(LCD_W + 2);
                                ufo.vx = -spd;
                            }

                            ufo_cd = (u16)rng_range_u8((u8)(UFO_SPAWN_MIN_FR/2), (u8)(UFO_SPAWN_MAX_FR/2));
                            ufo_cd *= 2;
                        }
                    } else {
                        ufo.x += ufo.vx;
                        if (ufo.vx > 0 && ufo.x > (f32)(LCD_W + 2)) ufo.active = 0;
                        if (ufo.vx < 0 && ufo.x < (f32)(-UFO_W - 2)) ufo.active = 0;
                    }
                } else {
                    ufo.active = 0;
                }

                // -------- AUTO FIRE --------
                if (auto_fire_cd > 0) auto_fire_cd--;
                if (auto_fire_cd == 0) {
                    for (u8 i = 0; i < MAX_PB; i++) {
                        if (!pb[i].active) {
                            pb[i].active = 1;
                            pb[i].x = ship_x + (SHIP_W/2) - (PB_W/2);
                            pb[i].y = (f32)SHIP_Y - 2.0f;

                            f32 pvy = PB_VY - 0.10f * (f32)(level - 1u);
                            if (pvy < -6.5f) pvy = -6.5f;
                            pb[i].vy = pvy;
                            break;
                        }
                    }
                    auto_fire_cd = AUTO_FIRE_PERIOD_FR;
                }

                // Move player bullets
                for (u8 i = 0; i < MAX_PB; i++) {
                    if (!pb[i].active) continue;
                    pb[i].y += pb[i].vy;
                    if (pb[i].y < -10.0f) pb[i].active = 0;
                }

                // ---- wave clear? ----
                {
                    u16 alive = count_aliens_alive(aliens);
                    if (alive == 0) {
                        level++;

                        clear_player_bullets(pb);
                        for (u8 i = 0; i < MAX_AB; i++) ab[i].active = 0;

                        pattern = (u8)((level - 1u) % 3u);
                        init_aliens_drop(aliens, pattern);
                        wave_entering = 1;

                        // reset esplosioni (NUOVO)
                        for (u8 r = 0; r < A_ROWS; r++)
                            for (u8 c = 0; c < A_COLS; c++)
                                expl_fr[r][c] = 0;

                        a_dir = 1;
                        a_step_cd = A_STEP_FR_START;
                        edge_bounces = 0;

                        wave_pause = (u8)WAVE_PAUSE_FR;
                        ufo.active = 0;
                    }
                }

                // ---- WAVE ENTRY ----
                if (wave_entering) {
                    u8 done = 1;
                    for (u8 r = 0; r < A_ROWS; r++) {
                        i16 ty = alien_target_y(r);
                        for (u8 c = 0; c < A_COLS; c++) {
                            if (!aliens[r][c].alive) continue;

                            i16 ny = (i16)(aliens[r][c].y + (i16)WAVE_ENTRY_STEP_PX);
                            if (ny >= ty) ny = ty;
                            else done = 0;

                            aliens[r][c].y = ny;
                        }
                    }
                    if (done) wave_entering = 0;
                }

                // Player bullets vs UFO
                if (ufo.active) {
                    i32 ux = (i32)ufo.x;
                    i32 uy = (i32)ufo.y;

                    for (u8 bi = 0; bi < MAX_PB; bi++) {
                        if (!pb[bi].active) continue;
                        if (aabb_i32((i32)pb[bi].x, (i32)pb[bi].y, PB_W, PB_H,
                                     ux, uy, UFO_W, UFO_H)) {
                            pb[bi].active = 0;

                            // salva posizione esplosione (centro ufo)
                            ufo_expl = (u8)UFO_EXP_FRAMES;
                            ufo_expl_x = (i32)ufo.x;
                            ufo_expl_y = (i32)ufo.y;

                            ufo.active = 0;
                            score += (u32)(UFO_POINTS * (u32)combo_lvl);
                            break;

                        }
                    }
                }

                // Player bullets vs aliens
                for (u8 bi = 0; bi < MAX_PB; bi++) {
                    if (!pb[bi].active) continue;

                    i32 bx = (i32)pb[bi].x;
                    i32 by = (i32)pb[bi].y;

                    for (u8 r = 0; r < A_ROWS && pb[bi].active; r++) {
                        for (u8 c = 0; c < A_COLS && pb[bi].active; c++) {
                            if (!aliens[r][c].alive) continue;

                            if (aabb_i32(bx, by, PB_W, PB_H,
                                         (i32)aliens[r][c].x, (i32)aliens[r][c].y, A_W, A_H)) {

                                // --- kill alien ---
                                aliens[r][c].alive = 0;
                                pb[bi].active = 0;

                                // EXPLOSIONE (NUOVO)
                                expl_fr[r][c] = (u8)EXP_FRAMES;

                                combo_hits++;
                                if ((combo_hits % 2u) == 0u && combo_lvl < (u8)COMBO_MAX_LVL) combo_lvl++;

                                u32 base = 10u + (u32)(r * 2u);
                                score += base * (u32)combo_lvl;
                            }
                        }
                    }
                }

                // movimento/sparo alieni SOLO se wave in posizione
                if (!wave_entering) {
                    i32 base_start = (i32)A_STEP_FR_START - (i32)((level - 1u) * 1u);
                    if (base_start < A_STEP_FR_MIN) base_start = A_STEP_FR_MIN;

                    u16 alive_now = count_aliens_alive(aliens);
                    i32 fr = base_start - (i32)((50 - (i32)alive_now) / 8);
                    if (fr < A_STEP_FR_MIN) fr = A_STEP_FR_MIN;
                    u8 a_step_fr = (u8)fr;

                    if (a_step_cd > 0) a_step_cd--;
                    if (a_step_cd == 0) {
                        a_step_cd = a_step_fr;

                        u8 hit_edge = 0;
                        for (u8 r = 0; r < A_ROWS; r++) {
                            for (u8 c = 0; c < A_COLS; c++) {
                                if (!aliens[r][c].alive) continue;
                                i32 nx = (i32)aliens[r][c].x + (a_dir ? 1 : -1) * 2;
                                if (nx < 2 || nx + A_W > (LCD_W - 2)) { hit_edge = 1; break; }
                            }
                            if (hit_edge) break;
                        }

                        if (hit_edge) {
                            a_dir = (u8)(!a_dir);
                            edge_bounces++;
                            if (edge_bounces >= (u8)EDGE_BOUNCES_BEFORE_DROP) {
                                edge_bounces = 0;
                                for (u8 r = 0; r < A_ROWS; r++)
                                    for (u8 c = 0; c < A_COLS; c++)
                                        if (aliens[r][c].alive)
                                            aliens[r][c].y = (i16)(aliens[r][c].y + A_DROP);
                            }
                        } else {
                            i16 dx = a_dir ? 2 : -2;
                            for (u8 r = 0; r < A_ROWS; r++)
                                for (u8 c = 0; c < A_COLS; c++)
                                    if (aliens[r][c].alive)
                                        aliens[r][c].x = (i16)(aliens[r][c].x + dx);
                        }

                        i32 fire_pct = 45 + (i32)((level - 1u) * 2u);
                        if (fire_pct > 70) fire_pct = 70;

                        if (rng_chance_percent_u8((u8)fire_pct)) {
                            i8 slot = -1;
                            for (u8 i = 0; i < MAX_AB; i++) if (!ab[i].active) { slot = (i8)i; break; }

                            if (slot >= 0) {
                                u8 col = rng_range_u8(0, A_COLS - 1);
                                i16 sx, sy;
                                if (find_shooter_in_col(aliens, col, &sx, &sy)) {
                                    ab[(u8)slot].active = 1;
                                    ab[(u8)slot].x = (f32)sx + (A_W/2) - (AB_W/2);
                                    ab[(u8)slot].y = (f32)sy + (f32)A_H + 1.0f;

                                    f32 vy = AB_VY + 0.12f * (f32)(level - 1u);
                                    if (vy > 4.0f) vy = 4.0f;
                                    ab[(u8)slot].vy = vy;
                                }
                            }
                        }
                    }

                    // GAME OVER se arrivano alla nave
                    for (u8 r = 0; r < A_ROWS; r++) {
                        for (u8 c = 0; c < A_COLS; c++) {
                            if (!aliens[r][c].alive) continue;
                            if ((i32)aliens[r][c].y + A_H >= SHIP_Y - 1) {
                                display_end();
                                display_close();
                                return (int)score;
                            }
                        }
                    }
                }

                // Alien bullets + ship collision
                for (u8 i = 0; i < MAX_AB; i++) {
                    if (!ab[i].active) continue;

                    ab[i].y += ab[i].vy;
                    if (ab[i].y > (f32)(LCD_H + 10)) { ab[i].active = 0; continue; }

                    if (invuln == 0 &&
                        aabb_i32((i32)ab[i].x, (i32)ab[i].y, AB_W, AB_H,
                                 (i32)ship_x, SHIP_Y, SHIP_W, SHIP_H)) {

                        combo_lvl = 1;
                        combo_hits = 0;

                        if (lives > 0) lives--;
                        if (lives == 0) {
                            display_end();
                            display_close();
                            return (int)score;
                        }

                        clear_player_bullets(pb);
                        for (u8 k = 0; k < MAX_AB; k++) ab[k].active = 0;

                        ship_x = (LCD_W - SHIP_W) / 2.0f;
                        invuln = (u8)RESPAWN_INVULN_FR;
                        wave_pause = (u8)RESPAWN_PAUSE_FR;
                        break;
                    }
                }
            }
        }

        // ---------- DRAW ----------
        clear_screen();

        // In IDLE vogliamo "schermata iniziale" non vuota
        if (state == IDLE) {
            draw_lives(START_LIVES);

            // score 0
            {
                i32 s = 2, thick = 1;
                u32 sc = 0;
                u8 digits = count_digits_u32(sc);
                i32 digit_w = 3 * s;
                i32 gap = s;
                i32 total_w = (i32)digits * digit_w + ((i32)digits - 1) * gap;
                i32 score_x = (LCD_W - 2) - total_w;
                i32 score_y = 2;
                draw_int_7seg(score_x, score_y, s, thick, sc, T_THREE);
            }

            // ship segue già il sensore
            {
                i32 sx = (i32)ship_x;
                if (sx < 0) sx = 0;
                if (sx > (LCD_W - SHIP_W)) sx = (LCD_W - SHIP_W);
                draw_texture((u8)sx, (u8)SHIP_Y, ship_tex);
            }

            // preview alieni (pattern 0)
            for (u8 r = 0; r < A_ROWS; r++) {
                for (u8 c = 0; c < A_COLS; c++) {
                    if (!alien_spawn_alive(0, r, c)) continue;
                    i32 ax = (i32)(A_LEFT + (i32)c * (A_W + A_GAPX));
                    i32 ay = (i32)alien_target_y(r);
                    TextureHandle texp = ((r & 1u) == 0u) ? alien1_tex : alien2_tex;
                    if (ax >= 0 && ax <= 255 && ay >= 0 && ay <= 255) draw_texture((u8)ax, (u8)ay, texp);
                }
            }

            display_end();
            continue;
        }

        // PLAYING UI
        draw_lives(lives);

        // Score top-right
        {
            i32 s = 2, thick = 1;
            u8 digits = count_digits_u32(score);
            i32 digit_w = 3 * s;
            i32 gap = s;
            i32 total_w = (i32)digits * digit_w + ((i32)digits - 1) * gap;

            i32 score_x = (LCD_W - 2) - total_w;
            i32 score_y = 2;
            draw_int_7seg(score_x, score_y, s, thick, score, T_THREE);
        }

        // Combo dots top-center
        draw_combo_dots(combo_lvl);

        // UFO
        if (ufo.active) {
            i32 ux = (i32)ufo.x;
            i32 uy = (i32)ufo.y;
            if (ux >= -UFO_W && ux <= 255 && uy >= 0 && uy <= 255) {
                i32 dx = ux;
                if (dx < 0) dx = 0;
                if (dx > 255) dx = 255;
                draw_texture((u8)dx, (u8)uy, ufo_tex);
            }
        }
        // UFO explosion (anche se ufo.active=0)
        if (ufo_expl > 0) {
            draw_explosion_at(ufo_expl_x, ufo_expl_y, PAL_UFO, ufo_expl);
        }


        // Player ship (blink durante invuln)
        {
            u8 draw_ship = 1;
            if (invuln > 0) draw_ship = (((invuln >> 2) & 1u) == 0u) ? 1u : 0u;

            if (draw_ship) {
                i32 sx = (i32)ship_x;
                if (sx < 0) sx = 0;
                if (sx > (LCD_W - SHIP_W)) sx = (LCD_W - SHIP_W);
                draw_texture((u8)sx, (u8)SHIP_Y, ship_tex);
            }
        }

        // Player bullets
        for (u8 i = 0; i < MAX_PB; i++)
            if (pb[i].active)
                draw_rectangle((i32)pb[i].x, (i32)pb[i].y, PB_W, PB_H, T_THREE);

        // Alien bullets
        for (u8 i = 0; i < MAX_AB; i++)
            if (ab[i].active)
                draw_rectangle((i32)ab[i].x, (i32)ab[i].y, AB_W, AB_H, T_THREE);

        // Aliens
        for (u8 r = 0; r < A_ROWS; r++) {
            for (u8 c = 0; c < A_COLS; c++) {
                if (!aliens[r][c].alive) continue;

                i32 ax = (i32)aliens[r][c].x;
                i32 ay = (i32)aliens[r][c].y;

                if (ay >= 0) {
                    TextureHandle texp = (aliens[r][c].kind == 0) ? alien1_tex : alien2_tex;
                    if (ax >= 0 && ax <= 255 && ay <= 255) draw_texture((u8)ax, (u8)ay, texp);
                } else {
                    u8 pal = (aliens[r][c].kind == 0) ? PAL_ALIEN1 : PAL_ALIEN2;
                    draw_rectangle_p(ax, ay, A_W, A_H, T_THREE, pal);
                }
            }
        }

        // ESPLOSIONI (NUOVO) - disegno SOPRA agli alieni
        for (u8 r = 0; r < A_ROWS; r++) {
            for (u8 c = 0; c < A_COLS; c++) {
                if (expl_fr[r][c] == 0) continue;

                i32 ax = (i32)aliens[r][c].x;
                i32 ay = (i32)aliens[r][c].y;
                u8 pal = (aliens[r][c].kind == 0) ? PAL_ALIEN1 : PAL_ALIEN2;

                draw_explosion_at(ax, ay, pal, expl_fr[r][c]);
            }
        }

        display_end();
    }

    display_close();
    return (int)score;
}

int main(void) {
    int score = space_invaders_game();
    printf("Score: %d\n", score);
    return score;
}
