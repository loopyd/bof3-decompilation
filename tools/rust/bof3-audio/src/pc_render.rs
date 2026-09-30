//! Validated SMF/SoundFont inspection playback through the approved Rust engine.
//! BOF3 archive translation, PSX comparison and delivery codecs remain separate.

use crate::{
    digest::sha256_hex,
    interchange::midi::{Message, Midi},
    interchange::wave::Wave,
    Result,
};
use rustysynth::{SoundFont, Synthesizer, SynthesizerSettings};
use serde::Serialize;
use std::{collections::BTreeSet, io::Cursor, sync::Arc};

#[derive(Clone, Debug)]
pub struct Options {
    pub sample_rate: u32,
    /// Complete SMF restarts, not inferred BOF3 controller-loop traversals.
    pub repeats: u32,
    /// Fixed body duration; after the final replay, sustain the final synth state.
    pub duration_frames: Option<u64>,
    pub release_frames: u64,
    pub safety_frames: u64,
}

impl Default for Options {
    fn default() -> Self {
        Self {
            sample_rate: 44100,
            repeats: 1,
            duration_frames: None,
            release_frames: 88200,
            safety_frames: 44100 * 600,
        }
    }
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Scheduled {
    pub frame: u64,
    pub status: u8,
    pub data: Vec<u8>,
    pub track: usize,
    pub event: usize,
}

#[derive(Clone, Debug)]
pub struct Schedule {
    pub events: Vec<Scheduled>,
    pub frames: u64,
    pub metadata_events: usize,
}

/// Validate before synthesis, and accumulate rational time without rounding each
/// delta. Equal-tick order is track index then event order; conductor track first.
pub fn schedule(midi: &Midi, sample_rate: u32) -> Result<Schedule> {
    midi.to_bytes()?; // Validate mutable public fields before indexing payloads.
    if !(16000..=192000).contains(&sample_rate) {
        return Err("PC render: sample rate must be 16000..192000 Hz".into());
    }
    let mut events: Vec<_> = midi
        .tracks
        .iter()
        .enumerate()
        .flat_map(|(track, t)| {
            t.events
                .iter()
                .enumerate()
                .map(move |(index, event)| (event.tick, track, index, event))
        })
        .collect();
    events.sort_by_key(|&(tick, track, index, _)| (tick, track, index));
    let mut tick = 0;
    let mut numerator = 0u128;
    let denominator = u128::from(midi.ppqn) * 1_000_000;
    let mut tempo = 500_000u32;
    let mut result = Schedule {
        events: Vec::new(),
        frames: 0,
        metadata_events: 0,
    };
    for (next_tick, track, index, event) in events {
        numerator = numerator
            .checked_add(u128::from(next_tick - tick) * u128::from(tempo) * u128::from(sample_rate))
            .ok_or("PC render: timing overflow")?;
        tick = next_tick;
        result.frames = (numerator / denominator)
            .try_into()
            .map_err(|_| "PC render: frame count overflow")?;
        let fail = |detail: &str| -> Box<dyn std::error::Error + Send + Sync> {
            format!("PC render track {track} event {index} tick {tick}: {detail}").into()
        };
        match &event.message {
            Message::Channel { status, data } => {
                match status & 0xf0 {
                    0x80 | 0x90 | 0xc0 | 0xe0 => (),
                    0xb0 if matches!(
                        data[0],
                        0 | 1 | 7 | 10 | 11 | 33 | 39 | 42 | 43 | 64 | 91 | 93 | 120 | 121 | 123
                    ) => {}
                    0xb0 => {
                        return Err(fail(&format!(
                        "unsupported controller {}; NRPN/RPN and game loops require translation",
                        data[0]
                    )))
                    }
                    _ => return Err(fail(&format!("unsupported channel status {status:#04x}"))),
                }
                result.events.push(Scheduled {
                    frame: result.frames,
                    status: *status,
                    data: data.clone(),
                    track,
                    event: index,
                });
            }
            Message::Meta { kind, data } => {
                result.metadata_events += 1;
                match *kind {
                    0x51 if data.len() == 3 && (midi.format == 0 || track == 0) => {
                        tempo = u32::from_be_bytes([0, data[0], data[1], data[2]]);
                        if tempo == 0 { return Err(fail("zero tempo")); }
                    }
                    0x2f => (),
                    0x01..=0x07 => (), // Textual metadata has no MIDI synthesis effect.
                    0x00 if data.len() == 2 => (),
                    0x58 if data.len() == 4 && data[0] != 0 && data[1] <= 7 => (),
                    0x59 if data.len() == 2 && (-7..=7).contains(&(data[0] as i8)) && data[1] <= 1 => (),
                    _ => return Err(fail(&format!("unsupported or malformed meta {kind:#04x}; format-1 tempo belongs in track 0"))),
                }
            }
            Message::SysEx { .. } => return Err(fail("SysEx playback is unsupported")),
        }
    }
    if result.frames == 0 {
        return Err("PC render: zero-length MIDI is not repeatable".into());
    }
    Ok(result)
}

#[derive(Clone, Debug, Serialize)]
pub struct Report {
    pub schema: &'static str,
    pub engine: &'static str,
    pub midi_sha256: String,
    pub soundfont_sha256: String,
    pub sample_rate: u32,
    pub channels: u16,
    pub sequence_frames: u64,
    pub body_frames: u64,
    pub release_frames: u64,
    pub output_frames: u64,
    pub requested_repeats: u32,
    pub started_repeats: u32,
    pub duration_cutoff: bool,
    pub post_sequence_frames: u64,
    pub safety_frames: u64,
    pub scheduled_channel_events: usize,
    pub metadata_events: usize,
    pub clipped_samples: u64,
    pub peak: f32,
    pub limitations: Vec<&'static str>,
}

pub struct Rendered {
    pub wave: Wave,
    pub report: Report,
}

pub fn render(midi_bytes: &[u8], sf2_bytes: &[u8], options: &Options) -> Result<Rendered> {
    if options.repeats == 0 {
        return Err("PC render: repeats must be positive".into());
    }
    let midi = Midi::from_bytes(midi_bytes)?;
    let schedule = schedule(&midi, options.sample_rate)?;
    let total = schedule
        .frames
        .checked_mul(u64::from(options.repeats))
        .ok_or("PC render: repeat duration overflow")?;
    let body = options.duration_frames.unwrap_or(total);
    if body == 0 {
        return Err("PC render: duration must be positive".into());
    }
    let output = body
        .checked_add(options.release_frames)
        .ok_or("PC render: duration overflow")?;
    if output > options.safety_frames {
        return Err(format!(
            "PC render: {output} frames exceed safety limit {}; no output published",
            options.safety_frames
        )
        .into());
    }
    if output > (u64::from(u32::MAX) - 64) / 4 {
        return Err("PC render: PCM exceeds RIFF capacity".into());
    }
    let font = Arc::new(SoundFont::new(&mut Cursor::new(sf2_bytes))?);
    require_presets(&schedule, &font)?;
    let mut settings = SynthesizerSettings::new(options.sample_rate as i32);
    settings.block_size = 8;
    settings.maximum_polyphony = 256;
    settings.enable_reverb_and_chorus = true;
    let mut synth = Synthesizer::new(&font, &settings)?;
    synth.set_master_volume(0.5);
    let capacity = usize::try_from(output)?
        .checked_mul(2)
        .ok_or("PC render: allocation overflow")?;
    let mut pcm = Vec::new();
    pcm.try_reserve_exact(capacity)?;
    let mut report = Report {
        schema: "bof3.audio.pc-render/v1", engine: "rustysynth-1.3.6",
        midi_sha256: sha256_hex(midi_bytes), soundfont_sha256: sha256_hex(sf2_bytes),
        sample_rate: options.sample_rate, channels: 2, sequence_frames: schedule.frames,
        body_frames: body, release_frames: options.release_frames, output_frames: output,
        requested_repeats: options.repeats, started_repeats: 0, duration_cutoff: body < total,
        post_sequence_frames: body.saturating_sub(total),
        safety_frames: options.safety_frames, scheduled_channel_events: schedule.events.len(),
        metadata_events: schedule.metadata_events, clipped_samples: 0, peak: 0.0,
        limitations: vec![
            "Whole-file repeats reset synthesis state; BOF3 controller-loop regions are not inferred.",
            "MIDI time is floored to output frames; the engine applies changes at 8-frame block boundaries (up to 7 additional frames).",
            "RustySynth SoundFont behavior, 256-voice limit, effects enabled, master volume 0.5; voice stealing and SF2 approximations remain possible.",
            "PCM16 rounding and clipping are reported; no dither is applied.",
            "The fixed release tail may end before long envelopes or effects have decayed.",
            "PSX fidelity, BOF3 event translation and MP3/Ogg delivery are not verified by this render.",
        ],
    };
    let mut rendered = 0;
    while rendered < body && report.started_repeats < options.repeats {
        synth.reset();
        report.started_repeats += 1;
        let length = schedule.frames.min(body - rendered);
        let mut at = 0;
        for event in &schedule.events {
            if event.frame > length {
                break;
            }
            append(&mut synth, event.frame - at, &mut pcm, &mut report)?;
            at = event.frame;
            synth.process_midi_message(
                i32::from(event.status & 15),
                i32::from(event.status & 0xf0),
                i32::from(event.data[0]),
                i32::from(*event.data.get(1).unwrap_or(&0)),
            );
        }
        append(&mut synth, length - at, &mut pcm, &mut report)?;
        rendered += length;
    }
    // A fixed duration does not imply another restart after the requested
    // replays. Let the file's final notes, releases and effects continue.
    append(&mut synth, body - rendered, &mut pcm, &mut report)?;
    for channel in 0..16 {
        synth.process_midi_message(channel, 0xb0, 64, 0);
    }
    synth.note_off_all(false);
    append(&mut synth, options.release_frames, &mut pcm, &mut report)?;
    Ok(Rendered {
        wave: Wave::new(2, options.sample_rate, pcm)?,
        report,
    })
}

pub(crate) fn require_presets(schedule: &Schedule, font: &SoundFont) -> Result<()> {
    let presets: BTreeSet<_> = font
        .get_presets()
        .iter()
        .map(|p| (p.get_bank_number(), p.get_patch_number()))
        .collect();
    let mut bank = [0i32; 16];
    bank[9] = 128;
    let mut program = [0i32; 16];
    for event in &schedule.events {
        let channel = usize::from(event.status & 15);
        match event.status & 0xf0 {
            0xb0 if event.data[0] == 0 => {
                bank[channel] = i32::from(event.data[1]) + if channel == 9 { 128 } else { 0 }
            }
            0xc0 => program[channel] = i32::from(event.data[0]),
            0x90 if event.data[1] != 0 && !presets.contains(&(bank[channel], program[channel])) => {
                return Err(format!("PC render track {} event {}: missing SoundFont bank {} program {} for channel {}; timbre substitution rejected",
                    event.track, event.event, bank[channel], program[channel], channel).into());
            }
            _ => (),
        }
    }
    Ok(())
}

fn append(
    synth: &mut Synthesizer,
    frames: u64,
    pcm: &mut Vec<i16>,
    report: &mut Report,
) -> Result<()> {
    let mut left = [0f32; 1024];
    let mut right = [0f32; 1024];
    let mut remaining = frames;
    while remaining != 0 {
        let count = remaining.min(left.len() as u64) as usize;
        synth.render(&mut left[..count], &mut right[..count]);
        for (&l, &r) in left[..count].iter().zip(&right[..count]) {
            for sample in [l, r] {
                if !sample.is_finite() {
                    return Err("PC render: engine produced nonfinite PCM".into());
                }
                report.peak = report.peak.max(sample.abs());
                let scaled = (f64::from(sample) * 32768.0).round();
                if scaled < f64::from(i16::MIN) || scaled > f64::from(i16::MAX) {
                    report.clipped_samples += 1;
                }
                pcm.push(scaled.clamp(f64::from(i16::MIN), f64::from(i16::MAX)) as i16);
            }
        }
        remaining -= count as u64;
    }
    Ok(())
}
