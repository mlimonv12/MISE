#include <msp430fr2355.h>
#include <stdint.h>
#include <stdio.h>
#include "i2cAddr.h"
#include "robotcommands.h"
#include "languages.h"
#include "timers.h"
#include "gpios.h"
#include "i2c.h"
#include "lcd.h"
#include "adc.h"
#include "ultrasounds.h"
#include "uart_alumnos.h"
#include "AT.h"

char word[17];
uint8_t llarg;
uint8_t linea;
uint8_t resultadoAT;
uint8_t modo;
uint8_t conexion; // Variable para saber si se ha conectado con exito(0)
uint8_t cliente;
uint8_t ip_char;
char ip[14];
RxReturn recepcion;
RxReturn envio_LDR_l;
RxReturn envio_LDR_r;
RecepcionWifi modWifi;
uint8_t id;
uint8_t length;
uint8_t instruction;
uint8_t parametre;
uint8_t check_sum;

// Funciones que se realizaran en cada modo de funcionamiento:
void inicio(){
    /**********************************
    * Modo de funcionamiento inicial: *
    * Inicialmente el robot girara    *
    * sobre si mismo y paradearan los *
    * leds de color amarillo mientras *
    * la pantalla dice bienvenido en  *
    * el idioma seleccionado, el cual *
    * podemos cambiar con la acción   *
    * del joystick hacia atrás        *
    **********************************/
    clear_LCD();
    // segun el idioma seleccionado enviaremos un mensaje u otro
    switch(language){
        case CATALAN:
            llarg=sprintf(word,"@Benvingut!");
            I2C_send(addressLCD,word,llarg);
            break;
        case SPANISH:
            llarg=sprintf(word,"@Bienvenido!");
            I2C_send(addressLCD,word,llarg);
            break;
        case ENGLISH:
            llarg=sprintf(word,"@Welcome!");
            I2C_send(addressLCD,word,llarg);
            break;
        case PORTUGUESE:
            llarg=sprintf(word,"@Bem vindo!");
            I2C_send(addressLCD,word,llarg);
            break;
        case FRENCH:
            llarg=sprintf(word,"@Bienvenu!");
            I2C_send(addressLCD,word,llarg);
            break;
        case DEUTSCH:
            llarg=sprintf(word,"@Willkommen!");
            I2C_send(addressLCD,word,llarg);
            break;
        case JAPANESE:
            word[0]=0x40;   // enviar dato
            word[1]=0xD6;
            word[2]=0xB3;
            word[3]=0xBA;
            word[4]=0xBF;
            word[5]=0x21;
            // los caracteres en katakana necesarios no son compatibles con ascii, con lo que usamos valores del datasheet del LCD para saber que datos enviar
            // 0xD6, 0xB3, 0xBA, 0xBF, 0x21 (datasheet LCD, page 18, table 3)
            // transcrito en Romaji (https://en.wikipedia.org/wiki/Romanization_of_Japanese) como "Yookoso!"
            I2C_send(addressLCD,word,6);
            break;
    }
    leds_custom(YELLOW, OFF);
    delay_ms(500);
    leds_custom(OFF, YELLOW);
    move_lspin(1);
    delay_ms(500);
    move_stop();
    leds_off();
}

void manual(){
    /*********************************
    * Modo de funcionamiento manual: *
    *   Control mediante joystick    *
    *********************************/
    clear_LCD();
    // segun el idioma seleccionado enviaremos un mensaje u otro
    switch(language){
        case CATALAN:
            llarg=sprintf(word,"@Control joystick");
            I2C_send(addressLCD,word,llarg);
            break;
        case SPANISH:
            llarg=sprintf(word,"@Control joystick");
            I2C_send(addressLCD,word,llarg);
            break;
        case ENGLISH:
            llarg=sprintf(word,"@Joystick mode");
            I2C_send(addressLCD,word,llarg);
            break;
        case PORTUGUESE:
            llarg=sprintf(word,"@Control joystick");
            I2C_send(addressLCD,word,llarg);
            break;
        case FRENCH:
            llarg=sprintf(word,"@Mode manuel");
            I2C_send(addressLCD,word,llarg);
            break;
        case DEUTSCH:
            llarg=sprintf(word,"@Manueller modus");
            I2C_send(addressLCD,word,llarg);
            break;
        case JAPANESE:
            word[0]=0x40;   // enviar dato
            word[1]=0xBC;
            word[2]=0xDE;
            word[3]=0xAE;
            word[4]=0xB2;
            word[5]=0xBD;
            word[6]=0xC3;
            word[7]=0xA8;
            word[8]=0xAF;
            word[9]=0xB9;
            // los caracteres en katakana necesarios no son compatibles con ascii, con lo que usamos valores del datasheet del LCD para saber que datos enviar
            // 0xBC, 0xDE, 0xAE, 0xB2, 0xBD, 0xC3, 0xA8, 0xAF, 0xB9 (datasheet LCD, page 18, table 3)
            // transcrito en Romaji como "Joisutikku"
            I2C_send(addressLCD,word,10);
            break;
    }
    delay_ms(10);
    leds_pink();
    move_joystick();
}

void auto_ldr(){
    /******************************
    * Modo de funcionamiento LDR: *
    *     A.K.A. MODO POLILLA     *
    *  Control mediante las LDR   *
    *  En este modo el robot va a *
    *  seguir cualquier fuente de *
    *  luz, dirigiendose a esa    *
    *  direccion
    ******************************/
    clear_LCD();
    // segun el idioma seleccionado enviaremos un mensaje u otro
    switch(language){
        case CATALAN:
            llarg=sprintf(word,"@Seguidor de llum");
            I2C_send(addressLCD,word,llarg);
            break;
        case SPANISH:
            llarg=sprintf(word,"@Seguidor de luz");
            I2C_send(addressLCD,word,llarg);
            break;
        case ENGLISH:
            llarg=sprintf(word,"@Light follower");
            I2C_send(addressLCD,word,llarg);
            break;
        case PORTUGUESE:
            llarg=sprintf(word,"@Seguidor de luz");
            I2C_send(addressLCD,word,llarg);
            break;
        case FRENCH:
            llarg=sprintf(word,"@Suiv. de lumiere");
            I2C_send(addressLCD,word,llarg);
            break;
        case DEUTSCH:
            llarg=sprintf(word,"@Lichtfolger");
            I2C_send(addressLCD,word,llarg);
            break;
        case JAPANESE:
            word[0]=0x40; // enviar dato
            word[1]=0xD7;
            word[2]=0xB2;
            word[3]=0xC4;
            word[4]=0xCC;
            word[5]=0xAB;
            word[6]=0xDB;
            word[7]=0xB8;
            word[8]=0xB0;
            // los caracteres en katakana necesarios no son compatibles con ascii, con lo que usamos valores del datasheet del LCD para saber que datos enviar
            // 0xD7, 0xB2, 0xC4, 0xCC, 0xAB, 0xDB, 0xB8, 0xB0 (datasheet LCD, page 18, table 3)
            // transcrito en Romaji como "Raitoforowaa"
            I2C_send(addressLCD,word,9);
            break;
    }
    leds_white();
    init_adc();
    delay_ms(10);
    measure_A8();
    delay_ms(50);
    measure_A9();
    movement_ldr();
}

void auto_linetrack(){
    /*************************************
    * Modo de funcionamiento seguimiento *
    * de linea:                          *
    *  Control mediante sensores de luz  *
    * En este modo, el robot seguira un  *
    * circuito midiendo la direccion por *
    * los optoacopladores                *
    *************************************/
    clear_LCD();
    // segun el idioma seleccionado enviaremos un mensaje u otro
    switch(language){
        case CATALAN:
            llarg=sprintf(word,"@Seguidor linia");
            I2C_send(addressLCD,word,llarg);
            break;
        case SPANISH:
            llarg=sprintf(word,"@Seguidor linea");
            I2C_send(addressLCD,word,llarg);
            break;
        case ENGLISH:
            llarg=sprintf(word,"@Line follower");
            I2C_send(addressLCD,word,llarg);
            break;
        case PORTUGUESE:
            llarg=sprintf(word,"@Seguidor linha");
            I2C_send(addressLCD,word,llarg);
            break;
        case FRENCH:
            llarg=sprintf(word,"@Suiveur de ligne");
            I2C_send(addressLCD,word,llarg);
            break;
        case DEUTSCH:
            llarg=sprintf(word,"@Linienfolger");
            I2C_send(addressLCD,word,llarg);
            break;
        case JAPANESE:
            word[0]=0x40;
            word[1]=0xD7;
            word[2]=0xB2;
            word[3]=0xBF;
            word[4]=0xCC;
            word[5]=0xAB;
            word[6]=0xDB;
            word[7]=0xB8;
            word[8]=0xB0;
            // los caracteres en katakana necesarios no son compatibles con ascii, con lo que usamos valores del datasheet del LCD para saber que datos enviar
            // 0xD7, 0xB2, 0xBF, 0xCC, 0xAB, 0xDB, 0xB8, 0xB0 (datasheet LCD, page 18, table 3)
            // transcrito en Romaji como "Rainforowaa"
            I2C_send(addressLCD,word,9);
            break;
    }
    lectura_linetrack();
    delay_ms(50);

}

void auto_ultrasounds(void){
    /***************************************
    * Modo de funcionamiento ultrasonidos: *
    *            A.K.A. MODO ROOMBA        *
    *  Control mediante el sensor          *
    *  de ultrasonidos                     *
    *  En este modo el robot leerá los     *
    *  datos de un sensor de ultrasonidos, *
    *  cuando detecte un obstaculo a       *
    *  una cierta distancia, girará        *
    *  con tal de evitar dicho obstaculo   *
    ***************************************/
    clear_LCD();
    // segun el idioma seleccionado enviaremos un mensaje u otro
    switch(language){
        case CATALAN:
            llarg=sprintf(word,"@Ultrasons");
            I2C_send(addressLCD,word,llarg);
            break;
        case SPANISH:
            llarg=sprintf(word,"@Ultrasonidos");
            I2C_send(addressLCD,word,llarg);
            break;
        case ENGLISH:
            llarg=sprintf(word,"@Ultrasound");
            I2C_send(addressLCD,word,llarg);
            break;
        case PORTUGUESE:
            llarg=sprintf(word,"@Ultrassons");
            I2C_send(addressLCD,word,llarg);
            break;
        case FRENCH:
            llarg=sprintf(word,"@Ultrasons");
            I2C_send(addressLCD,word,llarg);
            break;
        case DEUTSCH:
            llarg=sprintf(word,"@Ultraschall");
            I2C_send(addressLCD,word,llarg);
            break;
        case JAPANESE:
            word[0]=0x40;
            word[1]=0xB7;
            word[2]=0xAE;
            word[3]=0xB3;
            word[4]=0xB5;
            word[5]=0xBF;
            word[6]=0xCA;
            word[7]=0xDF;
            // los caracteres en katakana necesarios no son compatibles con ascii, con lo que usamos valores del datasheet del LCD para saber que datos enviar
            // 0xB7, 0xAE, 0xB3, 0xB5, 0xBF, 0xCA, 0xDF (datasheet LCD, page 18, table 3)
            // transcrito en Romaji como "Chouonpa"
            I2C_send(addressLCD,word,8);
            break;
    }
    leds_cyan();
    delay_ms(10);
    movement_ultrasounds();
}

void wifi(){
    /**********************************
    * Modo de funcionamiento WiFi:    *
    * Comunicacion por UART con el PC *
    * y el robot                      *
    **********************************/
    move_stop();
    clear_LCD();
    // segun el idioma seleccionado enviaremos un mensaje u otro
    switch(language){
        case CATALAN:
            llarg=sprintf(word,"@Mode Wi-Fi");
            I2C_send(addressLCD,word,llarg);
            break;
        case SPANISH:
            llarg=sprintf(word,"@Modo Wi-Fi");
            I2C_send(addressLCD,word,llarg);
            break;
        case ENGLISH:
            llarg=sprintf(word,"@Wi-Fi mode");
            I2C_send(addressLCD,word,llarg);
            break;
        case PORTUGUESE:
            llarg=sprintf(word,"@Modo Wi-Fi");
            I2C_send(addressLCD,word,llarg);
            break;
        case FRENCH:
            llarg=sprintf(word,"@Mode Wi-Fi");
            I2C_send(addressLCD,word,llarg);
            break;
        case DEUTSCH:
            llarg=sprintf(word,"@Wi-Fi modus");
            I2C_send(addressLCD,word,llarg);
            break;
        case JAPANESE:
            word[0]=0x40;
            word[1]=0x57;
            word[2]=0x69;
            word[3]=0x2D;
            word[4]=0x46;
            word[5]=0x69;
            word[6]=0xD3;
            word[7]=0xB0;
            word[8]=0xC4;
            word[9]=0xDF;
            // los caracteres en katakana necesarios no son compatibles con ascii, con lo que usamos valores del datasheet del LCD para saber que datos enviar
            // 0x57, 0x69, 0x2D, 0x46, 0x69, 0xD3, 0xB0, 0xC4, 0xDF (datasheet LCD, page 18, table 3)
            // transcrito en Romaji como "Waifai moodo"
            I2C_send(addressLCD,word,10);
            break;
    }
    delay_ms(10);
    leds_blue();
    // Funciones de la libreria AT.h
    resultadoAT=comando_AT();
    // Comprobamos que haya conexion
    conexion=init_servidor_esp(MODO_STA);
    if(conexion!=0){ // Si el modulo wifi no responde, no nos podemos conectar
        escribir_botline();
        linea=sprintf(word,"@Not connected :(");
        I2C_send(addressLCD,word,linea);
    }
    // Detectamos la direccion IP del modulo y la imprimimos por pantalla
    ip_char=getIP(sizeof(ip)-1,ip);
    escribir_botline();
    linea=sprintf(word,"@IP:%s",ip);
    I2C_send(addressLCD,word,linea);
    // Comprobamos que estamos en el modo correcto
    modo=get_Modo_Wifi();
    // Vemos si hay algun cliente conectado
    cliente=getConState();
    if(!conexion){ // Si se ha establecido una conexion con exito:
        while(1){ // Segundo bucle infinito a la espera de instrucciones
            //Comprobamos posibles mensajes o tramas
            recepcion=recibir_wifi(); // si time_out = 1 es que no hay nada que se recibe
            if(recepcion.num_bytes){ // Si hemos recibido algo:
                if(joystick==3){ // Accionaremos el joystick hacia atras para enviar la info del ADC
                    init_adc(); // Volvemos a inicializar el ADC por si se ha modificado algun registro
                    while(1){
                        // Ahora vamos a enviar la info de los LDR:
                        uint8_t trama_ldr_l[9];
                        uint8_t trama_ldr_r[9];
                        uint16_t medida_left=0;
                        uint16_t medida_right=0;
                        uint8_t left_low=0;
                        uint8_t left_high=0;
                        uint8_t right_low=0;
                        uint8_t right_high=0;
                        medida_left=measure_A8();
                        left_high=(medida_left>>8); 
                        left_low=(medida_left&MASCARA_LOW);
                        medida_right=measure_A9();
                        right_high=(medida_right>>8); 
                        right_low=(medida_right&MASCARA_LOW);
                        //Creacion de la trama para enviar por wifi la info del ADC izquierdo
                        trama_ldr_l[0]=0xFF;
                        trama_ldr_l[1]=0xFF;
                        trama_ldr_l[2]=1;       //Izquierda
                        trama_ldr_l[3]=5;       //n parameter
                        trama_ldr_l[4]=3;       //write
                        trama_ldr_l[5]=0x2D;    //ldr_l
                        trama_ldr_l[6]=left_low;
                        trama_ldr_l[7]=left_high;
                        trama_ldr_l[8]=~(1+5+3+45+left_low+left_high);
                        //Creacion de la trama para enviar por wifi la info del ADC derecho
                        trama_ldr_r[0]=0xFF;
                        trama_ldr_r[1]=0xFF;
                        trama_ldr_r[2]=2;       // Derecha
                        trama_ldr_r[3]=5;       //n parameter
                        trama_ldr_r[4]=3;       //write
                        trama_ldr_r[5]=0x2D;    //ldr_r
                        trama_ldr_r[6]=right_low;
                        trama_ldr_r[7]=right_high;
                        trama_ldr_r[8]=~(2+5+3+45+right_low+right_high);
                        // Enviamos por wifi la informacion de las tramas del ADC
                        envio_LDR_l=enviar_wifi(trama_ldr_l,sizeof(trama_ldr_l));
                        delay_ms(1000);
                        envio_LDR_r=enviar_wifi(trama_ldr_r,sizeof(trama_ldr_r));
                        // En el caso de accionar cualquier boton del joystick dejamos de enviar la info de los ADC
                        if(joystick!=3){ 
                            break;
                        }
                    }
                }
                // Con la nueva estructura que hemos creado ordenamos los datos recibidos
                uint8_t contador;
                id=recepcion.StatusPacket[2];
                length=recepcion.StatusPacket[3];
                instruction=recepcion.StatusPacket[4];
                for(contador=0;contador<(length-2);contador++){
                    parametre=recepcion.StatusPacket[contador+5];
                    modWifi.parametre[contador]=parametre;
                }
                check_sum=recepcion.StatusPacket[recepcion.num_bytes-1];
                // Juntamos los datos a la nueva estructura:
                modWifi.id=id;
                modWifi.length=length;
                modWifi.instruction=instruction;
                modWifi.check_sum=check_sum;
                // Ahora, el objetivo es enviar por i2c al robot la info mandad por wifi
                switch(modWifi.parametre[0]){
                    case LED_RGB:
                        if(modWifi.id==1){ //Modificamos el led izquierdo
                            leds_custom(modWifi.parametre[1], 0);
                        }else if(modWifi.id==2){ // modificamos el led derecho
                        leds_custom(0,modWifi.parametre[1]);
                    }else{ //modificamos los dos leds
                        leds_custom(modWifi.parametre[1],modWifi.parametre[1]);
                    }
                    break;
                    case MOTORES:
                        if(modWifi.id==1){ //Modificamos el motor izquierdo
                            switch (modWifi.parametre[2]) {
                                case stopmotor: // Direccion STOP 
                                    move_stop();
                                    break;
                                case forwardmotor: // Direccion ADELANTE
                                    wifi_left_forward(modWifi.parametre[1]);
                                    break;
                                case backwardmotor: // Direccion ATRAS
                                    wifi_left_backward(modWifi.parametre[1]);
                                    break;
                        }
                    }else if(modWifi.id==2) { // modificamos el motor derecho
                        switch(modWifi.parametre[2]) {
                            case stopmotor: // Direccion STOP 
                                move_stop();
                                break;
                            case forwardmotor: // Direccion ADELANTE
                                wifi_right_forward(modWifi.parametre[1]);
                                break;
                            case backwardmotor: // Direccion ATRAS
                                wifi_right_backward(modWifi.parametre[1]);
                                break;
                        }
                    }else{ //modificamos los dos motores
                        switch (modWifi.parametre[2]) {
                            case stopmotor: // Direccion STOP 
                                move_stop();
                                break;
                            case forwardmotor: // Direccion ADELANTE
                                wifi_both_forward(modWifi.parametre[1]);
                                break;
                            case backwardmotor: // Direccion ATRAS
                                wifi_both_backward(modWifi.parametre[1]);
                                break;
                        }
                    }
                    break;
                }
            }
            if(menu==0){ // Si le damos al boton de cambiar de modo rompemos el bucle para ir al inicio
                break;
            }
        }
    }
}

void select_menu(menu){
    /***************************
     *    Seleccion de menu    *
     * Vamos a cambiar de menu *
     * segun apretemos el SW   *
     **************************/
    switch(menu){
    case menu_inici:
        inicio();
        break;
    case menu_manual:
        manual();
        break;
    case menu_auto:
        auto_ldr();
        break;
    case menu_line:
        auto_linetrack();
        break;
    case menu_ultra:
        auto_ultrasounds();
        break;
    case menu_wifi:
        wifi();
        break;
    default:
        clear_LCD();
        break;
    }
}


void main(void){
    __disable_interrupt();          // desactivamos interrupciones
    WDTCTL=WDTPW|WDTHOLD;           // stop watchdog timer
    init_clocks();                  //
    init_timers();                  //
    init_GPIOs();                   //
    init_I2C();                     //
    __enable_interrupt();           //
    init_LCD();                     //
    init_uart_wifi();               //
    init_timeout();                 //
    while(1){                       // Bucle a la espera de instrucciones
        select_menu(menu);          //
    }
}
