//! Explicit native producer, consumer and CPU-goal operations from a fresh context.
use super::{boot, output_clock::Collection, DeviceProgress, Execution, StepOutcome, Transition};
use crate::{
    machine::{
        cpu::{Fault, Step},
        interconnect::ProducerProgress,
        native_queue::{self, CpuAction, EnqueueOutcome, MasterAdvance, ProducerState, Queue},
    },
    Result,
};

pub(super) const OUTPUT_LIMIT: usize = 4096;

/// Select which caller-scheduled operation advances the native master counter.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ConsumerClock {
    Callback,
    Null,
}

pub enum NativeProducerProgress {
    Advanced,
    Batch {
        source_frame: u64,
        key_on_mask: u32,
        pre_sample_steps: [u32; 24],
        admission: ProducerState,
    },
    Waiting,
}

pub(super) struct Audio {
    queue: Queue,
    clock: ConsumerClock,
    collection: Collection,
    delivered: u64,
    pcm: Vec<[f32; 2]>,
    pending: Option<Continuation>,
}

enum Continuation {
    Access {
        instruction: Box<boot::timing::PreparedInstruction>,
        before: u64,
    },
    Counters,
}

impl Audio {
    pub(super) fn has_continuation(&self) -> bool {
        self.pending.is_some()
    }
}

impl Execution {
    /// Select a fresh native context before owned instructions or SPU work.
    /// The caller supplies every producer operation and callback/null-clock
    /// boundary, including their order relative to guest execution. This API
    /// does not infer a host schedule or accept a partial native snapshot.
    pub fn enable_native_audio(
        &mut self,
        clock: ConsumerClock,
        collection: Collection,
    ) -> Result<()> {
        if self.native.is_some()
            || self.output.is_some()
            || self.reference.is_some()
            || self.clock.ticks() != 0
            || self
                .instruction
                .as_ref()
                .is_none_or(|clock| clock.cycles() != 0)
        {
            return Err("native audio requires fresh owned instruction timing".into());
        }
        self.bus.configure_native_audio()?;
        self.native = Some(Audio {
            queue: Queue::fresh(),
            clock,
            collection,
            delivered: 0,
            pcm: Vec::new(),
            pending: None,
        });
        Ok(())
    }

    pub fn native_audio_queue(&self) -> Result<&Queue> {
        Ok(&self
            .native
            .as_ref()
            .ok_or("native audio is not enabled")?
            .queue)
    }

    pub fn native_delivered_frames(&self) -> Result<u64> {
        Ok(self
            .native
            .as_ref()
            .ok_or("native audio is not enabled")?
            .delivered)
    }

    pub(super) fn begin_native_capture(&mut self) -> Result<u64> {
        let audio = self.native.as_mut().unwrap();
        if audio.collection != Collection::Discard {
            return Err("native audio is already retaining PCM".into());
        }
        audio.collection = Collection::Retain;
        Ok(audio.delivered)
    }

    pub(super) fn take_native_frames(&mut self) -> Result<Vec<[f32; 2]>> {
        Ok(std::mem::take(&mut self.native.as_mut().unwrap().pcm))
    }

    /// Current guest cycle, including a timer access held before retirement.
    pub fn native_audio_cycle(&self) -> Result<u64> {
        let audio = self.native.as_ref().ok_or("native audio is not enabled")?;
        Ok(match &audio.pending {
            Some(Continuation::Access { instruction, .. }) => instruction.request_cycle().unwrap(),
            _ => self.instruction.as_ref().unwrap().cycles(),
        })
    }

    /// Advance one native synthesis operation without CPU or consumer work.
    pub fn step_native_producer(&mut self) -> Result<NativeProducerProgress> {
        self.require_manual_native_schedule()?;
        self.require_execution()?;
        let result = self.advance_native_producer();
        self.retain_failure(&result);
        result
    }

    fn advance_native_producer(&mut self) -> Result<NativeProducerProgress> {
        let cycle = self.native_audio_cycle()?;
        if self.native.as_ref().unwrap().queue.enqueue_pending() {
            return Ok(NativeProducerProgress::Waiting);
        }
        if self.bus.step_native_spu_producer(cycle)? == ProducerProgress::Advanced {
            return Ok(NativeProducerProgress::Advanced);
        }
        let batch = self.bus.take_native_spu_batch()?;
        let admission = self.native.as_mut().unwrap().queue.submit45(batch.frames)?;
        Ok(NativeProducerProgress::Batch {
            source_frame: batch.source_frame,
            key_on_mask: batch.key_on_mask,
            pre_sample_steps: batch.pre_sample_steps,
            admission,
        })
    }

    pub fn resume_native_enqueue(&mut self, outcome: EnqueueOutcome) -> Result<ProducerState> {
        self.require_manual_native_schedule()?;
        self.require_execution()?;
        let result = self
            .native
            .as_mut()
            .ok_or("native audio is not enabled")?
            .queue
            .finish_enqueue(outcome);
        self.retain_failure(&result);
        result
    }

    /// Deliver one callback chunk and its explicit fast-forward discard runs.
    /// The native null-clock context still drains here but advances its master
    /// only through advance_native_null_clock. Producer work never runs here.
    pub fn native_audio_callback(&mut self, frames: usize, speed: i32) -> Result<()> {
        self.require_manual_native_schedule()?;
        self.require_execution()?;
        let result = self.consume_native_audio(frames, speed, OUTPUT_LIMIT);
        self.retain_failure(&result);
        result
    }

    pub(super) fn consume_native_audio(
        &mut self,
        frames: usize,
        speed: i32,
        collection_limit: usize,
    ) -> Result<()> {
        let audio = self.native.as_mut().ok_or("native audio is not enabled")?;
        if !(1..=2048).contains(&frames)
            || (audio.collection == Collection::Retain
                && audio.pcm.len() + frames > collection_limit)
        {
            return Err(format!(
                "native callback requires 1..=2048 frames and room in the {collection_limit}-frame PCM collection"
            ).into());
        }
        let next = audio
            .delivered
            .checked_add(frames as u64)
            .ok_or("native delivered-frame overflow")?;
        let block = audio.queue.dequeue(frames)?;
        for voice in block.frames {
            let frame = self.bus.native_callback_frame(voice)?;
            if audio.collection == Collection::Retain {
                audio.pcm.push(frame);
            }
        }
        for _ in 1..native_queue::effective_speed(speed) {
            audio.queue.discard(frames)?;
            self.bus.native_discard_cd(frames);
        }
        audio.delivered = next;
        if audio.clock == ConsumerClock::Callback {
            audio.queue.advance_master(frames as u32, speed)?;
        }
        Ok(())
    }

    pub fn advance_native_null_clock(&mut self, frames: u32, speed: i32) -> Result<MasterAdvance> {
        self.require_manual_native_schedule()?;
        self.require_execution()?;
        let audio = self.native.as_mut().ok_or("native audio is not enabled")?;
        if audio.clock != ConsumerClock::Null {
            return Err("native master is owned by callback clock".into());
        }
        let result = audio.queue.advance_master(frames, speed);
        self.retain_failure(&result);
        result
    }

    pub(super) fn prepare_native_instruction(
        &mut self,
        before: u64,
    ) -> Result<Option<std::result::Result<Step, Fault>>> {
        let clock = self.instruction.as_mut().unwrap();
        let instruction = match clock.prepare_instruction(&mut self.cpu, &mut self.bus) {
            Ok(instruction) => instruction,
            Err(fault) => return Ok(Some(Err(fault))),
        };
        if instruction.request().is_some_and(|request| {
            self.bus
                .reference_access_updates(request, instruction.isolated())
        }) && self.native_cpu_wait(instruction.request_cycle().unwrap())?
        {
            self.native.as_mut().unwrap().pending = Some(Continuation::Access {
                instruction: Box::new(instruction),
                before,
            });
            return Ok(None);
        }
        Ok(Some(
            self.instruction.as_mut().unwrap().complete_instruction(
                instruction,
                &mut self.cpu,
                &mut self.bus,
            ),
        ))
    }

    fn native_cpu_wait(&mut self, cycle: u64) -> Result<bool> {
        let queue = &mut self.native.as_mut().unwrap().queue;
        if queue.cpu_decision(cycle)?.action != CpuAction::Wait {
            return Ok(false);
        }
        queue.publish_cpu_goal()?;
        Ok(true)
    }

    pub(super) fn native_branch_wait(&mut self, branch: bool) -> Result<bool> {
        let cycle = self.instruction.as_ref().unwrap().cycles();
        if branch && self.bus.reference_update_due(cycle) && self.native_cpu_wait(cycle)? {
            self.native.as_mut().unwrap().pending = Some(Continuation::Counters);
            return Ok(true);
        }
        Ok(false)
    }

    pub(super) fn resume_native(&mut self) -> Result<Transition> {
        let audio = self.native.as_mut().unwrap();
        if !audio.queue.poll_release()? {
            return Ok(Transition::Advanced(StepOutcome {
                retired: None,
                devices: DeviceProgress::WaitingForSpu,
            }));
        }
        match audio.pending.take().unwrap() {
            Continuation::Access {
                instruction,
                before,
            } => {
                let step = self.instruction.as_mut().unwrap().complete_instruction(
                    *instruction,
                    &mut self.cpu,
                    &mut self.bus,
                );
                let retired = self.accept_instruction(step)?;
                self.finish_transition(Some(before), retired, false)
            }
            Continuation::Counters => {
                let cycle = self.instruction.as_ref().unwrap().cycles();
                self.bus.advance_native_time(cycle, true);
                let devices = self.bus.advance_devices(cycle, true)?;
                Ok(Transition::Advanced(StepOutcome {
                    retired: None,
                    devices,
                }))
            }
        }
    }
}
