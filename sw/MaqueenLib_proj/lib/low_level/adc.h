#ifndef LOW_LEVEL_ADC_H_
#define LOW_LEVEL_ADC_H_

#include <stdint.h>

// Declare 'ADC_value' as extern so other files can access the conversion result.
// It's defined in low_level_adc.c
extern uint16_t ADC_value;

/**
 * @brief Initializes the ADC12 module for single-channel conversions.
 * Sets ADC on, 12-bit resolution, and sample-and-hold time.
 * Note: Individual channel selection and conversion start are handled
 * by the sensor reading functions in top_level.
 */
void init_adc(void);

// ISR for ADC12 module
// This declaration is for the linker, the actual definition is in the .c file.
#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void);

#endif /* LOW_LEVEL_ADC_H_ */

