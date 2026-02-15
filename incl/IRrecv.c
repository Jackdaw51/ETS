#include "msp432p401r.h"
#include "IRrecv.h"

#define IR_PORT P6
#define IR_PIN BIT3

volatile uint32_t last_ts = 0;
volatile uint8_t ir_in_progress = 0;
volatile uint8_t ir_bit_index = 0;
volatile uint32_t ir_data = 0;
volatile uint8_t ir_almost_ready = 0;
void handle_valley(uint32_t ticks);
void handle_peak(uint32_t ticks);
void init_timerA2(void)
{
    TIMER_A2->CTL =
        TIMER_A_CTL_SSEL__SMCLK |    // SMCLK ~3 MHz
        TIMER_A_CTL_MC__CONTINUOUS | // continuous mode
        TIMER_A_CTL_CLR;             // clear timer
}
void init_gpio_ir(void)
{

    IN_PORT->DIR &= ~IN_PIN;  // input
    IN_PORT->SEL0 &= ~IN_PIN; // GPIO function
    IN_PORT->SEL1 &= ~IN_PIN; // GPIO function
    IN_PORT->REN &= ~IN_PIN;  // no pull resistor

    IN_PORT->IES |= IN_PIN; // interrupt on falling edge
    IN_PORT->IFG = 0;
    IN_PORT->IE |= IN_PIN;

    NVIC_EnableIRQ(PORT6_IRQn);
}
void PORT6_IRQHandler(void)
{
    if (IN_PORT->IFG & IN_PIN)
    {
        IN_PORT->IFG &= ~IN_PIN; // clear flag

        if (!ir_in_progress)
        {
            ir_in_progress = 1;
            TIMER_A2->CTL |= TIMER_A_CTL_CLR;
            IN_PORT->IES &= ~IN_PIN;
            return;
        }

        uint32_t now = TIMER_A2->R;
        uint32_t delta = now - last_ts;
        last_ts = now;

        uint8_t level = (IN_PORT->IN & IN_PIN) ? 1 : 0;

        if (level)
            handle_valley(delta);
        else
            handle_peak(delta);

        IN_PORT->IES ^= IN_PIN; // toggle edge detection
    }
}
void handle_valley(uint32_t ticks)
{
    if (ir_almost_ready)
    {
        uint32_t us = ticks / 3;
        if(us > 3000){
            return;
        }

        ir_almost_ready = 0;
        ir_in_progress = 0;
        last_ts = 0;
        ir_bit_index = 0;
        

        // for now just reset data
        ir_data = 0;


        IN_PORT->IES |= IN_PIN; //set on rising, later will be set to falling
        return;
    }
}

void handle_peak(uint32_t ticks)
{
    if (ir_almost_ready)
        return;

    uint32_t us = ticks / 3;
    
    if(us > 20000)
    {
        ir_almost_ready = 1;
        return;
    }

    if (us > 3000)
        return;

    ir_bit_index++;

    if (us > 1000)
        ir_data = (ir_data << 1) | 1; // bit 1
    else
        ir_data = (ir_data << 1); // bit 0
}
