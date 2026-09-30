use bof3_audio::interchange::wave::{SampleLoop, Sampler, Wave};

fn chunk(id: &[u8; 4], payload: &[u8], padding: u8) -> Vec<u8> {
    let mut bytes = id.to_vec();
    bytes.extend((payload.len() as u32).to_le_bytes());
    bytes.extend(payload);
    if payload.len() & 1 != 0 {
        bytes.push(padding)
    }
    bytes
}

fn riff(chunks: &[Vec<u8>]) -> Vec<u8> {
    let mut bytes = b"RIFF\0\0\0\0WAVE".to_vec();
    for chunk in chunks {
        bytes.extend(chunk)
    }
    let length = (bytes.len() - 8) as u32;
    bytes[4..8].copy_from_slice(&length.to_le_bytes());
    bytes
}

fn fmt() -> Vec<u8> {
    // Independently laid out PCM16 stereo, 22050 frames/second, 4 bytes/frame.
    vec![1, 0, 2, 0, 0x22, 0x56, 0, 0, 0x88, 0x58, 1, 0, 4, 0, 16, 0]
}

fn sampler() -> Vec<u8> {
    let mut words = vec![
        0x01000013u32,
        7,
        45351,
        60,
        0x80000000,
        25,
        0x01020304,
        1,
        3,
    ];
    // RIFF endpoint 2 is inclusive: stereo frame 2 contains PCM values 5 and 6.
    words.extend([123, 0, 1, 2, 0, 0]);
    let mut data: Vec<_> = words.into_iter().flat_map(u32::to_le_bytes).collect();
    data.extend([0x21, 0x43, 0x65]);
    data
}

fn fixture() -> Vec<u8> {
    let mut extended_fmt = fmt();
    extended_fmt.extend([0, 0]);
    riff(&[
        chunk(b"JUNK", &[9, 8, 7], 0xa5),
        chunk(b"smpl", &sampler(), 0x5a),
        chunk(b"fmt ", &extended_fmt, 0),
        chunk(b"data", &[1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0], 0),
        chunk(b"fact", &[3, 0, 0, 0, 0x77], 0x91),
        chunk(b"JUNK", &[1], 0x82),
    ])
}

#[test]
fn loop_endpoints_count_frames_instead_of_channel_samples_or_bytes() {
    let mut wave = Wave::new(2, 44100, vec![-32768, 32767, 10, -10, 20, -20]).unwrap();
    let mut sampler = Sampler::new(44100, 60).unwrap();
    assert_eq!(sampler.sample_period_ns, 22675);
    sampler.loops.push(SampleLoop::forward(1, 3).unwrap());
    wave.sampler = Some(sampler);
    let encoded = wave.to_bytes().unwrap();
    assert_eq!(&encoded[..4], b"RIFF");
    assert_eq!(
        u32::from_le_bytes(encoded[4..8].try_into().unwrap()) as usize + 8,
        encoded.len()
    );
    // fmt is 24 bytes including its header, data is 20, so smpl starts at 56.
    assert_eq!(&encoded[56..60], b"smpl");
    assert_eq!(u32::from_le_bytes(encoded[108..112].try_into().unwrap()), 1);
    assert_eq!(u32::from_le_bytes(encoded[112..116].try_into().unwrap()), 2);
    let decoded = Wave::from_bytes(&encoded).unwrap();
    assert_eq!(decoded.frames(), 3);
    assert_eq!(decoded.pcm, wave.pcm);
    assert_eq!(decoded.sampler, wave.sampler);
    assert_eq!(
        decoded
            .single_forward_loop()
            .unwrap()
            .unwrap()
            .end_frame_exclusive,
        3
    );
    let mut single = Wave::new(1, 37800, vec![99]).unwrap();
    let mut sampler = Sampler::new(37800, 0).unwrap();
    sampler.loops.push(SampleLoop::forward(0, 1).unwrap());
    single.sampler = Some(sampler);
    assert_eq!(
        Wave::from_bytes(&single.to_bytes().unwrap())
            .unwrap()
            .single_forward_loop()
            .unwrap()
            .unwrap()
            .end_frame_exclusive,
        1
    );
}

#[test]
fn untouched_wave_preserves_unknown_chunks_order_padding_extensions_and_sampler_fields() {
    let original = fixture();
    let wave = Wave::from_bytes(&original).unwrap();
    assert_eq!(wave.to_bytes().unwrap(), original);
    let chunks: Vec<_> = wave.opaque_chunks().collect();
    assert_eq!(chunks.len(), 2);
    assert_eq!(chunks[0].id, *b"JUNK");
    assert_eq!(chunks[0].data, [9, 8, 7]);
    assert_eq!(chunks[0].padding, Some(0xa5));
    let sampler = wave.sampler.as_ref().unwrap();
    assert_eq!(sampler.midi_pitch_fraction, 0x80000000);
    assert_eq!(sampler.sampler_data, [0x21, 0x43, 0x65]);
    assert_eq!(sampler.loops[0].identifier, 123);
}

#[test]
fn edits_rebuild_pcm_rates_and_fact_counts_without_discarding_opaque_metadata() {
    let mut wave = Wave::from_bytes(&fixture()).unwrap();
    wave.pcm.extend([-10, 10]);
    wave.sample_rate = 11025;
    let changed = Wave::from_bytes(&wave.to_bytes().unwrap()).unwrap();
    assert_eq!(changed.frames(), 4);
    assert_eq!(changed.sample_rate, 11025);
    assert_eq!(changed.pcm, [1, 2, 3, 4, 5, 6, -10, 10]);
    assert_eq!(
        changed.opaque_chunks().collect::<Vec<_>>(),
        wave.opaque_chunks().collect::<Vec<_>>()
    );
    assert_eq!(
        changed.sampler, wave.sampler,
        "sampler period/tuning is independent metadata, not silently reset"
    );
    wave.sampler = None;
    assert!(Wave::from_bytes(&wave.to_bytes().unwrap())
        .unwrap()
        .sampler
        .is_none());
}

#[test]
fn legal_but_unrepresentable_loop_semantics_remain_visible_and_are_rejected_for_spu() {
    let original = Wave::from_bytes(&fixture()).unwrap();
    for (loop_type, fraction, play_count) in
        [(1, 0, 0), (2, 0, 0), (32, 0, 0), (0, 1, 0), (0, 0, 3)]
    {
        let mut wave = original.clone();
        let sample_loop = &mut wave.sampler.as_mut().unwrap().loops[0];
        sample_loop.loop_type = loop_type;
        sample_loop.fraction = fraction;
        sample_loop.play_count = play_count;
        let roundtrip = Wave::from_bytes(&wave.to_bytes().unwrap()).unwrap();
        assert_eq!(roundtrip.sampler, wave.sampler);
        assert!(roundtrip.single_forward_loop().is_err());
    }
    let mut multiple = original.clone();
    multiple
        .sampler
        .as_mut()
        .unwrap()
        .loops
        .push(SampleLoop::forward(0, 1).unwrap());
    assert!(Wave::from_bytes(&multiple.to_bytes().unwrap())
        .unwrap()
        .single_forward_loop()
        .is_err());
}

#[test]
fn malformed_riff_chunks_and_pcm_formats_fail_without_panicking() {
    let original = fixture();
    for length in 0..original.len() {
        assert!(Wave::from_bytes(&original[..length]).is_err())
    }
    let mut too_long = original.clone();
    too_long.push(0);
    assert!(Wave::from_bytes(&too_long).is_err());
    let mut cases = Vec::new();
    let mut bad = fmt();
    bad[0] = 3;
    cases.push(riff(&[chunk(b"fmt ", &bad, 0), chunk(b"data", &[], 0)]));
    let mut bad = fmt();
    bad[14] = 24;
    cases.push(riff(&[chunk(b"fmt ", &bad, 0), chunk(b"data", &[], 0)]));
    let mut bad = fmt();
    bad[8] ^= 1;
    cases.push(riff(&[chunk(b"fmt ", &bad, 0), chunk(b"data", &[], 0)]));
    let mut bad = fmt();
    bad[12] = 2;
    cases.push(riff(&[chunk(b"fmt ", &bad, 0), chunk(b"data", &[], 0)]));
    let mut bad = fmt();
    bad.extend([1, 0]);
    cases.push(riff(&[chunk(b"fmt ", &bad, 0), chunk(b"data", &[], 0)]));
    cases.push(riff(&[chunk(b"fmt ", &fmt(), 0), chunk(b"data", &[0], 0)]));
    cases.push(riff(&[
        chunk(b"fmt ", &fmt(), 0),
        chunk(b"data", &[0, 0], 0),
    ]));
    cases.push(riff(&[
        chunk(b"fmt ", &fmt(), 0),
        chunk(b"fmt ", &fmt(), 0),
        chunk(b"data", &[], 0),
    ]));
    cases.push(riff(&[
        chunk(b"fmt ", &fmt(), 0),
        chunk(b"data", &[], 0),
        chunk(b"data", &[], 0),
    ]));
    cases.push(riff(&[chunk(b"fmt ", &fmt(), 0)]));
    cases.push(riff(&[chunk(b"data", &[], 0)]));
    cases.push(riff(&[
        chunk(b"fmt ", &fmt(), 0),
        chunk(b"data", &[], 0),
        chunk(b"LIST", b"wavl", 0),
    ]));
    cases.push(riff(&[
        chunk(b"fmt ", &fmt(), 0),
        chunk(b"data", &[], 0),
        chunk(b"fact", &[1, 0, 0, 0], 0),
    ]));
    cases.push(riff(&[
        chunk(b"fmt ", &fmt(), 0),
        chunk(b"data", &[], 0),
        chunk(b"fact", &[0, 0, 0], 0),
    ]));
    let mut oversized = riff(&[chunk(b"fmt ", &fmt(), 0)]);
    oversized[16..20].copy_from_slice(&u32::MAX.to_le_bytes());
    cases.push(oversized);
    for case in cases {
        assert!(Wave::from_bytes(&case).is_err())
    }
    for channels in [0, 3, u16::MAX] {
        assert!(Wave::new(channels, 44100, vec![]).is_err())
    }
    for rate in [0, u32::MAX] {
        assert!(Wave::new(2, rate, vec![]).is_err())
    }
    assert!(Wave::new(2, 44100, vec![0]).is_err());
    assert!(
        Wave::from_bytes(&Wave::new(1, 44100, vec![]).unwrap().to_bytes().unwrap())
            .unwrap()
            .pcm
            .is_empty()
    );
}

#[test]
fn sampler_lengths_and_frame_bounds_are_checked_before_allocating_or_converting() {
    let mut variants = Vec::new();
    variants.push(vec![0; 35]);
    for (offset, value) in [
        (12, 128u32),
        (28, u32::MAX),
        (32, u32::MAX),
        (44, 3),
        (48, u32::MAX),
    ] {
        let mut bad = sampler();
        bad[offset..offset + 4].copy_from_slice(&value.to_le_bytes());
        variants.push(bad);
    }
    for smpl in variants {
        let bytes = riff(&[
            chunk(b"fmt ", &fmt(), 0),
            chunk(b"data", &[0; 12], 0),
            chunk(b"smpl", &smpl, 0),
        ]);
        assert!(Wave::from_bytes(&bytes).is_err());
    }
    let duplicate = riff(&[
        chunk(b"fmt ", &fmt(), 0),
        chunk(b"data", &[0; 12], 0),
        chunk(b"smpl", &sampler(), 0),
        chunk(b"smpl", &sampler(), 0),
    ]);
    assert!(Wave::from_bytes(&duplicate).is_err());
    let mut wave = Wave::from_bytes(&fixture()).unwrap();
    wave.pcm.clear();
    assert!(wave.to_bytes().is_err());
    assert!(SampleLoop::forward(1, 1).is_err());
}

#[test]
#[ignore = "requires installed FFmpeg as an independent development-only WAV consumer"]
fn independent_consumer_reads_pcm16_wave_with_sampler_and_unknown_chunks() {
    use std::{
        io::Write,
        process::{Command, Stdio},
    };
    let bytes = fixture();
    let mut child = Command::new("ffmpeg")
        .args([
            "-hide_banner",
            "-loglevel",
            "error",
            "-f",
            "wav",
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
    child.stdin.take().unwrap().write_all(&bytes).unwrap();
    let output = child.wait_with_output().unwrap();
    assert!(
        output.status.success(),
        "{}",
        String::from_utf8_lossy(&output.stderr)
    );
    assert_eq!(output.stdout, [1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0]);
}
