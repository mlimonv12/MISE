/*
 * gpios.h
 *
 *  Created on: 27 abr. 2022
 *      Author: AJ & AB
 */

#ifndef GPIOS_H_
#define GPIOS_H_

#define LEFT 0
#define RIGHT 1
#define FORWARD 2
#define BACKWARD 3

/* Variables externas de las interrupciones */
extern volatile uint8_t joystick;
extern volatile uint8_t menu;
extern volatile uint8_t language;

/* Inicializacion de los GPIO */
void init_GPIOs(void);

/* Funcion para mover el robot segun *
*     la direccion del joystick      */
void move_joystick(void);

#endif /* GPIOS_H_ */
