//! US game music execution with observed loop/end boundaries and reference clocks.
//! Independent PCM fidelity remains an acceptance gate.
use crate::{
    interchange::wave::Wave,
    machine::{
        boot,
        bus::{Bus, Width},
        executable::Executable,
        execution::Call,
        firmware::Image,
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
pub struct Report {
    pub schema: &'static str,
    pub engine: &'static str,
    pub sample_rate: u32,
    pub identity: music::Identity,
    pub boot: boot::Evidence,
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

pub struct Rendered {
    pub wave: Wave,
    pub report: Report,
}

pub fn render(
    executable: &Executable,
    firmware: Image,
    archive: &ArchiveImage,
    options: &Options,
) -> Result<Rendered> {
    options.validate()?;
    let mut runtime = music::prepare(
        executable,
        firmware,
        archive,
        options.layout,
        options.sequence,
    )?;
    let execution = &mut runtime.execution;
    let instruction_start = execution.cpu.instructions();
    execution.enable_output(output_clock::Model::PcsxReduxNtsc)?;
    let mut progress = music_progress::Progress::new(
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
        0x8016b9cc,
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
        execution.call_observed(0x801753dc, [0; 4], 100_000, &mut |e| progress.observe(e))?;
        pcm.extend(execution.take_audio_frames()?.into_iter().flatten());
    };
    let total = body_frames + options.release_frames;
    let sequence_flags_at_cutoff = execution.bus.read(runtime.record + 0x90, Width::Word)?;
    let stop_call_started_at_frame = execution.output().unwrap().frames();
    let stop_call = execution.call_observed(
        0x8016d534,
        [runtime.handle, options.sequence as u32, 0, 0],
        100_000,
        &mut |e| progress.observe(e),
    )?;
    pcm.extend(execution.take_audio_frames()?.into_iter().flatten());
    let stop_call_returned_at_frame = execution.output().unwrap().frames();
    if execution.bus.read(runtime.record + 0x90, Width::Word)? & 1 != 0 {
        return Err("PSX render: original sequence stop left playback active".into());
    }
    while execution.output().unwrap().frames() < total {
        execution.call_observed(0x801753dc, [0; 4], 100_000, &mut |e| progress.observe(e))?;
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
        schema: "bof3.psx-render/v1", engine: "original_us_runtime", sample_rate: SAMPLE_RATE,
        identity: runtime.identity, boot: runtime.boot, initialization_calls: runtime.calls,
        requested_body: options.body, progress,
        play_call, stop_call, body_frames, release_frames: options.release_frames,
        output_frames: total, safety_frames: options.safety_frames, stop_call_started_at_frame,
        stop_call_returned_at_frame, sequence_flags_at_cutoff,
        executed_frames: clock.frames(), execution_instructions: execution.cpu.instructions() - instruction_start,
        output_clock: serde_json::to_value(clock)?, peak_magnitude, independent_pcm_validated: false,
        limitations: vec![
            "Reference NTSC scanline/output clocks with two base ticks per instruction; CPU stalls and hardware sub-frame timing are unverified.",
            "Host stages one VAB/SEP archive into game layout; original CD transport and multi-bank selection are not implemented.",
            "Output phase starts after initialization. Original BIOS and audio routines execute; hardware/reference PCM agreement is unverified.",
            "Duration or the observed loop/end boundary selects the body cutoff. Original stop runs at the next safe guest-call boundary; tail includes that delay and the guest key-off/voice flush.",
            "CD/XA input, delivery codecs and SFX playback contexts are outside this music rendering path.",
        ],
    };
    Ok(Rendered {
        wave: Wave::new(2, SAMPLE_RATE, pcm)?,
        report,
    })
}
