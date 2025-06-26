#ifndef WIFI_H_
#define WIFI_H_

#include "../low_level/AT.h"
#include "../low_level/uart.h"

// LED IDs for Wi-Fi commands
#define LEDS        0x19
#define LED_LEFT    0x01
#define LED_RIGHT   0x02

// LED Colors for Wi-Fi commands
#define COLOR_OFF   0x00
#define COLOR_RED   0x01
#define COLOR_GREEN 0x02
#define COLOR_BLUE  0x03
#define COLOR_YELLOW 0x04
#define COLOR_MAGENTA 0x05
#define COLOR_CYAN 0x06
#define COLOR_WHITE 0x07

// Motor controls
#define MOTORS 0x20
#define MOTOR_LEFT 1
#define MOTOR_RIGHT 2
#define MOTOR_BOTH 3
#define STOP 0
#define FRONT 1
#define BACK 2

extern StrConexion conexion;
extern RxReturn wifi_rx;
extern wifi_info wifi_msg;

// Declare the main Wi-Fi control function
void wifi_control(void);

void wifi_init(void);

#endif /* WIFI_H_ */
