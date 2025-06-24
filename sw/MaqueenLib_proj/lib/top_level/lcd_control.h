#ifndef LCD_CONTROL_H_
#define LCD_CONTROL_H_

#include <stdint.h> // For uint8_t

/**
 * @brief Initializes the LCD display.
 * Performs a hardware reset sequence and sends initial configuration commands
 * via I2C to prepare the LCD for use.
 */
void init_LCD(void);

/**
 * @brief Clears the LCD display.
 * Sends an I2C command to clear the display and reset the cursor position.
 */
void clear_LCD(void);

/**
 * @brief Sets the cursor position on the LCD.
 *
 * @param row The row number (0 for first line, 1 for second line).
 * @param col The column number (0-15).
 */
void LCD_set_cursor(uint8_t row, uint8_t col);

/**
 * @brief Writes a single character to the LCD at the current cursor position.
 *
 * @param c The character to write.
 */
void LCD_write_char(char c);

/**
 * @brief Displays a string on the LCD.
 * Handles strings up to 32 characters, splitting them into two lines if necessary.
 * This function also sets the cursor to the beginning of the first line before writing.
 * For specific cursor positioning, use LCD_set_cursor first.
 *
 * @param msg Pointer to the null-terminated string to display.
 */
void display_LCD(char *msg);

void update_LCD(char *msg);

#endif /* LCD_CONTROL_H_ */

