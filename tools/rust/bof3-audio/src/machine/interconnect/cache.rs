//! Functional instruction cache and BIOS isolation access, without cycle timing.
//! Evidence: psx-spx memory-map/cache and memory-control hardware descriptions.
use super::error;
use crate::machine::bus::{BusError, Width};

pub(super) const CONTROL: u32 = 0xfffe_0130;

#[derive(Clone, Copy, Default)]
struct Line {
    tag: u32,
    valid: u8,
    words: [u32; 4],
}

pub(super) struct Cache {
    control: Option<u32>,
    isolated: bool,
    lines: [Line; 256],
}

impl Default for Cache {
    fn default() -> Self {
        Self {
            control: None,
            isolated: false,
            lines: [Line::default(); 256],
        }
    }
}

pub(super) enum Fetch {
    Direct,
    Hit(u32),
    Fill { start: usize, end: usize },
}

fn cached(address: u32) -> bool {
    matches!(address, 0..=0x1fff_ffff | 0x8000_0000..=0x9fff_ffff)
}

fn tag(address: u32) -> u32 {
    address & 0x1fff_f000
}

impl Cache {
    pub(super) fn scratch_enabled(&self) -> bool {
        self.control.is_none_or(|value| value & 0x88 == 0x88)
    }

    pub(super) fn set_isolation(&mut self, isolated: bool) -> Result<(), &'static str> {
        if isolated && self.control.is_none_or(|value| value & 0x800 == 0) {
            return Err("cache isolation requires a configured instruction cache");
        }
        self.isolated = isolated;
        Ok(())
    }

    pub(super) fn read_control(&self, width: Width) -> Result<u32, BusError> {
        if width != Width::Word {
            return Err(error(CONTROL, "cache control requires word accesses"));
        }
        Ok(self.control.unwrap_or(0))
    }

    pub(super) fn write_control(&mut self, width: Width, value: u32) -> Result<(), BusError> {
        if width != Width::Word || !matches!(value, 0 | 0x800 | 0x804 | 0x1e988 | 0x1e90c) {
            return Err(error(
                CONTROL,
                "unsupported cache-control width/configuration",
            ));
        }
        self.control = Some(value);
        Ok(())
    }

    pub(super) fn isolated_access(&self, address: u32) -> bool {
        self.isolated && cached(address)
    }

    pub(super) fn read_isolated(&self, address: u32, width: Width) -> Result<u32, BusError> {
        if width != Width::Word || address & 3 != 0 {
            return Err(error(address, "isolated cache reads require aligned words"));
        }
        let line = &self.lines[((address >> 4) & 255) as usize];
        let word = line.words[((address >> 2) & 3) as usize];
        Ok(if self.control.unwrap() & 4 != 0 {
            (word & !31) | u32::from(line.valid) | (u32::from(line.tag == tag(address)) << 4)
        } else {
            word
        })
    }

    pub(super) fn write_isolated(
        &mut self,
        address: u32,
        width: Width,
        value: u32,
    ) -> Result<(), BusError> {
        if width != Width::Word || address & 3 != 0 {
            return Err(error(
                address,
                "isolated cache stores require aligned words",
            ));
        }
        let line = &mut self.lines[((address >> 4) & 255) as usize];
        if self.control.unwrap() & 4 != 0 {
            line.tag = tag(address);
            line.valid = (value & 15) as u8;
        } else {
            line.words[((address >> 2) & 3) as usize] = value;
        }
        Ok(())
    }

    pub(super) fn fetch(&self, address: u32) -> Result<Fetch, BusError> {
        if address & 3 != 0 {
            return Err(error(address, "unaligned instruction fetch"));
        }
        if !cached(address) || self.control.is_none_or(|value| value & 0x800 == 0) {
            return Ok(Fetch::Direct);
        }
        let line = &self.lines[((address >> 4) & 255) as usize];
        let word = ((address >> 2) & 3) as usize;
        if line.tag == tag(address) && line.valid & (1 << word) != 0 {
            return Ok(Fetch::Hit(line.words[word]));
        }
        // A matching tag with an invalid word refills the complete line.
        // Different-tag misses refill forward from the requested word.
        let start = if line.tag == tag(address) { 0 } else { word };
        let end = if line.tag == tag(address) || start != 0 || self.control.unwrap() & 0x100 != 0 {
            4
        } else {
            2
        };
        Ok(Fetch::Fill { start, end })
    }

    pub(super) fn fill(&mut self, address: u32, start: usize, end: usize, words: [u32; 4]) {
        let line = &mut self.lines[((address >> 4) & 255) as usize];
        line.tag = tag(address);
        line.valid = (((1 << end) - 1) & !((1 << start) - 1)) as u8;
        line.words[start..end].copy_from_slice(&words[start..end]);
    }
}
