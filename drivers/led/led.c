#include "led.h"

void
led_toggle(led_t* led) {
    if(led->state == 1) {
        LL_GPIO_ResetOutputPin(led->port, led->pin);
        led->state = 0;
    } else {
        LL_GPIO_SetOutputPin(led->port, led->pin);
        led->state = 1;
    }
}

void
led_on(led_t* led) {
    LL_GPIO_SetOutputPin(led->port, led->pin);
    led->state = 1;
}

void
led_off(led_t* led) {
    LL_GPIO_ResetOutputPin(led->port, led->pin);
    led->state = 0;
}
