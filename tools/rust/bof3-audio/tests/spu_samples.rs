use bof3_audio::{
    digest::sha256_hex,
    machine::{
        spu_sample::{interpolate, pitch_step, Model, Player},
        spu_transfer::RAM_BYTES,
    },
};

fn block(ram: &mut [u8], address: usize, header: u8, flags: u8, data: u8) {
    ram[address..address + 16].fill(data);
    ram[address] = header;
    ram[address + 1] = flags;
}

#[test]
fn interpolation_matches_independently_calculated_phase_and_signed_boundary_vectors() {
    // Python integer-floor vectors, all 256 phases; coefficients cross-checked
    // between published numeric ROM data and a separate emulator's table.
    for (model, expected) in [
        (
            Model::Published,
            "d59fb9a26a8051edfd0fefa16b09fd2aac308bd0e4dfa04c63f34081c601d7c9",
        ),
        (
            Model::EmulatorReference,
            "ebc7cd36a9b2ec1be648ec49e51597c37e2c85e0472818b876cfd31a072a4fa0",
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
fn modulation_retains_signed_pitch_wrap_and_explicit_ceiling_difference() {
    for (model, ceiling) in [
        (Model::Published, 0x4000),
        (Model::EmulatorReference, 0x3fff),
    ] {
        assert_eq!(pitch_step(0, None, model), 0);
        assert_eq!(pitch_step(0xffff, None, model), ceiling);
        assert_eq!(pitch_step(0xffff, Some(-32768), model), 0);
        assert_eq!(pitch_step(0x1000, Some(-16384), model), 0x800);
        assert_eq!(pitch_step(0x1000, Some(16384), model), 0x1800);
        assert_eq!(pitch_step(0x1000, Some(32767), model), 0x1fff);
        assert_eq!(pitch_step(0x8000, Some(32767), model), 1);
        assert_eq!(pitch_step(0x8000, Some(0), model), ceiling);
        assert_eq!(pitch_step(0xffff, Some(32767), model), ceiling);
        assert_eq!(pitch_step(0x4000, None, model), ceiling);
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
            "ca1fff9af1c77290c41fc401a08a897d942a26fdbc385ce3e2ec11ce2c9f2a95",
        ),
    ] {
        let mut player = Player::new(model);
        player.key_on(0x41); // ignored low address bit
        let mut bytes = Vec::new();
        for frame in 0..84 {
            let output = player.tick(&ram, 0x1000, None).unwrap();
            bytes.extend(output.sample.to_le_bytes());
            assert_eq!(output.loop_end, frame == 55);
            assert!(!output.mute);
            assert_eq!(
                output.fetched_address,
                match frame {
                    0 | 56 => Some(0x200),
                    28 => Some(0x210),
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
    player.key_on(0xffff);
    for _ in 0..7 {
        player.tick(&ram, 0x4000, None).unwrap();
    }
    assert_eq!(player.current_address(), 0);
    player.write_repeat_address(0x21);
    for i in 0..7 {
        let frame = player.tick(&ram, 0x4000, None).unwrap();
        assert_eq!(frame.mute, i == 6);
        assert_eq!(frame.loop_end, i == 6);
    }
    assert_eq!(player.current_address(), 0x20);
    assert_eq!(
        player.tick(&ram, 0, None).unwrap().fetched_address,
        Some(0x100)
    );
    assert_eq!(player.repeat_address(), 0x20); // new block's loop-start replaces manual register
    let counter = player.counter();
    for _ in 0..100 {
        player.tick(&ram, 0, None).unwrap();
    }
    assert_eq!(player.counter(), counter);
}

#[test]
fn predictors_survive_loops_but_reset_on_key_on_and_failures_preserve_state() {
    let mut ram = vec![0; RAM_BYTES];
    block(&mut ram, 0, 0x1c, 7, 0x77);
    let mut player = Player::new(Model::Published);
    player.key_on(0);
    let first: Vec<_> = (0..28)
        .map(|_| player.tick(&ram, 0x1000, None).unwrap().sample)
        .collect();
    let second: Vec<_> = (0..28)
        .map(|_| player.tick(&ram, 0x1000, None).unwrap().sample)
        .collect();
    assert_ne!(first, second); // ADPCM loops re-decode with carried prediction
    player.key_on(0);
    let again: Vec<_> = (0..28)
        .map(|_| player.tick(&ram, 0x1000, None).unwrap().sample)
        .collect();
    assert_eq!(first, again);
    let state = (player.counter(), player.current_address(), player.history());
    ram[0] = 0x50;
    assert!(player
        .tick(&ram, 0x1000, None)
        .unwrap_err()
        .to_string()
        .contains("byte 0x00000"));
    assert_eq!(
        state,
        (player.counter(), player.current_address(), player.history())
    );
    assert!(player.tick(&ram[..RAM_BYTES - 1], 0x1000, None).is_err());
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE and BOF3_AUDIO_CORPUS; sample state check, not reference audio"]
fn original_bgm_banks_match_linear_decode_through_two_loop_traversals() {
    use bof3_audio::{
        archive::MediaImage, catalog::loader, catalog::model::AssetData, catalog::model::Catalog,
        codec::adpcm::decode_sample, codec::adpcm::Termination, machine::executable::Executable,
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
    loader::resolve(&mut catalog, &exe).unwrap();
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
            player.key_on(0x200);
            for frame in 0..expected.len() - 3 {
                let actual = player.tick(&ram, 0x1000, None).unwrap();
                assert_eq!(
                    actual.sample,
                    interpolate(
                        expected[frame..frame + 4].try_into().unwrap(),
                        0,
                        Model::Published
                    ),
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
fn zero_pitch_holds_interpolation_state_and_does_not_mean_mute() {
    let mut ram = vec![0; RAM_BYTES];
    block(&mut ram, 0x200, 0, 1, 0x11);
    for model in [Model::Published, Model::EmulatorReference] {
        let mut player = Player::new(model);
        player.key_on(0x40);
        for frame in 0..100 {
            let output = player.tick(&ram, 0, None).unwrap();
            // First decoded sample is +4096. With three cleared history
            // samples at phase zero, the -1 Gaussian coefficient gives -1.
            assert_eq!(output.sample, -1);
            assert_eq!(output.fetched_address, (frame == 0).then_some(0x200));
            assert!(!output.loop_end && !output.mute);
            assert_eq!(player.counter(), 0);
        }
        for _ in 0..8 {
            player.tick(&ram, 0x1000, None).unwrap();
        }
        let position = player.counter();
        let held = player.tick(&ram, 0, None).unwrap().sample;
        assert!(held > 4000);
        for _ in 0..100 {
            assert_eq!(player.tick(&ram, 0, None).unwrap().sample, held);
            assert_eq!(player.counter(), position);
        }
        // A new key-on clears history; changing pitch alone did not.
        player.key_on(0x40);
        assert_eq!(player.tick(&ram, 0, None).unwrap().sample, -1);
    }
}
