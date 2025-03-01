use defmt::*;
use embassy_time::Timer;
use embassy_stm32::adc::{Adc, AnyAdcChannel};
use embassy_stm32::peripherals::ADC1;

#[embassy_executor::task]
pub async fn adc_controller_task(
    mut adc: Adc<'static, ADC1>,
    mut ch0: AnyAdcChannel<ADC1>,
    mut ch1: AnyAdcChannel<ADC1>,
    mut ch2: AnyAdcChannel<ADC1>,
    mut ch3: AnyAdcChannel<ADC1>) {
    info!("Starting adc controller task");
    loop {
        let ch0_v = adc.blocking_read(&mut ch0);
        let ch1_v = adc.blocking_read(&mut ch1);
        let ch2_v = adc.blocking_read(&mut ch2);
        let ch3_v = adc.blocking_read(&mut ch3);
        info!("Adc controller: {} {} {} {}", ch0_v, ch1_v, ch2_v, ch3_v);
        Timer::after_millis(500).await;
    }
}

