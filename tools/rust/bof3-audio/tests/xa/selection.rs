use bof3_audio::{archive::XaImage, xa::Arithmetic, xa::Decoder, xa::Histories, xa::Stream};

fn sector(parameter: u8, value: u8) -> Vec<u8> {
    let mut bytes = vec![0; 2336];
    bytes[..8].copy_from_slice(&[1, 0, 0x64, 0, 1, 0, 0x64, 0]);
    for group in 0..18 {
        let at = 8 + group * 128;
        bytes[at..at + 16].fill(parameter);
        bytes[at + 16..at + 128].fill(value);
    }
    bytes
}
fn stream() -> Stream {
    Stream {
        file: 1,
        channel: 0,
        coding: 0,
    }
}
fn pcm(image: &XaImage, indices: &[usize]) -> Vec<i16> {
    let mut decoder =
        Decoder::new(stream(), Arithmetic::CombinedRounded, Histories::default()).unwrap();
    indices
        .iter()
        .flat_map(|&i| decoder.decode_sector(image.sector(i).unwrap()).unwrap())
        .collect()
}

#[test]
fn partial_edits_preserve_matching_entry_and_exit_history_and_unselected_sectors() {
    let image =
        XaImage::from_bytes([sector(12, 0), sector(12, 0), sector(0x1c, 0x33)].concat()).unwrap();
    let mut edit = pcm(&image, &[1]);
    for x in &mut edit[..100] {
        *x = 1500;
    }
    let result = bof3_audio::xa::selection::encode(
        &image,
        stream(),
        &[1],
        &edit,
        Arithmetic::CombinedRounded,
        Histories::default(),
    )
    .unwrap();
    assert_eq!(result.report.reencoded_sectors, 1);
    assert_eq!(result.patches.len(), 1);
    assert_eq!(result.patches[0].0, 1);
    let mut decoder =
        Decoder::new(stream(), Arithmetic::CombinedRounded, Histories::default()).unwrap();
    decoder.decode_sector(&result.patches[0].1).unwrap();
    assert_eq!(decoder.histories(), Histories::default());
}

#[test]
fn exit_and_entry_history_failures_are_explicit_but_unchanged_cues_reuse_bytes() {
    let image =
        XaImage::from_bytes([sector(12, 0x11), sector(12, 0), sector(12, 0)].concat()).unwrap();
    let original = pcm(&image, &[1]);
    let unchanged = bof3_audio::xa::selection::encode(
        &image,
        stream(),
        &[1],
        &original,
        Arithmetic::CombinedRounded,
        Histories::default(),
    )
    .unwrap();
    assert_eq!(unchanged.patches[0].1, image.sector(1).unwrap());
    let mut edited = original;
    edited[0] = 3000;
    let error = bof3_audio::xa::selection::encode(
        &image,
        stream(),
        &[1],
        &edited,
        Arithmetic::CombinedRounded,
        Histories::default(),
    )
    .err()
    .unwrap()
    .to_string();
    assert!(error.contains("entry history"), "{error}");
    let mut edited = pcm(&image, &[0]);
    *edited.last_mut().unwrap() = 5000;
    let error = bof3_audio::xa::selection::encode(
        &image,
        stream(),
        &[0],
        &edited,
        Arithmetic::CombinedRounded,
        Histories::default(),
    )
    .err()
    .unwrap()
    .to_string();
    assert!(error.contains("exit history"), "{error}");
    for indices in [vec![1, 0], vec![0, 2], vec![0, 0]] {
        assert!(bof3_audio::xa::selection::encode(
            &image,
            stream(),
            &indices,
            &[],
            Arithmetic::CombinedRounded,
            Histories::default()
        )
        .is_err());
    }
}
