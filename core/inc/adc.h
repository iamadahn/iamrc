#ifndef ADC_H_
#define ADC_H_

#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_adc.h"

typedef struct {
    ADC_TypeDef* adc_base;
    GPIO_TypeDef* port;
    uint32_t pin;
    uint32_t channel;
    uint32_t rank;
} adc_t;

void adc_config(adc_t* adc);

#endif