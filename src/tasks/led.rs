use defmt::*;
use embassy_stm32::gpio::Output;
use embassy_time::Timer;

#[embassy_executor::task]
pub async fn led_controller_task(mut led: Output<'static>) {
    info!("Starting led heartbeat task");
    loop {
        led.toggle();
        info!("Led controller: toggled led");
        Timer::after_millis(500).await;
    }
}

