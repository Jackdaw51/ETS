#include "timers.h"

// Timer A3 used in hcsr04.c
// Timer A2_0_handler defined in hcsr04.c
// Timer A1_0_handler defined in hcsr04.c
// Using Timer A0 for general purpose delays

volatile uint8_t ta_done = 0;

void init_A0()
{

    TIMER_A0->CTL = TIMER_A_CTL_TASSEL_1 | // ACLK
                    TIMER_A_CTL_MC_1 |     // up mode
                    TIMER_A_CTL_CLR;       // clear timer

    TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE; // enable interrupt

    NVIC_EnableIRQ(TA0_0_IRQn);
}
void init_A1()
{
    TIMER_A1->CCR[0] = 16384; // 500 ms at 32.768 kHz
    TIMER_A1->CTL = TIMER_A_CTL_TASSEL_1 | // ACLK
                    TIMER_A_CTL_MC_1 |     // up mode
                    TIMER_A_CTL_CLR;       // clear timer
    TIMER_A1->CTL &= ~TIMER_A_CTL_MC_MASK; // stop timer
    TIMER_A1->CCTL[0] = TIMER_A_CCTLN_CCIE; // enable interrupt

    NVIC_EnableIRQ(TA1_0_IRQn);
}

void init_A2(void)
{
    TIMER_A2->CTL = TIMER_A_CTL_TASSEL_1 | // ACLK
                    TIMER_A_CTL_MC_1 |     // up mode
                    TIMER_A_CTL_CLR;       // clear timer

    TIMER_A2->CCTL[0] = TIMER_A_CCTLN_CCIE; // enable interrupt

    NVIC_EnableIRQ(TA2_0_IRQn);
}

void init_A3(void)
{
    TIMER_A3->CTL =
        TIMER_A_CTL_SSEL__ACLK |     // ACLK
        TIMER_A_CTL_MC__CONTINUOUS | // continuous mode
        TIMER_A_CTL_CLR;             // clear timer
}

void sleep_ms(uint32_t ms)
{
    uint32_t ticks = (ms * 32768) / 1000;
    TIMER_A0->CCR[0] = ticks - 1;

    TIMER_A0->CTL |= TIMER_A_CTL_MC__UP | TIMER_A_CTL_CLR; // start timer and clear
    while (!ta_done)
    {
        __WFI();
    }
    ta_done = 0;

    TIMER_A0->CTL &= ~TIMER_A_CTL_MC_MASK; // Stop timer
}

void TA0_0_IRQHandler(void)
{
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG; // clear flag
    ta_done = 1;
}
