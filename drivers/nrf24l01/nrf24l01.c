#include "nrf24l01.h"
#include "string.h"

#define DWT_CONTROL *(volatile unsigned long*)0xE0001000
#define SCB_DEMCR   *(volatile unsigned long*)0xE000EDFC

#define nrf24_max(a, b) (a>b?a:b)
#define nrf24_min(a, b) (a<b?a:b)

extern void delay_ms(uint32_t ms);

static void dwt_init(void);

static const uint8_t child_pipe[] = {RX_ADDR_P0, RX_ADDR_P1, RX_ADDR_P2, RX_ADDR_P3, RX_ADDR_P4, RX_ADDR_P5};
static const uint8_t child_payload_size[] = {RX_PW_P0, RX_PW_P1, RX_PW_P2, RX_PW_P3, RX_PW_P4, RX_PW_P5};
static const uint8_t child_pipe_enable[] = {ERX_P0, ERX_P1, ERX_P2, ERX_P3, ERX_P4, ERX_P5};

static void
nrf24_transmit_spi(SPI_TypeDef* spi, uint8_t data) {
    while (!LL_SPI_IsActiveFlag_TXE(spi)) {
        ;
    }
    LL_SPI_TransmitData8(spi, data);
}

static uint8_t
nrf24_receive_spi(SPI_TypeDef* spi) {
    while (!LL_SPI_IsActiveFlag_RXNE(spi)) {
        ;
    }
    return LL_SPI_ReceiveData8(spi);
}

uint8_t
nrf24_init(nrf24_t* nrf24) {
    uint8_t setup = 0;
    nrf24->payload_size = 32;
    nrf24->addr_width = NRF24_ADDR_WIDTH;
    nrf24->p_variant = 0;
    nrf24->pipe0_reading_address[0] = 0;

    dwt_init();

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

static void
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
    delay_ms(1);
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
    nrf24_transmit_spi(nrf24->spi, addr);
    data = nrf24_receive_spi(nrf24->spi);
    nrf24_transmit_spi(nrf24->spi, 0xFF);
    data = nrf24_receive_spi(nrf24->spi);
    nrf24_csn(nrf24, 1);

    return data;
}

uint8_t
nrf24_write_register_single(nrf24_t* nrf24, uint8_t reg, uint8_t value) {
    uint8_t status = 0;
    uint8_t addr = W_REGISTER | (REGISTER_MASK & reg);

    nrf24_csn(nrf24, 0);
    nrf24_transmit_spi(nrf24->spi, addr);
    status = nrf24_receive_spi(nrf24->spi);
    nrf24_transmit_spi(nrf24->spi, value);
    nrf24_csn(nrf24, 1);

    return status;
}

uint8_t
nrf24_write_register_multi(nrf24_t* nrf24, uint8_t reg, const uint8_t* buf, uint8_t len) {
    uint8_t status;
    uint8_t addr = W_REGISTER | (REGISTER_MASK & reg);

    uint8_t* data = (uint8_t*)buf;

    nrf24_csn(nrf24, 0);
    nrf24_transmit_spi(nrf24->spi, addr);
    status = nrf24_receive_spi(nrf24->spi);
    for (uint8_t i; i < len; i++) {
        nrf24_transmit_spi(nrf24->spi, data[i]);
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
    uint8_t blank_len = nrf24->dynamic_payloads_enabled ? 0 : nrf24->payload_size - len;

    nrf24_csn(nrf24, 0);
    nrf24_transmit_spi(nrf24->spi, addr);
    status = nrf24_receive_spi(nrf24->spi);
    for (uint8_t i; i < len; i++) {
        nrf24_transmit_spi(nrf24->spi, data[i]);
    }

    while (blank_len--) {
        uint8_t zero = 0;
        nrf24_transmit_spi(nrf24->spi, zero);
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

    uint8_t blank_len = nrf24->dynamic_payloads_enabled ? 0 : nrf24->payload_size - len;

    uint8_t addr = R_RX_PAYLOAD;
    nrf24_csn(nrf24, 0);
    nrf24_transmit_spi(nrf24->spi, addr);
    for (uint8_t i; i < len; i++) {
        data[i] = nrf24_receive_spi(nrf24->spi);
    }

    while(blank_len--) {
        uint8_t zero = 0;
        zero = nrf24_receive_spi(nrf24->spi);
    }
    nrf24_csn(nrf24, 1);

    return status = 0;
}

uint8_t
nrf24_spi_tx_rx(nrf24_t* nrf24, uint8_t cmd) {
    uint8_t status = 0;

    nrf24_csn(nrf24, 0);
    nrf24_transmit_spi(nrf24->spi, cmd);
    status = nrf24_receive_spi(nrf24->spi);
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
    nrf24_write_register_single(nrf24, EN_RXADDR, (nrf24_read_register(nrf24, EN_RXADDR)) | (1 << child_pipe_enable[0]));

    return 1;
}

uint8_t
nrf24_start_fast_write(nrf24_t* nrf24, const void* buf, uint8_t len, const uint8_t multicast, uint8_t start_tx) {
    nrf24_write_payload(nrf24, buf, len, multicast ? W_TX_PAYLOAD_NO_ACK : W_TX_PAYLOAD);

    if (start_tx) {
        nrf24_ce(nrf24, 1);
        return 1;
    }

    return 0;
}

uint8_t
nrf24_write(nrf24_t* nrf24, const void* buf, uint8_t len) {
    nrf24_start_fast_write (nrf24, buf, len, 1, 1);

    while (!(nrf24_get_status(nrf24) & ((1 << TX_DS) | (1 << MAX_RT)))) {
        ;
    }

    nrf24_ce(nrf24, 0);
    uint8_t status = nrf24_write_register_single(nrf24, NRF_STATUS, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));

    if (status & (1 << MAX_RT)) {
        nrf24_flush_tx(nrf24);
        return 0;
    }

    return 1;
}

uint8_t
nrf24_mask_irq(nrf24_t* nrf24, uint8_t tx, uint8_t fail, uint8_t rx) {
    uint8_t config = nrf24_read_register(nrf24, NRF_CONFIG);
    config &= ~(1 << MASK_MAX_RT | 1 << MASK_TX_DS | 1 << MASK_RX_DR);
    config |= fail << MASK_MAX_RT | tx << MASK_TX_DS | tx << MASK_RX_DR;
    uint8_t status = nrf24_write_register_single(nrf24, NRF_CONFIG, config);
    return status;
}

uint8_t
nrf24_get_dynamic_payload_size(nrf24_t* nrf24) {
    uint8_t result = 0, addr = R_RX_PL_WID;

    nrf24_csn(nrf24, 0);
    nrf24_transmit_spi(nrf24->spi, addr);
    result = nrf24_receive_spi(nrf24->spi);
    nrf24_transmit_spi(nrf24->spi, (uint8_t)0xFF);
    result = nrf24_receive_spi(nrf24->spi);
    nrf24_csn(nrf24, 1);

    if (result > 32) {
        nrf24_flush_rx(nrf24);
        delay_ms(2);
        return 0;
    }

    return result;
}

uint8_t
nrf24_is_available(nrf24_t* nrf24, uint8_t* pipe_num) {
    if (!(nrf24_read_register(nrf24, FIFO_STATUS) & (1 << RX_EMPTY))) {
        if (pipe_num) {
            uint8_t status = nrf24_get_status(nrf24);
            *pipe_num = (status >> RX_P_NO) & 0x07;
        }

        return 1;
    }

    return 0;
}

uint8_t
nrf24_is_available_null(nrf24_t* nrf24) {
    return nrf24_is_available(nrf24, NULL);
}

uint8_t
nrf24_read(nrf24_t* nrf24, void* buf, uint8_t len) {
    nrf24_read_payload(nrf24, buf, len);
    nrf24_write_register_single(nrf24, NRF_STATUS, (1 << RX_DR) | (1 << MAX_RT) | (1 << TX_DS));
    return 1;
}

uint8_t
nrf24_what_happened(nrf24_t* nrf24) {
    uint8_t status = nrf24_write_register_single(nrf24, NRF_STATUS, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));
    return status;
}

uint8_t
nrf24_open_writing_pipe(nrf24_t* nrf24, uint64_t value) {
    nrf24_write_register_multi(nrf24, RX_ADDR_P0, (uint8_t*)&value, nrf24->addr_width);
    nrf24_write_register_multi(nrf24, TX_ADDR, (uint8_t*)&value, nrf24->addr_width);
    nrf24_write_register_single(nrf24, RX_PW_P0, nrf24->payload_size);
    return 1;
}

uint8_t
nrf24_open_reading_pipe(nrf24_t* nrf24, uint8_t child, uint64_t address) {
    if (child == 0) {
        memcpy(nrf24->pipe0_reading_address, &address, nrf24->addr_width);
    }

    if (child > 6) {
        return 0;
    }

    if (child < 2) {
        nrf24_write_register_multi(nrf24, child_pipe[child], (const uint8_t*)&address, nrf24->addr_width);
    } else {
        nrf24_write_register_multi(nrf24, child_pipe[child], (const uint8_t*)&address, 1);
    }

    nrf24_write_register_single(nrf24, child_payload_size[child], nrf24->payload_size);
    nrf24_write_register_single(nrf24, EN_RXADDR, nrf24_read_register(nrf24, EN_RXADDR) | (1 << child_pipe_enable[child]));

    return 1;
}

uint8_t
nrf24_set_address_width(nrf24_t* nrf24, uint8_t a_width) {
    if (a_width -= 2) {
        nrf24_write_register_single(nrf24, SETUP_AW, a_width % 4);
        nrf24->addr_width = (a_width % 4) + 2;
    } else {
        nrf24_write_register_single(nrf24, SETUP_AW, 0);
        nrf24->addr_width = 2;
    }

    return 1;
}

uint8_t
nrf24_close_reading_pipe(nrf24_t* nrf24, uint8_t pipe) {
    nrf24_write_register_single(nrf24, EN_RXADDR, nrf24_read_register(nrf24, EN_RXADDR) & ~(1 <<child_pipe_enable[pipe]));

    return 1;
}

uint8_t
nrf24_toggle_features(nrf24_t* nrf24) {
    uint8_t addr = ACTIVATE;
    nrf24_csn(nrf24, 0);
    nrf24_transmit_spi(nrf24->spi, addr);
    nrf24_transmit_spi(nrf24->spi, (uint8_t)0x73);
    nrf24_csn(nrf24, 1);

    return 1;
}

uint8_t
nrf24_enable_dynamic_payloads(nrf24_t* nrf24) {
    nrf24_write_register_single(nrf24, FEATURE, nrf24_read_register(nrf24, FEATURE) | (1 << EN_DPL));
    nrf24_write_register_single(nrf24, DYNPD, nrf24_read_register(nrf24, DYNPD) | (1 << DPL_P1) | (1 << DPL_P0));
    nrf24->dynamic_payloads_enabled = 1;

    return 1;
}

uint8_t
nrf24_disable_dynamic_payloads(nrf24_t* nrf24) {
    nrf24_write_register_single(nrf24, FEATURE, 0);
    nrf24_write_register_single(nrf24, DYNPD, 0);
    nrf24->dynamic_payloads_enabled = 0;

    return 1;
}

uint8_t
nrf24_enable_ack_payload(nrf24_t* nrf24) {
    nrf24_write_register_single(nrf24, FEATURE, nrf24_read_register(nrf24, FEATURE) | (1 << EN_ACK_PAY) | (1 << EN_DPL));
	nrf24_write_register_single(nrf24, DYNPD, nrf24_read_register(nrf24, DYNPD) | (1 << DPL_P1) | (1 << DPL_P0));
	nrf24->dynamic_payloads_enabled = 1;

    return 1;
}

uint8_t
nrf24_enable_dynamic_ack(nrf24_t* nrf24) {
    nrf24_write_register_single(nrf24, FEATURE, nrf24_read_register(nrf24, FEATURE) | (1 << EN_DYN_ACK));
    return 1;
}

uint8_t
nrf24_write_ack_payload(nrf24_t* nrf24, uint8_t pipe, const void* buf, uint8_t len) {
    const uint8_t* data = (const uint8_t*)buf;

    uint8_t data_len = nrf24_min(len, 32);
    uint8_t addr = W_ACK_PAYLOAD | (pipe & 0x07);

    nrf24_csn(nrf24, 0);
    nrf24_transmit_spi(nrf24->spi, addr);
    for (uint8_t i = 0; i < data_len; i++) {
        nrf24_transmit_spi(nrf24->spi, data[i]);
    }
    nrf24_csn(nrf24, 1);

    return 1;
}

uint8_t
nrf24_is_ack_payload_available(nrf24_t* nrf24) {
    return !(nrf24_read_register(nrf24, FIFO_STATUS) & (1 << RX_EMPTY));
}

uint8_t
nrf24_is_p_variant(nrf24_t* nrf24) {
    return nrf24->p_variant;
}

uint8_t
nrf24_set_auto_ack(nrf24_t* nrf24, uint8_t enable) {
    if (enable) {
        nrf24_write_register_single(nrf24, EN_AA, 0x3F);
    } else {
        nrf24_write_register_single(nrf24, EN_AA, 0x00);
    }

    return 1;
}

uint8_t
nrf24_set_auto_ack_pipe(nrf24_t* nrf24, uint8_t pipe, uint8_t enable) {
    if (pipe < 0 | pipe > 6) {
        return 0;
    }

    uint8_t en_aa = nrf24_read_register(nrf24, EN_AA);

    if (enable) {
        en_aa |= (1 << pipe);
    } else {
        en_aa &= ~(1 << pipe);
    }

    nrf24_write_register_single(nrf24, EN_AA, en_aa);

    return 1;
}

uint8_t
nrf24_set_pa_level(nrf24_t* nrf24, uint8_t level) {
    uint8_t setup = nrf24_read_register(nrf24, RF_SETUP) & 0xF8;

    if (level > 3) {
        level = (RF24_PA_MAX << 1) + 1;
    } else {
        level = (level << 1) + 1;
    }

    nrf24_write_register_single(nrf24, RF_SETUP, setup |= level);

    return 1;
}

uint8_t
nrf24_get_pa_level(nrf24_t* nrf24) {
    return (nrf24_read_register(nrf24, RF_SETUP) & ((1 << RF_PWR_LOW) | (1 << RF_PWR_HIGH))) >> 1;
}

uint8_t
nrf24_set_data_rate(nrf24_t* nrf24, nrf24_datarate_e speed) {
    uint8_t result = 0;
	uint8_t setup = nrf24_read_register(nrf24, RF_SETUP);
	setup &= ~((1 << RF_DR_LOW) | (1 << RF_DR_HIGH));
	nrf24->tx_delay_us = 85;

	if (speed == NRF24_250KBPS) {
		setup |= (1 << RF_DR_LOW);
		nrf24->tx_delay_us = 155;
	} else {
		if (speed == NRF24_2MBPS) {
			setup |= (1 << RF_DR_HIGH);
			nrf24->tx_delay_us = 65;
		}
	}

	nrf24_write_register_single(nrf24, RF_SETUP, setup);
	uint8_t check = nrf24_read_register(nrf24, RF_SETUP);

	if (check == setup) {
		result = 1;
	}

	return result;
}

nrf24_datarate_e
nrf24_get_data_rate(nrf24_t* nrf24) {
	nrf24_datarate_e result;
	uint8_t dr = nrf24_read_register(nrf24, RF_SETUP) & ((1 << RF_DR_LOW) | (1 << RF_DR_HIGH));

	// switch uses RAM (evil!)
	// Order matters in our case below
	if (dr == (1 << RF_DR_LOW)) {
		result = NRF24_250KBPS;
	} else if (dr == (1 << RF_DR_HIGH)) {
		result = NRF24_2MBPS;
	} else {
		result = NRF24_1MBPS;
	}

	return result;
}

uint8_t
nrf24_set_crc_length(nrf24_t* nrf24, nrf24_crclength_e length) {
    uint8_t config = nrf24_read_register(nrf24, NRF_CONFIG) & ~((1 << CRCO) | (1 << EN_CRC));

	if (length == NRF24_CRC_DISABLED) {
		// Do nothing, we turned it off above.
	} else if (length == NRF24_CRC_8) {
		config |= (1 << EN_CRC);
	} else {
		config |= (1 << EN_CRC);
		config |= (1 << CRCO);
	}

	nrf24_write_register_single(nrf24, NRF_CONFIG, config);
}

nrf24_crclength_e
nrf24_get_crc_length(nrf24_t* nrf24) {
	nrf24_crclength_e result = NRF24_CRC_DISABLED;

	uint8_t config = nrf24_read_register(nrf24, NRF_CONFIG) & ((1 << CRCO) | (1 << EN_CRC));
	uint8_t AA = nrf24_read_register(nrf24, EN_AA);

	if (config & (1 << EN_CRC) || AA) {
		if(config & (1 << CRCO)) {
		  result = NRF24_CRC_16;
        } else {
		  result = NRF24_CRC_8;
        }
	}

	return result;
}

uint8_t
nrf24_disable_crc(nrf24_t* nrf24) {
	uint8_t disable = nrf24_read_register(nrf24, NRF_CONFIG) & ~(1 << EN_CRC);
	nrf24_write_register_single(nrf24, NRF_CONFIG, disable);

    return 1;
}

uint8_t
nrf24_set_retries(nrf24_t* nrf24, uint8_t delay, uint8_t count) {
	nrf24_write_register_single(nrf24, SETUP_RETR, (delay & 0xf)<<ARD | (count & 0xf)<<ARC);

    return 1;
}
