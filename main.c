#include <msp432p401r.h>
#include "incl/hcsr04.h"
//#include "incl/IRrecv.h"
#include "incl/joystick.h"
#include "incl/screen.h"
#include "games/game_files/example.h"
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>


volatile uint8_t ta_done = 0;

void init_red(void);
void init_green(void);
void sleep_ms(uint32_t ms);


volatile uint32_t myMCLK = 0;
volatile uint32_t mySMCLK = 0;

int main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD; // stop watchdog timer
    NVIC_EnableIRQ(TA0_0_IRQn);
    NVIC_EnableIRQ(TA3_0_IRQn);
    __enable_irq();

    init_green();
    init_red();
    init_hcsr04();

    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);

    // 2. Connect MCLK (CPU) and SMCLK (SPI) to the 12 MHz DCO
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    //init_timerA2();
    //init_gpio_ir();

    js_init();

    screen_init();

    //__WFI();

    joystick_t a;

    while (1)
    {

        // P1->OUT &= ~BIT0; // Turn off Red LED
        // P2->OUT &= ~BIT1; // Turn off Green LED

        // /*
        // float distance_cm = trigger_hcsr04();

        // if (distance_cm > 100)
        // {
        //     P1->OUT ^= BIT0; // Toggle Red LED state
        // }
        // else
        // {
        //     P2->OUT ^= BIT1; // Toggle Green LED state
        // }
        // //__WFI(); // Enter low-power mode until an interrupt occurs
        // */
        // a = read_joystick();
        // switch (a)
        // {
        // case JS_DOWN:
        //     P1->OUT ^= BIT0;
        //     break;
        // case JS_RIGHT:
        //     P2->OUT ^= BIT1;
        //     break;
        // case JS_BUTTON:
        //     P1->OUT ^= BIT0;
        //     P2->OUT ^= BIT1;
        //     break;
        // default:
        //     break;
        // }
         //LCD_FillColor(0xF800);
         //LCD_FillColor(0x0000);
        //myMCLK = CS_getMCLK();   // Returns frequency in Hz (e.g., 48000000)
        //mySMCLK = CS_getSMCLK(); // Returns frequency in Hz
        m_example();
    }
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

//up to around 20 ms
void sleep_ms(uint32_t ms)
{
    ta_done = 0;

    TIMER_A3->CTL = TIMER_A_CTL_CLR;
    TIMER_A3->CCTL[0] = TIMER_A_CCTLN_CCIE;   // Enable CCR0 interrupt
    TIMER_A3->CCR[0] = ms * 3000 - 1;             // 3 MHz → 10 ms
    TIMER_A3->CTL = TIMER_A_CTL_SSEL__SMCLK | // SMCLK
                    TIMER_A_CTL_MC__UP;


    while (!ta_done)
        __WFI();                              // Sleep
    ta_done = 0;

    TIMER_A3->CTL = TIMER_A_CTL_MC__STOP;     // Stop timer
}

void TA3_0_IRQHandler(void)
{
    TIMER_A3->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG; // Clear interrupt flag
    ta_done = 1;
}

