/*
 * AT.c
 *
 *  Created on: 01/2021
 *      Author: C. Serre, UB
 */

#include <msp430.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>


#include "AT.h"
#include LIBUART_AT //#include "uart_wifi.h"

typedef struct Conexion {
    uint8_t IP[16]; //4x3 digitos de IP + 3 puntos + \0 = 16 caracteres
    uint8_t conectado; //1: conectado, 0: no conectado
    uint8_t tipo;   //0: UDP, 1: TCP
    uint8_t conID; //'0'...'9' //El identificador de conexion, lo limitaremos a 1 caracter "0"..."9" = 0x30...0x39. Otro valor si no hay conexion activa
}StrConexion;

uint8_t WiFi_Mode = AP_MODE; //Modo 1: Station, 2: AP, 3: AP + Station
uint8_t SSID_actual[32]; //El SSID del AP al que estamos conectado.

//Mensajes de comunicacion para uart<->wifi:
const uint8_t echo_OFF[] = {"ATE0\r\n"}; //echoing of AT commands = off
const uint8_t echo_ON[] = {"ATE1\r\n"}; //echoing of AT commands = on
const uint8_t crear_AP[] = {"AT+CWSAP_CUR="}; //Para configurar el ESP como AP. SSID y pwd se definiran en AT.h. El resto en la funcion de creacion del AP

const uint8_t local_IP[] = {"AT+CIFSR\r\n"}; //Pedir propia IP, sea cual sea su papel (AP o STA)
const uint8_t lista_clientes[] = {"AT+CWLIF\r\n"}; //Pide la lista de clientes conectados a nosotros cuando somos AP
const uint8_t test[] = {"AT\r\n"}; //El comando mas basico para comprobar si el modulo responde. Debería contestar "OK"
const uint8_t version[] = {"AT+GMR\r\n"}; //Para comprobar la version de firmware del modulo

const uint8_t modo_Cliente[] = {"AT+CWMODE_CUR=1\r\n"}; //Para configurar el ESP en modo Cliente (o "Station")
const uint8_t conectar_AP[] = {"AT+CWJAP_CUR="}; //SSID y pwd se definiran en AT.h.
const uint8_t el_SSID[] = {"AT+CWJAP_CUR?\r\n"}; //Para preguntar donde estoy conectado como Station.
const uint8_t modo_AP[] = {"AT+CWMODE_CUR=2\r\n"}; //Para configurar el ESP en modo AP
const uint8_t quien_Soy[] = {"AT+CWSAP_CUR?\r\n"}; //Para pedir propio SSID cuando somos AP
const uint8_t modo_multi[] = {"AT+CIPMUX=1\r\n"}; //Activar modo mutli-conexiones (necesario para hacer de servidor)
const uint8_t modo_servidor[] = {"AT+CIPSERVER=1,80\r\n"}; //Activar modo servidor
const uint8_t estado_conexiones[] = {"AT+CIPSTATUS\r\n"}; //Pedir info de la(s) conexion(es) con cliente(s)
const uint8_t opt_lista_APs[] = {"AT+CWLAPOPT=1,3\r\n"}; //Ordenar por potencia de transmision, mostrar solo SSID y encriptacion.
const uint8_t lista_APs[] = {"AT+CWLAP\r\n"}; //Pedir la lista de los APs detectados
StrConexion conexion = {.conID = '0'}; //Una struct para almacenar los datos de una conexion
const uint8_t n_num_caract = 32; //numnero max de caracteres a enviar.
const uint8_t num_caract[] = {"32"}; //forma ASCII del numnero max de caracteres a enviar.
//const uint8_t retorno[] = {"\r\n"}; //0x0d 0x0A
//* No hace falta añadir a las cadenas el caracter nulo "\0" de terminacion,
//* el compilador ya lo hace.

const uint8_t Respuesta_OK [] = "AT OK";
const uint8_t Prefijo[] = "+IPD";

uint8_t get_Modo_Wifi(){
    return WiFi_Mode;
}

/*
 * Funcion para activar/desactivar el eco de los comandos que enviamos al ESP
 */
uint8_t comando_ECHO(uint8_t echo){
    struct RxATReturn respuesta;
    if(echo)
        TxAT(sizeof(echo_ON) - 1,echo_ON);
    else
        TxAT(sizeof(echo_OFF) - 1,echo_OFF);
    //y aqui viene una llamada a RxAT() para leer la respuesta:
    respuesta = RxAT(50);
    if(!respuesta.num_bytes) //no hay una respuesta
        return 0; //error
    else { //supondremos que si hay respuesta, sera "OK"
        return 1; //exito
    }
}

/*
 * Un comando de test "AT".
 * La respuesta deberia ser "OK".
 */
uint8_t comando_AT(){
    struct RxATReturn respuesta;
    TxAT(sizeof(test)-1,test);
    //llamada a RxAT() para leer la respuesta:
    respuesta = RxAT(50);
    if(respuesta.num_bytes) //hay una respuesta
        //supondremos que si hay respuesta, tiene que ser "OK"
        return 1; //exito
    return 0;
}

struct RxATReturn pedir_estado_conexiones(){
    struct RxATReturn respuesta;
    TxAT(sizeof(estado_conexiones)-1,estado_conexiones);
    respuesta = RxAT(20);
    return respuesta;
}

uint8_t getConID(){
    if((conexion.conID >= '0') && (conexion.conID <'9'))
        return conexion.conID; //es una var. global mia
    else return '?';
}

uint8_t getConState(){
	struct RxATReturn respuesta;
	TxAT(sizeof(estado_conexiones)-1, estado_conexiones);
	respuesta = RxAT(50);
	return conexion.conectado;
}
//Pedir propia IP, sea cual sea su papel (AP o STA)
struct RxATReturn pedir_mi_IP(){
    struct RxATReturn respuesta;
    TxAT(sizeof(local_IP)-1, local_IP);
    respuesta = RxAT(50);
    return respuesta;
}

/*
 * Para preguntar donde estoy conectado como Station,
 * o para pedir propio SSID y password cuando somos AP
 */
struct RxATReturn pedir_mi_SSID(){
    struct RxATReturn respuesta;
    respuesta.num_bytes = 0;
    if (WiFi_Mode == MODO_AP)
    	TxAT(sizeof(quien_Soy)-1,quien_Soy);
    else
    	TxAT(sizeof(el_SSID)-1,el_SSID);
    respuesta = RxAT(50);
    return respuesta;
}

/*
 * Funcion para formatear la IP en una cadena adecuada para nuestra pantalla
 * cadena: mensaje a destripar
 * texto: cadena imprimible a devolver
 * max_car: numero max de caracteres a almacenar en *texto
 */
int8_t pinta_IP(uint8_t max_car_linea, uint8_t *cadena, uint8_t *texto){
    uint8_t indice = 0, columna = 0;

//    while (cadena[indice]!= 58 /* buscamos el caracter ":"*/){
    //La cadena deberia contener:
    // IFSR:STAIP,"xxx.xxx.xxx.xxx"
    while (cadena[indice]!= 34 /* buscamos el caracter comillas dobles '"'*/){
        indice++;
        if (indice > RX_BUFFER_SIZE) return ERR_TEXT_OVERFLOW; //indicamos overflow
    }
    columna = 0;
    indice++;
//    while (cadena[indice]!= 34 /*comillas*/){
//        texto[columna] = cadena[indice];
//        indice++;
//        if (indice>RX_BUFFER_SIZE) return ERR_TEXT_OVERFLOW; //Salimos indicando overflow
//        if(columna < max_car_linea) columna++;
//        else return 0; //Si en este punto aun no hemos encontrado las comillas, no es normal, abortamos
//    }
//    indice++;
    while(cadena[indice] != 34 /*comillas*/){ //leemos hasta las segundas comillas
        texto[columna] = cadena[indice];
        indice++;
        if(indice>RX_BUFFER_SIZE) return ERR_TEXT_OVERFLOW;//Salimos indicando overflow
        if(columna < max_car_linea) columna++;
        else break; //El resto de caracteres que no caben en max_car se descartan...
    }
    texto[columna] = 0; //El caracter de terminacion de cadena.
    return columna; //Si llegamos aqui, devolvemos el numero de carecteres guardados
}


/*
 * Funcion para formatear el SSID y password en cadenas adecuadas para nuestra pantalla
 * respuesta: una struc que contiene el mensaje a destripar
 * ssid: para almacenar el SSID como cadena imprimible a devolver
 * texto: para almacenar el password (AP) o la MAC del AP (STA) como cadena imprimible a devolver
 * max_car: numero max de caracteres a almacenar en cada cadena *ssid y *texto
 */
int8_t pinta_SSID(uint8_t max_car_linea, struct RxATReturn respuesta, uint8_t *ssid, uint8_t *texto){
    uint8_t indice, columna = 0;
    if(memcmp(&respuesta.StatusPacket[16], "No AP",5) == 0){
        sprintf(texto, "SSID: No AP "); //Devuelve el numero de caracteres
        return ERR_NO_AP;
    }
    if(respuesta.StatusPacket[0] == 43 || respuesta.StatusPacket[2] == 43){ //buscamos el signo "+" (ASCII 43)
        //Depende si el echo esta on o off, empieza con CR LF seguido del signo "+", o directamente con "+"
        indice= 0;
        while(respuesta.StatusPacket[indice] != ':'){ //buscamos el caracter "="
            indice ++;
            if(indice > respuesta.num_bytes) return ERR_TEXT_OVERFLOW; //Salimos indicando overflow
        }
        indice+=2; //saltamos los : y unas "
        columna = 0;
        while(respuesta.StatusPacket[indice] != 34 /*comillas*/){ //buscamos las segundas comillas
            SSID_actual[columna] = respuesta.StatusPacket[indice];
            indice++;
            if(indice>RX_BUFFER_SIZE) return ERR_TEXT_OVERFLOW;//Salimos indicando overflow
            if(columna < max_car_linea) columna++;
            else break; //mas largo que una linea de mi LCD, salimos del while descartando lo que sobra.
        }
        SSID_actual[columna] = 0;
        sprintf(ssid, "%s", SSID_actual);
        indice+=3; //saltamos comillas, una coma, y otras comillas
        columna = 0;
        while(respuesta.StatusPacket[indice] != 34 /*comillas*/){ //buscamos las segundas comillas
            texto[columna] = respuesta.StatusPacket[indice];
            indice++;
            if(indice>RX_BUFFER_SIZE) return ERR_TEXT_OVERFLOW;//Salimos indicando overflow
            if(columna < max_car_linea) columna++;
            else break; //mas largo que una linea de mi LCD,  salimos del while descartando lo que sobra.
        }
        texto[columna] = 0;
        return 0; //indica que no ha habido ningun error
    }
    else{
        sprintf(ssid, "SSID: ??");
        sprintf(texto,"PWD: ??");
        return ERR_NO_SSID; //devolvemos un codigo de error
    }
}

/*
 * Leer y formatear la IP en una cadena adecuada para nuestra pantalla
 * cadena: la cadena imprimible a devolver
 * max_car: numero max de caracteres a almacenar en *cadena
 */
uint8_t getIP(uint8_t max_car, uint8_t *cadena){
    struct RxATReturn respuesta;
    uint8_t talla;
    respuesta = pedir_mi_IP();
    talla = pinta_IP(max_car, respuesta.StatusPacket, cadena);
    return talla; //devolvemos el numero de carecteres guardados
}

/*
 * Leer y formatear el SSID y password en cadenas adecuadas para nuestra pantalla
 * ssid: para almacenar el SSID como cadena imprimible a devolver
 * pwd: para almacenar el password (si somos AP), o la MAC (si somos STA) como cadena imprimible a devolver
 * max_car: numero max de caracteres a almacenar en cada cadena *ssid y *pwd, en adecuacion con nuestra pantalla
 * Puede retornar un valor negativo si hay error
 */
int8_t getSSID(uint8_t max_car, uint8_t *ssid, uint8_t *pwd){
    struct RxATReturn respuesta;
    int8_t talla;
    respuesta = pedir_mi_SSID();
    talla = pinta_SSID(max_car, respuesta, ssid, pwd);
    return talla;
}

int8_t esperar_configuracion(uint32_t TIME_OUT){
    uint32_t intentos = 0;
    struct RxATReturn respuesta;
    int8_t timeout = 1;
    uint8_t OK = 0, FAIL = 0;

    //    for(intentos = 0; intentos < 6; intentos++){//Reinicializo parte de la cadena de respuesta
    //        respuesta.StatusPacket[intentos] = 0;
    //    }
    memset(respuesta.StatusPacket, 0, sizeof(respuesta.StatusPacket));//Reinicializo la cadena de respuesta
    respuesta.num_bytes = 0;
    respuesta.time_out = 0;
    intentos = 0;
    while ((intentos < TIME_OUT) && !OK && !FAIL){//espero la palabra OK o FAIL, o un timeout
        respuesta.num_bytes = 0;
        respuesta.time_out = 0;
        respuesta = RxAT(100); //100x0.1ms = 10ms
        if (respuesta.num_bytes !=0) { //Hemos recibido una cadena
            if(strstr(respuesta.StatusPacket, "OK") ) {
                OK = 1;
                FAIL = 0;
                timeout = 0;
            }
            else if (strstr(respuesta.StatusPacket, "FAIL")) {
                FAIL = 1;
                OK = 0;
                timeout = 0;
            }
            respuesta.StatusPacket[respuesta.num_bytes] = 0; //Terminacion de cadena
        }
        intentos++;
    }
    if (timeout|| FAIL) return ERR_WAIT_CONFIG;
    else return 0;
}

int8_t init_servidor_esp(uint8_t modo){
    volatile int8_t error = 0;
    struct RxATReturn respuesta; //Aunque no se usa, me sirve para debugar en diferentes puntos de esta rutina
    uint8_t instruccion[64], talla;

    //Primero de todo, esta el modulo wifi?
    if(!comando_AT()) {
    	WiFi_Mode = 0;
    	return ERR_NO_AT; //Si no responde al comando de test mas basico "AT", abortamos
    }

    //Si ha respondido, primero le quito el eco:
    comando_ECHO(0); //echoing of AT commands = off


    //Y pasamos a configurar el servidor
    WiFi_Mode = modo;
    if (WiFi_Mode == MODO_AP) TxAT(sizeof(modo_AP)-1,modo_AP);
    else TxAT(sizeof(modo_Cliente)-1,modo_Cliente);
    respuesta = RxAT(50);

    if (WiFi_Mode == MODO_AP) { //Si somos AP:
        talla = sprintf(instruccion, "%s\"%s\",\"%s\",1,4\r\n", crear_AP, SSID_AP, PWD_AP);
        TxAT(talla,instruccion); //Creamos nuestro SSID
        respuesta = RxAT(50);
    }
    else { //Si somos cliente:
        talla = sprintf(instruccion, "%s\"%s\",\"%s\"\r\n", conectar_AP, SSID_STA, PWD_STA);
        //nos conectamos a un SSID predefinido con credenciales predefinidas
        TxAT(talla,instruccion); //nos conectamos a un SSID predefinido en "credenciales"
        respuesta = RxAT(50); //respuesta de confirmacion de la recepcion del comando
        //Ahora, hemos de recibir una serie de mensajes que confirman la configuracion:
        error = esperar_configuracion(30000); //30000 intentos, con un timeout de 10ms dentro de la funcion = 30s de espera
    }
    if (error < 0)  {
    	WiFi_Mode = 0;
        return error; //Abortamos con mensage de error < 0
    }

    TxAT(sizeof(modo_multi)-1,modo_multi);
    respuesta = RxAT(20);

    TxAT(sizeof(modo_servidor)-1,modo_servidor);
    respuesta = RxAT(20);

//    TxAT(sizeof(modo_uart_passthru)-1,modo_uart_passthru);
//    respuesta = RxAT(20);

    return error; //Si todo esta en orden, deberia retornar 0
}

struct RxATReturn decodificar_trama(struct RxATReturn trama){
    uint8_t indice;
    uint8_t numDatos[4] = {0}, *ptr;//caracteres: numero de datos transmitidos max 3 digitos + \0
    int16_t LenData = 0; //
    RxATReturn respuesta;
    respuesta.time_out = 0; //Si hemos entrado aqui, es que no ha habido timeout
    //Primero, miramos si se trata de un mensaje de estado AT respecto a una conexion/Desconexion:
    if (ptr = strstr(trama.StatusPacket, CONEXION_STRING)){ //mensaje "c,CONNECT", c = '0'...'3'
        //no es un comando "Bioloid" sino una info del modulo ESP, escribimos 'A' 'T'
        respuesta.StatusPacket[0] = 'A';
        respuesta.StatusPacket[1] = 'T';
        conexion.conID = trama.StatusPacket[0]; //identificador de la nueva conexion;
        conexion.conectado = 1;
        conexion.tipo = 1; //TCP
        respuesta.StatusPacket[2] = conexion.conID;
        respuesta.StatusPacket[3] = strlen(CONEXION_STRING); //LEN
        respuesta.StatusPacket[4] = CONNECTED; //INSTR
        memcpy(&respuesta.StatusPacket[5], CONEXION_STRING, strlen(CONEXION_STRING)); //PARAMS = CONEXION_STRING
        //Fin de trama 0D 0A:
        respuesta.StatusPacket[5 + strlen(CONEXION_STRING)] = 0x0D;
        respuesta.StatusPacket[5 + strlen(CONEXION_STRING) +1] = 0x0A;
        respuesta.num_bytes = 5 + strlen(CONEXION_STRING) + 2;
    }
    else if (ptr = strstr(trama.StatusPacket, DISCONEXION_STRING)){ //mensaje "c,CLOSED", c = '0'...'3'
        conexion.conectado = 0;
        conexion.conID = trama.StatusPacket[0]; //identificador de la conexion afectada
        respuesta.StatusPacket[0] = 'A';
        respuesta.StatusPacket[1] = 'T';
        respuesta.StatusPacket[2] = conexion.conID; //identificador de la conexion afectada
        respuesta.StatusPacket[3] = strlen(DISCONEXION_STRING); //LEN
        respuesta.StatusPacket[4] = DISCONNECTED; //INSTR
        memcpy(&respuesta.StatusPacket[5], DISCONEXION_STRING, strlen(DISCONEXION_STRING)); //PARAMS = DESCONEXION_STRING
        //Fin de trama 0D 0A:
        respuesta.StatusPacket[5 + strlen(DISCONEXION_STRING)] = 0x0D;
        respuesta.StatusPacket[5 + strlen(DISCONEXION_STRING) +1] = 0x0A;
        respuesta.num_bytes = 5 + strlen(DISCONEXION_STRING) + 2;
        conexion.conID = 0x00; //Identificador de la conexion: inactiva.
    }
    //Miramos si se trata de un mensaje Wifi retransmitido por la uart:
    else if(ptr = strstr(trama.StatusPacket, Prefijo)){//Buscamos "+IPD"
        ptr += strlen(Prefijo) + 3;//saltamos Prefijo + ',' + conID + ','
        indice = 0;
        while(*ptr != ':'){ //Leemos los digitos hasta encontrar ':'
            //reconstruimos el valor del numero de datos transmitidos
            numDatos[indice++] = *ptr++;
            if(indice >= 3)  //hemos recorrido mas de 3 digitos sin encontrar el caracter ':'
            	break; //No es una trama AT valida, abortamos
        }
        if (indice >= 3){//Al haber abortado, gestionamos el error
        	//descartamos la trama desconocida,
        	//y devolvemos una de error, con 6 bytes:
        	//	'?' '?' 0xFF 2 0 0
            LenData = 2; //ha habido un error, L = 0 + 2
            respuesta.StatusPacket[0] = '?';
            respuesta.StatusPacket[1] = '?';
            respuesta.StatusPacket[2] = 0xFF; //no hay ID
            respuesta.StatusPacket[3] = LenData; //2+0, no hay PARAMS
            respuesta.StatusPacket[4] = ERR_CMD; //-7 = 0xF9
            respuesta.StatusPacket[5] = 0; //Ponemos un valor checksum = 0, que tambien actua como fin de cadena en un printf
            respuesta.num_bytes = 6;
        }
        else {//Ya hemos alcanzado ":", y hemos almacenado los caracteres que representan los digitos del numero de bytes transmitidos
            numDatos[indice] = 0; //fin de cadena \0, para poder usar atoi()
            LenData = atoi(numDatos);//y transformar numDatos a valor uint16_t lenData
            ptr++; //Saltamos el caracter ":"
            //Al llegar aqui, si todo ha ido bien, ptr apunta al primer byte transmitido (p. ej. una trama Bioloid),
            // y LenData es la cantidad de bytes transmitidos:
            memcpy(respuesta.StatusPacket, ptr, LenData); //Copiamos todos los datos al status packet
            respuesta.num_bytes = LenData;
        }
    }
    //Si no hemos encontrado ningun código AT, no sabemos que c... hemos recibido:
    else respuesta = trama; //devolvemos la trama tal cual para depuracio.
    return respuesta;
}

struct RxATReturn enviar_wifi(uint8_t *mensaje, uint8_t medida){
    uint8_t trama[128];
    volatile uint8_t talla;
    struct RxATReturn respuesta;

    //Preparamos el ESP para recibir datos por la uart y enviarlos por wifi:
    talla = sprintf(trama, "AT+CIPSENDEX=%c,%d\r\n", conexion.conID, medida);
    TxAT(talla,trama);
    respuesta = RxAT(15);

    //Enviando la trama:
    TxAT(medida, mensaje);
    respuesta = RxAT(15);

    return respuesta;
}

//Comprobar posibles mensaje imprevistos que vengan por la wifi
//Se tendrá que llamar periodicamente desde el main()
struct RxATReturn recibir_wifi(){
    struct RxATReturn respuesta;
    respuesta.num_bytes = 0;
    respuesta = RxAT(100); //10ms: timeout relativamente largo, para maximizar el tiempo de escucha
    if (respuesta.num_bytes !=0) {
        respuesta = decodificar_trama(respuesta);
    }
    return respuesta;
}

