//! Bounded formatter-byte capture for the callback manager's integer diagnostics.

use super::{
    bus::{Bus, Width},
    cpu::Cpu,
    executable::ram_offset,
};
use crate::Result;

const FORMAT_BYTES: u32 = 4096;
const MAX_CAPACITY: usize = 1024 * 1024;

#[derive(Clone, Debug)]
pub(super) struct Console {
    bytes: Vec<u8>,
    capacity: usize,
}

impl Console {
    pub(super) fn new(capacity: usize) -> Result<Self> {
        if !(1..=MAX_CAPACITY).contains(&capacity) {
            return Err("diagnostic capacity must be 1..=1048576 bytes".into());
        }
        Ok(Self {
            bytes: Vec::new(),
            capacity,
        })
    }

    pub(super) fn bytes(&self) -> &[u8] {
        &self.bytes
    }

    pub(super) fn printf(&mut self, cpu: &Cpu, bus: &mut impl Bus) -> Result<u32> {
        let format = read_format(bus, cpu.register(4))?;
        let mut output = Vec::new();
        let mut count = 0;
        let mut argument = 0;
        let mut position = 0;
        while position < format.len() {
            let byte = format[position];
            position += 1;
            if byte != b'%' {
                output.push(byte);
            } else if format.get(position) == Some(&b'%') {
                position += 1;
                output.push(b'%');
                count += 1;
            } else {
                let remaining = &format[position..];
                let (length, decimal, width) = if remaining.starts_with(b"d") {
                    (1, true, 0)
                } else if remaining.starts_with(b"04x") {
                    (3, false, 4)
                } else if remaining.starts_with(b"08x") {
                    (3, false, 8)
                } else {
                    return Err(
                        "unsupported diagnostic format; expected %d, %04x, %08x or %%".into(),
                    );
                };
                let value = read_argument(cpu, bus, argument)?;
                argument += 1;
                position += length;
                let field = if decimal {
                    (value as i32).to_string()
                } else {
                    format!("{value:0width$x}")
                };
                count += u32::try_from(field.len())?;
                output.extend_from_slice(field.as_bytes());
            }
            if output.len() > self.capacity - self.bytes.len() {
                return Err("diagnostic output capacity exceeded".into());
            }
        }
        self.bytes.extend_from_slice(&output);
        Ok(count)
    }
}

fn read_format(bus: &mut impl Bus, address: u32) -> Result<Vec<u8>> {
    if address == 0 {
        return Ok(Vec::new());
    }
    let mut bytes = Vec::new();
    for offset in 0..FORMAT_BYTES {
        let current = address
            .checked_add(offset)
            .ok_or("diagnostic format address overflows")?;
        ram_offset(current, 1)?;
        let byte = bus.read(current, Width::Byte)? as u8;
        if byte == 0 {
            return Ok(bytes);
        }
        bytes.push(byte);
    }
    Err("diagnostic format exceeds 4096 bytes including NUL".into())
}

fn read_argument(cpu: &Cpu, bus: &mut impl Bus, index: u32) -> Result<u32> {
    if index < 3 {
        return Ok(cpu.register(index as usize + 5));
    }
    let offset = (index - 3)
        .checked_mul(4)
        .and_then(|offset| offset.checked_add(16))
        .ok_or("diagnostic argument offset overflows")?;
    let address = cpu
        .register(29)
        .checked_add(offset)
        .ok_or("diagnostic argument address overflows")?;
    if address & 3 != 0 {
        return Err("diagnostic stack argument is unaligned".into());
    }
    ram_offset(address, 4)?;
    Ok(bus.read(address, Width::Word)?)
}
