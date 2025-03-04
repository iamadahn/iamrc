use defmt::info;
use core::fmt::Write;
use heapless::String;
use embassy_time::Delay;
use embassy_sync::channel::Receiver;
use embassy_sync::blocking_mutex::raw::NoopRawMutex;
use embassy_stm32::gpio::Output;
use embassy_stm32::spi::Spi;
use embassy_stm32::mode::Async;
use embedded_graphics::{
    prelude::*,
    pixelcolor::{Rgb565},
    primitives::Rectangle,
    text::Text,
    mono_font::{ascii::FONT_6X10, MonoTextStyle},
};
use embedded_graphics_framebuf::FrameBuf;
use crate::JoystickData;

#[embassy_executor::task]
pub async fn display_controller_task(
    spi: Spi<'static, Async>, 
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
        let mut fdata = [Rgb565::BLACK; 160 * 10];
        let mut fbuf = FrameBuf::new(&mut fdata, 160, 10);
        let mut data_str = String::<32>::new();
        let _ = write!(data_str, "data {} {} {} {}", data.x1, data.y1, data.x2, data.y2);
        let style = MonoTextStyle::new(&FONT_6X10, Rgb565::WHITE);
        Text::new(&data_str, Point::new(1, 9), style).draw(&mut fbuf).unwrap();
        let area = Rectangle::new(Point::new(0, 0), fbuf.size());
        display_device.fill_contiguous(&area, fdata).unwrap();
        info!("Display: data {} {} {} {}.", data.x1, data.y1, data.x2, data.y2);
    }
}

