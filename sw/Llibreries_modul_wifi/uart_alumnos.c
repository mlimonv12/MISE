/*
 * uart.c
 *
 *  Created on:
 *      Author:
 */
#include <msp430.h>
#include <stdint.h>
#include "uart_alumnos.h"



void init_uart_wifi(void) //UART configuration: UCA0, 115200bps
{
    UCA0CTLW0 |= UCSWRST; // Fem un reset de la USCI i que estigui “inoperativa”
    UCA0CTLW0 |= UCSSEL__SMCLK;
    // UCSYNC=0 mode asíncron
    // UCMODEx=0 seleccionem mode UART
    // UCSPB=0 nomes 1 stop bit
    // UC7BIT=0 8 bits de dades
    // UCMSB=0 bit de menys pes primer
    // UCPAR=x no es fa servir bit de paritat
    // UCPEN=0 sense bit de paritat
    // Triem SMCLK (16MHz) com a font del clock BRCLK
    UCA0MCTLW = UCOS16; // oversampling => bit 0 = UCOS16 = 1.
    UCA0BRW = 8; // Prescaler de BRCLK
    UCA0MCTLW |= (10 << 4); // Desem el valor 10 al camp UCBRFx del registre, first modulation stage select
    // ELS CÀLCULS EM DONEN 8, INICIALMENT DEIA 10?
    UCA0MCTLW |= (0xF7 << 8); // Desem el valor 0xF7 al camp UCBRS0 del registre,
                                // Config. corresponent a la part fraccional de N (second modulation stage select)
    P1SEL0 |= BIT7 | BIT6; // I/O funció: P1.7 = UART_TX, P1.6 = UART_RX
    P1SEL1 &= ~(BIT7 | BIT6); // I/O funció: P1.7 = UART_TX, P1.6 = UART_RX
    UCA0CTLW0 &= ~UCSWRST; // Reactivem la línia de comunicacions sèrie
    UCA0IE |= UCRXIE; // Interrupcions activades per la recepció a la UART
    UCA0IFG &= ~UCRXIFGE; // Per si de cas, esborrem qualsevol activació d’interrupció fins ara

}


/*
RxPacket() read data from UART buffer (received from Wifi module).
RxPacket() need a timeout parameter, in order to abort if no response.
RxPacket() return struct containing Received Packet.
 */
RxReturn RxPacket(uint32_t time_out)
{

}

/*
TxPacket() send data to Wifi module (UART).
TxPacket() needs 2 parameters:
     Length of parameters (number of characters to be sent)
     Pointer to parameters to send.
 */
uint8_t TxPacket(uint8_t bParameterLength, const uint8_t *Parameters){
    // Definim trama a enviar
    uint8_t inst_packet[16];
    inst_packet[0] = 0xFF;
    inst_packet[1] = 0xFF;
}

//interrupcion de recepcion en la uart UCA0:
#pragma vector = EUSCI_A0_VECTOR
__interrupt void EUSCIA0_IRQHandler(void)
{

}
