/*
 * i2c.c
 *
 *  Created on: 27 abr. 2022
 *      Author: AJ & AB
 */

#include <msp430fr2355.h>
#include <stdint.h>
#include <stdio.h>

uint8_t *PTxData;
uint8_t TXByteCtr;
uint8_t *PRxData;
uint8_t RXByteCtr;

void init_I2C(void){                        // Inicializamos la comunicacion I2C
    P4SEL0|=BIT7+BIT6;                      // P4.7-SCL P4.6-SDA
                                            // Usamos el puerto eUSCI_B1
    UCB1CTLW0|=UCSWRST;                     // eUSCI_B1 logic held in reset state
    UCB1CTLW0|=UCMST+UCMODE_3+UCSSEL_2;     // eUSCI_B1 Control Word Register 0:
                                            // UCMST -> Master mode select
                                            // UCMODE_3 -> I2C mode
                                            // UCSSEL_2 -> La fuente de reloj del modulo es SMCLK (16 MHz)
    UCB1BRW=160;                            // Seleccionamos la frecuencia de reloj del bus I2C a 100 kHz (16 MHz / 160)
    UCB1CTLW0&=~UCSWRST;                    // eUSCI_B1 released for operation
    UCB1IE|=UCTXIE0|UCRXIE0|UCSTPIE;        // Habilitamos las interrupciones
}

// funcion para enviar datos
void I2C_send(uint8_t addr, uint8_t *buffer, uint8_t n_dades){
    UCB1I2CSA=addr;                         
    PTxData=buffer;
    TXByteCtr=n_dades;
    UCB1CTLW0|=UCTR+UCTXSTT;
    __bis_SR_register(LPM0_bits+GIE);
    __no_operation();
    while(UCB1CTLW0&UCTXSTP);
}

// funcion para recivir datos
void I2C_receive(uint8_t addr, uint8_t *buffer, uint8_t n_dades){
    PRxData=buffer;
    RXByteCtr=n_dades;
    UCB1I2CSA=addr;
    UCB1CTLW0&=~UCTR;
    while(UCB1CTLW0&UCTXSTP);
    UCB1CTLW0|=UCTXSTT;
    __bis_SR_register(LPM0_bits+GIE);
    __no_operation();
}

#pragma vector=USCI_B1_VECTOR
__interrupt void ISR_USCI_PLACA_PROFES(void){
    switch(__even_in_range(UCB1IV,12)){
        case USCI_NONE:
            break;
        case USCI_I2C_UCALIFG:
            break;
        case USCI_I2C_UCNACKIFG:
            break;
        case USCI_I2C_UCSTTIFG:
            break;
        case USCI_I2C_UCSTPIFG:
            __bic_SR_register_on_exit(LPM0_bits);       // salir del modo de bajo de consumo cuando se emite un bi de stop
            break;
        case USCI_I2C_UCRXIFG0:
            if(RXByteCtr){
                *PRxData++=UCB1RXBUF;
                if(RXByteCtr==1){
                    UCB1CTLW0|=UCTXSTP;
                }
            }else{
                *PRxData=UCB1RXBUF;
            }
            RXByteCtr--;
            break;
        case USCI_I2C_UCTXIFG0:
            if(TXByteCtr){
                UCB1TXBUF=*PTxData++;
                TXByteCtr--;
            }else{
                UCB1CTLW0|=UCTXSTP;
                UCB1IFG&=~UCTXIFG;
            }
            break;
        default:
            break;
    }
}
