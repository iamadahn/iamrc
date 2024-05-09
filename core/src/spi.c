#include "spi.h"
#include "stm32f1xx_ll_spi.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_bus.h"

void 
spi_config(spi_t* spi) {
    /* Enable the peripheral clock of chosen SPI's gpio port */
    if (spi->spi_base == SPI1) {
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
    } else if (spi->spi_base == SPI2) {
        LL_APB1_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
    }

    /* Configure SPI clock pin */
    LL_GPIO_SetPinMode(spi->sck_port, spi->sck_pin, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinSpeed(spi->sck_port, spi->sck_pin, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinPull(spi->sck_port, spi->sck_pin, LL_GPIO_PULL_DOWN);

    /* Configure SPI MISO pin */
    LL_GPIO_SetPinMode(spi->miso_port, spi->miso_pin, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinSpeed(spi->miso_port, spi->miso_pin, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinPull(spi->miso_port, spi->miso_pin, LL_GPIO_PULL_DOWN);

    /* Configure SPI MOSI pin */
    LL_GPIO_SetPinMode(spi->mosi_port, spi->mosi_pin, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinSpeed(spi->mosi_port, spi->mosi_pin, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinPull(spi->mosi_port, spi->mosi_pin, LL_GPIO_PULL_DOWN);

    /* Configure SPI interrupt */
    NVIC_SetPriority(spi->spi_irq, 0);
    NVIC_EnableIRQ(spi->spi_irq);

    /* Enable the peripheral clock of chosen SPI itself */
    if (spi->spi_base == SPI1) {
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
    } else if (spi->spi_base == SPI2) {
        LL_APB1_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
    }

    /* Configure SPI itself */
    LL_SPI_SetBaudRatePrescaler(spi->spi_base, LL_SPI_BAUDRATEPRESCALER_DIV16);
    LL_SPI_SetTransferDirection(spi->spi_base, LL_SPI_FULL_DUPLEX);
    LL_SPI_SetClockPhase(spi->spi_base, LL_SPI_PHASE_1EDGE);
    LL_SPI_SetClockPolarity(spi->spi_base, LL_SPI_POLARITY_LOW);
    LL_SPI_SetTransferBitOrder(spi->spi_base, LL_SPI_MSB_FIRST);
    LL_SPI_SetDataWidth(spi->spi_base, LL_SPI_DATAWIDTH_8BIT);
    LL_SPI_SetNSSMode(spi->spi_base, LL_SPI_NSS_SOFT);
    LL_SPI_SetMode(spi->spi_base, LL_SPI_MODE_MASTER);

    /* Configure SPI transfer interrupts */
    /* Enable RXNE (rx not empty) interrupt */
    LL_SPI_EnableIT_RXNE(spi->spi_base);

    /* Enable TXE (tx empy) interrupt */
    LL_SPI_EnableIT_TXE(spi->spi_base);

    /* Enable ERR (error) interrupt */
    LL_SPI_EnableIT_ERR(spi->spi_base);

    /* Enable configured SPI */
    LL_SPI_Enable(spi->spi_base);
}