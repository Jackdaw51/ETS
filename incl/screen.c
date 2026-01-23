#include "screen.h"
#include <msp432p401r.h>
#include "ti/devices/msp432p4xx/driverlib/driverlib.h"


// --- Pin Definitions ---
// P1.5 = CLK, P1.6 = MOSI
#define LCD_CS_PORT    GPIO_PORT_P2
#define LCD_CS_PIN     GPIO_PIN5
#define LCD_RST_PORT   GPIO_PORT_P3
#define LCD_RST_PIN    GPIO_PIN0
#define LCD_DC_PORT    GPIO_PORT_P5
#define LCD_DC_PIN     GPIO_PIN7

// --- ST7735 Commands ---
#define ST7735_SWRESET 0x01
#define ST7735_SLPOUT  0x11
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_MADCTL  0x36
#define ST7735_COLMOD  0x3A

#define SPI_WRITE_BYTE(byte) \
    while(!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG)); \
    EUSCI_B0->TXBUF = (byte);

// --- SPI Configuration ---
const eUSCI_SPI_MasterConfig spiMasterConfig =
{
    EUSCI_B_SPI_CLOCKSOURCE_SMCLK,             // SMCLK Clock Source
    12000000,                                  // SMCLK = 12MHz (default) (Adjust if you changed clock)
    6000000,                               // SPICLK = 6MHz (Fast enough for LCD)
    EUSCI_B_SPI_MSB_FIRST,                     // MSB First
    EUSCI_B_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT, // Phase
    EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW,  // Polarity
    EUSCI_B_SPI_3PIN                           // 3-wire mode
};

// --- Helper Functions ---

void SPI_Write(uint8_t data) {
    while (!(SPI_getInterruptStatus(EUSCI_B0_BASE, EUSCI_B_SPI_TRANSMIT_INTERRUPT)));
    SPI_transmitData(EUSCI_B0_BASE, data);
}

void LCD_Command(uint8_t cmd) {
    GPIO_setOutputLowOnPin(LCD_DC_PORT, LCD_DC_PIN); // DC Low = Command
    GPIO_setOutputLowOnPin(LCD_CS_PORT, LCD_CS_PIN); // CS Low = Select
    SPI_Write(cmd);
    GPIO_setOutputHighOnPin(LCD_CS_PORT, LCD_CS_PIN); // CS High = Deselect
}

void write_data_fast(uint8_t highByte, uint8_t lowByte){
    while(!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG));
            // Write byte directly to register
            EUSCI_B0->TXBUF = highByte;

            // SEND LOW BYTE
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG));
            EUSCI_B0->TXBUF = lowByte;
}


void LCD_Data(uint8_t data) {
    GPIO_setOutputHighOnPin(LCD_DC_PORT, LCD_DC_PIN); // DC High = Data
    GPIO_setOutputLowOnPin(LCD_CS_PORT, LCD_CS_PIN);
    SPI_Write(data);
    GPIO_setOutputHighOnPin(LCD_CS_PORT, LCD_CS_PIN);
}

void set_address_window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    // Note: Some screens have an offset (e.g., x0 + 2). Try adding +2 if pixels are cut off.
    LCD_Command(ST7735_CASET); // Column Addr Set
    LCD_Data(0x00); LCD_Data(x0); // X Start
    LCD_Data(0x00); LCD_Data(x1); // X End

    LCD_Command(ST7735_RASET); // Row Addr Set
    LCD_Data(0x00); LCD_Data(y0); // Y Start
    LCD_Data(0x00); LCD_Data(y1); // Y End

    LCD_Command(ST7735_RAMWR); // Write to RAM
}

void LCD_Init() {
    // 1. Hardware Reset
    GPIO_setOutputHighOnPin(LCD_RST_PORT, LCD_RST_PIN);
    __delay_cycles(300000); // Delay 100ms
    GPIO_setOutputLowOnPin(LCD_RST_PORT, LCD_RST_PIN);
    __delay_cycles(300000);
    GPIO_setOutputHighOnPin(LCD_RST_PORT, LCD_RST_PIN);
    __delay_cycles(300000);

    // 2. Software Initialization Sequence
    LCD_Command(ST7735_SWRESET);
    __delay_cycles(360000); // 120ms delay

    LCD_Command(ST7735_SLPOUT); // Sleep out
    __delay_cycles(360000);

    LCD_Command(ST7735_COLMOD); // Interface Pixel Format
    LCD_Data(0x05);             // 16-bit color

    LCD_Command(ST7735_MADCTL); // Memory Access Control (Rotation)
    LCD_Data(0xA0);             // MY=1, MX=1 (Adjust this if screen is mirrored)

    LCD_Command(ST7735_DISPON); // Display On
    __delay_cycles(375000);
}

void LCD_SetAddressWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    // Note: Some screens have an offset (e.g., x0 + 2). Try adding +2 if pixels are cut off.
    LCD_Command(ST7735_CASET); // Column Addr Set
    LCD_Data(0x00); LCD_Data(x0); // X Start
    LCD_Data(0x00); LCD_Data(x1); // X End

    LCD_Command(ST7735_RASET); // Row Addr Set
    LCD_Data(0x00); LCD_Data(y0); // Y Start
    LCD_Data(0x00); LCD_Data(y1); // Y End

    LCD_Command(ST7735_RAMWR); // Write to RAM
}

void pin_init(){
    GPIO_setOutputHighOnPin(LCD_DC_PORT, LCD_DC_PIN);
    GPIO_setOutputLowOnPin(LCD_CS_PORT, LCD_CS_PIN);
}


void pin_des(){
    GPIO_setOutputHighOnPin(LCD_CS_PORT, LCD_CS_PIN);
}

void LCD_FillColor(uint16_t color) {
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;
    
    // Set window to full 128x160
    LCD_SetAddressWindow(0, 0, 159, 127);

    GPIO_setOutputHighOnPin(LCD_DC_PORT, LCD_DC_PIN);
    GPIO_setOutputLowOnPin(LCD_CS_PORT, LCD_CS_PIN);

    // Blast pixels
    int i = 0;
    for(i = 0; i < 128 * 160; i++) {
        SPI_Write(hi);
        SPI_Write(lo);
    }
    
    GPIO_setOutputHighOnPin(LCD_CS_PORT, LCD_CS_PIN);
}

// --- Main ---
void screen_init(void)
{
    // 1. Configure GPIO for SPI (P1.5 CLK, P1.6 MOSI)
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1, GPIO_PIN5 | GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION);

    // 2. Configure Control Pins (CS, DC, RST)
    GPIO_setAsOutputPin(LCD_CS_PORT, LCD_CS_PIN);
    GPIO_setAsOutputPin(LCD_DC_PORT, LCD_DC_PIN);
    GPIO_setAsOutputPin(LCD_RST_PORT, LCD_RST_PIN);

    // Set initial states
    GPIO_setOutputHighOnPin(LCD_CS_PORT, LCD_CS_PIN);
    GPIO_setOutputHighOnPin(LCD_DC_PORT, LCD_DC_PIN);
    GPIO_setOutputHighOnPin(LCD_RST_PORT, LCD_RST_PIN);

    // 3. Initialize SPI Master
    SPI_initMaster(EUSCI_B0_BASE, &spiMasterConfig);
    SPI_enableModule(EUSCI_B0_BASE);

    // 4. Initialize LCD
    LCD_Init();

    // 5. Fill Screen with RED
    // Color format is RGB565. Red = 0xF800
    //LCD_FillColor(0xF800);
}
