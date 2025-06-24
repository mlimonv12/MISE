/*
 * AT.h
 *
 *  Created on: 01/2021
 *      Author: c. Serre, UB
 */

#ifndef AT_H_
#define AT_H_

#include <stdint.h>

/*
 * Esta libreria necesita otra libreria con las funciones especificas de la UART (p. ej "uart_wifi.h"),
 * a programar/adaptar para cada micro, y que contenga las siguientes funciones:
 *
    RxReturn RxPacket(uint32_t time_out);   //El timeout debería ser un valor en el rango de los ms.
                                            //Devuelve una struct con la respuesta recibida.

    uint8_t TxPacket(uint8_t bParameterLength, const uint8_t *Parameters);
                      //Los parametros son el numero de bytes a enviar y un puntero a los datos a enviar.
                      //Devuelve el numero de bytes realmente enviados. Es opcional, podría ser void.
 *
 * RxReturn debe ser una struct para almacenar la respuesta, y debe tener como mínimo los siguientes campos
 * para el buen funcionamiento de la libreria AT:
      typedef struct RxReturn
        {
        uint8_t StatusPacket[RX_BUFFER_SIZE]; //Para almacenar la trama recibida
        uint8_t time_out;   //Indica si ha habido un problema de timeout durante la recepcion
        uint16_t num_bytes;//El numero de bytes recibidos. Ojo: puede superar los 255 caracteres => tipo de 16 bits
        //Se puede ampliar con mas campos si se considera oportuno.
        //...
    }RxReturn;
 *
 * Los nombres
     RxReturn
     RxPacket
     TxReturn
    "uart_wifi.h"
 * se pueden cambiar, pero se tendran que indicar a la libreria AT,  editando las siguientes lineas,
 * para asignar los nombres reales de vuestra(s) libreria(s) a los nombres que se espera la libreria AT:
 */
#define LIBUART_AT "uart_alumnos.h" //el nombre de la libreria real de la UART
#define RxAT RxPacket           //el nombre real de la funcion de recepcion por la UART
#define TxAT TxPacket           //el nombre real de la funcion de transmision por la UART
#define RxATReturn RxReturn     //el nombre real de la struct para las respuestas recibidas por la UART

/*Credenciales para la conexion Wifi. Hay 2 opciones:
 * 1- Actuamos como cliente ("STA", estacion) de una Wifi existente
 * 2- Actuamos como servidor de una Wifi ("AP", punto de acceso)
 */
//1- Si actuamos como cliente ("STA", estacion) de una Wifi existente:
//#define SSID_STA "Nombre_del_AP" //El SSID del AP al que queremos conectarnos cuando somos cliente (STA)
//#define PWD_STA "Contrasenya_del_AP"
#define SSID_STA "TP-LINK_FF8C"
#define PWD_STA  "09437470"

//#define SSID_STA "N12L" //El SSID del AP al que queremos conectarnos cuando somos cliente (STA)
//#define PWD_STA "d1g1t4l!"


//2- Si actuamos como servidor de una Wifi ("AP", punto de acceso):
#define SSID_AP "MiSE" //asi nos llamamos cuando somos servidor (AP)
#define PWD_AP "c0nTra53nyA"    //cambiar por la contraseña deseada

#define MODO_STA 1  //Modo 1: Station
#define MODO_AP 2   //Modo 2: AP
#define AP_MODE MODO_STA //Modo inicial: cliente ("STA") por defecto.

/*
 *  A partir de aquí, lo que sigue NO se debería alterar!
 */

//Recursos para el ESP-01:
#define CONNECTED           0x08 //Otro equipo se acaba de conectar a nuestro modulo
#define CONEXION_STRING "CONNECT"
#define DISCONNECTED        0x10 //Un equipo conectado se acaba de desconectar.
#define DISCONEXION_STRING "CLOSED"

//Codigos de errores:
#define ERR_NO_AT -1 //El modulo no responde
#define ERR_WAIT_CONFIG -2 //Cuando somos STA, No se ha podido conectar al AP
#define ERR_NO_AP -3 //No se ha detetado el AP requerido
#define ERR_NO_SSID -4 //No se ha podido leer el nombre del SSID
#define ERR_TEXT_OVERFLOW -5 //El texto recibido es mas largo que el buffer de recepcion
#define ERR_CMD -7 //El comando recibido no se reconoce



/************************************************************************
 * Funcion para inicializar el modulo Wifi ESP-01.
 * El parametro "modo" sirve para indicar como queremos configurarlo:
        modo = 1: cliente (STA) de una red Wifi existente
        modo = 2: como servidor Wifi (AP)
 * Devuelve 0 (exito) o mensage de error < 0
 */
int8_t init_servidor_esp(uint8_t modo);

/************************************************************************
 * Lee y formatea la IP en una cadena adecuada para nuestra pantalla
 * cadena: la cadena imprimible a devolver
 * max_car: numero max de caracteres a almacenar en *cadena
 * Devuelve el numero de caracteres realmente escritos en *cadena
 */
uint8_t getIP(uint8_t max_car, uint8_t *cadena);

/************************************************************************
 * Lee y formatea el SSID y password en cadenas sendas
 * ssid: para almacenar el SSID como cadena imprimible a devolver
 * pwd: para almacenar el password como cadena imprimible a devolver
 * max_car: numero max de caracteres a almacenar en cada cadena *ssid y *pwd
 * Puede retornar un valor negativo si hay error:
 * devuelve 0 (exito) o mensage de error < 0
 */
int8_t getSSID(uint8_t max_car, uint8_t *ssid, uint8_t *pwd);

/************************************************************************
 * Un comando de test. Se mandan los caracteres ASCII
 *  'A' 'T' 0x0D 0x0A
 *  (equivalente a la cadena "AT\r\n")
 *  por la uart al modulo Wifi.
 * La respuesta deberia ser "OK".
 * Devuelve 1 si hay respuesta, 0 si el modulo no responde.
 */
uint8_t comando_AT();

/************************************************************************
 * Funcion para saber en que modo se ha configurado el modulo.
 * (Solo devuelve el valor de una variable de la libreria, no se comunica con el modulo Wifi)
 * 1: modo cliente (STA) de una red Wifi existente
 * 2: modo servidor Wifi (AP)
 */
uint8_t get_Modo_Wifi();

/************************************************************************
 * Funcion para saber si tenemos algun cliente conectado.
 * (Solo devuelve el valor de una variable de la libreria, no se comunica con el modulo Wifi)
 * 0: no hay ningun cliente
 * 1: hay al menos 1 cliente
 */
uint8_t getConState();

/*************************************************************************
 * Funcion para enviar una trama (o cualquier mensaje) por la Wifi
 * Los parametros son
       un puntero al buffer de datos a enviar (*mensaje)
       el numero de datos a enviar (medida)
 * Devuelve la respuesta en una struct
 */
struct RxATReturn enviar_wifi(uint8_t *mensaje, uint8_t medida);

/*
 * Funcion para comprobar posibles mensaje o tramas imprevistos (que no sean una respuesta
 * a la funcion enviar_wifi) que vengan por la wifi.
 * Se tendria que llamar periodicamente desde el main()
 * El mensaje recibido estara en la struct.
 * Si acaba con un timeout sin que haya llegado ningun mensaje, el numero de caracteres (num_bytes)
 * de la struc tendra valor 0.
 */
struct RxATReturn recibir_wifi();


#endif /* AT_H_ */
