#include "i2c.h"
#include <stddef.h>
#include "stm32f1xx_ll_i2c.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_utils.h"

void
i2c_config(i2c_t* i2c) {
    LL_I2C_InitTypeDef i2c_init = {0};
    LL_GPIO_InitTypeDef sda_init = {0}, scl_init = {0};

    if (i2c->sda_port == GPIOA) {
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
    } else if (i2c->sda_port == GPIOB) {
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
    } else if (i2c->sda_port == GPIOC) {
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOC);
    } else if (i2c->sda_port == GPIOD) {
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOD);
    }

    if (i2c->scl_port == GPIOA) {
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
    } else if (i2c->scl_port == GPIOB) {
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
    } else if (i2c->scl_port == GPIOC) {
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOC);
    } else if (i2c->scl_port == GPIOD) {
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOD);
    }

    sda_init.Pin = i2c->sda_pin;
    sda_init.Mode = LL_GPIO_MODE_ALTERNATE;
    sda_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    sda_init.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    LL_GPIO_Init(i2c->sda_port, &sda_init);

    scl_init.Pin = i2c->sda_pin;
    scl_init.Mode = LL_GPIO_MODE_ALTERNATE;
    scl_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    scl_init.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    LL_GPIO_Init(i2c->sda_port, &scl_init);

    if (i2c->i2c_base == I2C1) {
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
    } else if (i2c->i2c_base == I2C2) {
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
    } 

    LL_I2C_DisableOwnAddress2(i2c->i2c_base);
    LL_I2C_DisableGeneralCall(i2c->i2c_base);
    LL_I2C_EnableClockStretching(i2c->i2c_base);
    i2c_init.PeripheralMode = LL_I2C_MODE_I2C;
    i2c_init.ClockSpeed = 400000;
    i2c_init.DutyCycle = LL_I2C_DUTYCYCLE_2;
    i2c_init.OwnAddress1 = 0;
    i2c_init.TypeAcknowledge = LL_I2C_ACK;
    i2c_init.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
    LL_I2C_Init(i2c->i2c_base, &i2c_init);
    LL_I2C_SetOwnAddress2(i2c->i2c_base, 0);
}


void 
i2c_mem_write(I2C_TypeDef* i2c_base, uint8_t i2c_addr, uint16_t reg_addr, void* buf, uint32_t len) {
    /* check void pointer for NULL*/
    if (buf == NULL) {
        return;
    } 
    uint8_t* msg = buf;
    LL_I2C_DisableBitPOS(i2c_base);

    /* enable ACK bit and generate start condition on the bus */
    LL_I2C_AcknowledgeNextData(i2c_base, LL_I2C_ACK);
    LL_I2C_GenerateStartCondition(i2c_base);

    /* wait until start bit is set */
    while (!LL_I2C_IsActiveFlag_SB(i2c_base)) {
        ;
    }
    (void) i2c_base->SR1;

    /* transmit i2c slave address with write mode (0x00) */
    LL_I2C_TransmitData8(i2c_base, i2c_addr | 0x00);
    while (!LL_I2C_IsActiveFlag_ADDR(i2c_base)) {
        ;
    }
    LL_I2C_ClearFlag_ADDR(i2c_base);
    while (!LL_I2C_IsActiveFlag_TXE(i2c_base)) {
        ;
    }

    /* transmit i2c slave's register start adress */
    LL_I2C_TransmitData8(i2c_base, (uint8_t)reg_addr);
    while (!LL_I2C_IsActiveFlag_TXE(i2c_base)) {
        ;
    }

    /* transmit the data to be written */
    for (uint32_t i = 0; i < len; i++) {
        LL_I2C_TransmitData8(i2c_base, msg[i]);
        while (!LL_I2C_IsActiveFlag_TXE(i2c_base)) {
            ;
        }
    }

    /* generate stop condition on the bus */
    LL_I2C_GenerateStopCondition(i2c_base);
}

