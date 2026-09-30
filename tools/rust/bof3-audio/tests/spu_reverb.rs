use bof3_audio::{
    digest::sha256_hex,
    machine::{
        spu_reverb::{Model, Reverb},
        spu_transfer::RAM_BYTES,
    },
};

const ROOM: [u16; 32] = [
    0x7d, 0x5b, 0x6d80, 0x54b8, 0xbed0, 0, 0, 0xba80, 0x5800, 0x5300, 0x4d6, 0x333, 0x3f0, 0x227,
    0x374, 0x1ef, 0x334, 0x1b5, 0, 0, 0, 0, 0, 0, 0, 0, 0x1b4, 0x136, 0xb8, 0x5c, 0x8000, 0x8000,
];

fn configured(registers: &[u16; 32], base: u16) -> Reverb {
    let mut reverb = Reverb::new(Model::EmulatorReference);
    reverb.write(0x1a2, base).unwrap();
    for (i, &value) in registers.iter().enumerate() {
        reverb.write(0x1c0 + i as u32 * 2, value).unwrap();
    }
    reverb
}
fn put(ram: &mut [u8], at: usize, value: i16) {
    ram[at..at + 2].copy_from_slice(&value.to_le_bytes());
}
fn get(ram: &[u8], at: usize) -> i16 {
    i16::from_le_bytes(ram[at..at + 2].try_into().unwrap())
}

#[test]
fn memory_network_preserves_signed_rounding_and_write_disable_does_not_mute_output() {
    let mut registers = [0; 32];
    for (i, v) in [
        (3, 0x4000),
        (8, 0x4000),
        (9, 0x4000),
        (0, 1),
        (1, 1),
        (10, 3),
        (11, 4),
        (18, 5),
        (19, 6),
        (12, 1),
        (13, 2),
        (26, 8),
        (27, 9),
        (28, 16),
        (29, 17),
    ] {
        registers[i] = v;
    }
    let mut ram = vec![0; RAM_BYTES];
    put(&mut ram, 0x60008, 1000);
    put(&mut ram, 0x60010, -1001);
    let original = ram.clone();
    let mut reverb = configured(&registers, 0xc000);
    assert_eq!(
        reverb.tick_half_rate(&mut ram, [0; 2], false).unwrap(),
        [125, -126]
    );
    assert_eq!(ram, original);
    assert_eq!(reverb.cursor_bytes(), 0x60002);
    reverb.write(0x1a2, 0xc000).unwrap();
    assert_eq!(
        reverb.tick_half_rate(&mut ram, [0; 2], true).unwrap(),
        [125, 249]
    );
    // Right delay addresses alias the just-written left APF state: 500/250.
    // Right APF1 = floor((-1001-500)/2)=-751; APF2=-1; return=249.
    for (at, expected) in [
        (0x60040, 500),
        (0x60048, -751),
        (0x60080, 250),
        (0x60088, -1),
    ] {
        assert_eq!(get(&ram, at), expected);
    }
}

#[test]
fn disabled_reverb_resamples_stored_ram_and_preserves_it_through_cursor_wrap() {
    let mut ram = vec![0; RAM_BYTES];
    for bytes in ram.as_chunks_mut::<2>().0 {
        bytes.copy_from_slice(&1000i16.to_le_bytes());
    }
    let original = ram.clone();
    let mut reverb = configured(&[0; 32], 0xffff);
    for frame in 0..100u32 {
        let sample = reverb.tick(&mut ram, [32767, -32768], false).unwrap();
        if frame > 40 {
            // Even FIR coefficients sum to 16382, not 16384.
            assert_eq!(sample, [if frame & 1 == 0 { 1000 } else { 999 }; 2]);
        }
        assert_eq!(reverb.cursor_bytes(), 0x7fff8 + (frame.div_ceil(2) % 4) * 2);
    }
    assert_eq!(ram, original);
    // ESA resets only the work cursor, not the resampling history.
    reverb.write(0x1a2, 0x8000).unwrap();
    assert_eq!(reverb.tick(&mut ram, [0; 2], false).unwrap(), [1000; 2]);
    assert!(reverb
        .tick(&mut ram[..RAM_BYTES - 1], [0; 2], true)
        .is_err());
    assert_eq!(reverb.cursor_bytes(), 0x40000);
    for offset in [0x1a0, 0x1c1, 0x200] {
        assert!(reverb.read(offset).is_err());
        assert!(reverb.write(offset, 0).is_err());
    }
}

#[test]
fn reflection_minus_one_coefficient_retains_explicit_source_anomaly() {
    let mut registers = [0; 32];
    registers[2] = 0x8000;
    registers[10] = 8;
    registers[11] = 9;
    registers[18] = 16;
    registers[19] = 17;
    let mut reverb = configured(&registers, 0x8000);
    let mut ram = vec![0; RAM_BYTES];
    put(&mut ram, 0x4003e, 1000);
    put(&mut ram, 0x40046, -32768);
    reverb.tick_half_rate(&mut ram, [0; 2], true).unwrap();
    assert_eq!(get(&ram, 0x40040), -2000);
    assert_eq!(get(&ram, 0x40048), 0);
}

#[test]
fn different_side_reflections_read_opposite_channel_delay() {
    let mut registers = [0; 32];
    for (i, v) in [
        (2, 0x4000),
        (7, 0x4000),
        (10, 8),
        (11, 9),
        (18, 3),
        (19, 4),
        (24, 2),
        (25, 1),
    ] {
        registers[i] = v;
    }
    let mut reverb = configured(&registers, 0xc000);
    let mut ram = vec![0; RAM_BYTES];
    put(&mut ram, 0x60008, 2000);
    put(&mut ram, 0x60010, -4000);
    reverb.tick_half_rate(&mut ram, [0; 2], true).unwrap();
    assert_eq!(get(&ram, 0x60018), 500);
    assert_eq!(get(&ram, 0x60020), -1000);
}

#[test]
fn reference_address_fold_preserves_alias_below_nominal_work_area() {
    let mut registers = [0; 32];
    registers[26] = 8;
    registers[27] = 9;
    registers[28] = 16;
    registers[29] = 17;
    let mut reverb = configured(&registers, 0xc000);
    let mut ram = vec![0; RAM_BYTES];
    // At cursor 0x30000 halfwords, register zero with displacement -1:
    // (0x30000+0x3ffff+0x30000)&0x3ffff = 0x1ffff.
    put(&mut ram, 0x3fffe, 1234);
    reverb.tick_half_rate(&mut ram, [0; 2], true).unwrap();
    assert_eq!(get(&ram, 0x60000), 1234);
    assert_eq!(get(&ram, 0x3fffe), 1234);
}

#[test]
fn room_impulses_and_disabled_tail_match_scalar_integer_reference() {
    let mut reverb = configured(&ROOM, ((RAM_BYTES - 0x26c0) / 8) as u16);
    let mut ram = vec![0; RAM_BYTES];
    let mut bytes = Vec::new();
    let mut energy = [0u64; 2];
    let mut disabled_ram = None;
    for frame in 0..88200 {
        if frame == 44100 {
            disabled_ram = Some(ram.clone());
        }
        let input = if frame % 1009 == 0 && frame < 44100 {
            [30000, -12345]
        } else {
            [0; 2]
        };
        for sample in reverb.tick(&mut ram, input, frame < 44100).unwrap() {
            bytes.extend(sample.to_le_bytes());
            energy[frame / 44100] += u64::from(sample.unsigned_abs());
        }
    }
    assert_eq!(ram, disabled_ram.unwrap());
    assert!(energy.iter().all(|&e| e > 1_000_000));
    assert_eq!(
        sha256_hex(&bytes),
        "e847c13ababdb98655b02a9ddb408361a517572a621bd3f029b2a80ef4eee6d0"
    );
    assert_eq!(
        sha256_hex(&ram),
        "63861ac1df8d3e27ddfc746fa1bcc5b18337702ba7b2d50481e531b84bbc46b3"
    );
}

#[test]
#[ignore = "requires exact US BOF3_AUDIO_EXE; preset identity, not runtime/audio acceptance"]
fn original_us_room_preset_matches_published_register_values() {
    use bof3_audio::machine::{executable::Executable, profile::Profile};
    let bytes = std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap();
    Profile::identify(&Executable::from_bytes(bytes.clone()).unwrap()).unwrap();
    let expected: Vec<u8> = ROOM.into_iter().flat_map(u16::to_le_bytes).collect();
    assert_eq!(&bytes[0xee024..0xee064], expected);
}
