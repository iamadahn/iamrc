#include "adc.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_adc.h"
#include "stm32f1xx_ll_bus.h"

void 
adc_config(adc_t* adc) {
    LL_ADC_InitTypeDef ADC_InitStruct = {0};
    LL_ADC_CommonInitTypeDef ADC_CommonInitStruct = {0};
    LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};

    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (adc->adc_base == ADC1) {
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC1);
    } else {
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC2);
    }

    if (adc->port == GPIOA) {
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
    } else if (adc->port == GPIOB) {
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
    } else if (adc->port == GPIOC) {
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOC);
    } else if (adc->port == GPIOD) {
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOD);
    } else if (adc->port == GPIOE) {
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOE);
    }

    GPIO_InitStruct.Pin = adc->pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
    LL_GPIO_Init(adc->port, &GPIO_InitStruct);

    ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
    ADC_InitStruct.SequencersScanMode = LL_ADC_SEQ_SCAN_DISABLE;
    LL_ADC_Init(adc->adc_base, &ADC_InitStruct);

    ADC_CommonInitStruct.Multimode = LL_ADC_MULTI_INDEPENDENT;
    LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(adc->adc_base), &ADC_CommonInitStruct);

    ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
    ADC_REG_InitStruct.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE;
    ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
    ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
    ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_NONE;
    LL_ADC_REG_Init(adc->adc_base, &ADC_REG_InitStruct);

    LL_ADC_REG_SetSequencerRanks(adc->adc_base, LL_ADC_REG_RANK_1, adc->channel);
    LL_ADC_SetChannelSamplingTime(adc->adc_base, adc->channel, LL_ADC_SAMPLINGTIME_13CYCLES_5);

}