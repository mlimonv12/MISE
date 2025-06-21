#ifndef ROBOT_CONTROL_H_
#define ROBOT_CONTROL_H_

#include <stdint.h> // For uint8_t

// Motor macros for line following logic
#define STRAIGHT 0 // Robot is moving straight on the line
#define TURN_R 1   // Robot needs to turn right
#define TURN_L 2   // Robot needs to turn left
#define STOP 3     // Robot should stop (e.g., at a cul-de-sac)
#define LOST 4     // Robot has lost the line

uint16_t extern ldr_vals [2];

/**
 * @brief Controls the robot's onboard LEDs.
 * Sends I2C commands to set the color of the left and right LEDs.
 *
 * @param color_left Numerical code for the left LED color.
 * @param color_right Numerical code for the right LED color.
 */
void robot_LEDs(uint8_t color_left, uint8_t color_right);

/**
 * @brief Controls the robot's motors.
 * Sends I2C commands to set the direction and speed for both left and right motors.
 *
 * @param left_dir Direction for the left motor (e.g., 1 for forward, 2 for backward).
 * @param left_speed Speed for the left motor (0-255).
 * @param right_dir Direction for the right motor.
 * @param right_speed Speed for the right motor.
 */
void motors(uint8_t left_dir, uint8_t left_speed, uint8_t right_dir, uint8_t right_speed);

/**
 * @brief Reads the state of the robot's photodetectors.
 * Sends an I2C command to request photodetector data and receives it.
 * The received byte's bits typically represent the state of individual photodetectors.
 *
 * @param buffer_out Pointer to a uint8_t where the photodetector status will be stored.
 */
void fotodetectors(uint8_t *buffer_out);

/**
 * @brief Calculates the next motor commands based on photodetector readings
 * for line-following behavior.
 * Analyzes the photodetector status to determine if the robot is on the line,
 * needs to turn, or has reached a specific junction.
 *
 * @param previous Pointer to an array containing the previous motor directions and speeds.
 * @param next Pointer to an array where the calculated next motor directions and speeds will be stored.
 * @param speed Value for the max speed
 * @return A uint8_t value representing the robot's calculated state (e.g., STRAIGHT, TURN_R, STOP).
 */
uint8_t linetrack_motors(uint8_t *previous, uint8_t *next, uint8_t speed);

/**
 * @brief Executes the whole line tracking algorithm
 *
 * @param speed Value for the max speed
 * @return A uint8_t value representing the robot's calculated state (e.g., STRAIGHT, TURN_R, STOP).
 */
void linetrack(uint8_t speed);

uint8_t follow_motors(uint8_t *previous, uint8_t *next, uint8_t speed, uint16_t *max_light, uint16_t *min_light);

void follow_light(uint8_t speed, uint16_t *max_light, uint16_t *min_light);

uint8_t escape_motors(uint8_t *previous, uint8_t *next, uint8_t speed, uint16_t *max_light, uint16_t *min_light);

void escape_light(uint8_t speed, uint16_t *max_light, uint16_t *min_light);

#endif /* ROBOT_CONTROL_H_ */

