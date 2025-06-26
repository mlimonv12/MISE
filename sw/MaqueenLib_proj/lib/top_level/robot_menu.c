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
uint8_t topIndex = 0;

// Robot state variables (defined here, matches extern in .h)
uint8_t robotRunning = 0;
uint8_t ledsOn = 0;
uint8_t wifi_started = 0;

char ssid_sta [] = "BRCO";
char pwd_sta [] = "SJT7b&$Te8BQFfvq5b";

// Mode setting (defined here, matches extern in .h)
uint8_t currentNavigationMode = 0;
uint8_t navigationMode = 0; // This flag indicates if the robot is in an active navigation state

// Speed settings (defined here, matches extern in .h)
const char* speed_names[NUM_SPEEDS] = {"Slow", "Medium", "Fast", "Kuchauuu"};
const uint8_t speed_values[NUM_SPEEDS] = {70, 170, 255, 10}; // Example values
uint8_t currentSpeedIndex = 0;
uint8_t currentSpeed = 70;

// LEDs
uint8_t currentRightLedColor = 0;
uint8_t currentLeftLedColor = 0;
uint8_t settingRightLed = 0;

// LDR calibration readings
uint16_t max_light [2] = {3000, 3000};
uint16_t min_light [2] = {800, 800};
uint16_t ldr_vals  [2];

// Wi-Fi
const char* ssid_list [] = {
    "BRCO",
    "MiFibra-BFE6",
    "N12L"
};
const char* pwd_list [] = {"SJT7b&$Te8BQFfvq5b", "9pDHx4nV", "d1g1t4l!"};
#define WIFI_MENU_LENGTH (sizeof(ssid_list) / sizeof(ssid_list[0]))

// Time tracking for LDR display (adapted for delay_ms)
static uint16_t ldr_display_counter = 0;
#define LDR_DISPLAY_INTERVAL_MS 100 // 1 second
#define MAIN_LOOP_DELAY_MS 10       // Delay for main loop iteration
#define LDR_DISPLAY_PERIODS (LDR_DISPLAY_INTERVAL_MS / MAIN_LOOP_DELAY_MS) // 100 periods of 10ms

// Menu definitions (defined here, matches extern in .h)
const char* main_menu_items[] = {
    "Start           ",
    "Toggle LEDs     ",
    "Mode select     ",
    "Settings        ",
    "About           "
};
#define MAIN_MENU_LENGTH (sizeof(main_menu_items) / sizeof(main_menu_items[0]))

const char*  mode_names[] = {
    "Follow Light    ",
    "Escape Light    ",
    "Line Follow     ",
    "Wi-Fi Control   "
};
#define MODE_MENU_LENGTH 4

const char* settings_menu_items[] = {
    "Speed          ",
    "LED colors     ",
    "Calibrate LDRs ",
    "Choose network "
};
#define SETTINGS_MENU_LENGTH (sizeof(settings_menu_items) / sizeof(settings_menu_items[0]))

const char* speed_menu_items[] = {
    "Slow           ",
    "Medium         ",
    "Fast           ",
    "Kuchauuu       "
};
#define SPEED_MENU_LENGTH (sizeof(speed_menu_items) / sizeof(speed_menu_items[0]))

const char* led_colors_menu_items[] = {
    "Right LED      ",
    "Left LED       "
};
#define LED_COLORS_MENU_LENGTH (sizeof(led_colors_menu_items) / sizeof(led_colors_menu_items[0]))

const char* calibrate_ldrs_menu_items[] = {
    "Min light      ",
    "Max light      "
};
#define LDR_MENU_LENGTH 2

const char* about_items[] = {
    "Authors        ",
    "Specs          "
};
#define ABOUT_MENU_LENGTH 2

// LED color settings (defined here, matches extern in .h)
const char* color_Names[] = {
     "Off           ",
     "Red           ",
     "Green         ",
     "Yellow        ",
     "Blue          ",
     "Purple        ",
     "Cyan          ",
     "White         "
};
#define NUM_SINGLE_COLORS (sizeof(color_Names) / sizeof(color_Names[0]))

// Current menu pointers (defined here, matches extern in .h)
const char** menuContent = main_menu_items;
uint8_t current_menu_length = MAIN_MENU_LENGTH;

/**
 * @brief Initializes the menu system, setting the initial menu and displaying it.
 */
void init_menu(void) {
    currentMenu = MAIN_MENU;
    menuIndex = 0;
    topIndex = 0;
    menuContent = main_menu_items;
    current_menu_length = MAIN_MENU_LENGTH;
    navigationMode = 0; // Ensure navigation mode is off when initializing menu
    ldr_display_counter = 0; // Reset counter
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
    if (menuIndex == topIndex) {
        display_buffer[current_pos++] = '>';
    } else {
        display_buffer[current_pos++] = ' ';
    }

    // Copy the first visible menu item text
    const char* item1_text = menuContent[topIndex];
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
    if (topIndex + 1 < current_menu_length) {
        // Add cursor for the second visible item
        if (menuIndex == topIndex + 1) {
            display_buffer[current_pos++] = '>';
        } else {
            display_buffer[current_pos++] = ' ';
        }

        // Copy the second visible menu item text
        const char* item2_text = menuContent[topIndex + 1];
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
 * @brief Displays the current navigation info (mode, and LDR values if applicable).
 */
void display_navigation_info(void) {
    char temp_buffer[33];
    memset(temp_buffer, ' ', sizeof(temp_buffer) - 1);
    temp_buffer[32] = '\0';

    // Line 1: Show current navigation mode
    strncpy(temp_buffer, mode_names[currentNavigationMode], 16);

    // Line 2: Show LDR values for specific modes
    if (currentNavigationMode == 0 || currentNavigationMode == 1) { // Follow Light or Escape Light
        read_LDRs(ldr_vals); // Read current LDR values
        sprintf(&temp_buffer[16], "%d        %d", (uint16_t)ldr_vals[1], (uint16_t)ldr_vals[0]);
    } else {
        // For other modes, clear the second line or display a generic message
        memset(&temp_buffer[16], ' ', 16); // Fill with spaces
    }

    if (currentNavigationMode != 3) // Don't want updates on Wifi mode
        update_LCD(temp_buffer);
}

/**
 * @brief Updates the LCD display with the current robot's running state.
 */
void update_robot_state_display(void) {
    char temp_buffer[33]; // Buffer for combined lines (16 + 16 + null)
    // snprintf copies "Robot is" to line 1, and "RUNNING!" or "STOPPED." to line 2, padded
    snprintf(temp_buffer, sizeof(temp_buffer), "Robot is        %s", robotRunning ? "RUNNING!" : "STOPPED.");
    update_LCD(temp_buffer);
    delay_ms(1000);
}

/**
 * @brief Updates the LCD display with the currently selected navigation mode.
 */
void update_mode_display(void) {
    char temp_buffer[33];
    snprintf(temp_buffer, sizeof(temp_buffer), "Mode set to     %s", mode_names[currentNavigationMode]);
    update_LCD(temp_buffer);
    delay_ms(1000);
}

/**
 * @brief Updates the LCD display with the currently selected speed.
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

    strncpy(temp_buffer, line1_text, 16);
    strncpy(&temp_buffer[16], line2_text, 16);

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

    update_LCD(temp_buffer);
    delay_ms(1500); // Show message for a bit longer
}


void update_about_display(uint8_t index) {
    char temp_buffer[33];
    memset(temp_buffer, ' ', sizeof(temp_buffer) - 1);
    temp_buffer[32] = '\0';

    if (!index)
        sprintf(temp_buffer, "Adria Bru       Miquel Limon");
    else
        sprintf(temp_buffer, "Maqueen kuchau  MSP430FR255");
    
    update_LCD(temp_buffer);
    delay_ms(3000); // Show message for a bit longer
}

void update_network_display(void) {
    char temp_buffer[33];
    memset(temp_buffer, ' ', sizeof(temp_buffer) - 1);
    temp_buffer[32] = '\0';

    sprintf(temp_buffer, "Will connect to:%s", ssid_sta);
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

/**
 * @brief Calibrates the LDRs by reading current values and storing them as min or max.
 * @param isMaxLight Flag: 1 for max light calibration, 0 for min light calibration.
 */
void calibrate_LDR(uint8_t isMaxLight)
{
    read_LDRs(ldr_vals); // Assumes ldr_vals is a global or accessible array
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
        robot_LEDs(0, 0); // Turn off all LEDs
    }

    char temp_buffer[33];
    snprintf(temp_buffer, sizeof(temp_buffer), "All LEDs are:   %s", ledsOn ? "ON" : "OFF");
    update_LCD(temp_buffer);
    delay_ms(1000);
}

void select_network(uint8_t index) {
    strcpy(ssid_sta, ssid_list[index]);
    strcpy(pwd_sta, pwd_list[index]);
}

/**
 * @brief Handles going back in the menu hierarchy.
 */
void go_back_in_menu(void) {
    switch(currentMenu) {
        case MODE_MENU:
        case SETTINGS_MENU:
            currentMenu = MAIN_MENU;
            menuContent = main_menu_items;
            current_menu_length = MAIN_MENU_LENGTH;
            break;
        case SPEED_MENU:
        case LED_COLORS_MENU:
            currentMenu = SETTINGS_MENU;
            menuContent = settings_menu_items;
            current_menu_length = SETTINGS_MENU_LENGTH;
            break;
        case SINGLE_LED_COLOR_MENU:
            currentMenu = LED_COLORS_MENU;
            menuContent = led_colors_menu_items;
            current_menu_length = LED_COLORS_MENU_LENGTH;
            break;
        case CALIBRATE_LDR_MENU:
            currentMenu = SETTINGS_MENU;
            menuContent = settings_menu_items;
            current_menu_length = SETTINGS_MENU_LENGTH;
            break;
        case SELECT_NETWORK:
            wifi_started = 0;
            currentMenu = SETTINGS_MENU;
            menuContent = settings_menu_items;
            current_menu_length = SETTINGS_MENU_LENGTH;
            break;
        case ABOUT:
            currentMenu = MAIN_MENU;
            menuContent = main_menu_items;
            current_menu_length = MAIN_MENU_LENGTH;
            break;
        case MAIN_MENU:
            // No action if already in main menu
            return;
    }
    menuIndex = 0;
    topIndex = 0;
    update_menu_display();
}

/**
 * @brief Executes the action associated with the selected menu item.
 * Handles menu navigation (switching between menus) and specific robot actions.
 * @param index The index of the selected item in the current menu.
 */
void execute_menu_action(uint8_t index) {
    switch(currentMenu) {
        case MAIN_MENU:
            switch(index) {
                case 0: // "Start / Stop"
                    robotRunning = !robotRunning;
                    navigationMode = robotRunning; // Set navigationMode based on robotRunning
                    update_robot_state_display();
                    if (robotRunning) {
                        // When starting, show current mode immediately
                        display_navigation_info(); // Call the dedicated display function
                        ldr_display_counter = 0; // Initialize counter for LDR display
                    }
                    return; // Don't fall through to update_menu_display as we're in a special state
                case 1: // "LEDS: ON/OFF"
                    toggle_all_leds();
                    break;
                case 2: // "Mode"
                    currentMenu = MODE_MENU;
                    menuContent =  mode_names;
                    current_menu_length = MODE_MENU_LENGTH;
                    menuIndex = currentNavigationMode;
                    topIndex = (menuIndex > 0) ? menuIndex - 1 : 0;
                    update_menu_display();
                    return;
                case 3: // "Settings"
                    currentMenu = SETTINGS_MENU;
                    menuContent = settings_menu_items;
                    current_menu_length = SETTINGS_MENU_LENGTH;
                    menuIndex = 0;
                    topIndex = 0;
                    update_menu_display();
                    return;
                case 4: // "About"
                    currentMenu = ABOUT;
                    menuContent = about_items;
                    current_menu_length = ABOUT_MENU_LENGTH;
                    menuIndex = 0;
                    topIndex = 0;
                    update_menu_display();
                    return;
            }
            break;

        case SETTINGS_MENU:
            switch(index) {
                case 0: // "Speed"
                    currentMenu = SPEED_MENU;
                    menuContent = speed_menu_items;
                    current_menu_length = SPEED_MENU_LENGTH;
                    menuIndex = currentSpeedIndex;
                    topIndex = (menuIndex > 0) ? menuIndex - 1 : 0;
                    update_menu_display();
                    return;
                case 1: // "LED colors"
                    currentMenu = LED_COLORS_MENU;
                    menuContent = led_colors_menu_items;
                    current_menu_length = LED_COLORS_MENU_LENGTH;
                    menuIndex = 0;
                    topIndex = 0;
                    update_menu_display();
                    return;
                case 2: // Calibrate LDRs
                    currentMenu = CALIBRATE_LDR_MENU;
                    menuContent = calibrate_ldrs_menu_items;
                    current_menu_length = LDR_MENU_LENGTH;
                    menuIndex = 0;
                    topIndex = 0;
                    update_menu_display();
                    return;
                case 3: // Choose Wifi network
                    currentMenu = SELECT_NETWORK;
                    menuContent = ssid_list;
                    current_menu_length = WIFI_MENU_LENGTH;
                    menuIndex = 0;
                    topIndex = 0;
                    update_menu_display();
                    return;
            }
            break;

        case MODE_MENU:
            currentNavigationMode = index;
            update_mode_display();
            currentMenu = MAIN_MENU;
            menuContent = main_menu_items;
            current_menu_length = MAIN_MENU_LENGTH;
            menuIndex = 0;
            topIndex = 0;
            update_menu_display();
            return;

        case SPEED_MENU:
            set_robot_speed(index);
            update_speed_display();
            currentMenu = SETTINGS_MENU;
            menuContent = settings_menu_items;
            current_menu_length = SETTINGS_MENU_LENGTH;
            menuIndex = 0;
            topIndex = 0;
            update_menu_display();
            return;

        case LED_COLORS_MENU:
            switch(index) {
                case 0: // "Right LED"
                    currentMenu = SINGLE_LED_COLOR_MENU;
                    menuContent = color_Names;
                    current_menu_length = NUM_SINGLE_COLORS;
                    menuIndex = currentRightLedColor;
                    topIndex = (menuIndex > 0) ? menuIndex - 1 : 0;
                    settingRightLed = 1;
                    update_menu_display();
                    return;
                case 1: // "Left LED"
                    currentMenu = SINGLE_LED_COLOR_MENU;
                    menuContent = color_Names;
                    current_menu_length = NUM_SINGLE_COLORS;
                    menuIndex = currentLeftLedColor;
                    topIndex = (menuIndex > 0) ? menuIndex - 1 : 0;
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
            menuContent = led_colors_menu_items;
            current_menu_length = LED_COLORS_MENU_LENGTH;
            menuIndex = (settingRightLed == 1) ? 0 : 1;
            topIndex = 0;
            update_menu_display();
            return;

        case CALIBRATE_LDR_MENU:
            calibrate_LDR(index == 1); // MAX
            update_ldr_calibration_display(index == 1);
            currentMenu = CALIBRATE_LDR_MENU;
            menuContent = calibrate_ldrs_menu_items;
            current_menu_length = LDR_MENU_LENGTH;
            menuIndex = 0;
            topIndex = 0;
            update_menu_display();
            return;

        case SELECT_NETWORK:
            select_network(index);
            update_network_display();
            currentMenu = MAIN_MENU;
            menuContent = main_menu_items;
            current_menu_length = MAIN_MENU_LENGTH;
            menuIndex = 0;
            topIndex = 0;
            update_menu_display();
            return; // MAYBE THIS IS IT

        case ABOUT:
            update_about_display(index);
            currentMenu = ABOUT;
            menuContent = about_items;
            current_menu_length = ABOUT_MENU_LENGTH;
            menuIndex = 0;
            topIndex = 0;
            update_menu_display();
            return;
    }

    delay_ms(1000); // Only execute this if an action didn't already return
    update_menu_display();
}


/**
 * @brief Handles menu navigation based on joystick button presses.
 */
void handle_menu(void) {
    if (robotRunning) { // If robot is in a running/navigation mode
        if (joystick_left_pressed) {
            joystick_left_pressed = 0; // Clear the flag
            robotRunning = 0;          // Stop the robot
            navigationMode = 0;        // Exit navigation mode
            init_menu();               // Go back to the main menu and display it
            delay_ms(200);             // Debounce delay
            return;                    // Exit the function to prevent further menu processing
        }

        // Increment the counter for LDR display
        ldr_display_counter++;
        // Check if it's time to update the LDR display (100 * 10ms = 1000ms = 1 second)
        if (ldr_display_counter >= LDR_DISPLAY_PERIODS) {
            display_navigation_info();
            ldr_display_counter = 0; // Reset counter
        }
        
        // If robot is running, no other menu interactions (up, down, right) should occur
        // We only allow pressing LEFT to exit navigation mode.
        // Other joystick presses while robotRunning will be ignored in this function.
        delay_ms(MAIN_LOOP_DELAY_MS); 
        return; 
    }

    // If robot is NOT running, handle regular menu navigation
    if (joystick_up_pressed) {
        joystick_up_pressed = 0;
        if (menuIndex > 0) {
            menuIndex--;
            if (menuIndex < topIndex) {
                topIndex = menuIndex;
            }
            update_menu_display();
        }
        delay_ms(200);
    }
    else if (joystick_down_pressed) {
        joystick_down_pressed = 0;
        if (menuIndex < current_menu_length - 1) {
            menuIndex++;
            if (menuIndex > topIndex + 1) {
                topIndex++;
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
    else if (joystick_left_pressed) { // This is for going back in menus when not running
        joystick_left_pressed = 0;
        go_back_in_menu();
        delay_ms(200);
    }
}
