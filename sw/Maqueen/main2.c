#include <msp430.h>
#include <stdint.h>

#define ADDR_ROBOT 0x10
#define ADDR_LCD 0x3E

uint8_t *PTxData, *PRxData, TXByteCtr, RXByteCtr;
uint8_t foto_vals;
uint8_t buffer_i2c [12];

uint32_t delay = 0;


/**
 * main.c
 */

 void init_clocks(void)
 { // Configure one FRAM waitstate as required by the device datasheet for MCLK operation beyond 8MHz before configuring the clock system
   FRCTL0 = FRCTLPW | NWAITS_1;
   P2SEL1 |= BIT6 | BIT7; // P2.6~P2.7: crystal pins
   do
   {
     CSCTL7 &= ~(XT1OFFG | DCOFFG); // Clear XT1 and DCO fault flag
     SFRIFG1 &= ~OFIFG;
   } while (SFRIFG1 & OFIFG); // Test oscillator fault flag
 
   __bis_SR_register(SCG0); // disable FLL
   CSCTL3 |= SELREF__XT1CLK; // Set XT1 as FLL reference source
   //CSCTL1 = DCOFTRIMEN_1 | DCOFTRIM0 | DCOFTRIM1 | DCORSEL_5; // DCOFTRIM=5, DCO Range = 16MHz**
   CSCTL1 = DCORSEL_5; // DCOFTRIM=5, DCO Range = 16MHz
   CSCTL2 = FLLD_0 + 487; // DCOCLKDIV = 16MHz
   __delay_cycles(3);
   __bic_SR_register(SCG0); // enable FLL
   //Software_Trim(); // Software Trim to get the best DCOFTRIM value**
   CSCTL4 = SELMS__DCOCLKDIV | SELA__XT1CLK; // set XT1 (~32768Hz) as ACLK source, ACLK = 32768Hz
   // default DCOCLKDIV as MCLK and SMCLK source
   P1DIR |= BIT0 | BIT1; // set SMCLK, ACLK pin as output
   P1SEL1 |= BIT0 | BIT1; // set SMCLK and ACLK pin as second function
   PM5CTL0 &= ~LOCKLPM5; // Disable the GPIO power-on default high-impedance mode to activate previously configured port settings
 }
 
 void init_timers() {
   TB0CCR0 = 33; // ~1ms at 2^15 Hz
   TB0CTL = TBCLR | TBSSEL_1 | MC_1; // CLEAR+ACLK+UPMODE
   TB0CCTL0 = CCIE; // INTERRUPTS
 }
 
 void delay_ms(uint32_t temps) { //temps en ms
   TB0CTL = TBCLR | TBSSEL_1 | MC_1; // CLEAR+ACLK+UPMODE
   TB0CCTL0 = CCIE; // Enable interrupts
   delay = 0;
   while(delay<temps);
   TB0CCTL0 = ~CCIE; // Disable interrupts
 }
 
 // I2C
 void init_i2c()
 {
     //P4SEL0 |= BIT7 + BIT6; * // P4.6 SDA i P4.7 SCL com a USCI si fem server USCI B1
     P1SEL0 |= BIT3 + BIT2; // P1.2 SDA i P1.3 SCL com a USCI si fem server USCI B0
     UCB0CTLW0 |= UCSWRST; // Aturem el mòdul
     //El configurem com a master, síncron i mode i2c, per defecte, està en single-master mode
     UCB0CTLW0 |= UCMST + UCMODE_3 + UCSSEL_2; // Use SMCLK,
     UCB0BR0 = 160; // fSCL = SMCLK(16MHz)/160 = ~100kHz
     UCB0BR1 = 0;
     UCB0CTLW0 &= ~UCSWRST; // Clear SW reset, resume operation
     UCB0IE |= UCTXIE0 | UCRXIE0; // Habilita les interrupcions a TX i RX
 }
 
 //Envia una sèrie de "n_dades" a la adreça "addr" del I2C
 void I2C_send(uint8_t addr, uint8_t *buffer, uint8_t n_dades)
 {
     UCB0I2CSA = addr; //Coloquem l’adreça de slave
     PTxData = buffer; //adreça del bloc de dades a transmetre
     TXByteCtr = n_dades; //carreguem el número de dades a transmetre;
     UCB0CTLW0 |= UCTR + UCTXSTT; //I2C en mode TX, enviem la condició de start
     __bis_SR_register(LPM0_bits + GIE); //Entrem a mode LPM0, enable interrupts
     __no_operation(); //Resta en mode LPM0 fins que es trasmetin les dades
     while (UCB0CTLW0 & UCTXSTP); //Ens assegurem que s'ha enviat la condició de stop
 }
 
 void I2C_receive(uint8_t addr, uint8_t *buffer, uint8_t n_dades)
 {
     PRxData = buffer; //adreça del buffer on ficarem les dades rebudes
     RXByteCtr = n_dades; //carreguem el número de dades a rebre
     UCB0I2CSA = addr; //Coloquem l’adreça de slave
     UCB0CTLW0 &= ~UCTR; //I2C en mode Recepció
     while (UCB0CTLW0 & UCTXSTP); //Ens assegurem que el bus està en stop
     UCB0CTLW0 |= UCTXSTT; //I2C start condition en recepció
     __bis_SR_register(LPM0_bits + GIE); //Entrem en mode LPM0, enable interrupts
     __no_operation(); // Resta en mode LPM0 fins que es rebin totes les dades
 }

// Interfície
void LEDs(uint8_t color_left, uint8_t color_right) {
    buffer_i2c[0] = 0x0B;
    buffer_i2c[1] = color_left;
    buffer_i2c[2] = color_right;
    I2C_send(ADDR_ROBOT, buffer_i2c, 3); // LEDs
}

void motors(uint8_t dir_left, uint8_t dir_right, uint8_t vel_left, uint8_t vel_right) {
    buffer_i2c[0] = 0x00;
    buffer_i2c[1] = dir_left; // 1 forward, 2 backwards
    buffer_i2c[2] = vel_left; // 0 to 255
    buffer_i2c[3] = dir_right; // 1 forward, 2 backwards
    buffer_i2c[4] = vel_right; // 0 to 255
    I2C_send(ADDR_ROBOT, buffer_i2c, 5); // Motor
}

uint8_t fotodetectors(){
    uint8_t *receive = 0;
    buffer_i2c[0] = 0x1D;
    I2C_send(ADDR_ROBOT, buffer_i2c, 1);
    I2C_receive(ADDR_ROBOT, receive, 1);
    return receive;
}

main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
    init_clocks();
    init_i2c();

    LEDs(3, 3);
    motors(1, 2, 50, 50);
    foto_vals = fotodetectors();

    buffer_i2c[0] = 0x00;
    buffer_i2c[0] = 0x0F;
    I2C_send(ADDR_LCD, buffer_i2c, 2); // Provar d'encendre display

    while(1){
        __no_operation();
    };
}

#pragma vector = USCI_B0_VECTOR
__interrupt void ISR_USCI_I2C(void)
{
    switch(__even_in_range(UCB0IV,12))
    {
        case USCI_NONE: break; // Vector 0: No interrupts
        case USCI_I2C_UCALIFG: break; // Vector 2: ALIFG
        case USCI_I2C_UCNACKIFG: break; // Vector 4: NACKIFG
        case USCI_I2C_UCSTTIFG: break; // Vector 6: STTIFG
        case USCI_I2C_UCSTPIFG: break; // Vector 8: STPIFG
        case USCI_I2C_UCRXIFG0: // Vector 10: RXIFG
            if (RXByteCtr)
            {
                *PRxData++ = UCB1RXBUF; // Mou la dada rebuda a l’adreça PRxData
                if (RXByteCtr == 1) // Queda només una?
                UCB0CTLW0 |= UCTXSTP; // Genera I2C stop condition
            }
            else {
                *PRxData = UCB0RXBUF; // Mou la dada rebuda a l’adreça PRxData
                __bic_SR_register_on_exit(LPM0_bits); // Exit del mode baix consum LPM0, activa la CPU
            }
            RXByteCtr--; // Decrement RX byte counter
            break;
        case USCI_I2C_UCTXIFG0: // Vector 12: TXIFG
            if (TXByteCtr) // Check TX byte counter
                {
                UCB0TXBUF = *PTxData++; // Carrega el TX buffer amb la dada a enviar
                TXByteCtr--; // Decrementa TX byte counter
                }
            else
                {
                UCB0CTLW0 |= UCTXSTP; // I2C stop condition
                UCB0IFG &= ~UCTXIFG; // Clear USCI_B1 TX int flag
                __bic_SR_register_on_exit(LPM0_bits); // Exit del mode baix consum LPM0, activa la CPU
                }
        default: break;
    }
}
