// i2c_master.h
#ifndef I2C_MASTER_H
#define I2C_MASTER_H

#include <stdint.h>

// I2C Mode Enum
typedef enum I2C_ModeEnum{
    IDLE_MODE,
    NACK_MODE,
    TX_REG_ADDRESS_MODE,
    RX_REG_ADDRESS_MODE,
    TX_DATA_MODE,
    RX_DATA_MODE,
    SWITCH_TO_RX_MODE,
    SWITCH_TO_TX_MODE,
    TIMEOUT_MODE
} I2C_Mode;

// Function Prototypes
void initGPIO(void);
void initClockTo16MHz(void);
void initI2C(uint8_t slave_addr);
I2C_Mode I2C_Master_WriteReg(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *reg_data, uint8_t count, I2C_Mode *MasterMode, uint8_t *TransmitRegAddr, uint8_t *TransmitBuffer, uint8_t *RXByteCtr, uint8_t *TXByteCtr, uint8_t *ReceiveIndex, uint8_t *TransmitIndex);
I2C_Mode I2C_Master_ReadReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t count, I2C_Mode *MasterMode, uint8_t *TransmitRegAddr, uint8_t *RXByteCtr, uint8_t *TXByteCtr, uint8_t *ReceiveIndex, uint8_t *TransmitIndex);
void CopyArray(const uint8_t *source, uint8_t *dest, uint8_t count);

#endif // I2C_MASTER_H
