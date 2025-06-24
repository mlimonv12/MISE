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
//uint8_t i = 0;

/**
 * main.c - Main application entry point
 */
main(void) {
    // Stop watchdog timer to prevent unexpected resets
    WDTCTL = WDTPW | WDTHOLD;

    // Initialize all necessary hardware components using low-level libraries
    init_clocks();      // Configure system clocks
    init_timers();      // Configure timers for delays
    init_i2c();         // Configure I2C communication
    init_GPIOs();       // Configure General Purpose I/Os (including joystick digital inputs)
    init_adc();         // NEW: Initialize ADC module (was in GPIO, moved for better separation)
    init_uart_wifi();   // Configure UART for Wi-Fi module communication

    // Initial robot and display setup
    robot_LEDs(5, 5);   // Set initial LED colors
    delay_ms(500);      // Wait for 500ms
    robot_LEDs(6, 6);   // Change LED colors
    delay_ms(500);      // Wait for 500ms
    robot_LEDs(0, 0);   // Change LED colors

    // Attempt to communicate with AT module (e.g., Wi-Fi module)
    volatile uint8_t at = comando_AT(); // Stores the result of AT command, though not used further

    init_LCD();         // Initialize the LCD display
    init_menu();        // NEW: Initialize the menu system (displays main menu)

    // Variables for robot control and sensor readings
    // stat_prev: Stores previous motor directions and speeds (left_dir, left_speed, right_dir, right_speed)
    uint8_t stat_prev[4] = {1, 50, 1, 50}; // Initial motor state (forward at speed 50)
    // stat_next: Stores calculated next motor directions and speeds
    uint8_t stat_next[4];
    uint8_t leds_state = 0; // Stores the state returned by calculate_motors (e.g., STRAIGHT, TURN_L)

    uint16_t LDR_reading[2]; // Array to store LDR sensor values

    // Main application loop
    while(1){
        // Handle menu interactions (joystick presses)
        handle_menu();
        delay_ms(100); // Small delay for overall loop cycle
        __no_operation(); // Placeholder for breakpoint during debugging

        // If in "Start" (navigation) mode, execute line-following logic
        // The `navigationMode` variable is handled by the menu system.
        if (navigationMode) {

            switch (currentNavigationMode)
            {
            case 0: // Follow light
                follow_light(currentSpeed, max_light, min_light);
                break;
            
            case 1: // Escape light
                escape_light(currentSpeed, max_light, min_light);
                break;

            case 2: // Line track
                linetrack(currentSpeed);
                break;

            case 3: // Wi-fi control
                wifi_control();
                break;

            case 4: // Joystick control
                break;

            default:
                motors(0,0,0,0); // Stop motors if out of navigation mode
                break;
            }
        }
        else
        {
            motors(0,0,0,0); // Stop motors if out of navigation mode
        }
        
    }
}

