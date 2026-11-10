//! Channel-major native SPU work, independent of CPU and output consumption.
use super::Interconnect;
use crate::{
    machine::{
        spu::mixer::Inputs,
        spu::sample::Model,
        spu::voice::envelope,
        spu::voice::ports::Frame,
        spu::voice::{ModulationRole, NativeCapture, NativeChannel, Output, VOICES},
    },
    Result,
};

const FRAMES: usize = 45;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ProducerProgress {
    Advanced,
    BatchReady,
}

/// A completed voice/reverb source range, before queue admission, callback CD
/// summing, mute and mono. Taking it does not consume any output frames.
pub struct NativeBatch {
    pub source_frame: u64,
    pub key_on_mask: u32,
    /// Interpolation steps installed at each channel's admission boundary.
    pub pre_sample_steps: [u32; VOICES],
    pub frames: [[i16; 2]; FRAMES],
}

enum Phase {
    Start,
    Noise(usize),
    Channel {
        index: usize,
        sample: usize,
        admitted: Option<NativeChannel>,
    },
    CaptureCd,
    CaptureAdvance,
    Mix(usize),
    CaptureIrq,
    Ready,
}

pub(super) struct Producer {
    phase: Phase,
    source_frame: u64,
    key_on_mask: u32,
    pre_sample_steps: [u32; VOICES],
    capture_start: u16,
    noise: [i16; FRAMES],
    modulation: [i32; FRAMES],
    voices: Box<[Frame; FRAMES]>,
    output: [[i16; 2]; FRAMES],
    failed: bool,
}

fn empty_frame() -> Frame {
    Frame {
        sample_model: Model::EmulatorReference,
        voices: [Output::default(); VOICES],
        reverb_mask: 0,
        noise_level: None,
    }
}

impl Producer {
    fn new() -> Self {
        Self {
            phase: Phase::Start,
            source_frame: 0,
            key_on_mask: 0,
            pre_sample_steps: [0; VOICES],
            capture_start: 0,
            noise: [0; FRAMES],
            modulation: [0; FRAMES],
            voices: Box::new(std::array::from_fn(|_| empty_frame())),
            output: [[0; 2]; FRAMES],
            failed: false,
        }
    }

    fn advance(&mut self, bus: &mut Interconnect, cycle: u64) -> Result<ProducerProgress> {
        match &mut self.phase {
            Phase::Start => {
                self.key_on_mask = 0;
                self.capture_start = bus.spu_transfer.capture_position();
                for frame in self.voices.iter_mut() {
                    *frame = empty_frame();
                }
                self.phase = Phase::Noise(0);
            }
            Phase::Noise(sample) => {
                let control = bus.spu_transfer.read(10)?;
                let level = bus
                    .spu_voices
                    .as_mut()
                    .unwrap()
                    .native_noise_sample(control)?;
                self.noise[*sample] = level;
                self.voices[*sample].noise_level = Some(level);
                *sample += 1;
                if *sample == FRAMES {
                    self.phase = Phase::Channel {
                        index: 0,
                        sample: 0,
                        admitted: None,
                    };
                }
            }
            Phase::Channel {
                index,
                sample,
                admitted,
            } => {
                let control = bus.spu_transfer.read(10)?;
                if let Some(channel) = admitted {
                    let preceding = (channel.role() == ModulationRole::Target)
                        .then_some(self.modulation[*sample]);
                    let value = bus.spu_voices.as_mut().unwrap().native_channel_sample(
                        channel,
                        bus.spu_transfer.ram(),
                        control,
                        self.noise[*sample],
                        preceding,
                    )?;
                    if channel.role() == ModulationRole::Source {
                        self.modulation[*sample] = value.modulation;
                    }
                    if value.modulation_consumed {
                        self.modulation[*sample] = 0;
                    }
                    if let Some(address) = value.output.fetched_address {
                        bus.spu_transfer
                            .observe_voice_fetch(address, value.output.fetched_loop_address);
                    }
                    let voice = channel.voice();
                    if matches!(voice, 1 | 3) {
                        let silence = match value.capture {
                            NativeCapture::Sample => {
                                bus.spu_transfer.native_capture_voice(
                                    voice,
                                    self.capture_start,
                                    *sample,
                                    value.output.mono,
                                )?;
                                None
                            }
                            NativeCapture::DeferredSilence if *sample + 1 != FRAMES => None,
                            NativeCapture::DeferredSilence | NativeCapture::SilenceAll => Some(0),
                            NativeCapture::SilenceRemainder => Some(*sample),
                        };
                        if let Some(first) = silence {
                            for position in first..FRAMES {
                                bus.spu_transfer.native_capture_voice(
                                    voice,
                                    self.capture_start,
                                    position,
                                    0,
                                )?;
                            }
                        }
                    }
                    self.voices[*sample].voices[voice] = value.output;
                    if value.reverb_active {
                        self.voices[*sample].reverb_mask |= 1 << voice;
                    }
                    *sample += 1;
                    if value.finished || *sample == FRAMES {
                        *index += 1;
                        *sample = 0;
                        *admitted = None;
                        if *index == VOICES {
                            self.phase = Phase::CaptureCd;
                        }
                    }
                } else {
                    let channel = bus
                        .spu_voices
                        .as_mut()
                        .unwrap()
                        .begin_native_channel(*index, control)?;
                    self.pre_sample_steps[*index] = channel.installed_step();
                    if channel.role() == ModulationRole::Source {
                        self.modulation.fill(0);
                    }
                    if channel.started() {
                        self.key_on_mask |= 1 << *index;
                    }
                    *admitted = Some(channel);
                }
            }
            Phase::CaptureCd => {
                // The native worker holds the CD capture mutex for this entire
                // copy, and samples the feeding predicate once before the loop.
                for _ in 0..FRAMES {
                    let cd = bus
                        .cd_audio
                        .as_ref()
                        .map_or(Some([0; 2]), |queue| queue.preview_capture(cycle));
                    let Some(cd) = cd else { break };
                    bus.spu_transfer.native_capture_cd(cd)?;
                    if let Some(queue) = &mut bus.cd_audio {
                        queue.consume_capture();
                    }
                }
                self.phase = Phase::CaptureAdvance;
            }
            Phase::CaptureAdvance => {
                bus.spu_transfer.native_capture_advance(FRAMES as u16);
                self.phase = Phase::Mix(0);
            }
            Phase::Mix(sample) => {
                let control = bus.spu_transfer.read(10)?;
                let sends =
                    bus.spu_mixer
                        .prepare(&self.voices[*sample], control, Inputs::default())?;
                let wet = bus.spu_reverb.as_mut().unwrap().tick(
                    bus.spu_transfer.ram_mut(),
                    sends.reverb,
                    control & 0x80 != 0,
                    bus.spu_mixer.reverb_gain(),
                )?;
                self.output[*sample] = bus.spu_mixer.finish_native_voice(sends, wet)?;
                *sample += 1;
                if *sample == FRAMES {
                    self.phase = Phase::CaptureIrq;
                }
            }
            Phase::CaptureIrq => {
                for _ in 0..FRAMES {
                    bus.spu_transfer.native_capture_irq();
                }
                self.phase = Phase::Ready;
            }
            Phase::Ready => return Ok(ProducerProgress::BatchReady),
        }
        Ok(if matches!(self.phase, Phase::Ready) {
            ProducerProgress::BatchReady
        } else {
            ProducerProgress::Advanced
        })
    }
}

impl Interconnect {
    pub(crate) fn configure_native_audio(&mut self) -> Result<()> {
        if self.reference_timers.is_some() || self.gpu_status.is_none() {
            return Err(
                "fresh native audio requires unstarted counters and configured GPU status".into(),
            );
        }
        self.configure_native_spu_producer()?;
        self.configure_reference_timers()
    }

    pub(crate) fn native_callback_frame(&mut self, voice: [i16; 2]) -> Result<[f32; 2]> {
        let cd = self
            .cd_audio
            .as_ref()
            .map_or([0; 2], |queue| queue.preview());
        let result = self.spu_mixer.finish_native_callback(voice, cd)?;
        if let Some(queue) = &mut self.cd_audio {
            queue.consume();
        }
        Ok(result)
    }

    pub(crate) fn native_discard_cd(&mut self, frames: usize) {
        if let Some(queue) = &mut self.cd_audio {
            for _ in 0..frames {
                queue.consume();
            }
        }
    }

    /// Select a fresh producer before any SPU work. Voice, envelope, delay,
    /// noise, reverb and host settings remain explicit caller-owned inputs.
    /// This creates no consumer, queue pacing, CPU stalls or host-time schedule.
    pub fn configure_native_spu_producer(&mut self) -> Result<()> {
        if self.spu_producer.is_some() || self.spu_work_started {
            return Err("native SPU producer requires an unstarted SPU".into());
        }
        let voices = self
            .spu_voices
            .as_ref()
            .ok_or("configure SPU voices before producer")?;
        voices.require_native_context()?;
        if voices.sample_model() != Model::EmulatorReference
            || voices.envelope_model() != envelope::Model::EmulatorReference
        {
            return Err("native SPU producer requires native sample and envelope models".into());
        }
        voices.require_disable_model()?;
        if voices.key_on_delay().is_none() || self.spu_reverb.is_none() {
            return Err("native SPU producer requires explicit delay and reverb models".into());
        }
        self.spu_transfer.require_ram_control()?;
        self.spu_mixer.require_model(Model::EmulatorReference)?;
        self.spu_producer = Some(Box::new(Producer::new()));
        Ok(())
    }

    /// Perform one producer operation. The caller selects CPU/DSP interleaving.
    /// A ready batch remains staged until taken, without repeating device work.
    /// Errors retain changed state and prevent further producer advancement.
    pub fn step_native_spu_producer(&mut self, cycle: u64) -> Result<ProducerProgress> {
        let mut producer = self
            .spu_producer
            .take()
            .ok_or("native SPU producer is not configured")?;
        let result = if producer.failed {
            Err("native SPU producer is unfinished after a previous failure".into())
        } else {
            self.spu_work_started = true;
            producer.advance(self, cycle)
        };
        producer.failed |= result.is_err();
        self.spu_producer = Some(producer);
        result
    }

    pub fn take_native_spu_batch(&mut self) -> Result<NativeBatch> {
        let producer = self
            .spu_producer
            .as_mut()
            .ok_or("native SPU producer is not configured")?;
        if producer.failed || !matches!(producer.phase, Phase::Ready) {
            return Err("native SPU producer has no completed batch".into());
        }
        let next = producer
            .source_frame
            .checked_add(FRAMES as u64)
            .ok_or("native SPU source position overflow")?;
        let result = NativeBatch {
            source_frame: producer.source_frame,
            key_on_mask: producer.key_on_mask,
            pre_sample_steps: producer.pre_sample_steps,
            frames: producer.output,
        };
        producer.source_frame = next;
        producer.phase = Phase::Start;
        Ok(result)
    }
}
