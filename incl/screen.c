#include "screen.h"
#include "timers.h"
#include <msp432p401r.h>
#include "ti/devices/msp432p4xx/driverlib/driverlib.h"
#include "../games/game_files/sprites/palettes.h"
// DMA part
#define CHUNK_SIZE 1024
// #if defined(__TI_COMPILER_VERSION__)
// #pragma DATA_ALIGN(dma_buffer1, 4)
// #elif defined(__IAR_SYSTEMS_ICC__)
// #pragma data_alignment = 4
// #elif defined(__GNUC__)
// __attribute__((aligned(4)))
// #endif
uint8_t ping[CHUNK_SIZE];

// #if defined(__TI_COMPILER_VERSION__)
// #pragma DATA_ALIGN(dma_buffer2, 4)
// #elif defined(__IAR_SYSTEMS_ICC__)
// #pragma data_alignment = 4
// #elif defined(__GNUC__)
// __attribute__((aligned(4)))
// #endif
uint8_t pong[CHUNK_SIZE];

volatile bool dmaTransferDone;

#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_ALIGN(controlTable, 1024)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment = 1024
#elif defined(__GNUC__)
__attribute__((aligned(1024)))
#endif
uint8_t controlTable[1024];

#define TOTAL_BYTES (128 * 160 * 2) // Total bytes for full frame (128x160 pixels, 2 bytes per pixel)
// --- Pin Definitions ---
// P1.5 = CLK, P1.6 = MOSI
#define LCD_CS_PORT GPIO_PORT_P2
#define LCD_CS_PIN GPIO_PIN5
#define LCD_RST_PORT GPIO_PORT_P3
#define LCD_RST_PIN GPIO_PIN0
#define LCD_DC_PORT GPIO_PORT_P5
#define LCD_DC_PIN GPIO_PIN7

// --- ST7735 Commands ---
#define ST7735_SWRESET 0x01
#define ST7735_SLPOUT 0x11
#define ST7735_DISPON 0x29
#define ST7735_CASET 0x2A
#define ST7735_RASET 0x2B
#define ST7735_RAMWR 0x2C
#define ST7735_MADCTL 0x36
#define ST7735_COLMOD 0x3A

#define SPI_WRITE_BYTE(byte)                     \
    while (!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG)) \
        ;                                        \
    EUSCI_B0->TXBUF = (byte);

void DMA_init()
{

    // 1. Enable DMA Module
    DMA_enableModule();
    DMA_setControlBase(controlTable);

    // 2. Assign DMA Channel 0 to EUSCI_B0 Transmit
    // This connects the "SPI Ready" signal to the DMA
    DMA_assignChannel(DMA_CH0_EUSCIB0TX0);

    // 3. Disable specific attributes to ensure clean slate
    // (High priority, alt selection, etc. are turned off)
    DMA_disableChannelAttribute(DMA_CH0_EUSCIB0TX0,
                                UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
                                    UDMA_ATTR_HIGH_PRIORITY | UDMA_ATTR_REQMASK);

    // 4. Configure Control Parameters
    // UDMA_SIZE_8:       Moving bytes (8-bit)
    // UDMA_SRC_INC_8:    After every move, move to NEXT byte in array
    // UDMA_DST_INC_NONE: After every move, stay at SAME SPI register
    // UDMA_ARB_1:        Arbitrate after every 1 byte (Safe for SPI)
    DMA_setChannelControl(UDMA_PRI_SELECT | DMA_CH0_EUSCIB0TX0,
                          UDMA_SIZE_8 | UDMA_SRC_INC_8 | UDMA_DST_INC_NONE | UDMA_ARB_1);

    // 6. Enable DMA Interrupt (Optional: if you want to know when it's done)
    DMA_assignInterrupt(DMA_INT0, 0); // Assign Channel 0 to Interrupt 0
    Interrupt_enableInterrupt(INT_DMA_INT0);
}

void startDMATransfer(uint8_t *buf)
{
    dmaTransferDone = false; // Reset flag

    // Re-config transfer (Source points to the specific buffer)
    DMA_setChannelTransfer(UDMA_PRI_SELECT | DMA_CH0_EUSCIB0TX0,
                           UDMA_MODE_BASIC,
                           (void *)buf,
                           (void *)SPI_getTransmitBufferAddressForDMA(EUSCI_B0_BASE),
                           CHUNK_SIZE);

    DMA_enableChannel(0);
}

void renderSlice(uint8_t *pang, uint8_t *frame_buffer, uint8_t *palette_buffer)
{
    // Copy from frame_buffer to the provided buffer
    // 8 pixels
    // 8 palettes

    // 16 bytes
    uint32_t i;
    for (i = 0; i < CHUNK_SIZE / 16; i++)
    {
        uint8_t batch1 = frame_buffer[i * 2];
        uint8_t batch2 = frame_buffer[i * 2 + 1];

        uint8_t palette_batch1 = palette_buffer[i * 4];
        uint8_t palette_batch2 = palette_buffer[i * 4 + 1];
        uint8_t palette_batch3 = palette_buffer[i * 4 + 2];
        uint8_t palette_batch4 = palette_buffer[i * 4 + 3];

        uint8_t p1_b1 = (batch1 >> 6) & 3;
        uint8_t p2_b1 = (batch1 >> 4) & 3;
        uint8_t p3_b1 = (batch1 >> 2) & 3;
        uint8_t p4_b1 = batch1 & 3;

        uint8_t p1_b2 = (batch2 >> 6) & 3;
        uint8_t p2_b2 = (batch2 >> 4) & 3;
        uint8_t p3_b2 = (batch2 >> 2) & 3;
        uint8_t p4_b2 = batch2 & 3;

        uint8_t pa1_b1 = (palette_batch1 >> 4) & 0x0F;
        uint8_t pa2_b1 = palette_batch1 & 0x0F;
        uint8_t pa1_b2 = (palette_batch2 >> 4) & 0x0F;
        uint8_t pa2_b2 = palette_batch2 & 0x0F;
        uint8_t pa1_b3 = (palette_batch3 >> 4) & 0x0F;
        uint8_t pa2_b3 = palette_batch3 & 0x0F;
        uint8_t pa1_b4 = (palette_batch4 >> 4) & 0x0F;
        uint8_t pa2_b4 = palette_batch4 & 0x0F;

        uint16_t el1 = (PaletteArray565[pa1_b1].colors[p1_b1]);
        uint16_t el2 = (PaletteArray565[pa2_b1].colors[p2_b1]);
        uint16_t el3 = (PaletteArray565[pa1_b2].colors[p3_b1]);
        uint16_t el4 = (PaletteArray565[pa2_b2].colors[p4_b1]);
        uint16_t el5 = (PaletteArray565[pa1_b3].colors[p1_b2]);
        uint16_t el6 = (PaletteArray565[pa2_b3].colors[p2_b2]);
        uint16_t el7 = (PaletteArray565[pa1_b4].colors[p3_b2]);
        uint16_t el8 = (PaletteArray565[pa2_b4].colors[p4_b2]);

        uint8_t ell_1 = el1 & 0xFF;
        uint8_t elh_1 = (el1 >> 8);
        uint8_t ell_2 = el2 & 0xFF;
        uint8_t elh_2 = (el2 >> 8);
        uint8_t ell_3 = el3 & 0xFF;
        uint8_t elh_3 = (el3 >> 8);
        uint8_t ell_4 = el4 & 0xFF;
        uint8_t elh_4 = (el4 >> 8);
        uint8_t ell_5 = el5 & 0xFF;
        uint8_t elh_5 = (el5 >> 8);
        uint8_t ell_6 = el6 & 0xFF; 
        uint8_t elh_6 = (el6 >> 8);    
        uint8_t ell_7 = el7 & 0xFF;
        uint8_t elh_7 = (el7 >> 8);
        uint8_t ell_8 = el8 & 0xFF;
        uint8_t elh_8 = (el8 >> 8);

        uint8_t *pang_fptr = &pang[i*16];
        pang_fptr[0] = elh_1;     // High byte
        pang_fptr[1] = ell_1;     // High byte
        pang_fptr[2] = elh_2;     // High byte
        pang_fptr[3] = ell_2;     // High byte
        pang_fptr[4] = elh_3;     // High byte
        pang_fptr[5] = ell_3;     // High byte
        pang_fptr[6] = elh_4;     // High byte
        pang_fptr[7] = ell_4;     // High byte
        pang_fptr[8] = elh_5;     // High byte
        pang_fptr[9] = ell_5;     // High byte
        pang_fptr[10] = elh_6;    // High byte
        pang_fptr[11] = ell_6;    // High byte
        pang_fptr[12] = elh_7;    // High byte
        pang_fptr[13] = ell_7;    // High byte
        pang_fptr[14] = elh_8;    // High byte
        pang_fptr[15] = ell_8;    // High byte
    }
}

void DMA_send_frame(uint8_t *frame_buffer, uint8_t *palette_buffer)
{
    
    uint32_t currentOffset = 0;
    bool usePing = true; // Toggle to track which buffer to use
    // We must fill the first buffer before we start the loop

    LCD_SetAddressWindow(0, 0, 159, 127);
    LCD_Command(ST7735_RAMWR);
    GPIO_setOutputHighOnPin(LCD_DC_PORT, LCD_DC_PIN); // DC = Data
    GPIO_setOutputLowOnPin(LCD_CS_PORT, LCD_CS_PIN);  // CS = Select (Open the door)

    renderSlice(ping, frame_buffer, palette_buffer); // Fill Ping with Red

    // 2. START THE FIRST TRANSFER
    startDMATransfer(ping);

    // Move offset to next chunk
    currentOffset += CHUNK_SIZE;
    frame_buffer += CHUNK_SIZE / 8;
    palette_buffer += CHUNK_SIZE / 4;
    usePing = false; // Next buffer to FILL is Pong

    // 3. ENTER THE INFINITE LOOP
    while (1)
    {
        // --- CPU DOES WORK ---
        // While the DMA is sending the PREVIOUS buffer, we fill the CURRENT one.
        if (usePing)
        {
            renderSlice(ping, frame_buffer, palette_buffer);
        }
        else
        {
            renderSlice(pong, frame_buffer, palette_buffer);
        }

        // ---WAIT FOR DMA ---
        while (dmaTransferDone == false)
        {
            P2->OUT |= BIT1; // Toggle Green LED state
            // PCM_gotoLPM0(); // Sleep CPU to save power while waiting
            // or do some work
        }
        P2->OUT &= ~BIT1; // Turn off Green LED
        // --- SEND THE NEW BUFFER ---
        if (usePing)
        {
            startDMATransfer(ping);
        }
        else
        {
            startDMATransfer(pong);
        }

        // --- STEP D: PREPARE FOR NEXT LOOP ---
        usePing = !usePing; // Toggle buffer
        currentOffset += CHUNK_SIZE;
        frame_buffer += CHUNK_SIZE / 8;
        palette_buffer += CHUNK_SIZE / 4;

        // Check for End of Frame
        if (currentOffset > TOTAL_BYTES)
        {
            GPIO_setOutputHighOnPin(LCD_CS_PORT, LCD_CS_PIN); // CS = Deselect (Close the door)
            return;
        }
    }
}

void DMA_INT0_IRQHandler(void)
{
    // Clear the interrupt flag for DMA Channel 0
    DMA_clearInterruptFlag(0);

    // optional for safety
    // DMA_disableChannel(0)

    // Set the transfer done flag
    dmaTransferDone = true;
}

#define SPI_WRITE_BYTE(byte) \
    while(!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG)); \
    EUSCI_B0->TXBUF = (byte);

// --- SPI Configuration ---
const eUSCI_SPI_MasterConfig spiMasterConfig =
    {
        EUSCI_B_SPI_CLOCKSOURCE_SMCLK,                           // SMCLK Clock Source
        2,                                                       // SMCLK = 12MHz (default) (Adjust if you changed clock)
        1,                                                       // SPICLK = 6MHz (Fast enough for LCD)
        EUSCI_B_SPI_MSB_FIRST,                                   // MSB First
        EUSCI_B_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT, // Phase
        EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW,                // Polarity
        EUSCI_B_SPI_3PIN                                         // 3-wire mode
};

// --- Helper Functions ---

void SPI_Write(uint8_t data)
{
    while (!(SPI_getInterruptStatus(EUSCI_B0_BASE, EUSCI_B_SPI_TRANSMIT_INTERRUPT)))
        ;
    SPI_transmitData(EUSCI_B0_BASE, data);
}

void LCD_Command(uint8_t cmd)
{
    GPIO_setOutputLowOnPin(LCD_DC_PORT, LCD_DC_PIN); // DC Low = Command
    GPIO_setOutputLowOnPin(LCD_CS_PORT, LCD_CS_PIN); // CS Low = Select
    SPI_Write(cmd);
    GPIO_setOutputHighOnPin(LCD_CS_PORT, LCD_CS_PIN); // CS High = Deselect
}

<<<<<<< HEAD
void write_data_fast(uint8_t highByte, uint8_t lowByte)
{
    while (!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG))
        ;
    // Write byte directly to register
    EUSCI_B0->TXBUF = highByte;

    // SEND LOW BYTE
    while (!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG))
        ;
    EUSCI_B0->TXBUF = lowByte;
}

void LCD_Data(uint8_t data)
{
=======
void write_data_fast(uint8_t highByte, uint8_t lowByte){
    while(!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG));
            // Write byte directly to register
            EUSCI_B0->TXBUF = highByte;

            // SEND LOW BYTE
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG));
            EUSCI_B0->TXBUF = lowByte;
}


void LCD_Data(uint8_t data) {
>>>>>>> 78a2633c05fb290f4c5c199bd116c1ae4f09e799
    GPIO_setOutputHighOnPin(LCD_DC_PORT, LCD_DC_PIN); // DC High = Data
    GPIO_setOutputLowOnPin(LCD_CS_PORT, LCD_CS_PIN);
    SPI_Write(data);
    GPIO_setOutputHighOnPin(LCD_CS_PORT, LCD_CS_PIN);
}

<<<<<<< HEAD
void set_address_window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    // Note: Some screens have an offset (e.g., x0 + 2). Try adding +2 if pixels are cut off.
    LCD_Command(ST7735_CASET); // Column Addr Set
    LCD_Data(0x00);
    LCD_Data(x0); // X Start
    LCD_Data(0x00);
    LCD_Data(x1); // X End

    LCD_Command(ST7735_RASET); // Row Addr Set
    LCD_Data(0x00);
    LCD_Data(y0); // Y Start
    LCD_Data(0x00);
    LCD_Data(y1); // Y End
=======
void set_address_window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    // Note: Some screens have an offset (e.g., x0 + 2). Try adding +2 if pixels are cut off.
    LCD_Command(ST7735_CASET); // Column Addr Set
    LCD_Data(0x00); LCD_Data(x0); // X Start
    LCD_Data(0x00); LCD_Data(x1); // X End

    LCD_Command(ST7735_RASET); // Row Addr Set
    LCD_Data(0x00); LCD_Data(y0); // Y Start
    LCD_Data(0x00); LCD_Data(y1); // Y End
>>>>>>> 78a2633c05fb290f4c5c199bd116c1ae4f09e799

    LCD_Command(ST7735_RAMWR); // Write to RAM
}

<<<<<<< HEAD
void LCD_Init()
{
=======
void LCD_Init() {
>>>>>>> 78a2633c05fb290f4c5c199bd116c1ae4f09e799
    // 1. Hardware Reset
    GPIO_setOutputHighOnPin(LCD_RST_PORT, LCD_RST_PIN);
    sleep_ms(100);
    GPIO_setOutputLowOnPin(LCD_RST_PORT, LCD_RST_PIN);
    sleep_ms(100);
    GPIO_setOutputHighOnPin(LCD_RST_PORT, LCD_RST_PIN);
    sleep_ms(100);

    // 2. Software Initialization Sequence
    LCD_Command(ST7735_SWRESET);
    sleep_ms(120);

    LCD_Command(ST7735_SLPOUT); // Sleep out
    sleep_ms(120);

    LCD_Command(ST7735_COLMOD); // Interface Pixel Format
    LCD_Data(0x05);             // 16-bit color

    LCD_Command(ST7735_MADCTL); // Memory Access Control (Rotation)
    LCD_Data(0xA0);             // MY=1, MX=1 (Adjust this if screen is mirrored)

    LCD_Command(ST7735_DISPON); // Display On
    sleep_ms(125);
}

void LCD_SetAddressWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    // Note: Some screens have an offset (e.g., x0 + 2). Try adding +2 if pixels are cut off.
    LCD_Command(ST7735_CASET); // Column Addr Set
    LCD_Data(0x00);
    LCD_Data(x0); // X Start
    LCD_Data(0x00);
    LCD_Data(x1); // X End

    LCD_Command(ST7735_RASET); // Row Addr Set
    LCD_Data(0x00);
    LCD_Data(y0); // Y Start
    LCD_Data(0x00);
    LCD_Data(y1); // Y End

    LCD_Command(ST7735_RAMWR); // Write to RAM
}

<<<<<<< HEAD
void pin_init()
{
=======
void pin_init(){
>>>>>>> 78a2633c05fb290f4c5c199bd116c1ae4f09e799
    GPIO_setOutputHighOnPin(LCD_DC_PORT, LCD_DC_PIN);
    GPIO_setOutputLowOnPin(LCD_CS_PORT, LCD_CS_PIN);
}

<<<<<<< HEAD
void pin_des()
{
    GPIO_setOutputHighOnPin(LCD_CS_PORT, LCD_CS_PIN);
}

void LCD_FillColor(uint16_t color)
{
=======

void pin_des(){
    GPIO_setOutputHighOnPin(LCD_CS_PORT, LCD_CS_PIN);
}

void LCD_FillColor(uint16_t color) {
>>>>>>> 78a2633c05fb290f4c5c199bd116c1ae4f09e799
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;

    // Set window to full 128x160
    LCD_SetAddressWindow(0, 0, 159, 127);

    GPIO_setOutputHighOnPin(LCD_DC_PORT, LCD_DC_PIN);
    GPIO_setOutputLowOnPin(LCD_CS_PORT, LCD_CS_PIN);

    // Blast pixels
    int i = 0;
    for (i = 0; i < 128 * 160; i++)
    {
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
<<<<<<< HEAD
    // LCD_FillColor(0xF800);

    DMA_init();
=======
    //LCD_FillColor(0xF800);
>>>>>>> 78a2633c05fb290f4c5c199bd116c1ae4f09e799
}
