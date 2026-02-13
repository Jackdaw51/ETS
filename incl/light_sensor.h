#pragma once
#include <msp432p401r.h>
/* --- APDS-9930 I2C Address --- */
#define APDS9930_ADDR   0x39

/* --- Registers (Command bit 0x80 must be set for register access) --- */
#define APDS_CMD        0x80
#define APDS_ENABLE     0x00
#define APDS_PDATAL     0x18
#define APDS_PDATAH     0x19

int APDS_Write(uint8_t reg, uint8_t value);
uint16_t APDS_ReadProximity();
void I2C_Init();
void I2C_Unstick();