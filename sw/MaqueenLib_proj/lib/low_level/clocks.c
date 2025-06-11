#include <msp430.h>
#include "clocks.h"

/**
 * @brief Initializes the MSP430's clock system.
 *
 * Configures one FRAM waitstate as required by the device datasheet for MCLK
 * operation beyond 8MHz before configuring the clock system.
 * Sets XT1 as FLL reference source (commented out, using REFOCLK instead).
 * Configures DCO to 16MHz and sets it as MCLK and SMCLK source.
 * Sets REFOCLK (~32768Hz) as ACLK source.
 */
void init_clocks()
{
    // Configure one FRAM waitstate as required by the device datasheet for MCLK operation beyond 8MHz
    FRCTL0 = FRCTLPW | NWAITS_1;

    // P2.6~P2.7: crystal pins (if external crystal is used for XT1)
    P2SEL1 |= BIT6 | BIT7;

    /* Original code had a loop to clear XT1 and DCO fault flags,
     * but it's typically for external crystal startup.
     * do
     * {
     * CSCTL7 &= ~(XT1OFFG | DCOFFG); // Clear XT1 and DCO fault flag
     * SFRIFG1 &= ~OFIFG;
     * } while (SFRIFG1 & OFIFG); // Test oscillator fault flag
     */

    __bis_SR_register(SCG0); // Disable FLL (Frequency Locked Loop) for configuration

    // Set ACLK source to REFOCLK (internal reference oscillator)
    // CSCTL3 |= SELREF__XT1CLK; // Original: Set XT1 as FLL reference source (commented out)
    CSCTL3 |= SELA__REFOCLK;

    // Set DCOFTRIM=5, DCO Range = 16MHz (DCORSEL_5 corresponds to 16MHz range)
    CSCTL1 = DCORSEL_5; // DCOFTRIMEN_1 | DCOFTRIM0 | DCOFTRIM1 | DCORSEL_5; // Original had trim enabled
    // Note: Software_Trim() was commented out in original, so DCOFTRIMEN is also removed.

    // Set DCOCLKDIV = 16MHz. FLLD_0 means divide by 1.
    // 487 is the FLLN parameter: DCOCLK = Fref * (FLLN + 1) -> 16MHz = 32768Hz * (487 + 1)
    CSCTL2 = FLLD_0 + 487;
    __delay_cycles(3); // Small delay for FLL to settle

    __bic_SR_register(SCG0); // Enable FLL

    // Set ACLK source to REFOCLK (approx 32768Hz)
    // CSCTL4 = SELMS__DCOCLKDIV | SELA__XT1CLK; // Original: set XT1 as ACLK source
    CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK; // set REFOCLK as ACLK source

    // default DCOCLKDIV as MCLK and SMCLK source
    // P1DIR |= BIT0 | BIT1; // Original: set SMCLK, ACLK pin as output
    // P1SEL1 |= BIT0 | BIT1; // Original: set SMCLK and ACLK pin as second function

    // Disable the GPIO power-on default high-impedance mode to activate previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;
}

