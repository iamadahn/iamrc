#![no_std]
#![no_main]

use defmt::*;
use embassy_executor::Spawner;
use embassy_time::Timer;
use embassy_stm32::time::Hertz;
use embassy_stm32::gpio::{Level, Output, Speed};
use {defmt_rtt as _, panic_probe as _};

#[embassy_executor::main]
async fn main(_spawner: Spawner) {
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
    let mut led = Output::new(peripherals.PC13, Level::High, Speed::Low);

    info!("Entering main loop..");
    loop {
        led.set_high();
        info!("Led set to high.");
        Timer::after_millis(500).await;

        led.set_low();
        info!("Led set to low.");
        Timer::after_millis(500).await;
    }
}

