use bof3_audio::{
    archive::XaImage, xa::encoder::Encoder, xa::reconstruction::replace_stream, xa::Arithmetic,
    xa::Decoder, xa::Histories, xa::Stream,
};

fn sector(channel: u8, parameter: u8, value: u8) -> Vec<u8> {
    let mut bytes = vec![0; 2336];
    bytes[..8].copy_from_slice(&[2, channel, 0x64, 0, 2, channel, 0x64, 0]);
    for group in 0..18 {
        let at = 8 + group * 128;
        bytes[at..at + 16].fill(parameter);
        bytes[at + 16..at + 128].fill(value);
    }
    bytes[2312..2332].fill(0x5a);
    bytes
}

#[test]
fn multiplexed_stream_rebuild_reuses_unchanged_bytes_and_preserves_every_other_sector() {
    let stream = Stream {
        file: 2,
        channel: 3,
        coding: 0,
    };
    let mut nonaudio = vec![0xa5; 2336];
    nonaudio[..8].copy_from_slice(&[1, 1, 0x28, 0, 1, 1, 0x28, 0]);
    let source = [
        sector(3, 0x1c, 0x11),
        nonaudio,
        sector(7, 0x0c, 0x22),
        sector(3, 0x1c, 0x33),
    ]
    .concat();
    let image = XaImage::from_bytes(source.clone()).unwrap();
    let mut decoder =
        Decoder::new(stream, Arithmetic::CombinedRounded, Histories::default()).unwrap();
    let mut pcm = decoder.decode_sector(image.sector(0).unwrap()).unwrap();
    pcm.extend(decoder.decode_sector(image.sector(3).unwrap()).unwrap());
    let unchanged = replace_stream(
        &image,
        stream,
        &pcm,
        Arithmetic::CombinedRounded,
        Histories::default(),
    )
    .unwrap();
    assert!(unchanged.image.bytes() == source);
    assert_eq!(unchanged.report.reused_sectors, 2);
    assert_eq!(unchanged.report.reencoded_sectors, 0);
    for sample in &mut pcm[..4032] {
        *sample = sample.saturating_add(4096);
    }
    let edited = replace_stream(
        &image,
        stream,
        &pcm,
        Arithmetic::CombinedRounded,
        Histories::default(),
    )
    .unwrap();
    assert_eq!(edited.report.sector_indices, [0, 3]);
    assert_eq!(edited.report.unrelated_sectors, 2);
    assert_ne!(edited.report.source_sha256, edited.report.output_sha256);
    assert_ne!(edited.image.sector(0).unwrap(), image.sector(0).unwrap());
    assert_ne!(
        edited.image.sector(3).unwrap(),
        image.sector(3).unwrap(),
        "following filtered audio must account for changed history"
    );
    for index in [1, 2] {
        assert_eq!(
            edited.image.sector(index).unwrap(),
            image.sector(index).unwrap()
        );
    }
    assert!(image.bytes() == source, "source mutation");
    for report in &edited.report.sectors {
        assert_eq!(report.channel_loss[0].frames, 4032);
    }
}

#[test]
fn stream_capacity_selection_and_late_sector_failure_do_not_change_original() {
    let stream = Stream {
        file: 2,
        channel: 3,
        coding: 0,
    };
    let source = [sector(3, 0x0c, 0x11), sector(3, 0x1c, 0x22)].concat();
    let image = XaImage::from_bytes(source.clone()).unwrap();
    for length in [0, 4032, 8063, 8065] {
        assert!(replace_stream(
            &image,
            stream,
            &vec![0; length],
            Arithmetic::SplitFloor,
            Histories::default()
        )
        .is_err());
    }
    assert!(replace_stream(
        &image,
        Stream {
            channel: 9,
            ..stream
        },
        &[0; 8064],
        Arithmetic::SplitFloor,
        Histories::default()
    )
    .is_err());
    let mut corrupt = source.clone();
    corrupt[2336 + 8 + 17 * 128..2336 + 24 + 17 * 128].fill(0x40);
    let bad = XaImage::from_bytes(corrupt.clone()).unwrap();
    let error = replace_stream(
        &bad,
        stream,
        &[0; 8064],
        Arithmetic::SplitFloor,
        Histories::default(),
    )
    .unwrap_err()
    .to_string();
    assert!(error.contains("sector 1"));
    assert!(bad.bytes() == corrupt);
    assert!(image.bytes() == source);
}

#[test]
#[ignore = "requires original BOF3_AUDIO_TRACK; complete source streams and bounded edited excerpts"]
fn complete_corpus_streams_reuse_original_files_and_edited_excerpts_keep_capacity() {
    use bof3_audio::{
        archive::disc::DiscImage,
        codec::edc::{check_form2, Status},
    };
    use std::collections::BTreeMap;
    let mut disc = DiscImage::open(std::path::Path::new(
        &std::env::var_os("BOF3_AUDIO_TRACK").unwrap(),
    ))
    .unwrap();
    let (mut streams, mut sectors, mut frames, mut edited_sectors, mut absent_edc) =
        (0, 0, 0, 0, 0);
    let (mut edited_samples, mut peak, mut squared_error) = (0, 0, 0f64);
    let mut one_sector_streams = 0;
    for (name, truncated_sectors) in [
        ("BIN/BMAG_XA/MAGIC00.STR", 13509),
        ("BIN/SCE_XA/S_XA00.STR", 35434),
        ("BIN/SCE_XA/VOICE.STR", 3101),
        ("LOGO/CAPCOM30.STR", 1013),
    ] {
        let image = disc.read_xa(name).unwrap();
        let mut selections = BTreeMap::<Stream, Vec<usize>>::new();
        for s in image.sectors().iter().filter(|s| s.is_audio()) {
            selections
                .entry(Stream {
                    file: s.file,
                    channel: s.channel,
                    coding: s.coding,
                })
                .or_default()
                .push(s.index);
        }
        for (stream, indices) in selections {
            one_sector_streams += usize::from(indices.len() == 1);
            let mut decoder =
                Decoder::new(stream, Arithmetic::CombinedRounded, Histories::default()).unwrap();
            let mut encoder =
                Encoder::new(stream, Arithmetic::CombinedRounded, Histories::default()).unwrap();
            let mut pcm = Vec::new();
            for (ordinal, &index) in indices.iter().enumerate() {
                let sector = image.sector(index).unwrap();
                absent_edc += usize::from(check_form2(sector).unwrap() == Status::Absent);
                let decoded = decoder.decode_sector(sector).unwrap();
                if ordinal < 2 {
                    let mut edit = decoded.clone();
                    for value in edit.iter_mut().step_by(17) {
                        *value = value.saturating_add(257);
                    }
                    let output = encoder.encode_sector(sector, &edit).unwrap();
                    assert_eq!(output.bytes.len(), sector.len());
                    assert_eq!(&output.bytes[..8], &sector[..8]);
                    assert_eq!(&output.bytes[2312..2332], &sector[2312..2332]);
                    for loss in output.report.channel_loss {
                        edited_samples += loss.frames;
                        peak = peak.max(loss.peak_absolute_error);
                        squared_error += loss.rms_error.powi(2) * loss.frames as f64;
                    }
                    edited_sectors += 1;
                }
                pcm.extend(decoded);
            }
            let rebuilt = replace_stream(
                &image,
                stream,
                &pcm,
                Arithmetic::CombinedRounded,
                Histories::default(),
            )
            .unwrap();
            assert!(
                rebuilt.image.bytes() == image.bytes(),
                "unchanged {name} stream {stream:?}"
            );
            assert_eq!(rebuilt.report.source_completeness, "verified");
            assert_eq!(rebuilt.report.reused_sectors, indices.len());
            assert_eq!(rebuilt.report.reencoded_sectors, 0);
            frames += pcm.len() / usize::from(decoder.format().channels());
            streams += 1;
            sectors += indices.len();
        }
        let truncated =
            XaImage::from_bytes(image.bytes()[..truncated_sectors * 2336].to_vec()).unwrap();
        let error = replace_stream(
            &truncated,
            Stream {
                file: 0,
                channel: 0,
                coding: 0,
            },
            &[],
            Arithmetic::CombinedRounded,
            Histories::default(),
        )
        .unwrap_err()
        .to_string();
        assert!(error.contains("known truncated"), "{name}: {error}");
    }
    assert_eq!((streams, sectors, frames), (31, 26599, 84174048));
    assert_eq!(absent_edc, sectors);
    assert_eq!(one_sector_streams, 1); // CAPCOM30's silent mono stream has one sector.
    assert_eq!(edited_sectors, 61);
    eprintln!("XA unchanged reconstruction: {streams} streams / {sectors} sectors / {frames} frames; whole files byte-equal. Edited excerpts: {edited_sectors} sectors / {edited_samples} channel samples; peak={peak}, RMS={:.6}. All source audio EDC absent.", (squared_error / edited_samples as f64).sqrt());
}
