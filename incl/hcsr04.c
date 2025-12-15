#include "hcsr04.h"
#include <msp432p401r.h>
volatile uint32_t echo_ticks;
volatile uint8_t echo_arrived = 1;
void delay_ms_60()
{
    // SMCLK = 3 MHz

    TIMER_A0->CTL = TIMER_A_CTL_SSEL__SMCLK | TIMER_A_CTL_ID__8 | TIMER_A_CTL_MC__UP | TIMER_A_CTL_CLR;
    TIMER_A0->CCR[0] = 22500;               // 3,000,000 / 8 * 0.06 = 22500
    TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE; // interrupt enable
}
void TA0_0_IRQHandler(void)
{
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG; // clear flag
    TIMER_A0->CTL = 0;                         // stop timer
}
void init_hcsr04()
{
    P6->DIR |= BIT1;  // TRIG as output
    P6->OUT &= ~BIT1; // start low

    P7->DIR &= ~BIT7; // P7.7 as input
    P7->SEL0 |= BIT7; // P7.7 = TA1.1 CCI1A
    P7->SEL1 &= ~BIT7;

    TIMER_A1->CTL = TIMER_A_CTL_TASSEL_2 | // SMCLK
                    TIMER_A_CTL_MC_2 |     // continuous mode
                    TIMER_A_CTL_CLR;       // clear timer

    TIMER_A1->CCTL[1] &= ~TIMER_A_CCTLN_CCIFG; // clears flag just in case

    TIMER_A1->CCTL[1] = TIMER_A_CCTLN_CM_3 |       // rising + falling edges
                        TIMER_A_CCTLN_CCIS__CCIA | // uses CCIxA -> So P7.7
                        TIMER_A_CCTLN_CAP |        // capture mode
                        TIMER_A_CCTLN_SCS |        // synchronous capture
                        TIMER_A_CCTLN_CCIE;        // enable interrupt

    NVIC_EnableIRQ(TA1_N_IRQn);
}

//synchronous trigger
float trigger_hcsr04(void)
{
    //10 us pulse

    P6->OUT |= BIT1;
    __delay_cycles(30); // 10 us at 3 MHz
    P6->OUT &= ~BIT1;

    while (!echo_arrived)
    {
        __WFI(); // sleep until interrupt
    }
    echo_arrived = 0;

    float pulse_us = echo_ticks * 0.333f;
    float distance_cm = pulse_us / 58.0f;
    return distance_cm;

}
void TA1_N_IRQHandler(void)
{

    if (TIMER_A1->CCTL[1] & TIMER_A_CCTLN_CCIFG)
    {

        if (P7->IN & BIT7)
        {
            TIMER_A1->CTL |= TIMER_A_CTL_CLR;
        }
        else
        {
            echo_ticks = TIMER_A1->CCR[1];
            echo_arrived = 1;
        }

        TIMER_A1->CCTL[1] &= ~TIMER_A_CCTLN_CCIFG;
    }
}
