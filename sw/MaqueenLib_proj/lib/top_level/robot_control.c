#include <msp430.h>
#include <stdint.h>

#include "robot_control.h"
#include "sensor_reading.h"
#include "../low_level/i2c.h"    // For I2C_send, I2C_receive, ADDR_ROBOT
#include "../low_level/timers.h" // For delay_ms

uint8_t motors_prev [4] = {1, 0, 1, 0}; // PREVIOUS: left_dir, left_speed, right_dir, right_speed
uint8_t motors_next [4]; // NEXT: left_dir, left_speed, right_dir, right_speed
uint8_t leds_state = 0;
uint16_t ldr_vals [2];

float ldr_scale = 0.8;

uint8_t i = 0;

/**
 * @brief Controls the robot's onboard LEDs.
 * Sends I2C commands to set the color of the left and right LEDs.
 * The specific color codes (e.g., 1, 2, 3, 4, 5, 6) depend on the robot's hardware.
 *
 * @param color_left Numerical code for the left LED color.
 * @param color_right Numerical code for the right LED color.
 */
void robot_LEDs(uint8_t color_left, uint8_t color_right)
{
    uint8_t buffer_in[3]; // Buffer to send LED commands

    // Command byte for LEDs (0x0B based on robot protocol)
    buffer_in[0] = 0x0B;
    buffer_in[1] = color_left;  // Left LED color
    buffer_in[2] = color_right; // Right LED color

    I2C_send(ADDR_ROBOT, buffer_in, 3); // Send the command to the robot module
}

/**
 * @brief Controls the robot's motors.
 * Sends I2C commands to set the direction and speed for both left and right motors.
 *
 * @param left_dir Direction for the left motor (e.g., 1 for forward, 2 for backward).
 * @param left_speed Speed for the left motor (0-255).
 * @param right_dir Direction for the right motor.
 * @param right_speed Speed for the right motor.
 */
void motors(uint8_t left_dir, uint8_t left_speed, uint8_t right_dir, uint8_t right_speed)
{
    uint8_t buffer_in[5]; // Buffer to send motor commands

    // Command byte for motors (0x00 based on robot protocol)
    buffer_in[0] = 0x00;
    buffer_in[1] = left_dir;   // Left motor direction (1: forward, 2: backwards)
    buffer_in[2] = left_speed; // Left motor speed (0 to 255)
    buffer_in[3] = right_dir;  // Right motor direction (1: forward, 2: backwards)
    buffer_in[4] = right_speed; // Right motor speed (0 to 255)

    I2C_send(ADDR_ROBOT, buffer_in, 5); // Send the command to the robot module
}

/**
 * @brief Reads the state of the robot's photodetectors.
 * Sends an I2C command to request photodetector data and receives it.
 * The received byte's bits typically represent the state of individual photodetectors.
 *
 * @param buffer_out Pointer to a uint8_t where the photodetector status will be stored.
 */
void fotodetectors(uint8_t *buffer_out)
{
    uint8_t buffer_in = 0x1D; // Command byte to request photodetector data

    I2C_send(ADDR_ROBOT, &buffer_in, 1); // Send the request command
    delay_ms(2); // Small delay to allow the slave to prepare data
    I2C_receive(ADDR_ROBOT, buffer_out, 1); // Receive the 1-byte photodetector status
}

/**
 * @brief Calculates the next motor commands based on photodetector readings
 * for line-following behavior.
 *
 * Analyzes the photodetector status (bits in 'stat') to determine the robot's
 * position relative to a line and adjusts motor speeds/directions accordingly.
 *
 * Photodetector bit mapping (assumed from original code's bit checks):
 * Bit 0: (Likely outer left)
 * Bit 1: Inner Left (P4)
 * Bit 2: Front Center Left (P8)
 * Bit 3: Front Center Right (P16)
 * Bit 4: Inner Right (P2)
 * Bit 5: (Likely outer right)
 *
 * Original code checks:
 * `0b00011110`: All four central photodetectors (P2 | P4 | P8 | P16) -> cul-de-sac
 * `0b00001100`: Front center two photodetectors (P8 | P16) -> on the line, go straight
 * `0b00000100`: Only Front Center Left (P4) -> needs to turn right
 * `0b00001000`: Only Front Center Right (P8) -> needs to turn left
 *
 * @param previous Pointer to an array containing the previous motor directions and speeds.
 * Format: {left_dir, left_speed, right_dir, right_speed}
 * @param next Pointer to an array where the calculated next motor directions and speeds will be stored.
 * Format: {left_dir, left_speed, right_dir, right_speed}
 * @return A uint8_t value representing the robot's calculated state (e.g., STRAIGHT, TURN_R, STOP).
 */
uint8_t linetrack_motors(uint8_t *previous, uint8_t *next, uint8_t speed)
{
    uint8_t stat; // Variable to hold the photodetector status
    fotodetectors(&stat); // Obtain photodetector data

    // Initialize next motor parameters based on previous state
    // This ensures that if no specific line condition is met, motors maintain their previous state.
    next[0] = previous[0]; // left_dir
    next[1] = previous[1]; // left_speed
    next[2] = previous[2]; // right_dir
    next[3] = previous[3]; // right_speed

    // Check for cul-de-sac or T-junction using side photodetectors (all four central ones on line)
    // The mask 0b00011110 (decimal 30) corresponds to (BIT1 | BIT2 | BIT3 | BIT4) or (P2 | P4 | P8 | P16)
    // assuming bit values from original code's context.
    if ((stat & 0b00011110) == 0b00011110) {
        // Robot is at a cul-de-sac (T-junction or end of line)
        next[0] = 0; // Stop left motor (direction 0 means stop)
        next[2] = 0; // Stop right motor

        return STOP; // Return STOP state
    }
    // Check if the robot is perfectly on the line (both front center photodetectors are on the line)
    // The mask 0b00001100 (decimal 12) corresponds to (BIT2 | BIT3) or (P8 | P16)
    else if ((stat & 0b00001100) == 0b00001100) {
        // Both front center photodetectors are on the line
        next[0] = 1; // Enable left motor (direction 1: forward)
        next[2] = 1; // Enable right motor
        next[1] = speed; // Set left speed
        next[3] = speed; // Set right speed

        return STRAIGHT; // Return STRAIGHT state
    }
    // Check if only the left front photodetector is on the line (robot is slightly off to the left)
    // The mask 0b00001100 checks only front center bits.
    // The value 0b00000100 (decimal 4) means only BIT2 (P8) is active within the mask.
    else if ((stat & 0b00001100) == 0b00000100) {
        // Adjust speed to turn right (slow down left, keep right speed)
        next[0] = 1; // Enable left motor (forward)
        next[2] = 1; // Enable right motor (forward)
        next[1] = 0; // Slow down left motor (or stop it for sharper turn)
        next[3] = speed; // Keep right motor speed

        return TURN_R; // Return TURN_R state
    }
    // Check if only the right front photodetector is on the line (robot is slightly off to the right)
    // The mask 0b00001100 checks only front center bits.
    // The value 0b00001000 (decimal 8) means only BIT3 (P16) is active within the mask.
    else if ((stat & 0b00001100) == 0b00001000) {
        // Adjust speed to turn left (keep left speed, slow down right)
        next[0] = 1; // Enable left motor (forward)
        next[2] = 1; // Enable right motor (forward)
        next[1] = speed; // Keep left motor speed
        next[3] = 0; // Slow down right motor (or stop it)

        return TURN_L; // Return TURN_L state
    }
    else {
        // Robot is off the line or in an unhandled state, stop or perform recovery.
        // Original code had both speeds at 35, then direction 1 (forward).
        // This implies a slow forward search or stop.
        next[0] = 1; // Set left motor direction to forward
        next[2] = 1; // Set right motor direction to forward
        next[1] = speed*0.7; // Set left speed to low value
        next[3] = speed*0.7; // Set right speed to low value

        return LOST; // Return LOST state
    }
}

/**
 * @brief Executes the whole line tracking algorithm
 *
 * @param speed Value for the max speed
 * @return A uint8_t value representing the robot's calculated state (e.g., STRAIGHT, TURN_R, STOP).
 */
void linetrack(uint8_t speed)
{
    leds_state = linetrack_motors(motors_prev, motors_next, speed);
    motors(motors_next[0], motors_next[1], motors_next[2], motors_next[3]);

    // Update robot LEDs based on the calculated movement state
    switch (leds_state)
    {
    case STRAIGHT: // Robot moving straight
        robot_LEDs(2, 2); // Green-ish LEDs
        break;
    case TURN_L: // Robot turning left
        robot_LEDs(2, 3); // Green-ish left, Yellow-ish right
        break;
    case TURN_R: // Robot turning right
        robot_LEDs(3, 2); // Yellow-ish left, Green-ish right
        break;
    case STOP: // Robot stopped
        robot_LEDs(1, 1); // Red LEDs
        break;
    case LOST: // Robot lost the line
        robot_LEDs(4, 4); // Blue LEDs
        break;
    default:
        break;
    }

    // Copy the current motor state to the previous state for the next iteration
    for (i = 0; i < 4; i++) {
        motors_prev[i] = motors_next[i];
    }
}

uint8_t follow_motors(uint8_t *previous, uint8_t *next, uint8_t speed, uint16_t *max_light, uint16_t *min_light)
{
    read_LDRs(ldr_vals);
    uint16_t ldr_scaled [2];

    // Scale and make sure it doesn't underflow
    ldr_scaled[0] = (ldr_vals[0] >= min_light[0]) ? (ldr_vals[0] - min_light[0]) : 0;
    ldr_scaled[1] = (ldr_vals[1] >= min_light[1]) ? (ldr_vals[1] - min_light[1]) : 0;
    
    if ((ldr_scaled[0] > ((max_light[0] - min_light[0])*ldr_scale)) && (ldr_scaled[1] > ((max_light[1] - min_light[1])*ldr_scale)))
    {
        next[0] = 2; // Enable left motor
        next[2] = 2; // Enable right motor
        next[1] = speed; // Set left speed
        next[3] = speed; // Set right speed
    }
    else if (ldr_scaled[1] > ((max_light[1] - min_light[1])*ldr_scale))
    {
        next[0] = 2; // Enable left motor
        next[2] = 2; // Enable right motor
        next[1] = speed*0.4; // Set left speed
        next[3] = speed; // Set right speed
    }
    else if (ldr_scaled[0] > ((max_light[0] - min_light[0])*ldr_scale))
    {
        next[0] = 2; // Enable left motor
        next[2] = 2; // Enable right motor
        next[1] = speed; // Set left speed
        next[3] = speed*0.4; // Set right speed
    }
    else
    {
        next[0] = 2; // Enable left motor
        next[2] = 2; // Enable right motor
        next[1] = 0; // Set left speed
        next[3] = 0; // Set right speed
    }
}

void follow_light(uint8_t speed, uint16_t *max_light, uint16_t *min_light)
{
    follow_motors(motors_prev, motors_next, speed, max_light, min_light);
    motors(motors_next[0], motors_next[1], motors_next[2], motors_next[3]);

    // Copy the current motor state to the previous state for the next iteration
    for (i = 0; i < 4; i++) {
        motors_prev[i] = motors_next[i];
    }
}

uint8_t escape_motors(uint8_t *previous, uint8_t *next, uint8_t speed, uint16_t *max_light, uint16_t *min_light)
{
    read_LDRs(ldr_vals);
    uint16_t ldr_scaled [2];

    // Scale and make sure it doesn't underflow
    ldr_scaled[0] = (ldr_vals[0] >= min_light[0]) ? (ldr_vals[0] - min_light[0]) : 0;
    ldr_scaled[1] = (ldr_vals[1] >= min_light[1]) ? (ldr_vals[1] - min_light[1]) : 0;
    
    if ((ldr_scaled[0] > ((max_light[0] - min_light[0])*ldr_scale)) && (ldr_scaled[1] > ((max_light[1] - min_light[1])*ldr_scale)))
    {
        next[0] = 1; // Enable left motor (direction 2: backward)
        next[2] = 1; // Enable right motor
        next[1] = speed; // Set left speed
        next[3] = speed; // Set right speed
    }
    else if (ldr_scaled[1] > ((max_light[1] - min_light[1])*ldr_scale))
    {
        next[0] = 1; // Enable left motor (direction 2: backward)
        next[2] = 1; // Enable right motor
        next[1] = speed; // Set left speed
        next[3] = speed*0.4; // Set right speed
    }
    else if (ldr_scaled[0] > ((max_light[0] - min_light[0])*ldr_scale))
    {
        next[0] = 1; // Enable left motor (direction 2: backward)
        next[2] = 1; // Enable right motor
        next[1] = speed*0.4; // Set left speed
        next[3] = speed; // Set right speed
    }
    else
    {
        next[0] = 1; // Enable left motor (direction 2: backward)
        next[2] = 1; // Enable right motor
        next[1] = 0; // Set left speed
        next[3] = 0; // Set right speed
    }
}

void escape_light(uint8_t speed, uint16_t *max_light, uint16_t *min_light)
{
    escape_motors(motors_prev, motors_next, speed, max_light, min_light);
    motors(motors_next[0], motors_next[1], motors_next[2], motors_next[3]);

    // Copy the current motor state to the previous state for the next iteration
    for (i = 0; i < 4; i++) {
        motors_prev[i] = motors_next[i];
    }
}
