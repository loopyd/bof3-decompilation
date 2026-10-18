//! Linear event edits with complete semantic retranslation.

use std::collections::BTreeMap;

use super::{tempo, translate, Options, Rebuilt, Report};
use crate::{
    interchange::midi::{Event, Message, Midi},
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
    let last = baseline.report.mappings.last().unwrap();
    let before = &baseline.midi.tracks[1].events;
    let after = &edited.tracks[1].events;
    let mut first = 0;
    if options.initialize_channels {
        for channel in 0..16 {
            let setup = crate::sequence::midi::channel_setup(channel);
            if after[first..].starts_with(&setup) {
                first += setup.len();
            }
        }
    }
    let mut ends = after
        .iter()
        .enumerate()
        .filter(|(_, event)| event.message == before[last.event].message);
    let Some((end, end_event)) = ends.next() else {
        return Err(format!("{reject}: original source end marker missing").into());
    };
    if end < first || ends.next().is_some() {
        return Err(format!("{reject}: original source end marker duplicated or misplaced").into());
    }
    let final_tick = end_event.tick;
    if conductor.last().unwrap().message != Message::end()
        || conductor.last().unwrap().tick != final_tick
    {
        return Err(
            format!("{reject}: conductor termination must match the source end tick").into(),
        );
    }
    let mut normalized = edited.clone();
    let bend_lows = bend_lows(baseline, &after[first..end])?;
    let mut bend_lows = bend_lows.into_iter();
    let mut messages = Vec::<(u64, u8, Vec<u8>)>::new();
    for event in &mut normalized.tracks[1].events[first..end] {
        let location = format!("{reject}: performance tick {}", event.tick);
        let bytes = if is_note(&event.message) {
            let Message::Channel { status, data } = &mut event.message else {
                unreachable!()
            };
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
                    if data[0] != 0 {
                        return Err(format!(
                            "{location}: nonzero bend low byte is unrepresentable"
                        )
                        .into());
                    }
                    bytes.extend([bend_lows.next().unwrap(), data[1]]);
                }
                _ => return Err(format!("{location}: unsupported channel message").into()),
            }
            bytes
        };
        messages.push((event.tick, 1, bytes));
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
        return Err(format!("{reject}: reconstructed execution differs from requested MIDI; channel setup and termination must match the reconstructed channel sets").into());
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
                "Linear notes, tempo, program and volume/pan events support insertion/removal. Notes, programs and volume/pan may use any of the 16 channels when requested canonical setup and termination match the reconstructed channel sets. Rebuilt streams use explicit statuses and minimal deltas. Opaque suffixes and other sequence records survive.",
                "With layout_rebuilt, changed_source_bytes counts differing data positions and length; changed_source_events counts all rewritten or removed original events. timing_edits is empty because original delta identities are not retained.",
                "Equal-count bends retain ordered channels and ignored low bytes. Bend insertion/removal requires an exact ordered channel/value/tick subsequence; all valid correspondences must agree on retained low bytes. Inserted bends use zero. Generated channel defaults remain canonical and note releases follow the reconstructed note channels. Controller-loop structure changes and other controller messages require separate semantics.",
                "Bank availability, scheduler timing, synthesis and independent audio fidelity are separate checks.",
            ],
        },
        sequence: updated,
        bytes,
    })
}

type BendKey = (u64, u8, u8);
type BendOccurrences = BTreeMap<BendKey, Vec<(usize, u8, usize)>>;

fn bend_lows(baseline: &Translation, events: &[Event]) -> Result<Vec<u8>> {
    let mut source = Vec::new();
    let mut lows = Vec::new();
    for step in &baseline.timeline.steps {
        if let Kind::PitchBend { low, high } = step.kind {
            source.push((step.tick, step.status, high));
            lows.push(low);
        }
    }
    let edited: Vec<_> = events
        .iter()
        .filter_map(|event| match &event.message {
            Message::Channel { status, data } if status & 0xf0 == 0xe0 => {
                Some((event.tick, *status, data[1]))
            }
            _ => None,
        })
        .collect();
    if source.len() == edited.len() {
        if source.iter().zip(&edited).any(|(a, b)| a.1 != b.1) {
            return Err("SEP bend edit: ordered bend channels changed".into());
        }
        return Ok(lows);
    }
    let inserting = source.len() < edited.len();
    let (short, long) = if inserting {
        (&source, &edited)
    } else {
        (&edited, &source)
    };
    let mut earliest = Vec::with_capacity(short.len());
    let mut next = 0;
    for key in short {
        while next < long.len() && long[next] != *key {
            next += 1;
        }
        if next == long.len() {
            return Err("SEP bend insertion/removal: remaining bends must retain an ordered source channel/value/tick subsequence".into());
        }
        earliest.push(next);
        next += 1;
    }
    let mut latest = vec![0; short.len()];
    next = long.len();
    for (index, key) in short.iter().enumerate().rev() {
        while long[next - 1] != *key {
            next -= 1;
        }
        next -= 1;
        latest[index] = next;
    }
    // Each matching position between the earliest and latest embeddings can
    // occur in a valid correspondence. Prefix change counts keep duplicate
    // checks logarithmic instead of enumerating the possible embeddings.
    let mut occurrences = BendOccurrences::new();
    for (index, (key, low)) in source.iter().zip(lows).enumerate() {
        let positions = occurrences.entry(*key).or_default();
        let changes = positions.last().map_or(0, |&(_, previous, count)| {
            count + usize::from(previous != low)
        });
        positions.push((index, low, changes));
    }
    edited
        .iter()
        .enumerate()
        .map(|(index, key)| {
            if !inserting {
                return unique_low(&occurrences, key, earliest[index], latest[index] + 1)
                    .map(|low| low.unwrap());
            }
            let first = latest.partition_point(|&position| position < index);
            let end = earliest.partition_point(|&position| position <= index);
            let low = unique_low(&occurrences, key, first, end)?;
            let before = earliest.partition_point(|&position| position < index);
            let after = source.len() - latest.partition_point(|&position| position <= index);
            if before + after >= source.len() && low.is_some_and(|low| low != 0) {
                return Err(
                    "SEP bend insertion/removal: ambiguous ignored low-byte correspondence".into(),
                );
            }
            Ok(low.unwrap_or(0))
        })
        .collect()
}

fn unique_low(
    occurrences: &BendOccurrences,
    key: &BendKey,
    first: usize,
    end: usize,
) -> Result<Option<u8>> {
    let Some(positions) = occurrences.get(key) else {
        return Ok(None);
    };
    let first = positions.partition_point(|&(index, _, _)| index < first);
    let end = positions.partition_point(|&(index, _, _)| index < end);
    if first == end {
        return Ok(None);
    }
    if positions[first].2 != positions[end - 1].2 {
        return Err("SEP bend insertion/removal: ambiguous ignored low-byte correspondence".into());
    }
    Ok(Some(positions[first].1))
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
