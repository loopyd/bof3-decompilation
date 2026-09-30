//! BIOS event operations on guest EvCB RAM, not a separate host registry.
//! https://psx-spx.consoledev.net/kernelbios/#bios-event-functions
use super::{
    bus::{Bus, Width},
    executable::ram_offset,
};
use crate::Result;

pub const RECORD_BYTES: u32 = 0x1c;
pub const DISABLED: u32 = 0x1000;
pub const BUSY: u32 = 0x2000;
pub const READY: u32 = 0x4000;
pub const CALLBACK: u32 = 0x1000;
pub const POLLING: u32 = 0x2000;

#[derive(Clone, Copy, Debug)]
pub struct Table {
    base: u32,
    slots: u32,
}

#[derive(Clone, Debug)]
pub struct Delivery {
    table: Table,
    class: u32,
    spec: u32,
    next_slot: u32,
}

impl Delivery {
    /// Process polling events until the next callback. The caller executes that
    /// callback before resuming this cursor, so later records are read live.
    pub fn next_callback(&mut self, bus: &mut impl Bus) -> Result<Option<u32>> {
        while self.next_slot < self.table.slots {
            let slot = self.next_slot;
            let address = self.table.base + slot * RECORD_BYTES;
            self.next_slot += 1;
            if bus.read(address + 4, Width::Word)? != BUSY
                || bus.read(address, Width::Word)? != self.class
                || bus.read(address + 8, Width::Word)? != self.spec
            {
                continue;
            }
            match bus.read(address + 12, Width::Word)? {
                POLLING => bus.write(address + 4, Width::Word, READY)?,
                CALLBACK => {
                    let handler = bus.read(address + 16, Width::Word)?;
                    if handler != 0 {
                        return Ok(Some(handler));
                    }
                }
                mode => {
                    return Err(format!(
                        "BIOS DeliverEvent: slot {slot} has unsupported mode 0x{mode:08X}"
                    )
                    .into())
                }
            }
        }
        Ok(None)
    }
}

impl Table {
    /// The BIOS bootstrap must establish this descriptor. An empty loaded EXE
    /// is not evidence of an initialized BIOS, so no table is invented here.
    pub fn read(bus: &mut impl Bus) -> Result<Self> {
        let base = bus.read(0x120, Width::Word)?;
        let size = bus.read(0x124, Width::Word)?;
        if base == 0
            || base & 3 != 0
            || size == 0
            || size % RECORD_BYTES != 0
            || size / RECORD_BYTES > 65536
        {
            return Err("BIOS events: missing or malformed guest EvCB table descriptor".into());
        }
        ram_offset(base, size as usize)?;
        Ok(Self {
            base,
            slots: size / RECORD_BYTES,
        })
    }

    pub fn free_slot(&self, bus: &mut impl Bus) -> Result<u32> {
        for slot in 0..self.slots {
            if bus.read(self.base + slot * RECORD_BYTES + 4, Width::Word)? == 0 {
                return Ok(slot);
            }
        }
        Ok(u32::MAX)
    }

    pub fn open(
        &self,
        bus: &mut impl Bus,
        class: u32,
        spec: u32,
        mode: u32,
        handler: u32,
    ) -> Result<u32> {
        if !matches!(mode, CALLBACK | POLLING) {
            return Err(format!("BIOS OpenEvent: unsupported mode 0x{mode:08X}").into());
        }
        let slot = self.free_slot(bus)?;
        if slot == u32::MAX {
            return Ok(u32::MAX);
        }
        let address = self.base + slot * RECORD_BYTES;
        for (offset, value) in [
            (0, class),
            (4, DISABLED),
            (8, spec),
            (12, mode),
            (16, handler),
        ] {
            bus.write(address + offset, Width::Word, value)?;
        }
        Ok(0xf100_0000 | slot)
    }

    pub fn address(&self, handle: u32) -> Result<u32> {
        let slot = handle & 0xffff;
        if slot >= self.slots {
            // Real BIOS routines can access beyond their table. Do not silently
            // ignore such writes or claim their arbitrary-memory effects.
            return Err(format!("BIOS event handle 0x{handle:08X}: slot {slot} is outside {} allocated EvCB records", self.slots).into());
        }
        Ok(self.base + slot * RECORD_BYTES)
    }

    pub fn close(&self, bus: &mut impl Bus, handle: u32) -> Result<()> {
        bus.write(self.address(handle)? + 4, Width::Word, 0)?;
        Ok(())
    }

    pub fn enable(&self, bus: &mut impl Bus, handle: u32, enabled: bool) -> Result<()> {
        let address = self.address(handle)? + 4;
        if bus.read(address, Width::Word)? != 0 {
            bus.write(address, Width::Word, if enabled { BUSY } else { DISABLED })?;
        }
        Ok(())
    }

    pub fn test(&self, bus: &mut impl Bus, handle: u32) -> Result<bool> {
        Self::consume_ready(bus, self.address(handle)?)
    }

    pub fn consume_ready(bus: &mut impl Bus, address: u32) -> Result<bool> {
        if bus.read(address + 4, Width::Word)? != READY {
            return Ok(false);
        }
        bus.write(address + 4, Width::Word, BUSY)?;
        Ok(true)
    }

    pub fn delivery(&self, class: u32, spec: u32) -> Delivery {
        Delivery {
            table: *self,
            class,
            spec,
            next_slot: 0,
        }
    }

    pub fn undeliver(&self, bus: &mut impl Bus, class: u32, spec: u32) -> Result<()> {
        for slot in 0..self.slots {
            let address = self.base + slot * RECORD_BYTES;
            if bus.read(address + 4, Width::Word)? == READY
                && bus.read(address, Width::Word)? == class
                && bus.read(address + 8, Width::Word)? == spec
                && bus.read(address + 12, Width::Word)? == POLLING
            {
                bus.write(address + 4, Width::Word, BUSY)?;
            }
        }
        Ok(())
    }
}
