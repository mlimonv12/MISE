#include <msp430.h>
#include "adc.h"
#include "gpio.h" // For JS_ADC_PINS and LDR_PINS macros to clear flags

// Definition of the global variable to store ADC conversion results
uint16_t ADC_value = 0;

/**
 * @brief Initializes the ADC12 module for single-channel conversions.
 * Sets ADC on, 12-bit resolution, and sample-and-hold time.
 * Note: Individual channel selection and conversion start are handled
 * by the sensor reading functions in top_level.
 */
void init_adc()
{
    // Turn on ADC12, set Sample & Hold time to 16 clock cycles (ADCSHT_2)
    ADCCTL0 |= ADCON | ADCSHT_2;
    // Set ADCLK (ADC clock) to MODOSC (default internal oscillator)
    ADCCTL1 |= ADCSHP; // Use sampling timer
    // Clear ADCRES bits, then set to 12-bit resolution (ADCRES_2)
    ADCCTL2 &= ~ADCRES;
    ADCCTL2 |= ADCRES_2;
    // ADCIE = ADCIE0; // Original: Enable interrupt for ADCMEM0 (handled per-read in sensor_reading)

    // Clear any pending ADC interrupt flags for the relevant pins
    // ADCIFG &= ~(JS_ADC_PINS | LDR_PINS); // Clear flags for all pins associated with ADC
}


//******************************************************************************
// ADC Interrupt Service Routine (ISR) *****************************************
//******************************************************************************
#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void){
    // Disable ADC interrupts immediately to prevent re-entry or spurious interrupts
    ADCIE = 0;

    // Read the ADC conversion result from ADCMEM0
    ADC_value = ADCMEM0;

    // Clear the CPUOFF bit from LPM0 on exit from ISR, waking up the CPU
    __bic_SR_register_on_exit(LPM0_bits);

    // Clear any pending ADC interrupt flags for the relevant pins (redundant if ADCIE is 0)
    // ADCIFG &= ~(JS_ADC_PINS | LDR_PINS); // Clear flags for all ADC pins on P1
}

