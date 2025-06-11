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
#include "./lib/top_level/robot_menu.h" // NEW: Include the menu system


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

        // If in "Start" (navigation) mode, execute line-following logic
        // The `navigationMode` variable is handled by the menu system.
        if (navigationMode) {
            // Calculate the next motor commands based on photodetector readings
            leds_state = calculate_motors(stat_prev, stat_next);
            delay_ms(1); // Small delay

            // Apply the calculated motor commands to the robot
            motors(stat_next[0], stat_next[1], stat_next[2], stat_next[3]);
            delay_ms(1); // Small delay

            clear_LCD(); // Clear the LCD display for sensor readings

            // Read Light Dependent Resistors (LDRs)
            read_LDRs(LDR_reading); // Store LDR values into LDR_reading array

            // Format and display the LDR values on the LCD
            char LDR_msg[16]; // Buffer for LDR message (e.g., "1023,512")
            sprintf(LDR_msg, "%d,%d", LDR_reading[0], LDR_reading[1]);
            display_LCD(LDR_msg); // Display on first line

            delay_ms(500); // Wait for half a second before next iteration

            // Update robot LEDs based on the calculated movement state
            switch (leds_state)
            {
            case STRAIGHT: // Robot moving straight
                robot_LEDs(2, 2); // Green-ish LEDs
                break;
            case TURN_L: // Robot turning left
                robot_LEDs(2, 3); // Green-ish left, Yellow-ish right
                break;
            case TURN_R: // Robot turning right
                robot_LEDs(3, 2); // Yellow-ish left, Green-ish right
                break;
            case STOP: // Robot stopped
                robot_LEDs(1, 1); // Red LEDs
                break;
            case LOST: // Robot lost the line
                robot_LEDs(4, 4); // Blue LEDs
                break;
            default:
                break;
            }

            // Copy the current motor state to the previous state for the next iteration
            for (uint8_t i = 0; i < 4; i++) {
                stat_prev[i] = stat_next[i];
            }
        }
        // If not in navigation mode, ensure the menu is displayed (handle_menu does this)
        // and allow other background tasks if any.

        delay_ms(100); // Small delay for overall loop cycle
        __no_operation(); // Placeholder for breakpoint during debugging
    }
}

