/*
 * i2c.h
 *
 *  Created on: 27 abr. 2022
 *      Author: AJ & AB
 */

#ifndef I2C_H_
#define I2C_H_

/* Inicializacion de la comunicacion i2c */
void init_I2C(void);

/* Funcion para enviar "n_dades" datos en la direccion "addr" via i2c
*   addr: direccion del esclavo que queremos 
*   buffer: array para enviar (byte de comando + datos a enviar)
*   n_dades: numero de bytes para enviar
*/
void I2C_send(uint8_t addr, uint8_t *buffer, uint8_t n_dades);

/* Funcion para recibir "n_dades" datos de la direccion "addr" via i2c
*   addr: direccion del esclavo con el que queremos comunicar 
*   buffer: array para recibir datos(byte de comando + datos a enviar)
*   n_dades: numero de bytes que queremos recibit
*/
void I2C_receive(uint8_t addr, uint8_t *buffer, uint8_t n_dades);

#endif /* I2C_H_ */
