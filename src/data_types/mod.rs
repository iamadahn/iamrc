use embassy_stm32::adc::Resolution;
use embassy_stm32::adc::resolution_to_max_count;

#[derive(Clone, Debug)]
pub struct InputData {
    pub x1: u8,
    pub y1: u8,
    pub x2: u8,
    pub y2: u8,
}

pub trait Conversion {
    fn to_pct(&self, resolution: Resolution) -> u8;
}

/* self is always <= resolution_to_max_count(resolution) */
impl Conversion for u16 {
    fn to_pct(&self, resolution: Resolution) -> u8 {
        (*self as u32 * 100 / resolution_to_max_count(resolution) as u32).try_into().unwrap()
    }
}

