#include <msp430.h>
#include "sensor_reading.h"
#include "../low_level/adc.h" // For ADC_value, ADC_ISR, init_adc
#include "../low_level/gpio.h" // For LDR_PINS and JS_ADC macros to enable/disable ADCIE

/**
 * @brief Reads analog values from the two Light Dependent Resistors (LDRs).
 * This function initiates ADC conversions for ADC channels A0 and A5,
 * which are typically connected to LDRs. It relies on the ADC ISR to
 * update the global `ADC_value`.
 *
 * @param LDR_reading Pointer to an array of two uint16_t elements
 * where the readings from LDR1 (A0) and LDR2 (A5) will be stored.
 */
void read_LDRs(uint16_t *LDR_reading)
{
    // Enable ADC interrupts for LDR pins (specifically ADCIE for ADCMEM0, as only one is used)
    ADCIE |= (BIT0 | BIT5); // Enable ADC interrupt for A0 and A5 (LDR_PINS)

    // First LDR (connected to A0)
    ADCCTL0 &= ~ADCENC;                 // Disable ADC to configure
    ADCMCTL0 &= ~ADCINCH_15;            // Clear previous channel selection
    ADCMCTL0 |= ADCINCH_0;              // Select A0 (P1.0) as input channel
    ADCCTL0 |= ADCENC | ADCSC;          // Enable ADC and start conversion

    // Enter Low Power Mode 0 (LPM0) and enable general interrupts (GIE).
    // The CPU will wait here until the ADC conversion completes and its ISR
    // wakes the CPU up by clearing LPM0_bits.
    __bis_SR_register(LPM0_bits + GIE);
    __no_operation(); // Placeholder for breakpoint during debugging

    LDR_reading[0] = ADC_value; // Store the converted value from A0

    // Second LDR (connected to A5)
    ADCCTL0 &= ~ADCENC;                 // Disable ADC to configure
    ADCMCTL0 &= ~ADCINCH_15;            // Clear previous channel selection
    ADCMCTL0 |= ADCINCH_5;              // Select A5 (P1.5) as input channel
    ADCCTL0 |= ADCENC | ADCSC;          // Enable ADC and start conversion

    // Wait again for the ADC conversion to complete
    __bis_SR_register(LPM0_bits + GIE);
    __no_operation(); // Placeholder for breakpoint during debugging

    LDR_reading[1] = ADC_value; // Store the converted value from A5
}


/**
 * @brief Reads analog values from the two Joystick analog axes.
 * This function initiates ADC conversions for ADC channels A1 and A4,
 * which are typically connected to the X and Y axes of a joystick.
 * It relies on the ADC ISR to update the global `ADC_value`.
 *
 * @param JS_reading Pointer to an array of two uint16_t elements
 * where the readings from Joystick X (A1) and Joystick Y (A4) will be stored.
 */
void read_JS_analog(uint16_t *JS_reading)
{
    // Enable ADC interrupts for Joystick analog pins (specifically ADCIE for ADCMEM0)
    ADCIE |= (BIT1 | BIT4); // Enable ADC interrupt for A1 and A4 (JS_ADC)

    // First Joystick axis (connected to A1)
    ADCCTL0 &= ~ADCENC;                 // Disable ADC to configure
    ADCMCTL0 &= ~ADCINCH_15;            // Clear previous channel selection
    ADCMCTL0 |= ADCINCH_1;              // Select A1 (P1.1) as input channel
    ADCCTL0 |= ADCENC | ADCSC;          // Enable ADC and start conversion

    // Wait for the ADC conversion to complete
    __bis_SR_register(LPM0_bits + GIE);
    __no_operation();

    JS_reading[0] = ADC_value; // Store the converted value from A1

    // Second Joystick axis (connected to A4)
    ADCCTL0 &= ~ADCENC;                 // Disable ADC to configure
    ADCMCTL0 &= ~ADCINCH_15;            // Clear previous channel selection
    ADCMCTL0 |= ADCINCH_4;              // Select A4 (P1.4) as input channel
    ADCCTL0 |= ADCENC | ADCSC;          // Enable ADC and start conversion

    // Wait for the ADC conversion to complete
    __bis_SR_register(LPM0_bits + GIE);
    __no_operation();

    JS_reading[1] = ADC_value; // Store the converted value from A4
}

