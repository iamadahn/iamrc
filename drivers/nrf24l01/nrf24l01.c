#include "nrf24l01.h"

#define DWT_CONTROL *(volatile unsigned long*)0xE0001000
#define SCB_DEMCR   *(volatile unsigned long*)0xE000EDFC

#define nrf24_max(a, b) (a>b?a:b)
#define nrf24_min(a, b) (a<b?a:b)

extern void delay_ms(uint32_t ms);

uint8_t
nrf24_init(nrf24_t* nrf24) {
    uint8_t setup = 0;
    nrf24->payload_size = 32;
    nrf24->addr_width = NRF24_ADDR_WIDTH;
    nrf24->p_variant = 0;
    nrf24->pipe0_reading_address[0] = 0;
    
    for (uint8_t i = 0; i < 6; i++) {
        nrf24->child_pipe_enable[i] = i;
    }

    nrf24_ce(nrf24, 0);
    nrf24_csn(nrf24, 1);
    delay_ms(5);

    nrf24_write_register_single(nrf24, NRF_CONFIG, 0x0C);
    nrf24_set_retries(nrf24, 5, 15);
    nrf24_set_pa_level(nrf24, RF24_PA_MAX);

    if (nrf24_set_data_rate(nrf24, NRF24_250KBPS)) {
        nrf24->p_variant = 1;
    }

    setup = nrf24_read_register(nrf24, RF_SETUP);
    nrf24_set_data_rate(nrf24, NRF24_1MBPS);

    nrf24_toggle_features(nrf24);
    nrf24_write_register_single(nrf24, FEATURE, 0);
    nrf24_write_register_single(nrf24, DYNPD, 0);

    nrf24_write_register_single(nrf24, NRF_STATUS, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));
    nrf24_set_channel(nrf24, 76);
    nrf24_flush_rx(nrf24);
    nrf24_flush_tx(nrf24);
    nrf24_turn_on(nrf24);
    nrf24_write_register_single(nrf24, NRF_CONFIG, (nrf24_read_register(nrf24, NRF_CONFIG)) & ~(1 << PRIM_RX));

    return (setup != 0x00 && setup != 0xFF);
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

uint8_t
nrf24_write_payload(nrf24_t* nrf24, const void* buf, uint8_t len, const uint8_t write_type) {
    uint8_t status = 0;
    const uint8_t* data = (const uint8_t*)buf;
    uint8_t addr = write_type;

    len = nrf24_min(len, nrf24->payload_size);
    uint8_t blank_len = nrf24->payload_size - len;

    nrf24_csn(nrf24, 0);
    LL_SPI_TransmitData8(nrf24->spi, addr);
    status = LL_SPI_ReceiveData8(nrf24->spi);
    for (uint8_t i; i < len; i++) {
        LL_SPI_TransmitData8(nrf24->spi, data[i]);
    }

    while (blank_len--) {
        uint8_t zero = 0;
        LL_SPI_TransmitData8(nrf24->spi, zero);
    }
    nrf24_csn(nrf24, 1);

    return status;
}

uint8_t
nrf24_read_payload(nrf24_t* nrf24, void* buf, uint8_t len) {
    uint8_t status = 0;
    uint8_t* data = (uint8_t*)buf;

    if(len > nrf24->payload_size) {
        len = nrf24->payload_size;
    }

    uint8_t blank_len = nrf24->payload_size - len;

    uint8_t addr = R_RX_PAYLOAD;
    nrf24_csn(nrf24, 0);
    LL_SPI_TransmitData8(nrf24->spi, addr);
    for (uint8_t i; i < len; i++) {
        data[i] = LL_SPI_ReceiveData8(nrf24->spi);
    }

    while(blank_len--) {
        uint8_t zero = 0;
        zero = LL_SPI_ReceiveData8(nrf24->spi);
    }
    nrf24_csn(nrf24, 1);

    return status = 0;
}

uint8_t
nrf24_spi_tx_rx(nrf24_t* nrf24, uint8_t cmd) {
    uint8_t status = 0;
    
    nrf24_csn(nrf24, 0);
    LL_SPI_TransmitData8(nrf24->spi, cmd);
    status = LL_SPI_ReceiveData8(nrf24->spi);
    nrf24_csn(nrf24, 1);

    return status;
}

uint8_t
nrf24_flush_rx(nrf24_t* nrf24) {
    return nrf24_spi_tx_rx(nrf24, FLUSH_RX);
}

uint8_t
nrf24_flush_tx(nrf24_t* nrf24) {
    return nrf24_spi_tx_rx(nrf24, FLUSH_TX);
}

uint8_t
nrf24_get_status(nrf24_t* nrf24) {
    return nrf24_spi_tx_rx(nrf24, NOP);
}

uint8_t
nrf24_set_channel(nrf24_t* nrf24, uint8_t channel) {
    nrf24_write_register_single(nrf24, RF_CH, channel);
    return 1;
}

uint8_t
nrf24_get_channel(nrf24_t* nrf24) {
    return nrf24_read_register(nrf24, RF_CH);
}

uint8_t
nrf24_set_payload_size(nrf24_t* nrf24, uint8_t size) {
    nrf24->payload_size = nrf24_min(size, 32);
    return 1;
}

uint8_t
nrf24_get_payload_size(nrf24_t* nrf24) {
    return nrf24->payload_size;
}

uint8_t
nrf24_is_connected(nrf24_t* nrf24) {
    uint8_t setup = nrf24_read_register(nrf24, SETUP_AW);

    if (setup >= 1 && setup <= 3) {
        return 1;
    }

    return 0;
}

uint8_t
nrf24_turn_off(nrf24_t* nrf24) {
    nrf24_ce(nrf24, 0);
    nrf24_write_register_single(nrf24, NRF_CONFIG, nrf24_read_register(nrf24, NRF_CONFIG) & ~(1 << PWR_UP));
    
    return 1;
}

uint8_t
nrf24_turn_on(nrf24_t* nrf24) {
    int8_t cfg = nrf24_read_register(nrf24, NRF_CONFIG);
    // if not powered up then power up and wait for the radio to initialize
    if(!(cfg & (1 << PWR_UP))) {
        nrf24_write_register_single(nrf24, NRF_CONFIG, cfg | (1 << PWR_UP));
        delay_ms(5);
    }

    return 1;
}

uint8_t
nrf24_listening_start(nrf24_t* nrf24) {
    nrf24_turn_on(nrf24);

    nrf24_write_register_single(nrf24, NRF_CONFIG, nrf24_read_register(nrf24, NRF_CONFIG) | (1 << PRIM_RX));
    nrf24_write_register_single(nrf24, NRF_STATUS, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));
    nrf24_ce(nrf24, 1);
    // Restore the pipe0 adddress, if exists
    if (nrf24->pipe0_reading_address[0] > 0) {
        nrf24_write_register_multi(nrf24, RX_ADDR_P0, nrf24->pipe0_reading_address, nrf24->addr_width);
    } else {
        nrf24_close_reading_pipe(nrf24, 0);
    }

    if (nrf24_read_register(nrf24, FEATURE) & (1 << EN_ACK_PAY)) {
        nrf24_flush_tx(nrf24);
    }

    return 1;
}

uint8_t
nrf24_listening_stop(nrf24_t* nrf24) {
    nrf24_ce(nrf24, 0);
    delay_us(nrf24->tx_delay_us);

    if (nrf24_read_register(nrf24, FEATURE) & (1 << EN_ACK_PAY)) {
        delay_us(nrf24->tx_delay_us);
        nrf24_flush_tx(nrf24);
    }

    nrf24_write_register_single(nrf24, NRF_CONFIG, (nrf24_read_register(nrf24, NRF_CONFIG)) & ~(1 << PRIM_RX));
    nrf24_write_register_single(nrf24, EN_RXADDR, (nrf24_read_register(nrf24, EN_RXADDR)) | (1 << nrf24->child_pipe_enable[0]));
    
    return 1;
}