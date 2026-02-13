#include "joystick.h"
#include <msp432p401r.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

volatile uint8_t conversion_complete = 0;
volatile uint8_t buttonPressed = 0;
static uint16_t resultsBuffer[2];

joystick_t read_joystick()
{
    
    ADC14_toggleConversionTrigger();
    while (!conversion_complete)
        __WFI(); // Wait for interrupt
    conversion_complete = 0;
    // Read values
    uint16_t x = resultsBuffer[0];
    uint16_t y = resultsBuffer[1];

    if (buttonPressed)
    {
        buttonPressed = 0;
        return JS_BUTTON;
    }

    // Determine direction (idle is approx 8000)

    if (x < 6000)
    {
        if (x < y)
            return JS_LEFT;
        else
            return JS_DOWN;
    }
    else if (x > 10000)
    {
        if (x > y)
            return JS_RIGHT;
        else
            return JS_UP;
    }
    else
    {
        if (y < 6000)
            return JS_UP;
        else if (y > 10000)
            return JS_DOWN;
    }
    return JS_NONE;
}
void gpio_init_js(void)
{
    // X-axis: P6.0 → ADC
    P6SEL0 |= BIT0;
    P6SEL1 |= BIT0;

    // Y-axis: P4.4 → ADC
    P4SEL0 |= BIT4;
    P4SEL1 |= BIT4;

    // Button: P4.1 input with pull-up
    P4DIR &= ~BIT1;
    P4REN |= BIT1;
    P4OUT |= BIT1;
}

void adc_init_js(void)
{
    /* Configures Pin 6.0 and 4.4 as ADC input */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6, GPIO_PIN0, GPIO_TERTIARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN4, GPIO_TERTIARY_MODULE_FUNCTION);

    /* Initializing ADC (ADCOSC/64/8) */
    /* 25 Mhz / 64 / 8 = 48.828 kHz */
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_ADCOSC, ADC_PREDIVIDER_64, ADC_DIVIDER_8, 0);

    ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM1, false);

    
    ADC14_configureConversionMemory(ADC_MEM0,
                                    ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                    ADC_INPUT_A15, ADC_NONDIFFERENTIAL_INPUTS);
    ADC14_configureConversionMemory(ADC_MEM1,
                                    ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                    ADC_INPUT_A9, ADC_NONDIFFERENTIAL_INPUTS);

    ADC14_enableInterrupt(ADC_INT1);

    Interrupt_enableInterrupt(INT_ADC14);
    Interrupt_enableMaster();

    ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

    /* Triggering the start of the sample */
    ADC14_enableConversion();
}

void ADC14_IRQHandler(void)
{
    uint64_t status;

    status = ADC14_getEnabledInterruptStatus();
    ADC14_clearInterruptFlag(status);

    /* ADC_MEM1 conversion completed */
    if (status & ADC_INT1)
    {
        /* Store ADC14 conversion results */
        resultsBuffer[0] = ADC14_getResult(ADC_MEM0);
        resultsBuffer[1] = ADC14_getResult(ADC_MEM1);

        /* Determine if JoyStick button is pressed */

        if (!(P4IN & GPIO_PIN1))
            buttonPressed = 1;
    }
    conversion_complete = 1;
}

void read_button()
{
    uint8_t pressed = !(P4IN & BIT1); // 1 = pressed
}
void js_init(void)
{
    gpio_init_js();
    adc_init_js();
}
