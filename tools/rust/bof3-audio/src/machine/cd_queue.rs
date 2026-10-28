//! XA/CDDA-to-SPU queue. Caller supplies selected sectors and output boundaries.
use super::cd_audio::{Model as Resampling, Volume, XaAudio};
use crate::{
    xa::{Arithmetic, Histories, Stream},
    Result,
};
use std::collections::VecDeque;

/// Native stream-1 ring size. Whole-batch admission leaves one frame free.
pub const CAPACITY: usize = 16384;

#[derive(Clone, Copy, Debug)]
pub enum Model {
    /// Native whole-batch capacity predicate; the caller supplies wait outcomes.
    /// This models neither the host's 200 ms deadline nor hardware timing.
    EmulatorReference,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Admission {
    Queued {
        frames: usize,
    },
    Pending {
        frames: usize,
        buffered_frames: usize,
    },
    Muted,
    Dropped {
        buffered_frames: usize,
    },
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum WaitOutcome {
    Retry,
    TimedOut,
}

#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct Statistics {
    pub queued_frames: usize,
    pub pending_frames: usize,
    pub admitted_sectors: u64,
    pub muted_sectors: u64,
    pub dropped_sectors: u64,
    pub consumed_frames: u64,
    pub empty_frames: u64,
    pub resets: u64,
    pub predictor_resets: u64,
    pub reset_discarded_frames: u64,
}

pub struct Queue {
    audio: Option<XaAudio>,
    arithmetic: Arithmetic,
    resampling: Resampling,
    frames: VecDeque<[i16; 2]>,
    pending: Option<Vec<[i16; 2]>>,
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
            arithmetic,
            resampling,
            frames: VecDeque::new(),
            pending: None,
            model,
            statistics: Statistics::default(),
        })
    }

    pub fn statistics(&self) -> Statistics {
        Statistics {
            queued_frames: self.frames.len(),
            pending_frames: self.pending.as_ref().map_or(0, Vec::len),
            ..self.statistics
        }
    }
    /// Explicit fresh context: discard queued and pending output and forget
    /// the decoder context. This is not a drive command. The next sector starts
    /// zero predictor/interpolation history, retaining arithmetic/model choices.
    pub fn reset(&mut self) -> usize {
        let discarded = self.frames.len() + self.pending.take().map_or(0, |frames| frames.len());
        self.frames.clear();
        self.audio = None;
        self.statistics.resets += 1;
        self.statistics.reset_discarded_frames += discarded as u64;
        discarded
    }

    /// Reset only ADPCM predictors. Native sector feeds
    /// finish their wait before the CPU can service a command.
    pub fn reset_predictors(&mut self) -> Result<()> {
        if self.pending.is_some() {
            return Err("CD audio queue has a pending submission".into());
        }
        if let Some(audio) = self.audio.as_mut() {
            audio.reset_predictors();
        }
        self.statistics.predictor_resets += 1;
        Ok(())
    }

    /// Input must already be selected by the drive. Validate on a private
    /// decoder candidate so errors preserve both histories. A pending submission
    /// must finish before another sector. Decode/interpolation precede the wait;
    /// a timeout discards output while retaining both advanced histories.
    /// Mute precedes admission; gain is sampled before a submission can wait.
    pub fn sector(
        &mut self,
        bytes: &[u8],
        command_muted: bool,
        volume: &Volume,
    ) -> Result<Admission> {
        if self.pending.is_some() {
            return Err("CD audio queue has a pending submission".into());
        }
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
                candidate.select_stream(stream)?;
                candidate
            }
            None => XaAudio::new(
                stream,
                self.arithmetic,
                Histories::default(),
                self.resampling,
            )?,
        };
        if command_muted || volume.xa_muted() {
            candidate.muted_sector(bytes)?;
            self.audio = Some(candidate);
            self.statistics.muted_sectors += 1;
            return Ok(Admission::Muted);
        }
        let frames = candidate.sector(bytes)?;
        if frames.len() >= CAPACITY {
            return Err("XA batch exceeds queue capacity".into());
        }
        self.pending = Some(
            frames
                .into_iter()
                .map(|frame| volume.apply_xa(frame))
                .collect(),
        );
        self.audio = Some(candidate);
        self.finish_submission(WaitOutcome::Retry)
    }

    /// One already-selected raw 2352-byte CDDA sector, little-endian stereo at
    /// 44.1 kHz. It bypasses XA decoding/interpolation without changing either
    /// history. The caller owns track selection and drive timing.
    pub fn cdda_sector(
        &mut self,
        bytes: &[u8],
        command_muted: bool,
        volume: &Volume,
    ) -> Result<Admission> {
        if self.pending.is_some() {
            return Err("CD audio queue has a pending submission".into());
        }
        if bytes.len() != 2352 {
            return Err("CDDA queue requires one whole selected raw sector".into());
        }
        if command_muted {
            self.statistics.muted_sectors += 1;
            return Ok(Admission::Muted);
        }
        self.pending = Some(
            bytes
                .as_chunks::<4>()
                .0
                .iter()
                .map(|frame| {
                    volume.apply_cdda([
                        i16::from_le_bytes([frame[0], frame[1]]),
                        i16::from_le_bytes([frame[2], frame[3]]),
                    ])
                })
                .collect(),
        );
        self.finish_submission(WaitOutcome::Retry)
    }

    /// Recheck capacity at a caller-supplied wake/deadline boundary. Available
    /// room wins even at the deadline, as in the native wait predicate. Retry
    /// while full retains the batch; timeout while full discards only its output.
    pub fn finish_submission(&mut self, outcome: WaitOutcome) -> Result<Admission> {
        let count = self
            .pending
            .as_ref()
            .ok_or("CD audio queue has no pending submission")?
            .len();
        let room = match self.model {
            Model::EmulatorReference => self.frames.len() + count < CAPACITY,
        };
        if room {
            self.frames.extend(self.pending.take().unwrap());
            self.statistics.admitted_sectors += 1;
            Ok(Admission::Queued { frames: count })
        } else if outcome == WaitOutcome::TimedOut {
            self.pending = None;
            self.statistics.dropped_sectors += 1;
            Ok(Admission::Dropped {
                buffered_frames: self.frames.len(),
            })
        } else {
            Ok(Admission::Pending {
                frames: count,
                buffered_frames: self.frames.len(),
            })
        }
    }

    /// Post-drive-gain frame, ready for SPU CD input. Empty output does
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
