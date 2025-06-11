#ifndef LOW_LEVEL_I2C_H_
#define LOW_LEVEL_I2C_H_

#include <stdint.h>

// I2C Slave Addresses
#define ADDR_ROBOT 0x10 // I2C address for the robot's control module
#define ADDR_LCD 0x3E   // I2C address for the LCD display

// Extern declarations for global variables used in I2C communication.
// These are defined in low_level_i2c.c
extern uint8_t *PTxData;   // Pointer to transmit data buffer
extern uint8_t *PRxData;   // Pointer to receive data buffer
extern uint8_t TXByteCtr;  // Transmit byte counter
extern uint8_t RXByteCtr;  // Receive byte counter

/**
 * @brief Initializes the USCI_B0 module for I2C Master communication.
 * Configures the I2C pins (P1.2 SDA, P1.3 SCL), sets the module as master,
 * synchronous, and I2C mode, with a clock frequency of approximately 100kHz.
 */
void init_i2c(void);

/**
 * @brief Sends a series of 'n_dades' bytes via I2C to the specified 'addr'.
 *
 * @param addr The 7-bit I2C slave address.
 * @param buffer Pointer to the data buffer to transmit.
 * @param n_dades The number of bytes to transmit.
 */
void I2C_send(uint8_t addr, uint8_t *buffer, uint8_t n_dades);

/**
 * @brief Receives a series of 'n_dades' bytes via I2C from the specified 'addr'.
 *
 * @param addr The 7-bit I2C slave address.
 * @param buffer Pointer to the buffer where received data will be stored.
 * @param n_dades The number of bytes to receive.
 */
void I2C_receive(uint8_t addr, uint8_t *buffer, uint8_t n_dades);

// ISR for USCI_B0 I2C module.
// This declaration is for the linker, the actual definition is in the .c file.
#pragma vector = USCI_B0_VECTOR
__interrupt void ISR_USCI_I2C(void);

#endif /* LOW_LEVEL_I2C_H_ */

