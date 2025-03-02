use defmt::*;
use embassy_stm32::gpio::Output;
use embassy_stm32::spi::Spi;
use embassy_stm32::mode::Blocking;
use embassy_time::Timer;
use embedded_nrf24l01::*;

#[embassy_executor::task]
pub async fn rc_controller_task(spi: Spi<'static, Blocking>, ce: Output<'static>, cns: Output<'static>) {
    info!("Starting remote controller task");
    let mut nrf = NRF24L01::new(ce, cns, spi).unwrap();
    nrf.set_frequency(8).unwrap();
    nrf.set_auto_retransmit(15, 15).unwrap();
    nrf.set_rf(&DataRate::R2Mbps, 0).unwrap();
    nrf
        .set_pipes_rx_enable(&[true, false, false, false, false, false])
        .unwrap();
    nrf
        .set_auto_ack(&[true, false, false, false, false, false])
        .unwrap();
    nrf.set_pipes_rx_lengths(&[None; 6]).unwrap();
    nrf.set_crc(CrcMode::TwoBytes).unwrap();
    nrf.set_rx_addr(0, &b"fnord"[..]).unwrap();
    nrf.set_tx_addr(&b"fnord"[..]).unwrap();
    nrf.flush_rx().unwrap();
    nrf.flush_tx().unwrap();

    let mut nrf = nrf.tx().unwrap();
    let mut bytes: [u8; 5] = [0x11, 0x22, 0x33, 0x44, 0x55];

    loop {
        for i in 0..bytes.len() {
            if bytes[i] == 255 {
                bytes[i] = 0;
            } else {
                bytes[i] += 1;
            }
        }

        _ = nrf.send(&bytes);
        _ = nrf.poll_send();
        info!("Rc controller: sent {} bytes", bytes);
        Timer::after_millis(500).await;
    }
}

