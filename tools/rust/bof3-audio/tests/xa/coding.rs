use bof3_audio::{
    digest::sha256_hex,
    machine::{
        cd_audio::Model as Resampling,
        cd_queue::{Admission, Model, Queue},
    },
    xa::{Arithmetic, Histories, Stream},
};
use serde::Deserialize;

#[derive(Deserialize)]
struct Vector {
    first: u8,
    second: u8,
    admission: String,
    counts: [usize; 3],
    sha256: String,
}

fn queue(coding: u8, resampling: Resampling) -> Queue {
    Queue::new(
        Stream {
            file: 1,
            channel: 0,
            coding,
        },
        Arithmetic::SplitFloor,
        Histories::default(),
        resampling,
        Model::EmulatorReference,
    )
    .unwrap()
}

fn sector(coding: u8, seed: usize) -> Vec<u8> {
    let mut bytes = vec![0; 2336];
    bytes[..8].copy_from_slice(&[1, 0, 0x64, coding, 1, 0, 0x64, coding]);
    for (index, group) in bytes[8..2312]
        .as_chunks_mut::<128>()
        .0
        .iter_mut()
        .enumerate()
    {
        group[..16].fill(0x18);
        for (offset, byte) in group[16..].iter_mut().enumerate() {
            *byte = (index * 17 + offset * 29 + seed * 43) as u8;
        }
    }
    bytes
}

fn drain(q: &mut Queue) -> Vec<[i16; 2]> {
    let mut frames = Vec::new();
    while q.statistics().queued_frames > 0 {
        frames.push(q.preview());
        q.consume();
    }
    frames
}

#[test]
fn all_coding_transitions_match_independent_state_and_arithmetic_vectors() {
    let vectors: Vec<Vector> =
        serde_json::from_str(include_str!("../reference/xa_transitions.json")).unwrap();
    assert_eq!(vectors.len(), 8 * 8 * 3);
    for vector in vectors {
        let (counts, hash) =
            super::reference::transition(vector.first, vector.second, &vector.admission);
        assert_eq!(counts, vector.counts);
        assert_eq!(hash, vector.sha256);
        let mut q = queue(vector.first, Resampling::EmulatorReference);
        let mut output = Vec::new();
        q.sector(&sector(vector.first, 1), false).unwrap();
        assert_eq!(q.statistics().queued_frames, vector.counts[0]);
        if vector.admission != "dropped" {
            output.extend(drain(&mut q));
        }
        let second = q
            .sector(&sector(vector.second, 2), vector.admission == "muted")
            .unwrap();
        match vector.admission.as_str() {
            "queued" => assert_eq!(
                second,
                Admission::Queued {
                    frames: vector.counts[1]
                }
            ),
            "muted" => assert_eq!(second, Admission::Muted),
            "dropped" => assert_eq!(
                second,
                Admission::Dropped {
                    buffered_frames: vector.counts[0]
                }
            ),
            _ => panic!("unknown reference admission"),
        }
        output.extend(drain(&mut q));
        q.sector(&sector(vector.first, 3), false).unwrap();
        let last = drain(&mut q);
        assert_eq!(last.len(), vector.counts[2]);
        output.extend(last);
        let packed: Vec<u8> = output
            .into_iter()
            .flatten()
            .flat_map(i16::to_le_bytes)
            .collect();
        assert_eq!(
            sha256_hex(&packed),
            vector.sha256,
            "coding {} -> {} -> {}, {}",
            vector.first,
            vector.second,
            vector.first,
            vector.admission
        );
    }
}

#[test]
fn unsupported_or_malformed_transition_is_atomic_and_published_model_stays_explicit() {
    for resampling in [Resampling::EmulatorReference, Resampling::Published37800] {
        let mut q = queue(1, resampling);
        let mut reference = queue(1, resampling);
        let first = sector(1, 1);
        q.sector(&first, false).unwrap();
        reference.sector(&first, false).unwrap();
        let before = q.statistics();
        let mut malformed = sector(0x14, 2);
        malformed[8 + 17 * 128 + 4] = 0xf8;
        for bytes in [vec![], sector(0x40, 2), sector(2, 2), malformed] {
            assert!(q.sector(&bytes, false).is_err());
            assert_eq!(q.statistics(), before);
        }
        if resampling == Resampling::Published37800 {
            assert!(q
                .sector(&sector(0, 2), false)
                .unwrap_err()
                .to_string()
                .contains("emulator-reference"));
            assert_eq!(q.statistics(), before);
        }
        assert_eq!(drain(&mut q), drain(&mut reference));
        q.sector(&sector(1, 3), false).unwrap();
        reference.sector(&sector(1, 3), false).unwrap();
        assert_eq!(drain(&mut q), drain(&mut reference));
    }
}

#[test]
fn coding_changes_do_not_release_identity_but_eof_allows_both_to_change() {
    let mut q = queue(1, Resampling::EmulatorReference);
    q.sector(&sector(1, 1), false).unwrap();
    drain(&mut q);
    let mut other = sector(0x14, 2);
    other[1] = 7;
    other[5] = 7;
    assert!(q.sector(&other, false).is_err());
    let mut eof = sector(0, 2);
    eof[2] |= 0x80;
    eof[6] |= 0x80;
    q.sector(&eof, false).unwrap();
    drain(&mut q);
    assert!(matches!(
        q.sector(&other, false).unwrap(),
        Admission::Queued { .. }
    ));
    drain(&mut q);
    assert!(q.sector(&sector(0x14, 3), false).is_err());
}
