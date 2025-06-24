/*
 * lcd.c
 *
 *  Created on: 27 abr. 2022
 *      Author: AJ & AB
 */

#include <msp430fr2355.h>
#include <stdint.h>

#include "lcd.h"
#include "timers.h"
#include "i2cAddr.h"
#include "i2c.h"

/* LCD CONFIGURATION */
void reset_LCD(void){                   // función para reiniciar el LCD
    P5OUT&=~BIT2;                       // P5.2 = 0
    delay_ms(2);                        // esperamos 2 ms
    P5OUT|=BIT2;                        // P5.2 = 1
    delay_ms(10);                       // esperamos 10 ms
}

void init_LCD(void){                    // función para inicializar el LCD
    reset_LCD();                        // reiniciamos el LCD
    uint8_t initLCDbuf[8];              // creamos un vector con 8 posiciones para enviar comandos de control de inicialización
    initLCDbuf[0]=0x00;                 // Write command
    initLCDbuf[1]=0x39;                 // Function set
    initLCDbuf[2]=0x14;                 // Internal OSC freq
    initLCDbuf[3]=0x74;                 // Contrast set
    initLCDbuf[4]=0x54;                 // Power/icon control/contrast set
    initLCDbuf[5]=0x6F;                 // Follower control
    initLCDbuf[6]=0x0C;                 // Display ON/OFF
    initLCDbuf[7]=0x01;                 // Clear Display
    I2C_send(addressLCD,initLCDbuf,8);  // enviamos el vector mediante el bus I2C para inicializar el LCD
    delay_ms(100);                      // esperamos 100 ms
}

void clear_LCD(void){                   // función para limpiar el LCD
    uint8_t clear_LCD[2];               // creamos un vector con 2 posiciones para enviar el comando necesario
    clear_LCD[0]=0x00;                  // Wrtie command
    clear_LCD[1]=0x01;                  // Clear Display
    I2C_send(addressLCD,clear_LCD,2);   // enviamos el vector mediante el bus I2C para limpiar el LCD
    delay_ms(2);                        // esperamos 2 ms
}

void return_home_LCD(void){             // función para enviar el cursor a la primera posición de la pantalla
    uint8_t home_LCD[2];                // creamos un vector con 2 posiciones para enviar el comando necesario
    home_LCD[0]=0x00;                   // Write command
    home_LCD[1]=0x03;                   // Set cursor to position 0x00 in DDRAM
    I2C_send(addressLCD,home_LCD,2);    // enviamos el vector mediante el bus I2C para poder enviar el cursor a la primera posición
}

void escribir_botline(void){            // función para escribir en la linea inferior del LCD
    uint8_t botline[2];                 // creamos un vector con 2 posiciones para enviar el comando necesario
    botline[0]=0x00;                    // Write command
    botline[1]=0xC0;                    // posiciones en la DDRAM de la pantalla (0x80+0x40)
    I2C_send(addressLCD,botline,2);     // enviamos el vector mediante el bus I2C para poder escribir en la linea inferior
}
