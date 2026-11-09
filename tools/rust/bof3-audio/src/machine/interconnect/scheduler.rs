//! Unit-scale native callbacks in branch-dispatch order. Root counters run first.
use super::Interconnect;
use crate::Result;

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
}

impl Scheduler {
    pub(super) fn cycle(&self) -> u64 {
        self.cycle
    }

    pub(super) fn has_pending(&self) -> bool {
        self.deadlines.iter().any(|slot| slot.pending)
    }

    pub(super) fn attach(&mut self, cycle: u64) -> Result<()> {
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
    pub fn advance_devices(&mut self, cycle: u64, branch_boundary: bool) -> Result<()> {
        if cycle < self.scheduler.cycle {
            return Err("device callback cycle moved backwards".into());
        }
        self.scheduler.cycle = cycle;
        let pending = self.scheduler.deadlines.map(|deadline| deadline.pending);
        if !branch_boundary
            || !pending.iter().any(|&pending| pending)
            || self.scheduler.lowest_target >= cycle
        {
            return Ok(());
        }
        for (slot, was_pending) in ORDER.into_iter().zip(pending) {
            if !was_pending || self.scheduler.deadlines[slot as usize].target > cycle {
                continue;
            }
            self.scheduler.cancel(slot);
            match slot {
                Slot::CdFifo | Slot::CdCommand | Slot::CdRead => self.dispatch_cd(slot, cycle)?,
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
        Ok(())
    }

    fn complete_dma(&mut self, channel: u8) -> Result<()> {
        self.dma.complete(channel)?;
        self.interrupts
            .set_line(crate::machine::interrupts::Source::Dma, self.dma.irq_line());
        Ok(())
    }
}
