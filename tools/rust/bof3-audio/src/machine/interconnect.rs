//! RAM, scratchpad and explicitly modeled I/O. All other devices fail closed.
mod cache;
mod cd;
mod mapped;
mod memory;
mod producer;
mod scheduler;
pub use producer::{NativeBatch, ProducerProgress};
pub use scheduler::DeviceProgress;

use super::{
    bus::{Bus, BusError, Ram, Width},
    cd::host,
    dma::Dma,
    image::executable::Executable,
    image::firmware::Image as Firmware,
    interrupts::{Interrupts, Source},
    spu::mixer::{Inputs as SpuInputs, Mixer},
    spu::noise::Noise,
    spu::readback::{Readback, Settings as ReadbackSettings},
    spu::reverb::{Model as ReverbModel, Reverb},
    spu::transfer::Transfer as SpuTransfer,
    spu::voice::ports,
    timers::Timers,
};

pub struct Interconnect {
    cache: cache::Cache,
    memory_control: memory::Control,
    post_status: Option<u8>,
    ram: Ram,
    firmware: Option<Firmware>,
    cd_host: Option<host::Host>,
    cd_controller: Option<cd::clock::Controller>,
    cd_audio: Option<crate::machine::cd::queue::Queue>,
    cd_command_muted: bool,
    cd_bus_control: u32,
    common_delay: u32,
    scratchpad: [u8; 1024],
    interrupts: Interrupts,
    dma: Dma,
    timers: Timers,
    reference_timers: Option<super::timers::reference::Counters>,
    gpu_status: Option<super::gpu_status::Status>,
    spu_transfer: SpuTransfer,
    dma_observation: Option<super::dma::Observation>,
    spu_voices: Option<ports::Ports>,
    spu_readback: Option<Readback>,
    reference_access: Option<u64>,
    scheduler: scheduler::Scheduler,
    spu_mixer: Mixer,
    spu_reverb: Option<Reverb>,
    spu_producer: Option<Box<producer::Producer>>,
    spu_work_started: bool,
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
            cd_controller: None,
            cd_audio: None,
            cd_command_muted: false,
            cd_bus_control: 0,
            common_delay: 0,
            scratchpad: [0; 1024],
            interrupts: Interrupts::default(),
            dma: Dma::default(),
            timers: Timers::default(),
            reference_timers: None,
            gpu_status: None,
            spu_transfer: SpuTransfer::default(),
            dma_observation: None,
            spu_voices: None,
            spu_readback: None,
            reference_access: None,
            scheduler: scheduler::Scheduler::default(),
            spu_mixer: Mixer::default(),
            spu_reverb: None,
            spu_producer: None,
            spu_work_started: false,
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
    /// Opt in to bounded, ordered successful DMA3/4 RAM accesses. No extra bus
    /// reads or clock changes occur. Overflow is reported without stopping DMA.
    pub fn enable_dma_observation(&mut self, limit: usize) -> crate::Result<()> {
        if self.dma_observation.is_some() {
            return Err("DMA observation already enabled".into());
        }
        self.dma_observation = Some(super::dma::Observation::new(limit)?);
        Ok(())
    }
    /// Drain accesses and overflow for the interval, retaining the enabled limit.
    /// None means observation has not been enabled.
    pub fn take_dma_observation(&mut self) -> Option<super::dma::Observation> {
        self.dma_observation
            .as_mut()
            .map(super::dma::Observation::take)
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
        sample: crate::machine::spu::sample::Model,
        envelope: crate::machine::spu::voice::envelope::Model,
    ) -> crate::Result<()> {
        if self.spu_voices.is_some() {
            return Err("SPU voices already configured".into());
        }
        self.spu_mixer.configure_sweeps(envelope)?;
        self.spu_voices = Some(ports::Ports::new(sample, envelope));
        Ok(())
    }

    /// Select immutable host settings before rendering native-reference output.
    pub fn configure_spu_native_output(
        &mut self,
        settings: crate::machine::spu::mixer::NativeSettings,
    ) -> crate::Result<()> {
        let voices = self
            .spu_voices
            .as_ref()
            .ok_or("configure SPU voices before native output")?;
        if voices.sample_model() != crate::machine::spu::sample::Model::EmulatorReference {
            return Err("native output requires native-reference sample voices".into());
        }
        self.spu_mixer.configure_native(settings)
    }

    pub fn spu_native_output_settings(&self) -> Option<crate::machine::spu::mixer::NativeSettings> {
        self.spu_mixer.native_settings()
    }

    pub fn configure_spu_key_on_delay(&mut self, frames: i32) -> crate::Result<()> {
        self.spu_voices
            .as_mut()
            .ok_or("configure SPU voices before key-on delay")?
            .configure_key_on_delay(frames)
    }

    pub fn spu_key_on_delay(&self) -> Option<i32> {
        self.spu_voices
            .as_ref()
            .and_then(|voices| voices.key_on_delay())
    }

    pub(crate) fn configure_spu_readback(
        &mut self,
        settings: ReadbackSettings,
    ) -> crate::Result<()> {
        if self.spu_readback.is_some() {
            return Err("SPU reference readback already configured".into());
        }
        let voices = self
            .spu_voices
            .as_ref()
            .ok_or("configure SPU voices before reference readback")?;
        if voices.envelope_model() != crate::machine::spu::voice::envelope::Model::EmulatorReference
        {
            return Err("SPU reference readback requires the explicit emulator ADSR model".into());
        }
        self.spu_readback = Some(Readback::new(settings));
        Ok(())
    }

    pub(crate) fn set_reference_access(&mut self, cycle: Option<u64>) {
        self.reference_access = cycle;
        if let (Some(timers), Some(cycle)) = (&mut self.reference_timers, cycle) {
            timers.set_cycle(cycle);
        }
    }

    pub(crate) fn configure_reference_timers(&mut self) -> crate::Result<()> {
        if self.reference_timers.is_some() {
            return Err("reference root counters already configured".into());
        }
        if self.gpu_status.is_none() {
            return Err("reference root counters require GPU status".into());
        }
        self.reference_timers = Some(super::timers::reference::Counters::default());
        Ok(())
    }

    pub(crate) fn advance_reference_time(&mut self, cycle: u64, branch: bool) {
        if let Some(timers) = &mut self.reference_timers {
            timers.set_cycle(cycle);
            if branch && timers.due() {
                timers.update(
                    &mut self.interrupts,
                    self.gpu_status.as_mut().expect("configured reference GPU"),
                );
            }
        }
    }

    pub(crate) fn reference_update_due(&self, cycle: u64) -> bool {
        self.reference_timers
            .as_ref()
            .is_some_and(|timers| timers.due_at(cycle))
    }

    pub(crate) fn reference_access_updates(
        &self,
        request: &super::cpu::MemoryRequest,
        isolated: bool,
    ) -> bool {
        use super::cpu::MemoryRequest;
        let (address, width, write) = match *request {
            MemoryRequest::Read { address, width } => (address, width, false),
            MemoryRequest::Write { address, width, .. } => (address, width, true),
            MemoryRequest::WriteMasked { .. } => return false,
        };
        if self.reference_timers.is_none()
            || (isolated && matches!(address >> 29, 0 | 4))
            || width == Width::Byte
            || address & 3 != 0
        {
            return false;
        }
        let physical = Self::physical(address);
        (0x1f80_1100..0x1f80_1130).contains(&physical)
            && matches!(physical & 15, 0 | 4 | 8)
            && (write || physical & 15 != 8)
    }

    pub(crate) fn advance_native_time(&mut self, cycle: u64, branch: bool) {
        self.advance_reference_time(cycle, branch);
        if branch && self.spu_transfer.take_irq_request() {
            self.interrupts.set_line(Source::Spu, true);
            self.interrupts.set_line(Source::Spu, false);
        }
    }

    /// Attach reset-state NTSC status/control before starting its frame clock.
    /// This enables no graphics, VRAM transfers or GPU DMA.
    pub fn configure_gpu_status(&mut self) -> crate::Result<()> {
        if self.gpu_status.is_some() {
            return Err("GPU status model already configured".into());
        }
        self.gpu_status = Some(super::gpu_status::Status::default());
        Ok(())
    }

    pub(crate) fn gpu_vblank(&mut self, active: bool) {
        if let Some(status) = &mut self.gpu_status {
            status.set_vblank(active);
        }
    }

    /// Per-voice signals before main gain, mixing, capture, CD and reverb. The
    /// caller supplies frame boundaries; CPU/device clock synchronization is open.
    pub fn step_spu_voices(&mut self) -> crate::Result<ports::Frame> {
        if self.spu_producer.is_some() {
            return Err("functional SPU frames cannot advance the native producer".into());
        }
        let voices = self
            .spu_voices
            .as_mut()
            .ok_or("SPU voice arithmetic models must be configured explicitly")?;
        if self.spu_transfer.read(10)? & 0x8000 == 0 {
            voices.require_disable_model()?;
        }
        let control = self.spu_transfer.read(10)?;
        self.spu_work_started = true;
        let frame = voices.tick(self.spu_transfer.ram(), control)?;
        for voice in &frame.voices {
            if let Some(address) = voice.fetched_address {
                self.spu_transfer
                    .observe_voice_fetch(address, voice.fetched_loop_address);
            }
        }
        self.deliver_spu_irq();
        Ok(frame)
    }

    pub fn configure_spu_irq(
        &mut self,
        model: crate::machine::spu::transfer::IrqModel,
    ) -> crate::Result<()> {
        self.spu_transfer.configure_irq(model)
    }

    pub fn configure_spu_ram_control(
        &mut self,
        model: crate::machine::spu::transfer::RamControlModel,
    ) -> crate::Result<()> {
        self.spu_transfer.configure_ram_control(model)
    }

    fn deliver_spu_irq(&mut self) {
        if self.spu_producer.is_some() {
            return;
        }
        if self.spu_transfer.take_irq_request() {
            self.interrupts.set_line(Source::Spu, true);
            self.interrupts.set_line(Source::Spu, false);
        }
    }

    pub fn configure_spu_noise(
        &mut self,
        state: crate::machine::spu::noise::State,
    ) -> crate::Result<()> {
        self.spu_voices
            .as_mut()
            .ok_or("configure SPU voices before noise")?
            .configure_noise(Noise::new(state)?)
    }

    pub fn configure_spu_disable(&mut self, model: ports::DisableModel) -> crate::Result<()> {
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
    pub fn step_spu_output(&mut self, inputs: SpuInputs) -> crate::Result<[f32; 2]> {
        self.step_spu_output_with_capture(inputs, Some(inputs.cd))
    }

    fn step_spu_output_with_capture(
        &mut self,
        inputs: SpuInputs,
        cd_capture: Option<[i16; 2]>,
    ) -> crate::Result<[f32; 2]> {
        let sample = self
            .spu_voices
            .as_ref()
            .ok_or("configure SPU voices before output")?
            .sample_model();
        self.spu_mixer.require_model(sample)?;
        if sample == crate::machine::spu::sample::Model::EmulatorReference
            && inputs.external != [0; 2]
        {
            return Err("native SPU output has no external input source".into());
        }
        let control = self.spu_transfer.read(10)?;
        if self.spu_reverb.is_none() {
            self.spu_mixer.require_dry(control)?;
        }
        self.spu_transfer
            .require_ram_control()
            .map_err(|error| format!("SPU output capture: {error}"))?;
        let voices = self.step_spu_voices()?;
        let sends = self.spu_mixer.prepare(&voices, control, inputs)?;
        let wet = if let Some(reverb) = &mut self.spu_reverb {
            reverb.tick(
                self.spu_transfer.ram_mut(),
                sends.reverb,
                control & 0x80 != 0,
                self.spu_mixer.reverb_gain(),
            )?
        } else {
            [0; 2]
        };
        let output = self.spu_mixer.finish(sends, wet)?;
        self.spu_transfer
            .capture_frame(cd_capture, [voices.voices[1].mono, voices.voices[3].mono])?;
        self.deliver_spu_irq();
        Ok(output)
    }

    fn read_spu_half(&mut self, physical: u32) -> crate::Result<u16> {
        if (0x1f80_1c00..0x1f80_1d80).contains(&physical) && physical & 15 == 12 {
            if let Some(readback) = &mut self.spu_readback {
                let cycle = self
                    .reference_access
                    .ok_or("reference ENVX read requires a CPU access cycle")?;
                return readback.read(
                    ((physical - 0x1f80_1c00) / 16) as usize,
                    cycle,
                    self.spu_transfer.ram(),
                );
            }
        }
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
        if let Some(readback) = &mut self.spu_readback {
            let cycle = self
                .reference_access
                .ok_or("reference SPU write requires a CPU access cycle")?;
            readback.write(
                physical - 0x1f80_1c00,
                value,
                cycle,
                self.spu_transfer.ram(),
            )?;
        }
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
            if let Some(voices) = self.spu_voices.as_ref() {
                voices.require_disable_model()?;
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

    fn write_spu_dma(&mut self, offset: u32, value: u32) -> crate::Result<()> {
        let mut channel = self.dma.spu.clone();
        channel.write(offset, value)?;
        if offset != 8 || !channel.active() || self.dma.priority() & (1 << 19) == 0 {
            self.dma.spu = channel;
            return Ok(());
        }
        if !matches!(self.spu_bus_control, 0x2009_31e1 | 0x2209_31e1)
            || (!channel.from_ram() && self.spu_bus_control != 0x2209_31e1)
        {
            return Err("DMA4 requires normal DEV4_CTRL; stable reads require 0x220931e1".into());
        }
        self.spu_transfer.require_ram_control()?;
        let (address, words) = channel.transfer();
        let physical = address & 0x001f_fffc;
        let bytes = u64::from(words) * 4;
        if u64::from(physical) + bytes > self.ram.bytes().len() as u64 {
            return Err("DMA4: native contiguous payload exceeds the 2 MiB RAM backing".into());
        }
        let from_ram = channel.from_ram();
        for halfword in 0..words * 2 {
            let at = physical + halfword * 2;
            let value = if from_ram {
                let value = self.ram.read(at, Width::Half)? as u16;
                self.spu_transfer.write_halfword(value)?;
                value
            } else {
                let value = self.spu_transfer.read_halfword()?;
                self.ram.write(at, Width::Half, u32::from(value))?;
                value
            };
            if let Some(observation) = &mut self.dma_observation {
                observation.record(super::dma::RamAccess {
                    channel: 4,
                    width: Width::Half,
                    address: address + halfword * 2,
                    physical_address: at,
                    value: u32::from(value),
                    from_ram,
                });
            }
        }
        channel.advance_registers();
        self.dma.spu = channel;
        self.scheduler.schedule(
            scheduler::Slot::SpuDma,
            self.reference_access.unwrap_or(self.scheduler.cycle()),
            u64::from(words / 2),
        )
    }

    fn read_spu_port(&mut self, address: u32, width: Width) -> Result<u32, BusError> {
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
        let offset = Self::physical(address);
        // Native maps current-volume addresses as ordinary hardware mirrors,
        // outside its SPU handler range. Both byte lanes remain writable.
        if width == Width::Byte
            && (0x1f80_1e00..0x1f80_1e60).contains(&offset)
            && self.spu_voices.as_ref().is_some_and(|ports| {
                ports.sample_model() == crate::machine::spu::sample::Model::EmulatorReference
            })
        {
            let shift = (offset & 1) * 8;
            let old = self
                .read_spu_half(offset & !1)
                .map_err(|e| error(address, &e.to_string()))?;
            let merged = (u32::from(old) & !(0xff << shift)) | ((value & 0xff) << shift);
            return self
                .write_spu_half(offset & !1, merged as u16)
                .map_err(|e| error(address, &e.to_string()));
        }
        if width == Width::Byte && address & 1 != 0 {
            return Ok(());
        }
        if address as usize & (width.bytes() - 1) != 0 {
            return Err(error(address, "unaligned SPU port write"));
        }
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
        self.gpu_vblank(active);
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
