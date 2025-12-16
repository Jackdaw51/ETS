#include <msp432p401r.h>
#include "incl/hcsr04.h"
#include "incl/IRrecv.h"
#include "incl/joystick.h"

volatile uint8_t ta_done = 0;

void init_red(void);
void init_green(void);
void sleep_ms(uint32_t ms);

int main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD; // stop watchdog timer
    NVIC_EnableIRQ(TA0_0_IRQn);
    NVIC_EnableIRQ(TA1_0_IRQn);
    __enable_irq();

    init_green();
    init_red();
    init_hcsr04();
    init_timerA2();
    init_gpio_ir();

    js_init();

    joystick_t a;

    while (1)
    {
        // float distance_cm = trigger_hcsr04();

        P1->OUT &= ~BIT0; // Turn off Red LED
        P2->OUT &= ~BIT1; // Turn off Green LED

        // if (distance_cm > 100)
        // {
        //     P1->OUT ^= BIT0; // Toggle Red LED state
        // }
        // else
        // {
        //     P2->OUT ^= BIT1; // Toggle Green LED state
        // }
        //        __WFI(); // Enter low-power mode until an interrupt occurs
        a = read_joystick();
        switch (a)
        {
        case JS_UP:
            P1->OUT ^= BIT0;
            break;
        case JS_LEFT:
            P2->OUT ^= BIT1;
            break;
        case JS_BUTTON:
            P1->OUT ^= BIT0;
            P2->OUT ^= BIT1;
            break;
        default:
            break;
        }

        sleep_ms(10);
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
void sleep_ms(uint32_t ms)
{
    ta_done = 0;

    TIMER_A1->CTL = TIMER_A_CTL_SSEL__SMCLK | // SMCLK
                    TIMER_A_CTL_MC__STOP |   // Stop timer
                    TIMER_A_CTL_CLR;         // Clear timer

    TIMER_A1->CCR[0] = ms * 3 - 1;             // 3 MHz → 10 ms
    TIMER_A1->CCTL[0] = TIMER_A_CCTLN_CCIE;   // Enable CCR0 interrupt

  

    TIMER_A1->CTL |= TIMER_A_CTL_MC__UP;      // Start timer in up mode

    while (!ta_done)
        __WFI();                              // Sleep
    ta_done = 0;

    TIMER_A1->CTL = TIMER_A_CTL_MC__STOP;     // Stop timer
}

void TA1_0_IRQHandler(void)
{
    TIMER_A1->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG; // Clear interrupt flag
    ta_done = 1;
}

