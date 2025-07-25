/*
 * uart.c
 *
 *  Created on:
 *      Author: AJ & AB
 */
#include <msp430fr2355.h>
#include <stdint.h>
#include "uart_alumnos.h"
#include "timers.h"

byte Byte_Recibido;
uint8_t TXData;
uint8_t RXData;

void init_uart_wifi(void){       //UART configuration: UCA0, 115200bps
   UCA0CTLW0|=UCSWRST;           // Fem un reset de la USCI i que estigui inoperativa
   UCA0CTLW0|=UCSSEL__SMCLK;
                                 // UCSYNC=0 mode asincron
                                 // UCMODEx=0 seleccionem mode UART
                                 // UCSPB=0 nomes 1 stop bit
                                 // UC7BIT=0 8 bits de dades
                                 // UCMSB=0 bit de menys pes primer
                                 // UCPAR=x no es fa servir bit de paritat
                                 // UCPEN=0 sense bit de paritat
                                 // Triem SMCLK (16MHz) com a font del clock BRCLK
   UCA0MCTLW=UCOS16;             // oversampling => bit 0 = UCOS16 = 1.
   UCA0BRW=8;                    // Prescaler de BRCLK
   UCA0MCTLW|=(10<<4);           // BRCLK   BR   UCOS  UCBRx UCBRFx UCBRSx
   UCA0MCTLW|=(0xF7<<8);         // 16M   115200   1     8     10    0xF7
   P1SEL0|=BIT7|BIT6;            // I/O funcio: P1.7 = UART_TX, P1.6 = UART_RX
   P1SEL1&=~(BIT7|BIT6);         // I/O funcio³: P1.7 = UART_TX, P1.6 = UART_RX
   UCA0CTLW0&=~UCSWRST;          // Reactivem la linia de comunicacions serie
   UCA0IE|=UCRXIE;               // Interrupcions activades per la recepciÃ³ a la UART
   UCA0IFG&=~UCRXIFG;            // Per si de cas, esborrem qualsevol activaciÃ³ dâ€™interrupciÃ³ fins ara
}

/*
RxPacket() read data from UART buffer (received from Wifi module).
RxPacket() need a timeout parameter, in order to abort if no response.
RxPacket() return struct containing Received Packet.
*/

RxReturn RxPacket(uint32_t time_out){
   struct RxReturn respuesta;
   //Inicializacion de la estructura
   respuesta.time_out=0;
   respuesta.num_bytes=0;
   uint16_t bCount;
   uint8_t Rx_time_out=0;
   // Inicializacion del contador timeout
   init_timeout();
   for(bCount=0;bCount<RX_BUFFER_SIZE;bCount++){
      reset_timeout();
      Byte_Recibido=No;
      while(!Byte_Recibido){
         Rx_time_out=TimeOut(time_out); //tiempo en centenas de microsegundos
         if(Rx_time_out){
            respuesta.time_out=1;
            break;                      // Salimos del while pq ha habido timeout
         }
      }
      if(Rx_time_out){
         respuesta.time_out=1;
         break; // Salimos del for pq Timeout
      }
      respuesta.StatusPacket[bCount]=RXData;
      respuesta.num_bytes=bCount;
   }
   desactiva_timeout();
   return respuesta;
}

/*
TxPacket() send data to Wifi module (UART).
TxPacket() needs 2 parameters:
- Length of parameters (number of characters to be sent)
- Pointer to parameters to send.
uint8_t para que devuelva el numero real de bytes enviados...
*/
uint8_t TxPacket(uint8_t bParameterLength,const uint8_t *Parameters){
   uint8_t bytes_sent=0;
   for(bytes_sent=0;bytes_sent<bParameterLength;bytes_sent++){
      while(!TXD0_READY); // Comprobamos que el bufer de transmision este preparado
      UCA0TXBUF=Parameters[bytes_sent];
   }
   return bytes_sent;
}

//interrupcion de recepcion en la uart UCA0:
#pragma vector=EUSCI_A0_VECTOR
__interrupt void EUSCIA0_IRQHandler(void){
   switch(__even_in_range(UCA0IV,USCI_UART_UCTXCPTIFG)){
      case USCI_NONE: break;
      case USCI_UART_UCRXIFG:
         UCA0IFG&=~UCRXIFG;       // Clear interrupt
         UCA0IE&=~UCRXIE;         // Interrupciones desactivadas en RX
         RXData=UCA0RXBUF;        // Ponemo a la variable el dato del BUFFER
         Byte_Recibido=Si;        // Se ha recibido un byte por la UART
         UCA0IE|=UCRXIE;          // Interrupciones reactivadas en RX
         break;
      case USCI_UART_UCTXIFG: break;
      case USCI_UART_UCSTTIFG: break;
      case USCI_UART_UCTXCPTIFG: break;
   }
}

