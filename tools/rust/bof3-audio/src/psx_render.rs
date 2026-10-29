//! Owned music and cold-bank effects rendering with explicit reference clocks.
//! Independent PCM fidelity remains an acceptance gate.
use crate::{
    interchange::wave::Wave,
    machine::{
        bank,
        bus::{Bus, Width},
        cues, driver, effects,
        execution::Call,
        music, music_progress, output_clock,
    },
    Result,
};
use emi_ex_v2::image::ArchiveImage;
use serde::Serialize;

pub const SAMPLE_RATE: u32 = 44_100;

#[derive(Clone, Copy, Debug, Serialize)]
#[serde(tag = "kind", content = "value", rename_all = "snake_case")]
pub enum Body {
    Duration(u64),
    Loops(u32),
}

#[derive(Clone, Debug)]
pub struct Options {
    pub sequence: usize,
    pub layout: Option<u8>,
    pub body: Body,
    pub release_frames: u64,
    pub safety_frames: u64,
}

impl Options {
    pub fn validate(&self) -> Result<()> {
        if self.sequence >= 4 || self.layout.is_some_and(|layout| layout > 2) {
            return Err("PSX render: sequence must be 0..3 and layout 0..2".into());
        }
        if self.safety_frames == 0 || self.safety_frames > u64::from(SAMPLE_RATE) * 600 {
            return Err(
                "PSX render: audio safety limit must be positive and at most 600 seconds".into(),
            );
        }
        if self.release_frames >= self.safety_frames {
            return Err("PSX render: tail leaves no body within audio safety limit".into());
        }
        match self.body {
            Body::Duration(0) | Body::Loops(0) => {
                return Err("PSX render: duration or loop count must be positive".into())
            }
            Body::Duration(frames) if frames > self.safety_frames - self.release_frames => {
                return Err("PSX render: duration plus tail exceeds audio safety limit".into());
            }
            _ => {}
        }
        Ok(())
    }
}

#[derive(Debug, Serialize)]
pub struct Report<P = driver::Evidence> {
    pub schema: &'static str,
    pub engine: &'static str,
    pub sample_rate: u32,
    pub identity: music::Identity,
    pub startup: P,
    pub initialization_calls: Vec<Call>,
    pub play_call: Call,
    pub stop_call: Call,
    pub requested_body: Body,
    pub progress: music_progress::Progress,
    pub body_frames: u64,
    pub release_frames: u64,
    pub output_frames: u64,
    pub safety_frames: u64,
    pub stop_call_started_at_frame: u64,
    pub stop_call_returned_at_frame: u64,
    pub sequence_flags_at_cutoff: u32,
    pub executed_frames: u64,
    pub execution_instructions: u64,
    pub output_clock: serde_json::Value,
    pub peak_magnitude: u16,
    pub independent_pcm_validated: bool,
    pub limitations: Vec<&'static str>,
}

pub struct Rendered<P = driver::Evidence> {
    pub wave: Wave,
    pub report: Report<P>,
}

pub fn render(
    image: &driver::Image,
    archive: &ArchiveImage,
    options: &Options,
) -> Result<Rendered> {
    options.validate()?;
    let runtime = music::prepare(image, archive, options.layout, options.sequence)?;
    render_prepared(runtime, options, "owned_us_driver", vec![
        "Instruction fetch, data access and multiply/divide costs use the reference interpreter model. Host kernel services retain two functional ticks per dispatch; their latency and hardware sub-frame timing are unverified.",
        "Host stages one VAB/SEP archive into the owned layout; original CD transport and multi-bank selection are not implemented.",
        "Output phase starts after owned initialization. Native timing and independent PCM agreement remain unverified.",
        "Duration or the observed loop/end boundary selects the body cutoff. Stop runs at the next safe guest-call boundary; tail includes that delay and the guest key-off/voice flush.",
        "CD/XA input and SFX playback contexts are outside this music rendering path.",
    ])
}

pub(crate) fn render_prepared<P>(
    mut runtime: music::Prepared<P>,
    options: &Options,
    engine: &'static str,
    limitations: Vec<&'static str>,
) -> Result<Rendered<P>> {
    let execution = &mut runtime.execution;
    let instruction_start = execution.cpu.instructions();
    execution.enable_output(output_clock::Model::PcsxReduxNtsc)?;
    let mut progress = music_progress::Progress::new(
        music_progress::Instructions {
            loop_start: runtime.audio.loop_start,
            infinite_loop: runtime.audio.infinite_loop,
            end: runtime.audio.sequence_end,
        },
        runtime.handle,
        options.sequence as u32,
        runtime.record,
        match options.body {
            Body::Loops(count) => Some(count),
            Body::Duration(_) => None,
        },
    )?;
    // One original SEP play: encoded controller loops retain guest semantics.
    let play_call = execution.call_observed(
        runtime.audio.play,
        [runtime.handle, options.sequence as u32, 1, 1],
        100_000,
        &mut |e| progress.observe(e),
    )?;
    let mut pcm: Vec<i16> = execution
        .take_audio_frames()?
        .into_iter()
        .flatten()
        .collect();
    let body_frames = loop {
        let frames = execution.output().unwrap().frames();
        match options.body {
            Body::Duration(limit) if frames >= limit => break limit,
            Body::Loops(_) => {
                if let Some(boundary) = &progress.boundary {
                    if boundary.frame > options.safety_frames - options.release_frames {
                        return Err(
                            "PSX render: loop/end boundary exceeds audio safety limit".into()
                        );
                    }
                    break boundary.frame;
                }
                if frames >= options.safety_frames - options.release_frames {
                    return Err(format!("PSX render: audio safety limit reached before requested loop/end boundary ({} infinite traversals observed); no output published", progress.total_infinite_traversals).into());
                }
            }
            _ => {}
        }
        execution.call_observed(runtime.audio.idle, [0; 4], 100_000, &mut |e| {
            progress.observe(e)
        })?;
        pcm.extend(execution.take_audio_frames()?.into_iter().flatten());
    };
    let total = body_frames + options.release_frames;
    let sequence_flags_at_cutoff = execution.bus.read(runtime.record + 0x90, Width::Word)?;
    let stop_call_started_at_frame = execution.output().unwrap().frames();
    let stop_call = execution.call_observed(
        runtime.audio.stop,
        [runtime.handle, options.sequence as u32, 0, 0],
        100_000,
        &mut |e| progress.observe(e),
    )?;
    pcm.extend(execution.take_audio_frames()?.into_iter().flatten());
    let stop_call_returned_at_frame = execution.output().unwrap().frames();
    if execution.bus.read(runtime.record + 0x90, Width::Word)? & 1 != 0 {
        return Err("PSX render: sequence stop left playback active".into());
    }
    while execution.output().unwrap().frames() < total {
        execution.call_observed(runtime.audio.idle, [0; 4], 100_000, &mut |e| {
            progress.observe(e)
        })?;
        pcm.extend(execution.take_audio_frames()?.into_iter().flatten());
    }
    pcm.truncate(usize::try_from(
        total
            .checked_mul(2)
            .ok_or("PSX render: PCM size overflow")?,
    )?);
    let peak_magnitude = pcm.iter().map(|s| s.unsigned_abs()).max().unwrap_or(0);
    let clock = execution.output().unwrap();
    let report = Report {
        schema: "bof3.psx-render/v2",
        engine,
        sample_rate: SAMPLE_RATE,
        identity: runtime.identity,
        startup: runtime.startup,
        initialization_calls: runtime.calls,
        requested_body: options.body,
        progress,
        play_call,
        stop_call,
        body_frames,
        release_frames: options.release_frames,
        output_frames: total,
        safety_frames: options.safety_frames,
        stop_call_started_at_frame,
        stop_call_returned_at_frame,
        sequence_flags_at_cutoff,
        executed_frames: clock.frames(),
        execution_instructions: execution.cpu.instructions() - instruction_start,
        output_clock: serde_json::to_value(clock)?,
        peak_magnitude,
        independent_pcm_validated: false,
        limitations,
    };
    Ok(Rendered {
        wave: Wave::new(2, SAMPLE_RATE, pcm)?,
        report,
    })
}

#[derive(Clone, Copy, Debug, Serialize)]
#[serde(tag = "kind", rename_all = "snake_case")]
pub enum EffectsSelection {
    SdkNote { note: effects::Note },
    GameCue { auxiliary_entry: usize, row: usize },
}

#[derive(Clone, Copy, Debug, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum EffectsHistory {
    Cold,
}

pub struct EffectsOptions {
    pub header_entry: usize,
    pub body_entry: usize,
    pub layout: Option<u8>,
    pub selection: EffectsSelection,
    pub history: EffectsHistory,
    pub duration_frames: u64,
    pub release_frames: u64,
    pub safety_frames: u64,
}

impl EffectsOptions {
    pub fn validate(&self) -> Result<()> {
        if self.layout.is_some_and(|layout| layout > 2) {
            return Err("PSX effects render: layout must be 0..2".into());
        }
        if self.safety_frames == 0 || self.safety_frames > u64::from(SAMPLE_RATE) * 600 {
            return Err(
                "PSX effects render: audio safety limit must be positive and at most 600 seconds"
                    .into(),
            );
        }
        if self.duration_frames == 0
            || self.release_frames >= self.safety_frames
            || self.duration_frames > self.safety_frames - self.release_frames
        {
            return Err(
                "PSX effects render: positive duration plus tail must fit the audio safety limit"
                    .into(),
            );
        }
        Ok(())
    }
}

#[derive(Debug, Serialize)]
pub struct EffectsReport {
    pub schema: &'static str,
    pub stop_condition: &'static str,
    pub cue_record: Option<[u8; 4]>,
    pub engine: &'static str,
    pub sample_rate: u32,
    pub identity: bank::Identity,
    pub selection: EffectsSelection,
    pub history: EffectsHistory,
    pub startup: driver::Evidence,
    pub initialization_calls: Vec<Call>,
    pub play_call: Call,
    pub poll_calls: Vec<Call>,
    pub state_after_play: cues::State,
    pub state_after_body_poll: cues::State,
    pub release_voices: Vec<u8>,
    pub stop_calls: Vec<Call>,
    pub body_frames: u64,
    pub release_frames: u64,
    pub output_frames: u64,
    pub safety_frames: u64,
    pub stop_call_started_at_frame: u64,
    pub stop_call_returned_at_frame: u64,
    pub executed_frames: u64,
    pub execution_instructions: u64,
    pub output_clock: serde_json::Value,
    pub peak_magnitude: u16,
    pub independent_pcm_validated: bool,
    pub limitations: Vec<&'static str>,
}

pub struct RenderedEffects {
    pub wave: Wave,
    pub report: EffectsReport,
}

/// Render one explicitly selected event in a fresh, otherwise empty bank.
pub fn render_effects(
    image: &driver::Image,
    archive: &ArchiveImage,
    options: &EffectsOptions,
) -> Result<RenderedEffects> {
    options.validate()?;
    let mut runtime = bank::prepare(
        image,
        archive,
        &bank::Options {
            header_entry: options.header_entry,
            body_entry: options.body_entry,
            sequence_entry: None,
            layout: options.layout,
        },
        &mut |_, _| Ok(()),
    )?;
    let callbacks = runtime
        .execution
        .call(runtime.audio.callbacks, [0; 4], 100_000)?;
    runtime.calls.push(callbacks);
    let table = match options.selection {
        EffectsSelection::GameCue {
            auxiliary_entry, ..
        } => Some(cues::stage(&mut runtime, archive, auxiliary_entry)?),
        EffectsSelection::SdkNote { .. } => None,
    };
    let mut poll_calls = vec![cues::refresh(&mut runtime, &mut |_| Ok(()))?];
    let instruction_start = runtime.execution.cpu.instructions();
    runtime
        .execution
        .enable_output(output_clock::Model::PcsxReduxNtsc)?;
    let play_call = match options.selection {
        EffectsSelection::SdkNote { note } => effects::key_on(&mut runtime, note, &mut |_| Ok(()))?,
        EffectsSelection::GameCue { row, .. } => {
            cues::dispatch(&mut runtime, table.as_ref().unwrap(), row, &mut |_| Ok(()))?
        }
    };
    let cue_record = match options.selection {
        EffectsSelection::GameCue { row, .. } => table
            .as_ref()
            .and_then(|table| table.records.get(row))
            .copied(),
        EffectsSelection::SdkNote { .. } => None,
    };
    let state_after_play = cues::state(&runtime)?;
    let mut pcm: Vec<i16> = runtime
        .execution
        .take_audio_frames()?
        .into_iter()
        .flatten()
        .collect();
    while runtime.execution.output().unwrap().frames() < options.duration_frames {
        runtime
            .execution
            .call(runtime.audio.idle, [0; 4], 100_000)?;
        pcm.extend(runtime.execution.take_audio_frames()?.into_iter().flatten());
    }
    poll_calls.push(cues::refresh(&mut runtime, &mut |_| Ok(()))?);
    pcm.extend(runtime.execution.take_audio_frames()?.into_iter().flatten());
    let state_after_body_poll = cues::state(&runtime)?;
    let release_voices: Vec<u8> = match options.selection {
        EffectsSelection::SdkNote { note } => vec![note.voice],
        EffectsSelection::GameCue { .. } => (0..24).collect(),
    };
    let stop_call_started_at_frame = runtime.execution.output().unwrap().frames();
    let mut stop_calls = Vec::new();
    for &voice in &release_voices {
        stop_calls.push(effects::key_off(&mut runtime, voice, &mut |_| Ok(()))?);
        pcm.extend(runtime.execution.take_audio_frames()?.into_iter().flatten());
    }
    let stop_call_returned_at_frame = runtime.execution.output().unwrap().frames();
    let total = options.duration_frames + options.release_frames;
    while runtime.execution.output().unwrap().frames() < total {
        runtime
            .execution
            .call(runtime.audio.idle, [0; 4], 100_000)?;
        pcm.extend(runtime.execution.take_audio_frames()?.into_iter().flatten());
    }
    pcm.truncate(usize::try_from(
        total
            .checked_mul(2)
            .ok_or("PSX effects render: PCM size overflow")?,
    )?);
    let peak_magnitude = pcm
        .iter()
        .map(|sample| sample.unsigned_abs())
        .max()
        .unwrap_or(0);
    let clock = runtime.execution.output().unwrap();
    let report = EffectsReport {
        schema: "bof3.psx-effects-render/v1",
        stop_condition: "duration",
        cue_record,
        engine: "owned_us_driver",
        sample_rate: SAMPLE_RATE,
        identity: runtime.identity,
        selection: options.selection,
        history: options.history,
        startup: runtime.startup,
        initialization_calls: runtime.calls,
        play_call,
        poll_calls,
        state_after_play,
        state_after_body_poll,
        release_voices,
        stop_calls,
        body_frames: options.duration_frames,
        release_frames: options.release_frames,
        output_frames: total,
        safety_frames: options.safety_frames,
        stop_call_started_at_frame,
        stop_call_returned_at_frame,
        executed_frames: clock.frames(),
        execution_instructions: runtime.execution.cpu.instructions() - instruction_start,
        output_clock: serde_json::to_value(clock)?,
        peak_magnitude,
        independent_pcm_validated: false,
        limitations: vec![
            "One event runs in a cold, host-staged bank. Repeated-event histories, cross-bank cues and original CD loading are unsupported.",
            "SDK notes use the explicit voice, program, tone, key, fine pitch and channel volumes; this does not recover a game cue mapping or classify a bank sample.",
            "Game cues use the selected local table row and guest allocation. Voice status is polled before playback and after the body. Release calls key off all 24 voices in the otherwise empty machine.",
            "The fixed tail includes safe-call cutoff delay and guest release. It may end before the sound becomes silent.",
            "Output phase starts after owned initialization. Functional service latency, native timing and independent PCM agreement remain unverified.",
            "Vocal/XA streaming and multi-bank playback remain unsupported.",
        ],
    };
    Ok(RenderedEffects {
        wave: Wave::new(2, SAMPLE_RATE, pcm)?,
        report,
    })
}
