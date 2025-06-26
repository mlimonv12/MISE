#include <msp430.h>
#include "gpio.h"
#include "timers.h" // For delay_ms

// Global flags for joystick button presses, defined here
volatile uint8_t joystick_up_pressed = 0;
volatile uint8_t joystick_down_pressed = 0;
volatile uint8_t joystick_select_pressed = 0;
volatile uint8_t joystick_left_pressed = 0;
volatile uint8_t joystick_right_pressed = 0;

/**
 * @brief Initializes various General Purpose Input/Output (GPIO) pins.
 * Configures ADC pins for joystick and LDRs, sets up joystick digital inputs
 * with pull resistors and enables interrupts. Also configures the LCD reset pin.
 */
void init_GPIOs()
{
    // Initialize ADC pins (P1.0, P1.1, P1.4, P1.5 to analog function)
    P1SEL0 |= (JS_ADC_PINS | LDR_PINS);
    P1SEL1 &= ~(JS_ADC_PINS | LDR_PINS); // Clear secondary function (selects primary module function)

    // Initialize all ports on P3 to primary function (GPIO) for joystick digital inputs
    P3SEL0 &= ~JS_BITS;
    P3SEL1 &= ~JS_BITS;

    // Initialize P2.4 to GPIO mode for LCD RST
    P2SEL0 &= ~LCD_RST_PIN;
    P2SEL1 &= ~LCD_RST_PIN;

    P3DIR &= ~JS_BITS; // Set Joystick pins (P3.0, P3.2, P3.3, P3.4, P3.5) as inputs
    P2DIR |= LCD_RST_PIN; // Set P2.4 as output for LCD RST

    // Configure pull-up/pull-down resistors for Joystick inputs
    P3REN |= (JS_SEL | JS_B | JS_F | JS_R | JS_L); // Enable pull resistors for all JS pins

    P3OUT |= JS_BITS; // All joystick inputs pulled high (pull-up resistors)
                      // This means buttons should pull the pin to ground when pressed.

    P2OUT &= ~LCD_RST_PIN; // LCD RST Initially set to low (for reset sequence)

    P3IFG &= ~JS_BITS; // Clear any pending Joystick interrupt flags
    P3IE |= JS_BITS; // Enable interrupts for all Joystick pins

    // Set interrupt edge for High-to-low transition (button pressed from idle high)
    P3IES |= JS_BITS;
}

//******************************************************************************
// Port 3 Interrupt Service Routine (ISR) for Joystick *************************
//******************************************************************************
#pragma vector=PORT3_VECTOR
__interrupt void readjoystick(void)
{
    // Reading P3IV clears the highest priority interrupt flag and returns its value.
    // This is the most reliable way to handle multiple flags in a single ISR.
    uint16_t P3IV_val = P3IV;

    // Acknowledge all flags to prevent re-entry if not using P3IV with switch.
    // Or, more precisely, P3IFG &= ~JS_BITS; can be used if P3IV isn't fully utilized in switch.
    // Using P3IV for switch cases handles clearing automatically for the matched case.
    // However, if multiple flags are set, only the highest priority is processed by P3IV.
    // So, clearing all flags explicitly is often safer if you don't care about priority.
    P3IFG &= ~JS_BITS; // Clear all Port 3 interrupt flags (crucial for ensuring flags are cleared)


    // Check the individual button states by reading the input register directly,
    // and setting flags based on the detected low (pressed) state.
    // P3IN & BIT0 is `1` if P3.0 is high, `0` if P3.0 is low.
    // Since we set pull-ups, a press means the pin goes low.
    if (!(P3IN & JS_F)) { // If Forward (P3.4) is pressed (goes low)
        joystick_up_pressed = 1;
    }
    if (!(P3IN & JS_B)) { // If Back/down is pressed (goes low) -> acting as DOWN in menu
        joystick_down_pressed = 1;
    }
    if (!(P3IN & JS_SEL)) { // If Select is pressed (goes low)
        joystick_select_pressed = 1;
    }
    if (!(P3IN & JS_L)) { // If Left is pressed (goes low)
        joystick_left_pressed = 1;
    }
    if (!(P3IN & JS_R)) { // If Right is pressed (goes low) - currently unused in menu
        joystick_right_pressed = 1;
    }

    P3IE |= JS_BITS;
}

