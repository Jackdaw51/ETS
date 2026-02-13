#include "light_sensor.h"
#include "timers.h"

/* --- Global Variable for Distance --- */
volatile uint16_t distance_reading = 0;

/* --- I2C Helper Functions --- */
void I2C_Unstick()
{
    // 1. Configure P6.4 (SDA) and P6.5 (SCL) as GPIO first
    P6->SEL0 &= ~(BIT4 | BIT5);
    P6->SEL1 &= ~(BIT4 | BIT5);

    // 2. Set SCL (P6.5) as Output, SDA (P6.4) as Input
    P6->DIR |= BIT5;  // SCL Output
    P6->DIR &= ~BIT4; // SDA Input

    // 3. Toggle SCL 9 times to clear any stuck slave
    int i, d;
    for (i = 0; i < 9; i++)
    {
        P6->OUT |= BIT5; // SCL High
        for (d = 0; d < 100; d++)
            ;             // Small delay
        P6->OUT &= ~BIT5; // SCL Low
        for (d = 0; d < 100; d++)
            ; // Small delay
    }

    // 4. Send a STOP bit sequence (SDA Low -> High while SCL is High)
    P6->DIR |= BIT4;  // SDA Output
    P6->OUT &= ~BIT4; // SDA Low
    P6->OUT |= BIT5;  // SCL High
    for (d = 0; d < 100; d++)
        ;
    P6->OUT |= BIT4; // SDA High (STOP)
}
// Initialize MSP432 I2C (eUSCI_B1) on P6.4/P6.5
void I2C_Init()
{
    // 1. Configure GPIO for I2C (P6.4 SDA, P6.5 SCL)
    P6->SEL0 |= (BIT4 | BIT5);
    P6->SEL1 &= ~(BIT4 | BIT5);

    // 2. Configure eUSCI_B1
    EUSCI_B1->CTLW0 |= EUSCI_B_CTLW0_SWRST;   // Put into software reset
    EUSCI_B1->CTLW0 = EUSCI_B_CTLW0_SWRST |   // Keep in reset
                      EUSCI_B_CTLW0_MODE_3 |  // I2C Mode
                      EUSCI_B_CTLW0_MST |     // Master Mode
                      EUSCI_B_CTLW0_SYNC |    // Sync Mode
                      EUSCI_B_CTLW0_UCSSEL_2; // SMCLK (usually 3MHz default)

    EUSCI_B1->BRW = 2400;    // 100kHz I2C speed
    EUSCI_B1->CTLW0 &= ~EUSCI_B_CTLW0_SWRST; // Release from reset
}

int APDS_Write(uint8_t reg, uint8_t value) {
    EUSCI_B1->I2CSA = APDS9930_ADDR;

    while (EUSCI_B1->STATW & EUSCI_B_STATW_BBUSY);

    // 1. Send START + Address
    EUSCI_B1->CTLW0 |= EUSCI_B_CTLW0_TR | EUSCI_B_CTLW0_TXSTT;

    // 2. Wait for Address Result
    while (!(EUSCI_B1->IFG & EUSCI_B_IFG_TXIFG0) && !(EUSCI_B1->IFG & EUSCI_B_IFG_NACKIFG));

    if (EUSCI_B1->IFG & EUSCI_B_IFG_NACKIFG) {
        EUSCI_B1->CTLW0 |= EUSCI_B_CTLW0_TXSTP; // Force STOP
        EUSCI_B1->IFG &= ~EUSCI_B_IFG_NACKIFG;  // Clear flag
        return -1; // Address NACK (Sensor missing/unplugged)
    }

    // 3. Send Register
    EUSCI_B1->TXBUF = (reg | APDS_CMD);

    // 4. Wait for Register ACK
    while (!(EUSCI_B1->IFG & EUSCI_B_IFG_TXIFG0) && !(EUSCI_B1->IFG & EUSCI_B_IFG_NACKIFG));

    if (EUSCI_B1->IFG & EUSCI_B_IFG_NACKIFG) {
        EUSCI_B1->CTLW0 |= EUSCI_B_CTLW0_TXSTP;
        EUSCI_B1->IFG &= ~EUSCI_B_IFG_NACKIFG;
        return -2; // Register NACK
    }

    // 5. Send Value
    EUSCI_B1->TXBUF = value;

    // 6. Wait for Value ACK
    while (!(EUSCI_B1->IFG & EUSCI_B_IFG_TXIFG0) && !(EUSCI_B1->IFG & EUSCI_B_IFG_NACKIFG));

    if (EUSCI_B1->IFG & EUSCI_B_IFG_NACKIFG) {
        EUSCI_B1->CTLW0 |= EUSCI_B_CTLW0_TXSTP;
        EUSCI_B1->IFG &= ~EUSCI_B_IFG_NACKIFG;
        return -3; // Value NACK
    }

    // 7. Success - Send STOP
    EUSCI_B1->CTLW0 |= EUSCI_B_CTLW0_TXSTP;
    while (EUSCI_B1->CTLW0 & EUSCI_B_CTLW0_TXSTP);

    return 0;
}

// Read 16-bit Proximity Data
uint16_t APDS_ReadProximity()
{
    uint8_t lowByte, highByte;

    // Step 1: Write the address we want to read from (PDATAL)
    EUSCI_B1->I2CSA = APDS9930_ADDR;
    while (EUSCI_B1->STATW & EUSCI_B_STATW_BBUSY)
        ;

    EUSCI_B1->CTLW0 |= EUSCI_B_CTLW0_TR;    // Tx Mode
    EUSCI_B1->CTLW0 |= EUSCI_B_CTLW0_TXSTT; // Send START
    while (!(EUSCI_B1->IFG & EUSCI_B_IFG_TXIFG0))
        ;
    EUSCI_B1->TXBUF = (APDS_PDATAL | APDS_CMD); // Point to PDATAL register

    while (!(EUSCI_B1->IFG & EUSCI_B_IFG_TXIFG0))
        ;

    // Step 2: Restart and Read 2 Bytes
    EUSCI_B1->CTLW0 &= ~EUSCI_B_CTLW0_TR;   // Switch to Rx Mode
    EUSCI_B1->CTLW0 |= EUSCI_B_CTLW0_TXSTT; // Send RE-START

    // Read Low Byte
    while (!(EUSCI_B1->IFG & EUSCI_B_IFG_RXIFG0))
        ;
    lowByte = EUSCI_B1->RXBUF;

    // Read High Byte (Send STOP before reading the last byte)
    EUSCI_B1->CTLW0 |= EUSCI_B_CTLW0_TXSTP;
    while (!(EUSCI_B1->IFG & EUSCI_B_IFG_RXIFG0))
        ;
    highByte = EUSCI_B1->RXBUF;

    // Wait for STOP to finish
    while (EUSCI_B1->CTLW0 & EUSCI_B_CTLW0_TXSTP)
        ;

    // Combine bytes
    return ((uint16_t)highByte << 8) | lowByte;
}
