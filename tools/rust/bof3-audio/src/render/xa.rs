//! Owned US XA cue playback with explicit media, TOC and cold decoder history.
use crate::{
    archive::disc::DiscImage,
    catalog::model::XaCue,
    digest::sha256_hex,
    interchange::wave::Wave,
    machine::{
        bus::{Bus, Width},
        cd::audio,
        cd::data,
        cd::drive::{Drive, Toc, Track, TrackKind},
        cd::media::Media,
        cd::position::CdPosition,
        cd::queue::{self, Queue},
        driver,
        execution::{Call, Execution},
        spu::noise,
        spu::transfer,
    },
    psx_render::{self, Delivery, SAMPLE_RATE},
    xa::{self, Arithmetic, Histories, Stream},
    Result,
};
use serde::{Deserialize, Serialize};
use std::{fs::File, path::Path};

const TICK_PERIOD: u64 = 33_868_800 / 60;
const SOURCES: [&str; 3] = [
    "BIN/SCE_XA/S_XA00.STR",
    "BIN/BMAG_XA/MAGIC00.STR",
    "BIN/SCE_XA/VOICE.STR",
];

#[derive(Clone, Debug, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct DiscToc {
    pub schema: String,
    pub tracks: Vec<DiscTrack>,
    pub lead_out_lba: i32,
}

#[derive(Clone, Debug, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct DiscTrack {
    pub kind: Kind,
    pub pregap_lba: i32,
    pub start_lba: i32,
    pub end_lba: i32,
}

#[derive(Clone, Copy, Debug, Deserialize, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum Kind {
    Data,
    Audio,
}

impl DiscToc {
    fn drive(&self, data_sectors: u64) -> Result<Drive> {
        let first = self.tracks.first().ok_or("XA render: TOC has no tracks")?;
        if self.schema != "bof3.disc-toc/v1"
            || !matches!(first.kind, Kind::Data)
            || first.pregap_lba != 0
            || first.start_lba != 0
            || u64::try_from(first.end_lba)? != data_sectors
            || self
                .tracks
                .iter()
                .skip(1)
                .any(|track| !matches!(track.kind, Kind::Audio))
        {
            return Err("XA render: TOC must describe the supplied whole first DATA track and subsequent AUDIO tracks".into());
        }
        let tracks = self
            .tracks
            .iter()
            .map(|track| {
                Ok(Track {
                    kind: match track.kind {
                        Kind::Data => TrackKind::Data,
                        Kind::Audio => TrackKind::Audio,
                    },
                    pregap_start: CdPosition::from_lba(track.pregap_lba)?,
                    start: CdPosition::from_lba(track.start_lba)?,
                    end: CdPosition::from_lba(track.end_lba)?,
                })
            })
            .collect::<Result<Vec<_>>>()?;
        let mut drive = Drive::ready(CdPosition::from_lba(0)?, 0, [0, 0])?;
        drive.configure_toc(Toc::new(tracks, CdPosition::from_lba(self.lead_out_lba)?)?)?;
        Ok(drive)
    }
}

#[derive(Clone, Copy, Debug, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum History {
    Cold,
}

#[derive(Clone, Debug)]
pub struct Options {
    pub cue: u16,
    pub history: History,
    pub duration_frames: Option<u64>,
    pub release_frames: u64,
    pub safety_frames: u64,
}

impl Options {
    pub fn validate(&self) -> Result<()> {
        if self.safety_frames == 0 || self.safety_frames > u64::from(SAMPLE_RATE) * 600 {
            return Err("XA render: safety limit must be positive and at most 600 seconds".into());
        }
        if self.release_frames >= self.safety_frames {
            return Err("XA render: tail leaves no body within the safety limit".into());
        }
        if self
            .duration_frames
            .is_some_and(|frames| frames == 0 || frames > self.safety_frames - self.release_frames)
        {
            return Err("XA render: duration must be positive and duration plus tail must fit the safety limit".into());
        }
        Ok(())
    }
}

#[derive(Debug, Serialize)]
pub struct Source {
    pub data_track: String,
    pub data_track_bytes: u64,
    pub disc_path: &'static str,
    pub source_lba: u32,
    pub source_sectors: usize,
    pub source_sha256: String,
    pub coding: u8,
}

#[derive(Debug, Serialize)]
pub struct Transition {
    pub state: u8,
    pub frame: u64,
}

#[derive(Debug, Serialize)]
pub struct Report {
    pub schema: &'static str,
    pub engine: &'static str,
    pub sample_rate: u32,
    pub source: Source,
    pub toc: DiscToc,
    pub cue: XaCue,
    pub history: History,
    pub startup: driver::Evidence,
    pub initialization_calls: Vec<Call>,
    pub play_call: Call,
    pub cancellation_calls: Vec<Call>,
    pub requested_duration_frames: Option<u64>,
    pub body_frames: u64,
    pub release_frames: u64,
    pub output_frames: u64,
    pub safety_frames: u64,
    pub capture_start_frame: u64,
    pub stop_request_frame: Option<u64>,
    pub idle_frame: u64,
    pub tick_period_cpu_cycles: u64,
    pub tick_calls: u64,
    pub states_at_tick_return: Vec<Transition>,
    pub commands: Vec<u8>,
    pub final_queue: serde_json::Value,
    pub executed_frames: u64,
    pub execution_instructions: u64,
    pub output_clock: serde_json::Value,
    pub pcm16_delivery: Delivery,
    pub independent_pcm_validated: bool,
    pub limitations: Vec<&'static str>,
}

pub struct Rendered {
    pub wave: Wave,
    pub report: Report,
}

pub fn render(
    image: &driver::Image,
    track: &Path,
    toc: &DiscToc,
    options: &Options,
) -> Result<Rendered> {
    render_observed(image, track, toc, options, &mut |_| Ok(()), &mut |_| Ok(()))
}

/// Observe the production path before each guest instruction, including reset,
/// CD initialization and caller-input copies. Observation cannot mutate execution.
/// After completion checks, observe all retained stereo float PCM before frame
/// trimming and PCM16 conversion. Either observer may stop the render with an error.
pub fn render_observed(
    image: &driver::Image,
    track: &Path,
    toc: &DiscToc,
    options: &Options,
    observer: &mut impl FnMut(&Execution) -> Result<()>,
    pcm_observer: &mut impl FnMut(&[f32]) -> Result<()>,
) -> Result<Rendered> {
    options.validate()?;
    let media = File::open(track)?;
    let data_track_bytes = media.metadata()?.len();
    if data_track_bytes == 0 || !data_track_bytes.is_multiple_of(2352) {
        return Err("XA render: expected a whole raw 2352-byte DATA track".into());
    }
    let drive = toc.drive(data_track_bytes / 2352)?;
    let mut disc = DiscImage::open(track)?;
    let mut bases = [0; 3];
    for (index, path) in SOURCES.iter().enumerate() {
        bases[index] = disc
            .files()
            .get(*path)
            .ok_or_else(|| format!("XA render: disc is missing {path}"))?
            .lba;
    }
    let cue = xa::cue::read_owned_cues(image, bases)?
        .into_iter()
        .find(|cue| cue.packed_id == options.cue)
        .ok_or("XA render: unknown US packed cue ID")?;
    let source = disc.read_xa(cue.disc_path)?;
    let source_sha256 = sha256_hex(source.bytes());
    let reference = xa::reference::identify(&source_sha256)
        .filter(|reference| {
            !reference.is_truncated() && reference.reference_source == cue.disc_path
        })
        .ok_or("XA render: selected source is not a recognized complete US disc extent")?;
    let first = source
        .sectors()
        .get(cue.sector_start)
        .ok_or("XA render: cue starts outside source")?;
    let stream = Stream {
        file: cue.filter_file,
        channel: cue.channel,
        coding: first.coding,
    };
    for offset in 0..cue.sector_count {
        let sector = source
            .sectors()
            .get(cue.sector_start + offset * cue.sector_stride)
            .ok_or("XA render: cue extends outside source")?;
        if [sector.file, sector.channel, sector.coding]
            != [stream.file, stream.channel, stream.coding]
            || sector.submode & 0x64 != 0x64
        {
            return Err(
                "XA render: selected cue has unsupported stream or coding transitions".into(),
            );
        }
    }
    let source = Source {
        data_track: track.to_string_lossy().into_owned(),
        data_track_bytes,
        disc_path: cue.disc_path,
        source_lba: bases[usize::from(cue.stream)],
        source_sectors: reference.actual_sectors,
        source_sha256,
        coding: stream.coding,
    };
    let command = image.entry("audio_cd_command_core")?;
    let mut commands = Vec::new();
    let mut observe = |e: &Execution| {
        if e.cpu.pc() == command {
            commands.push(e.cpu.register(4) as u8);
        }
        observer(e)
    };
    let started = image.start_cd(
        drive,
        Some(Media::new(media, vec![])?),
        &mut |e| {
            e.bus.configure_cd_data(data::Model::EmulatorReference)?;
            e.bus.configure_cd_audio(Queue::new(
                stream,
                Arithmetic::CombinedRounded,
                Histories::default(),
                audio::Model::EmulatorReference,
                queue::Model::EmulatorReference,
            )?)?;
            e.bus
                .configure_spu_noise(noise::State::EmulatorReference { value: 1, count: 0 })?;
            e.bus
                .configure_spu_ram_control(transfer::RamControlModel::EmulatorReference)?;
            e.bus
                .configure_spu_irq(transfer::IrqModel::EmulatorReference)?;
            e.enable_cooperative_audio()
        },
        &mut observe,
    )?;
    let mut e = started.services.execution;
    let mut initialization_calls = vec![started.services.initialization, started.initialization];
    initialization_calls.push(e.call_observed(
        image.entry("audio_sound_initialize")?,
        [0; 4],
        1_000_000,
        &mut observe,
    )?);
    initialization_calls.push(e.call_observed(
        image.entry("audio_cd_spu_initialize")?,
        [0; 4],
        100_000,
        &mut observe,
    )?);
    let copy = image.entry("audio_staging_copy")?;
    for (index, slot) in [681, 434, 682].into_iter().enumerate() {
        initialization_calls.extend(e.copy_input(
            copy,
            image.symbol("audio_xa_file_lbas")? + slot * 4,
            &bases[index].to_le_bytes(),
            &mut observe,
        )?);
    }
    let state = image.symbol("audio_xa_state")?;
    let foreground = image.entry("audio_xa_foreground")?;
    let tick = image.entry("audio_xa_tick")?;
    let capture_start_frame = e.begin_audio_capture()?;
    let instruction_start = e.cpu.instructions();
    let play_call = e.call_observed(
        image.entry("audio_xa_start")?,
        [u32::from(options.cue), 0, 0, 0],
        100_000,
        &mut observe,
    )?;
    if e.bus.read(state, Width::Word)? != cue.runtime_start_lba
        || e.bus.read(state + 4, Width::Word)? != cue.runtime_stop_threshold_lba
    {
        return Err("XA render: guest selection disagrees with owned cue plan".into());
    }
    let mut pcm = Vec::new();
    let mut body_frames = options.duration_frames;
    let mut stop_request_frame = None;
    let mut cancellation_calls = Vec::new();
    let mut cancellation_pending = false;
    let mut idle_frame = None;
    let mut transitions = Vec::new();
    let mut tick_calls = 0;
    let mut next_tick = e.clock.ticks();
    loop {
        pcm.extend(e.take_audio_frames()?.into_iter().flatten());
        let frames = e.audio_frames()? - capture_start_frame;
        let current = e.bus.read(state + 0x21, Width::Byte)? as u8;
        if current > 8 {
            return Err("XA render: guest state escaped the nine-state dispatcher".into());
        }
        if current == 0 {
            idle_frame.get_or_insert(frames);
            body_frames.get_or_insert(frames);
        }
        if let Some(body) = body_frames {
            if frames >= body && current != 0 && stop_request_frame.is_none() {
                // Original caller-owned cancellation input. The guest tick owns
                // recovery, gain ramp, Pause and completion callback teardown.
                cancellation_calls.extend(e.copy_input(copy, state + 0x20, &[1], &mut observe)?);
                stop_request_frame = Some(frames);
                cancellation_pending = true;
            }
            if frames >= body + options.release_frames && current == 0 {
                break;
            }
        }
        if frames >= options.safety_frames {
            return Err(
                "XA render: safety limit reached before cue stop and tail; no output published"
                    .into(),
            );
        }
        if e.clock.ticks() >= next_tick {
            e.call_observed(tick, [0; 4], 1_000_000, &mut observe)?;
            if cancellation_pending {
                // This is a caller input pulse. Leaving it asserted would make
                // the tick postamble restart recovery after state 7 reaches idle.
                cancellation_calls.extend(e.copy_input(copy, state + 0x20, &[0], &mut observe)?);
                cancellation_pending = false;
            }
            tick_calls += 1;
            let current = e.bus.read(state + 0x21, Width::Byte)? as u8;
            if transitions
                .last()
                .is_none_or(|entry: &Transition| entry.state != current)
            {
                transitions.push(Transition {
                    state: current,
                    frame: e.audio_frames()? - capture_start_frame,
                });
            }
            next_tick += TICK_PERIOD;
            continue;
        }
        let boundary = match body_frames {
            Some(body) if frames < body => body,
            Some(body) if frames < body + options.release_frames => body + options.release_frames,
            _ => options.safety_frames,
        };
        let mut collected_frame = e.audio_frames()?;
        while e.clock.ticks() < next_tick && e.audio_frames()? - capture_start_frame < boundary {
            e.call_observed(foreground, [0; 4], 100_000, &mut observe)?;
            let advanced_frames = e.audio_frames()?;
            if advanced_frames != collected_frame {
                pcm.extend(e.take_audio_frames()?.into_iter().flatten());
                collected_frame = advanced_frames;
            }
            if advanced_frames - capture_start_frame >= options.safety_frames {
                // The final callback can cross a frame limit. The outer loop
                // checks idle and exact body/tail coverage before publication.
                break;
            }
        }
    }
    let body_frames = body_frames.unwrap();
    let output_frames = body_frames + options.release_frames;
    if output_frames > options.safety_frames {
        return Err("XA render: cue completion plus tail exceeds the safety limit".into());
    }
    let output_samples = usize::try_from(output_frames * 2)?;
    if pcm.len() < output_samples {
        return Err("XA render: retained PCM is shorter than the requested body and tail".into());
    }
    pcm_observer(&pcm)?;
    pcm.truncate(output_samples);
    let (pcm, pcm16_delivery) = psx_render::deliver_pcm16(pcm);
    let queue = e.bus.cd_audio()?.statistics();
    let report = Report {
        schema: "bof3.xa-render/v2", engine: "owned_us_driver", sample_rate: SAMPLE_RATE,
        source, toc: toc.clone(), cue, history: options.history, startup: started.services.evidence,
        initialization_calls, play_call, cancellation_calls, requested_duration_frames: options.duration_frames,
        body_frames, release_frames: options.release_frames, output_frames, safety_frames: options.safety_frames,
        capture_start_frame, stop_request_frame, idle_frame: idle_frame.unwrap(), tick_period_cpu_cycles: TICK_PERIOD,
        tick_calls, states_at_tick_return: transitions, commands,
        final_queue: serde_json::json!({"queued_frames":queue.queued_frames,"pending_frames":queue.pending_frames,
            "admitted_sectors":queue.admitted_sectors,"muted_sectors":queue.muted_sectors,
            "dropped_sectors":queue.dropped_sectors,"consumed_frames":queue.consumed_frames,
            "empty_frames":queue.empty_frames,"resets":queue.resets,"predictor_resets":queue.predictor_resets,
            "reset_discarded_frames":queue.reset_discarded_frames}),
        executed_frames: e.audio_frames()? - capture_start_frame,
        execution_instructions: e.cpu.instructions() - instruction_start,
        output_clock: serde_json::to_value(e.audio_clock()?)?, pcm16_delivery,
        independent_pcm_validated: false,
        limitations: vec![
            "Cold playback uses a supplied authenticated, closed and spinning disc context, a complete recognized US XA source and the original packed cue ID. Repeated or incoming playback histories are unsupported.",
            "XA ticks run every 564480 interpreter CPU cycles, at safe foreground boundaries. This host cadence, service latency and production host-call phase are not independently accepted native timing.",
            "The cue's encoded extent and guest GetlocL stop threshold differ. Cue completion means the guest reaches idle after Pause; it does not establish the original audible endpoint. The requested tail may cut queued audio.",
            "The SPU clock runs through game sound initialization. Capture starts before XA START without resetting its phase. Independent runtime PCM agreement and hardware fidelity remain unverified.",
            "The supplied TOC controls track and position metadata. Only the first raw DATA track is attached; CDDA playback and disc authentication are outside this path.",
        ],
    };
    Ok(Rendered {
        wave: Wave::new(2, SAMPLE_RATE, pcm)?,
        report,
    })
}
