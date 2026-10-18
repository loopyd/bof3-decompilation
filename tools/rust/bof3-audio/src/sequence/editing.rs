//! Checked inverse of the bounded SEP MIDI export.
//!
//! Regenerate correspondence from original bytes, never from manifest claims.
//! This component does not validate bank availability or publish an archive.

use std::collections::{BTreeMap, BTreeSet};

use serde::Serialize;

use crate::{
    interchange::midi::{Message, Midi},
    sequence::events::Kind,
    sequence::midi::{self as translation, Translation},
    sequence::timeline::Limits,
    sequence::timing,
    sequence::{Sequence, SequenceSet},
    Result,
};

mod notes;

#[derive(Clone, Debug)]
pub struct Options {
    pub limits: Limits,
    pub initialize_channels: bool,
}

impl Default for Options {
    fn default() -> Self {
        Self {
            limits: Limits::default(),
            initialize_channels: true,
        }
    }
}

#[derive(Debug, Serialize)]
pub struct Report {
    pub schema: &'static str,
    pub sequence_index: usize,
    pub changed_source_bytes: usize,
    pub changed_source_events: usize,
    pub initial_tempo_changed: bool,
    pub original_bytes_reused: bool,
    pub layout_rebuilt: bool,
    pub timing_edits: Vec<timing::Delta>,
    pub original_data_bytes: usize,
    pub output_data_bytes: usize,
    pub limitations: Vec<&'static str>,
}

pub struct Rebuilt {
    pub sequence: Sequence,
    pub bytes: Vec<u8>,
    pub report: Report,
}

pub struct RebuiltEntry {
    pub bytes: Vec<u8>,
    pub reports: Vec<Report>,
}

fn translate(sequence: &Sequence, bytes: &[u8], options: &Options) -> Result<Translation> {
    let mut translation = translation::translate(sequence, bytes, &options.limits)?;
    if options.initialize_channels {
        translation::initialize_channels(&mut translation)?;
    }
    Ok(translation)
}

/// Rebuild supported events and timing, preserving unrelated source data.
pub fn rebuild(
    sequence: &Sequence,
    original: &[u8],
    edited: &Midi,
    options: &Options,
) -> Result<Rebuilt> {
    // Validate the whole input even when individual messages appear unchanged.
    edited.to_bytes()?;
    let mut baseline = translate(sequence, original, options)?;
    let mut container = edited.clone();
    container.tracks = baseline.midi.tracks.clone();
    if container.to_bytes()? != baseline.midi.to_bytes()? {
        return Err("SEP edit: MIDI format, PPQN, header or chunk layout changed".into());
    }
    if edited.tracks.len() != baseline.midi.tracks.len() {
        return Err("SEP edit: MIDI track count changed".into());
    }
    let mut patches = loop_counts(&baseline, edited)?;
    let mut changed_events = BTreeSet::new();
    if patches
        .iter()
        .any(|(&offset, &(value, _))| original[offset] != value)
    {
        let mut controlled = original.to_vec();
        for (&offset, &(value, step)) in &patches {
            controlled[offset] = value;
            if original[offset] != value {
                changed_events.insert(baseline.timeline.steps[step].source_cursor);
            }
        }
        baseline = translate(sequence, &controlled, options)?;
    }
    let controller_loop = baseline.timeline.steps.iter().any(|step| {
        matches!(
            step.kind,
            Kind::Controller {
                controller: 6 | 98 | 99,
                ..
            }
        )
    });
    if baseline
        .midi
        .tracks
        .iter()
        .zip(&edited.tracks)
        .any(|(before, after)| {
            before.events.len() != after.events.len()
                || before.events.iter().zip(&after.events).any(|(a, b)| {
                    notes::is_note(&a.message) != notes::is_note(&b.message)
                        || (!controller_loop && notes::identity_changed(&a.message, &b.message))
                })
        })
    {
        return notes::rebuild(sequence, original, edited, options, &baseline);
    }
    let timing = timing::derive(original, &baseline, edited)?;
    let mut normalized = edited.clone();
    let mut updated = sequence.clone();
    updated.tempo_us = tempo(&edited.tracks[0].events[2].message)?;
    let mut handled = BTreeSet::from([(0, 2)]);
    for mapping in &baseline.report.mappings {
        let step = &baseline.timeline.steps[mapping.step];
        let before = &baseline.midi.tracks[mapping.track].events[mapping.event].message;
        let after = &mut normalized.tracks[mapping.track].events[mapping.event].message;
        let location = format!(
            "SEP edit: source {} track {} event {}",
            step.source_cursor, mapping.track, mapping.event
        );
        let data_offset = step.source_cursor + usize::from(step.explicit_status);
        let patch = match step.kind {
            Kind::Note { .. } => {
                let (status, data) = match after {
                    Message::Channel { status, data } if matches!(*status & 0xf0, 0x80 | 0x90) => {
                        if *status & 0xf0 == 0x80 {
                            if data[1] != 0 {
                                return Err(format!(
                                "{location}: nonzero note-off release velocity is unrepresentable"
                            )
                                .into());
                            }
                            *status = 0x90 | (*status & 15);
                        }
                        (*status, data)
                    }
                    _ => return Err(format!("{location}: expected note message").into()),
                };
                // Include unchanged explicit statuses in repeated-visit checks.
                // Retranslation validates inherited status at implicit notes.
                if step.explicit_status {
                    let mut message = vec![status];
                    message.extend_from_slice(data);
                    Some((step.source_cursor, message))
                } else {
                    Some((data_offset, data.to_vec()))
                }
            }
            Kind::Program { .. } => Some((
                data_offset,
                channel(after, step.status, &location)?.to_vec(),
            )),
            Kind::Controller {
                controller: 7 | 10, ..
            } => {
                let data = channel(after, step.status, &location)?;
                if data[0] != original[data_offset] {
                    return Err(format!("{location}: controller identity changed").into());
                }
                Some((data_offset, data.to_vec()))
            }
            Kind::PitchBend { .. } => {
                let data = channel(after, step.status, &location)?;
                if data[0] != 0 {
                    return Err(
                        format!("{location}: nonzero bend low byte is unrepresentable").into(),
                    );
                }
                // The game ignores the source low byte. Retain its exact encoding.
                Some((data_offset + 1, vec![data[1]]))
            }
            Kind::Tempo { .. } => {
                let value = tempo(after).map_err(|error| format!("{location}: {error}"))?;
                Some((data_offset + 1, value.to_be_bytes()[1..].to_vec()))
            }
            _ => {
                if before != after {
                    return Err(format!("{location}: loop/end marker edit unsupported").into());
                }
                None
            }
        };
        if let Some((offset, data)) = patch {
            for (index, value) in data.into_iter().enumerate() {
                let offset = offset + index;
                if let Some(&(previous, visit)) = patches.get(&offset) {
                    if previous != value {
                        return Err(format!("{location}: repeated/overlapping source byte {offset} disagrees with visit {visit}").into());
                    }
                } else {
                    patches.insert(offset, (value, mapping.step));
                }
                if original[offset] != value {
                    changed_events.insert(step.source_cursor);
                }
            }
        }
        handled.insert((mapping.track, mapping.event));
    }
    for (track, (before, after)) in baseline
        .midi
        .tracks
        .iter()
        .zip(&normalized.tracks)
        .enumerate()
    {
        for (event, (before, after)) in before.events.iter().zip(&after.events).enumerate() {
            if handled.contains(&(track, event)) {
                continue;
            }
            let termination = event + 1 == baseline.midi.tracks[track].events.len()
                || (track == 1
                    && baseline
                        .report
                        .mappings
                        .iter()
                        .filter(|m| m.track == 1)
                        .all(|m| m.event < event));
            let expected_tick = if termination {
                timing.final_tick
            } else {
                before.tick
            };
            if before.message != after.message || after.tick != expected_tick {
                return Err(
                    format!("SEP edit: generated track {track} event {event} message/tick changed; expected tick {expected_tick}").into(),
                );
            }
        }
    }
    let mut bytes = original.to_vec();
    let mut changed_source_bytes = 0;
    for (offset, (value, _)) in patches {
        changed_source_bytes += usize::from(bytes[offset] != value);
        bytes[offset] = value;
    }
    // Initial tempo is editable, but its generated placement remains at tick zero.
    if normalized.tracks[0].events[2].tick != 0 {
        return Err("SEP edit: initial tempo tick must remain zero".into());
    }
    let bytes = timing::apply(&bytes, &timing);
    updated.data_bytes = bytes.len();
    let verified = translate(&updated, &bytes, options)?;
    if verified.report.mappings.len() != baseline.report.mappings.len() {
        return Err("SEP edit: reconstructed execution visit count changed".into());
    }
    // Source-offset text is generated correspondence, not an authored musical
    // edit. Only replace it after the original marker text passed validation.
    for (before, after) in baseline
        .report
        .mappings
        .iter()
        .zip(&verified.report.mappings)
    {
        if (before.step, before.track, before.event) != (after.step, after.track, after.event) {
            return Err("SEP edit: reconstructed event correspondence changed".into());
        }
        if matches!(
            baseline.midi.tracks[before.track].events[before.event].message,
            Message::Meta { kind: 6, .. }
        ) {
            normalized.tracks[before.track].events[before.event].message =
                verified.midi.tracks[after.track].events[after.event]
                    .message
                    .clone();
        }
    }
    if !verified
        .midi
        .tracks
        .iter()
        .zip(&normalized.tracks)
        .all(|(a, b)| a.events == b.events)
    {
        return Err("SEP edit: reconstructed execution differs from requested MIDI".into());
    }
    let initial_tempo_changed = updated.tempo_us != sequence.tempo_us;
    Ok(Rebuilt {
        sequence: updated,
        report: Report {
            schema: "bof3.audio.sequence-edit/v2",
            sequence_index: sequence.sequence_index,
            changed_source_bytes,
            changed_source_events: changed_events.len(),
            initial_tempo_changed,
            original_bytes_reused: bytes == original && !initial_tempo_changed,
            layout_rebuilt: false,
            timing_edits: timing.deltas,
            original_data_bytes: original.len(),
            output_data_bytes: bytes.len(),
            limitations: vec![
                "Event order, loop start/end structure and generated setup remain fixed. Count-controller 6/98 marker values may change when all requested visits agree and the requested expansion matches the rederived execution. Note-channel changes must agree at every source visit and reproduce inherited running status. Timing edits must agree at every repeated delta consumption. Infinite jumps force zero delay.",
                "changed_source_bytes/events count message-value edits; timing_edits records changed delta spans separately. Unchanged delta encodings and opaque suffixes survive; resized deltas relocate later sequence headers.",
                "Bank programs, tone pitch ranges, SF2 edits and publication require separate validation.",
                "Forward MIDI approximations and configured infinite-loop traversal bounds remain unchanged.",
            ],
        },
        bytes,
    })
}

fn loop_counts(baseline: &Translation, edited: &Midi) -> Result<BTreeMap<usize, (u8, usize)>> {
    let mut sources = BTreeMap::new();
    for (index, step) in baseline.timeline.steps.iter().enumerate() {
        if let Kind::Controller {
            controller: controller @ (6 | 98),
            ..
        } = step.kind
        {
            let identity = format!("BOF3 source={} CC{controller}", step.source_cursor);
            let offset = step.source_cursor + usize::from(step.explicit_status) + 1;
            sources.entry(identity).or_insert((offset, index));
        }
    }
    let mut patches = BTreeMap::new();
    for event in &edited.tracks[1].events {
        let Message::Meta { kind: 6, data } = &event.message else {
            continue;
        };
        let Ok(text) = std::str::from_utf8(data) else {
            continue;
        };
        let Some((identity, value)) = text.rsplit_once('=') else {
            continue;
        };
        let Some(&(offset, step)) = sources.get(identity) else {
            continue;
        };
        let value = value
            .parse::<u8>()
            .ok()
            .filter(|&value| value <= 127)
            .ok_or("SEP loop count edit: expected a seven-bit decimal marker value")?;
        if text != format!("{identity}={value}") {
            return Err("SEP loop count edit: noncanonical count marker".into());
        }
        if let Some(&(previous, _)) = patches.get(&offset) {
            if previous != value {
                return Err(format!(
                    "SEP loop count edit: source {} disagrees across requested visits",
                    baseline.timeline.steps[step].source_cursor
                )
                .into());
            }
        } else {
            patches.insert(offset, (value, step));
        }
    }
    Ok(patches)
}

/// Rebuild selected sequences; preserve other sequence records and trailing bytes.
pub fn rebuild_entry(
    original: &[u8],
    selections: &[(usize, &Midi)],
    options: &Options,
) -> Result<RebuiltEntry> {
    let parsed = SequenceSet::parse(original)?;
    let mut seen = BTreeSet::new();
    let mut replacements = BTreeMap::new();
    let mut reports = Vec::new();
    for &(index, midi) in selections {
        if !seen.insert(index) {
            return Err(format!("SEP edit: duplicate sequence index {index}").into());
        }
        let sequence = parsed
            .sequences
            .get(index)
            .ok_or_else(|| format!("SEP edit: sequence index {index} out of range"))?;
        let range = sequence.data_offset..sequence.data_offset + sequence.data_bytes;
        let rebuilt = rebuild(sequence, &original[range.clone()], midi, options)?;
        reports.push(rebuilt.report);
        replacements.insert(index, (rebuilt.sequence, rebuilt.bytes));
    }
    let mut bytes = original[..6].to_vec();
    for sequence in &parsed.sequences {
        let range = sequence.data_offset - 13..sequence.data_offset + sequence.data_bytes;
        let Some((rebuilt, data)) = replacements.remove(&sequence.sequence_index) else {
            bytes.extend_from_slice(&original[range]);
            continue;
        };
        let mut header = original[sequence.data_offset - 13..sequence.data_offset].to_vec();
        header[4..7].copy_from_slice(&rebuilt.tempo_us.to_be_bytes()[1..]);
        let length =
            u32::try_from(data.len()).map_err(|_| "SEP edit: sequence length exceeds u32")?;
        header[9..13].copy_from_slice(&length.to_be_bytes());
        bytes.extend(header);
        bytes.extend(data);
    }
    bytes.extend_from_slice(&original[parsed.trailing_offset..]);
    SequenceSet::parse(&bytes)?;
    Ok(RebuiltEntry { bytes, reports })
}

fn channel<'a>(message: &'a Message, expected: u8, location: &str) -> Result<&'a [u8]> {
    match message {
        Message::Channel { status, data } if *status == expected => Ok(data),
        _ => Err(format!("{location}: message type or channel edit unsupported").into()),
    }
}

fn tempo(message: &Message) -> Result<u32> {
    match message {
        Message::Meta { kind: 0x51, data } if data.len() == 3 => {
            let value = u32::from_be_bytes([0, data[0], data[1], data[2]]);
            if value == 0 {
                return Err("SEP edit: tempo must be nonzero".into());
            }
            Ok(value)
        }
        _ => Err("SEP edit: expected three-byte tempo event".into()),
    }
}
