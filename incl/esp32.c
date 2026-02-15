#include "esp32.h"
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

#define RX_BUFFER_SIZE 128
volatile uint8_t rxBuffer[RX_BUFFER_SIZE];
volatile uint16_t rxIndex = 0;
volatile uint8_t messageReceived = 0; // Flag to tell main loop data is ready

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

    UART_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    Interrupt_enableInterrupt(INT_EUSCIA2);
}
void EUSCIA2_IRQHandler(void)
{
    uint32_t status = UART_getEnabledInterruptStatus(EUSCI_A2_BASE);

    UART_clearInterruptFlag(EUSCI_A2_BASE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        // Read the character from the buffer
        char receivedChar = UART_receiveData(EUSCI_A2_BASE);

        // Store in buffer if there is space
        if(rxIndex < RX_BUFFER_SIZE - 1)
        {
            rxBuffer[rxIndex] = receivedChar;
            rxIndex++;
        }

        if(receivedChar == '\n')
        {
            rxBuffer[rxIndex] = '\0'; // Null-terminate string
            rxIndex = 0;              // Reset index for next message
            messageReceived = true;   // Tell main loop to process rxBuffer
        }
    }
}