use bof3_audio::{
    archive::MediaImage,
    sequence::events::{Events, Kind},
    sequence::SequenceSet,
};
use std::{collections::BTreeMap, path::Path};

#[test]
fn framing_retains_running_status_tempo_pitch_bytes_order_and_opaque_tail() {
    let bytes = [
        0, 0xc3, 7, 0, 8, 0x81, 0, 0x93, 60, 100, 0, 60, 0, 1, 0xb3, 99, 20, 0, 0xe3, 17, 65, 0,
        0xff, 0x51, 7, 0xa1, 0x20, 0, 0x51, 6, 0x1a, 0x80, 3, 0xff, 0x2f, 0xde, 0xad,
    ];
    let parsed = Events::parse(&bytes).unwrap();
    assert_eq!(parsed.events.len(), 9);
    assert_eq!(parsed.events[1].status, 0xc3);
    assert!(!parsed.events[1].explicit_status);
    assert_eq!(parsed.events[2].delta, 128);
    assert_eq!(parsed.events[3].tick, 128);
    assert_eq!(
        parsed.events[3].kind,
        Kind::Note {
            key: 60,
            velocity: 0
        }
    );
    assert_eq!(parsed.events[5].kind, Kind::PitchBend { low: 17, high: 65 });
    assert_eq!(
        parsed.events[6].kind,
        Kind::Tempo {
            microseconds_per_quarter: 500_000
        }
    );
    assert_eq!(
        parsed.events[7].kind,
        Kind::Tempo {
            microseconds_per_quarter: 400_000
        }
    );
    assert_eq!(parsed.events[8].tick, 132);
    assert_eq!(&bytes[parsed.trailing_offset..], &[0xde, 0xad]);
    assert_eq!(parsed.trailing_bytes, 2);
    let mut reconstructed = Vec::new();
    for event in &parsed.events {
        assert_eq!(reconstructed.len(), event.offset);
        reconstructed.extend_from_slice(&bytes[event.offset..event.offset + event.encoded_bytes]);
    }
    reconstructed.extend_from_slice(&bytes[parsed.trailing_offset..]);
    assert_eq!(reconstructed, bytes);
}

#[test]
fn malformed_or_unsupported_events_are_never_silently_truncated() {
    for bytes in [
        vec![],
        vec![0],
        vec![0, 60, 100],
        vec![0, 0x90, 60],
        vec![0, 0x90, 60, 128],
        vec![0, 0x80, 60, 0],
        vec![0, 0xa0, 60, 1],
        vec![0, 0xd0, 1],
        vec![0, 0xf0, 0],
        vec![0, 0xff, 0x58, 4, 2],
        vec![0, 0xff, 0x51, 0, 0, 0],
        vec![0x80; 5],
        vec![0, 0xc0, 1],
    ] {
        assert!(Events::parse(&bytes).is_err(), "accepted {bytes:02x?}");
    }
    for prefix in 0..6 {
        assert!(Events::parse(&[0, 0xff, 0x51, 7, 0xa1, 0x20][..prefix]).is_err());
    }
    // Non-canonical VLQ encodings remain representable without normalizing bytes.
    let events = Events::parse(&[0x80, 0, 0xff, 0x2f]).unwrap();
    assert_eq!(events.events[0].delta, 0);
    assert_eq!(events.events[0].encoded_bytes, 4);
}

#[test]
#[ignore = "requires original EMI corpus in BOF3_AUDIO_CORPUS"]
fn every_original_sequence_matches_independent_raw_toc_event_survey() {
    fn visit(path: &Path, files: &mut Vec<std::path::PathBuf>) {
        for entry in std::fs::read_dir(path).unwrap() {
            let entry = entry.unwrap();
            if entry.file_type().unwrap().is_dir() {
                visit(&entry.path(), files);
            } else if entry
                .path()
                .extension()
                .is_some_and(|s| s.eq_ignore_ascii_case("emi"))
            {
                files.push(entry.path());
            }
        }
    }
    let mut files = Vec::new();
    visit(
        Path::new(&std::env::var_os("BOF3_AUDIO_CORPUS").unwrap()),
        &mut files,
    );
    let mut counts = BTreeMap::new();
    let mut controllers = BTreeMap::new();
    let mut sequences = 0;
    let mut loop_controls = BTreeMap::new();
    for path in files {
        let MediaImage::Emi(image) = MediaImage::read(&path).unwrap() else {
            unreachable!()
        };
        for index in 0..image.entries().len() {
            let bytes = image.entry(index).unwrap();
            if !bytes.starts_with(b"pQES") {
                continue;
            }
            for seq in SequenceSet::parse(bytes).unwrap().sequences {
                let bytes = &bytes[seq.data_offset..seq.data_offset + seq.data_bytes];
                let parsed = Events::parse(bytes).unwrap_or_else(|e| {
                    panic!(
                        "{} entry {index} sequence {}: {e}",
                        path.display(),
                        seq.sequence_index
                    )
                });
                assert_eq!(&bytes[parsed.trailing_offset..], &[0]);
                assert_eq!(parsed.events.last().unwrap().kind, Kind::End);
                let mut loops = bof3_audio::sequence::loops::LoopState::new(0);
                // Inspect physical order only. Following jumps or reaching EOT
                // during playback is a separate traversal/scheduler obligation.
                for (position, event) in parsed.events.iter().enumerate() {
                    *counts.entry(event.status >> 4).or_insert(0u64) += 1;
                    if let Kind::Controller { controller, value } = event.kind {
                        *controllers.entry(controller).or_insert(0u64) += 1;
                        if matches!(controller, 6 | 98 | 99) {
                            let delta = parsed.events.get(position + 1).map_or(0, |e| e.delta);
                            loops.apply(controller, value, position + 1, delta).unwrap();
                            *loop_controls.entry((controller, value)).or_insert(0u64) += 1;
                        }
                    }
                }
                sequences += 1;
            }
        }
    }
    assert_eq!(sequences, 476);
    assert_eq!(
        counts,
        BTreeMap::from([(9, 462686), (11, 7149), (12, 1902), (14, 36258), (15, 476)])
    );
    assert_eq!(
        controllers,
        BTreeMap::from([(6, 282), (7, 1905), (10, 3842), (98, 186), (99, 934)])
    );
    assert_eq!(
        loop_controls,
        BTreeMap::from([
            ((6, 127), 282),
            ((98, 127), 186),
            ((99, 20), 468),
            ((99, 30), 466)
        ])
    );
}
