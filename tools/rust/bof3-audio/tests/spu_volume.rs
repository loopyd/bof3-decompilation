use bof3_audio::{
    digest::sha256_hex,
    machine::{adsr::Model, spu_volume::Volume},
};

#[test]
fn sweep_signed_directions_saturate_and_exponential_decrease_ignores_phase_for_step() {
    for (raw, initial, expected) in [
        (0x8000, 0, [14336, 28672, 32767, 32767]),
        (0x9000, 0, [-16384, -32768, -32768, -32768]),
        (0xa000, 32767, [16383, 0, 0, 0]),
        (0xb000, -32768, [-18432, -4096, 0, 0]),
        (0xe000, 32767, [16383, 8191, 4095, 2047]),
        (0xf000, -32768, [-16384, -8192, -4096, -2048]),
        (0xb000, 32767, [0; 4]), // Opposite-sign decrease snaps to zero.
        (0xa000, -32768, [0; 4]),
    ] {
        let mut volume = Volume::new(Model::Published);
        volume.write_level(initial as u16);
        volume.write(raw).unwrap();
        for expected in expected {
            volume.tick();
            assert_eq!(volume.level(), expected, "{raw:04x}");
        }
    }
}

#[test]
fn writes_preserve_current_level_restart_counter_and_require_explicit_model() {
    let mut volume = Volume::default();
    volume.write(0x6000).unwrap();
    assert_eq!(volume.level(), -16384);
    assert!(volume.write(0x8000).is_err());
    assert_eq!(volume.register(), 0x6000);
    volume.configure(Model::Published).unwrap();
    assert!(volume.configure(Model::EmulatorReference).is_err());
    volume.write(0x8030).unwrap(); // 0.5 counter increment, no initial level reset.
    volume.tick();
    assert_eq!((volume.level(), volume.counter()), (-16384, 0x4000));
    volume.write(0x87b0).unwrap(); // Reserved bits preserved, same rate, reset counter.
    assert_eq!((volume.level(), volume.counter()), (-16384, 0));
    volume.tick();
    volume.tick();
    assert_eq!(volume.level(), -16377);
    volume.write(0x8000).unwrap();
    volume.write_level(32767);
    volume.tick();
    assert!(!volume.active());
    volume.write_level(0); // Does not reactivate the stopped envelope.
    volume.tick();
    assert_eq!(volume.level(), 0);
    volume.write(0x8000).unwrap();
    volume.tick();
    assert_eq!(volume.level(), 14336);
    volume.write(0x4000).unwrap();
    assert_eq!(volume.level(), -32768);
    assert!(!volume.active());
    volume.tick();
    assert_eq!(volume.level(), -32768);
}

#[test]
fn frozen_rates_do_not_clamp_and_source_models_keep_threshold_and_slow_rate_differences() {
    for model in [Model::Published, Model::EmulatorReference] {
        for mode in 0..8 {
            let mut volume = Volume::new(model);
            volume.write_level(0x8000);
            volume.write(0x807f | (mode << 12)).unwrap();
            for _ in 0..65536 {
                volume.tick();
            }
            assert_eq!(volume.level(), -32768);
            assert!(!volume.active());
        }
    }
    let mut published = Volume::new(Model::Published);
    let mut emulator = Volume::new(Model::EmulatorReference);
    for v in [&mut published, &mut emulator] {
        v.write_level(0x6000);
        v.write(0xc028).unwrap();
        v.tick();
    }
    assert_eq!(published.level(), 24590);
    assert_eq!(emulator.level(), 24576);
    emulator.tick();
    assert_eq!(emulator.level(), 24583);
    for v in [&mut published, &mut emulator] {
        v.write_level(30000);
        v.write(0xc07c).unwrap();
        for _ in 0..32768 {
            v.tick();
        }
    }
    assert_eq!(published.level(), 30007);
    assert_eq!(emulator.level(), 30000);
}

#[test]
fn all_sweep_modes_rates_and_signed_start_levels_match_independent_integer_vectors() {
    // Separate Python oracle: integer division, signed step, phase saturation,
    // rate counter and terminal-active state. 512 pre-tick frames per context.
    for (model, expected) in [
        (
            Model::Published,
            "a2f5dc635564de06a2b8554ade8d5ce234098c58d1c70c4de181687a1d989316",
        ),
        (
            Model::EmulatorReference,
            "7388ac278502c579d5758840515eeee52682d7e05e4b63890527f5924a85635e",
        ),
    ] {
        let mut bytes = Vec::new();
        for mode in 0..8 {
            for rate in 0..128 {
                for initial in [0, 24576, -24576, 32767, -32768] {
                    let mut volume = Volume::new(model);
                    volume.write_level(initial as u16);
                    volume.write(0x8000 | (mode << 12) | rate).unwrap();
                    for _ in 0..512 {
                        bytes.extend(volume.level().to_le_bytes());
                        volume.tick();
                    }
                    bytes.extend(volume.counter().to_le_bytes());
                    bytes.push(u8::from(volume.active()));
                }
            }
        }
        assert_eq!(sha256_hex(&bytes), expected);
    }
}
