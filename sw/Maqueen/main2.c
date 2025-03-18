#include <msp430.h>
#include <stdint.h>
//#include <intrinsics.h>
//#include <Energia.h>

#define ADDR_ROBOT 0x10
#define ADDR_LCD 0x3E

uint8_t *PTxData, *PRxData, TXByteCtr, RXByteCtr;
uint8_t buffer_i2c [12];

uint32_t count = 0;

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
    TB0CCTL0 = ~CCIE; // INTERRUPTS
}

void delay_ms(uint32_t temps) { //temps en ms
    TB0CTL = TBCLR | TBSSEL_1 | MC_1; // CLEAR+ACLK+UPMODE
    TB0CCTL0 = CCIE; // Enable interrupts
    count = 0;
    while(count<temps);
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
    uint8_t buffer_in [3];
    buffer_in[0] = 0x0B;
    buffer_in[1] = color_left;
    buffer_in[2] = color_right;
    I2C_send(ADDR_ROBOT, buffer_in, 3); // LEDs
}

void motors(uint8_t left_dir, uint8_t left_speed, uint8_t right_dir, uint8_t right_speed) {
    uint8_t buffer_in [5];
    buffer_in[0] = 0x00;
    buffer_in[1] = left_dir; // 1 forward, 2 backwards
    buffer_in[2] = left_speed; // 0 to 255
    buffer_in[3] = right_dir; // 1 forward, 2 backwards
    buffer_in[4] = right_speed; // 0 to 255
    I2C_send(ADDR_ROBOT, buffer_in, 5); // Motor
}

void fotodetectors(uint8_t *buffer_out) {
    uint8_t buffer_in = 0x1D;
    I2C_send(ADDR_ROBOT, &buffer_in, 1);
    I2C_receive(ADDR_ROBOT, buffer_out, 1);
    //buffer_out &= (BIT7 | BIT6); // Mask to filter 2 MSBs
}

void calculate_motors(uint8_t *previous, uint8_t *next) {
    uint8_t stat;
    fotodetectors(&stat); // Obtain data

    // Initialize next motor parameters based on previous state
    next[0] = previous[0]; // left_dir
    next[1] = previous[1]; // left_speed
    next[2] = previous[2]; // right_dir
    next[3] = previous[3]; // right_speed

    // Check if the robot is on the line
    if ((stat & 0b00001100) == 0b00001100) { // Line is under the robot
        // Both front center photodetectors are on the line
        next[1] = 50; // Set left speed
        next[3] = 50; // Set right speed
    } else if ((stat & 0b00001100) == 0b00000100) { // Only left front photodetector is on the line
        // Adjust speed to turn right
        next[1] = 30; // Slow down left motor
        next[3] = 50; // Keep right motor speed
    } else if ((stat & 0b00001100) == 0b00001000) { // Only right front photodetector is on the line
        // Adjust speed to turn left
        next[1] = 50; // Keep left motor speed
        next[3] = 30; // Slow down right motor
    } else {
        // Robot is off the line, stop or turn
        next[1] = 0; // Stop left motor
        next[3] = 0; // Stop right motor
    }

    // Check for cul-de-sac using side photodetectors
    if ((stat & 0b00100001) == 0b00100001) { // Both side photodetectors are on the surface
        // Robot is at a cul-de-sac (T-junction)
        next[0] = 0; // Stop left motor
        next[2] = 0; // Stop right motor
    }
}

main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
    init_clocks();
    init_i2c();

    LEDs(6, 5);

    uint8_t vals_foto;
    fotodetectors(&vals_foto);

    /*
    buffer_i2c[0] = 0x00;
    buffer_i2c[0] = 0x0F;
    I2C_send(ADDR_LCD, buffer_i2c, 2); // Provar d'encendre display
    */

    uint8_t stat_prev [4] = {1, 50, 1, 50}; // PREVIOUS: left_dir, left_speed, right_dir, right_speed
    uint8_t stat_next [4]; // NEXT: left_dir, left_speed, right_dir, right_speed

    //motors(stat_prev[0], stat_prev[1], stat_prev[2], stat_prev[3]);

    uint8_t i = 0;
    while(1){
        calculate_motors(stat_prev, stat_next);
        //motors(stat_next[0], stat_next[1], stat_next[2], stat_next[3]);

        for (i = 0; i < 4; i++) {
            stat_prev[i] = stat_next[i];
        }

        //delay_ms(10);
        __no_operation();
    };
}

//******************************************************************************
// Timer B0 ********************************************************************
//******************************************************************************
#pragma vector=TIMER0_B0_VECTOR
__interrupt void timerB0_0_isr(void){
    TB0CTL &= ~CCIFG; // CLEAR FLAG
    count++;
}

//******************************************************************************
// I2C Interrupt ***************************************************************
//******************************************************************************
#pragma vector = USCI_B0_VECTOR
__interrupt void ISR_USCI_I2C(void)
{
    switch(__even_in_range(UCB0IV,12))
    {
        case USCI_NONE: break; // Vector 0: No interrupts
        case USCI_I2C_UCALIFG: break; // Vector 2: ALIFG
        case USCI_I2C_UCNACKIFG: // Vector 4: NACKIFG
            // Handle NACK condition
            UCB0CTLW0 |= UCTXSTP; // Generate STOP condition
            __bic_SR_register_on_exit(LPM0_bits); // Exit low power mode
            break;
        case USCI_I2C_UCSTTIFG: break; // Vector 6: STTIFG
        case USCI_I2C_UCSTPIFG: break; // Vector 8: STPIFG
        case USCI_I2C_UCRXIFG0: // Vector 10: RXIFG
            /*
            if (RXByteCtr > 0) {
                *PRxData++ = UCB0RXBUF; // Move received data to PRxData
                RXByteCtr--; // Decrement RX byte counter
                if (RXByteCtr == 0) {
                    UCB0CTLW0 |= UCTXSTP; // Generate I2C STOP condition
                    __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
                }
            }
            break;
            */
            /*
            if (RXByteCtr)
            {
                *PRxData++ = UCB0RXBUF; // Mou la dada rebuda a l’adreça PRxData
                if (RXByteCtr == 1) { // Queda només una?
                    UCB0CTLW0 |= UCTXSTP; // Genera I2C stop condition
                }
            } else {
                *PRxData = UCB0RXBUF; // Mou la dada rebuda a l’adreça PRxData
                __bic_SR_register_on_exit(LPM0_bits); // Exit del mode baix consum LPM0, activa la CPU
            }
            RXByteCtr--; // Decrement RX byte counter
            break;
            */
            
            if (RXByteCtr)
            {
                *PRxData++ = UCB0RXBUF;
                RXByteCtr--;
            }

            if (RXByteCtr == 1)
            {
                UCB0CTLW0 |= UCTXSTP;
            }
            else if (RXByteCtr == 0)
            {
                UCB0IE &= ~UCRXIE;
                __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
            }
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
