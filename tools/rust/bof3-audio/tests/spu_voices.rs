use bof3_audio::machine::{
    adsr::Model as EnvelopeModel, spu_sample::Model, spu_transfer::RAM_BYTES, spu_voices::Voices,
};

fn fixture() -> (Voices, Vec<u8>) {
    let mut ram = vec![0; RAM_BYTES];
    ram[..16].fill(0x11);
    ram[0] = 0;
    ram[1] = 7;
    let mut voices = Voices::new(Model::Published, EnvelopeModel::Published);
    for voice in 0..24 {
        voices.write(voice, 0, 0x3fff).unwrap();
        voices.write(voice, 2, 0x4000).unwrap(); // negative full gain
        voices.write(voice, 4, 0x1000).unwrap();
        voices.write(voice, 8, 0x7f00).unwrap(); // frozen attack for explicit ENVX tests
    }
    (voices, ram)
}

#[test]
fn all_voices_apply_envelopes_before_signed_stereo_gain_and_preserve_start_register() {
    let (mut voices, ram) = fixture();
    assert!(voices
        .tick(&ram, None)
        .unwrap()
        .iter()
        .all(|o| o.fetched_address.is_none()));
    voices.apply_keys(0x00ff_ffff, 0);
    for v in 0..24 {
        voices.write(v, 12, 0x4000).unwrap();
    }
    for expected in [-1, 303, 1738, 2038] {
        for output in voices.tick(&ram, None).unwrap() {
            assert_eq!(output.mono, expected);
            assert_eq!(
                output.stereo,
                [((i32::from(expected) * 32766) >> 15) as i16, -expected]
            );
        }
    }
    // SSA writes do not redirect an active voice; LSAX is an independent register.
    voices.write(23, 6, 0x20).unwrap();
    assert_eq!(voices.read(23, 6).unwrap(), 0x20);
    assert_eq!(voices.read(23, 14).unwrap(), 0);
    for _ in 4..28 {
        voices.tick(&ram, None).unwrap();
    }
    assert_eq!(voices.end_flags(), 0x00ff_ffff);
    let out = voices.tick(&ram, None).unwrap();
    assert_eq!(out[23].fetched_address, Some(0));
    voices.apply_keys(1 << 23, 1 << 23);
    assert_eq!(voices.end_flags(), 0x007f_ffff);
    assert_eq!(voices.read(23, 12).unwrap(), 0);
    assert_eq!(
        voices.tick(&ram, None).unwrap()[23].fetched_address,
        Some(0x100)
    );
}

#[test]
fn modulation_uses_pre_pan_signal_and_ignores_voice_zero_bit() {
    let (mut voices, ram) = fixture();
    voices.apply_keys(3, 0);
    voices.write(0, 12, 0x7fff).unwrap();
    voices.write(1, 12, 0x7fff).unwrap();
    voices.write(0, 0, 0).unwrap();
    voices.write(0, 2, 0).unwrap();
    let mut reference = voices.clone();
    reference.set_modulation(1); // unused bit zero
    let mut unmodulated = voices.clone();
    voices.set_modulation(3);
    let mut differs = false;
    for _ in 0..100 {
        let actual = voices.tick(&ram, None).unwrap();
        let expected = reference.tick(&ram, None).unwrap();
        assert_eq!(expected, unmodulated.tick(&ram, None).unwrap());
        assert_eq!(actual[0], expected[0]);
        assert_eq!(actual[0].stereo, [0, 0]);
        differs |= actual[1].mono != expected[1].mono;
    }
    assert!(differs);
}

#[test]
fn release_mute_and_invalid_registers_are_explicit() {
    let (mut voices, mut ram) = fixture();
    assert!(voices.write(24, 0, 0).is_err());
    assert!(voices.read(0, 1).is_err());
    assert!(voices.write(0, 16, 0).is_err());
    voices.apply_keys(1, 0);
    voices.write(0, 12, 0x7fff).unwrap();
    voices.apply_keys(0, 1);
    voices.tick(&ram, None).unwrap();
    assert_eq!(voices.read(0, 12).unwrap(), 0x3fff);
    voices.tick(&ram, None).unwrap();
    assert_eq!(voices.read(0, 12).unwrap(), 0);
    ram[1] = 1; // one-shot mute does not stop address progression
    voices.apply_keys(1, 0);
    voices.write(0, 12, 0x7fff).unwrap();
    for _ in 0..28 {
        voices.tick(&ram, None).unwrap();
    }
    assert_eq!(voices.read(0, 12).unwrap(), 0);
    assert_eq!(voices.end_flags(), 1);
    let next = voices.tick(&ram, None).unwrap()[0];
    assert_eq!(next.mono, 0);
    assert_eq!(next.fetched_address, Some(0));
}
