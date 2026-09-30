use bof3_audio::{codec::vorbis, interchange::wave::Wave};

#[test]
fn invalid_pcm_and_quality_fail_before_encoding() {
    let mut wave = Wave::new(1, 37800, vec![1000]).unwrap();
    for quality in [f32::NAN, f32::INFINITY, -0.1, 1.1] {
        assert!(vorbis::encode(&wave, quality).is_err());
    }
    wave.channels = 2;
    assert!(vorbis::encode(&wave, 0.7).is_err());
    wave.channels = 0;
    assert!(vorbis::encode(&wave, 0.7).is_err());
    wave.channels = 1;
    wave.sample_rate = 0;
    assert!(vorbis::encode(&wave, 0.7).is_err());
    wave.sample_rate = 37800;
    wave.pcm.clear();
    assert!(vorbis::encode(&wave, 0.7).is_err());
}

#[test]
fn deterministic_xa_rate_output_and_corruption_checks() {
    let wave = Wave::new(1, 37800, vec![0; 1153]).unwrap();
    let encoded = vorbis::encode(&wave, 0.7).unwrap();
    assert_eq!(encoded.bytes, vorbis::encode(&wave, 0.7).unwrap().bytes);
    assert_eq!(encoded.stream.sample_rate, 37800);
    assert_eq!(encoded.stream.channels, 1);
    assert_eq!(encoded.stream.frames, 1153);
    assert_eq!(vorbis::inspect(&encoded.bytes).unwrap(), encoded.stream);
    for length in [0, 3, 26, encoded.bytes.len() - 1] {
        assert!(vorbis::inspect(&encoded.bytes[..length]).is_err());
    }
    for position in [0, 5, 14, 18, 22, encoded.bytes.len() - 1] {
        let mut damaged = encoded.bytes.clone();
        damaged[position] ^= 1;
        assert!(
            vorbis::inspect(&damaged).is_err(),
            "corruption at {position}"
        );
    }
    let mut trailing = encoded.bytes.clone();
    trailing.push(0);
    assert!(vorbis::inspect(&trailing).is_err());
    trailing = encoded.bytes.clone();
    trailing.extend(&encoded.bytes);
    assert!(vorbis::inspect(&trailing).is_err());
}

#[test]
#[ignore = "requires installed Xiph oggdec; independent Vorbis timing/PCM check"]
fn xiph_preserves_rates_channels_lengths_and_boundary_impulses() {
    use std::{
        fs,
        process::Command,
        time::{SystemTime, UNIX_EPOCH},
    };
    let root = std::env::temp_dir().join(format!(
        "bof3-vorbis-{}-{}",
        std::process::id(),
        SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_nanos()
    ));
    fs::create_dir(&root).unwrap();
    for rate in [18900, 32000, 37800, 44100, 48000] {
        for channels in [1u16, 2] {
            for frames in [1usize, 100, 576, 1152, 1153, 8193] {
                let count = usize::from(channels);
                let mut pcm = vec![0i16; frames * count];
                for ch in 0..count {
                    pcm[ch] = 22000 - ch as i16 * 6000;
                    pcm[(frames - 1) * count + ch] = pcm[ch];
                }
                let wave = Wave::new(channels, rate, pcm).unwrap();
                let encoded = vorbis::encode(&wave, 0.7).unwrap();
                let name = format!("{rate}-{channels}-{frames}");
                let input = root.join(format!("{name}.ogg"));
                let output = root.join(format!("{name}.wav"));
                fs::write(&input, encoded.bytes).unwrap();
                let result = Command::new("oggdec")
                    .args(["-Q", "-b", "16", "-o"])
                    .arg(&output)
                    .arg(&input)
                    .output()
                    .unwrap();
                assert!(
                    result.status.success(),
                    "{name}: {}",
                    String::from_utf8_lossy(&result.stderr)
                );
                let decoded = Wave::from_bytes(&fs::read(output).unwrap()).unwrap();
                assert_eq!(
                    (decoded.sample_rate, decoded.channels, decoded.frames()),
                    (rate, channels, frames),
                    "{name}"
                );
                for ch in 0..count {
                    assert!(
                        i32::from(decoded.pcm[ch]) > i32::from(wave.pcm[ch]) / 2,
                        "missing first impulse {name}/{ch}"
                    );
                    assert!(
                        i32::from(decoded.pcm[(frames - 1) * count + ch])
                            > i32::from(wave.pcm[ch]) / 2,
                        "missing last impulse {name}/{ch}"
                    );
                }
            }
        }
    }
    fs::remove_dir_all(root).unwrap();
}
