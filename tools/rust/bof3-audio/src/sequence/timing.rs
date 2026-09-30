//! Invert execution-order MIDI ticks to the SEP deltas actually consumed.
use crate::{interchange::midi::Midi, sequence::events, sequence::midi::Translation, Result};
use serde::Serialize;
use std::collections::BTreeMap;

#[derive(Debug, Serialize)]
pub struct Delta {
    pub source_offset: usize,
    pub original_value: u32,
    pub output_value: u32,
    pub original_bytes: usize,
    pub output_bytes: usize,
}

pub(crate) struct Timing {
    pub deltas: Vec<Delta>,
    pub final_tick: u64,
}

pub(crate) fn derive(original: &[u8], baseline: &Translation, edited: &Midi) -> Result<Timing> {
    let mut ticks = vec![None; baseline.timeline.steps.len()];
    for mapping in &baseline.report.mappings {
        ticks[mapping.step] = Some(edited.tracks[mapping.track].events[mapping.event].tick);
    }
    let ticks: Vec<u64> = ticks
        .into_iter()
        .collect::<Option<_>>()
        .ok_or("SEP timing edit: incomplete regenerated step mapping")?;
    let mut requests = BTreeMap::new();
    request(&mut requests, 0, ticks[0], 0)?;
    for (index, pair) in ticks.windows(2).enumerate() {
        let delay = pair[1].checked_sub(pair[0]).ok_or_else(|| {
            format!(
                "SEP timing edit: execution order reversed at visit {} across MIDI tracks",
                index + 1
            )
        })?;
        let step = &baseline.timeline.steps[index];
        if step.jumped && step.loops.remaining == 127 {
            if delay != 0 {
                return Err(format!(
                    "SEP timing edit: infinite jump at visit {index} forces zero delay"
                )
                .into());
            }
            // The consumed bytes do not determine this delay. Keep their encoding
            // unless another execution visit actually constrains the same delta.
            continue;
        }
        request(
            &mut requests,
            step.source_cursor + step.message_bytes,
            delay,
            index + 1,
        )?;
    }
    let mut ranges: Vec<_> = baseline
        .timeline
        .steps
        .iter()
        .map(|s| (s.source_cursor, s.source_cursor + s.message_bytes))
        .collect();
    ranges.sort_unstable();
    let mut messages: Vec<std::ops::Range<usize>> = Vec::new();
    for (start, end) in ranges {
        if let Some(last) = messages.last_mut().filter(|r| start <= r.end) {
            last.end = last.end.max(end);
        } else {
            messages.push(start..end);
        }
    }
    let mut deltas: Vec<Delta> = Vec::new();
    for (offset, (value, _)) in requests {
        let (before, end) = events::read_delta(original, offset)?;
        if before == value {
            continue;
        }
        let next = messages.partition_point(|r| r.end <= offset);
        if messages.get(next).is_some_and(|r| r.start < end) {
            return Err(format!(
                "SEP timing edit: delta at byte {offset} overlaps an executed message"
            )
            .into());
        }
        if deltas
            .last()
            .is_some_and(|d| d.source_offset + d.original_bytes > offset)
        {
            return Err("SEP timing edit: overlapping delta encodings cannot be relocated".into());
        }
        deltas.push(Delta {
            source_offset: offset,
            original_value: before,
            output_value: value,
            original_bytes: end - offset,
            output_bytes: encode(value).len(),
        });
    }
    Ok(Timing {
        deltas,
        final_tick: *ticks.last().unwrap(),
    })
}

fn request(
    requests: &mut BTreeMap<usize, (u32, usize)>,
    offset: usize,
    delay: u64,
    visit: usize,
) -> Result<()> {
    if delay > (i32::MAX / 10) as u64 {
        return Err(format!("SEP timing edit: delay {delay} at visit {visit} overflows positive signed runtime scaling").into());
    }
    let delay = delay as u32;
    if let Some(&(previous, previous_visit)) = requests.get(&offset) {
        if previous != delay {
            return Err(format!("SEP timing edit: delta at byte {offset} requested by visit {visit} disagrees with visit {previous_visit}").into());
        }
    } else {
        requests.insert(offset, (delay, visit));
    }
    Ok(())
}

/// Apply after value edits, whose offsets still refer to the original layout.
pub(crate) fn apply(bytes: &[u8], timing: &Timing) -> Vec<u8> {
    let mut output = Vec::new();
    let mut cursor = 0;
    for delta in &timing.deltas {
        output.extend_from_slice(&bytes[cursor..delta.source_offset]);
        output.extend(encode(delta.output_value));
        cursor = delta.source_offset + delta.original_bytes;
    }
    output.extend_from_slice(&bytes[cursor..]);
    output
}

fn encode(mut value: u32) -> Vec<u8> {
    let mut bytes = vec![(value & 127) as u8];
    value >>= 7;
    while value != 0 {
        bytes.push((value as u8 & 127) | 128);
        value >>= 7;
    }
    bytes.reverse();
    bytes
}
