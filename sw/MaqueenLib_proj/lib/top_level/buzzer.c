#include <msp430.h>
#include "../low_level/gpio.h"
#include "../low_level/timers.h" // For delay_ms

void sound_buzzer(uint8_t ms) {
    P6OUT |= BUZZER;
    delay_ms(ms);
    P6OUT &= ~BUZZER;
}

void pipip_car(void) {
    uint8_t j = 0;
    for (j = 0; j < 50; j++) {
        sound_buzzer(1);
        delay_ms(1);
    }

    delay_ms(80);

    for (j = 0; j < 50; j++) {
        sound_buzzer(1);
        delay_ms(1);
    }
}
