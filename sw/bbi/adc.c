/*
 * adc.c
 *
 *  Created on: 3 may. 2022
 *      Author: AJ & AB
 */

#include <msp430fr2355.h>
#include <stdint.h>
#include "adc.h"
#include "robotcommands.h"

uint8_t media=0;
uint16_t right_ldr=0;
uint16_t left_ldr=0;

void init_adc(){
    //P5.0 A8 P5.1 A9
    P5SEL0|=BIT1|BIT0;                      // Enable A/D channel inputs
    P5SEL1&=~(BIT1|BIT0);                   // Enable A/D channel inputs
    PM5CTL0&=~LOCKLPM5;                     // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings
    ADCCTL0|=ADCON+ADCSHT_2;                // Turn on ADC12, S&H=16 clks
    ADCCTL1|=ADCSHP+ADCCONSEQ_0;            // ADCLK=MODOSC,
    ADCCTL2&=~ADCRES;                       // clear ADCRES in ADCTL
    ADCCTL2|=ADCRES_2;                      // 12 bits resolution
    ADCIE=ADCIE0;                           // Habilita interrupcion
}   

uint16_t measure_A8(){
    uint16_t i=0;
    // Escogemos el Canal A8
    ADCMCTL0|=ADCINCH_8;
    for (i=15;i>0;i--){
        //Habilitacion de la conversion, empezamos el muestreo y la conversion
        ADCCTL0|=ADCENC+ADCSC;
        // Guardamos el resultado en en nuestro buffer medidas
        left_ldr=left_ldr+ADCMEM0;
    }
    // Se han acabado las 16 medidas, apagamos el ADC
    left_ldr=left_ldr>>4;
    // Deshablilitamos el ADC para pode cambiar de canal
    ADCCTL0&=~ADCENC;
    ADCCTL0&=~ADCON;
    // Quitamos el canal A8
    ADCMCTL0^=ADCINCH_8;
    return left_ldr;
}

uint16_t measure_A9(){
    uint16_t j=0;
    // Volvemos a encender el ADC:
    ADCCTL0|=ADCON;
    // Desactivamos el canal A8 y escogemos el Canal A9
    ADCMCTL0|=ADCINCH_9;
    for(j=15;j>0;j--){
        //Habilitacion de la conversion, empezamos el muestreo y la conversion
        ADCCTL0|=ADCENC+ADCSC;
        // Guardamos el resultado en en nuestro buffer medidas
        right_ldr=right_ldr+ADCMEM0;
    }
    right_ldr=right_ldr>>4;
    // Se han acabado las 16 medidas, apagamos el ADC
    // Deshablilitamos el ADC para pode cambiar de canal
    ADCCTL0&=~ADCENC;
    ADCCTL0&=~ADCON;
    // Quitamos el canal 9
    ADCMCTL0^=ADCINCH_9;
    return right_ldr;
}

/* MODO AUTOMATICO CON LDR */
void movement_ldr(){
    if(left_ldr>70){            // Comparamos si el valor del valor del LDR izquierdo supera un umbral
        move_bleft(0.5);        // En tal caso hacemos que el robot gire hacia la izquierda
    }else if(right_ldr>70){     // Comparamos si el valor del valor del LDR derecho supera un umbral
        move_bright(0.5);       // En tal caso hacemos que el robot gire hacia la derecha
    }else{                      // Si no se cumple ninguna de las anteriores condiciones
        move_backward(0.5);     // Nos movemos en linea recta en la dirección de los LDR
    }
}

#pragma vector=ADC_VECTOR;
__interrupt void ADC_LDR_isr(void){
    uint16_t flag_ADC=ADCIV;
    switch (flag_ADC) {
        case 0x02: /* ADCMEM0 overflow -> Interrupt Flag: ADCOVIFG */
            break;
        case 0x04: /* Conversion time overflow -> Interrupt Flag: ADCTOVIFG */
            break;
        case 0x06: /* ADCHI Interrupt flag -> Interrupt Flag: ADCHIIFG */
            break;
        case 0x08: /* ADCLO Interrupt flag -> Interrupt Flag: ADCLOIFG */
            break;
        case 0x0A: /* ADCIN Interrupt flag -> Interrupt Flag: ADCINIFG */
            break;
        case 0x0C: /*  ADC memory Interrupt flag -> Interrupt Flag: ADCIFG0*/
            //__bis_SR_register_on_exit(LPM0_bits);
            ADCIFG = 0;
            break;
        default:
            break;
    }
}
