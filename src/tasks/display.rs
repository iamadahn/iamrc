use defmt::*;
use embassy_stm32::gpio::Output;
use embassy_stm32::spi::Spi;
use embassy_stm32::mode::Blocking;
use embassy_time::{Timer, Delay};
use embedded_graphics::{
    pixelcolor::{Rgb565},
    prelude::*
};

#[embassy_executor::task]
pub async fn display_controller_task(
    spi: Spi<'static, Blocking>, 
    cs: Output<'static>,
    dc: Output<'static>,
    rst: Output<'static>,
    _bl: Output<'static>) {
    info!("Starting display controller task");
    let display_spi_device = embedded_hal_bus::spi::ExclusiveDevice::new_no_delay(spi, cs).unwrap();
    let mut display_device = st7735_lcd::ST7735::new(
        display_spi_device,
        dc,
        rst,
        true,
        false,
        162,
        130
    );

    display_device.init(&mut Delay).unwrap();
    display_device.set_orientation(&st7735_lcd::Orientation::Landscape).unwrap();
    display_device.clear(Rgb565::BLACK).unwrap();

    loop {
        display_device.clear(Rgb565::RED).unwrap();
        Timer::after_millis(300).await;
        display_device.clear(Rgb565::GREEN).unwrap();
        Timer::after_millis(300).await;
        display_device.clear(Rgb565::BLUE).unwrap();
        Timer::after_millis(300).await;
    }
}

