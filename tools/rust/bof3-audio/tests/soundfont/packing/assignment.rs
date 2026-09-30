use crate::fixture;
use bof3_audio::{
    bank::Bank, soundfont::bank as binding, soundfont::bank::Bound, soundfont::bank::Gain,
    soundfont::gain::Model, soundfont::packing::sample, soundfont::reader::Font,
    soundfont::Bank as FontBank, soundfont::LoopMode,
};

fn media(noise_slot: bool) -> (Vec<u8>, Vec<u8>) {
    let (mut header, looped) = fixture::fixture();
    header[0x16..0x18].copy_from_slice(&(if noise_slot { 255u16 } else { 3 }).to_le_bytes());
    header[0xc22..0xc24].copy_from_slice(&2u16.to_le_bytes());
    header[0xc26..0xc28].copy_from_slice(&4u16.to_le_bytes());
    // Raw zero with a nonzero high byte still resolves to sample two.
    header[0x836..0x838].copy_from_slice(&0xa500u16.to_le_bytes());
    let mut body = vec![0x22; 16];
    body[0] = 4;
    body[1] = 1;
    body.extend(looped);
    let mut third = vec![0x33; 32];
    third[0] = 4;
    third[1] = 0;
    third[16] = 4;
    third[17] = 1;
    body.extend(third);
    if noise_slot {
        header[0xe1e..0xe20].copy_from_slice(&2u16.to_le_bytes());
        let mut last = vec![0x11; 16];
        last[0] = 4;
        last[1] = 1;
        body.extend(last);
    }
    header.extend(b"opaque header tail");
    body.extend(b"opaque body tail");
    (header, body)
}
fn bind(header: &[u8], body: &[u8]) -> Bound {
    let metadata = Bank::parse(header).unwrap();
    let mut contexts = fixture::contexts(header);
    for c in &mut contexts {
        let p = metadata
            .programs
            .iter()
            .find(|p| p.program == c.program)
            .unwrap();
        c.gain = Gain::from_bank(&metadata, p, &p.tones[c.tone], Model::RustySynth136).unwrap();
    }
    binding::bind(
        fixture::identity(),
        header,
        body,
        &contexts,
        &fixture::options(),
    )
    .unwrap()
}
fn assign(font: &mut FontBank, instrument: usize, sample: usize, mode: LoopMode) {
    for zone in &mut font.instruments[instrument].zones {
        zone.sample = sample;
        zone.loop_mode = mode;
    }
}
fn pack(
    header: &[u8],
    body: &[u8],
    bound: &Bound,
    font: &FontBank,
) -> bof3_audio::Result<sample::PackedBank> {
    sample::pack_bank(
        &bound.report,
        header,
        body,
        &Font::from_bytes(bound.bytes.clone())?,
        &Font::from_bytes(font.encode()?.bytes)?,
        None,
    )
}

#[test]
fn changed_reference_preserves_high_byte_zero_aliases_other_layers_and_entire_pcm() {
    let (header, body) = media(false);
    let bound = bind(&header, &body);
    let same = pack(&header, &body, &bound, &bound.soundfont).unwrap();
    assert_eq!(same.header, header);
    assert_eq!(same.body, body);
    assert!(same.assignments.tones.is_empty());
    let mut font = bound.soundfont.clone();
    assign(&mut font, 0, 2, LoopMode::None);
    let result = pack(&header, &body, &bound, &font).unwrap();
    assert_eq!(result.body, body);
    for (i, (&a, &b)) in header.iter().zip(&result.header).enumerate() {
        if i != 0x836 {
            assert_eq!(a, b, "byte {i:x}");
        }
    }
    assert_eq!(result.header[0x836], 3);
    let change = &result.assignments.tones[0];
    assert_eq!(
        (
            change.original_reference as u16,
            change.output_reference as u16
        ),
        (0xa500, 0xa503)
    );
    assert_eq!((change.original_sample_id, change.output_sample_id), (2, 3));
    assert_eq!(
        (change.original_sf2_sample, change.output_sf2_sample),
        (1, 2)
    );
    assert!(result.report.samples.iter().all(|s| !s.content_changed));
    let rebuilt = bind(&result.header, &result.body);
    assert_eq!(rebuilt.bytes, font.encode().unwrap().bytes);
    rustysynth::SoundFont::new(&mut std::io::Cursor::new(&rebuilt.bytes)).unwrap();
    assert!(sample::pack(
        &bound.report,
        &body,
        &Font::from_bytes(bound.bytes.clone()).unwrap(),
        &Font::from_bytes(rebuilt.bytes).unwrap()
    )
    .is_err());
}

#[test]
fn losing_all_sample_users_preserves_looped_allocations_and_new_users_can_edit_pcm() {
    let (header, body) = media(false);
    let bound = bind(&header, &body);
    let mut font = bound.soundfont.clone();
    for tone in &bound.report.tones {
        assign(&mut font, tone.sf2_instrument, 2, LoopMode::None);
    }
    let unchanged_pcm = pack(&header, &body, &bound, &font).unwrap();
    assert_eq!(
        unchanged_pcm.body, body,
        "unreferenced looped sample must remain byte-exact"
    );
    assert_eq!(unchanged_pcm.assignments.tones.len(), 3);
    assert!(unchanged_pcm
        .report
        .samples
        .iter()
        .all(|s| !s.content_changed));
    font.samples[2].pcm[0] = -20000;
    let changed = pack(&header, &body, &bound, &font).unwrap();
    assert_eq!(&changed.body[..80], &body[..80]);
    assert_eq!(&changed.body[112..], &body[112..]);
    assert!(changed.report.samples[2].content_changed);
    assert!(changed.report.samples[2].encoding.is_some());
    assert!(changed.report.samples[2]
        .pcm24_total_loss
        .as_ref()
        .unwrap()
        .rms_error
        .is_finite());
}

#[test]
fn per_key_assignment_shared_loop_conflicts_and_unrepresentable_loops_reject() {
    let (header, body) = media(false);
    let bound = bind(&header, &body);
    let mut font = bound.soundfont.clone();
    font.instruments[0].zones[0].sample = 0;
    font.instruments[0].zones[0].loop_mode = LoopMode::None;
    assert!(pack(&header, &body, &bound, &font)
        .err()
        .unwrap()
        .to_string()
        .contains("all key zones"));
    assign(&mut font, 0, 0, LoopMode::Continuous);
    assert!(
        pack(&header, &body, &bound, &font).is_err(),
        "nonlooping target has no valid loop points"
    );
    assign(&mut font, 0, 0, LoopMode::None);
    assign(&mut font, 1, 1, LoopMode::None);
    assert!(pack(&header, &body, &bound, &font)
        .err()
        .unwrap()
        .to_string()
        .contains("conflicting loop modes"));
}

#[test]
fn generated_silence_and_noise_slot_are_not_pcm_assignments() {
    let (mut header, body) = media(true);
    let bound = bind(&header, &body);
    let mut font = bound.soundfont.clone();
    let noise = bound.report.samples[254].sf2_sample.unwrap();
    assign(&mut font, 0, noise, LoopMode::None);
    assert!(pack(&header, &body, &bound, &font)
        .err()
        .unwrap()
        .to_string()
        .contains("255 selects noise"));
    header[0x822] = 0;
    let bound = bind(&header, &body);
    let mut font = bound.soundfont.clone();
    assign(&mut font, 0, 0, LoopMode::None);
    let unmuted = pack(&header, &body, &bound, &font).unwrap();
    assert!(unmuted.assignments.tones[0].unmuted);
    assert_eq!(unmuted.body, body);
    let mut font = bound.soundfont.clone();
    assign(
        &mut font,
        1,
        bound.report.silence_sample.unwrap(),
        LoopMode::Continuous,
    );
    assert!(pack(&header, &body, &bound, &font)
        .err()
        .unwrap()
        .to_string()
        .contains("generated or has no VAB allocation"));
}
