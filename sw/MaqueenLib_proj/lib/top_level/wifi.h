#ifndef WIFI_H_
#define WIFI_H_

#include "../low_level/AT.h"
#include "../low_level/uart.h"

// LED IDs for Wi-Fi commands
#define LED_RGB     0x19
#define LED_LEFT    0x01
#define LED_RIGHT   0x02
#define LED_BOTH    0xFE  // Broadcast ID for both LEDs

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


extern StrConexion conexion;
extern RxReturn recepcion;
extern RecepcionWifi modWifi;

// Declare the main Wi-Fi control function
void wifi_control(void);

#endif /* WIFI_H_ */