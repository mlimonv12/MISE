/*
 * uart.h
 *
 *  Created on: 22 ene. 2021
 *      Author: C. Serre, UB
 */

#ifndef UART_H_
#define UART_H_

#define RX_BUFFER_SIZE 512 // buffer UART. Ojo: puede superar los 255 caracteres

#define Baud_rate 115200                        // symbols per second, 1 Symbol = 1 bit so UART transfer speed: 115,2kb/s
typedef unsigned char byte;
#define TXD0_READY (UCA0IFG&UCTXIFG)
#define TXD0_BUSY  (UCA0STATW&UCBUSY)
#define RxATReturn RxReturn
#define No 0
#define Si 1
//Dispositivo del modulo al que ca la instruccion
#define LED_RGB 0x19
#define MOTORES 0x20
#define VelocidadMotor 0x20
#define SentidoGiroMotor 0x21
#define ValorLDR_L 0x2D
#define ValorLDR_R 0x2E
#define MASCARA_LOW 0000000011111111

//Estructura de datos cuando recibimos datos del programa py. Va a ser el protocolo INSTRUCTION PACKET.
typedef struct RecepcionWifi
{
    uint8_t id;                  // Identificador: 1 = Izquierda, 2 = Derecha, 0xFE = todos (Broadcast)
    uint8_t length;              // Longitud de bytes del paquete (num de parametros + 2)
    uint8_t instruction;         // Instruccion: 2 = READ, 3 = WRITE
    uint8_t parametre[5]; // Instrucciones y/o parametros para comunicar
    uint8_t check_sum;           // Parametro para detectar posibles errores de comunicacion
}RecepcionWifi;

//Estructura de datos de respuesta a una recepcion.
typedef struct RxReturn
{
    uint8_t StatusPacket[RX_BUFFER_SIZE]; //Para almacenar la trama recibida
    uint8_t time_out;   //Indica si ha habido un problema de timeout durante la recepcion
    uint16_t num_bytes;//El numero de bytes recibidos. Ojo: puede superar los 255 caracteres => tipo de 16 bits
    //Se puede ampliar con mas campos si se considera oportuno.
    //...
}RxReturn;

/*
RxPacket() read data from UART buffer (received from Wifi module).
RxPacket() need a timeout parameter in ms, in order to abort if no response.
RxPacket() return struct containing Received Packet.
 */
RxReturn RxPacket(uint32_t time_out); //timeout in ms

/*
TxPacket() send data to Wifi module (UART).
TxPacket() needs 2 parameters:
     Length of parameters (number of characters to be sent)
     Pointer to parameters to send.
     Returns number of characters actually sent
 */
uint8_t TxPacket(uint8_t bParameterLength, const uint8_t *Parameters);

void init_uart_wifi(void); //Configuracion de la UART


#endif /* UART_H_ */
