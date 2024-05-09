#ifndef I2C_H_
#define I2C_H_

#include "stm32f1xx_ll_i2c.h"
#include "stm32f1xx_ll_gpio.h"

typedef struct {
    I2C_TypeDef* i2c_base;

    GPIO_TypeDef* scl_port;
    uint32_t scl_pin;
    GPIO_TypeDef* sda_port;
    uint32_t sda_pin;

    IRQn_Type irq_error;
    IRQn_Type irq_event;
} i2c_t;

void i2c_config(i2c_t* i2c);

void i2c_mem_write(I2C_TypeDef* i2c_base, uint8_t i2c_addr, uint16_t reg_addr, void* buf, uint32_t len);

#endif