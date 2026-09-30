//! Physical device/RAM routing, separate from CPU cache access.
use super::*;

impl Interconnect {
    pub(super) fn read_mapped(&mut self, address: u32, width: Width) -> Result<u32, BusError> {
        if (0x1f00_0000..0x1f08_0000).contains(&Self::physical(address)) {
            return self.memory_control.read_empty_expansion(address, width);
        }
        if Firmware::offset(address).is_some() {
            return self
                .firmware
                .as_ref()
                .ok_or_else(|| error(address, "BIOS ROM is not attached"))?
                .read(address, width);
        }
        if let Some(offset) = Self::scratch_offset(address, width)? {
            if !self.cache.scratch_enabled() {
                return Err(error(address, "scratchpad disabled by cache control"));
            }
            let mut word = [0; 4];
            word[..width.bytes()].copy_from_slice(&self.scratchpad[offset..offset + width.bytes()]);
            return Ok(u32::from_le_bytes(word));
        }
        let physical = Self::physical(address);
        if Self::is_cd_port(physical) {
            return self.read_cd_port(address, width);
        }
        if memory::Control::handles(physical) {
            return self.memory_control.read(address, width);
        }
        if physical == 0x1f80_1014 {
            if width != Width::Word {
                return Err(error(
                    address,
                    "DEV4_CTRL partial reads are not implemented",
                ));
            }
            return Ok(self.spu_bus_control);
        }
        if (0x1f80_1c00..0x1f80_1da0).contains(&physical)
            || physical == 0x1f80_1da2
            || (0x1f80_1dc0..0x1f80_1e00).contains(&physical)
            || (0x1f80_1da4..0x1f80_1dbc).contains(&physical)
            || (0x1f80_1e00..0x1f80_1e60).contains(&physical)
        {
            return self.read_spu_port(address, width);
        }
        if (0x1f80_10c0..0x1f80_10cc).contains(&physical) {
            Self::require_word_port(address, width)?;
            let value = self
                .dma
                .spu
                .read(physical & 15)
                .map_err(|e| error(address, &e.to_string()))?;
            return Ok(if width == Width::Half {
                value & 0xffff
            } else {
                value
            });
        }
        if (0x1f80_1100..0x1f80_1130).contains(&physical) {
            Self::require_word_port(address, width)?;
            return self
                .timers
                .read(((physical - 0x1f80_1100) >> 4) as usize, physical & 15)
                .map_err(|e| error(address, &e.to_string()));
        }
        let value = match physical {
            0x1f80_1070 => u32::from(self.interrupts.status()),
            0x1f80_1074 => u32::from(self.interrupts.mask()),
            0x1f80_10f0 => self.dma.priority(),
            0x1f80_10f4 => self.dma.interrupt(),
            _ if (0x1f80_1000..0x1f80_2000).contains(&physical) => {
                return Err(error(address, "unimplemented I/O read"))
            }
            _ => {
                return self
                    .ram
                    .read(self.memory_control.ram_address(address), width)
            }
        };
        Self::require_word_port(address, width)?;
        Ok(if width == Width::Half {
            value & 0xffff
        } else {
            value
        })
    }
    pub(super) fn write_mapped(
        &mut self,
        address: u32,
        width: Width,
        value: u32,
    ) -> Result<(), BusError> {
        if Firmware::offset(address).is_some() {
            return Err(error(address, "BIOS ROM stores are unsupported"));
        }
        if let Some(offset) = Self::scratch_offset(address, width)? {
            if !self.cache.scratch_enabled() {
                return Err(error(address, "scratchpad disabled by cache control"));
            }
            self.scratchpad[offset..offset + width.bytes()]
                .copy_from_slice(&value.to_le_bytes()[..width.bytes()]);
            return Ok(());
        }
        let physical = Self::physical(address);
        if physical == 0x1f80_2041 {
            if width != Width::Byte {
                return Err(error(address, "POST display requires a byte write"));
            }
            self.post_status = Some(value as u8);
            return Ok(());
        }
        if Self::is_cd_port(physical) {
            return self.write_cd_port(address, width, value);
        }
        if memory::Control::handles(physical) {
            return self.memory_control.write(address, width, value);
        }
        if physical == 0x1f80_1014 {
            if width != Width::Word || !matches!(value, 0x2009_31e1 | 0x2209_31e1) {
                return Err(error(address, "DEV4_CTRL requires a word write of 0x200931e1 or 0x220931e1; other bus configurations are not implemented"));
            }
            self.spu_bus_control = value;
            return Ok(());
        }
        if (0x1f80_1c00..0x1f80_1da0).contains(&physical)
            || physical == 0x1f80_1da2
            || (0x1f80_1dc0..0x1f80_1e00).contains(&physical)
            || (0x1f80_1da4..0x1f80_1dbc).contains(&physical)
            || (0x1f80_1e00..0x1f80_1e60).contains(&physical)
        {
            return self.write_spu_port(address, width, value);
        }
        if (0x1f80_10c0..0x1f80_10cc).contains(&physical) {
            Self::require_word_port(address, width)?;
            return self
                .dma
                .spu
                .write(physical & 15, value)
                .map_err(|e| error(address, &e.to_string()));
        }
        if (0x1f80_1100..0x1f80_1130).contains(&physical) {
            Self::require_word_port(address, width)?;
            return self
                .timers
                .write(
                    ((physical - 0x1f80_1100) >> 4) as usize,
                    physical & 15,
                    value,
                    &mut self.interrupts,
                )
                .map_err(|e| error(address, &e.to_string()));
        }
        match physical {
            0x1f80_1070 | 0x1f80_1074 | 0x1f80_10f0 | 0x1f80_10f4 => {
                Self::require_word_port(address, width)?
            }
            _ if (0x1f80_1000..0x1f80_2000).contains(&physical) => {
                return Err(error(address, "unimplemented I/O write"))
            }
            _ => {
                return self
                    .ram
                    .write(self.memory_control.ram_address(address), width, value)
            }
        }
        match physical {
            0x1f80_1070 => self.interrupts.acknowledge(value),
            0x1f80_1074 => self.interrupts.set_mask(value),
            // DMA accepts the full GPR value even for SH, as observed on hardware.
            0x1f80_10f0 => self.dma.set_priority(value),
            0x1f80_10f4 => {
                self.dma.set_interrupt(value);
                self.interrupts.set_line(Source::Dma, self.dma.irq_line());
            }
            _ => unreachable!(),
        }
        Ok(())
    }
    pub(super) fn write_masked_mapped(
        &mut self,
        address: u32,
        value: u32,
        lanes: u8,
    ) -> Result<(), BusError> {
        if Firmware::offset(address).is_some() {
            return Err(error(address, "BIOS ROM masked stores are unsupported"));
        }
        if let Some(offset) = Self::scratch_offset(address, Width::Word)? {
            if !self.cache.scratch_enabled() {
                return Err(error(address, "scratchpad disabled by cache control"));
            }
            if lanes & !15 != 0 {
                return Err(error(address, "invalid byte-enable mask"));
            }
            for (lane, byte) in value.to_le_bytes().iter().enumerate() {
                if lanes & (1 << lane) != 0 {
                    self.scratchpad[offset + lane] = *byte;
                }
            }
            return Ok(());
        }
        if (0x1f80_1000..0x1f80_2000).contains(&Self::physical(address)) {
            return Err(error(address, "masked I/O writes are not implemented"));
        }
        self.ram
            .write_masked(self.memory_control.ram_address(address), value, lanes)
    }
}
