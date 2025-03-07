use defmt::*;
use embassy_sync::pubsub::Subscriber;
use embassy_sync::blocking_mutex::raw::NoopRawMutex;
use embassy_stm32::gpio::Output;
use embassy_stm32::spi::Spi;
use embassy_stm32::mode::Blocking;
use embedded_nrf24l01::*;
use crate::data_types::InputData;

const PAYLOAD_LENGTH: usize = 10;

#[embassy_executor::task]
pub async fn rc_controller_task(
    spi: Spi<'static, Blocking>,
    ce: Output<'static>,
    cns: Output<'static>,
    mut input_sub: Subscriber<'static, NoopRawMutex, InputData, 1, 2, 1>) {
    info!("Starting remote controller task");
    let mut nrf = NRF24L01::new(ce, cns, spi).unwrap();
    nrf.set_frequency(8).unwrap();
    nrf.set_auto_retransmit(15, 15).unwrap();
    nrf.set_rf(&DataRate::R2Mbps, 0).unwrap();
    nrf.set_pipes_rx_enable(&[true, false, false, false, false, false]).unwrap();
    nrf.set_auto_ack(&[true, false, false, false, false, false]).unwrap();
    nrf.set_pipes_rx_lengths(&[None; 6]).unwrap();
    nrf.set_crc(CrcMode::TwoBytes).unwrap();
    nrf.set_rx_addr(0, &b"fnord"[..]).unwrap();
    nrf.set_tx_addr(&b"fnord"[..]).unwrap();
    nrf.flush_rx().unwrap();
    nrf.flush_tx().unwrap();

    let mut nrf = nrf.tx().unwrap();

    loop {
        let input = input_sub.next_message_pure().await;
        let payload = payload_construct(input);
        _ = nrf.send(&payload);
        _ = nrf.poll_send();
        info!("Rc controller: sent {} bytes", payload);
    }
}

fn payload_construct(input: InputData) -> [u8; PAYLOAD_LENGTH] {
    let mut bytes: [u8; PAYLOAD_LENGTH] = [0x00; PAYLOAD_LENGTH];
    bytes[0] = 0xDE;
    bytes[1] = 0xAD;
    bytes[2] = 0xBA;
    bytes[3] = 0xBE;
    bytes[4] = input.x1;
    bytes[5] = input.y1;
    bytes[6] = input.x2;
    bytes[7] = input.y2;
    checksum_add(&mut bytes);
    bytes
}

fn checksum_add(payload: &mut [u8; PAYLOAD_LENGTH]) {
    let mut ck_a: u8 = 0;
    let mut ck_b: u8 = 0;
    for byte in 0..(PAYLOAD_LENGTH - 2) {
        ck_a = ck_a.wrapping_add(payload[byte]);
        ck_b = ck_b.wrapping_add(ck_a);
    }
    payload[PAYLOAD_LENGTH - 2] = ck_a;
    payload[PAYLOAD_LENGTH - 1] = ck_b;
}

