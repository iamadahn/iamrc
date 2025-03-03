use defmt::*;
use embassy_time::Timer;
use embassy_sync::channel::Sender;
use embassy_sync::blocking_mutex::raw::NoopRawMutex;
use embassy_stm32::adc::{Adc, AnyAdcChannel};
use embassy_stm32::peripherals::ADC1;

pub struct JoystickData {
    pub x1: u16,
    pub y1: u16,
    pub x2: u16,
    pub y2: u16,
}

#[embassy_executor::task]
pub async fn joystick_controller_task(
    mut adc: Adc<'static, ADC1>,
    mut ch0: AnyAdcChannel<ADC1>,
    mut ch1: AnyAdcChannel<ADC1>,
    mut ch2: AnyAdcChannel<ADC1>,
    mut ch3: AnyAdcChannel<ADC1>,
    joystick_tx: Sender<'static, NoopRawMutex, JoystickData, 1>) {
    info!("Starting joystick controller task");
    loop {
        let data = JoystickData {
            x1: adc.blocking_read(&mut ch0),
            y1: adc.blocking_read(&mut ch1),
            x2: adc.blocking_read(&mut ch2),
            y2: adc.blocking_read(&mut ch3),
        };
        joystick_tx.send(data).await;
        Timer::after_millis(500).await;
    }
}

