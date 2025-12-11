#include <msp432p401r.h>
#include "incl/hcsr04.h"

void init_red(void);
void init_green(void);

int main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD; // stop watchdog timer
    NVIC->ISER[0] |= 1 << ((TA0_0_IRQn) & 31);

    init_green();
    init_red();
    init_hcsr04();
    while (1)
    {
        float distance_cm = trigger_hcsr04();

        P1->OUT &= ~BIT0; // Turn off Red LED
        P2->OUT &= ~BIT1; // Turn off Green LED

        if (distance_cm > 100)
        {
            P1->OUT ^= BIT0; // Toggle Red LED state
        }
        else
        {
            P2->OUT ^= BIT1; // Toggle Green LED state
        }
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