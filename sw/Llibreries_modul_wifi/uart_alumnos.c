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

}

//interrupcion de recepcion en la uart UCA0:
#pragma vector = EUSCI_A0_VECTOR
__interrupt void EUSCIA0_IRQHandler(void)
{

}
