//! Width-aware memory transactions. MMIO implementations must see the full store
//! register and byte-enable mask; splitting a word into four writes is incorrect.

use super::executable::{ram_offset, Executable, RAM_BYTES};
use std::fmt;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Width {
    Byte,
    Half,
    Word,
}

impl Width {
    pub fn bytes(self) -> usize {
        match self {
            Self::Byte => 1,
            Self::Half => 2,
            Self::Word => 4,
        }
    }
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct BusError {
    pub address: u32,
    pub detail: String,
}

impl fmt::Display for BusError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "bus at 0x{:08X}: {}", self.address, self.detail)
    }
}
impl std::error::Error for BusError {}

pub trait Bus {
    /// Per-step CPU isolation state; plain RAM fixtures do not model caches.
    fn set_cache_isolation(&mut self, isolated: bool) -> Result<(), &'static str> {
        if isolated {
            Err("isolated cache execution requires a cache-capable bus")
        } else {
            Ok(())
        }
    }
    fn fetch(&mut self, address: u32) -> Result<u32, BusError> {
        self.read(address, Width::Word)
    }
    fn read(&mut self, address: u32, width: Width) -> Result<u32, BusError>;
    /// `value` retains all 32 GPR bits even for byte/halfword writes.
    fn write(&mut self, address: u32, width: Width, value: u32) -> Result<(), BusError>;
    /// One aligned word transaction, with bit N enabling byte lane N.
    fn write_masked(&mut self, address: u32, value: u32, lanes: u8) -> Result<(), BusError>;
}

pub struct Ram {
    bytes: Vec<u8>,
}

impl Default for Ram {
    fn default() -> Self {
        Self {
            bytes: vec![0; RAM_BYTES],
        }
    }
}

impl Ram {
    /// Explicit raw RAM input, not an emulator save-state or BIOS initializer.
    pub fn from_bytes(bytes: Vec<u8>) -> crate::Result<Self> {
        if bytes.len() != RAM_BYTES {
            return Err("raw RAM image must contain exactly 2 MiB".into());
        }
        Ok(Self { bytes })
    }
    pub fn from_executable(executable: &Executable) -> Self {
        Self {
            bytes: executable.load_ram(),
        }
    }
    pub fn bytes(&self) -> &[u8] {
        &self.bytes
    }

    /// Host executable handoff: preserve RAM outside the declared payload.
    pub(crate) fn load_executable(&mut self, executable: &Executable) -> crate::Result<()> {
        let header = executable.header();
        let offset = ram_offset(header.load_address, header.text_size)?;
        self.bytes[offset..offset + header.text_size].copy_from_slice(executable.text());
        Ok(())
    }
    fn range(address: u32, width: Width) -> Result<usize, BusError> {
        if address as usize & (width.bytes() - 1) != 0 {
            return Err(BusError {
                address,
                detail: "unaligned access".into(),
            });
        }
        ram_offset(address, width.bytes()).map_err(|error| BusError {
            address,
            detail: error.to_string(),
        })
    }
}

impl Bus for Ram {
    fn read(&mut self, address: u32, width: Width) -> Result<u32, BusError> {
        let start = Self::range(address, width)?;
        let mut word = [0; 4];
        word[..width.bytes()].copy_from_slice(&self.bytes[start..start + width.bytes()]);
        Ok(u32::from_le_bytes(word))
    }
    fn write(&mut self, address: u32, width: Width, value: u32) -> Result<(), BusError> {
        let start = Self::range(address, width)?;
        self.bytes[start..start + width.bytes()]
            .copy_from_slice(&value.to_le_bytes()[..width.bytes()]);
        Ok(())
    }
    fn write_masked(&mut self, address: u32, value: u32, lanes: u8) -> Result<(), BusError> {
        let start = Self::range(address, Width::Word)?;
        if lanes & !15 != 0 {
            return Err(BusError {
                address,
                detail: "invalid byte-enable mask".into(),
            });
        }
        for (lane, byte) in value.to_le_bytes().iter().enumerate() {
            if lanes & (1 << lane) != 0 {
                self.bytes[start + lane] = *byte;
            }
        }
        Ok(())
    }
}
