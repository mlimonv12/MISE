/*
 * timers.c
 *
 *  Created on: 27 abr. 2022
 *      Author: AJ & AB
 */
#include <msp430fr2355.h>
#include <stdint.h>
#include "timers.h"

uint16_t counter;
uint8_t  delay_DONE;
uint8_t  delay_flag;
volatile uint16_t timeout_timer;

void init_clocks(void){ // Configure one FRAM waitstate as required by the device datasheet for MCLK operation beyond 8MHz before configuring the clock system
    FRCTL0 = FRCTLPW | NWAITS_1;
    P2SEL1 |= BIT6 | BIT7;                      // P2.6~P2.7: crystal pins
    do {
        CSCTL7 &= ~(XT1OFFG | DCOFFG);          // Clear XT1 and DCO fault flag
        SFRIFG1 &= ~OFIFG;
    } while (SFRIFG1 & OFIFG);                  // Test oscillator fault flag
    __bis_SR_register(SCG0);                    // disable FLL
    CSCTL3 |= SELREF__XT1CLK;                   // Set XT1 as FLL reference source
    //CSCTL1 = DCOFTRIMEN_1 | DCOFTRIM0 | DCOFTRIM1 | DCORSEL_5; // DCOFTRIM=5, DCO Range = 16MHz**
    CSCTL1 = DCORSEL_5;                         // DCOFTRIM=5, DCO Range = 16MHz
    CSCTL2 = FLLD_0 + 487;                      // DCOCLKDIV = 16MHz
    __delay_cycles(3);
    __bic_SR_register(SCG0);                    // enable FLL
    //Software_Trim();                          // Software Trim to get the best DCOFTRIM value**
    CSCTL4 = SELMS__DCOCLKDIV | SELA__XT1CLK;   // set XT1 (~32768Hz) as ACLK source, ACLK = 32768Hz
                                                // default DCOCLKDIV as MCLK and SMCLK source
    P1DIR |= BIT0 | BIT1;                       // set SMCLK, ACLK pin as output
    P1SEL1 |= BIT0 | BIT1;                      // set SMCLK and ACLK pin as second function
    PM5CTL0 &= ~LOCKLPM5; // Disable the GPIO power-on default high-impedance mode to activate previously configured port settings
}

void init_timers(void){
    /* Configuracion del control del timer B0 para la funcion delay
    *  Seleccionamos la fuente del timer como el ACLK = 32.768KHz   */
    TB0CTL|= MC_1|TBSSEL_1;
    TB0CCTL0&=~(CCIFG+CCIE);
}

void init_timeout(void){
    /* Configuracion del control del timer B1 para el timeout (UART)
    *  Usaremos el modo continuo y que la fuente de reloj sea de SMCLK (16MHz) */
    TB1CTL|= MC_2|TBSSEL_2;
    TB1CCTL0&=~(CCIFG+CCIE);
}

void reset_timeout(void){
    // Reseteamos el contador a 0.
    timeout_timer = 0;
    TB1CCTL0 = CCIE;
}

int8_t TimeOut(uint32_t out){
    if (timeout_timer < out){
        return 0; // Todo okay, no hay timeout
    } else {
        return 1; // Hay timeout
    }
}

void desactiva_timeout(void){
    TB1CTL^=MC_2;  //Desactivamos timer para que este STOP
    TB1CTL|=TBCLR; //Limpiamos la config del timer
}

void delay_ms(uint32_t miliseconds){
    TB0R=0;                       //ponemos el contador a 0
    TB0CCR0=miliseconds;          //ponemos el numero hasta el que tiene que contar como delayNumber
    TB0CCTL0=CCIE;                //activamos las interrupciones de este timer
    delay_flag=1;                 //delay flag
    while(!delay_DONE);
    delay_DONE=0;
}



//DIFERENTES TIMERS/CCR -> TIMERx_Bn_VECTOR
#pragma vector=TIMER0_B0_VECTOR
__interrupt void TIMERB0_ISR(void){
    // Como el timer funciona a 32000Hz y trabajamos en milisegundos, si pongo delay(1000),
    // TB0CCTL0 = 1000 y entrare 32 veces en un segundo
    if(delay_flag){
        if(counter<=32){
            counter+=1;
        }else{
            delay_DONE=1; //reseteamos la config del delay
            delay_flag=0;
            //comandos para resetear el delay
            TB0CCTL0&=~CCIE; //Desactivamos las interrupciones
            counter=0;
        }
    }
}

#pragma vector=TIMER1_B0_VECTOR
__interrupt void TIMERB1_ISR(void){
    timeout_timer++;
    TB1CCTL0&=~CCIFG; //Limpiamos bandera de interrupcion
}

