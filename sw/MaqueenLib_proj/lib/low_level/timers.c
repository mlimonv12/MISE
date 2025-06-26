#include <msp430.h>
#include "timers.h"

// Definition of the global counter variable, initialized to 0
volatile uint32_t count = 0;

/**
 * @brief Initializes Timer B0 for 1ms tick generation.
 * Sets the timer to count up to 16000 cycles using SMCLK (16MHz),
 * resulting in a 1ms period. Interrupts are initially disabled.
 */
void init_timers()
{
    TB0CCR0 = 16000; // 16000 cycles = 0.001s = 1ms (assuming SMCLK is 16MHz)
    TB0CTL |= (TBCLR | TBSSEL__SMCLK); // Clear timer, select SMCLK as source
    TB0CCTL0 &= ~CCIE; // Disable interrupts for CCR0 initially
}

/**
 * @brief Provides a blocking delay in milliseconds.
 * Uses Timer B0 to count the specified number of milliseconds.
 * The timer is configured for UP mode and its CCR0 interrupt is enabled
 * during the delay.
 *
 * @param temps The delay duration in milliseconds.
 */
void delay_ms(volatile uint32_t temps)
{
    // Configure Timer B0: Clear, SMCLK source, UP mode (counts to TB0CCR0)
    TB0CTL |= (TBCLR | TBSSEL__SMCLK | MC__UP);
    TB0CCTL0 |= CCIE; // Enable interrupts for CCR0

    // Wait until 'count' reaches the desired 'temps'
    while(count < temps);

    TB0CCTL0 &= ~CCIE; // Disable interrupts
    TB0CTL &= ~MC__UP; // Stop the timer by clearing the mode bits
    count = 0;         // Reset the counter for the next delay
}

//******************************************************************************
// Timer B0 Interrupt Service Routine (ISR) ************************************
//******************************************************************************
#pragma vector=TIMER0_B0_VECTOR
__interrupt void timerB0_0_isr(void)
{
    TB0CTL &= ~CCIFG; // Clear the Timer B0 Capture/Compare Interrupt Flag (CCIFG) for CCR0
    count++;          // Increment the global counter
}

