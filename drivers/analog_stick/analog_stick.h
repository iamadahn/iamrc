#ifndef ANALOG_STICK_H_
#define ANALOG_STICK_H_

#include "stm32f1xx_ll_adc.h"
#include "stm32f1xx_ll_gpio.h"

typedef struct {
    ADC_TypeDef* adc_base;
    GPIO_TypeDef* port;
    uint32_t pin;
    uint32_t channel;
} analog_stick_unit_t;

typedef struct {
    uint16_t x;
    uint16_t y;
} analog_stick_data_t;

#endif