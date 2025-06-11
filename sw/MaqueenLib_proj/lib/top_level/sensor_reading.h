#ifndef SENSOR_READING_H_
#define SENSOR_READING_H_

#include <stdint.h> // For uint16_t

/**
 * @brief Reads analog values from the two Light Dependent Resistors (LDRs).
 * This function initiates ADC conversions for ADC channels A0 and A5,
 * which are typically connected to LDRs.
 *
 * @param LDR_reading Pointer to an array of two uint16_t elements
 * where the readings from LDR1 (A0) and LDR2 (A5) will be stored.
 */
void read_LDRs(uint16_t *LDR_reading);

/**
 * @brief Reads analog values from the two Joystick analog axes.
 * This function initiates ADC conversions for ADC channels A1 and A4,
 * which are typically connected to the X and Y axes of a joystick.
 *
 * @param JS_reading Pointer to an array of two uint16_t elements
 * where the readings from Joystick X (A1) and Joystick Y (A4) will be stored.
 */
void read_JS_analog(uint16_t *JS_reading);

#endif /* SENSOR_READING_H_ */

