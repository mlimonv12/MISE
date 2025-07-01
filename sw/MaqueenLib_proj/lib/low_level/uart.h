/*
 * uart.h
 *
 *  Created on: 22 ene. 2021
 *      Author: C. Serre, UB
 */

#ifndef UART_H_
#define UART_H_

#define RX_BUFFER_SIZE 512 // buffer UART. Ojo: puede superar los 255 caracteres

#define RX_BUFFER_SIZE 512 // buffer UART. Ojo: puede superar los 255 caracteres

#define Baud_rate 115200                        // symbols per second, 1 Symbol = 1 bit so UART transfer speed: 115,2kb/s
typedef unsigned char byte;
#define TXD0_READY (UCA0IFG & UCTXIFG)
#define TXD0_BUSY  (UCA0STATW & UCBUSY)

typedef struct wifi_info
{
    uint8_t which;              // 1 = Left, 2 = Right, 0xFE = All
    uint8_t len;                // Command length
    uint8_t instr;              // W/R
    uint8_t id;                 // Device identifier (LEDs, Motors, Buzzer)
    uint8_t param[4];           // Misc. params
    uint8_t chasum;             // Checksum
} wifi_info;

//Estructura de datos de respuesta a una recepci�n.
typedef struct RxReturn
{
    uint8_t StatusPacket[RX_BUFFER_SIZE]; //Para almacenar la trama recibida
    uint8_t time_out;   //Indica si ha habido un problema de timeout durante la recepcion
    uint16_t num_bytes; //El numero de bytes recibidos. Ojo: puede superar los 255 caracteres => tipo de 16 bits

} RxReturn;

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
