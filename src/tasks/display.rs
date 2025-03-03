use defmt::info;
use embassy_time::Delay;
use embassy_sync::channel::Receiver;
use embassy_sync::blocking_mutex::raw::NoopRawMutex;
use embassy_stm32::gpio::Output;
use embassy_stm32::spi::Spi;
use embassy_stm32::mode::Blocking;
use embedded_graphics::{
    prelude::*,
    pixelcolor::{Rgb565},
};
use crate::JoystickData;

#[embassy_executor::task]
pub async fn display_controller_task(
    spi: Spi<'static, Blocking>, 
    cs: Output<'static>,
    dc: Output<'static>,
    rst: Output<'static>,
    _bl: Output<'static>,
    joystick_rx: Receiver<'static, NoopRawMutex, JoystickData, 1>) {
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
        let data = joystick_rx.receive().await;
        info!("Display: data {} {} {} {}.", data.x1, data.y1, data.x2, data.y2);
    }
}

