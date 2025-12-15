#include "joystick.h"
#include <msp432p401r.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

volatile uint8_t conversion_complete = 0;
volatile uint8_t buttonPressed = 0;
static uint16_t resultsBuffer[2];


void read_joystick(uint16_t *x, uint16_t *y, uint8_t *button)
{
    // Start conversion


    // Wait for conversion to complete
    ADC14_toggleConversionTrigger();
    while (!conversion_complete)
        __WFI(); // Wait for interrupt
    conversion_complete = 0;
    ADC14_toggleConversionTrigger();
    while (!conversion_complete)
        __WFI(); // Wait for interrupt
    conversion_complete = 0;
    // Read values
    *x = ADC14->MEM[0]; // X-axis
    *y = ADC14->MEM[1]; // Y-axis
    *button = buttonPressed;
    
    buttonPressed = 0;
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
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_ADCOSC, ADC_PREDIVIDER_64, ADC_DIVIDER_8, 0);

    /* Configuring ADC Memory (ADC_MEM0 - ADC_MEM1 (A15, A9)  with repeat)
     * with internal 2.5v reference */
    ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM1, true);
    ADC14_configureConversionMemory(ADC_MEM0,
                                    ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                    ADC_INPUT_A15, ADC_NONDIFFERENTIAL_INPUTS);

    ADC14_configureConversionMemory(ADC_MEM1,
                                    ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                    ADC_INPUT_A9, ADC_NONDIFFERENTIAL_INPUTS);

    /* Enabling the interrupt when a conversion on channel 1 (end of sequence)
     *  is complete and enabling conversions */
    ADC14_enableInterrupt(ADC_INT0);
    ADC14_enableInterrupt(ADC_INT1);

    /* Enabling Interrupts */
    Interrupt_enableInterrupt(INT_ADC14);
    Interrupt_enableMaster();

    /* Setting up the sample timer to automatically step through the sequence
     * convert.
     */
    ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);

    /* Triggering the start of the sample */
    ADC14_enableConversion();

}

void ADC14_IRQHandler(void)
{
    uint64_t status;

    status = ADC14_getEnabledInterruptStatus();
    ADC14_clearInterruptFlag(status);

    /* ADC_MEM1 conversion completed */
    if(status & ADC_INT1)
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
