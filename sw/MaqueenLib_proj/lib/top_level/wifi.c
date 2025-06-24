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
RxReturn recepcion;
RecepcionWifi modWifi;

void wifi_control(void) {
    uint8_t Error_recibido = 0;
    char ip_display_buffer[32]; // Buffer to hold IP address string for LCD

    // 1. Initial setup for Wi-Fi mode
    motors(0, 0, 0, 0); // Stop motors immediately upon entering Wi-Fi mode
    robot_LEDs(COLOR_RED, COLOR_BLUE); // Set LEDs to a default color (e.g., blue)
    clear_LCD(); // Clear the LCD screen
    update_LCD("Wi-Fi Control"); // Display initial mode message on LCD
    delay_ms(500); // Small delay for display to be visible

    // 2. Initialize ESP-01 module (as server or client, based on AT.h configuration)
    Error_recibido = init_servidor_esp(MODO_STA);
    if (Error_recibido) {
        clear_LCD();
        update_LCD("Wifi Init Error");
    }

    // 3. Get and display the robot's IP address
    getIP(13, conexion.IP); // This function (from AT.c) retrieves the IP and stores it in conexion.IP
    clear_LCD();
    // Format the IP address string for display on the LCD
    snprintf(ip_display_buffer, sizeof(ip_display_buffer), "Listening on:   %s", conexion.IP);
    update_LCD(ip_display_buffer);
    delay_ms(1000); // Display IP for 1 second

    // 4. Main loop for Wi-Fi communication and command processing
    // Stay in this loop as long as the current navigation mode is set to Wi-Fi control (mode 3)
    // Attempt to receive data from the Wi-Fi module
    recepcion = recibir_wifi(); // This function (from AT.c) calls RxAT to get data

    if (recepcion.num_bytes > 0) { // Check if any data was successfully received
        // Decode the received raw Wi-Fi packet into the structured modWifi format
        decodificar_trama(&recepcion, &modWifi); // Populates modWifi from recepcion.StatusPacket

        // Periodically check the Wi-Fi client connection state
        Error_recibido = getConState(); // This function (from AT.c) updates conexion.conectado
        if (Error_recibido) {
            clear_LCD();
            update_LCD("Conn. State Err"); // Display an error if connection state check fails
        } else if (!conexion.conectado) { // If no client is currently connected
            motors(0, 0, 0, 0); // Stop motors as no active control
            clear_LCD();
            update_LCD("No client"); // Inform user on LCD
        } else {
            // If a client is connected, display confirmation or clear previous "No client" message
            clear_LCD();
            update_LCD("Connected");
        }

        // Process the received instruction packet based on the command type
        switch (modWifi.instruction) { // modWifi.instruction holds the main command code (e.g., LED_RGB, MOTORES)
            case LED_RGB: // Command to control RGB LEDs (0x19, defined in uart.h)
                // modWifi.id specifies which LED(s) to control (LED_LEFT, LED_RIGHT, LED_BOTH)
                // modWifi.parametre[1] specifies the desired color (COLOR_RED, COLOR_GREEN, etc.)
                /*if (modWifi.id == LED_BOTH) {
                    robot_LEDs(LED_BOTH, modWifi.parametre[1]);
                } else if (modWifi.id == LED_LEFT) {
                    robot_LEDs(LED_LEFT, modWifi.parametre[1]);
                } else if (modWifi.id == LED_RIGHT) {
                    robot_LEDs(LED_RIGHT, modWifi.parametre[1]);
                }
                break;
                */
            /*
            case MOTORES: // Command to control motors (0x20, defined in uart.h)
                // modWifi.id specifies which motor(s) (LED_LEFT for left, LED_RIGHT for right, LED_BOTH for both)
                // modWifi.parametre[1] specifies the speed
                // modWifi.parametre[2] specifies the direction (stopmotor, forwardmotor, backwardmotor)
                if (modWifi.parametre[2] == stopmotor) {
                    motors(0, 0, 0, 0); // Stop all motors if 'stopmotor' command received
                } else if (modWifi.parametre[2] == forwardmotor) {
                    if (modWifi.id == LED_BOTH) { // Control both motors forward
                        //motors(modWifi.parametre[1], modWifi.parametre[1], FORWARD, FORWARD);
                    } else if (modWifi.id == LED_LEFT) { // Control left motor forward only
                        //motors(modWifi.parametre[1], 0, FORWARD, 0); // Right motor speed 0, direction irrelevant
                    } else if (modWifi.id == LED_RIGHT) { // Control right motor forward only
                        //motors(0, modWifi.parametre[1], 0, FORWARD); // Left motor speed 0, direction irrelevant
                    }
                } else if (modWifi.parametre[2] == backwardmotor) {
                    if (modWifi.id == LED_BOTH) { // Control both motors backward
                        //motors(modWifi.parametre[1], modWifi.parametre[1], BACKWARD, BACKWARD);
                    } else if (modWifi.id == LED_LEFT) { // Control left motor backward only
                        //motors(modWifi.parametre[1], 0, BACKWARD, 0);
                    } else if (modWifi.id == LED_RIGHT) { // Control right motor backward only
                        //motors(0, modWifi.parametre[1], 0, BACKWARD);
                    }
                }
                break;
            */
            default:
                // If an unknown instruction is received, you can log it, ignore it,
                // or implement a default error response. For this scope, we do nothing.
                break;
        }
    }
    // When exiting Wi-Fi control mode, ensure motors are stopped and LEDs are turned off or set to a default state
    motors(0, 0, 0, 0);
    robot_LEDs(LED_BOTH, COLOR_OFF);
}
