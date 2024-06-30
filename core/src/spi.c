#include "spi.h"
#include "stm32f1xx_ll_spi.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_bus.h"

void 
spi_config(spi_t* spi) {
    /* Enable the peripheral clock of chosen SPI's gpio port */

    LL_SPI_InitTypeDef spi_init = {0};

    LL_GPIO_InitTypeDef gpio_init = {0};

    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);

    gpio_init.Pin = spi->sck_pin | spi->mosi_pin | spi->miso_pin;
    gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(spi->mosi_port, &gpio_init);

    spi_init.TransferDirection = LL_SPI_FULL_DUPLEX;
    spi_init.Mode = LL_SPI_MODE_MASTER;
    spi_init.DataWidth = LL_SPI_DATAWIDTH_8BIT;
    spi_init.ClockPolarity = LL_SPI_POLARITY_HIGH;
    spi_init.ClockPhase = LL_SPI_PHASE_1EDGE;
    spi_init.NSS = LL_SPI_NSS_SOFT;
    spi_init.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV16;
    spi_init.BitOrder = LL_SPI_MSB_FIRST;
    spi_init.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
    spi_init.CRCPoly = 10;

    LL_SPI_Init(spi->spi_base, &spi_init);

    LL_SPI_EnableIT_RXNE(spi->spi_base);
    LL_SPI_EnableIT_TXE(spi->spi_base);
    LL_SPI_EnableIT_ERR(spi->spi_base);

    LL_SPI_Enable(spi->spi_base);
}