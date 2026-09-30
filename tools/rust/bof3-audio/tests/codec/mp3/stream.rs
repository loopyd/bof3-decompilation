use bof3_audio::{codec::mp3, interchange::wave::Wave};
use oxideav_mp3::{
    encoder::{encode_silent_frame, make_silent_header, silent_side_info, write_side_info},
    frame::ChannelMode,
    side_info::SideInfo,
    stream_encoder::Mp3Encoder,
};

fn frame(edit: impl FnOnce(&mut SideInfo)) -> Vec<u8> {
    let header = make_silent_header(128, 44100, ChannelMode::SingleChannel).unwrap();
    let mut bytes = encode_silent_frame(&header).unwrap();
    let mut side = silent_side_info(&header);
    edit(&mut side);
    let encoded = write_side_info(&side);
    bytes[4..4 + encoded.len()].copy_from_slice(&encoded);
    bytes
}

#[test]
fn coded_symbols_and_scalefactors_cannot_cross_their_granule_budget() {
    let symbols = frame(|side| {
        let gc = &mut side.granules[0][0];
        gc.big_values = 288;
        gc.table_select = [1; 3];
        gc.part2_3_length = 1;
    });
    let error = mp3::stream::inspect(&symbols).unwrap_err().to_string();
    assert!(
        error.contains("big-value symbols exceed coded length"),
        "{error}"
    );

    let factors = frame(|side| side.granules[0][0].scalefac_compress = 15);
    let error = mp3::stream::inspect(&factors).unwrap_err().to_string();
    assert!(
        error.contains("scalefactors exceed coded length"),
        "{error}"
    );

    let lengths = frame(|side| {
        for granule in &mut side.granules {
            granule[0].part2_3_length = 4095;
        }
    });
    let error = mp3::stream::inspect(&lengths).unwrap_err().to_string();
    assert!(
        error.contains("lengths exceed available main data"),
        "{error}"
    );
}

#[test]
fn reservoir_history_must_exist_and_cannot_reuse_consumed_data() {
    let missing = frame(|side| side.main_data_begin = 1);
    let error = mp3::stream::inspect(&missing).unwrap_err().to_string();
    assert!(error.contains("unavailable history"), "{error}");

    let mut first = frame(|side| side.granules[0][0].part2_3_length = 1);
    let slot_bytes = first.len() - 4 - 17;
    first.extend(frame(|side| side.main_data_begin = slot_bytes as u16));
    let error = mp3::stream::inspect(&first).unwrap_err().to_string();
    assert!(
        error.contains("overlaps previously consumed data"),
        "{error}"
    );
}

#[test]
fn raw_and_tagged_frames_validate_but_truncation_or_format_changes_fail() {
    let bytes = frame(|_| {});
    let inspected = mp3::stream::inspect(&bytes).unwrap();
    assert_eq!(inspected.mpeg_frames, 1);
    assert_eq!(inspected.sample_rate, 44100);
    assert_eq!(inspected.channels, 1);
    assert_eq!(inspected.bitrate_kbps, 128);
    for n in [0, 1, 3, 20, bytes.len() - 1] {
        assert!(mp3::stream::inspect(&bytes[..n]).is_err());
    }
    let mut trailing = bytes.clone();
    trailing.push(0);
    assert!(mp3::stream::inspect(&trailing).is_err());
    let header = make_silent_header(192, 44100, ChannelMode::SingleChannel).unwrap();
    let mut changed = bytes;
    changed.extend(encode_silent_frame(&header).unwrap());
    assert!(mp3::stream::inspect(&changed).is_err());

    let wave = Wave::new(2, 44100, vec![0; 2306]).unwrap();
    let encoded = mp3::encode(&wave, 192).unwrap();
    let inspected = mp3::stream::inspect(&encoded.bytes).unwrap();
    assert_eq!(inspected.mpeg_frames, encoded.timing.audio_frames + 1);
}

#[test]
fn registry_overflow_reproducer_fails_payload_validation() {
    // Intentionally bypass the application's configuration guard to retain a
    // small reproduction of the unchanged published encoder's 12-bit overflow.
    let mut encoder = Mp3Encoder::new(256, 32000, ChannelMode::SingleChannel).unwrap();
    let mut pcm = vec![0; 1153];
    pcm[0] = 22000;
    encoder.push_samples(&pcm).unwrap();
    let mut bytes = Vec::new();
    encoder.finish(&mut bytes).unwrap();
    let error = mp3::stream::inspect(&bytes).unwrap_err().to_string();
    assert!(
        error.contains("big-value symbols exceed coded length"),
        "{error}"
    );
}
