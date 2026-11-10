//! Explicit native voice-queue, master-clock and CPU-goal operations.
//! The caller orders actors and supplies callback or null-clock advances. This
//! module selects no host schedule and infers no elapsed OS wait or sample phase.
use crate::Result;

pub const BATCH_FRAMES: usize = 45;
pub const SUBMISSION_FRAMES: usize = 810;
pub const QUEUE_FRAMES: usize = 2048;
const CPU_HZ: u64 = 33_868_800;
const MAX_AUDIO_LAG: i32 = 44_100 / 4;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ProducerState {
    Staged {
        source_frame: u64,
        staged_frames: u32,
    },
    Enqueued {
        source_frame: u64,
        frames: u32,
    },
    Waiting {
        source_frame: u64,
        frames: u32,
    },
    TimedOut {
        source_frame: u64,
        frames: u32,
    },
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum EnqueueOutcome {
    Retry,
    /// The caller observed the host deadline. Recheck the capacity predicate
    /// before rejecting this attempt, as native wait_for does on return.
    TimedOut,
}

#[derive(Debug)]
pub struct VoiceBlock {
    /// Exactly the requested number of frames, with underflow zeros retained.
    pub frames: Vec<[i16; 2]>,
    pub available: u32,
    pub source_frame: u64,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Dequeue {
    pub available: u32,
    pub source_frame: u64,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct MasterAdvance {
    pub old_frames: u32,
    pub new_frames: u32,
    pub goal: u32,
    /// The trigger generation advanced before the native notification point.
    pub triggered: bool,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum CpuAction {
    Ready,
    Wait,
    Rebase,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct CpuDecision {
    pub action: CpuAction,
    pub cycle: u64,
    pub previous_cycle: u64,
    pub previous_audio_frames: u32,
    pub target: u32,
    pub master_frames: u32,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct CpuWait {
    pub cycle: u64,
    pub goal: u32,
    pub initial_trigger: u32,
}

enum CpuState {
    Ready,
    GoalPending { cycle: u64, goal: u32 },
    Waiting(CpuWait),
}

pub struct Queue {
    ring: Box<[[i16; 2]; QUEUE_FRAMES]>,
    head: usize,
    queued: usize,
    staging: Box<[[i16; 2]; SUBMISSION_FRAMES]>,
    staged: usize,
    enqueue_wait: bool,
    enqueued: u64,
    dequeued: u64,
    master_frames: u32,
    goal: u32,
    previous_goal: u32,
    trigger: u32,
    cpu_frames: u32,
    previous_cycles: u64,
    cpu: CpuState,
}

/// Match SDLAudio's signed configured speed, including its unbounded cap.
pub fn effective_speed(configured: i32) -> u32 {
    if configured <= 0 || configured > 1024 {
        1024
    } else {
        configured as u32
    }
}

fn chunk(frames: usize) -> Result<usize> {
    if !(1..=QUEUE_FRAMES).contains(&frames) {
        return Err("native consumer chunks require 1..=2048 frames".into());
    }
    Ok(frames)
}

impl Queue {
    /// A new audio owner and CPU counter context, before any guest or host work.
    pub fn fresh() -> Self {
        Self {
            ring: Box::new([[0; 2]; QUEUE_FRAMES]),
            head: 0,
            queued: 0,
            staging: Box::new([[0; 2]; SUBMISSION_FRAMES]),
            staged: 0,
            enqueue_wait: false,
            enqueued: 0,
            dequeued: 0,
            master_frames: 0,
            goal: 0,
            previous_goal: 0,
            trigger: 0,
            cpu_frames: 0,
            previous_cycles: 0,
            cpu: CpuState::Ready,
        }
    }

    pub fn queued_frames(&self) -> u32 {
        self.queued as u32
    }

    pub fn staged_frames(&self) -> u32 {
        self.staged as u32
    }

    pub fn source_enqueued(&self) -> u64 {
        self.enqueued
    }

    pub fn source_dequeued(&self) -> u64 {
        self.dequeued
    }

    pub fn master_frames(&self) -> u32 {
        self.master_frames
    }

    pub fn cpu_frames(&self) -> u32 {
        self.cpu_frames
    }

    pub fn previous_cycles(&self) -> u64 {
        self.previous_cycles
    }

    pub fn goal(&self) -> u32 {
        self.goal
    }

    pub fn trigger(&self) -> u32 {
        self.trigger
    }

    pub fn enqueue_pending(&self) -> bool {
        self.staged == SUBMISSION_FRAMES
    }

    pub fn producer_waiting(&self) -> bool {
        self.enqueue_wait
    }

    /// Retain one completed producer batch. Eighteen batches form the native
    /// submission; a blocked submission prevents staging another batch.
    pub fn submit45(&mut self, frames: [[i16; 2]; BATCH_FRAMES]) -> Result<ProducerState> {
        if self.enqueue_pending() {
            return Err(
                "native producer must finish its pending enqueue before another batch".into(),
            );
        }
        let source_frame = self
            .enqueued
            .checked_add(self.staged as u64)
            .ok_or("native producer source coordinate overflow")?;
        source_frame
            .checked_add(BATCH_FRAMES as u64)
            .ok_or("native producer source coordinate overflow")?;
        self.staging[self.staged..self.staged + BATCH_FRAMES].copy_from_slice(&frames);
        self.staged += BATCH_FRAMES;
        if self.enqueue_pending() {
            return self.try_enqueue(false);
        }
        Ok(ProducerState::Staged {
            source_frame,
            staged_frames: self.staged as u32,
        })
    }

    /// Retry or finish an explicitly expired wait. A timeout retains the entire
    /// submission for the worker's next attempt; no source frames are dropped.
    pub fn finish_enqueue(&mut self, outcome: EnqueueOutcome) -> Result<ProducerState> {
        if !self.enqueue_pending() {
            return Err("native producer has no complete pending submission".into());
        }
        if outcome == EnqueueOutcome::TimedOut && !self.enqueue_wait {
            return Err("native enqueue timeout requires an active wait".into());
        }
        self.try_enqueue(outcome == EnqueueOutcome::TimedOut)
    }

    fn try_enqueue(&mut self, timed_out: bool) -> Result<ProducerState> {
        let source_frame = self.enqueued;
        let frames = SUBMISSION_FRAMES as u32;
        if self.queued + SUBMISSION_FRAMES >= QUEUE_FRAMES {
            self.enqueue_wait = !timed_out;
            return Ok(if timed_out {
                ProducerState::TimedOut {
                    source_frame,
                    frames,
                }
            } else {
                ProducerState::Waiting {
                    source_frame,
                    frames,
                }
            });
        }
        let end = self
            .enqueued
            .checked_add(SUBMISSION_FRAMES as u64)
            .ok_or("native enqueue source coordinate overflow")?;
        for (index, frame) in self.staging.iter().enumerate() {
            self.ring[(self.head + self.queued + index) % QUEUE_FRAMES] = *frame;
        }
        self.queued += SUBMISSION_FRAMES;
        self.enqueued = end;
        self.staged = 0;
        self.enqueue_wait = false;
        Ok(ProducerState::Enqueued {
            source_frame,
            frames,
        })
    }

    pub fn dequeue(&mut self, frames: usize) -> Result<VoiceBlock> {
        let requested = chunk(frames)?;
        let available = self.queued.min(requested);
        let end = self
            .dequeued
            .checked_add(available as u64)
            .ok_or("native dequeue source coordinate overflow")?;
        let mut output = vec![[0; 2]; requested];
        for (index, frame) in output[..available].iter_mut().enumerate() {
            *frame = self.ring[(self.head + index) % QUEUE_FRAMES];
        }
        let source_frame = self.dequeued;
        self.consume(available, end);
        Ok(VoiceBlock {
            frames: output,
            available: available as u32,
            source_frame,
        })
    }

    /// Fast-forward discard consumes source frames without retaining PCM.
    pub fn discard(&mut self, frames: usize) -> Result<Dequeue> {
        let available = self.queued.min(chunk(frames)?);
        let end = self
            .dequeued
            .checked_add(available as u64)
            .ok_or("native dequeue source coordinate overflow")?;
        let source_frame = self.dequeued;
        self.consume(available, end);
        Ok(Dequeue {
            available: available as u32,
            source_frame,
        })
    }

    fn consume(&mut self, frames: usize, end: u64) {
        self.head = (self.head + frames) % QUEUE_FRAMES;
        self.queued -= frames;
        self.dequeued = end;
    }

    /// Advance only the selected master-clock actor. The existing wake branch
    /// compares the goal to fetch_add's old total, including uint32 wrap.
    pub fn advance_master(&mut self, frames: u32, speed: i32) -> Result<MasterAdvance> {
        chunk(frames as usize)?;
        let old_frames = self.master_frames;
        self.master_frames = old_frames.wrapping_add(frames * effective_speed(speed));
        let triggered =
            self.goal != self.previous_goal && (self.goal.wrapping_sub(old_frames) as i32) <= 0;
        if triggered {
            self.previous_goal = self.goal;
            self.trigger = self.trigger.wrapping_add(1);
        }
        Ok(MasterAdvance {
            old_frames,
            new_frames: self.master_frames,
            goal: self.goal,
            triggered,
        })
    }

    /// Sample the current master and retain a needed goal admission separately.
    /// CPU frames commit on wait return, while a debt adjustment commits here.
    pub fn cpu_decision(&mut self, cycle: u64) -> Result<CpuDecision> {
        if self.cpu_waiting() {
            return Err(
                "native CPU must finish its audio wait before another counter decision".into(),
            );
        }
        let elapsed = cycle
            .checked_sub(self.previous_cycles)
            .ok_or("native CPU audio cycle precedes its previous wait or debt adjustment")?;
        let elapsed_frames = elapsed.wrapping_mul(44_100) / CPU_HZ;
        let target = self.cpu_frames.wrapping_add(elapsed_frames as u32);
        let diff = target.wrapping_sub(self.master_frames) as i32;
        let action = if diff > 0 {
            CpuAction::Wait
        } else if diff < -MAX_AUDIO_LAG {
            CpuAction::Rebase
        } else {
            CpuAction::Ready
        };
        let result = CpuDecision {
            action,
            cycle,
            previous_cycle: self.previous_cycles,
            previous_audio_frames: self.cpu_frames,
            target,
            master_frames: self.master_frames,
        };
        match action {
            CpuAction::Wait => {
                self.previous_cycles = cycle;
                self.cpu = CpuState::GoalPending {
                    cycle,
                    goal: target,
                };
            }
            CpuAction::Rebase => {
                self.previous_cycles = cycle;
                self.cpu_frames = self.master_frames.wrapping_sub(MAX_AUDIO_LAG as u32);
            }
            CpuAction::Ready => {}
        }
        Ok(result)
    }

    pub fn publish_cpu_goal(&mut self) -> Result<CpuWait> {
        let CpuState::GoalPending { cycle, goal } = self.cpu else {
            return Err("native CPU goal publication requires a pending wait decision".into());
        };
        let wait = CpuWait {
            cycle,
            goal,
            initial_trigger: self.trigger,
        };
        self.goal = goal;
        self.cpu = CpuState::Waiting(wait);
        Ok(wait)
    }

    pub fn cpu_waiting(&self) -> bool {
        !matches!(self.cpu, CpuState::Ready)
    }

    /// Admit the CPU after the existing trigger predicate changes. This neither
    /// runs guest instructions nor advances producer or consumer work.
    pub fn poll_release(&mut self) -> Result<bool> {
        match self.cpu {
            CpuState::Ready => Ok(true),
            CpuState::GoalPending { .. } => {
                Err("native CPU wait must publish its goal before checking release".into())
            }
            CpuState::Waiting(wait) if self.trigger != wait.initial_trigger => {
                self.cpu_frames = wait.goal;
                self.cpu = CpuState::Ready;
                Ok(true)
            }
            CpuState::Waiting(_) => Ok(false),
        }
    }
}
