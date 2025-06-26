#include <msp430.h>
#include <stdio.h>
#include <string.h> 

#include "wifi.h"
#include "../low_level/AT.h"
#include "../low_level/uart.h"
#include "robot_control.h"
#include "lcd_control.h"
#include "robot_menu.h"
#include "../low_level/timers.h"

RxReturn wifi_rx;
wifi_info wifi_msg;

// Misc
uint8_t _i = 0;

void wifi_init(void) {
    uint8_t error_rx = 0;
    char ip_display_buffer[32]; // Buffer to hold IP address string for LCD

    // 1. Initial setup for Wi-Fi mode
    motors(0, 0, 0, 0); // Stop motors immediately upon entering Wi-Fi mode
    robot_LEDs(COLOR_RED, COLOR_BLUE); // Set LEDs to a default color (e.g., blue)
    update_LCD("Initializing..."); // Display initial mode message on LCD

    // 2. Initialize ESP-01 module (as server or client, based on AT.h configuration)
    error_rx = init_servidor_esp(MODO_STA);
    if (error_rx) {
        update_LCD("Wifi Init Error");
    }

    // 3. Get and display the robot's IP address
    getIP(14, conexion.IP);
    // Format the IP address string for display on the LCD
    snprintf(ip_display_buffer, sizeof(ip_display_buffer), "Listening on:   %s", conexion.IP);
    update_LCD(ip_display_buffer);
    delay_ms(2000); // Display IP for 2 seconds
}

void wifi_control(void) {
    uint8_t connected = 0;

    // Attempt to receive data from the Wi-Fi module
    wifi_rx = recibir_wifi();
    __no_operation();

    if (wifi_rx.num_bytes > 0) { // Check if any data was successfully received

        // Check the Wi-Fi client connection state
        connected = getConState();
        if (!connected)
            update_LCD("Disconnected"); // Display an error if connection state check fails
        else
            update_LCD("Connected");

        wifi_msg.id = wifi_rx.StatusPacket[2]; // Message ID
        wifi_msg.len = wifi_rx.StatusPacket[3]; // Message len
        wifi_msg.instr = wifi_rx.StatusPacket[4]; // Received instr

        for (_i = 0; _i < (wifi_msg.len-2); _i++)
            wifi_msg.param[_i] = wifi_rx.StatusPacket[_i+5];

        wifi_msg.chasum = wifi_rx.StatusPacket[wifi_rx.num_bytes-1];
        __no_operation();
        
        // Process the received instr packet based on the command type
        switch (wifi_msg.param[0]) {
            case LEDS:
                switch (wifi_msg.id)
                {
                    case LED_LEFT:
                        ledColor_left = wifi_msg.param[1];
                        break;
                    
                    case LED_RIGHT:
                        ledColor_right = wifi_msg.param[1];
                        break;

                    default:
                        ledColor_left = wifi_msg.param[1];
                        ledColor_right = wifi_msg.param[1];
                        break;
                }
                break;
                
            case MOTORS:
                switch (wifi_msg.id)
                {
                    case MOTOR_LEFT:
                        motors(wifi_msg.param[2], wifi_msg.param[1], 0, 0);
                        break;
                    
                    case MOTOR_RIGHT:
                        motors(0, 0, wifi_msg.param[2], wifi_msg.param[1]);
                        break;

                    default: // Both motors
                        motors(wifi_msg.param[2], wifi_msg.param[1], wifi_msg.param[2], wifi_msg.param[1]);
                        break;
                }

            default:
                break;
        }
    }
    else
    {
        //update_LCD("Waiting for connection");
    }
}
