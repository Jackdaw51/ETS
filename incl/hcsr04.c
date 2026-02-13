#include "hcsr04.h"
#include "timers.h"
#include <msp432p401r.h>
#define MAX_DISTANCE_CM 400
#define DELAY_TRIGGER CLOCK_SPEED / 100000 // 10 us pulse
volatile uint32_t echo_ticks;
volatile uint8_t echo_arrived = 0;
volatile uint32_t last_ts = 0;
volatile uint32_t now = 0;
static uint32_t delay_trigger = 0;
static uint32_t max_iteration = 100000;
volatile uint8_t ready = 1;
// volatile uint32_t times_intrrpt_called = 0;

#define IN_PORT P3
#define IN_PIN BIT2

#define TRIG_PORT P6
#define TRIG_PIN BIT1

void init_hcsr04()
{
    // trigger
    TRIG_PORT->DIR |= TRIG_PIN;  // TRIG as output
    TRIG_PORT->OUT &= ~TRIG_PIN; // start low

    // 37 ms at mclk.
    max_iteration = (CLOCK_SPEED / 1000) * 37; // 37 ms at mclk
    // echo

    IN_PORT->DIR &= ~IN_PIN;  // input
    IN_PORT->SEL0 &= ~IN_PIN; // GPIO function
    IN_PORT->SEL1 &= ~IN_PIN; // GPIO function
    IN_PORT->REN |= IN_PIN;   // enable pull resistor
    IN_PORT->OUT &= ~IN_PIN;  // pull-down

    IN_PORT->IES &= ~IN_PIN; // interrupt on rising edge
    IN_PORT->IFG = 0;
    IN_PORT->IE |= IN_PIN;

    NVIC_EnableIRQ(PORT3_IRQn);
}

// synchronous trigger
float trigger_hcsr04(void)
{
    echo_arrived = 0;
    if (!ready)
    {
        delay_trigger++;
        return echo_ticks * 0.333f / 58.0f; // last value
    }
    // 10 us pulse
    int i = 0;
    // int cycles = delay_trigger;

    __disable_irq();
    TRIG_PORT->OUT |= TRIG_PIN;
    __delay_cycles(DELAY_TRIGGER);
    TRIG_PORT->OUT &= ~TRIG_PIN;

    __delay_cycles(DELAY_TRIGGER); // Let noise die down before we start listening for echo. Adjust as needed.

    __enable_irq();

    while (!echo_arrived && i < max_iteration)
    {
        i++;
    }
    ready = 0;
    TIMER_A1->CTL |= TIMER_A_CTL_MC__UP | TIMER_A_CTL_CLR;
    if (i == max_iteration)
    {
        return MAX_DISTANCE_CM; // return max distance if timeout
    }

    float pulse_us = echo_ticks * 32.768f;
    float distance_cm = pulse_us / 58.0f;
    return distance_cm;
}

void PORT3_IRQHandler(void)
{
    if(echo_arrived)
    {
        IN_PORT->IFG &= ~IN_PIN; // Clear interrupt flag
        return; // Ignore if we already got an echo
    }
    if (IN_PORT->IFG & IN_PIN)
    {
        // If IES bit is 0, we were waiting for Rising Edge.
        if (!(IN_PORT->IES & IN_PIN))
        {
            TIMER_A3->CTL |= TIMER_A_CTL_CLR | TIMER_A_CTL_MC__CONTINUOUS;
            IN_PORT->IES |= IN_PIN; // Switch to detect Falling Edge (High-to-Low)
        }
        else // IES bit is 1, so we were waiting for Falling Edge
        {
            echo_ticks = TIMER_A3->R;
            echo_arrived = 1;
            IN_PORT->IES &= ~IN_PIN; // Switch back to detect Rising Edge
        }

        IN_PORT->IFG &= ~IN_PIN;
    }
}
// Do 60 ms cooldown
void TA1_0_IRQHandler(void)
{
    TIMER_A1->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG; // clear interrupt flag
    ready = 1;
    TIMER_A1->CTL &= ~TIMER_A_CTL_MC_MASK; // stop timer
}
