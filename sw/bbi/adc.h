/*
 * adc.h
 *
 *  Created on: 3 may. 2022
 *      Author: AJ & AB
 */

#ifndef ADC_H_
#define ADC_H_

/* Inicializacion del ADC */
void init_adc();

/*   Funcion para medir el canal A8     *
*  Mediremos el nivel de luz que llega  *
*          al LDR izquierdo             *
*   retorna el valor del LDR izquierdo  */
uint16_t measure_A8();

/*   Funcion para medir el canal A9     *
*  Mediremos el nivel de luz que llega  *
*           al LDR derecho              *
*    retorna el valor del LDR derecho   */
uint16_t measure_A9();

/*  Funcion para mover al robot segun   *
*   los niveles de luz detectados en    *
*   las fotoresistencias                */
void movement_ldr();


#endif /* ADC_H_ */
