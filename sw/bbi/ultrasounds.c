/*
 * ultrasounds.c
 *
 *  Created on: 25 de maig 2022
 *      Author: AJ & AB
 */

#include <msp430fr2355.h>
#include <stdint.h>
#include <stdio.h>
#include "i2cAddr.h"
#include "robotcommands.h"
#include "timers.h"
#include "gpios.h"
#include "i2c.h"
#include "lcd.h"
#include "adc.h"
#include "ultrasounds.h"

uint8_t compareDone=0,reset;
uint16_t negEdge,posEdge,distance,diff;
const uint16_t onecycle=30;

// la frecuencia del ACLK es 32768 Hz, así que 1/32768 = 30.5us implica un 1 tick contado
// la formula necesaria para la medida de la distancia necesita el tiempo en microsegundos microseconds así que onecycle vale 30
// el ancho del pulso de la señal "echo" es dada por (onecycle*diff)/58

void read_distance(void){						// Función para leer la distancia del sensor de ultrasonidos
    P4DIR|=BIT4;                                // seleccionamos P4.4 como salida (trigger)
	P4OUT&=~BIT4;                               // P4.4 = 0
	P6DIR&=~BIT0;                               // seleccionamos P6.0 como entrada (echo)
	P6SEL0|=BIT0;                               // seleccionamos la función secundaria del pin P6.0
	P6SEL1&=~BIT0;                              // TB3.1 capture input
	PM5CTL0&=~LOCKLPM5;                         // Disable the GPIO power-on default high-impedance mode to activate previously configured port settings
	TB3CCTL1=CM_3|CCIS_0|SCS|CAP|CCIE;          // Timer_B 3 capture compare control register 1 configuration:
	                                            // CM_3 -> capturar en ambos flancos, subida y bajada
	                                            // CCIS_0 -> señal a capturar en CCI1A (P6.0)
	                                            // SCS -> captura sincrona
	                                            // CAP -> modo captura
	                                            // CCIE -> capture/compare interrupt enable
	P4OUT|=BIT4;                                // P4.4 = 1
	delay_ms(1);                                // esperamos 1 ms
	P4OUT&=~BIT4;                               // P4.4 = 0
	TB3CTL=TBSSEL_1|MC_2|TBIE;                  // Timer_B 3 control register configuration:
	                                            // TBSSEL_1 -> ACLK (32768Hz) como fuente de reloj para el timer
	                                            // MC_2 -> modo continuo
	                                            // TBIE -> timerB interrupt enable
	__bis_SR_register(GIE);                     // activate interrupts at global level
	if(compareDone==0x02){
	    __disable_interrupt();              	// disable interrupts while calculus run
	    TB3CTL=TBCLR;                       	// reset timer
	    diff=negEdge-posEdge;               	// calculamos la diferencia temporal entre flancos
	    distance=(onecycle*diff)/58;        	// calculamos la distancia
	    compareDone=0x00;                   	// despues de los calculos pasamos al estado de espera de flanco de subida
	    __enable_interrupt();              		// activate interrupts
		P4OUT|=BIT4;                            // P4.4 = 1
		delay_ms(1);                            // espera 1 ms
		P4OUT&=~BIT4;                           // P4.4 = 0
		TB3CTL=TBSSEL_1|MC_2|TBIE;              // Timer_B 3 control register configuration:
	                                            // TBSSEL_1 -> ACLK (32768Hz) como fuente de reloj para el timer
	                                            // MC_2 -> modo continuo
	                                            // TBIE -> timerB interrupt enable
	    reset=0;                            	// set reset counter to 0
	}
	delay_ms(800);                          	// esperamos 800 ms
	reset++;                                	// increase reset counter
	if(reset==10){                          	// si fallan los calculos reiniciamos el sistema
	    __disable_interrupt();              	// disable interrupts
	    TB3CTL=TBCLR;                       	// reset timer
	    compareDone=0x00;                   	// pasamos al estado de espera de flanco de subida
	    __enable_interrupt();               	// activate interrupts
		P4OUT|=BIT4;                            // P4.4 = 1
		delay_ms(1);                            // espera 1 ms
		P4OUT&=~BIT4;                           // P4.4 = 0
		TB3CTL=TBSSEL_1|MC_2|TBIE;              // Timer_B 3 control register configuration:
	                                            // TBSSEL_1 -> ACLK (32768Hz) como fuente de reloj para el timer
	                                            // MC_2 -> modo continuo
	                                            // TBIE -> timerB interrupt enable
	    reset=0x00;                         	// set reset counter to 0
	}
}

void movement_ultrasounds(void){				// Función para mover el robot en función de la distancia medida con el sensor de ultrasonidos
    read_distance();							// leemos la distancia del sensor al objeto más proximo
    if(distance>10){							// si la distancia leida es mayor a un cierto umbral
        move_forward(0.5);						// el robot avanzará
    }else{										// en caso contrario
        move_rspin(0.2);						// el robot girará sobre si mismo en sentido anti-horario para evitar el obstaculo
    }
    delay_ms(50);								// esperamos 50 ms
}

#pragma vector=TIMER3_B1_VECTOR
__interrupt void Timer3_B1_ISR(void){
    __bic_SR_register_on_exit(LPM0_bits);
    switch(TB3IV){
    case TBIV__TBCCR1:
        if(compareDone==0x00){
            //Capture the timer value when echo pin becomes high (ranging start)
            posEdge=TB3CCR1;
            compareDone=0x01;
        }else if(compareDone==0x01){
            //Capture the timer value when echo pin becomes low (ranging stops)
            negEdge=TB3CCR1;
            compareDone=0x02;
        }
        break;
    default:
        break;
    }
}
