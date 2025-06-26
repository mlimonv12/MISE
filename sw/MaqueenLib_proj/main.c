#include <msp430.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Low-Level Library Includes
#include "./lib/low_level/clocks.h"
#include "./lib/low_level/timers.h"
#include "./lib/low_level/gpio.h"
#include "./lib/low_level/i2c.h"
#include "./lib/low_level/adc.h"
#include "./lib/low_level/AT.h"
#include "./lib/low_level/uart.h"
#include "./lib/low_level/recursos.h"

// Top-Level Library Includes
#include "./lib/top_level/lcd_control.h"
#include "./lib/top_level/robot_control.h"
#include "./lib/top_level/sensor_reading.h"
#include "./lib/top_level/robot_menu.h"
#include "./lib/top_level/wifi.h"

// Variables


/**
 * main.c - Main application entry point
 */
main(void) {
    // Stop watchdog timer to prevent unexpected resets
    WDTCTL = WDTPW | WDTHOLD;

    // Initialize all necessary hardware components using low-level libraries
    init_clocks();      // Configure system clocks
    init_timers();      // Configure timers for delays
    init_GPIOs();       // Configure General Purpose I/Os (including joystick digital inputs)
    init_adc();         // Initialize ADC module
    init_i2c();         // Configure I2C communication
    init_uart_wifi();   // Configure UART for Wi-Fi module communication

    // Initial robot and display setup
    robot_LEDs(5, 5);   // Set initial LED colors
    delay_ms(500);      // Wait for 500ms
    robot_LEDs(6, 6);   // Change LED colors
    delay_ms(500);      // Wait for 500ms
    robot_LEDs(0, 0);   // Change LED colors

    init_LCD();         // Initialize the LCD display
    init_menu();        // Initialize the menu system (displays main menu)

    // Main application loop
    while(1){
        // Handle menu interactions (joystick presses)
        menu_loop();

        if (active) {
            switch (mode)
            {
                case 0: // Follow light
                    follow_light(speed, max_light, min_light);
                    break;
                
                case 1: // Escape light
                    escape_light(speed, max_light, min_light);
                    break;

                case 2: // Line track
                    linetrack(speed);
                    break;

                case 3: // Wi-fi control
                    if (!wifi_started) {
                        wifi_started = 1;
                        wifi_init();
                    }
                    wifi_control();
                    break;

                case 4: // Manual Mode
                    control_joystick(speed);
                    break;

                default:
                    motors(0,0,0,0); // Stop motors if out of navigation mode
                    break;
            }
        }
        else
        {
            wifi_started = 0;
            motors(0,0,0,0); // Stop motors if out of navigation mode
        }
        
    }
}

