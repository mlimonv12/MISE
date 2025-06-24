/*
 * lcd.h
 *
 *  Created on: 27 abr. 2022
 *      Author: AJ & AB
 */

#ifndef LCD_H_
#define LCD_H_

/* Funcion para resetear el recurso del LCD
*  apagando el gpio y volviendolo a encender*/
void reset_LCD(void);

/* Inicializacion del LCD segun los comandos 
*  del datasheet */
void init_LCD(void);

/* Funcion para borrar lo que haya escrito en la LCD */
void clear_LCD(void);

/* Funcion para volver el cursor al inicion de la LCD */
void return_home_LCD(void);

/* Funcion para escribir en la segunda linea de la LCD */
void escribir_botline(void);

#endif /* LCD_H_ */
