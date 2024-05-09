#include "i2c.h"
#include <stddef.h>
#include "stm32f1xx_ll_i2c.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_utils.h"

void
i2c_config(i2c_t* i2c) {
    /* this type of initalization doesn't work currently */
    /* 
    LL_RCC_ClocksTypeDef rcc_clocks;

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
    
    LL_I2C_DisableOwnAddress2(i2c->i2c_base);
    LL_I2C_DisableGeneralCall(i2c->i2c_base);
    LL_I2C_EnableClockStretching(i2c->i2c_base);

    LL_I2C_SetMode(i2c->i2c_base, LL_I2C_MODE_I2C);

    NVIC_SetPriority(i2c->irq_error, 0);
    NVIC_EnableIRQ(i2c->irq_error);

    NVIC_SetPriority(i2c->irq_event, 0);
    NVIC_EnableIRQ(i2c->irq_event);

    LL_I2C_Disable(i2c->i2c_base);
    LL_RCC_GetSystemClocksFreq(&rcc_clocks);
    LL_I2C_ConfigSpeed(i2c->i2c_base, rcc_clocks.PCLK1_Frequency, 100000, LL_I2C_DUTYCYCLE_2);

    LL_I2C_EnableIT_EVT(i2c->i2c_base);
    LL_I2C_EnableIT_ERR(i2c->i2c_base);

    LL_GPIO_SetPinMode(i2c->scl_port, i2c->scl_pin, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinSpeed(i2c->scl_port, i2c->scl_pin, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinOutputType(i2c->scl_port, i2c->scl_pin, LL_GPIO_OUTPUT_OPENDRAIN);
    LL_GPIO_SetPinPull(i2c->scl_port, i2c->scl_pin, LL_GPIO_PULL_UP);

    LL_GPIO_SetPinMode(i2c->sda_port, i2c->sda_pin, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinSpeed(i2c->sda_port, i2c->sda_pin, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinOutputType(i2c->sda_port, i2c->sda_pin, LL_GPIO_OUTPUT_OPENDRAIN);
    LL_GPIO_SetPinPull(i2c->sda_port, i2c->sda_pin, LL_GPIO_PULL_UP);
    */

    LL_I2C_InitTypeDef I2C_InitStruct = {0};

    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
    
    GPIO_InitStruct.Pin = LL_GPIO_PIN_6|LL_GPIO_PIN_7;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

    LL_I2C_DisableOwnAddress2(I2C1);
    LL_I2C_DisableGeneralCall(I2C1);
    LL_I2C_EnableClockStretching(I2C1);
    I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
    I2C_InitStruct.ClockSpeed = 400000;
    I2C_InitStruct.DutyCycle = LL_I2C_DUTYCYCLE_2;
    I2C_InitStruct.OwnAddress1 = 0;
    I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
    I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
    LL_I2C_Init(I2C1, &I2C_InitStruct);
    LL_I2C_SetOwnAddress2(I2C1, 0);
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

