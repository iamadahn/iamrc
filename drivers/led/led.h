#ifndef LED_H_
#define LED_H_

#include "stm32f103xb.h"

typedef struct {
    GPIO_TypeDef* port;
    uint32_t pin;
    uint8_t state;
} led_t;

void
led_toggle(led_t* led);

void
led_on(led_t* led);

void
led_off(led_t* led);

#endif
