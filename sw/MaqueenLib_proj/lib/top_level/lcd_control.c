#include <msp430.h>
#include <stdint.h>
#include <string.h> // For strlen, strncpy
#include <stdio.h>  // For sprintf

#include "lcd_control.h"
#include "../low_level/i2c.h"    // For I2C_send, ADDR_LCD
#include "../low_level/timers.h" // For delay_ms
#include "../low_level/gpio.h"   // For LCD_RST_PIN

/**
 * @brief Initializes the LCD display.
 * Performs a hardware reset sequence and sends initial configuration commands
 * via I2C to prepare the LCD for use.
 */
void init_LCD()
{
    uint8_t buffer_i2c[8]; // Buffer for LCD initialization commands

    // Hardware reset sequence for the LCD
    P2OUT &= ~LCD_RST_PIN; // Set LCD_RST_PIN (P2.4) low
    delay_ms(20);          // Wait for 20ms
    P2OUT |= LCD_RST_PIN;  // Set LCD_RST_PIN high
    delay_ms(20);          // Wait for 20ms

    // Sequence of commands to initialize the LCD (from assembly routine equivalent)
    buffer_i2c[0] = 0x00; // Command prefix for instructions
    buffer_i2c[1] = 0x39; // Function Set (8-bit, 2-line, normal instruction set)
    buffer_i2c[2] = 0x14; // Internal OSC frequency
    buffer_i2c[3] = 0x74; // Contrast set (original had 0x74, check if this is correct for your LCD)
    buffer_i2c[4] = 0x54; // Power/ICON/Contrast control
    buffer_i2c[5] = 0x6F; // Follower control
    buffer_i2c[6] = 0x0C; // Display ON, Cursor OFF, Blink OFF
    buffer_i2c[7] = 0x01; // Clear Display

    I2C_send(ADDR_LCD, buffer_i2c, 8); // Send initialization commands to LCD
    delay_ms(20); // Small delay after initialization
}

/**
 * @brief Clears the LCD display.
 * Sends an I2C command to clear the display and reset the cursor position.
 */
void clear_LCD()
{
    uint8_t buffer_i2c[2]; // Only need 2 bytes for Clear Display command

    buffer_i2c[0] = 0x00; // Command prefix for instructions
    buffer_i2c[1] = 0x01; // Clear Display instruction (also sets cursor to home)

    I2C_send(ADDR_LCD, buffer_i2c, 2); // Send clear command to LCD
    delay_ms(2); // Small delay for LCD to process (original had 20ms, 2ms might be enough)
}

/**
 * @brief Sets the cursor position on the LCD.
 *
 * @param row The row number (0 for first line, 1 for second line).
 * @param col The column number (0-15).
 */
void LCD_set_cursor(uint8_t row, uint8_t col)
{
    uint8_t buffer_i2c[2];
    uint8_t address;

    // Calculate DDRAM address based on row and column
    // Line 0 starts at 0x00, Line 1 starts at 0x40
    if (row == 0) {
        address = 0x00 + col;
    } else { // row == 1
        address = 0x40 + col;
    }

    buffer_i2c[0] = 0x00;           // Command prefix for instructions
    buffer_i2c[1] = 0x80 | address; // Set DDRAM Address command (0x80) ORed with address

    I2C_send(ADDR_LCD, buffer_i2c, 2);
}

/**
 * @brief Writes a single character to the LCD at the current cursor position.
 *
 * @param c The character to write.
 */
void LCD_write_char(char c)
{
    uint8_t buffer_i2c[2];
    buffer_i2c[0] = 0x40; // Data prefix for writing characters
    buffer_i2c[1] = c;    // The character to write

    I2C_send(ADDR_LCD, buffer_i2c, 2);
}


/**
 * @brief Displays a string on the LCD.
 * Handles strings up to 32 characters, splitting them into two lines if necessary.
 * This function also sets the cursor to the beginning of the first line before writing.
 * For specific cursor positioning, use LCD_set_cursor first.
 *
 * @param msg Pointer to the null-terminated string to display.
 */
void display_LCD(char *msg)
{
    size_t length = strlen(msg); // Get the length of the input string

    // Ensure cursor is at the beginning of the first line for consistent display
    LCD_set_cursor(0, 0);

    if (length > 16) {
        // If message is longer than 16 characters, display it on two lines.
        // LCD is 2x16 characters.

        char buffer1[17]; // 16 chars + null terminator
        char buffer2[17]; // 16 chars + null terminator

        // 1st line: first 16 characters of the message
        strncpy(buffer1, msg, 16); // Copy up to 16 characters
        buffer1[16] = '\0';        // Null-terminate

        // Send the first line to the LCD
        // We use LCD_write_char repeatedly for simplicity, or we could reformulate I2C_send
        // The original display_LCD used a prefix @ and sent the whole string at once.
        // Let's stick to the original @ prefix logic for multi-char sends.
        char send_buffer1[18]; // '@' (1 byte) + 16 chars (16 bytes) + null terminator (1 byte)
        sprintf(send_buffer1, "@%s", buffer1);
        I2C_send(ADDR_LCD, (uint8_t *)send_buffer1, strlen(send_buffer1));

        // Command to move cursor to the beginning of the second line
        LCD_set_cursor(1, 0);

        // 2nd line: next characters (up to 16) for the second line
        strncpy(buffer2, msg + 16, (length - 16 > 16) ? 16 : (length - 16));
        buffer2[ (length - 16 > 16) ? 16 : (length - 16) ] = '\0'; // Null-terminate

        char send_buffer2[18]; // '@' (1 byte) + 16 chars (16 bytes) + null terminator (1 byte)
        sprintf(send_buffer2, "@%s", buffer2);
        I2C_send(ADDR_LCD, (uint8_t *)send_buffer2, strlen(send_buffer2));

    } else {
        // If message fits on one line (16 characters or less)
        char send_buffer[18]; // Buffer for '@' + message + null
        sprintf(send_buffer, "@%s", msg); // Format with '@' prefix
        I2C_send(ADDR_LCD, (uint8_t *)send_buffer, strlen(send_buffer)); // Send to LCD
    }
}

void update_LCD(char *msg) 
{
    clear_LCD();
    display_LCD(msg);
}