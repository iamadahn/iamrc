#ifndef SPI_H_
#define SPI_H_

#include "stm32f1xx_ll_gpio.h"

typedef struct {
    SPI_TypeDef* spi_base;

    GPIO_TypeDef* sck_port;
    uint32_t sck_pin;
    GPIO_TypeDef* mosi_port;
    uint32_t mosi_pin;
    GPIO_TypeDef* miso_port;
    uint32_t miso_pin;

    IRQn_Type spi_irq;
} spi_t;

void spi_config(spi_t* spi);

#endif