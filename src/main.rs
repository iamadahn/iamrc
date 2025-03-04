#![no_std]
#![no_main]

use static_cell::StaticCell;
use defmt::*;
use embassy_executor::Spawner;
use embassy_time::Timer;
use embassy_sync::channel::Channel;
use embassy_sync::blocking_mutex::raw::NoopRawMutex;
use embassy_stm32::time::Hertz;
use embassy_stm32::gpio::{Level, Output, Speed};
use embassy_stm32::adc::{Adc, AdcChannel};
use embassy_stm32::spi;
use {defmt_rtt as _, panic_probe as _};

mod tasks;
use tasks::led::led_controller_task as led_controller;
use tasks::rc::rc_controller_task as rc_controller;
use tasks::joystick::joystick_controller_task as joystick_controller;
use tasks::display::display_controller_task as display_controller;

use tasks::joystick::JoystickData;

static JOYSTICK_CHANNEL: StaticCell<Channel<NoopRawMutex, JoystickData, 1>> = StaticCell::new();

#[embassy_executor::main]
async fn main(spawner: Spawner) -> ! {
    info!("Booting iamrc...");

    info!("Initialising clocks...");
    let mut config = embassy_stm32::Config::default();
    {
        use embassy_stm32::rcc::*;
        config.rcc.hse = Some(Hse {
            freq: Hertz(25_000_000),
            mode: HseMode::Oscillator,
        });
        config.rcc.pll_src = PllSource::HSE;
        config.rcc.pll = Some(Pll {
            divp: Some(PllPDiv::DIV2),
            divq: None,
            divr: None,
            prediv: PllPreDiv::DIV12,
            mul: PllMul::MUL96,
        });
        config.rcc.sys = Sysclk::PLL1_P;
        config.rcc.ahb_pre = AHBPrescaler::DIV1;
        config.rcc.apb1_pre = APBPrescaler::DIV2;
        config.rcc.apb2_pre = APBPrescaler::DIV1;
    }

    info!("Initialising heartbeat led on PC13.");
    let peripherals = embassy_stm32::init(config);
    let led = Output::new(peripherals.PC13, Level::High, Speed::Low);

    // PB10 - SPI2_SCK, PB14 - SPI2_MISO, PB15 - SPI2_MOSI
    info!("Initialising SPI2 for communication with nrf24.");
    let mut nrf_spi_config = spi::Config::default();
    nrf_spi_config.frequency = Hertz(1_000_000);

    let nrf_spi = spi::Spi::new_blocking(
        peripherals.SPI2,
        peripherals.PB10,
        peripherals.PB15,
        peripherals.PB14,
        nrf_spi_config
    ); 
    let nrf_ce = Output::new(peripherals.PB13, Level::High, Speed::High);
    let nrf_cns = Output::new(peripherals.PA8, Level::High, Speed::High);

    // PB12 - SPI3_SCK, PB4 - SPI3_MISO, PB5 - SPI3_MOSI
    info!("Initialising SPI3 for working with display.");
    let mut disp_spi_config = spi::Config::default();
    disp_spi_config.frequency = Hertz(40_000_000);

    let disp_spi = spi::Spi::new(
        peripherals.SPI3,
        peripherals.PB12,
        peripherals.PB5,
        peripherals.PB4,
        peripherals.DMA1_CH5,
        peripherals.DMA1_CH2,
        disp_spi_config
    );

    let disp_cs = Output::new(peripherals.PB3, Level::Low, Speed::High);
    let disp_dc = Output::new(peripherals.PB6, Level::Low, Speed::High);
    let disp_rst = Output::new(peripherals.PB7, Level::Low, Speed::High);
    let disp_bl = Output::new(peripherals.PA15, Level::Low, Speed::Low);
    
    info!("Initialising ADC1 for reading input from joysticks.");
    let adc = Adc::new(peripherals.ADC1);
    let adc_ch0 = peripherals.PA0.degrade_adc();
    let adc_ch1 = peripherals.PA1.degrade_adc();
    let adc_ch2 = peripherals.PA2.degrade_adc();
    let adc_ch3 = peripherals.PA3.degrade_adc();

    let joystick_ch: &'static mut _ = JOYSTICK_CHANNEL.init(Channel::new());
    let joy_tx = joystick_ch.sender();
    let disp_joy_rx = joystick_ch.receiver();
    let nrf_joy_rx = joystick_ch.receiver();

    info!("Spawning tasks.");
    spawner.spawn(led_controller(led)).ok();
    spawner.spawn(joystick_controller(adc, adc_ch0, adc_ch1, adc_ch2, adc_ch3, joy_tx)).ok();
    spawner.spawn(rc_controller(nrf_spi, nrf_ce, nrf_cns, nrf_joy_rx)).ok();
    spawner.spawn(display_controller(disp_spi, disp_cs, disp_dc, disp_rst, disp_bl, disp_joy_rx)).ok();

    loop {
        Timer::after_millis(1000).await;
    }
}

