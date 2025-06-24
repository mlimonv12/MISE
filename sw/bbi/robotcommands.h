/*
 * robotcommands.h
 *
 *  Created on: 27 abr. 2022
 *      Author: AJ & AB
 */

#ifndef ROBOTCOMMANDS_H_
#define ROBOTCOMMANDS_H_


/*
 *   ID     COMMAND     PARAMETERS                  DESCRIPTION
 *
 *  0x10     0x00       LDIR LSPEED RDIR RSPEED     DESCRIBES THE DIRECTION (STOP: 0, BACKWARDS: 1, FORWARDS: 2) OF ROTATION OF EITHER WHEEL AND IT'S SPEED (0x00 THRU 0xFF)
 *
 *  0x10     0x0B       LCOLOR RCOLOR               DESCRIBES THE COLOR OF ONE OF EITHER RGB LEDS (OFF: 0, RED: 1, GREEN: 2, YELLOW: 3, BLUE: 4, PINK: 5, CYAN: 6, WHITE: 7)
 *
 *  0x10     0x1D       00L1L2L3R1R2R3 (1 BYTE)     DESCRIBES THE READINGS OF THE OPTO-COUPLERS (LINE FOLLOWING SENSOR)
 *
 *  0x10     0x0A                                   PID CONTROLLER
 *
 *  0x10     0x14                                   CONTROL OF S1 SERVOMOTOR
 *
 *  0x10     0x15                                   CONTROL OF S2 SERVOMOTOR
 *
 *  0x10     0x16                                   CONTROL OF S3 SERVOMOTOR
 *
 */

/* DEFINICIONES DE OTRAS FUNCIONES DEL ROBOT*/
#define pidcontrollerrobot 0x0A
#define servos1 0x14
#define servos2 0x15
#define servos3 0x16

/* DEFINICIONES DE SELECCION DE MODO FUNCIONAMIENTO*/
#define menu_inici  0
#define menu_manual 1
#define menu_auto   2
#define menu_line   3
#define menu_ultra  4
#define menu_wifi   5


/* DEFINICIONES DE LOS COLORES DEL LED DEL ROBOT*/
#define LEDsRGBrobot 0x0B
#define OFF 0
#define RED 1
#define GREEN 2
#define YELLOW 3
#define BLUE 4
#define PINK 5
#define CYAN 6
#define WHITE 7

/* Funciones para controlar los LEDs del robot via i2c */
void leds_off(void);
void leds_red(void);
void leds_green(void);
void leds_yellow(void);
void leds_blue(void);
void leds_pink(void);
void leds_cyan(void);
void leds_white(void);

/* Funcion para controlar de manera customizada los LEDs del robot
*   l_led: color del led izquierdo
*   r_led: color del led derecho
*/
void leds_custom(uint8_t l_led,uint8_t r_led);

/* DEFINICIONES DE CONTROL DEL MOTOR*/
#define motorrobot 0x00
#define stopmotor 0
#define forwardmotor 1
#define backwardmotor 2
#define SPEED 0xFF

/* Funciones para controlar los motores del robot via i2c
   duty: es un valor decimal entre 0 y 1 para indicar la velocidad el motor */

void move_stop(void);               // parar el robot
void move_forward(float duty);      // mover el robot hacia adelante
void move_backward(float duty);     // mover el robot hacia atrás
void move_left(float duty);         // mover el robot hacia la izquierda y adelante
void move_right(float duty);        // mover el robot hacia la derecha y adelante
void move_lspin(float duty);        // girar el robot sobre si mismo en sentido horario
void move_rspin(float duty);        // girar el robot sobre si mismo en sentido anti-horario
void move_bleft(float duty);        // mover el robot hacia la izquierda y atrás
void move_bright(float duty);       // mover el robot hacia la derecha y atrás

/* Funciones para controlar los motores desde el programa .py comunicandolo via wifi
   Hacemos nuevas funciones para ajustarse al formato del programa .py
   velocity: velocidad del motor de 0 a 255. */

void wifi_left_forward(uint8_t velocity);
void wifi_left_backward(uint8_t velocity);
void wifi_right_forward(uint8_t velocity);
void wifi_right_backward(uint8_t velocity);
void wifi_both_forward(uint8_t velocity);
void wifi_both_backward(uint8_t velocity);

/* DEFINICIONES DE FUNCION SEGUIDOR DE LINEA */

#define linesensorrobot 0x1D
#define front_sensor    0b001100
#define right_sensor    0b000110
#define left_sensor     0b011000

/* Funcion para leer los datos de los sensores del robot via i2c */

void lectura_linetrack(void);

#endif /* ROBOTCOMMANDS_H_ */
