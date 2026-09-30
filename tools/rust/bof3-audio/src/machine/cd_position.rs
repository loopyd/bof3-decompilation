//! CD absolute BCD minute/second/sector positions and signed logical sectors.
//! The lead-in is 150 sectors. This conversion does not model drive timing.

use crate::Result;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct CdPosition {
    bcd: [u8; 3],
}

impl CdPosition {
    pub const MIN_LBA: i32 = -150;
    pub const MAX_LBA: i32 = 100 * 60 * 75 - 151;

    pub fn from_lba(lba: i32) -> Result<Self> {
        if !(Self::MIN_LBA..=Self::MAX_LBA).contains(&lba) {
            return Err(
                format!("CD LBA {lba} cannot be represented by a two-digit BCD position").into(),
            );
        }
        let absolute = lba + 150;
        let encode = |value: i32| (((value / 10) << 4) | (value % 10)) as u8;
        Ok(Self {
            bcd: [
                encode(absolute / 4500),
                encode(absolute / 75 % 60),
                encode(absolute % 75),
            ],
        })
    }

    pub fn from_bcd(bcd: [u8; 3]) -> Result<Self> {
        if bcd.iter().any(|value| value & 15 > 9 || value >> 4 > 9)
            || bcd[1] >= 0x60
            || bcd[2] >= 0x75
        {
            return Err(format!("invalid CD BCD minute/second/sector position {bcd:02x?}").into());
        }
        Ok(Self { bcd })
    }

    pub fn bcd(self) -> [u8; 3] {
        self.bcd
    }

    pub fn lba(self) -> i32 {
        let decode = |value: u8| i32::from(value >> 4) * 10 + i32::from(value & 15);
        decode(self.bcd[0]) * 4500 + decode(self.bcd[1]) * 75 + decode(self.bcd[2]) - 150
    }
}
