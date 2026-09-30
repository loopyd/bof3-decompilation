//! Explicit reference clock for SPU output and NTSC VBlank, without GPU/CD
//! execution or CPU stalls. Origin is the caller-selected activation boundary.
use super::{interconnect::Interconnect, spu_clock, spu_mixer::Inputs};
use crate::Result;
use serde::Serialize;

const SAMPLE_TICKS: u32 = 768;
// Pinned PCSX-Redux Counters::init/calculateHsync, without VSyncWA.
const LINE_TICKS: u32 = 33_868_800 / (60 * 263);
const QUEUE_LIMIT: usize = 4096;

#[derive(Clone, Copy, Debug, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum Model {
    PcsxReduxNtsc,
}

#[derive(Debug, Serialize)]
pub struct Clock {
    model: Model,
    ticks: u64,
    frames: u64,
    vblanks: u64,
    line: u32,
    sample_remaining: u32,
    line_remaining: u32,
    #[serde(skip)]
    pending: Vec<[i16; 2]>,
}

impl Clock {
    pub fn new(model: Model) -> Self {
        Self {
            model,
            ticks: 0,
            frames: 0,
            vblanks: 0,
            line: 0,
            sample_remaining: SAMPLE_TICKS,
            line_remaining: LINE_TICKS,
            pending: Vec::new(),
        }
    }
    pub fn ticks(&self) -> u64 {
        self.ticks
    }
    pub fn frames(&self) -> u64 {
        self.frames
    }
    pub fn vblanks(&self) -> u64 {
        self.vblanks
    }
    pub fn take_frames(&mut self) -> Vec<[i16; 2]> {
        std::mem::take(&mut self.pending)
    }

    /// Advance implemented devices to each output/line boundary. Guest writes
    /// precede this call; IRQ delivery follows at the next CPU step. Errors are
    /// terminal: elapsed work is not rolled back. Drain the bounded PCM queue
    /// regularly. CD/external input, HBlank duration and dot clocks are absent.
    pub fn advance(
        &mut self,
        bus: &mut Interconnect,
        transfer: &mut spu_clock::Clock,
        ticks: u32,
    ) -> Result<()> {
        bus.require_scanline_clocks()?;
        self.ticks
            .checked_add(u64::from(ticks))
            .ok_or("output clock overflow")?;
        let mut left = ticks;
        while left != 0 {
            let step = left.min(self.sample_remaining).min(self.line_remaining);
            transfer.advance(bus, step)?;
            bus.advance_cpu(step)?;
            self.ticks += u64::from(step);
            self.sample_remaining -= step;
            self.line_remaining -= step;
            left -= step;
            if self.line_remaining == 0 {
                self.line_remaining = LINE_TICKS;
                self.line = (self.line + 1) % 263;
                // One reference scanline pulse advances timer 1. Timer 0
                // HBlank gating is rejected because pulse width is unmodeled.
                bus.set_hblank(true)?;
                bus.set_hblank(false)?;
                if self.line == 243 {
                    self.vblanks += 1;
                    bus.set_vblank(true);
                } else if self.line == 0 {
                    bus.set_vblank(false);
                }
            }
            if self.sample_remaining == 0 {
                self.sample_remaining = SAMPLE_TICKS;
                if self.pending.len() == QUEUE_LIMIT {
                    return Err("SPU output queue full; drain frames before advancing".into());
                }
                self.pending.push(bus.step_spu_output(Inputs::default())?);
                self.frames += 1;
            }
        }
        Ok(())
    }
}
