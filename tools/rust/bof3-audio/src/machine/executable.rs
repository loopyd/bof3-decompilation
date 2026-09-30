//! PS-X EXE header loading with explicit 2 MiB RAM bounds and aliases.

use crate::Result;

pub const RAM_BYTES: usize = 2 * 1024 * 1024;
const HEADER_BYTES: usize = 0x800;

#[derive(Clone, Debug)]
pub struct Executable {
    bytes: Vec<u8>,
    header: Header,
}

#[derive(Clone, Debug)]
pub struct Header {
    pub entry_pc: u32,
    pub global_pointer: u32,
    pub load_address: u32,
    pub text_size: usize,
    pub stack_base: u32,
    pub stack_offset: u32,
}

/// Accept only physical RAM and its KSEG0/KSEG1 aliases, not arbitrary masking.
pub fn ram_offset(address: u32, length: usize) -> Result<usize> {
    let physical = match address {
        0x0000_0000..=0x001f_ffff => address,
        0x8000_0000..=0x801f_ffff => address - 0x8000_0000,
        0xa000_0000..=0xa01f_ffff => address - 0xa000_0000,
        _ => return Err(format!("0x{address:08X} is not a supported 2 MiB RAM address").into()),
    } as usize;
    if length > RAM_BYTES - physical {
        return Err(format!("RAM range at 0x{address:08X} ({length} bytes) exceeds 2 MiB").into());
    }
    Ok(physical)
}

impl Executable {
    pub fn from_bytes(bytes: Vec<u8>) -> Result<Self> {
        if bytes.len() < HEADER_BYTES || &bytes[..8] != b"PS-X EXE" {
            return Err("expected a complete PS-X EXE header".into());
        }
        let word = |offset| u32::from_le_bytes(bytes[offset..offset + 4].try_into().unwrap());
        let entry_pc = word(0x10);
        let global_pointer = word(0x14);
        let load_address = word(0x18);
        let text_size = word(0x1c) as usize;
        let stack_base = word(0x30);
        let stack_offset = word(0x34);
        if load_address & 3 != 0 || entry_pc & 3 != 0 || text_size == 0 {
            return Err("PS-X EXE needs aligned load/entry addresses and nonempty text".into());
        }
        ram_offset(load_address, text_size)?;
        ram_offset(entry_pc, 4)?;
        if text_size > bytes.len() - HEADER_BYTES {
            return Err("PS-X EXE text is truncated".into());
        }
        if stack_base != 0 {
            let stack = stack_base
                .checked_add(stack_offset)
                .ok_or("PS-X EXE stack address overflows")?;
            if stack & 3 != 0 {
                return Err("PS-X EXE stack is not word-aligned".into());
            }
            ram_offset(stack, 0)?;
        }
        Ok(Self {
            bytes,
            header: Header {
                entry_pc,
                global_pointer,
                load_address,
                text_size,
                stack_base,
                stack_offset,
            },
        })
    }

    pub fn header(&self) -> &Header {
        &self.header
    }

    pub fn bytes(&self) -> &[u8] {
        &self.bytes
    }

    pub fn text(&self) -> &[u8] {
        &self.bytes[HEADER_BYTES..HEADER_BYTES + self.header.text_size]
    }

    pub fn stack_pointer(&self) -> Option<u32> {
        (self.header.stack_base != 0).then(|| self.header.stack_base + self.header.stack_offset)
    }

    pub fn load_ram(&self) -> Vec<u8> {
        let mut ram = vec![0; RAM_BYTES];
        let offset = ram_offset(self.header.load_address, self.header.text_size)
            .expect("validated executable range");
        ram[offset..offset + self.header.text_size].copy_from_slice(self.text());
        ram
    }
}
