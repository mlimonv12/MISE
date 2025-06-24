/*
 * timers.h
 *
 *  Created on: 27 abr. 2022
 *      Author: AJ & AB
 */

#ifndef TIMERS_H_
#define TIMERS_H_

/* Variable para las funciones que tengan un timeout */
extern volatile uint16_t timeout;

/* Inicializacion del reloj del microcontrolador */
void init_clocks(void);

/* Inicializacion de los timers para hacer la funcion delay */
void init_timers(void);

/* Inicializacion del timer para hacer la funcion timeout */
void init_timeout(void);

/* Funcion para resetear el timer timeout */
void reset_timeout(void);

/* Funcion que detecta si ha habido un timeout en una recepcion de la UART
*   timeout: limite en ms para detectar si ha habido un timeout
*  la funcion retorna (1) si hay un timeout y retorna (0) si esta todo okay (no timeout)
*/
int8_t TimeOut(uint32_t timeout);

/* Funcion para desactivar el timeout */
void desactiva_timeout(void);

/* Funcion para gestionar un delay en el software
*   miliseconds: timepo en ms que queremos esperar
*/
void delay_ms(uint32_t miliseconds);

#endif /* TIMERS_H_ */
