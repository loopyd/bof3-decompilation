//! Unit-scale native callbacks in branch-dispatch order. Root counters run first.
use super::Interconnect;
use crate::Result;

/// A host audio wait suspends a branch-dispatch pass without advancing CPU time.
#[must_use]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum DeviceProgress {
    Ready,
    WaitingForCdAudio,
}

impl DeviceProgress {
    /// Callers without a host audio consumer must stop at an explicit wait.
    pub fn require_ready(self) -> Result<()> {
        match self {
            Self::Ready => Ok(()),
            Self::WaitingForCdAudio => {
                Err("CD audio host wait requires an explicit queue wake or timeout outcome".into())
            }
        }
    }
}

#[derive(Clone, Copy)]
pub(super) enum Slot {
    CdFifo,
    CdCommand,
    CdRead,
    SpuDma,
    CdDma,
}

const ORDER: [Slot; 5] = [
    Slot::CdFifo,
    Slot::CdCommand,
    Slot::CdRead,
    Slot::SpuDma,
    Slot::CdDma,
];

#[derive(Clone, Copy, Default)]
struct Deadline {
    target: u64,
    pending: bool,
}

#[derive(Default)]
pub(super) struct Scheduler {
    cycle: u64,
    lowest_target: u64,
    deadlines: [Deadline; 5],
    dispatch: Option<Dispatch>,
}

struct Dispatch {
    pending: [bool; 5],
    next: usize,
}

impl Scheduler {
    pub(super) fn cycle(&self) -> u64 {
        self.cycle
    }

    pub(super) fn has_pending(&self) -> bool {
        self.deadlines.iter().any(|slot| slot.pending)
    }

    pub(super) fn attach(&mut self, cycle: u64) -> Result<()> {
        if self.dispatch.is_some() {
            return Err("controller attachment cannot interrupt a CD audio host wait".into());
        }
        if cycle < self.cycle {
            return Err("controller attachment requires a nondecreasing cycle".into());
        }
        self.cycle = cycle;
        if !self.has_pending() {
            self.lowest_target = cycle;
        }
        Ok(())
    }

    pub(super) fn deadline(&self, slot: Slot) -> Option<u64> {
        let deadline = self.deadlines[slot as usize];
        deadline.pending.then_some(deadline.target)
    }

    pub(super) fn schedule(&mut self, slot: Slot, cycle: u64, delay: u64) -> Result<()> {
        let target = cycle
            .checked_add(delay)
            .ok_or("device callback clock overflow")?;
        self.deadlines[slot as usize] = Deadline {
            target,
            pending: true,
        };
        self.lowest_target = self.lowest_target.min(target);
        Ok(())
    }

    pub(super) fn cancel(&mut self, slot: Slot) {
        self.deadlines[slot as usize].pending = false;
    }
}

impl Interconnect {
    /// Advance absolute CPU cycles. Only a retired branch delay slot dispatches
    /// callbacks. Unimplemented SIO, GPU and MDEC work has no scheduled slots.
    /// Errors retain prior device work and are terminal for this execution.
    pub fn advance_devices(&mut self, cycle: u64, branch_boundary: bool) -> Result<DeviceProgress> {
        if self.scheduler.dispatch.is_some() {
            return Err("resume the suspended CD audio dispatch before advancing devices".into());
        }
        if cycle < self.scheduler.cycle {
            return Err("device callback cycle moved backwards".into());
        }
        self.scheduler.cycle = cycle;
        let pending = self.scheduler.deadlines.map(|deadline| deadline.pending);
        if !branch_boundary
            || !pending.iter().any(|&pending| pending)
            || self.scheduler.lowest_target >= cycle
        {
            return Ok(DeviceProgress::Ready);
        }
        self.scheduler.dispatch = Some(Dispatch { pending, next: 0 });
        self.continue_device_dispatch()
    }

    pub fn device_progress(&self) -> DeviceProgress {
        if self.scheduler.dispatch.is_some() {
            DeviceProgress::WaitingForCdAudio
        } else {
            DeviceProgress::Ready
        }
    }

    /// Resume the retained CD callback and then the rest of its original pass.
    /// Queue capacity wins over timeout. Retry may leave the same wait pending.
    /// No device output, guest instruction or CPU cycle runs here.
    pub fn resume_cd_audio(
        &mut self,
        outcome: crate::machine::cd_queue::WaitOutcome,
    ) -> Result<DeviceProgress> {
        if self.scheduler.dispatch.is_none() {
            return Err("no suspended CD audio dispatch".into());
        }
        if self.resume_clocked_cd_audio(outcome, self.scheduler.cycle)?
            == DeviceProgress::WaitingForCdAudio
        {
            return Ok(DeviceProgress::WaitingForCdAudio);
        }
        self.continue_device_dispatch()
    }

    fn continue_device_dispatch(&mut self) -> Result<DeviceProgress> {
        let cycle = self.scheduler.cycle;
        while self.scheduler.dispatch.as_ref().unwrap().next < ORDER.len() {
            let dispatch = self.scheduler.dispatch.as_mut().unwrap();
            let index = dispatch.next;
            dispatch.next += 1;
            let slot = ORDER[index];
            let was_pending = dispatch.pending[index];
            if !was_pending || self.scheduler.deadlines[slot as usize].target > cycle {
                continue;
            }
            self.scheduler.cancel(slot);
            match slot {
                Slot::CdFifo | Slot::CdCommand | Slot::CdRead => {
                    if self.dispatch_cd(slot, cycle)? == DeviceProgress::WaitingForCdAudio {
                        return Ok(DeviceProgress::WaitingForCdAudio);
                    }
                }
                Slot::SpuDma => {
                    self.dma.spu.complete();
                    self.complete_dma(4)?;
                }
                Slot::CdDma => {
                    if self.dma.cd.finish_reference() {
                        self.complete_dma(3)?;
                    }
                }
            }
        }
        // The pinned native build narrows INT64_MAX into signed32 -1 for its
        // minimum-distance scan. Positive future distances never replace it.
        self.scheduler.lowest_target = cycle;
        self.scheduler.dispatch = None;
        Ok(DeviceProgress::Ready)
    }

    fn complete_dma(&mut self, channel: u8) -> Result<()> {
        self.dma.complete(channel)?;
        self.interrupts
            .set_line(crate::machine::interrupts::Source::Dma, self.dma.irq_line());
        Ok(())
    }
}
