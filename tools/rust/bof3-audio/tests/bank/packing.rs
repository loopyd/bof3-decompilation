use bof3_audio::{
    bank::Sample, codec::adpcm, codec::adpcm::encoder, digest::sha256_hex, document::manifest,
    interchange::wave::SampleLoop, interchange::wave::Sampler, interchange::wave::Wave,
};

fn fixture(looping: bool) -> (Sample, Vec<u8>, manifest::Element, Wave) {
    let pcm = (0..112)
        .map(|i| ((i * 311) % 12000 - 6000) as i16)
        .collect::<Vec<_>>();
    let mut bytes = encoder::encode(
        &pcm,
        &encoder::Options {
            sample_loop: looping.then_some(28..112),
            ..Default::default()
        },
    )
    .unwrap()
    .bytes;
    bytes.extend([0xcd; 32]);
    let decoded = adpcm::decode_sample(&bytes).unwrap();
    let mut wave = Wave::new(1, 44100, decoded.pcm).unwrap();
    let mut sampler = Sampler::new(44100, 60).unwrap();
    let loops = if let Some(l) = decoded.sample_loop {
        sampler
            .loops
            .push(SampleLoop::forward(l.start_frame as u32, l.end_frame_exclusive as u32).unwrap());
        format!("<sample_loop start_frame=\"{}\" end_frame_exclusive=\"{}\" pcm_repeat_is_stable=\"{}\"/>", l.start_frame, l.end_frame_exclusive, l.pcm_repeat_is_stable)
    } else {
        String::new()
    };
    wave.sampler = Some(sampler);
    let xml = format!("<sample sample_id=\"1\" body_offset=\"16\" encoded_bytes=\"{}\" path=\"one.wav\" encoded_sha256=\"{}\" wav_sha256=\"{}\" frames=\"112\" decoded_bytes=\"64\" trailing_bytes=\"32\" termination=\"{}\">{loops}</sample>", bytes.len(), sha256_hex(&bytes), sha256_hex(&wave.to_bytes().unwrap()), if looping {"end_repeat"} else {"end_mute"});
    (
        Sample {
            sample_id: 1,
            body_offset: 16,
            encoded_bytes: bytes.len(),
        },
        bytes,
        manifest::parse(&xml, Default::default()).unwrap(),
        wave,
    )
}

#[test]
fn parsed_unchanged_audio_reuses_all_adpcm_bytes_and_noncanonical_wave_formatting() {
    for looping in [false, true] {
        let (sample, bytes, mut node, wave) = fixture(looping);
        node.attributes.insert("sample_id".into(), "0001".into());
        let mut wav = wave.to_bytes().unwrap();
        // A valid 18-byte PCM fmt chunk changes file identity, not PCM semantics.
        wav.splice(36..36, [0, 0]);
        wav[16..20].copy_from_slice(&18u32.to_le_bytes());
        let size = wav.len() as u32 - 8;
        wav[4..8].copy_from_slice(&size.to_le_bytes());
        let result = bof3_audio::bank::packing::pack(&node, &sample, &bytes, 44100, &wav).unwrap();
        assert_eq!(result.bytes, bytes);
        assert!(!result.report.changed);
        assert!(result.report.encoding.is_none());
        assert_eq!(result.report.preserved_tail_bytes, 32);
    }
}

#[test]
fn edited_pcm_and_loops_encode_with_loss_and_preserve_unconsumed_tail() {
    let (sample, bytes, node, mut wave) = fixture(true);
    wave.pcm[4] = wave.pcm[4].saturating_add(512);
    wave.sampler.as_mut().unwrap().loops[0].start_frame = 56;
    let result =
        bof3_audio::bank::packing::pack(&node, &sample, &bytes, 44100, &wave.to_bytes().unwrap())
            .unwrap();
    assert!(result.report.changed);
    assert_eq!(&result.bytes[64..], &bytes[64..]);
    let decoded = adpcm::decode_sample(&result.bytes).unwrap();
    assert_eq!(decoded.sample_loop.unwrap().start_frame, 56);
    assert!(decoded.sample_loop.unwrap().pcm_repeat_is_stable);
    assert_eq!(result.report.encoding.unwrap().loss.frames, 112);
    wave.sampler = None;
    let result =
        bof3_audio::bank::packing::pack(&node, &sample, &bytes, 44100, &wave.to_bytes().unwrap())
            .unwrap();
    assert_eq!(
        adpcm::decode_sample(&result.bytes).unwrap().termination,
        adpcm::Termination::EndMute
    );
}

#[test]
fn shortening_and_growth_keep_allocations_fixed_and_report_consumed_tail() {
    let (sample, bytes, node, mut wave) = fixture(false);
    wave.pcm.truncate(56);
    let result =
        bof3_audio::bank::packing::pack(&node, &sample, &bytes, 44100, &wave.to_bytes().unwrap())
            .unwrap();
    assert_eq!(adpcm::decode_sample(&result.bytes).unwrap().pcm.len(), 56);
    assert_eq!(&result.bytes[32..], &bytes[32..]);
    assert_eq!(result.report.preserved_tail_bytes, 64);
    wave.pcm.resize(168, 1234);
    let result =
        bof3_audio::bank::packing::pack(&node, &sample, &bytes, 44100, &wave.to_bytes().unwrap())
            .unwrap();
    assert_eq!(result.report.replaced_original_tail_bytes, 32);
    assert_eq!(result.report.preserved_tail_bytes, 0);
    wave.pcm.resize(196, 0);
    assert!(bof3_audio::bank::packing::pack(
        &node,
        &sample,
        &bytes,
        44100,
        &wave.to_bytes().unwrap()
    )
    .unwrap_err()
    .to_string()
    .contains("capacity"));
}

#[test]
fn rejects_conflicting_manifest_and_unsupported_wave_edits() {
    let (sample, bytes, node, wave) = fixture(false);
    let mut conflict = node.clone();
    conflict
        .attributes
        .insert("wav_sha256".into(), "forged".into());
    assert!(bof3_audio::bank::packing::pack(
        &conflict,
        &sample,
        &bytes,
        44100,
        &wave.to_bytes().unwrap()
    )
    .unwrap_err()
    .to_string()
    .contains("wav_sha256"));
    let mut modified = wave.clone();
    modified.sample_rate = 22050;
    assert!(bof3_audio::bank::packing::pack(
        &node,
        &sample,
        &bytes,
        44100,
        &modified.to_bytes().unwrap()
    )
    .unwrap_err()
    .to_string()
    .contains("reference rate"));
    modified = wave.clone();
    modified.sampler.as_mut().unwrap().midi_unity_note = 61;
    assert!(bof3_audio::bank::packing::pack(
        &node,
        &sample,
        &bytes,
        44100,
        &modified.to_bytes().unwrap()
    )
    .unwrap_err()
    .to_string()
    .contains("pitch"));
    modified = wave.clone();
    modified.pcm.clear();
    assert!(bof3_audio::bank::packing::pack(
        &node,
        &sample,
        &bytes,
        44100,
        &modified.to_bytes().unwrap()
    )
    .unwrap_err()
    .to_string()
    .contains("deleting"));
    modified = wave.clone();
    modified.pcm.pop();
    assert!(bof3_audio::bank::packing::pack(
        &node,
        &sample,
        &bytes,
        44100,
        &modified.to_bytes().unwrap()
    )
    .unwrap_err()
    .to_string()
    .contains("28"));
    modified = wave.clone();
    modified
        .sampler
        .as_mut()
        .unwrap()
        .loops
        .push(SampleLoop::forward(1, 112).unwrap());
    assert!(bof3_audio::bank::packing::pack(
        &node,
        &sample,
        &bytes,
        44100,
        &modified.to_bytes().unwrap()
    )
    .unwrap_err()
    .to_string()
    .contains("28-frame"));
    let mut wav = wave.to_bytes().unwrap();
    wav.extend(b"JUNK\x02\0\0\0ab");
    let len = wav.len() as u32 - 8;
    wav[4..8].copy_from_slice(&len.to_le_bytes());
    assert!(
        bof3_audio::bank::packing::pack(&node, &sample, &bytes, 44100, &wav)
            .unwrap_err()
            .to_string()
            .contains("ancillary")
    );
}
