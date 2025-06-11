#include <stdint.h>
#include <string.h> // For strlen, though not strictly used directly
#include <stdio.h>  // For sprintf, though not used directly in this menu logic

#include "robot_menu.h"
#include "lcd_control.h"    // For LCD display functions
#include "robot_control.h"  // For robot_LEDs function
#include "../low_level/timers.h" // For delay_ms
#include "../low_level/gpio.h"   // For joystick button pressed flags

// Menu state global variables (defined here)
uint8_t currentMenu = MAIN_MENU;
uint8_t menuIndex = 0;
uint8_t topVisibleIndex = 0; // The index of the top-most visible item on the LCD

// Mode setting
const char* mode_names[NUM_MODES] = {"Free Roam", "Light Mode", "Dark Mode"};
uint8_t currentNavigationMode = 0; // Default to "Free Roam" (index 0)
uint8_t navigationMode = 0; // Flag: 0 = menu active, 1 = robot navigation active

// Speed settings
const char* speed_names[NUM_SPEEDS] = {"Slow", "Medium", "Fast"};
const uint8_t speed_values[NUM_SPEEDS] = {70, 170, 255}; // Values used for motor control
uint8_t currentSpeedIndex = 0; // Default to slow speed (index 0)
uint8_t currentSpeed = 70; // Default to slow speed value

// LED color settings
const char* color_Names[NUM_COLORS] = {
    "Red", "Green", "Yellow", "Blue",
    "Pink", "Cyan", "White", "Off"
};
uint8_t currentColor = 7; // Default to "Off" (index 7 for Off)

// Menu definitions
const char* main_menu_items[] = {
    "Start",
    "Settings",
    "About",
    "Shutdown"
};

const char* settings_menu_items[] = {
    "Mode",
    "Speed",
    "LED Color",
    "Back"
};

const char* mode_menu_items[] = {
    "Free Roam",
    "Light Mode",
    "Dark Mode",
    "Back" // To go back to settings menu
};

const char* speed_menu_items[] = {
    "Slow",
    "Medium",
    "Fast",
    "Back" // To go back to settings menu
};

const char* led_menu_items[] = {
    "Red", "Green", "Yellow", "Blue",
    "Pink", "Cyan", "White", "Off",
    "Back" // To go back to settings menu
};

// Current menu pointers (initialized to main menu)
const char** current_menu = main_menu_items;
uint8_t current_menu_length = sizeof(main_menu_items) / sizeof(main_menu_items[0]);


/**
 * @brief Initializes the menu system, setting the initial menu and displaying it.
 */
void init_menu(void) {
    currentMenu = MAIN_MENU;
    menuIndex = 0;
    topVisibleIndex = 0;
    current_menu = main_menu_items;
    current_menu_length = sizeof(main_menu_items) / sizeof(main_menu_items[0]);
    navigationMode = 0; // Ensure menu is active, not navigation
    update_menu_display();
}

/**
 * @brief Updates the LCD display with the current menu items.
 * Shows two visible items and a '>' cursor for the selected item.
 */
void update_menu_display(void) {
    clear_LCD(); // Clear the entire LCD

    // Display the first visible line (topVisibleIndex)
    LCD_set_cursor(0, 0); // Set cursor to the beginning of the first line
    if (menuIndex == topVisibleIndex) {
        LCD_write_char('>'); // Indicate selected item
    } else {
        LCD_write_char(' '); // No cursor
    }
    // Write the actual menu item text
    display_LCD((char*)current_menu[topVisibleIndex]);

    // Display the second visible line (topVisibleIndex + 1), if available
    if (topVisibleIndex + 1 < current_menu_length) {
        LCD_set_cursor(1, 0); // Set cursor to the beginning of the second line
        if (menuIndex == topVisibleIndex + 1) {
            LCD_write_char('>'); // Indicate selected item
        } else {
            LCD_write_char(' '); // No cursor
        }
        // Write the actual menu item text
        display_LCD((char*)current_menu[topVisibleIndex + 1]);
    }
}

/**
 * @brief Updates the LCD display to show the currently selected navigation mode.
 */
void update_mode_display(void) {
    clear_LCD();
    LCD_set_cursor(0,0);
    display_LCD("Mode set to ");
    LCD_set_cursor(1,0);
    display_LCD((char*)mode_names[currentNavigationMode]);
    delay_ms(1000); // Display message for 1 second
}

/**
 * @brief Updates the LCD display to show the currently selected speed.
 */
void update_speed_display(void) {
    clear_LCD();
    LCD_set_cursor(0,0);
    display_LCD("Speed set to ");
    LCD_set_cursor(1,0);
    display_LCD((char*)speed_names[currentSpeedIndex]);
    delay_ms(1000); // Display message for 1 second
}

/**
 * @brief Updates the LCD display to show the currently selected LED color.
 */
void update_led_display(void) {
    clear_LCD();
    LCD_set_cursor(0,0);
    display_LCD("Color set to ");
    LCD_set_cursor(1,0);
    display_LCD((char*)color_Names[currentColor]); // Display the name of the color
    delay_ms(1000); // Display message for 1 second
}

/**
 * @brief Sets the robot's operating speed.
 * @param speed_id The index of the selected speed (0 for Slow, 1 for Medium, 2 for Fast).
 */
void set_robot_speed(uint8_t speed_id) {
    currentSpeedIndex = speed_id;
    currentSpeed = speed_values[speed_id];
    // No direct motor command here, it's used by robot_control in main loop
}

/**
 * @brief Sets the robot's LED color.
 * @param color_id The index of the selected color.
 */
void set_led_color(uint8_t color_id) {
    currentColor = color_id; // Set current color index directly
    // The original code had color_id + 1, assuming color 0 was "Off" and actual colors started at 1.
    // If "Off" is at index 7, then index corresponds to the color.
    // Assuming color_id directly maps to the robot_LEDs parameter here.
    if (color_id < NUM_COLORS) { // Ensure it's a valid color index
        robot_LEDs(color_id +1, color_id +1); // Assuming 0 for off, 1 for red etc.
        if (color_id == (NUM_COLORS -1) ) // If 'Off' is selected
             robot_LEDs(0, 0); // Send 0,0 to turn off
    }
}

/**
 * @brief Executes the action associated with the selected menu item.
 * Handles menu navigation (switching between menus) and specific robot actions.
 * @param index The index of the selected item in the current menu.
 */
void execute_menu_action(uint8_t index) {
    clear_LCD(); // Clear LCD before showing action feedback or new menu

    switch(currentMenu) {
        case MAIN_MENU:
            switch(index) {
                case 0: // "Start" Navigation
                    navigationMode = 1; // Activate robot navigation
                    display_LCD("Starting...");
                    delay_ms(1000);
                    clear_LCD(); // Clear LCD for navigation mode display
                    display_LCD("Navigation Mode"); // Display brief message for navigation mode
                    return; // Return immediately, as main loop will handle navigation
                case 1: // "Settings"
                    currentMenu = SETTINGS_MENU;
                    current_menu = settings_menu_items;
                    current_menu_length = sizeof(settings_menu_items) / sizeof(settings_menu_items[0]);
                    menuIndex = 0; // Reset index for new menu
                    topVisibleIndex = 0;
                    update_menu_display(); // Display the new menu
                    return;
                case 2: // "About"
                    display_LCD("Robot v1.0");
                    LCD_set_cursor(1, 0);
                    display_LCD("MSP430FR2355");
                    break; // Will fall through to delay_ms and then update_menu_display
                case 3: // "Shutdown"
                    display_LCD("Shutting down...");
                    // Potentially add shutdown logic here (e.g., stop motors, turn off LEDs)
                    // For now, just a message
                    break; // Will fall through to delay_ms and then update_menu_display
            }
            break;

        case SETTINGS_MENU:
            switch(index) {
                case 0: // "Mode"
                    currentMenu = MODE_MENU;
                    current_menu = mode_menu_items;
                    current_menu_length = sizeof(mode_menu_items) / sizeof(mode_menu_items[0]);
                    menuIndex = currentNavigationMode; // Set initial index to current mode
                    topVisibleIndex = (menuIndex > 0) ? menuIndex - 1 : 0;
                    update_menu_display();
                    return;
                case 1: // "Speed"
                    currentMenu = SPEED_MENU;
                    current_menu = speed_menu_items;
                    current_menu_length = sizeof(speed_menu_items) / sizeof(speed_menu_items[0]);
                    menuIndex = currentSpeedIndex; // Set initial index to current speed
                    topVisibleIndex = (menuIndex > 0) ? menuIndex - 1 : 0;
                    update_menu_display();
                    return;
                case 2: // "LED Color"
                    currentMenu = LED_MENU;
                    current_menu = led_menu_items;
                    current_menu_length = sizeof(led_menu_items) / sizeof(led_menu_items[0]);
                    menuIndex = currentColor; // Set initial index to current color
                    topVisibleIndex = (menuIndex > 0) ? menuIndex - 1 : 0;
                    update_menu_display();
                    return;
                case 3: // "Back" to Main Menu
                    currentMenu = MAIN_MENU;
                    current_menu = main_menu_items;
                    current_menu_length = sizeof(main_menu_items) / sizeof(main_menu_items[0]);
                    menuIndex = 0; // Reset index for new menu
                    topVisibleIndex = 0;
                    update_menu_display();
                    return;
            }
            break;

        case MODE_MENU:
            if(index < NUM_MODES) { // Selected a mode (not "Back")
                currentNavigationMode = index;
                update_mode_display(); // Show confirmation
            } else { // "Back" was selected (index == NUM_MODES)
                // Go back to Settings Menu
                currentMenu = SETTINGS_MENU;
                current_menu = settings_menu_items;
                current_menu_length = sizeof(settings_menu_items) / sizeof(settings_menu_items[0]);
                menuIndex = 0;
                topVisibleIndex = 0;
            }
            update_menu_display(); // Display the menu (either current or settings)
            return; // Return immediately
        case SPEED_MENU:
            if(index < NUM_SPEEDS) { // Selected a speed (not "Back")
                set_robot_speed(index);
                update_speed_display(); // Show confirmation
            } else { // "Back" was selected (index == NUM_SPEEDS)
                // Go back to Settings Menu
                currentMenu = SETTINGS_MENU;
                current_menu = settings_menu_items;
                current_menu_length = sizeof(settings_menu_items) / sizeof(settings_menu_items[0]);
                menuIndex = 0;
                topVisibleIndex = 0;
            }
            update_menu_display(); // Display the menu (either current or settings)
            return; // Return immediately
        case LED_MENU:
            if(index < NUM_COLORS) { // Selected a color (not "Back")
                set_led_color(index);
                update_led_display(); // Show confirmation
            } else { // "Back" was selected (index == NUM_COLORS)
                // Go back to Settings Menu
                currentMenu = SETTINGS_MENU;
                current_menu = settings_menu_items;
                current_menu_length = sizeof(settings_menu_items) / sizeof(settings_menu_items[0]);
                menuIndex = 0;
                topVisibleIndex = 0;
            }
            update_menu_display(); // Display the menu (either current or settings)
            return; // Return immediately
    }

    // If an action resulted in a brief message (like "About" or "Shutdown"),
    // wait and then return to the previous menu display.
    delay_ms(1000);
    update_menu_display();
}


/**
 * @brief Handles menu navigation based on joystick button presses.
 * This function should be called periodically (e.g., in the main loop)
 * to check for UP, DOWN, and SELECT/BACK button presses and update the menu state.
 */
void handle_menu(void) {
    // Only process menu input if navigation mode is NOT active
    if (navigationMode) {
        // If "Back" button is pressed during navigation mode, return to main menu
        if (joystick_back_pressed) {
            joystick_back_pressed = 0; // Clear the flag
            navigationMode = 0; // Deactivate navigation mode
            init_menu(); // Return to main menu display
            return; // Exit function early
        }
        return; // Do not process other menu inputs if in navigation mode
    }

    // Handle UP button press (JS_F: Forward on joystick)
    if (joystick_up_pressed) {
        joystick_up_pressed = 0; // Clear the flag
        if (menuIndex > 0) {
            menuIndex--; // Move menu selection up
            if (menuIndex < topVisibleIndex) {
                topVisibleIndex = menuIndex; // Scroll display up if needed
            }
            update_menu_display(); // Refresh LCD
        }
        delay_ms(200); // Debounce delay
    }
    // Handle DOWN button press (JS_R: Right on joystick)
    else if (joystick_down_pressed) {
        joystick_down_pressed = 0; // Clear the flag
        if (menuIndex < current_menu_length - 1) {
            menuIndex++; // Move menu selection down
            if (menuIndex > topVisibleIndex + 1) { // Check if new index is beyond visible window
                topVisibleIndex++; // Scroll display down
            }
            update_menu_display(); // Refresh LCD
        }
        delay_ms(200); // Debounce delay
    }
    // Handle SELECT button press (JS_SEL: Select on joystick)
    else if (joystick_select_pressed) {
        joystick_select_pressed = 0; // Clear the flag
        execute_menu_action(menuIndex); // Execute action for selected item
        delay_ms(200); // Debounce delay
    }
    // Handle BACK button press (JS_B: Back on joystick)
    else if (joystick_back_pressed) {
        joystick_back_pressed = 0; // Clear the flag
        // Simulate "Back" action if in a sub-menu
        if (currentMenu == SETTINGS_MENU || currentMenu == MODE_MENU ||
            currentMenu == SPEED_MENU || currentMenu == LED_MENU) {
            // Find the "Back" option's index and execute it
            // This is a bit of a hack: assumes "Back" is always the last option.
            execute_menu_action(current_menu_length - 1);
        }
        delay_ms(200); // Debounce delay
    }
}

