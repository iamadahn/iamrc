#ifndef SPI_H_
#define SPI_H_

#include "stm32f1xx_ll_gpio.h"

typedef struct {
    GPIO_TypeDef* sck_port;
    uint32_t sck_pin;
    GPIO_TypeDef* mosi_port;
    uint32_t mosi_pin;
    GPIO_TypeDef* miso_port;
    uint32_t miso_pin;

    IRQn_Type spi_irq;

    SPI_TypeDef* spi_base;
} spi_t;

void spi_init(spi_t* spi);

#endif