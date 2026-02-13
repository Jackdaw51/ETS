#include <stdio.h>

#include "display/display.h"
#include "sprites/palettes.h"
#include "stddef.h"

// ------------------------------------------------------------
// LCD size (da vostro example.c: H=128; W tipico = 160)
#define LCD_W 160
#define LCD_H 128

// Paddle settings
#define PADDLE_W 28
#define PADDLE_H 5
#define PADDLE_Y (LCD_H - 12)

// Ball settings
#define BALL_S 4

// Speed tuning
#define SPEED_START  1.25f
#define SPEED_UP_MUL 1.03f
#define SPEED_MAX    5.0f


// Joystick -> paddle tuning (non tocca fisica/logica del gioco)
#define PADDLE_STEP 3          // pixel per tick

typedef enum {
    IDLE = 0,       // aspetta movimento barra
    PLAYING = 1,    // partita in corso
    GAMEOVER = 2    // mostra overlay e poi esce
} GameState;

typedef struct {
    f32 x, y;
    f32 vx, vy;
} Ball;

// ------------------------------------------------------------
// Helpers

static inline f32 clampf(f32 v, f32 lo, f32 hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static inline f32 absf(f32 v) { return (v < 0.0f) ? -v : v; }

// ------------------------------------------------------------
// Joystick helper
// PC: WASD/ENTER in HOLD -> movimento continuo
// MSP432: usa get_joystick()
static inline joystick_t joystick_action(void) {
#if defined(__MSP432P401R__) || defined(TARGET_IS_MSP432P4XX) || defined(__TI_COMPILER_VERSION__)
    return get_joystick();
#else
    if (IsKeyDown(KEY_ENTER)) return JS_BUTTON;
    if (IsKeyDown(KEY_W))     return JS_UP;
    if (IsKeyDown(KEY_A))     return JS_LEFT;
    if (IsKeyDown(KEY_S))     return JS_DOWN;
    if (IsKeyDown(KEY_D))     return JS_RIGHT;
    return JS_NONE;
#endif
}

// ------------------------------------------------------------
// 7-seg digits (no font needed)
// Segment mapping: bit6..bit0 = A,B,C,D,E,F,G (scelto così per semplicità)

static void draw_seg_digit(i16 x, i16 y, i16 s, i16 thick, u8 d, TWOS_COLOURS col) {
    // masks for digits 0..9 (A B C D E F G)
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

    if (d > 9) return;
    u8 m = mask[d];

    // geometry: width = 3*s, height = 5*s
    i16 w = (i16)(3 * s);
    i16 h = (i16)(5 * s);

    // A (top)    -> bit6
    if (m & (1u<<6)) draw_rectangle_p((i32)(x + thick), (i32)y, (i32)(w - 2*thick), (i32)thick, col, BW_INDEX);
    // B (upper-right) -> bit5
    if (m & (1u<<5)) draw_rectangle_p((i32)(x + w - thick), (i32)(y + thick), (i32)thick, (i32)((h/2) - thick), col, BW_INDEX);
    // C (lower-right) -> bit4
    if (m & (1u<<4)) draw_rectangle_p((i32)(x + w - thick), (i32)(y + (h/2)), (i32)thick, (i32)((h/2) - thick), col, BW_INDEX);
    // D (bottom) -> bit3
    if (m & (1u<<3)) draw_rectangle_p((i32)(x + thick), (i32)(y + h - thick), (i32)(w - 2*thick), (i32)thick, col, BW_INDEX);
    // E (lower-left) -> bit2
    if (m & (1u<<2)) draw_rectangle_p((i32)x, (i32)(y + (h/2)), (i32)thick, (i32)((h/2) - thick), col, BW_INDEX);
    // F (upper-left) -> bit1
    if (m & (1u<<1)) draw_rectangle_p((i32)x, (i32)(y + thick), (i32)thick, (i32)((h/2) - thick), col, BW_INDEX);
    // G (middle) -> bit0
    if (m & (1u<<0)) draw_rectangle_p((i32)(x + thick), (i32)(y + (h/2) - (thick/2)), (i32)(w - 2*thick), (i32)thick, col, BW_INDEX);
}

static void draw_digits_7seg(i16 x, i16 y, i16 s, i16 thick,
                             const u8* digits, u8 n, TWOS_COLOURS col) {
    // digits[] è già MSB -> LSB
    const i16 step = (i16)(3*s + s); // digit width + gap
    for (u8 i = 0; i < n; ++i) {
        draw_seg_digit(x, y, s, thick, digits[i], col);
        x = (i16)(x + step);
    }
}

// Cache: calcolo delle cifre solo quando score cambia
static u8 score_digits[6];
static u8 score_n = 1;

static void score_to_digits(u16 v) {
    // produce digits in MSB->LSB dentro score_digits[], aggiorna score_n
    if (v == 0) {
        score_digits[0] = 0;
        score_n = 1;
        return;
    }

    // prima in reverse (LSB->MSB) in un buffer piccolo
    u8 tmp[6];
    u8 n = 0;
    while (v > 0 && n < 6) {
        tmp[n++] = (u8)(v % 10);
        v /= 10;
    }

    // reverse in score_digits (MSB->LSB)
    score_n = n;
    for (u8 i = 0; i < n; ++i) {
        score_digits[i] = tmp[(u8)(n - 1 - i)];
    }
}

// ------------------------------------------------------------
// Game init/reset (solo valori, NON riavvia in automatico)

static void init_ball(Ball* b) {
    b->x  = (LCD_W * 0.5f) - (BALL_S * 0.5f);
    b->y  = (LCD_H * 0.5f) - (BALL_S * 0.5f);
    b->vx = SPEED_START;
    b->vy = -SPEED_START;
}

// ------------------------------------------------------------
// FUNZIONE GIOCO: ritorna lo score finale
// Nel progetto finale la chiamerai dal main "hub".

int pong_wall_game(void) {
    display_init_lcd();
    set_palette(PONG_CUSTOM_PALETTE_INDEX);

    // background color index (come nel vostro example)
    set_screen_color(T_ONE);

    Ball ball;
    u16 score = 0;
    score_to_digits(score);

    GameState state = IDLE;

    // Paddle
    i16 paddle_x = (i16)((LCD_W - PADDLE_W) / 2); // parti centrato

    init_ball(&ball);

    while (!window_should_close()) {
        display_begin();

        // ---------------- INPUT (joystick)
        joystick_t action = joystick_action();

        if (action == JS_LEFT)  paddle_x -= PADDLE_STEP;
        if (action == JS_RIGHT) paddle_x += PADDLE_STEP;

        // clamp paddle
        if (paddle_x < 0) paddle_x = 0;
        if (paddle_x > (LCD_W - PADDLE_W)) paddle_x = (LCD_W - PADDLE_W);

        // ---------------- STATE UPDATE
        if (state == IDLE) {
            // come prima: “parti quando l’utente si muove”
            if (action != JS_NONE) {
                state = PLAYING;
                score = 0;
                score_to_digits(score);
                init_ball(&ball);
            }
        } else if (state == PLAYING) {
            // Move ball
            ball.x += ball.vx;
            ball.y += ball.vy;

            // Left / right walls
            if (ball.x <= 0.0f) {
                ball.x = 0.0f;
                ball.vx = -ball.vx;
            } else if (ball.x >= (f32)(LCD_W - BALL_S)) {
                ball.x = (f32)(LCD_W - BALL_S);
                ball.vx = -ball.vx;
            }

            // Top wall ("muro" da colpire)
            if (ball.y <= 0.0f) {
                ball.y = 0.0f;
                ball.vy = -ball.vy;

                // score
                if (score < 999999u) score++; // cap soft (così non esplode la visualizzazione)
                score_to_digits(score);

                // Speed up leggero mantenendo segno
                f32 spx = absf(ball.vx) * SPEED_UP_MUL;
                f32 spy = absf(ball.vy) * SPEED_UP_MUL;
                if (spx > SPEED_MAX) spx = SPEED_MAX;
                if (spy > SPEED_MAX) spy = SPEED_MAX;

                ball.vx = (ball.vx < 0.0f) ? -spx : spx;
                ball.vy = (ball.vy < 0.0f) ? -spy : spy;
            }

            // Paddle collision (solo se la palla sta scendendo)
            if (ball.vy > 0.0f) {
                i16 bx = (i16)ball.x;
                i16 by = (i16)ball.y;

                // AABB Ball
                i16 bl = bx;
                i16 br = (i16)(bx + BALL_S);
                i16 bt = by;
                i16 bb = (i16)(by + BALL_S);

                // AABB Paddle
                i16 pl = paddle_x;
                i16 pr = (i16)(paddle_x + PADDLE_W);
                i16 pt = (i16)PADDLE_Y;
                i16 pb = (i16)(PADDLE_Y + PADDLE_H);

                u8 overlap =
                    (br >= pl) && (bl <= pr) &&
                    (bb >= pt) && (bt <= pb);

                if (overlap) {
                    // rimbalzo verso l'alto
                    ball.y = (f32)(PADDLE_Y - BALL_S);
                    ball.vy = -absf(ball.vy);

                    // "spin": cambia vx in base a dove colpisce la racchetta
                    f32 paddle_center = (f32)paddle_x + (PADDLE_W * 0.5f);
                    f32 ball_center   = ball.x + (BALL_S * 0.5f);

                    f32 hit = (ball_center - paddle_center) / (PADDLE_W * 0.5f);
                    hit = clampf(hit, -1.0f, 1.0f);

                    ball.vx += hit * 0.8f;
                    ball.vx = clampf(ball.vx, -SPEED_MAX, SPEED_MAX);
                }
            }

            if (ball.y > (f32)LCD_H) {
                display_end();
                break;   // esci subito, ritorna score
            }

        } else { }

        // ---------------- DRAW
        clear_screen();

        // Riga in alto per indicare il muro
        draw_rectangle(0, 0, LCD_W, 2, T_THREE);

        // Paddle
        draw_rectangle((i32)paddle_x, (i32)PADDLE_Y, PADDLE_W, PADDLE_H, T_THREE);

        // Ball
        if (state == PLAYING || state == GAMEOVER) {
            draw_rectangle((i32)(i16)ball.x, (i32)(i16)ball.y, BALL_S, BALL_S, T_TWO);
        }

        // Score in alto a destra, solo se non GAMEOVER (right-aligned)
        if (state != GAMEOVER) {
            const i16 s = 3;
            const i16 thick = 2;

            const i16 digit_w = (i16)(3 * s);
            const i16 gap = s;
            const i16 total_w = (i16)(score_n * digit_w + (score_n - 1) * gap);

            const i16 margin_r = 2;
            const i16 score_x = (i16)(LCD_W - margin_r - total_w);
            const i16 score_y = 2;

            draw_digits_7seg(score_x, score_y, s, thick, score_digits, score_n, T_ONE);
        }

        // Overlay IDLE
        if (state == IDLE) {
            draw_rectangle(LCD_W - 10, 2, 6, 2, T_THREE); // mini “ready” indicator
        }

        display_end();
    }

    display_close();
    return (int)score;
}

// ------------------------------------------------------------
// MAIN DI TEST (se vuoi compilarlo standalone)
// Nel progetto finale puoi togliere il main e chiamare pong_wall_game() dal tuo hub.

int main(void) {
    int score = pong_wall_game();
    printf("Score: %d\n", score);
    return score; // per debug: ritorna score come exit code
}
