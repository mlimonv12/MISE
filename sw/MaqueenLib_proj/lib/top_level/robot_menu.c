#include <stdint.h>
#include <string.h> // For strlen, memset
#include <stdio.h>  // For snprintf

#include "robot_menu.h"
#include "lcd_control.h"    // For LCD display functions
#include "robot_control.h"  // For robot_LEDs function
#include "sensor_reading.h"
#include "../low_level/timers.h" // For delay_ms
#include "../low_level/gpio.h"    // For joystick button pressed flags

// Menu state global variables (defined here, matches extern in .h)
uint8_t currentMenu = MAIN_MENU;
uint8_t menuIndex = 0;
uint8_t topVisibleIndex = 0;

// Robot state variables (defined here, matches extern in .h)
uint8_t robotRunning = 0;
uint8_t ledsOn = 0;

// Mode setting (defined here, matches extern in .h)
const char* mode_names[NUM_MODES] = {
    "Follow Light", "Escape Light", "Line Track", "Wi-Fi Control", "Joystick Control"
};
uint8_t currentNavigationMode = 0;
uint8_t navigationMode = 0;

// Speed settings (defined here, matches extern in .h)
const char* speed_names[NUM_SPEEDS] = {"Slow", "Medium", "Fast", "Kuchauuu"};
const uint8_t speed_values[NUM_SPEEDS] = {70, 170, 255, 10}; // Example values
uint8_t currentSpeedIndex = 0;
uint8_t currentSpeed = 70;

uint8_t currentRightLedColor = 0;
uint8_t currentLeftLedColor = 0;
uint8_t settingRightLed = 0;

// LDR calibration readings
uint16_t max_light [2];
uint16_t min_light [2];

// Menu definitions (defined here, matches extern in .h)
const char* main_menu_items[] = {
    "Start / Stop",
    "LEDS: ON/OFF",
    "Mode",
    "Settings"
};
#define MAIN_MENU_LENGTH (sizeof(main_menu_items) / sizeof(main_menu_items[0]))

const char* mode_menu_items[] = {
    "Follow Light",
    "Escape Light",
    "Line Follow",
    "Wi-Fi Control",
    "Joystick Control"
};
#define MODE_MENU_LENGTH (sizeof(mode_menu_items) / sizeof(mode_menu_items[0]))

const char* settings_menu_items[] = {
    "Speed",
    "LED colors",
    "Calibrate LDRs"
};
#define SETTINGS_MENU_LENGTH (sizeof(settings_menu_items) / sizeof(settings_menu_items[0]))

const char* speed_menu_items[] = {
    "Slow",
    "Medium",
    "Fast",
    "Kuchauuu"
};
#define SPEED_MENU_LENGTH (sizeof(speed_menu_items) / sizeof(speed_menu_items[0]))

const char* led_colors_menu_items[] = {
    "Right LED",
    "Left LED"
};
#define LED_COLORS_MENU_LENGTH (sizeof(led_colors_menu_items) / sizeof(led_colors_menu_items[0]))

const char* calibrate_ldrs_menu_items[] = {
    "Min light",
    "Max light"
};
#define LDR_MENU_LENGTH 2

// LED color settings (defined here, matches extern in .h)
const char* color_Names[] = {
     "Off", "Red", "Green", "Yellow", "Blue", "Purple", "Cyan", "White"
};
#define NUM_SINGLE_COLORS (sizeof(color_Names) / sizeof(color_Names[0]))

// Current menu pointers (defined here, matches extern in .h)
const char** current_menu = main_menu_items;
uint8_t current_menu_length = MAIN_MENU_LENGTH;

/**
 * @brief Initializes the menu system, setting the initial menu and displaying it.
 */
void init_menu(void) {
    currentMenu = MAIN_MENU;
    menuIndex = 0;
    topVisibleIndex = 0;
    current_menu = main_menu_items;
    current_menu_length = MAIN_MENU_LENGTH;
    navigationMode = 0;
    update_menu_display();
}

/**
 * @brief Updates the LCD display with the current menu items.
 * Shows two visible items and a '>' cursor for the selected item.
 * This version constructs a single string for update_LCD.
 */
void update_menu_display(void) {
    char display_buffer[33]; // 16 chars for line 1 + 16 chars for line 2 + null terminator
    memset(display_buffer, ' ', sizeof(display_buffer) - 1); // Fill with spaces
    display_buffer[32] = '\0'; // Null-terminate the entire buffer

    int current_pos = 0; // Track current position in display_buffer

    // --- Prepare Line 1 ---
    // Add cursor for the first visible item
    if (menuIndex == topVisibleIndex) {
        display_buffer[current_pos++] = '>';
    } else {
        display_buffer[current_pos++] = ' ';
    }

    // Copy the first visible menu item text
    const char* item1_text = current_menu[topVisibleIndex];
    int item1_len = strlen(item1_text);
    int copy_len1 = (item1_len > 15) ? 15 : item1_len; // Max 15 chars after '>' or ' '
    strncpy(&display_buffer[current_pos], item1_text, copy_len1);
    current_pos += copy_len1;

    // Pad with spaces to fill up to 16 characters for the first line
    while (current_pos < 16) {
        display_buffer[current_pos++] = ' ';
    }

    // --- Prepare Line 2 ---
    // Check if there's a second visible item
    if (topVisibleIndex + 1 < current_menu_length) {
        // Add cursor for the second visible item
        if (menuIndex == topVisibleIndex + 1) {
            display_buffer[current_pos++] = '>';
        } else {
            display_buffer[current_pos++] = ' ';
        }

        // Copy the second visible menu item text
        const char* item2_text = current_menu[topVisibleIndex + 1];
        int item2_len = strlen(item2_text);
        int copy_len2 = (item2_len > 15) ? 15 : item2_len; // Max 15 chars after '>' or ' '
        strncpy(&display_buffer[current_pos], item2_text, copy_len2);
        current_pos += copy_len2;

        // Pad with spaces to fill up to 32 characters (end of line 2)
        while (current_pos < 32) {
            display_buffer[current_pos++] = ' ';
        }
    } else {
        // If there's no second item, fill the second line with 16 spaces
        while (current_pos < 32) {
            display_buffer[current_pos++] = ' ';
        }
    }

    display_buffer[32] = '\0'; // Ensure the combined string is null-terminated

    // Now, call the existing update_LCD function with the combined string
    update_LCD(display_buffer);
}

/**
 * @brief Updates the LCD display to show the robot's running state.
 */
void update_robot_state_display(void) {
    char temp_buffer[33]; // Buffer for combined lines (16 + 16 + null)
    // snprintf copies "Robot is" to line 1, and "RUNNING!" or "STOPPED." to line 2, padded
    snprintf(temp_buffer, sizeof(temp_buffer), "Robot is        %s", robotRunning ? "RUNNING!" : "STOPPED.");
    update_LCD(temp_buffer);
    delay_ms(1000);
}

/**
 * @brief Updates the LCD display to show the currently selected navigation mode.
 */
void update_mode_display(void) {
    char temp_buffer[33];
    snprintf(temp_buffer, sizeof(temp_buffer), "Mode set to     %s", mode_names[currentNavigationMode]);
    update_LCD(temp_buffer);
    delay_ms(1000);
}

/**
 * @brief Updates the LCD display to show the currently selected speed.
 */
void update_speed_display(void) {
    char temp_buffer[33];
    snprintf(temp_buffer, sizeof(temp_buffer), "Speed set to    %s", speed_names[currentSpeedIndex]);
    update_LCD(temp_buffer);
    delay_ms(1000);
}

/**
 * @brief Updates the LCD display to show the currently selected LED color for a single LED.
 * @param isRightLed Flag to indicate if it's the Right (1) or Left (0) LED.
 */
void update_single_led_display(uint8_t isRightLed) {
    char temp_buffer[33]; // 16 chars for line 1 + 16 chars for line 2 + null terminator
    memset(temp_buffer, ' ', sizeof(temp_buffer) - 1); // Fill with spaces initially
    temp_buffer[32] = '\0'; // Null-terminate the entire buffer

    const char* line1_text = isRightLed ? "Right LED:" : "Left LED:";
    const char* line2_text = color_Names[isRightLed ? currentRightLedColor : currentLeftLedColor];

    int line1_len = strlen(line1_text);
    int line2_len = strlen(line2_text);

    // Copy first line text to the beginning of temp_buffer (max 16 chars)
    strncpy(temp_buffer, line1_text, 16);
    // Note: strncpy does NOT null-terminate if source is longer than n.
    // However, since we memset with spaces and then copy to a fixed size,
    // any remaining space on the first 16 chars will be spaces.

    // Copy second line text starting at buffer index 16 (for the second LCD line)
    // and ensure it doesn't write past 16 characters for the second line.
    strncpy(&temp_buffer[16], line2_text, 16);

    // No need for explicit padding here, as memset already filled with spaces,
    // and strncpy will not write beyond the specified length, leaving trailing spaces.
    // Also, the update_LCD function only processes the first 16 chars for the first line
    // and the next 16 for the second line.

    update_LCD(temp_buffer);
    delay_ms(1000);
}

/**
 * @brief Displays a calibration message and the current LDR readings.
 * @param isMaxLight Flag indicating if it was a max light (1) or min light (0) calibration.
 */
void update_ldr_calibration_display(uint8_t isMaxLight) {
    char temp_buffer[33];
    memset(temp_buffer, ' ', sizeof(temp_buffer) - 1);
    temp_buffer[32] = '\0';

    if (isMaxLight) {
        // Display "Maximum light values:" on the first line
        strncpy(temp_buffer, "Max light values", 16);
        // Display LDR values on the second line
        sprintf(&temp_buffer[16], "%d        %d", (uint16_t)max_light[0], (uint16_t)max_light[1]);
    } else {
        // Display "Minimum light values:" on the first line
        strncpy(temp_buffer, "Min light values", 16);
        // Display LDR values on the second line
        sprintf(&temp_buffer[16], "%d        %d", (uint16_t)min_light[0], (uint16_t)min_light[1]);
    }
    
    __no_operation();
    update_LCD(temp_buffer);
    delay_ms(1500); // Show message for a bit longer
}

/**
 * @brief Sets the robot's operating speed.
 * @param speed_id The index of the selected speed.
 */
void set_robot_speed(uint8_t speed_id) {
    currentSpeedIndex = speed_id;
    currentSpeed = speed_values[speed_id];
}


void calibrate_LDR(uint8_t isMaxLight)
{
    read_LDRs(ldr_vals);
    if (isMaxLight)
    {
        max_light[0] = ldr_vals[0];
        max_light[1] = ldr_vals[1];
    } else
    {
        min_light[0] = ldr_vals[0];
        min_light[1] = ldr_vals[1];
    }
}

/**
 * @brief Sets the robot's LED color for a specific LED.
 * @param isRightLed Flag to indicate if it's the Right (1) or Left (0) LED.
 * @param color_id The index of the selected color.
 */
void set_single_led_color(uint8_t isRightLed, uint8_t color_id) {
    if (isRightLed) {
        currentRightLedColor = color_id;
    } else {
        currentLeftLedColor = color_id;
    }
}

/**
 * @brief Toggles the overall LED state (ON/OFF) for the robot.
 */
void toggle_all_leds(void) {
    ledsOn = !ledsOn;

    if (ledsOn) {
        robot_LEDs(currentLeftLedColor, currentRightLedColor);
    } else {
        robot_LEDs(0, 0);
    }

    char temp_buffer[33];
    snprintf(temp_buffer, sizeof(temp_buffer), "All LEDs are:   %s", ledsOn ? "ON" : "OFF");
    update_LCD(temp_buffer);
    delay_ms(1000);
}

/**
 * @brief Handles going back in the menu hierarchy.
 */
void go_back_in_menu(void) {
    switch(currentMenu) {
        case MODE_MENU:
        case SETTINGS_MENU:
            currentMenu = MAIN_MENU;
            current_menu = main_menu_items;
            current_menu_length = MAIN_MENU_LENGTH;
            break;
        case SPEED_MENU:
        case LED_COLORS_MENU:
            currentMenu = SETTINGS_MENU;
            current_menu = settings_menu_items;
            current_menu_length = SETTINGS_MENU_LENGTH;
            break;
        case SINGLE_LED_COLOR_MENU:
            currentMenu = LED_COLORS_MENU;
            current_menu = led_colors_menu_items;
            current_menu_length = LED_COLORS_MENU_LENGTH;
            break;
        case CALIBRATE_LDR_MENU:
            currentMenu = SETTINGS_MENU;
            current_menu = settings_menu_items;
            current_menu_length = SETTINGS_MENU_LENGTH;
            break;
        case MAIN_MENU:
            // No action if already in main menu
            return;
    }
    menuIndex = 0;
    topVisibleIndex = 0;
    update_menu_display();
}

/**
 * @brief Executes the action associated with the selected menu item.
 * Handles menu navigation (switching between menus) and specific robot actions.
 * @param index The index of the selected item in the current menu.
 */
void execute_menu_action(uint8_t index) {
    // update_LCD will handle clearing the screen by writing from (0,0)

    switch(currentMenu) {
        case MAIN_MENU:
            switch(index) {
                case 0: // "Start / Stop"
                    robotRunning = !robotRunning;
                    navigationMode = robotRunning;
                    update_robot_state_display();
                    if (robotRunning) {
                        update_LCD("Navigation Mode"); // This message will be on line 1
                    }
                    return;
                case 1: // "LEDS: ON/OFF"
                    toggle_all_leds();
                    break;
                case 2: // "Mode"
                    currentMenu = MODE_MENU;
                    current_menu = mode_menu_items;
                    current_menu_length = MODE_MENU_LENGTH;
                    menuIndex = currentNavigationMode;
                    topVisibleIndex = (menuIndex > 0) ? menuIndex - 1 : 0;
                    update_menu_display();
                    return;
                case 3: // "Settings"
                    currentMenu = SETTINGS_MENU;
                    current_menu = settings_menu_items;
                    current_menu_length = SETTINGS_MENU_LENGTH;
                    menuIndex = 0;
                    topVisibleIndex = 0;
                    update_menu_display();
                    return;
            }
            break;

        case SETTINGS_MENU:
            switch(index) {
                case 0: // "Speed"
                    currentMenu = SPEED_MENU;
                    current_menu = speed_menu_items;
                    current_menu_length = SPEED_MENU_LENGTH;
                    menuIndex = currentSpeedIndex;
                    topVisibleIndex = (menuIndex > 0) ? menuIndex - 1 : 0;
                    update_menu_display();
                    return;
                case 1: // "LED colors"
                    currentMenu = LED_COLORS_MENU;
                    current_menu = led_colors_menu_items;
                    current_menu_length = LED_COLORS_MENU_LENGTH;
                    menuIndex = 0;
                    topVisibleIndex = 0;
                    update_menu_display();
                    return;
                case 2: // Calibrate LDRs
                    currentMenu = CALIBRATE_LDR_MENU;
                    current_menu = calibrate_ldrs_menu_items;
                    current_menu_length = LDR_MENU_LENGTH;
                    menuIndex = 0;
                    topVisibleIndex = 0;
                    update_menu_display();
            }
            break;

        case MODE_MENU:
            currentNavigationMode = index;
            update_mode_display();
            currentMenu = MAIN_MENU;
            current_menu = main_menu_items;
            current_menu_length = MAIN_MENU_LENGTH;
            menuIndex = 0;
            topVisibleIndex = 0;
            update_menu_display();
            return;

        case SPEED_MENU:
            set_robot_speed(index);
            update_speed_display();
            currentMenu = SETTINGS_MENU;
            current_menu = settings_menu_items;
            current_menu_length = SETTINGS_MENU_LENGTH;
            menuIndex = 0;
            topVisibleIndex = 0;
            update_menu_display();
            return;

        case LED_COLORS_MENU:
            switch(index) {
                case 0: // "Right LED"
                    currentMenu = SINGLE_LED_COLOR_MENU;
                    current_menu = color_Names;
                    current_menu_length = NUM_SINGLE_COLORS;
                    menuIndex = currentRightLedColor;
                    topVisibleIndex = (menuIndex > 0) ? menuIndex - 1 : 0;
                    settingRightLed = 1;
                    update_menu_display();
                    return;
                case 1: // "Left LED"
                    currentMenu = SINGLE_LED_COLOR_MENU;
                    current_menu = color_Names;
                    current_menu_length = NUM_SINGLE_COLORS;
                    menuIndex = currentLeftLedColor;
                    topVisibleIndex = (menuIndex > 0) ? menuIndex - 1 : 0;
                    settingRightLed = 0;
                    update_menu_display();
                    return;
            }
            break;

        case SINGLE_LED_COLOR_MENU:
            set_single_led_color(settingRightLed, index);
            if (ledsOn) {robot_LEDs(currentLeftLedColor, currentRightLedColor);} // WHY AREN'T THE COLORS UPDATED AUTOMATICALLY?
            update_single_led_display(settingRightLed);
            currentMenu = LED_COLORS_MENU;
            current_menu = led_colors_menu_items;
            current_menu_length = LED_COLORS_MENU_LENGTH;
            menuIndex = (settingRightLed == 1) ? 0 : 1;
            topVisibleIndex = 0;
            update_menu_display();
            return;

        case CALIBRATE_LDR_MENU:
            calibrate_LDR(index == 1); // MAX
            update_ldr_calibration_display(index == 1);
            currentMenu = CALIBRATE_LDR_MENU;
            current_menu = calibrate_ldrs_menu_items;
            current_menu_length = LDR_MENU_LENGTH;
            menuIndex = 0;
            topVisibleIndex = 0;
            update_menu_display();
            return;
    }

    delay_ms(1000);
    update_menu_display();
}

/**
 * @brief Handles menu navigation based on joystick button presses.
 */
void handle_menu(void) {
    if (navigationMode && robotRunning) {
        if (joystick_left_pressed) {
            joystick_left_pressed = 0;
            robotRunning = 0;
            navigationMode = 0;
            init_menu();
            delay_ms(200);
            return;
        }
        return;
    }

    if (joystick_up_pressed) {
        joystick_up_pressed = 0;
        if (menuIndex > 0) {
            menuIndex--;
            if (menuIndex < topVisibleIndex) {
                topVisibleIndex = menuIndex;
            }
            update_menu_display();
        }
        delay_ms(200);
    }
    else if (joystick_down_pressed) {
        joystick_down_pressed = 0;
        if (menuIndex < current_menu_length - 1) {
            menuIndex++;
            if (menuIndex > topVisibleIndex + 1) {
                topVisibleIndex++;
            }
            update_menu_display();
        }
        delay_ms(200);
    }
    else if (joystick_right_pressed) {
        joystick_right_pressed = 0;
        execute_menu_action(menuIndex);
        delay_ms(200);
    }
    else if (joystick_left_pressed) {
        joystick_left_pressed = 0;
        go_back_in_menu();
        delay_ms(200);
    }
}
