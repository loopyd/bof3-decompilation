//! Serialized CPU, synthesis-producer and output-consumer scheduling.
use super::{ConsumerClock, DeviceProgress, Execution, NativeProducerProgress};
use crate::{
    machine::{
        native_queue::{EnqueueOutcome, ProducerState},
        output_clock::Collection,
    },
    Result,
};
use serde::Serialize;

const RETIREMENT_QUANTUM: u64 = 200_000;
const CONSUMER_CHUNK: usize = 441;

#[derive(Clone, Debug, Serialize)]
pub struct Clock {
    pub model: &'static str,
    pub retirement_quantum: u64,
    pub consumer_chunk: usize,
    pub frames: u64,
    pub retired_opcodes: u64,
    pub pending_opcodes: u64,
    pub quantum_turns: u64,
    pub wait_turns: u64,
    pub producer_commands: u64,
    pub synthesis_batches: u64,
    pub consumer_commands: u64,
}

pub(super) struct Schedule {
    pub(super) clock: Clock,
    last_cpu: u64,
}

impl Schedule {
    fn fresh() -> Self {
        Self {
            clock: Clock {
                model: "pcsx_redux_cooperative",
                retirement_quantum: RETIREMENT_QUANTUM,
                consumer_chunk: CONSUMER_CHUNK,
                frames: 0,
                retired_opcodes: 0,
                pending_opcodes: 0,
                quantum_turns: 0,
                wait_turns: 0,
                producer_commands: 0,
                synthesis_batches: 0,
                consumer_commands: 0,
            },
            last_cpu: 0,
        }
    }

    fn fill(&mut self, execution: &mut Execution) -> Result<()> {
        for _ in 0..64 {
            self.clock.producer_commands += 1;
            if execution.native_audio_queue()?.enqueue_pending() {
                if matches!(
                    execution.resume_native_enqueue(EnqueueOutcome::Retry)?,
                    ProducerState::Waiting { .. }
                ) {
                    return Ok(());
                }
                continue;
            }
            let mut completed = false;
            for _ in 0..1_000_000 {
                match execution.step_native_producer()? {
                    NativeProducerProgress::Advanced => {}
                    NativeProducerProgress::Batch { admission, .. } => {
                        self.clock.synthesis_batches += 1;
                        if matches!(admission, ProducerState::Waiting { .. }) {
                            return Ok(());
                        }
                        completed = true;
                        break;
                    }
                    NativeProducerProgress::Waiting => {
                        return Err(
                            "cooperative audio: producer waited without a submission".into()
                        );
                    }
                }
            }
            if !completed {
                return Err("cooperative audio: synthesis operation limit reached".into());
            }
        }
        Err("cooperative audio: producer command limit reached".into())
    }

    fn service(&mut self, execution: &mut Execution) -> Result<()> {
        execution.native_audio_callback(CONSUMER_CHUNK, 1)?;
        self.clock.consumer_commands += 1;
        self.clock.frames = execution.native_delivered_frames()?;
        self.fill(execution)
    }

    pub(super) fn advance(
        &mut self,
        execution: &mut Execution,
        progress: DeviceProgress,
    ) -> Result<()> {
        let total = execution.cpu.instructions() + execution.syscalls();
        let delta = total
            .checked_sub(self.last_cpu)
            .ok_or("cooperative audio: CPU retirement moved backwards")?;
        self.last_cpu = total;
        self.clock.pending_opcodes += delta;
        if progress == DeviceProgress::WaitingForSpu {
            let trigger = execution.native_audio_queue()?.trigger();
            for _ in 0..65_536 {
                self.clock.wait_turns += 1;
                self.service(execution)?;
                if execution.native_audio_queue()?.trigger() != trigger {
                    return Ok(());
                }
            }
            return Err("cooperative audio: CPU wait service limit reached".into());
        }
        progress.require_ready()?;
        if self.clock.pending_opcodes > 1 {
            return Err("cooperative audio: multiple opcodes await completion".into());
        }
        let before = self.clock.retired_opcodes / RETIREMENT_QUANTUM;
        self.clock.retired_opcodes += self.clock.pending_opcodes;
        self.clock.pending_opcodes = 0;
        if self.clock.retired_opcodes / RETIREMENT_QUANTUM != before {
            self.clock.quantum_turns += 1;
            self.service(execution)?;
        }
        Ok(())
    }
}

impl Execution {
    /// Select the fixed cooperative reference context before the first owned
    /// instruction. Prime the producer once, then serialize consumer-before-
    /// producer turns at retirement quanta and CPU audio-goal waits. Host call
    /// boundaries contribute no opcodes. Manual actor operations are excluded.
    pub fn enable_cooperative_audio(&mut self) -> Result<()> {
        self.enable_native_audio(ConsumerClock::Callback, Collection::Discard)?;
        let mut schedule = Schedule::fresh();
        let result = schedule.fill(self);
        self.cooperative = Some(schedule);
        self.retain_failure(&result);
        result
    }

    pub(super) fn require_manual_native_schedule(&self) -> Result<()> {
        if self.cooperative.is_some() {
            return Err("native audio actors are owned by the cooperative schedule".into());
        }
        Ok(())
    }
}
