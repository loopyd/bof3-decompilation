//! Boot memory-controller registers for the existing retail ROM/RAM mapping.
//! Timing values are retained, not converted to CPU/device clocks here.
//! https://psx-spx.consoledev.net/memorycontrol/

use super::{error, Interconnect};
use crate::machine::bus::{BusError, Width};

#[derive(Default)]
pub(super) struct Control {
    expansion_base: [u32; 2],
    expansion_delay: [u32; 3],
    bios: u32,
    dram: u32,
}

impl Control {
    /// Reference-emulator value for an unpopulated EXP1 window, after its
    /// supported 512 KiB mapping is configured. This is not open-bus timing.
    pub(super) fn read_empty_expansion(&self, address: u32, width: Width) -> Result<u32, BusError> {
        if self.expansion_base[0] != 0x1f00_0000 || self.expansion_delay[0] != 0x0013_243f {
            return Err(error(address, "EXP1 mapping has not been configured"));
        }
        if address as usize & (width.bytes() - 1) != 0 {
            return Err(error(address, "unaligned EXP1 read"));
        }
        Ok(match width {
            Width::Byte => 0xff,
            Width::Half => 0xffff,
            Width::Word => u32::MAX,
        })
    }

    pub(super) fn ram_address(&self, address: u32) -> u32 {
        if self.dram == 0x0b88
            && matches!(address, 0..=0x007f_ffff | 0x8000_0000..=0x807f_ffff | 0xa000_0000..=0xa07f_ffff)
        {
            address & !0x0060_0000
        } else {
            address
        }
    }

    pub(super) fn handles(physical: u32) -> bool {
        matches!(
            physical,
            0x1f80_1000
                | 0x1f80_1004
                | 0x1f80_1008
                | 0x1f80_100c
                | 0x1f80_1010
                | 0x1f80_101c
                | 0x1f80_1060
        )
    }

    pub(super) fn read(&self, address: u32, width: Width) -> Result<u32, BusError> {
        if width != Width::Word {
            return Err(error(address, "boot memory controls require word accesses"));
        }
        Ok(match Interconnect::physical(address) {
            0x1f80_1000 => self.expansion_base[0],
            0x1f80_1004 => self.expansion_base[1],
            0x1f80_1008 => self.expansion_delay[0],
            0x1f80_100c => self.expansion_delay[1],
            0x1f80_101c => self.expansion_delay[2],
            0x1f80_1010 => self.bios,
            0x1f80_1060 => self.dram,
            _ => unreachable!(),
        })
    }

    pub(super) fn write(&mut self, address: u32, width: Width, value: u32) -> Result<(), BusError> {
        if width != Width::Word {
            return Err(error(address, "boot memory controls require word accesses"));
        }
        match (Interconnect::physical(address), value) {
            // Standard fixed expansion windows only. Access to expansion
            // devices themselves remains unsupported rather than returning
            // invented data from these configuration latches.
            (0x1f80_1000, 0x1f00_0000) => self.expansion_base[0] = value,
            (0x1f80_1004, 0x1f80_2000) => self.expansion_base[1] = value,
            (0x1f80_1008, 0x0013_243f) => self.expansion_delay[0] = value,
            (0x1f80_100c, 0x0000_3022) => self.expansion_delay[1] = value,
            (0x1f80_101c, 0x0007_0777) => self.expansion_delay[2] = value,
            // 512 KiB, incrementing 8-bit BIOS bus. The attached ROM already
            // supplies this data layout; per-access timing is separate.
            (0x1f80_1010, 0x0013_243f) => self.bios = value,
            // BIOS configures an 8 MiB aperture on 2 MiB retail RAM; the RAM
            // address translation mirrors the physical chip across it.
            (0x1f80_1060, 0x0000_0b88) => self.dram = value,
            _ => {
                return Err(error(
                    address,
                    "unsupported boot memory mapping/configuration",
                ))
            }
        }
        Ok(())
    }
}
