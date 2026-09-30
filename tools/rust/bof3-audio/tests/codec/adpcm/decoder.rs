use bof3_audio::{
    codec::adpcm::{decode_sample, History, Termination},
    digest::sha256_hex,
};

fn block(header: u8, flags: u8, data: u8) -> [u8; 16] {
    let mut block = [data; 16];
    block[0] = header;
    block[1] = flags;
    block
}

#[test]
fn signed_nibbles_are_low_first_and_end_block_samples_are_included() {
    let mut data = block(0, 1, 0);
    for index in 0..8 {
        data[index + 2] = (((index * 2 + 1) << 4) | (index * 2)) as u8;
    }
    let decoded = decode_sample(&data).unwrap();
    assert_eq!(
        &decoded.pcm[..16],
        &[
            0, 4096, 8192, 12288, 16384, 20480, 24576, 28672, -32768, -28672, -24576, -20480,
            -16384, -12288, -8192, -4096
        ]
    );
    assert_eq!(decoded.pcm.len(), 28);
    assert_eq!(decoded.termination, Termination::EndMute);
    assert_eq!(decoded.decoded_bytes, 16);
    assert!(decoded.sample_loop.is_none());
}

#[test]
fn all_predictors_and_shifts_match_independent_integer_reference_vectors() {
    // Python floor arithmetic, independently serialized as little-endian i16.
    let hashes = [
        "8690cf988989a77924c34b74a3ab11ee2ec77704641444c15932ee1faf68726d",
        "9a09d1201e51925a79ea7e662bd4d5ca82377ac51469aa6139fc9bb02d216279",
        "07fbdbc97612232cff1a5456bfd505c9fdd754bdc758a55eeb5f242de58f0a8a",
        "8c06d7814d4e4d3061f9b895351a9863093b1cb31b3ca2e66ae37610f6816998",
        "93c4c4b47ac5ff53146006e0b337b4dedc3b8adfe941e47b6ea715aaed6427fa",
    ];
    for (filter, expected) in hashes.iter().enumerate() {
        let mut history = History {
            previous: -12345,
            previous_previous: 23456,
        };
        let mut bytes = Vec::new();
        for shift in 0..=12 {
            let mut data = block((filter as u8) << 4 | shift, 0, 0);
            for (index, value) in data[2..].iter_mut().enumerate() {
                *value = (index * 37 + usize::from(shift) * 53 + filter * 11) as u8;
            }
            for sample in history.decode_block(&data).unwrap() {
                bytes.extend(sample.to_le_bytes())
            }
        }
        assert_eq!(sha256_hex(&bytes), *expected, "predictor {filter}");
    }
}

#[test]
fn clipped_samples_feed_predictor_history_and_invalid_headers_do_not_change_it() {
    let mut history = History {
        previous: 32767,
        previous_previous: -32768,
    };
    let decoded = history.decode_block(&block(0x4c, 0, 0)).unwrap();
    assert_eq!(decoded[0], 32767);
    assert_eq!(history.previous, decoded[27]);
    assert_eq!(history.previous_previous, decoded[26]);
    for invalid in [block(0x50, 0, 0), block(0x0d, 0, 0), block(0, 0x80, 0)] {
        let before = history;
        assert!(history.decode_block(&invalid).is_err());
        assert_eq!(history, before);
    }
    for size in [1, 8, 15, 17] {
        assert!(decode_sample(&vec![0; size]).is_err())
    }
    let error = decode_sample(&[block(0, 0, 0), block(0x50, 0, 0)].concat())
        .unwrap_err()
        .to_string();
    assert!(error.contains("block 1 at byte 16"));
}

#[test]
fn loop_ranges_use_the_latest_start_and_exclusive_end_and_detect_pcm_approximation() {
    let stable = decode_sample(
        &[
            block(0x0c, 4, 0x11),
            block(0x0c, 6, 0x22),
            block(0x0c, 3, 0x33),
        ]
        .concat(),
    )
    .unwrap();
    let range = stable.sample_loop.unwrap();
    assert_eq!((range.start_frame, range.end_frame_exclusive), (28, 84));
    assert!(range.pcm_repeat_is_stable);
    assert_eq!(stable.termination, Termination::EndRepeat);
    let dynamic = decode_sample(&[block(0x1c, 6, 0x77), block(0x1c, 3, 0x11)].concat()).unwrap();
    assert!(!dynamic.sample_loop.unwrap().pcm_repeat_is_stable);
    let combined = decode_sample(&block(0x0c, 7, 0x11)).unwrap();
    assert_eq!(combined.pcm, [1; 28]); // Flag 7 is not an instruction to zero the decoded data.
    assert_eq!(combined.sample_loop.unwrap().end_frame_exclusive, 28);
    assert!(decode_sample(&block(0x0c, 3, 0))
        .unwrap_err()
        .to_string()
        .contains("repeat-address context"));
}

#[test]
fn empty_unterminated_and_opaque_post_end_data_remain_distinct() {
    let empty = decode_sample(&[]).unwrap();
    assert!(empty.pcm.is_empty());
    assert_eq!(empty.termination, Termination::Empty);
    let bounded = decode_sample(&block(0, 2, 0)).unwrap();
    assert_eq!(bounded.termination, Termination::BoundedWithoutEnd);
    assert!(bounded.sample_loop.is_none());
    let padded = decode_sample(&[block(0, 1, 0), [0xff; 16]].concat()).unwrap();
    assert_eq!(padded.pcm, [0; 28]);
    assert_eq!(padded.trailing_bytes, 16);
    assert_eq!(padded.decoded_bytes, 16);
}

#[test]
#[ignore = "requires installed FFmpeg as an independent development-only decoder"]
fn unfiltered_synthetic_vag_matches_independent_ffmpeg_consumer() {
    use std::{
        io::Write,
        process::{Command, Stdio},
    };
    let encoded = [block(0, 0, 0x78), block(3, 0, 0x12), block(12, 1, 0xef)].concat();
    let expected = decode_sample(&encoded)
        .unwrap()
        .pcm
        .into_iter()
        .flat_map(i16::to_le_bytes)
        .collect::<Vec<_>>();
    let mut vag = vec![0; 48];
    vag[..4].copy_from_slice(b"VAGp");
    vag[4..8].copy_from_slice(&0x20u32.to_be_bytes());
    vag[12..16].copy_from_slice(&(encoded.len() as u32).to_be_bytes());
    vag[16..20].copy_from_slice(&44100u32.to_be_bytes());
    vag.extend(encoded);
    let mut child = Command::new("ffmpeg")
        .args([
            "-hide_banner",
            "-loglevel",
            "error",
            "-f",
            "vag",
            "-i",
            "pipe:0",
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
    child.stdin.take().unwrap().write_all(&vag).unwrap();
    let output = child.wait_with_output().unwrap();
    assert!(
        output.status.success(),
        "{}",
        String::from_utf8_lossy(&output.stderr)
    );
    assert_eq!(output.stdout, expected);
}
