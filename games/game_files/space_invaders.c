#include "space_invaders.h"

// ------------------------------------------------------------
// LCD size
#define LCD_W 160
#define LCD_H 128

// Joystick -> ship tuning
#define SHIP_STEP_PX  3   // pixel per tick

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

// Player
#define SHIP_W   20
#define SHIP_H    8
#define SHIP_Y   (LCD_H - 10)

// Player bullet
#define PB_W 2
#define PB_H 5

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

// Zig-zag
#define EDGE_BOUNCES_BEFORE_DROP  2

// Alien bullets
#define MAX_AB 2
#define AB_W 2
#define AB_H 5

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
// EXPLOSIONS
#define EXP_FRAMES        10
#define EXP_THICK         2
#define EXP_MAX_R         7
#define UFO_EXP_FRAMES   12

// ------------------------------------------------------------
// FIXED-POINT (Q8.8) for bullets/UFO
#define FP_SHIFT 8
#define FP_ONE   (1 << FP_SHIFT)
static inline i32 fp_from_i32(i32 v) { return (v << FP_SHIFT); }
static inline i32 fp_to_i32(i32 vfp) { return (vfp >> FP_SHIFT); }

// 4.5 * 256 = 1152
#define PB_VY_FP   (-1152)
// 0.10 * 256 = 25.6 ~ 26
#define PB_VY_LVL_DELTA_FP  (26)
// 6.5 * 256 = 1664
#define PB_VY_CAP_FP (-1664)

// 2.8 * 256 = 716.8 ~ 717
#define AB_VY_FP   (717)
// 0.12 * 256 = 30.72 ~ 31
#define AB_VY_LVL_DELTA_FP  (31)
// 4.0 * 256 = 1024
#define AB_VY_CAP_FP (1024)

// UFO speed: 1.2 * 256 = 307.2 ~ 307
#define UFO_SPD_FP   (307)
// 0.06 * 256 = 15.36 ~ 15
#define UFO_SPD_LVL_DELTA_FP (15)
// 2.2 * 256 = 563.2 ~ 563
#define UFO_SPD_CAP_FP (563)

// ------------------------------------------------------------
// Score helpers
static u8 count_digits_u32(u32 v) {
    if (v == 0) return 1;
    u8 n = 0;
    while (v > 0) { n++; v /= 10u; }
    return n;
}

// 7-seg
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
    int i;
    for (i = n - 1; i >= 0; --i) {
        draw_seg_digit(x, y, s, thick, digits[i], col);
        x += (3*s + s);
    }
}

// ------------------------------------------------------------
// Helpers
static inline i32 clampi32(i32 v, i32 lo, i32 hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static u8 aabb_i32(i32 ax, i32 ay, i32 aw, i32 ah,
                   i32 bx, i32 by, i32 bw, i32 bh) {
    i32 al = ax, ar = ax + aw;
    i32 at = ay, ab = ay + ah;
    i32 bl = bx, br = bx + bw;
    i32 bt = by, bb = by + bh;
    return (ar >= bl) && (al <= br) && (ab >= bt) && (at <= bb);
}

static inline u8 rect_on_screen(i32 x, i32 y, i32 w, i32 h) {
    if (w <= 0 || h <= 0) return 0;
    if (x >= LCD_W-1 || y >= LCD_H-1) return 0;
    if (x + w <= 0 || y + h <= 0) return 0;
    return 1;
}


// RNG (LCG)
static u32 rng_state = 2463534242u;
static u32 rng_u32(void) {
    rng_state = rng_state * 1664525u + 1013904223u;
    return rng_state;
}
static u8 rng_chance_percent_u8(u8 pct) {
    return ((u8)(rng_u32() % 100u) < pct) ? 1 : 0;
}
static u16 rng_range_u16(u16 lo, u16 hi) {
    if (hi <= lo) return lo;
    u32 span = (u32)(hi - lo) + 1u;
    return (u16)(lo + (u16)(rng_u32() % span));
}
static u8 rng_range_u8(u8 lo, u8 hi) {
    if (hi <= lo) return lo;
    u32 span = (u32)(hi - lo) + 1u;
    return (u8)(lo + (u8)(rng_u32() % span));
}
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


// ------------------------------------------------------------
// Data
typedef enum { IDLE=0, PLAYING=1 } GameState;

typedef struct {
    u8 active;
    i32 x_fp, y_fp; // Q8.8
    i32 vy_fp;      // Q8.8
} Bullet;

typedef struct {
    u8 alive;
    u8 kind;   // 0 = alien_1, 1 = alien_2
    i16 x, y;
} Alien;

typedef struct {
    u8 active;
    i32 x_fp, y_fp; // Q8.8
    i32 vx_fp;      // Q8.8
} Ufo;

// ------------------------------------------------------------
static i16 alien_target_y(u8 r) {
    return (i16)(A_TOP + (i16)r * (i16)(A_H + A_GAPY));
}

// pattern for waves
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

// init wave + returns alive_count (avoids count_aliens_alive for every frame)
static u16 init_aliens_drop(Alien a[A_ROWS][A_COLS], u8 pattern) {
    i16 total_h = (i16)(A_ROWS * (A_H + A_GAPY));
    i16 entry_offset = (i16)(A_TOP + total_h + WAVE_ENTRY_EXTRA_PAD);

    i16 x_shift = 0;
    if (pattern == 1) x_shift = 2;
    if (pattern == 2) x_shift = -2;

    u16 alive_count = 0;
    u8 r;
    for (r = 0; r < A_ROWS; r++) {
        u8 c;
        for (c = 0; c < A_COLS; c++) {
            u8 alive = alien_spawn_alive(pattern, r, c);
            a[r][c].alive = alive;
            a[r][c].kind  = (u8)(r & 1u);
            a[r][c].x = (i16)(A_LEFT + x_shift + (i16)c * (i16)(A_W + A_GAPX));
            a[r][c].y = (i16)(alien_target_y(r) - entry_offset);
            if (alive) alive_count++;
        }
    }
    return alive_count;
}

static u8 find_shooter_in_col(Alien a[A_ROWS][A_COLS], u8 col, i16 *out_x, i16 *out_y) {
    i8 r;
    for (r = (i8)A_ROWS - 1; r >= 0; --r) {
        if (a[(u8)r][col].alive) {
            *out_x = a[(u8)r][col].x;
            *out_y = a[(u8)r][col].y;
            return 1;
        }
    }
    return 0;
}

static void clear_player_bullets(Bullet pb[MAX_PB]) {
    u8 i;
    for (i = 0; i < MAX_PB; i++) pb[i].active = 0;
}

static void draw_life_icon(i32 x, i32 y) {
    draw_rectangle_p(x, y, LIFE_ICON_W, LIFE_ICON_H, T_THREE, PAL_SHIP);
    if (y > 0) draw_rectangle_p(x + 2, y - 1, LIFE_ICON_W - 4, 1, T_THREE, PAL_SHIP);
}

static void draw_lives(u8 lives) {
    i32 y = (LIFE_MARGIN_T < 1) ? 1 : LIFE_MARGIN_T;
    i32 x_left = LIFE_MARGIN_L;
    u8 i;
    for (i = 0; i < lives; i++) {
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

    u8 i;
    for (i = 0; i < dots; i++) {
        i32 x = x0 + (i32)i * (COMBO_DOT_W + COMBO_DOT_GAP);
        draw_rectangle(x, y0, COMBO_DOT_W, COMBO_DOT_H, T_THREE);
    }
}


// ------------------------------------------------------------
// EXPLOSION DRAW
static void draw_explosion_at(i32 ax, i32 ay, u8 pal, u8 timer_left) {
    if (timer_left == 0) return;

    u8 age = (u8)(EXP_FRAMES - timer_left);
    i32 r = 1 + (i32)((age * EXP_MAX_R) / (EXP_FRAMES - 1));
    if (r < 1) r = 1;
    if (r > EXP_MAX_R) r = EXP_MAX_R;

    i32 cx = ax + (A_W / 2);
    i32 cy = ay + (A_H / 2);

    TWOS_COLOURS col = ((age & 1u) == 0u) ? T_THREE : T_TWO;

    draw_rectangle_p(cx - r, cy - (EXP_THICK / 2), 2*r + 1, EXP_THICK, col, pal);
    draw_rectangle_p(cx - (EXP_THICK / 2), cy - r, EXP_THICK, 2*r + 1, col, pal);

    draw_rectangle_p(cx - r, cy - r, 1, 1, col, pal);
    draw_rectangle_p(cx + r, cy - r, 1, 1, col, pal);
    draw_rectangle_p(cx - r, cy + r, 1, 1, col, pal);
    draw_rectangle_p(cx + r, cy + r, 1, 1, col, pal);
}

// ------------------------------------------------------------
// GAME
int space_invaders_game(void) {
    display_init_lcd();

    // UI in BW, black background
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


    i32 ship_x = (LCD_W - SHIP_W) / 2;

    // --------- static for not stress the stack ---------
    static Bullet pb[MAX_PB];
    static Bullet ab[MAX_AB];
    static Alien  aliens[A_ROWS][A_COLS];
    static u8     expl_fr[A_ROWS][A_COLS];

    clear_player_bullets(pb);
    u8 i;
    for (i = 0; i < MAX_AB; i++) ab[i].active = 0;

    u8 pattern = 0;
    u16 alive_count = init_aliens_drop(aliens, pattern);

    // reset explosions
    u8 r;
    for (r = 0; r < A_ROWS; r++){
        u8 c;
        for (c = 0; c < A_COLS; c++)
            expl_fr[r][c] = 0;
    }

    u8 wave_entering = 1;
    u8 a_dir = 1;
    u8 a_step_cd = A_STEP_FR_START;
    u8 edge_bounces = 0;

    u32 score = 0;
    u32 level = 1; // used for speed

    u8 auto_fire_cd = AUTO_FIRE_PERIOD_FR;
    u8 wave_pause = 0;

    u8 lives = START_LIVES;
    u8 invuln = 0;

    // combo
    u8 combo_lvl = 1;
    u8 combo_hits = 0;

    // UFO
    Ufo ufo;
    ufo.active = 0;
    ufo.x_fp = 0;
    ufo.y_fp = fp_from_i32(UFO_Y);
    ufo.vx_fp = 0;

    u8 ufo_expl = 0;
    i32 ufo_expl_x = 0;
    i32 ufo_expl_y = 0;
    u16 ufo_cd = rng_range_u16(UFO_SPAWN_MIN_FR, UFO_SPAWN_MAX_FR);

    u8 c, k, bi;
    while (!window_should_close()) {
        display_begin();

        // ---------- INPUT (joystick) ----------
        joystick_t action = joystick_action();

        if (action == JS_LEFT) ship_x = clampi32(ship_x - SHIP_STEP_PX, 0, (LCD_W - SHIP_W));
;
        if (action == JS_RIGHT) ship_x = clampi32(ship_x + SHIP_STEP_PX, 0, (LCD_W - SHIP_W)) ;

        // clamp ship


        // ---------- UPDATE ----------
        if (state == IDLE) {
            // start: any kind of input
            if (action != JS_NONE) {
                state = PLAYING;

                score = 0;
                level = 1;
                lives = START_LIVES;
                invuln = 0;

                combo_lvl = 1;
                combo_hits = 0;

                clear_player_bullets(pb);
                for (i = 0; i < MAX_AB; i++) ab[i].active = 0;

                pattern = (u8)((level - 1u) % 3u);
                alive_count = init_aliens_drop(aliens, pattern);
                wave_entering = 1;

                for (r = 0; r < A_ROWS; r++)
                    for (c = 0; c < A_COLS; c++)
                        expl_fr[r][c] = 0;

                a_dir = 1;
                a_step_cd = A_STEP_FR_START;
                edge_bounces = 0;

                auto_fire_cd = AUTO_FIRE_PERIOD_FR;
                wave_pause = 0;

                ufo.active = 0;
                ufo_expl = 0;
                ufo_cd = rng_range_u16(UFO_SPAWN_MIN_FR, UFO_SPAWN_MAX_FR);
            }
        } else {
            if (invuln > 0) invuln--;
            for (r = 0; r < A_ROWS; r++) {
                for (c = 0; c < A_COLS; c++) {
                    if (expl_fr[r][c] > 0) expl_fr[r][c]--;
                }
            }
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
                            ufo.y_fp = fp_from_i32(UFO_Y);

                            // spd = 1.2 + 0.06*(level-1), cap 2.2
                            i32 spd_fp = UFO_SPD_FP + (i32)(level - 1u) * UFO_SPD_LVL_DELTA_FP;
                            if (spd_fp > UFO_SPD_CAP_FP) spd_fp = UFO_SPD_CAP_FP;

                            u8 left_to_right = rng_chance_percent_u8(50);
                            if (left_to_right) {
                                ufo.x_fp = fp_from_i32(-UFO_W - 2);
                                ufo.vx_fp = spd_fp;
                            } else {
                                ufo.x_fp = fp_from_i32(LCD_W + 2);
                                ufo.vx_fp = -spd_fp;
                            }

                            ufo_cd = rng_range_u16(UFO_SPAWN_MIN_FR, UFO_SPAWN_MAX_FR);
                        }
                    } else {
                        ufo.x_fp += ufo.vx_fp;
                        i32 ux = fp_to_i32(ufo.x_fp);
                        if (ufo.vx_fp > 0 && ux > (LCD_W + 2)) ufo.active = 0;
                        if (ufo.vx_fp < 0 && ux < (-UFO_W - 2)) ufo.active = 0;
                    }
                } else {
                    ufo.active = 0;
                }

                // -------- AUTO FIRE --------
                if (auto_fire_cd > 0) auto_fire_cd--;
                if (auto_fire_cd == 0) {
                    for (i = 0; i < MAX_PB; i++) {
                        if (!pb[i].active) {
                            pb[i].active = 1;
                            i32 bx = ship_x + (SHIP_W/2) - (PB_W/2);
                            pb[i].x_fp = fp_from_i32(bx);
                            pb[i].y_fp = fp_from_i32(SHIP_Y - 2);

                            // pvy = -4.5 - 0.10*(level-1), cap -6.5
                            i32 pvy_fp = PB_VY_FP - (i32)(level - 1u) * PB_VY_LVL_DELTA_FP;
                            if (pvy_fp < PB_VY_CAP_FP) pvy_fp = PB_VY_CAP_FP;
                            pb[i].vy_fp = pvy_fp;
                            break;
                        }
                    }
                    auto_fire_cd = AUTO_FIRE_PERIOD_FR;
                }

                // Move player bullets
                for (i = 0; i < MAX_PB; i++) {
                    if (!pb[i].active) continue;

                    pb[i].y_fp += pb[i].vy_fp;
                    i32 by = fp_to_i32(pb[i].y_fp);
                    if (by < 0) pb[i].active = 0;
                }



                // ---- wave clear? ----
                if (alive_count == 0) {
                    level++;

                    clear_player_bullets(pb);
                    for (i = 0; i < MAX_AB; i++) ab[i].active = 0;

                    pattern = (u8)((level - 1u) % 3u);
                    alive_count = init_aliens_drop(aliens, pattern);
                    wave_entering = 1;

                    for (r = 0; r < A_ROWS; r++)
                        for (c = 0; c < A_COLS; c++)
                            expl_fr[r][c] = 0;

                    a_dir = 1;
                    a_step_cd = A_STEP_FR_START;
                    edge_bounces = 0;

                    wave_pause = (u8)WAVE_PAUSE_FR;
                    ufo.active = 0;
                }

                // ---- WAVE ENTRY ----
                if (wave_entering) {
                    u8 done = 1;
                    for (r = 0; r < A_ROWS; r++) {
                        i16 ty = alien_target_y(r);
                        for (c = 0; c < A_COLS; c++) {
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
                    i32 ux = fp_to_i32(ufo.x_fp);
                    i32 uy = fp_to_i32(ufo.y_fp);

                    for (bi = 0; bi < MAX_PB; bi++) {
                        if (!pb[bi].active) continue;

                        i32 bx = fp_to_i32(pb[bi].x_fp);
                        i32 by = fp_to_i32(pb[bi].y_fp);

                        if (aabb_i32(bx, by, PB_W, PB_H, ux, uy, UFO_W, UFO_H)) {
                            pb[bi].active = 0;

                            ufo_expl = (u8)UFO_EXP_FRAMES;
                            ufo_expl_x = ux;
                            ufo_expl_y = uy;

                            ufo.active = 0;
                            score += (u32)(UFO_POINTS * (u32)combo_lvl);
                            break;
                        }
                    }
                }

                // Player bullets vs aliens
                for (bi = 0; bi < MAX_PB; bi++) {
                    if (!pb[bi].active) continue;

                    i32 bx = fp_to_i32(pb[bi].x_fp);
                    i32 by = fp_to_i32(pb[bi].y_fp);

                    for (r = 0; r < A_ROWS && pb[bi].active; r++) {
                        for (c = 0; c < A_COLS && pb[bi].active; c++) {
                            if (!aliens[r][c].alive) continue;

                            if (aabb_i32(bx, by, PB_W, PB_H,
                                         (i32)aliens[r][c].x, (i32)aliens[r][c].y, A_W, A_H)) {

                                aliens[r][c].alive = 0;
                                alive_count--;
                                pb[bi].active = 0;

                                expl_fr[r][c] = (u8)EXP_FRAMES;

                                combo_hits++;
                                if ((combo_hits % 2u) == 0u && combo_lvl < (u8)COMBO_MAX_LVL) combo_lvl++;

                                u32 base = 10u + (u32)(r * 2u);
                                score += base * (u32)combo_lvl;
                            }
                        }
                    }
                }

                // movement/we fire to the aliens only when the wave is in position
                if (!wave_entering) {
                    i32 base_start = (i32)A_STEP_FR_START - (i32)((level - 1u) * 1u);
                    if (base_start < A_STEP_FR_MIN) base_start = A_STEP_FR_MIN;

                    i32 fr = base_start - (i32)((50 - (i32)alive_count) / 8);
                    if (fr < A_STEP_FR_MIN) fr = A_STEP_FR_MIN;
                    u8 a_step_fr = (u8)fr;

                    if (a_step_cd > 0) a_step_cd--;
                    if (a_step_cd == 0) {
                        a_step_cd = a_step_fr;

                        u8 hit_edge = 0;
                        for (r = 0; r < A_ROWS; r++) {
                            for (c = 0; c < A_COLS; c++) {
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
                                for (r = 0; r < A_ROWS; r++)
                                    for (c = 0; c < A_COLS; c++)
                                        if (aliens[r][c].alive)
                                            aliens[r][c].y = (i16)(aliens[r][c].y + A_DROP);
                            }
                        } else {
                            i16 dx = a_dir ? 2 : -2;
                            for ( r = 0; r < A_ROWS; r++)
                                for ( c = 0; c < A_COLS; c++)
                                    if (aliens[r][c].alive)
                                        aliens[r][c].x = (i16)(aliens[r][c].x + dx);
                        }

                        i32 fire_pct = 45 + (i32)((level - 1u) * 2u);
                        if (fire_pct > 70) fire_pct = 70;

                        if (rng_chance_percent_u8((u8)fire_pct)) {
                            i8 slot = -1;
                            for (i = 0; i < MAX_AB; i++) if (!ab[i].active) { slot = (i8)i; break; }

                            if (slot >= 0) {
                                u8 col = rng_range_u8(0, A_COLS - 1);
                                i16 sx, sy;
                                if (find_shooter_in_col(aliens, col, &sx, &sy)) {
                                    ab[(u8)slot].active = 1;

                                    i32 ax = (i32)sx + (A_W/2) - (AB_W/2);
                                    i32 ay = (i32)sy + (i32)A_H + 1;

                                    ab[(u8)slot].x_fp = fp_from_i32(ax);
                                    ab[(u8)slot].y_fp = fp_from_i32(ay);

                                    // vy = 2.8 + 0.12*(level-1), cap 4.0
                                    i32 vy_fp = AB_VY_FP + (i32)(level - 1u) * AB_VY_LVL_DELTA_FP;
                                    if (vy_fp > AB_VY_CAP_FP) vy_fp = AB_VY_CAP_FP;
                                    ab[(u8)slot].vy_fp = vy_fp;
                                }
                            }
                        }
                    }

                    // GAME OVER when aliens arrive to our ship line
                    for (r = 0; r < A_ROWS; r++) {
                        for (c = 0; c < A_COLS; c++) {
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
                for (i = 0; i < MAX_AB; i++) {
                    if (!ab[i].active) continue;

                    ab[i].y_fp += ab[i].vy_fp;

                    i32 ax = fp_to_i32(ab[i].x_fp);
                    i32 ay = fp_to_i32(ab[i].y_fp);

                    if (ay > (LCD_H + 10)) { ab[i].active = 0; continue; }

                    if (invuln == 0 &&
                        aabb_i32(ax, ay, AB_W, AB_H, ship_x, SHIP_Y, SHIP_W, SHIP_H)) {

                        combo_lvl = 1;
                        combo_hits = 0;

                        if (lives > 0) lives--;
                        if (lives == 0) {
                            display_end();
                            display_close();
                            return (int)score;
                        }

                        clear_player_bullets(pb);
                        for (k = 0; k < MAX_AB; k++) ab[k].active = 0;

                        ship_x = (LCD_W - SHIP_W) / 2;
                        invuln = (u8)RESPAWN_INVULN_FR;
                        wave_pause = (u8)RESPAWN_PAUSE_FR;
                        break;
                    }
                }
            }
        }

        // ---------- DRAW ----------
        clear_screen();

        // IDLE:
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

            // ship follows the sensor
            {
                i32 sx = ship_x;
                if (sx < 0) sx = 0;
                if (sx > (LCD_W - SHIP_W)) sx = (LCD_W - SHIP_W);
                draw_texture((u8)sx, (u8)SHIP_Y, ship_tex);
            }

            // preview aliens (pattern 0)
            for (r = 0; r < A_ROWS; r++) {
                for (c = 0; c < A_COLS; c++) {
                    if (!alien_spawn_alive(0, r, c)) continue;
                    i32 ax = (i32)(A_LEFT + (i32)c * (A_W + A_GAPX));
                    i32 ay = (i32)alien_target_y(r);
                    TextureHandle texp = ((r & 1u) == 0u) ? alien1_tex : alien2_tex;
                    if (ax >= 0 && ay >= 0 && ax + A_W <= LCD_W && ay + A_H <= LCD_H)
                        draw_texture((u8)ax, (u8)ay, texp);

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
            i32 ux = fp_to_i32(ufo.x_fp);
            i32 uy = fp_to_i32(ufo.y_fp);
            if (ux >= -UFO_W && ux <= 255 && uy >= 0 && uy <= 255) {
                i32 dx = ux;
                if (dx < 0) dx = 0;
                if (dx > 255) dx = 255;
                draw_texture((u8)dx, (u8)uy, ufo_tex);
            }
        }
        // UFO explosion
        if (ufo_expl > 0) {
            draw_explosion_at(ufo_expl_x, ufo_expl_y, PAL_UFO, ufo_expl);
        }

        // Player ship (blink durante invuln)
        {
            u8 draw_ship = 1;
            if (invuln > 0) draw_ship = (((invuln >> 2) & 1u) == 0u) ? 1u : 0u;

            if (draw_ship) {
                i32 sx = ship_x;
                if (sx < 0) sx = 0;
                if (sx > (LCD_W - SHIP_W)) sx = (LCD_W - SHIP_W);
                draw_texture((u8)sx, (u8)SHIP_Y, ship_tex);
            }
        }

        // Player bullets
        for (i = 0; i < MAX_PB; i++) {
            if (!pb[i].active) continue;

            i32 bx = fp_to_i32(pb[i].x_fp);
            i32 by = fp_to_i32(pb[i].y_fp);

            if (!rect_on_screen(bx, by, PB_W+1, PB_H+1)) continue;

            draw_rectangle(bx, by, PB_W, PB_H, T_THREE);
        }



        // Alien bullets
        // Alien bullets
        for (i = 0; i < MAX_AB; i++) {
            if (!ab[i].active) continue;

            i32 bx = fp_to_i32(ab[i].x_fp);
            i32 by = fp_to_i32(ab[i].y_fp);

            if (!rect_on_screen(bx, by, AB_W+1, AB_H+1)) continue;
            draw_rectangle(bx, by, AB_W, AB_H, T_THREE);
        }


        // Aliens
        for (r = 0; r < A_ROWS; r++) {
            for (c = 0; c < A_COLS; c++) {
                if (!aliens[r][c].alive) continue;

                i32 ax = (i32)aliens[r][c].x;
                i32 ay = (i32)aliens[r][c].y;

                if (ax < 0 || ay < 0) continue;
                if (ax + A_W > LCD_W) continue;
                if (ay + A_H > LCD_H) continue;

                TextureHandle texp = (aliens[r][c].kind == 0) ? alien1_tex : alien2_tex;
                draw_texture((u8)ax, (u8)ay, texp);
            }
        }


        // Explosions above the aliens
        for (r = 0; r < A_ROWS; r++) {
            for (c = 0; c < A_COLS; c++) {
                if (expl_fr[r][c] == 0) continue;

                // alien is dead, but x/y are still valid: we use them to set the center of our explosions
                i32 ax = (i32)aliens[r][c].x;
                i32 ay = (i32)aliens[r][c].y;
                u8 pal = (aliens[r][c].kind == 0) ? PAL_ALIEN1 : PAL_ALIEN2;

                if (!rect_on_screen(ax - EXP_MAX_R, ay - EXP_MAX_R, A_W + 2*EXP_MAX_R, A_H + 2*EXP_MAX_R)) continue;

                draw_explosion_at(ax, ay, pal, expl_fr[r][c]);
            }
        }

        display_end();
    }

    display_close();
    return (int)score;
}

// ------------------------------------------------------------
// MAIN DI TEST
// int main(void) {
//     int score = space_invaders_game();


//     // su embedded reale spesso meglio evitare printf (o metterlo dietro debug)
// #ifdef DEBUG
//     printf("Score: %d\n", score);
// #endif


//     return score;
// }

