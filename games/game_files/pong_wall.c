// game_files/pong_wall.c

#include <stdio.h>

#include "display/display.h"
#include "sprites/palettes.h"

#include "stddef.h"

// ------------------------------------------------------------
// LCD size (da vostro example.c: H=128; W tipico = 160)
#define LCD_W 160
#define LCD_H 128

// Proximity range (da example.c: 0..1023)
#define PROX_MAX 1023.0f

// Start condition: quanto deve cambiare la proximity per far partire la partita
#define PROX_START_DELTA 8.0f

// Paddle settings
#define PADDLE_W 28
#define PADDLE_H 5
#define PADDLE_Y (LCD_H - 12)

// Ball settings
#define BALL_S 4

// Speed tuning
#define SPEED_START  1.25f   // <-- più lenta di prima
#define SPEED_UP_MUL 1.03f
#define SPEED_MAX    5.0f

// Quanto tempo mostrare il game-over prima di uscire (frame)
#define GAMEOVER_FRAMES 90

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

static i32 count_digits_i32(i32 v) {
    if (v <= 0) return 1;
    i32 n = 0;
    while (v > 0) { n++; v /= 10; }
    return n;
}


static i32 prox_to_paddle_x(f32 prox) {
    // map prox [0..1023] -> [0 .. LCD_W - PADDLE_W]
    f32 ratio = (f32)(LCD_W - PADDLE_W) / PROX_MAX;

    // rounding semplice senza math.h
    i32 x = (i32)(prox * ratio + 0.5f);

    if (x < 0) x = 0;
    if (x > (LCD_W - PADDLE_W)) x = (LCD_W - PADDLE_W);
    return x;
}

// ------------------------------------------------------------
// 7-seg digits (no font needed)
// Segment mapping: bit6..bit0 = A,B,C,D,E,F,G (scelto così per semplicità)

static void draw_seg_digit(i32 x, i32 y, i32 s, i32 thick, int d, TWOS_COLOURS col) {
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

    if (d < 0 || d > 9) return;
    u8 m = mask[d];

    // geometry: width = 3*s, height = 5*s
    i32 w = 3 * s;
    i32 h = 5 * s;

    // A (top)    -> bit6
    if (m & (1<<6)) draw_rectangle_p(x + thick, y, w - 2*thick, thick, col, BW_INDEX);
    // B (upper-right) -> bit5
    if (m & (1<<5)) draw_rectangle_p(x + w - thick, y + thick, thick, (h/2) - thick, col, BW_INDEX);
    // C (lower-right) -> bit4
    if (m & (1<<4)) draw_rectangle_p(x + w - thick, y + (h/2), thick, (h/2) - thick, col, BW_INDEX);
    // D (bottom) -> bit3
    if (m & (1<<3)) draw_rectangle_p(x + thick, y + h - thick, w - 2*thick, thick, col, BW_INDEX);
    // E (lower-left) -> bit2
    if (m & (1<<2)) draw_rectangle_p(x, y + (h/2), thick, (h/2) - thick, col, BW_INDEX);
    // F (upper-left) -> bit1
    if (m & (1<<1)) draw_rectangle_p(x, y + thick, thick, (h/2) - thick, col, BW_INDEX);
    // G (middle) -> bit0
    if (m & (1<<0)) draw_rectangle_p(x + thick, y + (h/2) - (thick/2), w - 2*thick, thick, col, BW_INDEX);

}

static void draw_int_7seg(i32 x, i32 y, i32 s, i32 thick, i32 value, TWOS_COLOURS col) {
    if (value < 0) value = 0;

    int digits[6];
    int n = 0;

    if (value == 0) {
        digits[n++] = 0;
    } else {
        while (value > 0 && n < 6) {
            digits[n++] = value % 10;
            value /= 10;
        }
    }

    // draw most-significant first
    for (int i = n - 1; i >= 0; --i) {
        draw_seg_digit(x, y, s, thick, digits[i], col);
        x += (3*s + s); // digit width + gap
    }
}

// ------------------------------------------------------------
// Game init/reset (solo valori, NON riavvia in automatico)

static void init_ball(Ball* b) {
    b->x = (LCD_W * 0.5f) - (BALL_S * 0.5f);
    b->y = (LCD_H * 0.5f) - (BALL_S * 0.5f);

    // parte verso il "muro" (alto), con un po' di orizzontale
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
    i32 score = 0;
    GameState state = IDLE;
    i32 gameover_countdown = 0;

    // Paddle
    f32 proximity = 0.0f;
    f32 prox0 = 0.0f;
    i32 paddle_x = 0;

    // Prima lettura: baseline per capire quando "inizi a muovere"
    proximity = get_proximity();
    prox0 = proximity;

    // Prepariamo la palla ma in IDLE non si muove finché non parti
    init_ball(&ball);

    while (!window_should_close()) {
        display_begin();


        // ---------------- INPUT
        proximity = get_proximity();
        paddle_x = prox_to_paddle_x(proximity);

        // ---------------- STATE UPDATE
        if (state == IDLE) {
            // parte solo quando cambi abbastanza la proximity rispetto al baseline
            if (absf(proximity - prox0) >= PROX_START_DELTA) {
                state = PLAYING;
                score = 0;
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

                score++;

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
                i32 bx = (i32)ball.x;
                i32 by = (i32)ball.y;

                // AABB Ball
                i32 bl = bx;
                i32 br = bx + BALL_S;
                i32 bt = by;
                i32 bb = by + BALL_S;

                // AABB Paddle
                i32 pl = paddle_x;
                i32 pr = paddle_x + PADDLE_W;
                i32 pt = PADDLE_Y;
                i32 pb = PADDLE_Y + PADDLE_H;

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

                    // hit in [-1..1]
                    f32 hit = (ball_center - paddle_center) / (PADDLE_W * 0.5f);
                    hit = clampf(hit, -1.0f, 1.0f);

                    // aggiusta leggermente vx
                    ball.vx += hit * 0.8f;
                    ball.vx = clampf(ball.vx, -SPEED_MAX, SPEED_MAX);
                }
            }

            // Miss -> Game over
            if (ball.y > (f32)LCD_H) {
                state = GAMEOVER;
                gameover_countdown = GAMEOVER_FRAMES;
            }
        } else { // GAMEOVER
            if (gameover_countdown > 0) gameover_countdown--;
            else {
                // finito: chiude e ritorna il punteggio
                display_end();
                break;
            }
        }

        // ---------------- DRAW
        clear_screen();

        // Riga in alto per indicare il muro
        draw_rectangle(0, 0, LCD_W, 2, T_THREE);

        // Paddle (sempre visibile)
        draw_rectangle(paddle_x, PADDLE_Y, PADDLE_W, PADDLE_H, T_THREE);



        // Ball: solo se non idle (così in idle non “sembra” che la partita sia partita)
        if (state == PLAYING || state == GAMEOVER) {
           draw_rectangle((i32)ball.x, (i32)ball.y, BALL_S, BALL_S, T_TWO);

        }


        if (state != GAMEOVER) {
            i32 s = 3;
            i32 thick = 2;

            i32 digits = count_digits_i32(score);
            i32 digit_w = 3 * s;
            i32 gap = s;
            i32 total_w = digits * digit_w + (digits - 1) * gap;

            i32 margin_r = 2;
            i32 score_x = LCD_W - margin_r - total_w;
            i32 score_y = 2;

            draw_int_7seg(score_x, score_y, s, thick, score, T_ONE);
        }



        // Overlay IDLE
        if (state == IDLE) {
            draw_rectangle(LCD_W - 10, 2, 6, 2, T_THREE); // mini “ready” indicator
        }


        // Overlay game over + score grande
        if (state == GAMEOVER) {
            draw_rectangle_outline_p(12, 32, LCD_W - 24, 64, 2.0f, T_ONE, BW_INDEX);

            // Score grande centrato nel box (opzionale)
            {
                i32 s = 6;
                i32 thick = 3;

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
        }

        display_end();
    }

    display_close();
    return score;
}

// ------------------------------------------------------------
// MAIN DI TEST (se vuoi compilarlo standalone)
// Nel progetto finale puoi togliere il main e chiamare pong_wall_game() dal tuo hub.

int main(void) {
    int score = pong_wall_game();
    printf("Score: %d\n", score);
    return score; // per debug: ritorna score come exit code
}
