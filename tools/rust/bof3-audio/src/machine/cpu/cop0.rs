//! R3000 exception/status state. Active debug breakpoints remain unsupported.
//! https://psx-spx.consoledev.net/cpuspecifications/#cop0-exception-handling

use serde::Serialize;

#[derive(Clone, Debug, Default, Serialize)]
pub struct Cop0 {
    status: u32,
    cause: u32,
    epc: u32,
    target: u32,
    bad_address: u32,
    debug_addresses: [u32; 4],
}

impl Cop0 {
    /// Literal reference-emulator reset state, not a hardware reset claim.
    /// PCSX-Redux 28438546c781fbe372a06399c82bed43ca2c6f4d, r3000a.cc:76.
    /// Its legacy SR includes reserved bit 23; ordinary MTC0 still uses the
    /// existing guest-write mask and does not preserve that bit.
    pub fn pcsx_redux_reset() -> Self {
        Self {
            status: 0x1090_0000,
            ..Self::default()
        }
    }

    pub fn read(&self, register: usize) -> Result<u32, &'static str> {
        Ok(match register {
            3 => self.debug_addresses[0],
            5 => self.debug_addresses[1],
            9 => self.debug_addresses[2],
            11 => self.debug_addresses[3],
            7 => 0, // Only disabled DCIC is supported.
            6 => self.target,
            8 => self.bad_address,
            12 => self.status,
            13 => self.cause,
            14 => self.epc,
            15 => 2,
            _ => return Err("unsupported COP0 register read"),
        })
    }

    pub fn write(&mut self, register: usize, value: u32) -> Result<(), &'static str> {
        match register {
            // The original ROM clears TAR during reset. Hardware documents
            // TAR as read-only, while Redux writes its stored value. Both leave
            // this already-zero seed unchanged; other TAR writes stay rejected.
            6 if value == 0 && self.target == 0 => {}
            3 => self.debug_addresses[0] = value,
            5 => self.debug_addresses[1] = value,
            9 => self.debug_addresses[2] = value,
            11 => self.debug_addresses[3] = value,
            7 if value == 0 => {}
            12 => self.status = value & 0xf27f_ff3f,
            13 => self.cause = (self.cause & !0x300) | (value & 0x300),
            _ => return Err("unsupported COP0 register write"),
        }
        Ok(())
    }

    pub fn status(&self) -> u32 {
        self.status
    }

    pub fn set_external_interrupt(&mut self, asserted: bool) {
        self.cause = (self.cause & !0x400) | (u32::from(asserted) << 10);
    }

    pub fn interrupt_pending(&self) -> bool {
        self.status & 1 != 0 && self.status & self.cause & 0xff00 != 0
    }

    pub fn return_from_exception(&mut self) {
        self.status = (self.status & !15) | ((self.status >> 2) & 15);
    }

    pub(crate) fn enter(
        &mut self,
        code: u32,
        pc: u32,
        delay: bool,
        taken: bool,
        target: u32,
        bad_address: Option<u32>,
    ) -> u32 {
        self.cause = (self.cause & 0xff00)
            | (code << 2)
            | (u32::from(delay) << 31)
            | (u32::from(delay && taken) << 30);
        self.epc = if delay { pc.wrapping_sub(4) } else { pc };
        if delay {
            self.target = target;
        }
        if let Some(address) = bad_address {
            self.bad_address = address;
        }
        self.status = (self.status & !63) | ((self.status << 2) & 63);
        if self.status & (1 << 22) != 0 {
            0xbfc0_0180
        } else {
            0x8000_0080
        }
    }
}
