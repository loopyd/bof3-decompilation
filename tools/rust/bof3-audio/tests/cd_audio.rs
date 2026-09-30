use bof3_audio::{
    digest::sha256_hex,
    machine::cd_audio::{Model, Resampler, Volume, XaAudio},
    xa::{Arithmetic, Format, Histories, Stream},
};

#[test]
fn interpolation_models_match_independent_signed_vectors_and_chunk_boundaries() {
    let input: Vec<i16> = (0..120)
        .flat_map(|n| {
            [
                ((n * 7919 + 12345) % 65536 - 32768) as i16,
                ((n * 3571 + 54321) % 65536 - 32768) as i16,
            ]
        })
        .collect();
    for (model, coding, count, hash) in [
        (
            Model::Published37800,
            1,
            140,
            "816c6f2ca300e28765df96890b96a6f28921256e988bcd75c3b3efe23c5b224b",
        ),
        (
            Model::EmulatorReference,
            1,
            140,
            "02c0564a53770ad10903fd71db17581e259586d9b47d3256a0718d2487ad981b",
        ),
        (
            Model::EmulatorReference,
            5,
            279,
            "c6f7caec5c80a90968773f8fac24149bb039ec81746ad3d0a5ef70814ad165a4",
        ),
    ] {
        let mut full = Resampler::new(Format::from_coding(coding).unwrap(), model).unwrap();
        let mut split = full.clone();
        let frames = full.process(&input).unwrap();
        assert_eq!(frames.len(), count);
        let mut chunked = Vec::new();
        for frame in input.as_chunks::<2>().0 {
            chunked.extend(split.process(frame).unwrap());
            assert!(split.process(&[]).unwrap().is_empty());
        }
        assert_eq!(frames, chunked);
        assert_eq!(full.pending_phase(), split.pending_phase());
        let bytes: Vec<u8> = frames
            .into_iter()
            .flatten()
            .flat_map(i16::to_le_bytes)
            .collect();
        assert_eq!(sha256_hex(&bytes), hash);
        let mut reference = split.clone();
        assert!(split.process(&[123]).is_err());
        assert_eq!(
            split.process(&input).unwrap(),
            reference.process(&input).unwrap()
        );
    }
}

#[test]
fn mono_expands_after_interpolation_and_unsupported_contexts_fail_explicitly() {
    for coding in [0, 4, 16, 20] {
        let input: Vec<i16> = (0..60)
            .map(|n| if n % 2 == 0 { 32767 } else { -32768 })
            .collect();
        let stereo: Vec<i16> = input.iter().flat_map(|&v| [v, v]).collect();
        let mut mono = Resampler::new(
            Format::from_coding(coding).unwrap(),
            Model::EmulatorReference,
        )
        .unwrap();
        let mut dual = Resampler::new(
            Format::from_coding(coding | 1).unwrap(),
            Model::EmulatorReference,
        )
        .unwrap();
        let frames = mono.process(&input).unwrap();
        assert_eq!(frames, dual.process(&stereo).unwrap());
        assert!(frames.iter().all(|f| f[0] == f[1]));
    }
    assert!(Resampler::new(Format::from_coding(4).unwrap(), Model::Published37800).is_err());
    assert!(Resampler::new(Format::from_coding(0x40).unwrap(), Model::EmulatorReference).is_err());
}

fn sector(coding: u8) -> Vec<u8> {
    let mut bytes = vec![0; 2336];
    bytes[..8].copy_from_slice(&[1, 3, 0x64, coding, 1, 3, 0x64, coding]);
    for group in 0..18 {
        let at = 8 + group * 128;
        bytes[at..at + 16].fill(0);
        for byte in &mut bytes[at + 16..at + 128] {
            *byte = 0x21;
        }
    }
    bytes
}
fn audio(coding: u8) -> XaAudio {
    XaAudio::new(
        Stream {
            file: 1,
            channel: 3,
            coding,
        },
        Arithmetic::SplitFloor,
        Histories::default(),
        Model::EmulatorReference,
    )
    .unwrap()
}

#[test]
fn sectors_keep_phase_and_reject_identity_or_decode_errors_without_advancing() {
    for coding in [0, 1, 4, 5, 16, 17, 20, 21] {
        let bytes = sector(coding);
        let mut actual = audio(coding);
        let mut reference = audio(coding);
        let source_frames = Format::from_coding(coding).unwrap().frames_per_sector();
        let expected = source_frames * 7 / if coding & 4 == 0 { 6 } else { 3 };
        let first = actual.sector(&bytes).unwrap();
        assert_eq!(first.len(), expected - usize::from(coding & 4 != 0));
        assert_eq!(first, reference.sector(&bytes).unwrap());
        let mut wrong = bytes.clone();
        wrong[1] = 4;
        wrong[5] = 4;
        assert!(actual.sector(&wrong).is_err());
        let mut bad = bytes.clone();
        bad[8 + 17 * 128 + 4] = 0xff;
        assert!(actual.sector(&bad).is_err());
        let second = actual.sector(&bytes).unwrap();
        assert_eq!(second.len(), expected);
        assert_eq!(second, reference.sector(&bytes).unwrap());
        assert!(second.iter().any(|f| f[0] != 0));
    }
}

#[test]
fn drive_matrix_latches_atomically_and_preserves_stereo_swap_mono_and_mute() {
    let mut volume = Volume::new([128, 0, 128, 0]).unwrap();
    let frame = [10001, -10001];
    assert_eq!(volume.apply(frame, true), frame);
    for (bank, offset, value) in [(2, 2, 0), (2, 3, 128), (3, 1, 0), (3, 2, 128)] {
        volume.write(bank, offset, value).unwrap();
    }
    assert_eq!(volume.apply(frame, true), frame);
    volume.write(3, 3, 0x20).unwrap();
    assert_eq!(volume.apply(frame, true), [-10001, 10001]);
    for (bank, offset) in [(2, 2), (2, 3), (3, 1), (3, 2)] {
        volume.write(bank, offset, 64).unwrap();
    }
    volume.write(3, 3, 0x20).unwrap();
    assert_eq!(volume.apply(frame, true), [-1, -1]); // Each product floors separately.
    volume.write(3, 3, 1).unwrap();
    assert_eq!(volume.apply(frame, true), [0; 2]);
    assert_eq!(volume.apply(frame, false), [-1, -1]);
    let old = volume.active();
    for (bank, offset) in [(2, 2), (2, 3), (3, 1), (3, 2)] {
        volume.write(bank, offset, 255).unwrap();
    }
    assert!(volume.write(3, 3, 0x20).is_err());
    assert_eq!(volume.active(), old);
    assert!(volume.xa_muted());
    assert!(volume.write(3, 3, 0x40).is_err());
    assert!(volume.write(0, 1, 0).is_err());
    assert!(Volume::new([255; 4]).is_err());
    let double = Volume::new([128; 4]).unwrap();
    assert_eq!(double.apply([32767; 2], false), [32767; 2]);
    assert_eq!(double.apply([-32768; 2], false), [-32768; 2]);
}

#[test]
#[ignore = "requires BOF3_AUDIO_CORPUS; bounded original sectors, not cue/runtime acceptance"]
fn original_voice_stream_complete_prefix_decodes_and_resamples_selected_sectors() {
    use std::{io::Read, path::PathBuf};
    let path =
        PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap()).join("BIN/SCE_XA/VOICE.STR");
    let mut bytes = Vec::new();
    std::fs::File::open(path)
        .unwrap()
        .take(2336 * 256)
        .read_to_end(&mut bytes)
        .unwrap();
    let sectors: Vec<_> = bytes
        .as_chunks::<2336>()
        .0
        .iter()
        .filter(|s| s[..4] == s[4..8] && s[2] & 0x2e == 0x24)
        .collect();
    let first = sectors[0];
    let stream = Stream {
        file: first[0],
        channel: first[1],
        coding: first[3],
    };
    let mut audio = XaAudio::new(
        stream,
        Arithmetic::SplitFloor,
        Histories::default(),
        Model::EmulatorReference,
    )
    .unwrap();
    let mut count = 0;
    let mut frames = 0;
    let mut audible = false;
    for sector in sectors {
        if [sector[0], sector[1], sector[3]] != [stream.file, stream.channel, stream.coding] {
            continue;
        }
        let output = audio.sector(sector).unwrap();
        count += 1;
        frames += output.len();
        audible |= output.iter().any(|&f| f != [0; 2]);
    }
    assert!(count >= 2 && frames > 2000 && audible);
    eprintln!(
        "Original VOICE prefix: {count} selected sectors, {frames} resampled frames, coding {:#x}",
        stream.coding
    );
}
