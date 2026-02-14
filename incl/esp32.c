#include <stdint.h>
#include <stdbool.h>
#include "esp32.h"
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* * UART Configuration for 115200 Baud @ 24MHz SMCLK
 * Calculation (Oversampling Mode):
 * N = 24,000,000 / 115,200 = 208.333
 * UCBR = 13 (Integer part of N/16)
 * UCBRF = 0 (Remainder calculation)
 * UCBRS = 0x49 (Fractional portion)
 */
const eUSCI_UART_ConfigV1 uartConfig = {
    EUSCI_A_UART_CLOCKSOURCE_SMCLK, // SMCLK = 24MHz
    13,                             // UCBR = 13
    0,                              // UCBRF = 0
    0x49,                           // UCBRS = 0x49
    EUSCI_A_UART_NO_PARITY,         // No Parity
    EUSCI_A_UART_LSB_FIRST,         // LSB First
    EUSCI_A_UART_ONE_STOP_BIT,      // One stop bit
    EUSCI_A_UART_MODE,              // UART mode
    EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION};

void transmitString(char *str)
{
    while (*str != '\0')
    {
        UART_transmitData(EUSCI_A2_BASE, *str++);
    }
}
void init_esp32_communication()
{
    /* Configure UART Pins: P3.2 (RX) and P3.3 (TX) */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3,
                                               GPIO_PIN2 | GPIO_PIN3,
                                               GPIO_PRIMARY_MODULE_FUNCTION);

    /* Initialize UART A2 */
    UART_initModule(EUSCI_A2_BASE, &uartConfig);
    UART_enableModule(EUSCI_A2_BASE);
}
