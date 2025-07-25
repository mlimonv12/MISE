#ifndef LOW_LEVEL_GPIO_H_
#define LOW_LEVEL_GPIO_H_

#include <stdint.h>

// Joystick macros (P3 pins for digital inputs)
#define JS_BITS 0x3D // (BIT0 | BIT2 | BIT3 | BIT4 | BIT5) - P3
#define JS_L BIT5    // P3.0 (Left/back button)
#define JS_SEL BIT0  // P3.2 (Select button)
#define JS_B BIT2    // P3.3 (Down button)
#define JS_F BIT3    // P3.4 (Forward button / Up in menu)
#define JS_R BIT4    // P3.5 (Right button / Down in menu)

// ADC Pins (P1 pins for analog inputs, used by low_level_adc and sensor_reading)
#define JS_ADC_PINS 0x12  // (BIT1 | BIT4) - P1.1 (A1) and P1.4 (A4) for Joystick analog axes
#define LDR_PINS    0x31  // (BIT0 | BIT5) - P1.0 (A0) and P1.5 (A5) for LDRs

// LCD Reset pin
#define LCD_RST_PIN BIT4 // P2.4

// Buzzer
#define BUZZER BIT3

// Global flags for joystick button presses (declared extern)
extern volatile uint8_t joystick_up_pressed;
extern volatile uint8_t joystick_down_pressed;
extern volatile uint8_t joystick_select_pressed;
extern volatile uint8_t joystick_left_pressed;
extern volatile uint8_t joystick_right_pressed;

/**
 * @brief Initializes various General Purpose Input/Output (GPIO) pins.
 * Configures ADC pins for joystick and LDRs, sets up joystick digital inputs
 * with pull resistors and enables interrupts. Also configures the LCD reset pin.
 */
void init_GPIOs(void);

// ISR for Port 3 (Joystick) Interrupts
// This declaration is for the linker, the actual definition is in the .c file.
#pragma vector=PORT3_VECTOR
__interrupt void readjoystick(void);

#endif /* LOW_LEVEL_GPIO_H_ */

