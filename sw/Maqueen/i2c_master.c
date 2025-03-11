// i2c_master.c
#include "i2c_master.h"
#include <msp430.h>

void initGPIO_I2C()
{
    // I2C pins
    P1SEL0 |= BIT2 | BIT3;
    P1SEL1 &= ~(BIT2 | BIT3);

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;
}

void initClockTo16MHz()
{
    // Configure one FRAM waitstate as required by the device datasheet for MCLK
    // operation beyond 8MHz _before_ configuring the clock system.
    FRCTL0 = FRCTLPW | NWAITS_1;

    // Clock System Setup
    __bis_SR_register(SCG0);                           // disable FLL
    CSCTL3 |= SELREF__REFOCLK;                         // Set REFO as FLL reference source
    CSCTL0 = 0;                                        // clear DCO and MOD registers
    CSCTL1 &= ~(DCORSEL_7);                            // Clear DCO frequency select bits first
    CSCTL1 |= DCORSEL_5;                               // Set DCO = 16MHz
    CSCTL2 = FLLD_0 + 487;                             // DCOCLKDIV = 16MHz
    __delay_cycles(3);
    __bic_SR_register(SCG0);                           // enable FLL
    while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1));         // FLL locked
}

void initI2C(uint8_t slave_addr)
{
    UCB0CTLW0 = UCSWRST;                      // Enable SW reset
    UCB0CTLW0 |= UCMODE_3 | UCMST | UCSSEL__SMCLK | UCSYNC; // I2C master mode, SMCLK
    UCB0BRW = 160;                            // fSCL = SMCLK/160 = ~100kHz
    UCB0I2CSA = slave_addr;                   // Slave Address
    UCB0CTLW0 &= ~UCSWRST;                    // Clear SW reset, resume operation
    UCB0IE |= UCNACKIE;
}

I2C_Mode I2C_Master_WriteReg(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *reg_data, uint8_t count, I2C_Mode *MasterMode, uint8_t *TransmitRegAddr, uint8_t *TransmitBuffer, uint8_t *RXByteCtr, uint8_t *TXByteCtr, uint8_t *ReceiveIndex, uint8_t *TransmitIndex)
{
    /* Initialize state machine */
    MasterMode = TX_REG_ADDRESS_MODE;
    TransmitRegAddr = reg_addr;

    //Copy register data to TransmitBuffer
    CopyArray(reg_data, TransmitBuffer, count);

    TXByteCtr = count;
    RXByteCtr = 0;
    ReceiveIndex = 0;
    TransmitIndex = 0;

    /* Initialize slave address and interrupts */
    
    UCB0I2CSA = dev_addr;
    UCB0IFG &= ~(UCTXIFG + UCRXIFG);
    UCB0IE &= ~UCRXIE;
    UCB0IE |= UCTXIE;
    UCB0CTLW0 |= UCTR + UCTXSTT;
    __bis_SR_register(LPM0_bits + GIE);
    //return MasterMode;
}

I2C_Mode I2C_Master_ReadReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t count, I2C_Mode *MasterMode, uint8_t *TransmitRegAddr, uint8_t *RXByteCtr, uint8_t *TXByteCtr, uint8_t *ReceiveIndex, uint8_t *TransmitIndex)
{
    /* Initialize state machine */
    MasterMode = TX_REG_ADDRESS_MODE;
    TransmitRegAddr = reg_addr;
    RXByteCtr = count;
    TXByteCtr = 0;
    ReceiveIndex = 0;
    TransmitIndex = 0;
    
    /* Initialize slave address and interrupts */
    UCB0I2CSA = dev_addr;
    UCB0IFG &= ~(UCTXIFG + UCRXIFG);
    UCB0IE &= ~UCRXIE;
    UCB0IE |= UCTXIE;
    UCB0CTLW0 |= UCTR + UCTXSTT;
    __bis_SR_register(LPM0_bits + GIE);
    //return MasterMode;
}

void CopyArray(const uint8_t *source, uint8_t *dest, uint8_t count) {
    uint8_t i;
    for (i = 0; i < count; i++) {
        dest[i] = source[i];
    }
}