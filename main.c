#include <msp432p401r.h>
#include "incl/joystick.h"
#include "incl/screen.h"
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "incl/timers.h"
#include "games/game_files/main_screen.h"
#include "games/game_files/dino_runner.h"
#include "games/game_files/pong_wall_game.h"
#include "games/game_files/space_invaders.h"
#include "games/game_files/keyboard.h"
#include "incl/esp32.h"

void init_red(void);
void init_green(void);


volatile uint32_t myMCLK = 0;
volatile uint32_t mySMCLK = 0;
volatile extern uint8_t messageReceived;
int main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD; // stop watchdog timer

    uint32_t mclk = CLOCK_SPEED;
    switch (mclk)
    {
    case 12000000:
        CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);
        break;
    case 24000000:
        CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_24);
        break;
    default:

        break;
    }
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    __enable_irq();
    init_A0();
    // init_A1();
    // init_A2();
    // init_A3();

    init_green();
    init_red();

    js_init();
    init_esp32_communication();
    screen_init();
    
    joystick_t a;

    game_loop();
}
void init_red(void)
{
    P1->DIR |= BIT0;  // Set P1.0 as output (Red LED)
    P1->OUT &= ~BIT0; // Initialize Red LED to OFF
}
void init_green(void)
{
    P2->DIR |= BIT1;  // Set P2.1 as output (Green LED)
    P2->OUT &= ~BIT1; // Initialize Green LED to OFF
}
