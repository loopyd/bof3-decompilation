use bof3_audio::{
    interchange::midi::{Message, Midi},
    sequence::editing::{self, Options},
    sequence::events::Events,
    sequence::midi as translation,
    sequence::SequenceSet,
};

fn entry(parts: &[&[u8]]) -> Vec<u8> {
    let mut bytes = b"pQES\0\0".to_vec();
    for (i, part) in parts.iter().enumerate() {
        bytes.extend_from_slice(&(17 + i as u16).to_be_bytes());
        bytes.extend_from_slice(&96u16.to_be_bytes());
        bytes.extend_from_slice(&[7, 0xa1, 0x20, 4, 2]);
        bytes.extend_from_slice(&(part.len() as u32).to_be_bytes());
        bytes.extend_from_slice(part);
    }
    bytes.extend_from_slice(&[0; 17]);
    bytes
}
fn translate(bytes: &[u8], index: usize) -> translation::Translation {
    let s = &SequenceSet::parse(bytes).unwrap().sequences[index];
    let mut result = translation::translate(
        s,
        &bytes[s.data_offset..s.data_offset + s.data_bytes],
        &Options::default().limits,
    )
    .unwrap();
    translation::initialize_channels(&mut result).unwrap();
    result
}
fn scale(midi: &mut Midi, factor: u64) {
    for track in &mut midi.tracks {
        for event in &mut track.events {
            event.tick *= factor;
        }
    }
}
fn equivalent(wanted: &Midi, actual: &Midi) {
    for (a, b) in wanted.tracks.iter().zip(&actual.tracks) {
        assert_eq!(a.events.len(), b.events.len());
        for (a, b) in a.events.iter().zip(&b.events) {
            assert_eq!(a.tick, b.tick);
            if !matches!(a.message, Message::Meta { kind: 6, .. }) {
                assert_eq!(a.message, b.message);
            }
        }
    }
    rustysynth::MidiFile::new(&mut std::io::Cursor::new(actual.to_bytes().unwrap())).unwrap();
}

#[test]
fn resized_deltas_relocate_records_preserve_suffixes_and_combine_value_edits() {
    let source = [0x80, 0, 0x90, 60, 100, 3, 60, 0, 0, 0xff, 0x2f, 0xde, 0xad];
    let original = entry(&[&source, &[0, 0xff, 0x2f, 0xab]]);
    let before = SequenceSet::parse(&original).unwrap();
    let baseline = translate(&original, 0);
    let mut edited = baseline.midi.clone();
    scale(&mut edited, 100);
    let first = &baseline.report.mappings[0];
    if let Message::Channel { data, .. } =
        &mut edited.tracks[first.track].events[first.event].message
    {
        data[1] = 93;
    }
    let rebuilt = editing::rebuild_entry(&original, &[(0, &edited)], &Options::default()).unwrap();
    let after = SequenceSet::parse(&rebuilt.bytes).unwrap();
    assert_eq!(
        after.sequences[1].data_offset,
        before.sequences[1].data_offset + 1
    );
    assert_eq!(
        &rebuilt.bytes[after.sequences[1].data_offset - 13..],
        &original[before.sequences[1].data_offset - 13..]
    );
    assert_eq!(&rebuilt.bytes[19..21], &[0x80, 0]);
    assert_eq!(
        &rebuilt.bytes[after.sequences[1].data_offset - 15..after.sequences[1].data_offset - 13],
        &[0xde, 0xad]
    );
    let report = &rebuilt.reports[0];
    assert_eq!(report.timing_edits.len(), 1);
    assert_eq!(report.changed_source_bytes, 1);
    assert_eq!(report.output_data_bytes, report.original_data_bytes + 1);
    assert!(!report.original_bytes_reused);
    equivalent(&edited, &translate(&rebuilt.bytes, 0).midi);
    let unchanged = editing::rebuild_entry(
        &rebuilt.bytes,
        &[(0, &translate(&rebuilt.bytes, 0).midi)],
        &Options::default(),
    )
    .unwrap();
    assert_eq!(unchanged.bytes, rebuilt.bytes);
}

fn loop_source(count: u8) -> Vec<u8> {
    vec![
        0, 0xb0, 99, 20, 0, 98, count, 0, 0xc0, 1, 0, 0x90, 60, 100, 12, 0x90, 60, 0, 0, 0xb0, 99,
        30, 7, 0xff, 0x2f, 0xaa,
    ]
}

#[test]
fn finite_and_infinite_loops_use_consumed_delta_and_reject_conflicting_visits() {
    for count in [2, 127] {
        let original = entry(&[&loop_source(count)]);
        let baseline = translate(&original, 0);
        let mut edited = baseline.midi.clone();
        scale(&mut edited, 100);
        let rebuilt =
            editing::rebuild_entry(&original, &[(0, &edited)], &Options::default()).unwrap();
        equivalent(&edited, &translate(&rebuilt.bytes, 0).midi);
        let s = &SequenceSet::parse(&rebuilt.bytes).unwrap().sequences[0];
        let events =
            Events::parse(&rebuilt.bytes[s.data_offset..s.data_offset + s.data_bytes]).unwrap();
        assert_eq!(
            events.events.last().unwrap().delta,
            if count == 127 { 7 } else { 700 }
        );
        let note = baseline
            .report
            .mappings
            .iter()
            .find(|m| {
                matches!(
            edited.tracks[m.track].events[m.event].message,
            Message::Channel { status: 0x90, ref data } if data[1] != 0)
            })
            .unwrap();
        edited.tracks[note.track].events[note.event].tick += 1;
        let error = editing::rebuild_entry(&original, &[(0, &edited)], &Options::default())
            .err()
            .unwrap()
            .to_string();
        assert!(error.contains("disagrees with visit"), "{error}");
    }
}

#[test]
fn rejects_cross_track_reordering_forced_jump_delay_and_stale_termination() {
    let original = entry(&[&[
        0, 0x90, 60, 100, 2, 0xff, 0x51, 7, 0xa1, 0x20, 2, 0x90, 60, 0, 2, 0xff, 0x2f,
    ]]);
    let baseline = translate(&original, 0);
    let mut edited = baseline.midi.clone();
    edited.tracks[0].events[4].tick = 5;
    let error = editing::rebuild_entry(&original, &[(0, &edited)], &Options::default())
        .err()
        .unwrap()
        .to_string();
    assert!(error.contains("execution order reversed"), "{error}");
    let mut edited = baseline.midi.clone();
    scale(&mut edited, 2);
    edited.tracks[0].events.last_mut().unwrap().tick += 1;
    let error = editing::rebuild_entry(&original, &[(0, &edited)], &Options::default())
        .err()
        .unwrap()
        .to_string();
    assert!(error.contains("generated"), "{error}");
    let original = entry(&[&loop_source(127)]);
    let baseline = translate(&original, 0);
    let jump = baseline
        .timeline
        .steps
        .iter()
        .position(|s| s.jumped)
        .unwrap();
    let mut edited = baseline.midi.clone();
    let next = &baseline.report.mappings[jump + 1];
    // Move the whole second traversal and its generated boundary one tick later.
    for e in &mut edited.tracks[1].events[next.event..] {
        e.tick += 1;
    }
    edited.tracks[0].events.last_mut().unwrap().tick += 1;
    let error = editing::rebuild_entry(&original, &[(0, &edited)], &Options::default())
        .err()
        .unwrap()
        .to_string();
    assert!(error.contains("forces zero delay"), "{error}");
}

#[test]
fn initial_delay_can_grow_and_shrink_without_changing_generated_setup() {
    let original = entry(&[&[0x81, 0, 0x90, 60, 100, 0, 0xff, 0x2f]]);
    let baseline = translate(&original, 0);
    for tick in [0, 1, 16384, 214748364, 214748365] {
        let mut edited = baseline.midi.clone();
        for m in &baseline.report.mappings {
            edited.tracks[m.track].events[m.event].tick = tick;
        }
        for t in &mut edited.tracks {
            t.events.last_mut().unwrap().tick = tick;
        }
        let last = edited.tracks[1].events.len();
        edited.tracks[1].events[last - 2].tick = tick;
        let result = editing::rebuild_entry(&original, &[(0, &edited)], &Options::default());
        if tick == 214748365 {
            assert!(result
                .err()
                .unwrap()
                .to_string()
                .contains("overflows positive signed runtime"));
            continue;
        }
        let rebuilt = result.unwrap();
        equivalent(&edited, &translate(&rebuilt.bytes, 0).midi);
        assert_eq!(rebuilt.reports[0].timing_edits[0].output_value, tick as u32);
    }
}
