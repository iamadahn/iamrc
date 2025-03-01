#![no_std]
#![no_main]

use defmt::*;
use embassy_executor::Spawner;
use embassy_time::Timer;
use embassy_stm32::time::Hertz;
use embassy_stm32::gpio::{Level, Output, Speed};
use embassy_stm32::spi;
use {defmt_rtt as _, panic_probe as _};

mod tasks;
use tasks::led::led_controller_task as led_controller;
use tasks::rc::rc_controller_task as rc_controller;

#[embassy_executor::main]
async fn main(spawner: Spawner) -> ! {
    info!("Booting iamrc...");

    let mut config = embassy_stm32::Config::default();
    info!("Initialising clocks...");
    {
        use embassy_stm32::rcc::*;
        config.rcc.hse = Some(Hse {
            freq: Hertz(25_000_000),
            mode: HseMode::Oscillator,
        });
        config.rcc.pll_src = PllSource::HSE;
        config.rcc.pll = Some(Pll {
            divp: Some(PllPDiv::DIV2),
            //divq: Some(PllQDiv::DIV4),
            //divr: Some(PllRDiv::DIV2),
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

    info!("Initialising hardware...");
    let peripherals = embassy_stm32::init(config);
    let led = Output::new(peripherals.PC13, Level::High, Speed::Low);

    // PB10 - SPI3_SCK, PB14 - SPI3_MISO, PB15 - SPI3_MOSI
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

    spawner.spawn(led_controller(led)).ok();
    spawner.spawn(rc_controller(nrf_spi, nrf_ce, nrf_cns)).ok();

    loop {
        Timer::after_millis(500).await;
    }
}

