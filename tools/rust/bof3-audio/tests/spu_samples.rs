use bof3_audio::{
    digest::sha256_hex,
    machine::{
        spu::pitch::Pitch,
        spu::sample::{interpolate, Model, Player, Signal},
        spu::transfer::RAM_BYTES,
    },
};

const SIGNAL: Signal = Signal::Ordinary { unmuted: true };

fn block(ram: &mut [u8], address: usize, header: u8, flags: u8, data: u8) {
    ram[address..address + 16].fill(data);
    ram[address] = header;
    ram[address + 1] = flags;
}

#[test]
fn interpolation_matches_independently_calculated_phase_and_signed_boundary_vectors() {
    // Python integer-floor vectors, all 256 phases; coefficients cross-checked
    // between published numeric ROM data and a separate emulator's table.
    // Native-reference output is from the unchanged current native interpolator.
    for (model, expected) in [
        (
            Model::Published,
            "d59fb9a26a8051edfd0fefa16b09fd2aac308bd0e4dfa04c63f34081c601d7c9",
        ),
        (
            Model::EmulatorReference,
            "341d2f91a3787d49c6e67eae79eca1462fb3c5153315d544a4db5975686bed93",
        ),
    ] {
        let mut bytes = Vec::new();
        for samples in [
            [32767; 4],
            [-32768; 4],
            [-32768, 32767, -12345, 23456],
            [1; 4],
            [0, 0, 0, 32767],
        ] {
            for phase in 0..=255 {
                bytes.extend(interpolate(samples, phase, model).to_le_bytes());
            }
        }
        assert_eq!(sha256_hex(&bytes), expected);
    }
    assert_eq!(interpolate([1; 4], 0, Model::Published), -1);
    assert_eq!(interpolate([1; 4], 0, Model::EmulatorReference), 0);
}

#[test]
fn modulation_uses_selected_pitch_state_and_register_ceiling() {
    let step = |raw, source, model| {
        let mut pitch = Pitch::new(model);
        pitch.write(raw);
        pitch.tick(source).unwrap()
    };
    for (model, expected) in [
        (
            Model::Published,
            [
                0, 0x40000, 0, 0x8000, 0x18000, 0x1fff0, 16, 0x40000, 0x40000, 0x40000,
            ],
        ),
        (
            Model::EmulatorReference,
            [
                1, 0x3fff0, 14, 32768, 98304, 131042, 262114, 0x3fff0, 262114, 0x3fff0,
            ],
        ),
    ] {
        for ((raw, source), expected) in [
            (0, None),
            (0xffff, None),
            (0xffff, Some(-32768)),
            (0x1000, Some(-16384)),
            (0x1000, Some(16384)),
            (0x1000, Some(32767)),
            (0x8000, Some(32767)),
            (0x8000, Some(0)),
            (0xffff, Some(32767)),
            (0x4000, None),
        ]
        .into_iter()
        .zip(expected)
        {
            assert_eq!(step(raw, source, model), expected);
        }
    }
}

#[test]
fn block_and_loop_boundaries_keep_three_sample_interpolation_history() {
    let mut ram = vec![0; RAM_BYTES];
    block(&mut ram, 0x200, 0, 4, 0x11);
    block(&mut ram, 0x210, 0, 3, 0x22);
    for (model, expected) in [
        (
            Model::Published,
            "789497cefba5a00879fbb3d012b2c8cf355574603605769b7724b21290b7dd20",
        ),
        (
            Model::EmulatorReference,
            "c35eed908ba34160f9a48120640e5399ae0a4bb7520091f3e3d2c62d160032da",
        ),
    ] {
        let mut player = Player::new(model);
        player.write_pitch(0x1000);
        player.key_on(0x41); // both register policies ignore the low address bit
        let mut bytes = Vec::new();
        for frame in 0..84 {
            let output = player.tick(&ram, None, SIGNAL).unwrap();
            bytes.extend(i16::try_from(output.sample).unwrap().to_le_bytes());
            let primed = model == Model::EmulatorReference;
            assert_eq!(
                output.loop_end,
                if primed {
                    matches!(frame, 26 | 82)
                } else {
                    frame == 55
                }
            );
            assert!(!output.mute);
            assert_eq!(
                output.fetched_address,
                match (primed, frame) {
                    (false, 0 | 56) | (true, 0 | 54) => Some(0x200),
                    (false, 28) | (true, 26 | 82) => Some(0x210),
                    _ => None,
                }
            );
        }
        assert_eq!(sha256_hex(&bytes), expected);
        assert_eq!(player.repeat_address(), 0x40);
    }
}

#[test]
fn live_repeat_register_mute_and_ram_wrap_affect_next_block() {
    let mut ram = vec![0; RAM_BYTES];
    block(&mut ram, RAM_BYTES - 16, 12, 0, 0x11);
    block(&mut ram, 0, 12, 1, 0x22);
    block(&mut ram, 0x100, 12, 7, 0x33);
    let mut player = Player::new(Model::Published);
    player.write_pitch(0x4000);
    player.key_on(0xffff);
    for _ in 0..7 {
        player.tick(&ram, None, SIGNAL).unwrap();
    }
    assert_eq!(player.current_address(), Some(0));
    player.write_repeat_address(0x21);
    for i in 0..7 {
        let frame = player.tick(&ram, None, SIGNAL).unwrap();
        assert_eq!(frame.mute, i == 6);
        assert_eq!(frame.loop_end, i == 6);
    }
    assert_eq!(player.current_address(), Some(0x20));
    player.write_pitch(0);
    assert_eq!(
        player.tick(&ram, None, SIGNAL).unwrap().fetched_address,
        Some(0x100)
    );
    assert_eq!(player.repeat_address(), 0x20); // new block's loop-start replaces manual register
    let counter = player.counter();
    for _ in 0..100 {
        player.tick(&ram, None, SIGNAL).unwrap();
    }
    assert_eq!(player.counter(), counter);
}

#[test]
fn predictors_survive_loops_but_reset_on_key_on_and_failures_preserve_state() {
    let mut ram = vec![0; RAM_BYTES];
    block(&mut ram, 0, 0x1c, 7, 0x77);
    let mut player = Player::new(Model::Published);
    player.write_pitch(0x1000);
    player.key_on(0);
    let first: Vec<_> = (0..28)
        .map(|_| player.tick(&ram, None, SIGNAL).unwrap().sample)
        .collect();
    let second: Vec<_> = (0..28)
        .map(|_| player.tick(&ram, None, SIGNAL).unwrap().sample)
        .collect();
    assert_ne!(first, second); // ADPCM loops re-decode with carried prediction
    player.key_on(0);
    let again: Vec<_> = (0..28)
        .map(|_| player.tick(&ram, None, SIGNAL).unwrap().sample)
        .collect();
    assert_eq!(first, again);
    let state = (player.counter(), player.current_address(), player.history());
    ram[0] = 0x50;
    assert!(player
        .tick(&ram, None, SIGNAL)
        .unwrap_err()
        .to_string()
        .contains("byte 0x00000"));
    assert_eq!(
        state,
        (player.counter(), player.current_address(), player.history())
    );
    assert!(player.tick(&ram[..RAM_BYTES - 1], None, SIGNAL).is_err());
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE and BOF3_AUDIO_CORPUS; sample state check, not reference audio"]
fn original_bgm_banks_match_linear_decode_through_two_loop_traversals() {
    use bof3_audio::{
        archive::MediaImage, catalog::loader, catalog::model::AssetData, catalog::model::Catalog,
        codec::adpcm::decode_sample, codec::adpcm::Termination,
        machine::image::executable::Executable,
    };
    use std::path::PathBuf;
    let root = PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap());
    let paths = [
        root.join("BIN/BGM/BGM000.EMI"),
        root.join("BIN/BGM/BGM004.EMI"),
    ];
    let exe =
        Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    let mut catalog = Catalog::read(None, &paths).unwrap();
    loader::resolve(
        &mut catalog,
        &bof3_audio::machine::reference::mapping::read(&exe).unwrap(),
    )
    .unwrap();
    let mut samples = 0;
    let mut loops = 0;
    let mut frames = 0;
    for asset in &catalog.assets {
        let AssetData::Bank {
            content, metadata, ..
        } = &asset.data
        else {
            continue;
        };
        let content = content.as_ref().unwrap();
        let source = catalog
            .sources
            .iter()
            .find(|s| s.source == asset.source)
            .unwrap();
        let body_index = source
            .entries
            .iter()
            .find(|e| e.id == content.body_entry)
            .unwrap()
            .entry;
        let MediaImage::Emi(image) = MediaImage::read(&PathBuf::from(&asset.source)).unwrap()
        else {
            unreachable!()
        };
        let body = image.entry(body_index).unwrap();
        for sample in &metadata.samples {
            let encoded = &body[sample.body_offset..sample.body_offset + sample.encoded_bytes];
            let decoded = decode_sample(encoded).unwrap();
            if decoded.pcm.is_empty() {
                continue;
            }
            assert!(matches!(
                decoded.termination,
                Termination::EndMute | Termination::EndRepeat
            ));
            let mut expected = vec![0; 3];
            expected.extend(&decoded.pcm);
            if let Some(loop_range) = &decoded.sample_loop {
                let mut history = decoded.final_history;
                let begin = loop_range.start_frame / 28 * 16;
                for block in encoded[begin..decoded.decoded_bytes].as_chunks::<16>().0 {
                    expected.extend(history.decode_block(block).unwrap());
                }
                loops += 1;
            }
            let mut ram = vec![0; RAM_BYTES];
            ram[0x1000..0x1000 + encoded.len()].copy_from_slice(encoded);
            let mut player = Player::new(Model::Published);
            player.write_pitch(0x1000);
            player.key_on(0x200);
            for frame in 0..expected.len() - 3 {
                let actual = player.tick(&ram, None, SIGNAL).unwrap();
                assert_eq!(
                    actual.sample,
                    i32::from(interpolate(
                        expected[frame..frame + 4].try_into().unwrap(),
                        0,
                        Model::Published
                    )),
                    "{} sample {} frame {frame}",
                    asset.id,
                    sample.sample_id
                );
                if frame == decoded.pcm.len() - 1 {
                    assert!(actual.loop_end);
                }
                frames += 1;
            }
            samples += 1;
        }
    }
    assert!(samples >= 21 && loops > 0 && frames > 100_000);
    eprintln!("{samples} original bank samples, {loops} repeated loops, {frames} voice frames");
}

#[test]
fn zero_pitch_uses_explicit_model_progression_and_does_not_mean_mute() {
    let mut ram = vec![0; RAM_BYTES];
    block(&mut ram, 0x200, 0, 1, 0x11);
    for model in [Model::Published, Model::EmulatorReference] {
        let mut player = Player::new(model);
        player.write_pitch(0);
        player.key_on(0x40);
        let native = model == Model::EmulatorReference;
        for frame in 0..100 {
            let output = player.tick(&ram, None, SIGNAL).unwrap();
            // Published starts with three cleared history samples. Native
            // primes three decoded samples and advances one 16.16 unit.
            assert_eq!(output.sample, if native { 3478 } else { -1 });
            assert_eq!(output.fetched_address, (frame == 0).then_some(0x200));
            assert_eq!(output.loop_end, native && frame == 0);
            assert!(!output.mute && !output.stopped);
            assert_eq!(
                player.counter(),
                if native { 0x20000 + frame + 1 } else { 0 }
            );
        }
        player.write_pitch(0x1000);
        for _ in 0..8 {
            player.tick(&ram, None, SIGNAL).unwrap();
        }
        let position = player.counter();
        player.write_pitch(0);
        let held = player.tick(&ram, None, SIGNAL).unwrap().sample;
        assert!(held > 4000);
        for frame in 0..100 {
            assert_eq!(player.tick(&ram, None, SIGNAL).unwrap().sample, held);
            assert_eq!(
                player.counter(),
                position + if native { frame + 2 } else { 0 }
            );
        }
        // A new key-on clears history; changing pitch alone did not.
        player.key_on(0x40);
        assert_eq!(
            player.tick(&ram, None, SIGNAL).unwrap().sample,
            if native { 3478 } else { -1 }
        );
    }
}
