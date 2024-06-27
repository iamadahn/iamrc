#include "nrf24l01.h"

#define DWT_CONTROL *(volatile unsigned long*)0xE0001000
#define SCB_DEMCR   *(volatile unsigned long*)0xE000EDFC

uint8_t
nrf24_init(nrf24_t* nrf24) {
    return 1;
}

void
dwt_init(void) {
    SCB_DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT_CONTROL |= DWT_CTRL_CYCCNTENA_Msk;
}

void
delay_us(uint32_t us) {
    uint32_t ticks_to_wait = us * (SystemCoreClock / 1000000);
    DWT->CYCCNT = 0U;
    while (DWT->CYCCNT < ticks_to_wait) {
        ;
    }
}

uint8_t
nrf24_csn(nrf24_t* nrf24, uint8_t mode) {
    if (mode) {
        LL_GPIO_SetOutputPin(nrf24->cs_port, nrf24->cs_pin);
    } else {
        LL_GPIO_ResetOutputPin(nrf24->cs_port, nrf24->cs_pin);
    }
    delay_us(5);
}

uint8_t
nrf24_ce(nrf24_t* nrf24, uint8_t level) {
    if (level) {
        LL_GPIO_SetOutputPin(nrf24->ce_port, nrf24->ce_pin);
    } else {
        LL_GPIO_ResetOutputPin(nrf24->ce_port, nrf24->ce_pin);
    }
}

uint8_t
nrf24_read_register(nrf24_t* nrf24, uint8_t reg) {
    uint8_t addr = R_REGISTER | (REGISTER_MASK & reg);
    uint8_t data = 0;

    nrf24_csn(nrf24, 0);
    LL_SPI_TransmitData8(nrf24->spi, addr);
    data = LL_SPI_ReceiveData8(nrf24->spi);
    LL_SPI_TransmitData8(nrf24->spi, (uint8_t)0xFF);
    data = LL_SPI_ReceiveData8(nrf24->spi);
    nrf24_csn(nrf24, 1);
    return data;
}

uint8_t
nrf24_write_register_single(nrf24_t* nrf24, uint8_t reg, uint8_t value) {
    uint8_t status = 0;
    uint8_t addr = W_REGISTER | (REGISTER_MASK & reg);
    
    nrf24_csn(nrf24, 0);
    LL_SPI_TransmitData8(nrf24->spi, addr);
    status = LL_SPI_ReceiveData8(nrf24->spi);
    LL_SPI_TransmitData8(nrf24->spi, value);
    nrf24_csn(nrf24, 1);

    return status;
}

uint8_t
nrf24_write_register_multi(nrf24_t* nrf24, uint8_t reg, const uint8_t* buf, uint8_t len) {
    uint8_t status;
    uint8_t addr = W_REGISTER | (REGISTER_MASK & reg);
    
    uint8_t* data = (uint8_t*)buf;

    nrf24_csn(nrf24, 0);
    LL_SPI_TransmitData8(nrf24->spi, addr);
    status = LL_SPI_ReceiveData8(nrf24->spi);
    for (uint8_t i; i < len; i++) {
        LL_SPI_TransmitData8(nrf24->spi, data[i]);
    }
    nrf24_csn(nrf24, 1);

    return status;
}