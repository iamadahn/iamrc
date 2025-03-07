use defmt::info;
use core::fmt::Write;
use heapless::String;
use embassy_time::Delay;
use embassy_sync::pubsub::Subscriber;
use embassy_sync::blocking_mutex::raw::NoopRawMutex;
use embassy_stm32::gpio::Output;
use embassy_stm32::spi::Spi;
use embassy_stm32::mode::Async;
use embedded_graphics::{
    prelude::*,
    pixelcolor::{Rgb565},
    primitives::{
        Rectangle,
        Line,
        PrimitiveStyle,
        PrimitiveStyleBuilder,
    },
    text::Text,
    mono_font::{ascii, MonoTextStyle},
    draw_target::DrawTarget,
};
use embedded_graphics_framebuf::FrameBuf;
use crate::data_types::InputData;

const SCR_WIDTH: u32 = 160;
const SCR_HEIGHT: u32 = 128;
const BACKGROUND_COLOR: Rgb565 = Rgb565::new(0xFF, 0x18, 0x00);
const FONT_COLOR: Rgb565 = Rgb565::BLACK;

const LOGO: &str = r#"
 _____          __  __ _____   _____  
|_   _|   /\   |  \/  |  __ \ / ____| 
  | |    /  \  | \  / | |__) | |      
  | |   / /\ \ | |\/| |  _  /| |      
 _| |_ / ____ \| |  | | | \ \| |____  
|_____/_/    \_\_|  |_|_|  \_\\_____| 
"#;

const CAR: &str = r#"
    _-_-  _/\______\\__
 _-_-__  / ,-. -|-  ,-.`-.
    _-_- `( o )----( o )-'
           `-'      `-'
"#;

#[embassy_executor::task]
pub async fn display_controller_task(
    spi: Spi<'static, Async>, 
    cs: Output<'static>,
    dc: Output<'static>,
    rst: Output<'static>,
    _bl: Output<'static>,
    mut input_sub: Subscriber<'static, NoopRawMutex, InputData, 1, 2, 1>) {
    info!("Starting display controller task");
    let display_spi_device = embedded_hal_bus::spi::ExclusiveDevice::new_no_delay(spi, cs).unwrap();
    let mut display_device = st7735_lcd::ST7735::new(
        display_spi_device,
        dc,
        rst,
        true,
        false,
        SCR_WIDTH + 2,
        SCR_HEIGHT + 2
    );

    display_device.init(&mut Delay).unwrap();
    display_device.set_orientation(&st7735_lcd::Orientation::Landscape).unwrap();
    display_device.clear(BACKGROUND_COLOR).unwrap();

    let style = PrimitiveStyleBuilder::new()
        .stroke_color(FONT_COLOR)
        .stroke_width(2)
        .fill_color(BACKGROUND_COLOR)
        .build();

    Rectangle::new(Point::new(4, 5), Size::new(SCR_WIDTH - 6, SCR_HEIGHT - 6))
        .into_styled(style)
        .draw(&mut display_device)
        .unwrap();

    Line::new(Point::new(5, 67), Point::new(156, 67))
        .into_styled(PrimitiveStyle::with_stroke(FONT_COLOR, 2))
        .draw(&mut display_device)
        .unwrap();
    
    Line::new(Point::new(80, 67), Point::new(80, 126))
        .into_styled(PrimitiveStyle::with_stroke(FONT_COLOR, 2))
        .draw(&mut display_device)
        .unwrap();
    
    let style = MonoTextStyle::new(&ascii::FONT_4X6, FONT_COLOR);
    Text::new(LOGO, Point::new(7, 2), style).draw(&mut display_device).unwrap();

    let style = MonoTextStyle::new(&ascii::FONT_4X6, FONT_COLOR);
    Text::new(CAR, Point::new(20, 41), style).draw(&mut display_device).unwrap();

    let style = MonoTextStyle::new(&ascii::FONT_6X13, FONT_COLOR);
    Text::new("Left stick", Point::new(7, 78), style).draw(&mut display_device).unwrap();

    let style = MonoTextStyle::new(&ascii::FONT_6X13, FONT_COLOR);
    Text::new("Right stick", Point::new(84, 78), style).draw(&mut display_device).unwrap();

    Line::new(Point::new(5, 81), Point::new(156, 81))
        .into_styled(PrimitiveStyle::with_stroke(FONT_COLOR, 1))
        .draw(&mut display_device)
        .unwrap();

    let style = MonoTextStyle::new(&ascii::FONT_9X15, FONT_COLOR);
    Text::new("X:", Point::new(6, 93), style).draw(&mut display_device).unwrap();

    let style = MonoTextStyle::new(&ascii::FONT_9X15, FONT_COLOR);
    Text::new("Y:", Point::new(6, 107), style).draw(&mut display_device).unwrap();

    let style = MonoTextStyle::new(&ascii::FONT_9X15, FONT_COLOR);
    Text::new("SW:", Point::new(6, 121), style).draw(&mut display_device).unwrap();

    let style = MonoTextStyle::new(&ascii::FONT_9X15, FONT_COLOR);
    Text::new("X:", Point::new(83, 93), style).draw(&mut display_device).unwrap();
    
    let style = MonoTextStyle::new(&ascii::FONT_9X15, FONT_COLOR);
    Text::new("Y:", Point::new(83, 107), style).draw(&mut display_device).unwrap();

    let style = MonoTextStyle::new(&ascii::FONT_9X15, FONT_COLOR);
    Text::new("SW:", Point::new(83, 121), style).draw(&mut display_device).unwrap();

    loop {
        let data = input_sub.next_message_pure().await;

        let mut fdata = [BACKGROUND_COLOR; 45 * 10];
        let mut fbuf = FrameBuf::new(&mut fdata, 45, 10);
        let mut data_str = String::<32>::new();
        let _ = write!(data_str, "{}", data.x1);
        let style = MonoTextStyle::new(&ascii::FONT_9X15, FONT_COLOR);
        Text::new(&data_str, Point::new(1, 9), style).draw(&mut fbuf).unwrap();
        let area = Rectangle::new(Point::new(24, 84), fbuf.size());
        display_device.fill_contiguous(&area, fdata).unwrap();

        let mut fdata = [BACKGROUND_COLOR; 45 * 10];
        let mut fbuf = FrameBuf::new(&mut fdata, 45, 10);
        let mut data_str = String::<32>::new();
        let _ = write!(data_str, "{}", data.y1);
        let style = MonoTextStyle::new(&ascii::FONT_9X15, FONT_COLOR);
        Text::new(&data_str, Point::new(1, 9), style).draw(&mut fbuf).unwrap();
        let area = Rectangle::new(Point::new(24, 98), fbuf.size());
        display_device.fill_contiguous(&area, fdata).unwrap();

        let mut fdata = [BACKGROUND_COLOR; 45 * 10];
        let mut fbuf = FrameBuf::new(&mut fdata, 45, 10);
        let mut data_str = String::<32>::new();
        let _ = write!(data_str, "{}", data.x2);
        let style = MonoTextStyle::new(&ascii::FONT_9X15, FONT_COLOR);
        Text::new(&data_str, Point::new(1, 9), style).draw(&mut fbuf).unwrap();
        let area = Rectangle::new(Point::new(101, 84), fbuf.size());
        display_device.fill_contiguous(&area, fdata).unwrap();

        let mut fdata = [BACKGROUND_COLOR; 45 * 10];
        let mut fbuf = FrameBuf::new(&mut fdata, 45, 10);
        let mut data_str = String::<32>::new();
        let _ = write!(data_str, "{}", data.y2);
        let style = MonoTextStyle::new(&ascii::FONT_9X15, FONT_COLOR);
        Text::new(&data_str, Point::new(1, 9), style).draw(&mut fbuf).unwrap();
        let area = Rectangle::new(Point::new(101, 98), fbuf.size());
        display_device.fill_contiguous(&area, fdata).unwrap();

        info!("Display: data {} {} {} {}.", data.x1, data.y1, data.x2, data.y2);
    }
}

