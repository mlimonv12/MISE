/*
 * uart.c
 *
 *  Created on:
 *      Author:
 */
#include <msp430.h>
#include <stdint.h>
#include "uart.h"

uint8_t DatoLeido_UART, Byte_Recibido;

void init_uart_wifi(void) //UART configuration: UCA0, 115200bps
{
    UCA0CTLW0 |= UCSWRST; // Fem un reset de la USCI i que estigui â€œinoperativaâ€�
    UCA0CTLW0 |= UCSSEL__SMCLK;
    // UCSYNC=0 mode asÃ­ncron
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
    // ELS CÃ€LCULS EM DONEN 8, INICIALMENT DEIA 10?
    UCA0MCTLW |= (0xF7 << 8); // Desem el valor 0xF7 al camp UCBRS0 del registre,
                                // Config. corresponent a la part fraccional de N (second modulation stage select)
    P1SEL0 |= BIT7 | BIT6; // I/O funciÃ³: P1.7 = UART_TX, P1.6 = UART_RX
    P1SEL1 &= ~(BIT7 | BIT6); // I/O funciÃ³: P1.7 = UART_TX, P1.6 = UART_RX
    UCA0CTLW0 &= ~UCSWRST; // Reactivem la lÃ­nia de comunicacions sÃ¨rie
    UCA0IE |= UCRXIE; // Interrupcions activades per la recepciÃ³ a la UART
    UCA0IFG &= ~UCRXIFG; // Per si de cas, esborrem qualsevol activaciÃ³ dâ€™interrupciÃ³ fins ara
}

// Global variable to count timer ticks (1 tick = 100us)
volatile uint32_t timer_ticks = 0;

// Function to initialize Timer A0 for timeout
void Activa_Timer_Timeout(void) {
    TB1CTL |= (TBCLR | TBSSEL__SMCLK | MC__UP); // CLEAR+SMCLK+UPMODE
    TB1CCR0 = 16000;           // 16 MHz / 16000 = 100 kHz (1ms period)
    TB1CCTL0 |= CCIE;          // Enable interrupt
    timer_ticks = 0;
}

// Function to reset the timer count
void Reset_Timeout(void) {
    timer_ticks = 0;
}

// Function to check for timeout
uint8_t TimeOut(uint32_t timeout_t) {
    if (timer_ticks >= timeout_t) {
        return 1; // Timeout occurred
    } else {
        return 0; // No timeout
    }
}

/*
RxPacket() read data from UART buffer (received from Wifi module).
RxPacket() need a timeout parameter, in order to abort if no response.
RxPacket() return struct containing Received Packet.
 */
RxReturn RxPacket(uint32_t time_out) {
    struct RxReturn respuesta;
    uint16_t bCount;
    uint8_t Rx_time_out = 0;

    Activa_Timer_Timeout(); // Start the timer

    for (bCount = 0; bCount < RX_BUFFER_SIZE; bCount++) {
        Reset_Timeout();   // Reset the timer
        Byte_Recibido = 0; // Assuming 0 represents "No"

        while (!Byte_Recibido) { // While no byte received
            Rx_time_out = TimeOut(time_out); // Check for timeout
            if (Rx_time_out)
                break; // Exit while loop on timeout
        }

        if (Rx_time_out)
            break; // Exit for loop on timeout

        // Byte received, read it
        respuesta.StatusPacket[bCount] = DatoLeido_UART;
    } // end of for loop

    respuesta.time_out = Rx_time_out; // Store timeout status
    respuesta.num_bytes = bCount;     // Store number of bytes received

    return respuesta;
}

/*
TxPacket() send data to Wifi module (UART).
TxPacket() needs 2 parameters:
     Length of parameters (number of characters to be sent)
     Pointer to parameters to send.
 */
uint8_t TxPacket(uint8_t bParameterLength, const uint8_t *buffer) {
    volatile uint8_t bytesSent;
    bytesSent = 0;
    _NOP();
    while (bytesSent < bParameterLength) {
        //while (!TXD0_READY); // Wait for TX buffer to be ready
        UCA0TXBUF = buffer[bytesSent++];
        _NOP();
    }

    return bytesSent;
}


// Timer A0 interrupt service routine (ISR) - for CCR0
#pragma vector = TIMER1_B0_VECTOR
__interrupt void timerB1_0_isr(void)
{
    TB1CTL &= ~CCIFG; // CLEAR FLAG
    timer_ticks++;
}

//interrupcion de recepcion en la uart UCA0:
#pragma vector = EUSCI_A0_VECTOR
__interrupt void EUSCIA0_IRQHandler(void) {
    switch (__even_in_range(UCA0IV, 4)) {
        case 0:
            break; // No interrupt
        case 2:    // RX interrupt
            UCA0IE &= ~UCRXIE; // Disable RX interrupt
            DatoLeido_UART = UCA0RXBUF;
            Byte_Recibido = 1; // Signal byte received
            UCA0IE |= UCRXIE;  // Re-enable RX interrupt
            break;
        case 4:    // TX interrupt
            break;
        default:
            break;
    }
}
