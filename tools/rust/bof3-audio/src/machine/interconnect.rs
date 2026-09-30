//! RAM, scratchpad and explicitly modeled I/O. All other devices fail closed.
mod cache;
mod cd;
mod mapped;
mod memory;

use super::{
    bus::{Bus, BusError, Ram, Width},
    cd_host,
    dma::Dma,
    executable::Executable,
    firmware::Image as Firmware,
    interrupts::{Interrupts, Source},
    spu_mixer::{Inputs as SpuInputs, Mixer},
    spu_noise::Noise,
    spu_reverb::{Model as ReverbModel, Reverb},
    spu_transfer::{Mode as SpuMode, Transfer as SpuTransfer},
    spu_voice_ports,
    timers::Timers,
};

pub struct Interconnect {
    cache: cache::Cache,
    memory_control: memory::Control,
    post_status: Option<u8>,
    ram: Ram,
    firmware: Option<Firmware>,
    cd_host: Option<cd_host::Host>,
    cd_audio: Option<super::cd_queue::Queue>,
    cd_command_muted: bool,
    cd_bus_control: u32,
    common_delay: u32,
    scratchpad: [u8; 1024],
    interrupts: Interrupts,
    dma: Dma,
    timers: Timers,
    spu_transfer: SpuTransfer,
    spu_voices: Option<spu_voice_ports::Ports>,
    spu_mixer: Mixer,
    spu_reverb: Option<Reverb>,
    // Only the linked runtime's two normal bus configurations are supported.
    // Retained for read/modify/write; bus wait-state timing is not modeled.
    spu_bus_control: u32,
}

impl Interconnect {
    pub fn from_executable(exe: &Executable) -> Self {
        Self::from_ram(Ram::from_executable(exe))
    }
    /// Devices retain their model defaults; raw RAM does not restore devices.
    pub fn from_ram(ram: Ram) -> Self {
        Self {
            cache: cache::Cache::default(),
            memory_control: memory::Control::default(),
            post_status: None,
            ram,
            firmware: None,
            cd_host: None,
            cd_audio: None,
            cd_command_muted: false,
            cd_bus_control: 0,
            common_delay: 0,
            scratchpad: [0; 1024],
            interrupts: Interrupts::default(),
            dma: Dma::default(),
            timers: Timers::default(),
            spu_transfer: SpuTransfer::default(),
            spu_voices: None,
            spu_mixer: Mixer::default(),
            spu_reverb: None,
            spu_bus_control: 0,
        }
    }
    pub fn attach_firmware(&mut self, firmware: Firmware) -> crate::Result<()> {
        if self.firmware.is_some() {
            return Err("BIOS ROM already attached".into());
        }
        self.firmware = Some(firmware);
        Ok(())
    }
    pub fn interrupts(&self) -> &Interrupts {
        &self.interrupts
    }
    pub fn interrupts_mut(&mut self) -> &mut Interrupts {
        &mut self.interrupts
    }
    pub fn dma(&self) -> &Dma {
        &self.dma
    }
    pub fn ram(&self) -> &Ram {
        &self.ram
    }
    pub(crate) fn load_executable(&mut self, executable: &Executable) -> crate::Result<()> {
        self.ram.load_executable(executable)
    }
    /// Last external POST display write, absent until the ROM writes it.
    /// This diagnostic sink has no CPU-readable register or interrupt effect.
    pub fn post_status(&self) -> Option<u8> {
        self.post_status
    }
    pub fn spu_transfer(&self) -> &SpuTransfer {
        &self.spu_transfer
    }

    /// Explicit evidence-model selection; never silently reset a configured device.
    pub fn configure_spu_voices(
        &mut self,
        sample: super::spu_sample::Model,
        envelope: super::adsr::Model,
    ) -> crate::Result<()> {
        if self.spu_voices.is_some() {
            return Err("SPU voices already configured".into());
        }
        self.spu_mixer.configure_sweeps(envelope)?;
        self.spu_voices = Some(spu_voice_ports::Ports::new(sample, envelope));
        Ok(())
    }

    /// Per-voice signals before main gain, mixing, capture, CD and reverb. The
    /// caller supplies frame boundaries; CPU/device clock synchronization is open.
    pub fn step_spu_voices(&mut self) -> crate::Result<spu_voice_ports::Frame> {
        let voices = self
            .spu_voices
            .as_mut()
            .ok_or("SPU voice arithmetic models must be configured explicitly")?;
        if self.spu_transfer.read(10)? & 0x8000 == 0 {
            return Err(
                "SPU voice stepping requires SPUCNT enable; disabled-frame execution is unmodeled"
                    .into(),
            );
        }
        voices.tick(self.spu_transfer.ram(), self.spu_transfer.read(10)?)
    }

    pub fn configure_spu_noise(&mut self, level: u16, timer: i32) -> crate::Result<()> {
        self.spu_voices
            .as_mut()
            .ok_or("configure SPU voices before noise")?
            .configure_noise(Noise::from_published_state(level, timer)?)
    }

    pub fn configure_spu_disable(
        &mut self,
        model: spu_voice_ports::DisableModel,
    ) -> crate::Result<()> {
        self.spu_voices
            .as_mut()
            .ok_or("configure SPU voices before disable behavior")?
            .configure_disable(model)
    }

    pub fn configure_spu_reverb(&mut self, model: ReverbModel) -> crate::Result<()> {
        if self.spu_reverb.is_some() {
            return Err("SPU reverb already configured".into());
        }
        self.spu_reverb = Some(Reverb::new(model));
        Ok(())
    }

    /// One scheduled output frame, including captures and currently supported
    /// mixing. Caller supplies already-resampled input frames. Audible reverb
    /// requires explicit configuration; no silent dry fallback is permitted.
    pub fn step_spu_output(&mut self, inputs: SpuInputs) -> crate::Result<[i16; 2]> {
        let control = self.spu_transfer.read(10)?;
        if self.spu_reverb.is_none() {
            self.spu_mixer.require_dry(control)?;
        }
        if self.spu_transfer.read(12)? != 4 {
            return Err("SPU output capture requires normal 512 KiB RAM control 0x0004".into());
        }
        let voices = self.step_spu_voices()?;
        let sends = self.spu_mixer.prepare(&voices, control, inputs);
        let wet = if let Some(reverb) = &mut self.spu_reverb {
            reverb.tick(
                self.spu_transfer.ram_mut(),
                sends.reverb,
                control & 0x80 != 0,
            )?
        } else {
            [0; 2]
        };
        let output = self.spu_mixer.finish(sends, wet)?;
        self.spu_transfer
            .capture_frame(inputs.cd, [voices.voices[1].mono, voices.voices[3].mono])?;
        Ok(output)
    }

    fn read_spu_half(&self, physical: u32) -> crate::Result<u16> {
        if physical == 0x1f80_1da2 || (0x1f80_1dc0..0x1f80_1e00).contains(&physical) {
            return self
                .spu_reverb
                .as_ref()
                .ok_or("SPU reverb model must be configured explicitly")?
                .read(physical - 0x1f80_1c00);
        }
        if (0x1f80_1d80..0x1f80_1d88).contains(&physical)
            || (0x1f80_1db0..0x1f80_1dbc).contains(&physical)
        {
            return self.spu_mixer.read(physical - 0x1f80_1c00);
        }
        if physical < 0x1f80_1da0 || (0x1f80_1e00..0x1f80_1e60).contains(&physical) {
            self.spu_voices
                .as_ref()
                .ok_or("SPU voice arithmetic models must be configured explicitly")?
                .read(physical - 0x1f80_1c00)
        } else {
            self.spu_transfer.read(physical - 0x1f80_1da0)
        }
    }

    fn write_spu_half(&mut self, physical: u32, value: u16) -> crate::Result<()> {
        if physical == 0x1f80_1da2 || (0x1f80_1dc0..0x1f80_1e00).contains(&physical) {
            return self
                .spu_reverb
                .as_mut()
                .ok_or("SPU reverb model must be configured explicitly")?
                .write(physical - 0x1f80_1c00, value);
        }
        if (0x1f80_1d80..0x1f80_1d88).contains(&physical)
            || (0x1f80_1db0..0x1f80_1dbc).contains(&physical)
        {
            return self.spu_mixer.write(physical - 0x1f80_1c00, value);
        }
        if physical == 0x1f80_1daa
            && self.spu_transfer.read(10)? & 0x8000 != 0
            && value & 0x8000 == 0
        {
            if let Some(voices) = self.spu_voices.as_mut() {
                voices.require_disable_model()?;
                self.spu_transfer.write(10, value)?;
                voices.disable();
                return Ok(());
            }
        }
        if physical < 0x1f80_1da0 || (0x1f80_1e00..0x1f80_1e60).contains(&physical) {
            self.spu_voices
                .as_mut()
                .ok_or("SPU voice arithmetic models must be configured explicitly")?
                .write(physical - 0x1f80_1c00, value)
        } else {
            self.spu_transfer.write(physical - 0x1f80_1da0, value)
        }
    }

    /// Service a bounded number of FIFO/RAM halfword transactions, pumping
    /// DMA4 slices at request boundaries. This does not advance CPU clocks and
    /// must not be used as evidence for bus latency or cycle-accurate DMA.
    pub fn service_spu(&mut self, halfword_budget: usize) -> crate::Result<usize> {
        self.spu_transfer.apply_control()?;
        self.pump_spu_dma()?;
        let mut count = 0;
        while count < halfword_budget && self.spu_transfer.service_halfword()? {
            count += 1;
            self.pump_spu_dma()?;
        }
        Ok(count)
    }

    fn pump_spu_dma(&mut self) -> crate::Result<()> {
        if self.dma.priority() & (1 << 19) == 0 || !self.dma.spu.active() {
            return Ok(());
        }
        let mode = self.spu_transfer.mode();
        if mode == SpuMode::Stopped {
            return Ok(());
        }
        let from_ram = self.dma.spu.from_ram();
        if !matches!(self.spu_bus_control, 0x2009_31e1 | 0x2209_31e1)
            || (!from_ram && self.spu_bus_control != 0x2209_31e1)
        {
            return Err("DMA4 requires normal DEV4_CTRL; stable reads require 0x220931e1".into());
        }
        if mode
            != if from_ram {
                SpuMode::DmaWrite
            } else {
                SpuMode::DmaRead
            }
        {
            return Err("DMA4 direction does not match SPU transfer mode".into());
        }
        while self.spu_transfer.can_dma_word(from_ram) {
            let Some(address) = self.dma.spu.next_word(self.spu_transfer.dma_request()) else {
                break;
            };
            if address >= 0x0080_0000 {
                self.dma.bus_error();
                self.interrupts.set_line(Source::Dma, self.dma.irq_line());
                return Err(format!(
                    "DMA4 address {address:#08x} is outside the RAM mirror region"
                )
                .into());
            }
            let physical = address & 0x001f_fffc;
            if from_ram {
                let word = self.ram.read(physical, Width::Word)?;
                self.spu_transfer.dma_write(word)?;
            } else {
                let word = self.spu_transfer.dma_read()?;
                self.ram.write(physical, Width::Word, word)?;
            }
            let slice_ended = self.dma.spu.finish_word()?;
            if slice_ended && (!self.dma.spu.active() || self.dma.interrupt() & (1 << 4) != 0) {
                self.dma.complete(4)?;
                self.interrupts.set_line(Source::Dma, self.dma.irq_line());
            }
        }
        Ok(())
    }

    fn read_spu_port(&self, address: u32, width: Width) -> Result<u32, BusError> {
        if address as usize & (width.bytes() - 1) != 0 {
            return Err(error(address, "unaligned SPU port read"));
        }
        let offset = Self::physical(address);
        let half = self
            .read_spu_half(offset & !1)
            .map_err(|e| error(address, &e.to_string()))?;
        Ok(match width {
            Width::Byte => u32::from((half >> ((offset & 1) * 8)) & 255),
            Width::Half => u32::from(half),
            Width::Word => {
                u32::from(half)
                    | (u32::from(
                        self.read_spu_half(offset + 2)
                            .map_err(|e| error(address, &e.to_string()))?,
                    ) << 16)
            }
        })
    }

    fn write_spu_port(&mut self, address: u32, width: Width, value: u32) -> Result<(), BusError> {
        if width == Width::Byte && address & 1 != 0 {
            return Ok(());
        }
        if address as usize & (width.bytes() - 1) != 0 {
            return Err(error(address, "unaligned SPU port write"));
        }
        let offset = Self::physical(address);
        // Even-address SB sends the low halfword, not a read/modify/write byte.
        self.write_spu_half(offset, value as u16)
            .map_err(|e| error(address, &e.to_string()))?;
        if width == Width::Word {
            self.write_spu_half(offset + 2, (value >> 16) as u16)
                .map_err(|e| error(address, &e.to_string()))?;
        }
        Ok(())
    }
    pub fn advance_cpu(&mut self, clocks: u32) -> crate::Result<()> {
        self.timers.advance_cpu(clocks, &mut self.interrupts)
    }
    pub(crate) fn require_scanline_clocks(&self) -> crate::Result<()> {
        self.timers.require_scanline_clocks()
    }
    pub fn advance_dotclocks(&mut self, clocks: u32) -> crate::Result<()> {
        self.timers.advance_dotclocks(clocks, &mut self.interrupts)
    }
    pub fn set_hblank(&mut self, active: bool) -> crate::Result<()> {
        self.timers.set_hblank(active, &mut self.interrupts)
    }
    pub fn set_vblank(&mut self, active: bool) {
        self.timers.set_vblank(active);
        self.interrupts.set_line(Source::Vblank, active);
    }

    fn physical(address: u32) -> u32 {
        match address >> 29 {
            4 | 5 => address & 0x1fff_ffff,
            _ => address,
        }
    }
    fn scratch_offset(address: u32, width: Width) -> Result<Option<usize>, BusError> {
        let physical = Self::physical(address);
        if (0x1f80_0000..0x1f80_0400).contains(&physical) {
            // Scratchpad has no uncached KSEG1 alias on the PSX.
            if address >> 29 == 5 {
                return Err(error(address, "uncached scratchpad is unsupported"));
            }
            let offset = (physical - 0x1f80_0000) as usize;
            if offset & (width.bytes() - 1) != 0 || offset + width.bytes() > 1024 {
                return Err(error(address, "unaligned scratchpad access"));
            }
            return Ok(Some(offset));
        }
        Ok(None)
    }
    fn require_word_port(address: u32, width: Width) -> Result<(), BusError> {
        if address & 3 != 0 || width == Width::Byte {
            return Err(error(
                address,
                "only aligned halfword/word access is implemented for this I/O register",
            ));
        }
        Ok(())
    }
}

impl Bus for Interconnect {
    fn set_cache_isolation(&mut self, isolated: bool) -> Result<(), &'static str> {
        self.cache.set_isolation(isolated)
    }

    fn fetch(&mut self, address: u32) -> Result<u32, BusError> {
        if (0x1f80_1000..0x1f80_2000).contains(&Self::physical(address)) {
            return Err(error(address, "instruction fetch from I/O is unsupported"));
        }
        if Self::scratch_offset(address, Width::Word)?.is_some() {
            return Err(error(address, "scratchpad is not executable"));
        }
        match self.cache.fetch(address)? {
            cache::Fetch::Hit(value) => Ok(value),
            cache::Fetch::Direct => self.read_mapped(address, Width::Word),
            cache::Fetch::Fill { start, end } => {
                let mut words = [0; 4];
                for (i, word) in words.iter_mut().enumerate().take(end).skip(start) {
                    *word = self.read_mapped((address & !15) + i as u32 * 4, Width::Word)?;
                }
                self.cache.fill(address, start, end, words);
                Ok(words[((address & 15) >> 2) as usize])
            }
        }
    }

    fn read(&mut self, address: u32, width: Width) -> Result<u32, BusError> {
        if address == cache::CONTROL {
            return self.cache.read_control(width);
        }
        if self.cache.isolated_access(address) {
            return self.cache.read_isolated(address, width);
        }
        self.read_mapped(address, width)
    }

    fn write(&mut self, address: u32, width: Width, value: u32) -> Result<(), BusError> {
        if address == cache::CONTROL {
            return self.cache.write_control(width, value);
        }
        if self.cache.isolated_access(address) {
            return self.cache.write_isolated(address, width, value);
        }
        self.write_mapped(address, width, value)
    }

    fn write_masked(&mut self, address: u32, value: u32, lanes: u8) -> Result<(), BusError> {
        if address == cache::CONTROL || self.cache.isolated_access(address) {
            return Err(error(
                address,
                "masked cache/control stores are unsupported",
            ));
        }
        self.write_masked_mapped(address, value, lanes)
    }
}

fn error(address: u32, detail: &str) -> BusError {
    BusError {
        address,
        detail: detail.into(),
    }
}
