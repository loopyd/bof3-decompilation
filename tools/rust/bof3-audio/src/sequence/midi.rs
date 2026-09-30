//! Finite SEP execution-order translation to a conductor/performance SMF pair.
//! Bank binding and measured game/SF2 response are separate acceptance gates.

use crate::{
    interchange::midi::{Event, Message, Midi, Track},
    sequence::events::Kind,
    sequence::timeline::{self, Limits, Stop, Timeline},
    sequence::Sequence,
    Result,
};
use serde::Serialize;

#[derive(Clone, Debug, Serialize)]
pub struct Mapping {
    pub step: usize,
    pub source_cursor: usize,
    pub track: usize,
    pub event: usize,
}

#[derive(Clone, Debug, Serialize)]
pub struct Tempo {
    pub tick: u64,
    pub source_microseconds_per_quarter: u32,
    pub linked_integer_bpm: u32,
    pub linked_bpm_microseconds_per_quarter: f64,
}

#[derive(Clone, Debug, Serialize)]
pub struct Report {
    pub schema: &'static str,
    pub sequence_index: usize,
    pub sequence_id: u16,
    pub resolution: u16,
    pub final_tick: u64,
    pub stop: Stop,
    pub executed_events: usize,
    pub ignored_bend_low_bytes: usize,
    pub tempos: Vec<Tempo>,
    pub mappings: Vec<Mapping>,
    pub limitations: Vec<&'static str>,
}

pub struct Translation {
    pub midi: Midi,
    pub timeline: Timeline,
    pub report: Report,
}

pub fn translate(sequence: &Sequence, bytes: &[u8], limits: &Limits) -> Result<Translation> {
    if sequence.data_bytes != bytes.len() {
        return Err("SEP MIDI: selected sequence byte length does not match header".into());
    }
    if sequence.resolution == 0 || sequence.resolution > 32767 {
        return Err("SEP MIDI: resolution must be PPQN 1..32767".into());
    }
    if !(1..=0xffffff).contains(&sequence.tempo_us) {
        return Err("SEP MIDI: header tempo must be a nonzero 24-bit value".into());
    }
    if sequence.time_numerator == 0 || sequence.time_denominator_power > 7 {
        return Err("SEP MIDI: unsupported time signature in sequence header".into());
    }
    let timeline = timeline::trace(bytes, limits)?;
    let meta = |kind, data| Message::Meta { kind, data };
    let mut conductor = vec![
        event(0, meta(0, sequence.sequence_id.to_be_bytes().to_vec())),
        event(
            0,
            meta(
                3,
                format!("BOF3 sequence {}", sequence.sequence_index).into_bytes(),
            ),
        ),
        event(0, tempo_message(sequence.tempo_us)),
        event(
            0,
            meta(
                0x58,
                vec![
                    sequence.time_numerator,
                    sequence.time_denominator_power,
                    24,
                    8,
                ],
            ),
        ),
    ];
    let mut performance = Vec::new();
    let mut mappings = Vec::new();
    let mut tempos = vec![tempo_report(0, sequence.tempo_us)];
    let mut ignored_bend_low_bytes = 0;
    let mut channels = [false; 16];
    for (index, step) in timeline.steps.iter().enumerate() {
        let mut track = 1;
        let message = match step.kind {
            Kind::Note { key, velocity } => {
                channels[usize::from(step.status & 15)] = true;
                channel(step.status, vec![key, velocity])
            }
            Kind::Controller {
                controller: 7 | 10,
                value,
            } => {
                let Kind::Controller { controller, .. } = step.kind else {
                    unreachable!()
                };
                channel(step.status, vec![controller, value])
            }
            Kind::Controller { controller, value } => {
                // The timeline already validated loop semantics. Keep their
                // source association as text markers, never standard NRPNs.
                meta(
                    6,
                    format!("BOF3 source={} CC{controller}={value}", step.source_cursor)
                        .into_bytes(),
                )
            }
            Kind::Program { program } => channel(step.status, vec![program]),
            Kind::PitchBend { low, high } => {
                ignored_bend_low_bytes += usize::from(low != 0);
                channel(step.status, vec![0, high])
            }
            Kind::Tempo {
                microseconds_per_quarter,
            } => {
                track = 0;
                tempos.push(tempo_report(step.tick, microseconds_per_quarter));
                tempo_message(microseconds_per_quarter)
            }
            Kind::End => meta(
                6,
                format!("BOF3 source={} end", step.source_cursor).into_bytes(),
            ),
        };
        let events = if track == 0 {
            &mut conductor
        } else {
            &mut performance
        };
        mappings.push(Mapping {
            step: index,
            source_cursor: step.source_cursor,
            track,
            event: events.len(),
        });
        events.push(event(step.tick, message));
    }
    // A chosen finite inspection boundary must release notes, including when an
    // infinite controller loop never reaches the source end marker. These are
    // generated termination events, not claimed original controller messages.
    for (index, used) in channels.into_iter().enumerate() {
        if used {
            performance.push(event(
                timeline.final_tick,
                channel(0xb0 | index as u8, vec![123, 0]),
            ));
        }
    }
    conductor.push(event(timeline.final_tick, Message::end()));
    performance.push(event(timeline.final_tick, Message::end()));
    let midi = Midi::new(
        1,
        sequence.resolution,
        vec![Track::new(conductor), Track::new(performance)],
    )?;
    let report = Report {
        schema: "bof3.audio.sequence-midi/v1", sequence_index: sequence.sequence_index,
        sequence_id: sequence.sequence_id, resolution: sequence.resolution, final_tick: timeline.final_tick,
        stop: timeline.stop.clone(), executed_events: timeline.steps.len(), ignored_bend_low_bytes,
        tempos, mappings,
        limitations: vec![
            "Finite execution-order expansion; controller loops are text markers, not MIDI NRPNs. Render this expansion once, without additional whole-file repeats.",
            "Source ticks and tempo values are retained; integer-BPM conversion and game scheduler quantization are reported, not emulated by SMF timing.",
            "Bend low bytes are zeroed for playback because the linked callback discards them; original values remain in the source timeline. Tone-specific bend ranges require bank binding.",
            "Programs and volume/pan values retain their source numbers. SF2 program/channel binding and measured game/SF2 gain, pan and envelope response are not yet accepted.",
            "Sequence index and game ID remain distinct; archive/entry qualification belongs to the extraction manifest.",
            "The final all-notes-off messages implement the chosen export boundary. Whole-sequence restart/successor behavior and release-tail audio are separate.",
            "Time-signature metronome/32nd fields use SMF defaults 24/8; SEP supplies only numerator/denominator.",
            "Reconstruction requires the separate sequence-edit validator and original SEP preservation bytes; arbitrary expanded MIDI edits are unsupported.",
        ],
    };
    Ok(Translation {
        midi,
        timeline,
        report,
    })
}

fn event(tick: u64, message: Message) -> Event {
    Event { tick, message }
}
fn channel(status: u8, data: Vec<u8>) -> Message {
    Message::Channel { status, data }
}
fn tempo_message(value: u32) -> Message {
    Message::Meta {
        kind: 0x51,
        data: value.to_be_bytes()[1..].to_vec(),
    }
}
fn tempo_report(tick: u64, value: u32) -> Tempo {
    let bpm = 60_000_000 / value;
    Tempo {
        tick,
        source_microseconds_per_quarter: value,
        linked_integer_bpm: bpm,
        linked_bpm_microseconds_per_quarter: 60_000_000.0 / f64::from(bpm),
    }
}

/// Add the channel defaults shared by song export and edit reconstruction.
pub fn initialize_channels(translation: &mut Translation) -> Result<usize> {
    let track = translation
        .midi
        .tracks
        .get_mut(1)
        .ok_or("SEP MIDI: translated performance track missing")?;
    let mut used = [false; 16];
    for event in &track.events {
        if let Message::Channel { status, .. } = &event.message {
            used[usize::from(status & 15)] = true;
        }
    }
    let mut setup = Vec::new();
    for (channel, used) in used.into_iter().enumerate() {
        if used {
            setup.push(Event {
                tick: 0,
                message: Message::Channel {
                    status: 0xc0 | channel as u8,
                    data: vec![channel as u8],
                },
            });
            for (controller, value) in [
                (0, 0),
                (7, 127),
                (39, 127),
                (11, 127),
                (43, 127),
                (10, 64),
                (42, 0),
                (91, 0),
                (93, 0),
            ] {
                setup.push(Event {
                    tick: 0,
                    message: Message::Channel {
                        status: 0xb0 | channel as u8,
                        data: vec![controller, value],
                    },
                });
            }
            setup.push(Event {
                tick: 0,
                message: Message::Channel {
                    status: 0xe0 | channel as u8,
                    data: vec![0, 64],
                },
            });
        }
    }
    let count = setup.len();
    setup.append(&mut track.events);
    track.events = setup;
    for mapping in &mut translation.report.mappings {
        if mapping.track == 1 {
            mapping.event += count;
        }
    }
    Ok(count)
}
