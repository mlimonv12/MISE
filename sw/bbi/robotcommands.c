/*
 * robotcommands.c
 *
 *  Created on: 7 may. 2022
 *      Author: AJ & AB
 */

#include <msp430fr2355.h>
#include <stdint.h>
#include "robotcommands.h"
#include "i2c.h"
#include "i2cAddr.h"

uint8_t led_color[3];
uint8_t move_control[5];
uint8_t comando_linetrack[1];
uint8_t sensor_line[1];

/* COLORES DEL LED DEL ROBOT*/
void leds_off(void){
    led_color[0]=LEDsRGBrobot;
    led_color[1]=OFF;
    led_color[2]=OFF;
    I2C_send(addressRobot,led_color,3);
}

void leds_red(void){
    led_color[0]=LEDsRGBrobot;
    led_color[1]=RED;
    led_color[2]=RED;
    I2C_send(addressRobot,led_color,3);
}

void leds_green(void){
    led_color[0]=LEDsRGBrobot;
    led_color[1]=GREEN;
    led_color[2]=GREEN;
    I2C_send(addressRobot,led_color,3);
}

void leds_yellow(void){
    led_color[0]=LEDsRGBrobot;
    led_color[1]=YELLOW;
    led_color[2]=YELLOW;
    I2C_send(addressRobot,led_color,3);
}

void leds_blue(void){
    led_color[0]=LEDsRGBrobot;
    led_color[1]=BLUE;
    led_color[2]=BLUE;
    I2C_send(addressRobot,led_color,3);
}

void leds_pink(void){
    led_color[0]=LEDsRGBrobot;
    led_color[1]=PINK;
    led_color[2]=PINK;
    I2C_send(addressRobot,led_color,3);
}

void leds_cyan(void){
    led_color[0]=LEDsRGBrobot;
    led_color[1]=CYAN;
    led_color[2]=CYAN;
    I2C_send(addressRobot,led_color,3);
}

void leds_white(void){
    led_color[0]=LEDsRGBrobot;
    led_color[1]=WHITE;
    led_color[2]=WHITE;
    I2C_send(addressRobot,led_color,3);
}

void leds_custom(uint8_t l_led,uint8_t r_led){
    led_color[0]=LEDsRGBrobot;
    led_color[1]=l_led;
    led_color[2]=r_led;
    I2C_send(addressRobot,led_color,3);
}

/* CONTROL DEL MOTOR*/
void move_stop(void){
    move_control[0]=motorrobot;
    move_control[1]=stopmotor;
    move_control[2]=0;
    move_control[3]=stopmotor;
    move_control[4]=0;
    I2C_send(addressRobot,move_control,5);
}

void move_forward(float duty){
    move_control[0]=motorrobot;
    move_control[1]=forwardmotor;
    move_control[2]=duty*SPEED;
    move_control[3]=forwardmotor;
    move_control[4]=duty*SPEED;
    I2C_send(addressRobot,move_control,5);
}

void move_backward(float duty){
    move_control[0]=motorrobot;
    move_control[1]=backwardmotor;
    move_control[2]=duty*SPEED;
    move_control[3]=backwardmotor;
    move_control[4]=duty*SPEED;
    I2C_send(addressRobot,move_control,5);
}

void move_left(float duty){
    move_control[0]=motorrobot;
    move_control[1]=stopmotor;
    move_control[2]=0;
    move_control[3]=forwardmotor;
    move_control[4]=duty*SPEED;
    I2C_send(addressRobot,move_control,5);
}

void move_right(float duty){
    move_control[0]=motorrobot;
    move_control[1]=forwardmotor;
    move_control[2]=duty*SPEED;
    move_control[3]=stopmotor;
    move_control[4]=0;
    I2C_send(addressRobot,move_control,5);
}

void move_lspin(float duty){
    move_control[0]=motorrobot;
    move_control[1]=forwardmotor;
    move_control[2]=duty*SPEED;
    move_control[3]=backwardmotor;
    move_control[4]=duty*SPEED;
    I2C_send(addressRobot,move_control,5);
}

void move_rspin(float duty){
    move_control[0]=motorrobot;
    move_control[1]=backwardmotor;
    move_control[2]=duty*SPEED;
    move_control[3]=forwardmotor;
    move_control[4]=duty*SPEED;
    I2C_send(addressRobot,move_control,5);
}

void move_bleft(float duty){
    move_control[0]=motorrobot;
    move_control[1]=stopmotor;
    move_control[2]=0;
    move_control[3]=backwardmotor;
    move_control[4]=duty*SPEED;
    I2C_send(addressRobot,move_control,5);
}

void move_bright(float duty){
    move_control[0]=motorrobot;
    move_control[1]=backwardmotor;
    move_control[2]=duty*SPEED;
    move_control[3]=stopmotor;
    move_control[4]=0;
    I2C_send(addressRobot,move_control,5);
}

/* MOVIMIENTO DE LOS MOTORES VIA WIFI */
void wifi_left_forward(uint8_t velocity){
    move_control[0]=motorrobot;
    move_control[1]=forwardmotor;
    move_control[2]=velocity;
    move_control[3]=stopmotor;
    move_control[4]=0;
    I2C_send(addressRobot,move_control,5);
}

void wifi_left_backward(uint8_t velocity){
    move_control[0]=motorrobot;
    move_control[1]=backwardmotor;
    move_control[2]=velocity;
    move_control[3]=stopmotor;
    move_control[4]=0;
    I2C_send(addressRobot,move_control,5);
}

void wifi_right_forward(uint8_t velocity){
    move_control[0]=motorrobot;
    move_control[1]=stopmotor;
    move_control[2]=0;
    move_control[3]=forwardmotor;
    move_control[4]=velocity;
    I2C_send(addressRobot,move_control,5);
}

void wifi_right_backward(uint8_t velocity){
    move_control[0]=motorrobot;
    move_control[1]=stopmotor;
    move_control[2]=0;
    move_control[3]=backwardmotor;
    move_control[4]=velocity;
    I2C_send(addressRobot,move_control,5);
}

void wifi_both_forward(uint8_t velocity){
    move_control[0]=motorrobot;
    move_control[1]=forwardmotor;
    move_control[2]=velocity;
    move_control[3]=forwardmotor;
    move_control[4]=velocity;
    I2C_send(addressRobot,move_control,5);
}

void wifi_both_backward(uint8_t velocity){
    move_control[0]=motorrobot;
    move_control[1]=backwardmotor;
    move_control[2]=velocity;
    move_control[3]=backwardmotor;
    move_control[4]=velocity;
    I2C_send(addressRobot,move_control,5);
}

/* MODO SEGUIDOR DE LINEA*/
void lectura_linetrack(void){
    comando_linetrack[0]=linesensorrobot;
    I2C_send(addressRobot,comando_linetrack,1);
    I2C_receive(addressRobot,sensor_line,1);
    if(sensor_line[0]==front_sensor){
        move_forward(0.2);
        leds_green();
    }else if(sensor_line[0]<front_sensor&&sensor_line[0]!=0x00){
        move_left(0.2);
        leds_custom(GREEN,RED);
    }else if(sensor_line[0]>front_sensor&&sensor_line[0]<0x3F){
        move_right(0.2);
        leds_custom(RED,GREEN);
    }else{
        move_stop();
        leds_red();
    }
}
