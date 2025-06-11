#ifndef LOW_LEVEL_CLOCKS_H_
#define LOW_LEVEL_CLOCKS_H_

/**
 * @brief Initializes the MSP430's clock system.
 * Configures MCLK and SMCLK to 16MHz using DCO, and ACLK to REFOCLK.
 */
void init_clocks(void);

#endif /* LOW_LEVEL_CLOCKS_H_ */

