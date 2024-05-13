#include "analog_stick_controller.h"
#include "analog_stick.h"
#include "stm32f1xx_ll_adc.h"
#include "stm32f1xx_ll_gpio.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "queue.h"
#include "u_rtos.h"

void
analog_stick_controller_handler(void) {
    analog_stick_unit_t analoag_stick_y = {
        .adc_base = ADC1,
        .port = GPIOA,
        .pin = LL_GPIO_PIN_0,
        .channel = LL_ADC_CHANNEL_0,
    };

    analog_stick_unit_t analoag_stick_x = {
        .adc_base = ADC2,
        .port = GPIOA,
        .pin = LL_GPIO_PIN_1,
        .channel = LL_ADC_CHANNEL_1,
    };

    analog_stick_data_t analog_stick_data;
    
    LL_ADC_Enable(analoag_stick_x.adc_base);
    LL_ADC_Enable(analoag_stick_y.adc_base);
    uint32_t delay = ((LL_ADC_DELAY_ENABLE_CALIB_ADC_CYCLES * 32) >> 1);
    while (delay != 0) {
        delay--;
    }

    LL_ADC_StartCalibration(analoag_stick_x.adc_base);
    LL_ADC_StartCalibration(analoag_stick_y.adc_base);
    while ((LL_ADC_IsCalibrationOnGoing(analoag_stick_x.adc_base) != 0) & ((LL_ADC_IsCalibrationOnGoing(analoag_stick_y.adc_base) != 0))) {
        ;
    }

    while (1) {
        LL_ADC_REG_StartConversionSWStart(analoag_stick_x.adc_base);
        LL_ADC_REG_StartConversionSWStart(analoag_stick_y.adc_base);
        while (!LL_ADC_IsActiveFlag_EOS(analoag_stick_x.adc_base) & !LL_ADC_IsActiveFlag_EOS(analoag_stick_y.adc_base)) {
            ;
        }
        analog_stick_data.x = LL_ADC_REG_ReadConversionData12(analoag_stick_x.adc_base);
        analog_stick_data.y = LL_ADC_REG_ReadConversionData12(analoag_stick_y.adc_base);
        xQueueOverwrite(analog_stick_queue, &analog_stick_data);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}