/*
 * gpios.c
 *
 *  Created on: 27 abr. 2022
 *      Author: AJ & AB
 */

#include <msp430fr2355.h>
#include <stdint.h>

#include "gpios.h"
#include "robotcommands.h"

extern volatile uint8_t joystick=5;
extern volatile uint8_t menu=0;
extern volatile uint8_t language=0;

void init_GPIOs(void){
    P1SEL0|=BIT6+BIT7;  // UART
    P1SEL1|=BIT0+BIT1;  // SMCLK/ACLK
    P1DIR&=~BIT6;       // RX -> input

    P2SEL0=0x00;
    P2SEL1|=BIT6+BIT7;  // XTAL pins
    P2DIR=0xFF;         // todos el puerto 2 como output

    P3SEL0=0x00;        // todos el puerto 3 como gpio
    P3SEL1=0x00;        // todos el puerto 3 como gpio
    P3DIR&=~(BIT0+BIT2+BIT3+BIT4+BIT5+BIT6);    // direcciones joystick + boton joystick + boton menu como inputs
    P3REN|=0xFF;        // habilitamos todas las resistencias en el puerto 3
    P3OUT|=0xFF;        // como pullup
    P3IE|=0xFF;         // habilitamos las interrupciones en todo el puerto 3
    P3IES|=0xFF;        // seleccionamos todas las interrupciones del puerto 3 a flanco de bajada
    P3IFG=0x00;         // limpiamos flag de interrupciones

    P4SEL0|=BIT6+BIT7;  // I2C
    P4DIR|=BIT4;        // Trigger ultrasonidos
    P4REN|=0xFF;        // habilitamos todas las resistencias en el puerto 4
    P4OUT|=0xFF;        // como pullup

    P5SEL0|=BIT0+BIT1;  // seleccionamos P5.0 y P5.1
    P5SEL1|=BIT0+BIT1;  // como A8 y A9
    P5DIR&=~BIT2;       // RST LCD
    P5OUT|=BIT2;        // P5.2 = 1

    P6SEL0|=BIT0+BIT1+BIT2+BIT3;    // TB3.(1/2/3/4)
    P6DIR|=BIT0+BIT1+BIT2+BIT3;     // TB3.CCI(1/2/3/4)A
}

void move_joystick(void){
    switch (joystick){
    case LEFT:
        move_left(0.3);
        break;
    case RIGHT:
        move_right(0.3);
        break;
    case FORWARD:
        move_forward(0.3);
        break;
    case BACKWARD:
        move_backward(0.3);
        break;
    default:
        joystick = 5;
        move_stop();
        break;
    }
}

#pragma vector=PORT3_VECTOR
__interrupt void PORT3_ISR(void){
   uint8_t flag_P3IV = P3IV; //Copiem el vector de interrupcions. De pas, al accedir a aquest vector, es neteja automaticament
   switch (__even_in_range(flag_P3IV, 16)) {
    case 0x02: // Aqui posem el que volem fer si s activa (Port 3.0)
        /* Hemos apretado el joystick para ir hacia la IZQUIERDA */
        P3IE &= ~BIT0; // Deactivamos las interrupciones en caso de rebote
        P3IE |= BIT0; //Los rebotes ya han acabado, es seguro volver a activar las interrupciones
        joystick = 0; //Left
        P3IFG = 0;
        break;
    case 0x04: // Aqui posem el que volem fer si s activa (Port 3.1)
        break;
    case 0x06: // Aqui posem el que volem fer si s activa (Port 3.2)
        /* Hemos apretado el joystick para ir hacia ATRAS */
        P3IE &= ~BIT2; // Deactivamos las interrupciones en caso de rebote
        P3IE |= BIT2; //Los rebotes ya han acabado, es seguro volver a activar las interrupciones
        if (menu==0 && (language>0 && language<=6)){
            language--;
        } else if(menu==0 && language==0){
            language=6;
        }
        joystick = 3;
        P3IFG = 0;
        break;
    case 0x08: // Aqui posem el que volem fer si s activa (Port 3.3)
        /* Hemos apretado el boton central del Joystick */
        P3IE &= ~BIT3; // Deactivamos las interrupciones en caso de rebote
        P3IE |= BIT3; //Los rebotes ya han acabado, es seguro volver a activar las interrupciones
        joystick = 5;
        P3IFG = 0;
        break;
    case 0x0A: // Aqui posem el que volem fer si s activa (Port 3.4)
        /* Hemos apretado el joystick para ir haceia la DERECHA */
        P3IE &= ~BIT4; // Deactivamos las interrupciones en caso de rebote
        P3IE |= BIT4; //Los rebotes ya han acabado, es seguro volver a activar las interrupciones
        joystick = 1;
        P3IFG = 0;   // Limpiamos la bandera de interrupcion antes de salir de la isr
        break;
    case 0x0C: // Aqui posem el que volem fer si s activa (Port 3.5)
        /* Hemos apretado para ir hacia ADELANTE */
        P3IE &= ~BIT5; // Deactivamos las interrupciones en caso de rebote
        P3IE |= BIT5; //Los rebotes ya han acabado, es seguro volver a activar las interrupciones
        if (menu==0 && (language>0 && language<6)){
            language++;
        } else if(menu==0 && language==6){
            language=0;
        }
        joystick = 2;
        P3IFG = 0;
        break;
    case 0x0E: // Aqui posem el que volem fer si s activa (Port 3.6)
        /* Hemos apretado el boton de seleccion de menu */
        P3IE &= ~BIT6;
        menu++;
        if (menu == menu_wifi){
            joystick = 5;
        }
        if (menu == 6){
            menu = 0;
        }
        P3IE |= BIT6;
        P3IFG = 0;
        break;
    case 0x10: // Aqui posem el que volem fer si s activa (Port 3.7)
        break;
    default:
        break;
    }
    P3IE = 0xFF; //Tornem a habilitar totes les interrupcions de P3 (P3.0 a P3.7)
}
