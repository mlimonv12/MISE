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
uint8_t visibleMenu = MAIN_MENU;
uint8_t menuIndex = 0;
uint8_t topIndex = 0;

// Robot state variables (defined here, matches extern in .h)
uint8_t inMovement = 0;
uint8_t ledsOn = 0;
uint8_t wifi_started = 0;

char ssid_sta [] = "BRCO";
char pwd_sta [] = "SJT7b&$Te8BQFfvq5b";

// Mode setting (defined here, matches extern in .h)
uint8_t mode = 0; 
uint8_t active = 0; // This flag indicates if the robot is in an active navigation state

// Speed settings (defined here, matches extern in .h)
const char* speed_names[NUM_SPEEDS] = {"Slow", "Medium", "Fast", "Kuchauuu"};
const uint8_t speed_values[NUM_SPEEDS] = {70, 170, 255, 10}; // Example values
uint8_t speed_i = 0;
uint8_t speed = 70;

// LEDs
uint8_t ledColor_right = 0;
uint8_t ledColor_left = 0;
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
    "Wi-Fi Control   ",
    "Manual Mode     " // Manual mode (wink wink)
};
#define MODE_MENU_LENGTH (sizeof(mode_names) / sizeof(mode_names[0]))

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
    visibleMenu = MAIN_MENU;
    menuIndex = 0;
    topIndex = 0;
    menuContent = main_menu_items;
    current_menu_length = MAIN_MENU_LENGTH;
    active = 0; // Ensure navigation mode is off when initializing menu
    ldr_display_counter = 0; // Reset counter
    menu_update();
}

/**
 * @brief Updates the LCD display with the current menu items.
 * Shows two visible items and a '>' cursor for the selected item.
 * This version constructs a single string for update_LCD.
 */
void menu_update(void) {
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
void navigation_info_display(void) {
    char temp_buffer[33];
    memset(temp_buffer, ' ', sizeof(temp_buffer) - 1);
    temp_buffer[32] = '\0';

    // Line 1: Show current navigation mode
    strncpy(temp_buffer, mode_names[mode], 16);

    // Line 2: Show LDR values for specific modes
    if (mode == 0 || mode == 1) { // Follow Light or Escape Light
        read_LDRs(ldr_vals); // Read current LDR values
        sprintf(&temp_buffer[16], "%d        %d", (uint16_t)ldr_vals[1], (uint16_t)ldr_vals[0]);
    } else {
        // For other modes, clear the second line or display a generic message
        memset(&temp_buffer[16], ' ', 16); // Fill with spaces
    }

    if (mode != 3) // Don't want updates on Wifi mode
        update_LCD(temp_buffer);
}

/**
 * @brief Updates the LCD display with the current robot's running state.
 */
void status_display(void) {
    char temp_buffer[33]; // Buffer for combined lines (16 + 16 + null)
    snprintf(temp_buffer, sizeof(temp_buffer), "Robot is        %s", inMovement ? "RUNNING!" : "STOPPED.");
    update_LCD(temp_buffer);
    delay_ms(1000);
}

/**
 * @brief Updates the LCD display with the currently selected navigation mode.
 */
void mode_display(void) {
    char temp_buffer[33];
    snprintf(temp_buffer, sizeof(temp_buffer), "Mode set to     %s", mode_names[mode]);
    update_LCD(temp_buffer);
    delay_ms(1000);
}

/**
 * @brief Updates the LCD display with the currently selected speed.
 */
void speed_display(void) {
    char temp_buffer[33];
    snprintf(temp_buffer, sizeof(temp_buffer), "Speed set to    %s", speed_names[speed_i]);
    update_LCD(temp_buffer);
    delay_ms(1000);
}

/**
 * @brief Updates the LCD display to show the currently selected LED color for a single LED.
 * @param isRightLed Flag to indicate if it's the Right (1) or Left (0) LED.
 */
void single_led_display(uint8_t isRightLed) {
    char temp_buffer[33]; // 16 chars for line 1 + 16 chars for line 2 + null terminator
    memset(temp_buffer, ' ', sizeof(temp_buffer) - 1); // Fill with spaces initially
    temp_buffer[32] = '\0'; // Null-terminate the entire buffer

    sprintf(temp_buffer, "%s      %s", isRightLed ? "Right LED:" : "Left LED: ", color_Names[isRightLed ? ledColor_right : ledColor_left]);

    update_LCD(temp_buffer);
    delay_ms(1000);
}

/**
 * @brief Displays a calibration message and the current LDR readings.
 * @param isMaxLight Flag indicating if it was a max light (1) or min light (0) calibration.
 */
void ldr_calibration_display(uint8_t isMaxLight) {
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


void about_display(uint8_t index) {
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

void network_display(void) {
    char temp_buffer[33];
    memset(temp_buffer, ' ', sizeof(temp_buffer) - 1);
    temp_buffer[32] = '\0';

    sprintf(temp_buffer, "Will connect to:%s", ssid_sta);
    update_LCD(temp_buffer);
    delay_ms(1500); // Show message for a bit longer
}

/**
 * @brief Sets the robot's operating speed.
 * @param speed_index The index of the selected speed.
 */
void set_robot_speed(uint8_t speed_index) {
    speed_i = speed_index;
    speed = speed_values[speed_index];
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
        ledColor_right = color_id;
    } else {
        ledColor_left = color_id;
    }
}

/**
 * @brief Toggles the overall LED state (ON/OFF) for the robot.
 */
void toggle_all_leds(void) {
    ledsOn = !ledsOn;

    if (ledsOn) {
        robot_LEDs(ledColor_left, ledColor_right);
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
void menu_back(void) {
    switch(visibleMenu) {
        case MODE_MENU:
        case SETTINGS_MENU:
            visibleMenu = MAIN_MENU;
            menuContent = main_menu_items;
            current_menu_length = MAIN_MENU_LENGTH;
            break;
        case SPEED_MENU:
        case LED_COLORS_MENU:
            visibleMenu = SETTINGS_MENU;
            menuContent = settings_menu_items;
            current_menu_length = SETTINGS_MENU_LENGTH;
            break;
        case SINGLE_LED_COLOR_MENU:
            visibleMenu = LED_COLORS_MENU;
            menuContent = led_colors_menu_items;
            current_menu_length = LED_COLORS_MENU_LENGTH;
            break;
        case CALIBRATE_LDR_MENU:
            visibleMenu = SETTINGS_MENU;
            menuContent = settings_menu_items;
            current_menu_length = SETTINGS_MENU_LENGTH;
            break;
        case SELECT_NETWORK:
            wifi_started = 0;
            visibleMenu = SETTINGS_MENU;
            menuContent = settings_menu_items;
            current_menu_length = SETTINGS_MENU_LENGTH;
            break;
        case ABOUT:
            visibleMenu = MAIN_MENU;
            menuContent = main_menu_items;
            current_menu_length = MAIN_MENU_LENGTH;
            break;
        case MAIN_MENU:
            // No action if already in main menu
            return;
    }
    menuIndex = 0;
    topIndex = 0;
    menu_update();
}

/**
 * @brief Executes the action associated with the selected menu item.
 * Handles menu navigation (switching between menus) and specific robot actions.
 * @param index The index of the selected item in the current menu.
 */
void menu_click(uint8_t index) {
    switch(visibleMenu) {
        case MAIN_MENU:
            switch(index) {
                case 0: // "Start / Stop"
                    inMovement = !inMovement;
                    active = inMovement; // Set active based on inMovement
                    status_display();
                    if (inMovement) {
                        // When starting, show current mode
                        navigation_info_display();
                        ldr_display_counter = 0; // Initialize counter for LDR display
                    }
                    return; // Don't fall through to menu_update as we're in a special state
                case 1: // "LEDS: ON/OFF"
                    toggle_all_leds();
                    break;
                case 2: // "Mode"
                    visibleMenu = MODE_MENU;
                    menuContent =  mode_names;
                    current_menu_length = MODE_MENU_LENGTH;
                    menuIndex = mode;
                    topIndex = (menuIndex > 0) ? menuIndex - 1 : 0;
                    menu_update();
                    return;
                case 3: // "Settings"
                    visibleMenu = SETTINGS_MENU;
                    menuContent = settings_menu_items;
                    current_menu_length = SETTINGS_MENU_LENGTH;
                    menuIndex = 0;
                    topIndex = 0;
                    menu_update();
                    return;
                case 4: // "About"
                    visibleMenu = ABOUT;
                    menuContent = about_items;
                    current_menu_length = ABOUT_MENU_LENGTH;
                    menuIndex = 0;
                    topIndex = 0;
                    menu_update();
                    return;
            }
            break;

        case SETTINGS_MENU:
            switch(index) {
                case 0: // "Speed"
                    visibleMenu = SPEED_MENU;
                    menuContent = speed_menu_items;
                    current_menu_length = SPEED_MENU_LENGTH;
                    menuIndex = speed_i;
                    topIndex = (menuIndex > 0) ? menuIndex - 1 : 0;
                    menu_update();
                    return;
                case 1: // "LED colors"
                    visibleMenu = LED_COLORS_MENU;
                    menuContent = led_colors_menu_items;
                    current_menu_length = LED_COLORS_MENU_LENGTH;
                    menuIndex = 0;
                    topIndex = 0;
                    menu_update();
                    return;
                case 2: // Calibrate LDRs
                    visibleMenu = CALIBRATE_LDR_MENU;
                    menuContent = calibrate_ldrs_menu_items;
                    current_menu_length = LDR_MENU_LENGTH;
                    menuIndex = 0;
                    topIndex = 0;
                    menu_update();
                    return;
                case 3: // Choose Wifi network
                    visibleMenu = SELECT_NETWORK;
                    menuContent = ssid_list;
                    current_menu_length = WIFI_MENU_LENGTH;
                    menuIndex = 0;
                    topIndex = 0;
                    menu_update();
                    return;
            }
            break;

        case MODE_MENU:
            mode = index;
            mode_display();
            visibleMenu = MAIN_MENU;
            menuContent = main_menu_items;
            current_menu_length = MAIN_MENU_LENGTH;
            menuIndex = 0;
            topIndex = 0;
            menu_update();
            return;

        case SPEED_MENU:
            set_robot_speed(index);
            speed_display();
            visibleMenu = SETTINGS_MENU;
            menuContent = settings_menu_items;
            current_menu_length = SETTINGS_MENU_LENGTH;
            menuIndex = 0;
            topIndex = 0;
            menu_update();
            return;

        case LED_COLORS_MENU:
            switch(index) {
                case 0: // "Right LED"
                    visibleMenu = SINGLE_LED_COLOR_MENU;
                    menuContent = color_Names;
                    current_menu_length = NUM_SINGLE_COLORS;
                    menuIndex = ledColor_right;
                    topIndex = (menuIndex > 0) ? menuIndex - 1 : 0;
                    settingRightLed = 1;
                    menu_update();
                    return;
                case 1: // "Left LED"
                    visibleMenu = SINGLE_LED_COLOR_MENU;
                    menuContent = color_Names;
                    current_menu_length = NUM_SINGLE_COLORS;
                    menuIndex = ledColor_left;
                    topIndex = (menuIndex > 0) ? menuIndex - 1 : 0;
                    settingRightLed = 0;
                    menu_update();
                    return;
            }
            break;

        case SINGLE_LED_COLOR_MENU:
            set_single_led_color(settingRightLed, index);
            if (ledsOn) {robot_LEDs(ledColor_left, ledColor_right);} // WHY AREN'T THE COLORS UPDATED AUTOMATICALLY?
            single_led_display(settingRightLed);
            visibleMenu = LED_COLORS_MENU;
            menuContent = led_colors_menu_items;
            current_menu_length = LED_COLORS_MENU_LENGTH;
            menuIndex = (settingRightLed == 1) ? 0 : 1;
            topIndex = 0;
            menu_update();
            return;

        case CALIBRATE_LDR_MENU:
            calibrate_LDR(index == 1); // MAX
            ldr_calibration_display(index == 1);
            visibleMenu = CALIBRATE_LDR_MENU;
            menuContent = calibrate_ldrs_menu_items;
            current_menu_length = LDR_MENU_LENGTH;
            menuIndex = 0;
            topIndex = 0;
            menu_update();
            return;

        case SELECT_NETWORK:
            select_network(index);
            network_display();
            visibleMenu = MAIN_MENU;
            menuContent = main_menu_items;
            current_menu_length = MAIN_MENU_LENGTH;
            menuIndex = 0;
            topIndex = 0;
            menu_update();
            return;

        case ABOUT:
            about_display(index);
            visibleMenu = ABOUT;
            menuContent = about_items;
            current_menu_length = ABOUT_MENU_LENGTH;
            menuIndex = 0;
            topIndex = 0;
            menu_update();
            return;
    }

    delay_ms(1000); // Only execute this if an action didn't already return
    menu_update();
}


/**
 * @brief Handles menu navigation based on joystick button presses.
 */
void menu_loop(void) {
    if (inMovement) { // If robot is in a running/navigation mode
        if (joystick_select_pressed) {
            joystick_select_pressed = 0; // Clear the flag
            inMovement = 0;          // Stop the robot
            active = 0;        // Exit navigation mode
            if (ledsOn) {robot_LEDs(ledColor_left, ledColor_right);}
            else {robot_LEDs(0,0);}    // Set correct LED colors (if ON)
            init_menu();               // Go back to the main menu and display it
            delay_ms(200);             // Debounce delay
            return;                    // Exit the function to prevent further menu processing
        }

        // Increment the counter for LDR display
        ldr_display_counter++;
        // Check if it's time to update the LDR display (100 * 10ms = 1000ms = 1 second)
        if (ldr_display_counter >= LDR_DISPLAY_PERIODS) {
            navigation_info_display();
            ldr_display_counter = 0; // Reset counter
        }
        
        // If robot is running, no other menu interactions (up, down, right) should occur
        // We only allow pressing LEFT to exit navigation mode.
        // Other joystick presses while inMovement will be ignored in this function.
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
            menu_update();
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
            menu_update();
        }
        delay_ms(200);
    }
    else if (joystick_right_pressed) {
        joystick_right_pressed = 0;
        menu_click(menuIndex);
        delay_ms(200);
    }
    else if (joystick_left_pressed) { // This is for going back in menus when not running
        joystick_left_pressed = 0;
        menu_back();
        delay_ms(200);
    }
}
