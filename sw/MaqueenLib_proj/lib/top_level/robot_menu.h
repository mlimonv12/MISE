#ifndef ROBOT_MENU_H_
#define ROBOT_MENU_H_

#include <stdint.h> // For uint8_t

// Menu States
#define MAIN_MENU               0
#define MODE_MENU               1
#define SETTINGS_MENU           2
#define SPEED_MENU              3
#define LED_COLORS_MENU         4
#define SINGLE_LED_COLOR_MENU   5
#define CALIBRATE_LDR_MENU      6
#define SELECT_NETWORK          7
#define ABOUT                   8

// Choose if we want to press the JS or just move right
//#define PRESS // Use JS press

#ifdef  PRESS
#define EXECUTE_JS joystick_select_pressed
#else
#define EXECUTE_JS joystick_right_pressed
#endif

// Global variables for menu state (declared extern)
extern uint8_t visibleMenu;
extern uint8_t menuIndex;
extern uint8_t topIndex;

// Robot state variables
extern uint8_t inMovement;
extern uint8_t ledsOn;
extern uint8_t wifi_started;

// Mode setting
#define NUM_MODES           5
extern const char* mode_names[NUM_MODES];
extern uint8_t mode;
extern uint8_t active;

// Speed settings
#define NUM_SPEEDS          4
extern const char* speed_names[NUM_SPEEDS];
extern const uint8_t speed_values[NUM_SPEEDS];
extern uint8_t speed_i;
extern uint8_t speed;

// LDR calibration readings
extern uint16_t max_light [2];
extern uint16_t min_light [2];

// LED color settings
//#define NUM_SINGLE_COLORS   7
//extern const char* color_Names[];
extern uint8_t ledColor_right;
extern uint8_t ledColor_left;
extern uint8_t settingRightLed;

// Wi-Fi
extern char ssid_sta [];
extern char pwd_sta [];

// Menu item definitions (declared extern)
extern const char* main_menu_items[];
extern const char* settings_menu_items[];
extern const char* mode_menu_items[];
extern const char* speed_menu_items[];
extern const char* led_colors_menu_items[];
extern const char* single_led_color_menu_items[];

// Current menu pointers (declared extern)
extern const char** current_menu;
extern uint8_t current_menu_length;


// Function Prototypes

/**
 * @brief Initializes the menu system, setting the initial menu and displaying it.
 */
void init_menu(void);

/**
 * @brief Sets the robot's operating speed.
 * @param speed_id The index of the selected speed.
 */
void set_robot_speed(uint8_t speed_id);

/**
 * @brief Sets the robot's LED color for a specific LED.
 * @param isRightLed Flag to indicate if it's the Right (1) or Left (0) LED.
 * @param color_id The index of the selected color.
 */
void set_single_led_color(uint8_t isRightLed, uint8_t color_id);

/**
 * @brief Toggles the overall LED state (ON/OFF) for the robot.
 */
void toggle_all_leds(void);

/**
 * @brief Handles menu navigation based on joystick button presses.
 * This function should be called periodically (e.g., in the main loop)
 * to check for UP, DOWN, SELECT, and LEFT (BACK) button presses and update the menu state.
 */
void menu_loop(void);

#endif /* ROBOT_MENU_H_ */
