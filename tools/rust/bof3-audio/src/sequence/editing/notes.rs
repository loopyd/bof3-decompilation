//! Linear event edits with complete semantic retranslation.

use super::{channel, tempo, translate, Options, Rebuilt, Report};
use crate::{
    interchange::midi::{Message, Midi},
    sequence::{events::Kind, midi::Translation, timeline::Stop, Sequence},
    Result,
};

pub(super) fn is_note(message: &Message) -> bool {
    matches!(message, Message::Channel { status, .. } if matches!(status & 0xf0, 0x80 | 0x90))
}

pub(super) fn identity_changed(before: &Message, after: &Message) -> bool {
    match (before, after) {
        (Message::Channel { status: a, data: x }, Message::Channel { status: b, data: y }) => {
            if is_note(before) && is_note(after) {
                return a & 15 != b & 15;
            }
            a != b || (a & 0xf0 == 0xb0 && x[0] != y[0])
        }
        _ => false,
    }
}

pub(super) fn rebuild(
    sequence: &Sequence,
    original: &[u8],
    edited: &Midi,
    options: &Options,
    baseline: &Translation,
) -> Result<Rebuilt> {
    let reject = "SEP event insertion/removal or channel edit";
    if baseline.timeline.stop != Stop::EndMarker
        || baseline.timeline.steps.iter().any(|step| {
            matches!(
                step.kind,
                Kind::Controller {
                    controller: 6 | 98 | 99,
                    ..
                }
            )
        })
    {
        return Err(format!(
            "{reject}: controller-loop structure requires repeated-visit correspondence"
        )
        .into());
    }
    let conductor = &edited.tracks[0].events;
    let original_conductor = &baseline.midi.tracks[0].events;
    if conductor.len() < 5 {
        return Err(format!("{reject}: generated conductor setup or termination missing").into());
    }
    for index in 0..4 {
        if index == 2 {
            tempo(&conductor[index].message)?;
            if conductor[index].tick != 0 {
                return Err(format!("{reject}: initial tempo tick must remain zero").into());
            }
        } else if conductor[index] != original_conductor[index] {
            return Err(format!("{reject}: generated conductor setup changed").into());
        }
    }
    let first = baseline
        .report
        .mappings
        .iter()
        .find(|m| m.track == 1)
        .unwrap()
        .event;
    let last = baseline.report.mappings.last().unwrap();
    let before = &baseline.midi.tracks[1].events;
    let after = &edited.tracks[1].events;
    let suffix = &before[last.event..];
    if after.len() < first + suffix.len() || after[..first] != before[..first] {
        return Err(format!("{reject}: generated performance setup changed").into());
    }
    let end = after.len() - suffix.len();
    let final_tick = after[end].tick;
    for (actual, expected) in after[end..].iter().zip(suffix) {
        if actual.message != expected.message || actual.tick != final_tick {
            return Err(
                format!("{reject}: generated termination or source end marker changed").into(),
            );
        }
    }
    if conductor.last().unwrap().message != Message::end()
        || conductor.last().unwrap().tick != final_tick
    {
        return Err(
            format!("{reject}: conductor termination must match the source end tick").into(),
        );
    }
    let mut note_channels = 0u16;
    let mut used_channels = 0u16;
    for step in &baseline.timeline.steps {
        if step.status != 0xff {
            used_channels |= 1 << (step.status & 15);
        }
        if matches!(step.kind, Kind::Note { .. }) {
            note_channels |= 1 << (step.status & 15);
        }
    }
    let mut normalized = edited.clone();
    let mut bends = baseline
        .timeline
        .steps
        .iter()
        .filter(|step| matches!(step.kind, Kind::PitchBend { .. }));
    let mut messages = Vec::<(u64, u8, Vec<u8>)>::new();
    for event in &mut normalized.tracks[1].events[first..end] {
        let location = format!("{reject}: performance tick {}", event.tick);
        let bytes = if is_note(&event.message) {
            let Message::Channel { status, data } = &mut event.message else {
                unreachable!()
            };
            if note_channels & (1 << (*status & 15)) == 0 {
                return Err(
                    format!("{location}: new channel requires different generated setup").into(),
                );
            }
            if *status & 0xf0 == 0x80 {
                if data[1] != 0 {
                    return Err(format!(
                        "{location}: nonzero note-off release velocity is unrepresentable"
                    )
                    .into());
                }
                *status = 0x90 | (*status & 15);
            }
            vec![*status, data[0], data[1]]
        } else {
            let Message::Channel { status, data } = &event.message else {
                return Err(format!("{location}: unsupported performance message").into());
            };
            if used_channels & (1 << (*status & 15)) == 0 {
                return Err(
                    format!("{location}: new channel requires different generated setup").into(),
                );
            }
            let mut bytes = vec![*status];
            match *status & 0xf0 {
                0xc0 => bytes.extend_from_slice(data),
                0xb0 => {
                    if !matches!(data[0], 7 | 10) {
                        return Err(format!("{location}: unsupported controller identity {}; only volume 7 and pan 10 are representable channel events", data[0]).into());
                    }
                    bytes.extend_from_slice(data);
                }
                0xe0 => {
                    let step = bends.next().ok_or_else(|| {
                        format!("{location}: bend insertion requires source correspondence")
                    })?;
                    let Kind::PitchBend { low, .. } = step.kind else {
                        unreachable!()
                    };
                    channel(&event.message, step.status, &location)?;
                    if data[0] != 0 {
                        return Err(format!(
                            "{location}: nonzero bend low byte is unrepresentable"
                        )
                        .into());
                    }
                    bytes.extend([low, data[1]]);
                }
                _ => return Err(format!("{location}: unsupported channel message").into()),
            }
            bytes
        };
        messages.push((event.tick, 1, bytes));
    }
    if bends.next().is_some() {
        return Err(format!("{reject}: bend removal requires source correspondence").into());
    }
    for event in &conductor[4..conductor.len() - 1] {
        let value = tempo(&event.message)?;
        let mut bytes = vec![0xff, 0x51];
        bytes.extend_from_slice(&value.to_be_bytes()[1..]);
        messages.push((event.tick, 0, bytes));
    }
    // Within each track, the validated MIDI order is stable. Tempo and channel
    // messages at the same logical tick commute in this translation contract.
    messages.sort_by_key(|(tick, track, _)| (*tick, *track));
    let mut bytes = Vec::new();
    let mut previous = 0;
    for (tick, _, message) in messages {
        delta(&mut bytes, tick, previous)?;
        bytes.extend(message);
        previous = tick;
    }
    delta(&mut bytes, final_tick, previous)?;
    bytes.extend([0xff, 0x2f]);
    bytes.extend_from_slice(&original[baseline.timeline.source_trailing_offset..]);
    let mut updated = sequence.clone();
    updated.tempo_us = tempo(&conductor[2].message)?;
    updated.data_bytes = bytes.len();
    let verified = translate(&updated, &bytes, options)?;
    let new_end = verified.report.mappings.last().unwrap();
    normalized.tracks[1].events[end].message = verified.midi.tracks[1].events[new_end.event]
        .message
        .clone();
    if !verified
        .midi
        .tracks
        .iter()
        .zip(&normalized.tracks)
        .all(|(a, b)| a.events == b.events)
    {
        return Err(format!("{reject}: reconstructed execution differs from requested MIDI; preserve generated channel setup and termination").into());
    }
    let changed_source_bytes = original.iter().zip(&bytes).filter(|(a, b)| a != b).count()
        + original.len().abs_diff(bytes.len());
    Ok(Rebuilt {
        report: Report {
            schema: "bof3.audio.sequence-edit/v2",
            sequence_index: sequence.sequence_index,
            changed_source_bytes,
            changed_source_events: baseline.timeline.steps.len(),
            initial_tempo_changed: updated.tempo_us != sequence.tempo_us,
            original_bytes_reused: false,
            layout_rebuilt: true,
            timing_edits: Vec::new(),
            original_data_bytes: original.len(),
            output_data_bytes: bytes.len(),
            limitations: vec![
                "Linear notes, tempo, program and volume/pan events support insertion/removal. Notes may move among previously used note channels; programs and volume/pan may move among previously used source channels. Rebuilt streams use explicit statuses and minimal deltas. Opaque suffixes and other sequence records survive.",
                "With layout_rebuilt, changed_source_bytes counts differing data positions and length; changed_source_events counts all rewritten or removed original events. timing_edits is empty because original delta identities are not retained.",
                "Ordered bend channels and ignored low bytes, generated setup and termination remain fixed. Bend insertion/removal, controller-loop structure changes, new channels and other controller messages require separate correspondence or semantics.",
                "Bank availability, scheduler timing, synthesis and independent audio fidelity are separate checks.",
            ],
        },
        sequence: updated,
        bytes,
    })
}

fn delta(bytes: &mut Vec<u8>, tick: u64, previous: u64) -> Result<()> {
    let value = tick
        .checked_sub(previous)
        .ok_or("SEP note edit: execution ticks go backwards")?;
    if value > (i32::MAX / 10) as u64 {
        return Err("SEP note edit: delta overflows positive signed runtime scaling".into());
    }
    let mut value = value as u32;
    let mut encoded = vec![(value & 127) as u8];
    value >>= 7;
    while value != 0 {
        encoded.push((value as u8 & 127) | 128);
        value >>= 7;
    }
    bytes.extend(encoded.into_iter().rev());
    Ok(())
}
