//! XA decode-to-SPU queue. Caller supplies sector arrivals and output boundaries.
use super::cd_audio::{Model as Resampling, XaAudio};
use crate::{
    xa::{Arithmetic, Histories, Stream},
    Result,
};
use std::collections::VecDeque;

#[derive(Clone, Copy, Debug)]
pub enum Model {
    /// Inspected emulator: reject admission above ten buffered frames by
    /// dropping that sector without advancing histories; empty output is zero.
    /// Physical queue capacity, latency and lifecycle remain unverified.
    EmulatorReference,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Admission {
    Queued { frames: usize },
    Muted,
    Dropped { buffered_frames: usize },
}

#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct Statistics {
    pub queued_frames: usize,
    pub admitted_sectors: u64,
    pub muted_sectors: u64,
    pub dropped_sectors: u64,
    pub consumed_frames: u64,
    pub empty_frames: u64,
    pub resets: u64,
    pub reset_discarded_frames: u64,
}

pub struct Queue {
    audio: Option<XaAudio>,
    selection_released: bool,
    arithmetic: Arithmetic,
    resampling: Resampling,
    frames: VecDeque<[i16; 2]>,
    model: Model,
    statistics: Statistics,
}

impl Queue {
    pub fn new(
        stream: Stream,
        arithmetic: Arithmetic,
        histories: Histories,
        resampling: Resampling,
        model: Model,
    ) -> Result<Self> {
        Ok(Self {
            audio: Some(XaAudio::new(stream, arithmetic, histories, resampling)?),
            selection_released: false,
            arithmetic,
            resampling,
            frames: VecDeque::new(),
            model,
            statistics: Statistics::default(),
        })
    }

    /// Setfilter releases file/channel selection without resetting histories
    /// or output. EOF does this at sector admission, even when backlog drops it.
    pub fn release_selection(&mut self) {
        self.selection_released = true;
    }

    pub fn statistics(&self) -> Statistics {
        Statistics {
            queued_frames: self.frames.len(),
            ..self.statistics
        }
    }
    /// Explicit emulator-reference seek/read reset: discard buffered output
    /// and release stream binding. The next selected sector starts zero
    /// predictor/interpolation history, retaining arithmetic/model choices.
    pub fn reset(&mut self) -> usize {
        let discarded = self.frames.len();
        self.frames.clear();
        self.audio = None;
        self.selection_released = true;
        self.statistics.resets += 1;
        self.statistics.reset_discarded_frames += discarded as u64;
        discarded
    }

    /// Input must already be selected by the drive. Validate on a private
    /// decoder candidate so errors and dropped sectors preserve both histories.
    /// Muting affects newly admitted sectors, not frames already queued.
    pub fn sector(&mut self, bytes: &[u8], muted: bool) -> Result<Admission> {
        if bytes.len() != 2336 {
            return Err("XA queue requires one whole selected sector".into());
        }
        let stream = Stream {
            file: bytes[0],
            channel: bytes[1],
            coding: bytes[3],
        };
        let mut candidate = match &self.audio {
            Some(audio) => {
                let mut candidate = audio.clone();
                candidate.select_stream(stream, self.selection_released)?;
                candidate
            }
            None => XaAudio::new(
                stream,
                self.arithmetic,
                Histories::default(),
                self.resampling,
            )?,
        };
        let blocked = match self.model {
            Model::EmulatorReference => self.frames.len() > 10,
        };
        if blocked || muted {
            let binding = candidate.clone();
            candidate.muted_sector(bytes)?;
            self.selection_released = bytes[2] & 0x80 != 0;
            if blocked {
                self.audio = Some(binding);
                self.statistics.dropped_sectors += 1;
                return Ok(Admission::Dropped {
                    buffered_frames: self.frames.len(),
                });
            }
            self.audio = Some(candidate);
            self.statistics.muted_sectors += 1;
            return Ok(Admission::Muted);
        }
        let frames = candidate.sector(bytes)?;
        self.selection_released = bytes[2] & 0x80 != 0;
        let count = frames.len();
        self.frames.extend(frames);
        self.audio = Some(candidate);
        self.statistics.admitted_sectors += 1;
        Ok(Admission::Queued { frames: count })
    }

    /// Before current drive-volume coefficients are applied. Empty output does
    /// not advance decoder/interpolation history or invent buffered samples.
    pub fn preview(&self) -> [i16; 2] {
        self.frames.front().copied().unwrap_or([0; 2])
    }

    /// Commit only after the downstream consumer accepts the scheduled frame.
    pub fn consume(&mut self) {
        if self.frames.pop_front().is_some() {
            self.statistics.consumed_frames += 1;
        } else {
            self.statistics.empty_frames += 1;
        }
    }
}
