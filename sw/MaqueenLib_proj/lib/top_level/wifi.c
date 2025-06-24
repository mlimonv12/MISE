#include <msp430.h>
#include <stdio.h>      // For snprintf
#include <string.h>     // For string manipulation (if needed, e.g., memcpy)

#include "wifi.h"       // Include its own header
#include "../low_level/AT.h"         // Ensures access to AT functions
#include "../low_level/uart.h"       // Ensures access to UART types and instruction codes
#include "robot_control.h" // Ensures access to motor and LED control functions
#include "lcd_control.h" // Ensures access to LCD display functions
#include "robot_menu.h" // Access to currentNavigationMode and MAIN_LOOP_DELAY_MS
#include "../low_level/timers.h" // For delay_ms

// Global variables used for Wi-Fi communication (declared extern in AT.h and uart.h, robot_menu.h)
//StrConexion conexion;
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
    clear_LCD(); // Clear the LCD screen
    update_LCD("Wi-Fi Control"); // Display initial mode message on LCD
    //delay_ms(500); // Small delay for display to be visible

    // 2. Initialize ESP-01 module (as server or client, based on AT.h configuration)
    error_rx = init_servidor_esp(MODO_STA);
    if (error_rx) {
        clear_LCD();
        update_LCD("Wifi Init Error");
    }

    // 3. Get and display the robot's IP address
    getIP(14, conexion.IP); // This function (from AT.c) retrieves the IP and stores it in conexion.IP
    clear_LCD();
    // Format the IP address string for display on the LCD
    snprintf(ip_display_buffer, sizeof(ip_display_buffer), "Listening on:   %s", conexion.IP);
    update_LCD(ip_display_buffer);
    delay_ms(2000); // Display IP for 2 seconds
}

void wifi_control(void) {
    uint8_t error_rx = 0;

    // Main routine for Wi-Fi communication and command processing
    // Stay in this loop as long as the current navigation mode is set to Wi-Fi control (mode 3)
    // Attempt to receive data from the Wi-Fi module
    wifi_rx = recibir_wifi(); // This function (from AT.c) calls RxAT to get data
    __no_operation();

    if (wifi_rx.num_bytes > 0) { // Check if any data was successfully received
        // Periodically check the Wi-Fi client connection state
        error_rx = getConState(); // This function (from AT.c) updates conexion.conectado
        if (!error_rx) {
            clear_LCD();
            update_LCD("Conn. State Err"); // Display an error if connection state check fails
        } else {
            // If a client is connected, display confirmation or clear previous "No client" message
            clear_LCD();
            update_LCD("Connected");
        }

        wifi_msg.id = wifi_rx.StatusPacket[2]; // Message ID
        wifi_msg.len = wifi_rx.StatusPacket[3]; // Message len
        wifi_msg.instr = wifi_rx.StatusPacket[4]; // Received instr

        for (_i = 0; _i < (wifi_msg.len-2); _i++)
            wifi_msg.param[_i] = wifi_rx.StatusPacket[_i+5];

        wifi_msg.chasum = wifi_rx.StatusPacket[wifi_rx.num_bytes-1];
        __no_operation();
        
        // Process the received instr packet based on the command type
        switch (wifi_msg.param[0]) { // wifi_msg.instr holds the main command code (e.g., LED_RGB, MOTORES)
            case LEDS: // Command to control RGB LEDs (0x19, defined in uart.h)
                switch (wifi_msg.id)
                {
                    case LED_LEFT:
                        currentLeftLedColor = wifi_msg.param[1];
                        break;
                    
                    case LED_RIGHT:
                        currentRightLedColor = wifi_msg.param[1];
                        break;

                    default:
                        currentLeftLedColor = wifi_msg.param[1];
                        currentRightLedColor = wifi_msg.param[1];
                        break;
                }
                break;
                
            case MOTORS: // Command to control motors (0x20, defined in uart.h)
                switch (wifi_msg.id)
                {
                    case MOTOR_LEFT:
                        motors(wifi_msg.param[2], wifi_msg.param[3], 0, 0);
                        break;
                    
                    case MOTOR_RIGHT:
                        motors(0, 0, wifi_msg.param[2], wifi_msg.param[3]);
                        break;

                    default: // Both motors
                        motors(wifi_msg.param[2], wifi_msg.param[3], wifi_msg.param[2], wifi_msg.param[3]);
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
    // When exiting Wi-Fi control mode, ensure motors are stopped and LEDs are turned off or set to a default state
    //motors(0, 0, 0, 0);
    //robot_LEDs(LED_BOTH, COLOR_OFF);
}
