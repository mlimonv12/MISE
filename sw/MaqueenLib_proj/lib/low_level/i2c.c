#include <msp430.h>
#include "i2c.h"
#include "timers.h" // For delay_ms (though not strictly needed inside ISR)

// Global variables for I2C communication, defined here.
uint8_t *PTxData;   // Pointer to transmit data buffer
uint8_t *PRxData;   // Pointer to receive data buffer
uint8_t TXByteCtr;  // Transmit byte counter
uint8_t RXByteCtr;  // Receive byte counter

// Note: RX_end was in original, but its usage was commented/redundant.
// Removed for clarity, as RXByteCtr should be sufficient for managing reception.

/**
 * @brief Initializes the USCI_B0 module for I2C Master communication.
 * Configures the I2C pins (P1.2 SDA, P1.3 SCL), sets the module as master,
 * synchronous, and I2C mode, with a clock frequency of approximately 100kHz.
 * Assumes SMCLK is 16MHz.
 */
void init_i2c()
{
    // Configure P1.2 (SDA) and P1.3 (SCL) as USCI_B0 I2C function pins
    P1SEL0 |= BIT3 + BIT2; // P1.2 SDA and P1.3 SCL
    P1SEL1 &= ~(BIT3 + BIT2); // Ensure P1SEL1 is clear for module function

    UCB0CTLW0 |= UCSWRST; // Put USCI_B0 in software reset state (halt module)

    // Configure as Master, I2C mode (UCMODE_3), Synchronous, Use SMCLK as source
    // UCMST: Master mode
    // UCMODE_3: I2C mode
    // UCSSEL_2: SMCLK as clock source
    UCB0CTLW0 |= UCMST + UCMODE_3 + UCSSEL_2;

    // Set bit rate for SCL (fSCL = SMCLK / (UCB0BR0 + UCB0BR1 * 256))
    // SMCLK = 16MHz, desired fSCL = 100kHz
    // UCB0BR0 = 160 means 16MHz / 160 = 100kHz
    UCB0BR0 = 160;
    UCB0BR1 = 0;

    UCB0CTLW0 &= ~UCSWRST; // Release USCI_B0 from software reset, resume operation

    // Enable I2C transmit (UCTXIE0) and receive (UCRXIE0) interrupts
    UCB0IE |= UCTXIE0 | UCRXIE0;
}

/**
 * @brief Sends a series of 'n_dades' bytes via I2C to the specified 'addr'.
 *
 * This function initiates an I2C transmit transaction. It sets the slave address,
 * loads the data buffer pointer and byte count, then issues a START condition
 * with transmit mode. The CPU enters LPM0 and waits for the transaction to complete
 * via interrupts.
 *
 * @param addr The 7-bit I2C slave address.
 * @param buffer Pointer to the data buffer to transmit.
 * @param n_dades The number of bytes to transmit.
 */
void I2C_send(uint8_t addr, uint8_t *buffer, uint8_t n_dades)
{
    UCB0I2CSA = addr; // Set the slave address
    PTxData = buffer; // Point to the data block to transmit
    TXByteCtr = n_dades; // Load the number of bytes to transmit

    // Set I2C to Transmit mode (UCTR) and generate a START condition (UCTXSTT)
    UCB0CTLW0 |= UCTR + UCTXSTT;

    // Enter Low Power Mode 0 (LPM0) and enable general interrupts (GIE)
    // The CPU will wake up on I2C transmit completion (handled by ISR_USCI_I2C)
    __bis_SR_register(LPM0_bits + GIE);

    __no_operation(); // Placeholder for breakpoint during debugging

    // Wait until the STOP condition has been sent (ensures bus is free)
    while (UCB0CTLW0 & UCTXSTP);
}

/**
 * @brief Receives a series of 'n_dades' bytes via I2C from the specified 'addr'.
 *
 * This function initiates an I2C receive transaction. It sets the slave address,
 * loads the receive buffer pointer and byte count, then issues a START condition
 * with receive mode. The CPU enters LPM0 and waits for the transaction to complete
 * via interrupts.
 *
 * @param addr The 7-bit I2C slave address.
 * @param buffer Pointer to the buffer where received data will be stored.
 * @param n_dades The number of bytes to receive.
 */
void I2C_receive(uint8_t addr, uint8_t *buffer, uint8_t n_dades)
{
    PRxData = buffer; // Point to the buffer for received data
    RXByteCtr = n_dades; // Load the number of bytes to receive
    UCB0I2CSA = addr; // Set the slave address

    UCB0CTLW0 &= ~UCTR; // Set I2C to Receive mode

    // Ensure the bus is in a STOP state before initiating a new START condition
    while (UCB0CTLW0 & UCTXSTP);

    // Generate I2C START condition in receive mode
    UCB0CTLW0 |= UCTXSTT;

    // If only one byte is to be received, set STOP immediately after START
    // This is important for single-byte reads to ensure NACK is sent
    // before the byte is fully received.
    if (n_dades == 1) {
        while (UCB0CTLW0 & UCTXSTT); // Wait for START condition to be sent
        UCB0CTLW0 |= UCTXSTP;        // Generate STOP condition
    }

    // Enter Low Power Mode 0 (LPM0) and enable general interrupts (GIE)
    // The CPU will wake up on I2C receive completion (handled by ISR_USCI_I2C)
    __bis_SR_register(LPM0_bits + GIE);
    __no_operation(); // Placeholder for breakpoint during debugging
}

//******************************************************************************
// I2C Interrupt Service Routine (ISR) for USCI_B0 ****************************
//******************************************************************************
#pragma vector = USCI_B0_VECTOR
__interrupt void ISR_USCI_I2C(void)
{
    // Using __even_in_range to efficiently handle interrupt vectors
    switch(__even_in_range(UCB0IV, USCI_I2C_UCTXIFG0)) // Max vector value is 12 (UCTXIFG0)
    {
        case USCI_NONE: break; // Vector 0: No interrupts
        case USCI_I2C_UCALIFG: break; // Vector 2: Arbitration Lost (ignored for this application)
        case USCI_I2C_UCNACKIFG: // Vector 4: NACK received
            // Handle NACK condition: generate STOP condition and exit low power mode.
            // This happens if the slave doesn't acknowledge its address or data.
            UCB0CTLW0 |= UCTXSTP; // Generate STOP condition
            __bic_SR_register_on_exit(LPM0_bits); // Exit low power mode (wake CPU)
            break;
        case USCI_I2C_UCSTTIFG: break; // Vector 6: START condition generated (ignored for this application)
        case USCI_I2C_UCSTPIFG: // Vector 8: STOP condition generated
            // Exit low power mode when a STOP bit is emitted
            __bic_SR_register_on_exit(LPM0_bits);
            break;
        case USCI_I2C_UCRXIFG0: // Vector 10: Receive Interrupt Flag (RXIFG0)
            // This case handles incoming data during I2C receive operations.
            if (RXByteCtr > 0)
            {
                // Store the received byte into the buffer pointed to by PRxData
                *PRxData++ = UCB0RXBUF;
                RXByteCtr--; // Decrement the byte counter

                // If this was the last byte to receive, generate a STOP condition
                // This NACKs the last byte (if RXByteCtr was 0 after decrement) and terminates the transaction.
                if (RXByteCtr == 0)
                {
                    UCB0CTLW0 |= UCTXSTP; // Generate STOP condition
                    __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0 as transaction is complete
                }
            } else {
                // This else block might be for an unexpected RXIFG when RXByteCtr is 0,
                // or for the single byte case where the STOP is set after START.
                // The original code had some commented out logic here.
                // For robustness, it's generally better to only receive if RXByteCtr > 0.
                // If it's 0, it means all expected bytes have been received.
                 __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
            }
            break;

        case USCI_I2C_UCTXIFG0: // Vector 12: Transmit Interrupt Flag (TXIFG0)
            // This case handles outgoing data during I2C transmit operations.
            if (TXByteCtr > 0) // Check if there are still bytes to transmit
            {
                // Load the next byte from the transmit buffer into UCB0TXBUF
                UCB0TXBUF = *PTxData++;
                TXByteCtr--; // Decrement the transmit byte counter
            }
            else
            {
                // All bytes have been transmitted.
                UCB0CTLW0 |= UCTXSTP; // Generate I2C STOP condition
                UCB0IFG &= ~UCTXIFG; // Clear the TX interrupt flag (to prevent immediate re-entry)
                __bic_SR_register_on_exit(LPM0_bits); // Exit low power mode (wake CPU)
            }
            break;

        default: break; // Catch-all for other unhandled vectors
    }
}

