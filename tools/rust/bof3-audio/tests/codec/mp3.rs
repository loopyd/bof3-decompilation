use bof3_audio::{codec::mp3, interchange::wave::Wave};

#[path = "mp3/stream.rs"]
mod stream;

#[cfg(target_os = "linux")]
#[path = "consumer.rs"]
mod consumer;

#[test]
fn encoding_is_deterministic_and_identifies_the_actual_encoder() {
    let wave = Wave::new(2, 44100, vec![0; 2306]).unwrap();
    let first = mp3::encode(&wave, 256).unwrap();
    let second = mp3::encode(&wave, 256).unwrap();
    assert_eq!(first.bytes, second.bytes);
    assert!(first.bytes.windows(9).any(|s| s == b"OxAV0.1.3"));
    assert!(!first.bytes.windows(4).any(|s| s == b"LAME"));
    assert_eq!(first.timing.input_frames, 1153);
}

#[test]
fn oversized_granule_budgets_are_rejected_without_changing_the_bitrate() {
    let wave = Wave::new(1, 32000, vec![22000]).unwrap();
    for (bitrate, budget) in [(256, 4508), (320, 5660)] {
        let error = mp3::encode(&wave, bitrate).unwrap_err().to_string();
        assert!(error.contains("oxideav-mp3 0.1.3"), "{error}");
        assert!(error.contains(&format!("{bitrate} kbit/s")), "{error}");
        assert!(error.contains(&format!("{budget}-bit")), "{error}");
        assert!(error.contains("4095-bit length field"), "{error}");
    }
}

#[test]
fn unsupported_rates_and_incomplete_pcm_are_rejected_before_encoding() {
    let wave = Wave::new(1, 37800, vec![1]).unwrap();
    assert!(mp3::encode(&wave, 192)
        .unwrap_err()
        .to_string()
        .contains("rate conversion"));
    let mut wave = Wave::new(1, 44100, vec![1]).unwrap();
    assert!(mp3::encode(&wave, 64).is_err());
    wave.channels = 2;
    assert!(mp3::encode(&wave, 192).is_err());
    wave.channels = 3;
    assert!(mp3::encode(&wave, 192).is_err());
    wave.channels = 1;
    wave.pcm.clear();
    assert!(mp3::encode(&wave, 192).is_err());
}

#[test]
#[cfg(target_os = "linux")]
#[ignore = "requires installed mpg123 library; independent delivery acceptance check"]
fn mpg123_preserves_duration_channel_layout_and_boundary_impulses() {
    check_boundary_impulses(&[192]);
}

#[test]
#[cfg(target_os = "linux")]
#[ignore = "requires installed mpg123; expanded codec conformance gate"]
fn mpg123_supported_bitrates_preserve_boundary_impulses() {
    check_boundary_impulses(&[128, 192, 256, 320]);
}

#[cfg(target_os = "linux")]
fn check_boundary_impulses(bitrates: &[u32]) {
    use std::{
        fs,
        time::{SystemTime, UNIX_EPOCH},
    };
    let root = std::env::temp_dir().join(format!(
        "bof3-mp3-{}-{}",
        std::process::id(),
        SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_nanos()
    ));
    fs::create_dir(&root).unwrap();
    let mut failures = Vec::new();
    let mut decoded_cases = 0;
    let mut rejected_cases = 0;
    for rate in [32000, 44100, 48000] {
        for channels in [1u16, 2] {
            for (frames, bitrate) in
                [1usize, 100, 576, 1152, 1153, 8193]
                    .into_iter()
                    .flat_map(|frames| {
                        bitrates
                            .iter()
                            .copied()
                            .map(move |bitrate| (frames, bitrate))
                    })
            {
                let count = usize::from(channels);
                let mut pcm = vec![0i16; frames * count];
                for ch in 0..count {
                    pcm[ch] = 22000 - ch as i16 * 6000;
                    pcm[(frames - 1) * count + ch] = pcm[ch];
                }
                let wave = Wave::new(channels, rate, pcm).unwrap();
                if rate == 32000 && channels == 1 && bitrate >= 256 {
                    let error = mp3::encode(&wave, bitrate).unwrap_err().to_string();
                    assert!(error.contains("4095-bit length field"), "{error}");
                    rejected_cases += 1;
                    continue;
                }
                let encoded = mp3::encode(&wave, bitrate).unwrap();
                let name = format!("{rate}-{channels}-{frames}-{bitrate}");
                let input = root.join(format!("{name}.mp3"));
                fs::write(&input, &encoded.bytes).unwrap();
                let decoded = consumer::decode(&input);
                assert_eq!(decoded.rate, rate, "{name}");
                assert_eq!(decoded.channels, channels, "{name}");
                let pcm = decoded.pcm;
                assert_eq!(pcm.len(), frames * count, "{name}");
                // A correct duration alone can conceal truncation or a shifted
                // signal. Both retained boundary impulses must also survive.
                for ch in 0..count {
                    let expected = i32::from(wave.pcm[ch]);
                    for (boundary, value) in
                        [("first", pcm[ch]), ("last", pcm[(frames - 1) * count + ch])]
                    {
                        if i32::from(value) <= expected / 2 {
                            failures.push(format!(
                                "{name} channel {ch}: {boundary} impulse {value}, expected > {}",
                                expected / 2
                            ));
                        }
                    }
                }
                decoded_cases += 1;
            }
        }
    }
    assert!(
        failures.is_empty(),
        "{decoded_cases} decoded cases, {rejected_cases} rejected configurations; fixtures in {}: {}",
        root.display(),
        failures.join("; ")
    );
    fs::remove_dir_all(root).unwrap();
}
