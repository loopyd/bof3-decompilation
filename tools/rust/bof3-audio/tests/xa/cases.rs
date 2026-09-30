use bof3_audio::{
    codec::adpcm::History,
    xa::{Arithmetic, Decoder, Format, Histories, Stream},
};

fn sector(coding: u8) -> Vec<u8> {
    let mut bytes = vec![0; 2336];
    bytes[..8].copy_from_slice(&[1, 3, 0x64, coding, 1, 3, 0x64, coding]);
    for group in 0..18 {
        let at = 8 + group * 128;
        bytes[at..at + 16].fill(12);
        for frame in 0..28 {
            for column in 0..4 {
                bytes[at + 16 + frame * 4 + column] = if coding & 0x10 == 0 {
                    column as u8 | ((column as u8 + 8) << 4)
                } else {
                    [127, 128, 64, 192][column]
                };
            }
        }
    }
    bytes
}

fn decoder(coding: u8, arithmetic: Arithmetic) -> Decoder {
    Decoder::new(
        Stream {
            file: 1,
            channel: 3,
            coding,
        },
        arithmetic,
        Histories::default(),
    )
    .unwrap()
}

fn filtered_sector(coding: u8, index: usize) -> Vec<u8> {
    let mut bytes = sector(coding);
    for group in 0..18 {
        let at = 8 + group * 128;
        for unit in 0..8 {
            bytes[at + 4 + unit] =
                (((unit + group) % 4) << 4 | ((index + unit + group) % 13)) as u8;
        }
        let head = bytes[at + 4..at + 12].to_vec();
        bytes[at..at + 4].copy_from_slice(&head[..4]);
        bytes[at + 12..at + 16].copy_from_slice(&head[4..]);
        for offset in 0..112 {
            bytes[at + 16 + offset] = (offset * 37 + group * 13 + index * 53) as u8;
        }
    }
    bytes
}

#[test]
fn all_depths_predictors_shifts_and_arithmetic_match_independent_vectors() {
    // Frozen pre-migration hashes plus separate Rust word unpacking and floor
    // division. Rate changes must not alter encoded-rate sample values.
    let hashes = [
        [
            "4ae1e99ad31c2ff918d2cf91846184f34fe04c2fc9c703d58fe02f05379ac515",
            "ed0b53ee9dae3005ac84f189ac81795590b9c82ef23564e0a7e86bd2232f54df",
            "9194a634497fd15dbbe7fe80e604283907582b5e3342b4ed26b959fc8c86ce2b",
            "efaaa96cb3256764ae5fe003a27970dad63db3c38704f70bed87f7e216ed8755",
        ],
        [
            "78682e6d5cfa535aa99668ee00f6ad99e87cf17fc0bfcc6d04731ae589a1c74e",
            "b8ab08aaae36e9a13b5c18f6f7544027afb652181d8d5343074ca2c84d44ddcc",
            "d0999f2c6b908fd48e7dbed117e15610eaf24855c68c39b342880636f24c4e2d",
            "3121d47424843493e10fe6ea5cd7b0f298c3695d1f465308e2db40220f223de1",
        ],
    ];
    for (variant, arithmetic) in [Arithmetic::SplitFloor, Arithmetic::CombinedRounded]
        .into_iter()
        .enumerate()
    {
        for coding in [0, 1, 4, 5, 16, 17, 20, 21] {
            let mut decoder = decoder(coding, arithmetic);
            let mut bytes = Vec::new();
            for index in 0..3 {
                bytes.extend(
                    decoder
                        .decode_sector(&filtered_sector(coding, index))
                        .unwrap()
                        .into_iter()
                        .flat_map(i16::to_le_bytes),
                );
            }
            let case = usize::from(coding & 1) + if coding & 16 != 0 { 2 } else { 0 };
            assert_eq!(
                super::reference::sample_hash(coding, variant == 1),
                hashes[variant][case]
            );
            assert_eq!(
                bof3_audio::digest::sha256_hex(&bytes),
                hashes[variant][case],
                "{arithmetic:?} coding {coding:#x}"
            );
        }
    }
}

#[test]
fn four_bit_units_are_column_interleaved_and_channel_histories_are_separate() {
    for coding in [0, 1, 4, 5] {
        let mut decoder = decoder(coding, Arithmetic::CombinedRounded);
        let pcm = decoder.decode_sector(&sector(coding)).unwrap();
        let format = decoder.format();
        assert_eq!(
            format.sample_rate(),
            if coding & 4 == 0 { 37800 } else { 18900 }
        );
        assert_eq!(pcm.len(), 4032);
        assert_eq!(
            format.frames_per_sector(),
            if coding & 1 == 0 { 4032 } else { 2016 }
        );
        if coding & 1 == 0 {
            assert_eq!(&pcm[..28], &[0; 28]);
            assert_eq!(&pcm[28..56], &[-8; 28]);
            assert_eq!(&pcm[56..84], &[1; 28]);
            assert_eq!(&pcm[196..224], &[-5; 28]);
            assert_eq!(decoder.histories().right, History::default());
        } else {
            assert_eq!(&pcm[..6], &[0, -8, 0, -8, 0, -8]);
            assert_eq!(&pcm[56..62], &[1, -7, 1, -7, 1, -7]);
            assert_eq!(decoder.histories().left.previous, 3);
            assert_eq!(decoder.histories().right.previous, -5);
        }
    }
}

#[test]
fn eight_bit_signed_values_rates_and_unused_parameters_are_explicit() {
    for coding in [0x10, 0x11, 0x14, 0x15] {
        let mut bytes = sector(coding);
        for group in 0..18 {
            let at = 8 + 128 * group;
            bytes[at..at + 8].fill(8);
            bytes[at + 8..at + 16].fill(0xff); // unused in 8-bit mode
        }
        let mut decoder = decoder(coding, Arithmetic::SplitFloor);
        let pcm = decoder.decode_sector(&bytes).unwrap();
        assert_eq!(pcm.len(), 2016);
        assert_eq!(
            decoder.format().frames_per_sector(),
            if coding & 1 == 0 { 2016 } else { 1008 }
        );
        assert_eq!(
            decoder.format().sample_rate(),
            if coding & 4 == 0 { 37800 } else { 18900 }
        );
        if coding & 1 == 0 {
            assert_eq!(&pcm[..28], &[127; 28]);
            assert_eq!(&pcm[28..56], &[-128; 28]);
            assert_eq!(&pcm[56..84], &[64; 28]);
        } else {
            assert_eq!(&pcm[..6], &[127, -128, 127, -128, 127, -128]);
            assert_eq!(&pcm[56..62], &[64, -64, 64, -64, 64, -64]);
        }
    }
}

#[test]
fn arithmetic_choice_is_observable_and_history_is_clipped_carried_and_explicitly_reset() {
    let mut bytes = sector(1);
    for group in 0..18 {
        let at = 8 + 128 * group;
        bytes[at..at + 16].fill(0x1c);
        bytes[at + 16..at + 128].fill(0);
    }
    let initial = Histories {
        left: History {
            previous: 1,
            previous_previous: 0,
        },
        right: History {
            previous: -1,
            previous_previous: 0,
        },
    };
    let mut split = decoder(1, Arithmetic::SplitFloor);
    let mut rounded = decoder(1, Arithmetic::CombinedRounded);
    split.reset(initial);
    rounded.reset(initial);
    let s = split.decode_sector(&bytes).unwrap();
    let r = rounded.decode_sector(&bytes).unwrap();
    assert_eq!(&s[..4], &[0, -1, 0, -1]);
    assert_eq!(&r[..4], &[1, -1, 1, -1]);
    bytes[2] = 0xe4;
    bytes[6] = 0xe4;
    assert_eq!(rounded.decode_sector(&bytes).unwrap(), r); // EOF does not reset
    rounded.reset(Histories::default());
    assert_eq!(rounded.decode_sector(&bytes).unwrap(), vec![0; 4032]);
    for group in 0..18 {
        let at = 8 + 128 * group;
        bytes[at..at + 16].fill(0x20);
        bytes[at + 16..at + 128].fill(0x87);
    }
    rounded.reset(Histories {
        left: History {
            previous: 32767,
            previous_previous: -32768,
        },
        right: History {
            previous: -32768,
            previous_previous: 32767,
        },
    });
    let pcm = rounded.decode_sector(&bytes).unwrap();
    assert_eq!(&pcm[..2], &[32767, -32768]);
    assert_eq!(rounded.histories().left.previous, pcm[4030]);
    assert_eq!(rounded.histories().right.previous, pcm[4031]);
}

#[test]
fn malformed_or_other_stream_sectors_fail_without_mutating_history() {
    let mut decoder = decoder(0, Arithmetic::SplitFloor);
    let valid = sector(0);
    decoder.decode_sector(&valid).unwrap();
    let history = decoder.histories();
    let mut invalid = vec![valid[..2335].to_vec(), vec![0; 2337]];
    for (offset, value) in [(4, 2), (2, 0x68), (0, 2), (1, 4), (3, 1)] {
        let mut bytes = valid.clone();
        bytes[offset] = value;
        if offset < 4 {
            bytes[offset + 4] = value;
        }
        invalid.push(bytes);
    }
    for parameter in [0x0d, 0x40, 0xfc] {
        let mut bytes = valid.clone();
        bytes[8 + 17 * 128..24 + 17 * 128].fill(parameter);
        invalid.push(bytes);
    }
    let mut wrong_copy = valid.clone();
    wrong_copy[8 + 17 * 128] ^= 1;
    invalid.push(wrong_copy);
    for bytes in invalid {
        assert!(decoder.decode_sector(&bytes).is_err());
        assert_eq!(decoder.histories(), history);
    }
    for coding in [2, 8, 0x20, 0x80, 0xff] {
        assert!(Format::from_coding(coding).is_err());
    }
    assert!(Decoder::new(
        Stream {
            file: 1,
            channel: 0,
            coding: 0x40
        },
        Arithmetic::SplitFloor,
        Histories::default()
    )
    .is_err());
}

#[test]
#[ignore = "requires installed FFmpeg as an independent development-only XA decoder"]
fn filtered_four_bit_xa_matches_ffmpeg_with_combined_rounding() {
    use std::{
        io::Write,
        process::{Command, Stdio},
    };
    for coding in [0, 1, 4, 5] {
        let mut decoder = decoder(coding, Arithmetic::CombinedRounded);
        let mut expected = Vec::new();
        let mut input = Vec::new();
        for index in 0..3 {
            let bytes = filtered_sector(coding, index);
            expected.extend(
                decoder
                    .decode_sector(&bytes)
                    .unwrap()
                    .into_iter()
                    .flat_map(i16::to_le_bytes),
            );
            input.extend_from_slice(&[
                0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 2, 0, 2,
            ]);
            input.extend(bytes);
        }
        let mut child = Command::new("ffmpeg")
            .args([
                "-hide_banner",
                "-loglevel",
                "error",
                "-f",
                "psxstr",
                "-i",
                "pipe:0",
                "-map",
                "0:a:0",
                "-f",
                "s16le",
                "-acodec",
                "pcm_s16le",
                "pipe:1",
            ])
            .stdin(Stdio::piped())
            .stdout(Stdio::piped())
            .stderr(Stdio::piped())
            .spawn()
            .unwrap();
        let mut stdin = child.stdin.take().unwrap();
        let writer = std::thread::spawn(move || stdin.write_all(&input));
        let actual = child.wait_with_output().unwrap();
        writer.join().unwrap().unwrap();
        assert!(
            actual.status.success(),
            "{}",
            String::from_utf8_lossy(&actual.stderr)
        );
        assert_eq!(actual.stdout, expected, "coding {coding:#x}");
    }
}
