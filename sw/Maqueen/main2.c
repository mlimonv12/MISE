#include <msp430.h>
#include <stdint.h>
#include <stdio.h>
//#include <intrinsics.h>
//#include <Energia.h>

#define ADDR_ROBOT 0x10
#define ADDR_LCD 0x3E

#define STRAIGHT 0
#define TURN_R 1
#define TURN_L 2
#define STOP 3
#define LOST 4

uint8_t *PTxData, *PRxData, TXByteCtr, RXByteCtr;
uint8_t RX_end = 0;
uint8_t buffer_i2c [12];
uint8_t msg [20];

uint32_t count = 0;

/**
 * main.c
 */

void init_clocks()
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

void init_timers()
{
    TB0CCR0 = 16000; // 16000 cycles = 0.001s = 1ms
    TB0CTL |= (TBCLR | TBSSEL__SMCLK | MC__UP); // CLEAR+SMCLK+UPMODE
    TB0CCTL0 &= ~CCIE; // INTERRUPTS
}

void init_GPIOs()
{
    P5SEL1 &= ~BIT2; // Assumim default = GPIO
    P5DIR |= BIT2; // Output RST LCD
    P5OUT &= ~BIT2;
}

void delay_ms(uint32_t temps)
{ //temps en ms
    TB0CTL |= (TBCLR | TBSSEL__SMCLK | MC__UP); // CLEAR+SMCLK+UPMODE
    TB0CCTL0 |= CCIE; // Enable interrupts
    count = 0;
    while(count<temps);
    TB0CCTL0 &= ~CCIE; // Disable interrupts
}

void init_LCD()
{
    uint8_t buffer_i2c[8];
    P5OUT &= ~BIT2;
    delay_ms(10);
    P5OUT |= BIT2;
    delay_ms(10);
    buffer_i2c[0] = 0x00;
    buffer_i2c[1] = 0x39;
    buffer_i2c[2] = 0x14;
    buffer_i2c[3] = 0x74;
    buffer_i2c[4] = 0x54;
    buffer_i2c[5] = 0x6F;
    buffer_i2c[6] = 0x0C;
    buffer_i2c[7] = 0x01;
    I2C_send(ADDR_LCD, buffer_i2c, 8); // Provar d'encendre display
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
    RX_end = 0;
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
void LEDs(uint8_t color_left, uint8_t color_right)
{
    uint8_t buffer_in [3];
    buffer_in[0] = 0x0B;
    buffer_in[1] = color_left;
    buffer_in[2] = color_right;
    I2C_send(ADDR_ROBOT, buffer_in, 3); // LEDs
}

void motors(uint8_t left_dir, uint8_t left_speed, uint8_t right_dir, uint8_t right_speed)
{
    uint8_t buffer_in [5];
    buffer_in[0] = 0x00;
    buffer_in[1] = left_dir; // 1 forward, 2 backwards
    buffer_in[2] = left_speed; // 0 to 255
    buffer_in[3] = right_dir; // 1 forward, 2 backwards
    buffer_in[4] = right_speed; // 0 to 255
    I2C_send(ADDR_ROBOT, buffer_in, 5); // Motor
}

void fotodetectors(uint8_t *buffer_out)
{
    uint8_t buffer_in = 0x1D;
    I2C_send(ADDR_ROBOT, &buffer_in, 1);
    delay_ms(1);
    I2C_receive(ADDR_ROBOT, buffer_out, 1);
}

/*
1- RST del display: commutar el GPIO RST_LCD, esperar uns ms i tornar a commutar el GPIO
2- Enviar comandaments I2C de la rutina ASSEMBLY

Per escriure un string, es pot fer servir la funció "sprint(msg, "@Hola com estas?")".
Aquesta funció ens guarda els valors ASCII a dins el buffer "msg", byte a byte.
L'@ es posa al principi ja que es correspon en codi HEX (0x40) amb la comanda que
necessita el display per mostrar text.
*/
void display_LCD(char *msg, uint8_t length)
{
    uint8_t buffer_LCD[64];
    char arroba [];
    arroba[0] = "@";
    strcat(&arroba, msg);
    //buffer_LCD = 0x40;
    sprintf(buffer_LCD, arroba);
    I2C_send(ADDR_LCD, buffer_LCD, length);
}

uint8_t calculate_motors(uint8_t *previous, uint8_t *next)
{
    uint8_t stat;
    fotodetectors(&stat); // Obtain data

    // Initialize next motor parameters based on previous state
    next[0] = previous[0]; // left_dir
    next[1] = previous[1]; // left_speed
    next[2] = previous[2]; // right_dir
    next[3] = previous[3]; // right_speed

    // Check for cul-de-sac using side photodetectors
    if ((stat & 0b00011110) == 0b00011110) { // All four fotodetectors
        // Robot is at a cul-de-sac (T-junction)
        next[0] = 0; // Stop left motor
        next[2] = 0; // Stop right motor

        return STOP;

    } else if ((stat & 0b00001100) == 0b00001100) { // Line is under the robot
        // Check if the robot is on the line
        // Both front center photodetectors are on the line
        next[0] = 1; // Enable left motor
        next[2] = 1; // Enable right motor
        next[1] = 50; // Set left speed
        next[3] = 50; // Set right speed

        return STRAIGHT;

    } else if ((stat & 0b00001100) == 0b00000100) { // Only left front photodetector is on the line
        // Adjust speed to turn right
        next[0] = 1; // Enable left motor
        next[2] = 1; // Enable right motor
        next[1] = 0; // Slow down left motor
        next[3] = 50; // Keep right motor speed
        
        return TURN_R;

    } else if ((stat & 0b00001100) == 0b00001000) { // Only right front photodetector is on the line
        // Adjust speed to turn left
        next[0] = 1; // Enable left motor
        next[2] = 1; // Enable right motor
        next[1] = 50; // Keep left motor speed
        next[3] = 0; // Slow down right motor

        return TURN_L;

    } else {
        // Robot is off the line, stop or turn
        next[1] = 35; // Stop left motor
        next[3] = 35; // Stop right motor
        next[0] = 1; // Stop left motor
        next[2] = 1; // Stop right motor

        return LOST;
    }
}

main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
    init_clocks();
    init_timers();
    init_i2c();
    init_GPIOs();

    LEDs(5, 5);

    init_LCD();

    char msg[] = "@ABCD";
    display_LCD(msg, 5);
    

    uint8_t stat_prev [4] = {1, 50, 1, 50}; // PREVIOUS: left_dir, left_speed, right_dir, right_speed
    uint8_t stat_next [4]; // NEXT: left_dir, left_speed, right_dir, right_speed
    uint8_t leds_state = 0;

    // Init motors
    //motors(stat_prev[0], stat_prev[1], stat_prev[2], stat_prev[3]);

    uint8_t i = 0;
    while(1){
        leds_state = calculate_motors(stat_prev, stat_next);
        delay_ms(1);
        // motors(stat_next[0], stat_next[1], stat_next[2], stat_next[3]);
        delay_ms(1);

        switch (leds_state)
        {
        case STRAIGHT:
            LEDs(2, 2);
            break;

        case TURN_L:
            LEDs(2, 3);
            break;

        case TURN_R:
            LEDs(3, 2);
            break;

        case STOP:
            LEDs(1, 1);
            break;

        case LOST:
            LEDs(4, 4);
            break;
        
        default:
            break;
        }

        for (i = 0; i < 4; i++) {
            stat_prev[i] = stat_next[i];
        }

        delay_ms(100);
        __no_operation();
    };
}

//******************************************************************************
// Timer B0 ********************************************************************
//******************************************************************************
#pragma vector=TIMER0_B0_VECTOR
__interrupt void timerB0_0_isr(void)
{
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
        case USCI_I2C_UCSTPIFG:
            __bic_SR_register_on_exit(LPM0_bits);       // salir del modo de bajo de consumo cuando se emite un bi de stop
            break;
        case USCI_I2C_UCRXIFG0: // Vector 10: RXIFG
            
            if(RXByteCtr){
                *PRxData++=UCB0RXBUF;
                if(RXByteCtr==1){
                    UCB0CTLW0|=UCTXSTP;
                }
            }else{
                *PRxData=UCB0RXBUF;
                __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
            }
            RXByteCtr--;
            break;
               
            /*
            if (RX_end == 0) {
                *PRxData++ = UCB0RXBUF; // Move received data to PRxData
                if (RXByteCtr == 0) {
                    RX_end = 1; // Make sure we don't come back in here
                    UCB0CTLW0 |= UCTXSTP; // Generate I2C STOP condition
                    __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
                } else {
                    RXByteCtr--; // Decrement RX byte counter
                }
            } else {
                __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
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
                RXByteCtr--; // Decrement RX byte counter
            } else {
                //*PRxData++ = UCB0RXBUF; // Mou la dada rebuda a l’adreça PRxData
                UCB0IFG &= ~UCRXIFG; // Clear USCI_B1 TX int flag
                __bic_SR_register_on_exit(LPM0_bits); // Exit del mode baix consum LPM0, activa la CPU
            }
            break;
            */
            /*
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
        */
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
