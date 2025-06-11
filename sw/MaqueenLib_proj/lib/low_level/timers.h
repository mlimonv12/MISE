#ifndef LOW_LEVEL_TIMERS_H_
#define LOW_LEVEL_TIMERS_H_

#include <stdint.h> // For uint32_t

// Declare 'count' as extern so other files can access it
// It's modified by the timer ISR and used by delay_ms
extern volatile uint32_t count;

/**
 * @brief Initializes Timer B0 for 1ms tick generation.
 * Sets the timer to count up to 16000 cycles using SMCLK (16MHz),
 * resulting in a 1ms period. Interrupts are initially disabled.
 */
void init_timers(void);

/**
 * @brief Provides a blocking delay in milliseconds.
 * Uses Timer B0 to count the specified number of milliseconds.
 *
 * @param temps The delay duration in milliseconds.
 */
void delay_ms(volatile uint32_t temps);

// ISR for Timer B0 Compare Register 0
// This declaration is for the linker, the actual definition is in the .c file.
#pragma vector=TIMER0_B0_VECTOR
__interrupt void timerB0_0_isr(void);

#endif /* LOW_LEVEL_TIMERS_H_ */

