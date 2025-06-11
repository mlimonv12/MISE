#ifndef ROBOT_MENU_H_
#define ROBOT_MENU_H_

#include <stdint.h> // For uint8_t

// Menu states
#define MAIN_MENU 0
#define SETTINGS_MENU 1
#define SPEED_MENU 2
#define LED_MENU 3
#define MODE_MENU 4

// Global variables for menu state (declared extern)
extern uint8_t currentMenu;
extern uint8_t menuIndex;
extern uint8_t topVisibleIndex;

// Mode setting
#define NUM_MODES 3
extern const char* mode_names[NUM_MODES];
extern uint8_t currentNavigationMode; // The selected navigation mode (0:Free Roam, 1:Light, 2:Dark)
extern uint8_t navigationMode; // Flag to indicate if robot navigation is active (1: active, 0: inactive)

// Speed settings
#define NUM_SPEEDS 3
extern const char* speed_names[NUM_SPEEDS];
extern const uint8_t speed_values[NUM_SPEEDS]; // Values used for motor control
extern uint8_t currentSpeedIndex;
extern uint8_t currentSpeed; // Default to slow speed

// LED color settings
#define NUM_COLORS 8
extern const char* color_Names[NUM_COLORS];
extern uint8_t currentColor; // Default to off (0x00) or 0-7 for colors

// Menu item definitions (declared extern)
extern const char* main_menu_items[];
extern const char* settings_menu_items[];
extern const char* mode_menu_items[];
extern const char* speed_menu_items[];
extern const char* led_menu_items[];

// Current menu pointers (declared extern)
extern const char** current_menu;
extern uint8_t current_menu_length;


/**
 * @brief Initializes the menu system, setting the initial menu and displaying it.
 */
void init_menu(void);

/**
 * @brief Updates the LCD display with the current menu items.
 * Shows two visible items and a '>' cursor for the selected item.
 */
void update_menu_display(void);

/**
 * @brief Updates the LCD display to show the currently selected navigation mode.
 */
void update_mode_display(void);

/**
 * @brief Updates the LCD display to show the currently selected speed.
 */
void update_speed_display(void);

/**
 * @brief Updates the LCD display to show the currently selected LED color.
 */
void update_led_display(void);

/**
 * @brief Sets the robot's operating speed.
 * @param speed_id The index of the selected speed (0 for Slow, 1 for Medium, 2 for Fast).
 */
void set_robot_speed(uint8_t speed_id);

/**
 * @brief Sets the robot's LED color.
 * @param color_id The index of the selected color.
 */
void set_led_color(uint8_t color_id);

/**
 * @brief Executes the action associated with the selected menu item.
 * Handles menu navigation (switching between menus) and specific robot actions.
 * @param index The index of the selected item in the current menu.
 */
void execute_menu_action(uint8_t index);

/**
 * @brief Handles menu navigation based on joystick button presses.
 * This function should be called periodically (e.g., in the main loop)
 * to check for UP, DOWN, and SELECT/BACK button presses and update the menu state.
 */
void handle_menu(void);

#endif /* ROBOT_MENU_H_ */

