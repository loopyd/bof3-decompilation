//! Functional 768-tick SPU/CD sample clock and NTSC VBlank.
//! Native-reference arithmetic retains this schedule; it does not select native
//! worker batching, host-queue pacing or CPU stalls. Original-ROM execution
//! retains its counter owner and requires an explicit functional sample phase.
use super::{clock, interconnect::Interconnect};
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

#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum Collection {
    Retain,
    Discard,
}

#[derive(Debug, Serialize)]
pub struct Clock {
    model: Model,
    collection: Collection,
    #[serde(skip_serializing_if = "Option::is_none")]
    reference_phase: Option<ReferencePhase>,
    ticks: u64,
    frames: u64,
    vblanks: u64,
    line: u32,
    sample_remaining: u32,
    line_remaining: u32,
    #[serde(skip)]
    pending: Vec<[f32; 2]>,
}

#[derive(Debug, Serialize)]
struct ReferencePhase {
    activation_cycle: u64,
    first_sample_ticks: u32,
}

impl Clock {
    pub fn new(model: Model, collection: Collection) -> Self {
        Self {
            model,
            collection,
            reference_phase: None,
            ticks: 0,
            frames: 0,
            vblanks: 0,
            line: 0,
            sample_remaining: SAMPLE_TICKS,
            line_remaining: LINE_TICKS,
            pending: Vec::new(),
        }
    }
    pub(super) fn for_reference(activation_cycle: u64, first_sample_ticks: u32) -> Result<Self> {
        if !(1..=SAMPLE_TICKS).contains(&first_sample_ticks) {
            return Err("reference output first sample must be within 1..=768 ticks".into());
        }
        let mut clock = Self::new(Model::PcsxReduxNtsc, Collection::Retain);
        clock.reference_phase = Some(ReferencePhase {
            activation_cycle,
            first_sample_ticks,
        });
        clock.sample_remaining = first_sample_ticks;
        Ok(clock)
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
    pub fn take_frames(&mut self) -> Vec<[f32; 2]> {
        std::mem::take(&mut self.pending)
    }

    /// Start retaining PCM at the current absolute frame without changing phase
    /// or device state. Discarded frames have already executed all device work.
    pub(super) fn begin_capture(&mut self) -> Result<u64> {
        if self.collection != Collection::Discard {
            return Err("output clock is already retaining PCM".into());
        }
        self.collection = Collection::Retain;
        Ok(self.frames)
    }

    /// Advance implemented devices to each output/line boundary. Guest writes
    /// precede this call; IRQ delivery follows at the next CPU step. Errors are
    /// terminal: elapsed work is not rolled back. Drain the bounded PCM queue
    /// regularly. Configured CD audio advances at sample boundaries. External
    /// input, drive scheduling, HBlank duration and dot clocks are absent.
    /// With an explicit reference phase, only elapsed cycles and PCM advance here;
    /// reference counters retain blanking and timer ownership. The line and
    /// VBlank counts then remain zero. CD callbacks follow this work in Execution.
    pub fn advance(
        &mut self,
        bus: &mut Interconnect,
        elapsed: &mut clock::Clock,
        ticks: u32,
    ) -> Result<()> {
        let scanlines = self.reference_phase.is_none();
        if scanlines {
            bus.require_scanline_clocks()?;
        }
        self.ticks
            .checked_add(u64::from(ticks))
            .ok_or("output clock overflow")?;
        let mut left = ticks;
        while left != 0 {
            let mut step = left.min(self.sample_remaining);
            if scanlines {
                step = step.min(self.line_remaining);
            }
            elapsed.advance(step)?;
            if scanlines {
                bus.advance_cpu(step)?;
                self.line_remaining -= step;
            }
            self.ticks += u64::from(step);
            self.sample_remaining -= step;
            left -= step;
            if scanlines && self.line_remaining == 0 {
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
                if self.collection == Collection::Retain && self.pending.len() == QUEUE_LIMIT {
                    return Err("SPU output queue full; drain frames before advancing".into());
                }
                let cycle = match &self.reference_phase {
                    Some(phase) => phase
                        .activation_cycle
                        .checked_add(self.ticks)
                        .ok_or("output cycle overflow")?,
                    None => elapsed.ticks(),
                };
                let frame = bus.step_spu_scheduled_output(cycle)?;
                if self.collection == Collection::Retain {
                    self.pending.push(frame);
                }
                self.frames += 1;
            }
        }
        Ok(())
    }
}
