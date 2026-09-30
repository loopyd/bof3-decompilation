use bof3_audio::{
    archive::MediaImage,
    interchange::midi::{Message, Midi},
    sequence::editing::{self, Options},
    sequence::midi as translation,
    sequence::SequenceSet,
};
use std::path::Path;
#[path = "../support/corpus/timing.rs"]
mod timing_corpus;

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
    let sequence = &SequenceSet::parse(bytes).unwrap().sequences[index];
    let mut translated = translation::translate(
        sequence,
        &bytes[sequence.data_offset..sequence.data_offset + sequence.data_bytes],
        &Options::default().limits,
    )
    .unwrap();
    translation::initialize_channels(&mut translated).unwrap();
    translated
}

const SOURCE: &[u8] = &[
    0x80, 0, 0xc3, 4, // Noncanonical initial delta.
    0, 0x93, 60, 100, 3, 62, 90, // Running note status.
    0, 0xe3, 17, 65, // Ignored low byte must survive reconstruction.
    0, 0xb3, 7, 90, 0, 10, 30, 4, 0xff, 0x51, 6, 0x1a, 0x80, 0, 0x51, 7, 0xa1,
    0x20, // SEP running meta status.
    5, 0x93, 60, 0, 0, 0xff, 0x2f, 0xde, 0xad, // Opaque suffix.
];

#[test]
fn fixed_value_edits_preserve_encoding_and_other_sequences() {
    let original = entry(&[SOURCE, &[0, 0xff, 0x2f, 0xaa]]);
    let translated = translate(&original, 0);
    let midi = Midi::from_bytes(&translated.midi.to_bytes().unwrap()).unwrap();
    let unchanged = editing::rebuild_entry(&original, &[(0, &midi)], &Options::default()).unwrap();
    assert_eq!(unchanged.bytes, original);
    assert!(unchanged.reports[0].original_bytes_reused);
    let mut edited = midi.clone();
    edited.tracks[0].events[2].message = Message::Meta {
        kind: 0x51,
        data: vec![6, 0x1a, 0x80],
    };
    let mut expected = original.clone();
    expected[10..13].copy_from_slice(&[6, 0x1a, 0x80]);
    let start = 19;
    for mapping in &translated.report.mappings {
        let step = &translated.timeline.steps[mapping.step];
        let offset = start + step.source_cursor + usize::from(step.explicit_status);
        let message = &mut edited.tracks[mapping.track].events[mapping.event].message;
        match message {
            Message::Channel { status, data } => match *status & 0xf0 {
                0xc0 => {
                    data[0] = 5;
                    expected[offset] = 5;
                }
                0x90 => {
                    data[0] += 1;
                    expected[offset] += 1;
                    if data[1] == 0 {
                        *status = 0x83;
                    }
                }
                0xe0 => {
                    data[1] = 66;
                    expected[offset + 1] = 66;
                }
                0xb0 => {
                    data[1] += 1;
                    expected[offset + 1] += 1;
                }
                _ => unreachable!(),
            },
            Message::Meta { kind: 0x51, data } => {
                data.copy_from_slice(&[9, 0x27, 0xc0]);
                expected[offset + 1..offset + 4].copy_from_slice(data);
            }
            _ => (),
        }
    }
    let rebuilt = editing::rebuild_entry(&original, &[(0, &edited)], &Options::default()).unwrap();
    assert_eq!(rebuilt.bytes, expected);
    assert!(rebuilt.reports[0].initial_tempo_changed);
    assert_eq!(rebuilt.reports[0].changed_source_events, 9);
    assert!(!rebuilt.reports[0].original_bytes_reused);
    let seq = &SequenceSet::parse(&original).unwrap().sequences[0];
    assert_eq!(
        &rebuilt.bytes[seq.data_offset + seq.data_bytes..],
        &original[seq.data_offset + seq.data_bytes..]
    );
    assert!(
        editing::rebuild_entry(&original, &[(0, &midi), (0, &midi)], &Options::default()).is_err()
    );
    assert!(editing::rebuild_entry(&original, &[(2, &midi)], &Options::default()).is_err());
}

#[test]
fn loop_visits_must_agree_including_unchanged_visits() {
    for count in [2, 127] {
        let source = [
            0, 0xb0, 99, 20, 0, 98, count, 0, 0xc0, 1, 0, 0x90, 60, 100, 12, 0x90, 60, 0, 0, 0xb0,
            99, 30, 7, 0xff, 0x2f, 0,
        ];
        let original = entry(&[&source]);
        let translated = translate(&original, 0);
        let mut edited = translated.midi.clone();
        let visits: Vec<_> = translated
            .report
            .mappings
            .iter()
            .filter(|m| {
                matches!(
                    edited.tracks[m.track].events[m.event].message,
                    Message::Channel { status: 0xc0, .. }
                )
            })
            .collect();
        assert_eq!(visits.len(), 2);
        let first = visits[0];
        edited.tracks[first.track].events[first.event].message = Message::Channel {
            status: 0xc0,
            data: vec![3],
        };
        let error = editing::rebuild_entry(&original, &[(0, &edited)], &Options::default())
            .err()
            .unwrap()
            .to_string();
        assert!(error.contains("disagrees with visit"), "{error}");
        let second = visits[1];
        edited.tracks[second.track].events[second.event].message = Message::Channel {
            status: 0xc0,
            data: vec![3],
        };
        let rebuilt =
            editing::rebuild_entry(&original, &[(0, &edited)], &Options::default()).unwrap();
        let mut expected = original.clone();
        expected[19 + 9] = 3;
        assert_eq!(rebuilt.bytes, expected);
        assert_eq!(rebuilt.reports[0].changed_source_events, 1);
        assert_eq!(rebuilt.reports[0].changed_source_bytes, 1);
    }
}

#[test]
fn unsupported_edits_are_diagnostic_and_never_silently_discarded() {
    let original = entry(&[SOURCE]);
    let translated = translate(&original, 0);
    let reject = |midi: Midi, reason: &str| {
        let error = editing::rebuild_entry(&original, &[(0, &midi)], &Options::default())
            .err()
            .unwrap()
            .to_string();
        assert!(error.contains(reason), "expected {reason}: {error}");
    };
    let mut midi = translated.midi.clone();
    midi.ppqn += 1;
    reject(midi, "PPQN");
    let mut midi = translated.midi.clone();
    midi.tracks[1].events[0].tick = 1;
    reject(midi, "tick");
    let mut midi = translated.midi.clone();
    midi.tracks[1].events.remove(0);
    reject(midi, "insertion/removal");
    let mut midi = translated.midi.clone();
    midi.tracks[1].events[0].message = Message::Channel {
        status: 0xc3,
        data: vec![1],
    };
    reject(midi, "generated");
    let mut midi = translated.midi.clone();
    midi.tracks[0].events[2].message = Message::Meta {
        kind: 0x51,
        data: vec![0, 0, 0],
    };
    reject(midi, "nonzero");
    for mapping in &translated.report.mappings {
        let before = &translated.midi.tracks[mapping.track].events[mapping.event].message;
        let (message, reason) = match before {
            Message::Channel { status: 0xe3, .. } => (
                Message::Channel {
                    status: 0xe3,
                    data: vec![1, 65],
                },
                "bend low byte",
            ),
            Message::Channel { status: 0x93, .. } => (
                Message::Channel {
                    status: 0x83,
                    data: vec![60, 1],
                },
                "release velocity",
            ),
            Message::Channel { status: 0xb3, .. } => (
                Message::Channel {
                    status: 0xb3,
                    data: vec![64, 90],
                },
                "controller identity",
            ),
            Message::Channel { status: 0xc3, .. } => (
                Message::Channel {
                    status: 0xc4,
                    data: vec![4],
                },
                "channel edit",
            ),
            Message::Meta { kind: 6, .. } => (
                Message::Meta {
                    kind: 6,
                    data: b"changed".to_vec(),
                },
                "marker edit",
            ),
            _ => continue,
        };
        let mut midi = translated.midi.clone();
        midi.tracks[mapping.track].events[mapping.event].message = message;
        reject(midi, reason);
    }
    let mut bytes = translated.midi.to_bytes().unwrap();
    bytes.extend_from_slice(b"JUNK\0\0\0\x01x");
    reject(Midi::from_bytes(&bytes).unwrap(), "chunk layout");
}

#[test]
fn alternate_midi_encoding_is_not_a_content_edit() {
    let original = entry(&[SOURCE]);
    let translated = translate(&original, 0);
    let mut bytes = translated.midi.to_bytes().unwrap();
    // Add a redundant VLQ byte to the first delta, adjusting only its chunk size.
    let length = u32::from_be_bytes(bytes[18..22].try_into().unwrap());
    bytes[18..22].copy_from_slice(&(length + 1).to_be_bytes());
    bytes.insert(22, 0x80);
    let parsed = Midi::from_bytes(&bytes).unwrap();
    assert_ne!(
        parsed.to_bytes().unwrap(),
        translated.midi.to_bytes().unwrap()
    );
    let rebuilt = editing::rebuild_entry(&original, &[(0, &parsed)], &Options::default()).unwrap();
    assert_eq!(rebuilt.bytes, original);
    assert!(rebuilt.reports[0].original_bytes_reused);
}

#[test]
#[ignore = "requires local EMI corpus via BOF3_AUDIO_CORPUS"]
fn corpus_all_sequences_reconstruct_exact_sep_entries() {
    fn visit(path: &Path, files: &mut Vec<std::path::PathBuf>) {
        for item in std::fs::read_dir(path).unwrap() {
            let item = item.unwrap();
            if item.file_type().unwrap().is_dir() {
                visit(&item.path(), files);
            } else if item
                .path()
                .extension()
                .is_some_and(|s| s.eq_ignore_ascii_case("emi"))
            {
                files.push(item.path());
            }
        }
    }
    let mut files = Vec::new();
    visit(
        Path::new(&std::env::var_os("BOF3_AUDIO_CORPUS").unwrap()),
        &mut files,
    );
    let mut count = 0;
    let mut entries = 0;
    let mut changed_entries = 0;
    let mut changed_events = 0;
    let mut changed_deltas = 0;
    let mut resized_entries = 0;
    for path in files {
        let MediaImage::Emi(image) = MediaImage::read(&path).unwrap() else {
            unreachable!()
        };
        for index in 0..image.entries().len() {
            let bytes = image.entry(index).unwrap();
            if !bytes.starts_with(b"pQES") {
                continue;
            }
            let parsed = SequenceSet::parse(bytes).unwrap();
            let midis: Vec<_> = parsed
                .sequences
                .iter()
                .map(|s| translate(bytes, s.sequence_index).midi)
                .collect();
            let selections: Vec<_> = midis.iter().enumerate().collect();
            let rebuilt = editing::rebuild_entry(bytes, &selections, &Options::default())
                .unwrap_or_else(|e| panic!("{} entry {index}: {e}", path.display()));
            assert_eq!(rebuilt.bytes, bytes, "{} entry {index}", path.display());
            assert!(rebuilt.reports.iter().all(|r| r.original_bytes_reused));
            let (deltas, resized) = timing_corpus::check(bytes, &midis);
            changed_deltas += deltas;
            resized_entries += resized;
            // Apply the same velocity edit to every visit, keeping note-off zero.
            // Reconstruction independently verifies the complete expanded result.
            let mut edits = midis.clone();
            for midi in &mut edits {
                for track in &mut midi.tracks {
                    for event in &mut track.events {
                        if let Message::Channel { status, data } = &mut event.message {
                            if *status & 0xf0 == 0x90 && data[1] > 1 {
                                data[1] -= 1;
                            }
                        }
                    }
                }
                rustysynth::MidiFile::new(&mut std::io::Cursor::new(midi.to_bytes().unwrap()))
                    .unwrap();
            }
            let selections: Vec<_> = edits.iter().enumerate().collect();
            let edited = editing::rebuild_entry(bytes, &selections, &Options::default())
                .unwrap_or_else(|e| panic!("{} entry {index} edited: {e}", path.display()));
            changed_entries += usize::from(edited.bytes != bytes);
            changed_events += edited
                .reports
                .iter()
                .map(|r| r.changed_source_events)
                .sum::<usize>();
            count += midis.len();
            entries += 1;
        }
    }
    assert_eq!(count, 476);
    assert!(changed_entries > 0);
    assert!(changed_events > 0);
    assert!(changed_deltas > 0);
    assert!(resized_entries > 0);
    eprintln!("SEP timing corpus: {count} sequences preserve requested tripled ticks; {changed_deltas} changed delta encodings across {resized_entries} resized entries; regenerated MIDI independently parses.");
    eprintln!("SEP edit corpus: {count} sequences in {entries} byte-identical complete entries; {changed_events} edited source events across {changed_entries} entries, complete retranslation agrees; edited SMFs independently parse");
}
