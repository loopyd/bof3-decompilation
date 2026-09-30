//! Local executable evidence, not a substitute for independently captured audio.
use bof3_audio::machine::{
    bus::{Bus, Ram, Width},
    cpu::Cpu,
    executable::Executable,
};
use bof3_audio::{
    digest::sha256_hex,
    machine::profile::{Profile, US_PITCH_SHA256},
};

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_pitch_routine_matches_instruction_derived_integer_formula() {
    let path = std::env::var_os("BOF3_AUDIO_EXE").expect("set BOF3_AUDIO_EXE");
    let exe = Executable::from_bytes(std::fs::read(path).unwrap()).unwrap();
    let profile = Profile::identify(&exe).unwrap();
    assert_eq!(exe.header().load_address, 0x8009_6800);
    let mut ram = Ram::from_executable(&exe);
    let table_offset = profile.pitch_table as usize - 0x8000_0000;
    assert_eq!(
        sha256_hex(&ram.bytes()[table_offset..table_offset + profile.pitch_table_entries * 2]),
        US_PITCH_SHA256
    );
    // Configure one tone context. Addresses derive from original routine loads;
    // no SDK entrypoint substitution or host note-to-pitch implementation is called.
    ram.write(0x8018_e7df, Width::Byte, 0).unwrap();
    ram.write(0x8018_e7e4, Width::Byte, 0).unwrap();
    ram.write(0x8018_e25c, Width::Word, 0x8001_0000).unwrap();
    let mut cases = 0;
    let mut max_instructions = 0;
    for center in [36, 48, 60, 72, 84] {
        for note in 24..=108 {
            for shift in [0, 1, 7, 8, 63, 127, 128, 191, 248, 255] {
                ram.write(0x8001_0004, Width::Byte, center).unwrap();
                ram.write(0x8001_0005, Width::Byte, shift).unwrap();
                let mut cpu = Cpu::new(profile.pitch_entry);
                cpu.set_register(4, note);
                cpu.set_register(5, 0);
                cpu.set_register(31, 0x8000_1000);
                let instructions = cpu.run_until(&mut ram, 0x8000_1000, 100).unwrap();
                let fine = shift / 8;
                let semitones = note + 60 - center + u32::from(fine >= 16);
                let index = (semitones % 12) * 16 + fine % 16;
                let table = ram.read(0x8018_445c + index * 2, Width::Half).unwrap();
                let octave = (semitones / 12) as i32 - 5;
                let expected = if octave >= 0 {
                    table << octave
                } else {
                    table >> -octave
                } & 0xffff;
                assert_eq!(
                    cpu.register(2),
                    expected,
                    "note={note} center={center} shift={shift}"
                );
                assert_eq!(cpu.register(0), 0);
                max_instructions = max_instructions.max(instructions);
                cases += 1;
            }
        }
    }
    eprintln!("Original US pitch routine: {cases} integer-formula comparisons, at most {max_instructions} instructions/call; timing and PCM not verified");
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn modified_or_extended_us_executable_is_rejected() {
    let path = std::env::var_os("BOF3_AUDIO_EXE").expect("set BOF3_AUDIO_EXE");
    let bytes = std::fs::read(path).unwrap();
    for offset in [0x800, 0xee45c, bytes.len() - 1] {
        let mut changed = bytes.clone();
        changed[offset] ^= 1;
        let exe = Executable::from_bytes(changed).unwrap();
        assert!(
            Profile::identify(&exe).is_err(),
            "changed offset {offset:x}"
        );
    }
    let mut changed = bytes;
    changed.push(0);
    assert!(Profile::identify(&Executable::from_bytes(changed).unwrap()).is_err());
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_cue_dispatch_selects_table_entries_and_handles_the_sentinel() {
    let path = std::env::var_os("BOF3_AUDIO_EXE").expect("set BOF3_AUDIO_EXE");
    let exe = Executable::from_bytes(std::fs::read(path).unwrap()).unwrap();
    let profile = Profile::identify(&exe).unwrap();
    let mut ram = Ram::from_executable(&exe);
    for cue in 0..255 {
        let expected_bank = ram.read(0x8018_1eba + cue * 4, Width::Byte).unwrap();
        let expected_sequence = ram.read(0x8018_1ebb + cue * 4, Width::Byte).unwrap();
        let mut cpu = Cpu::new(profile.cue_dispatch);
        cpu.set_register(4, cue | 0x1200); // dispatcher uses only the low byte
        cpu.set_register(5, 0x3456);
        cpu.set_register(6, 0x789a);
        cpu.set_register(29, exe.stack_pointer().unwrap());
        cpu.set_register(31, 0x8000_1000);
        // Stop at the real first callee. Do not substitute a host implementation
        // or claim the rest of playback ran merely because this dispatch works.
        cpu.run_until(&mut ram, 0x8015_d300, 40).unwrap();
        assert_eq!(cpu.register(4), expected_bank, "cue {cue}");
        assert_eq!(cpu.register(5), expected_sequence, "cue {cue}");
        assert_eq!((cpu.register(6), cpu.register(7)), (0, 0));
        assert_eq!(cpu.register(31), 0x8016_1c78);
    }
    let mut cpu = Cpu::new(profile.cue_dispatch);
    cpu.set_register(4, 0xffff);
    cpu.set_register(29, exe.stack_pointer().unwrap());
    cpu.set_register(31, 0x8000_1000);
    for register in 16..20 {
        cpu.set_register(register, 0x9876 + register as u32);
    }
    cpu.run_until(&mut ram, 0x8000_1000, 40).unwrap();
    assert_eq!(cpu.register(29), exe.stack_pointer().unwrap());
    for register in 16..20 {
        assert_eq!(cpu.register(register), 0x9876 + register as u32);
    }
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_sequence_table_initializer_builds_two_independent_four_sequence_handles() {
    let path = std::env::var_os("BOF3_AUDIO_EXE").expect("set BOF3_AUDIO_EXE");
    let exe = Executable::from_bytes(std::fs::read(path).unwrap()).unwrap();
    let profile = Profile::identify(&exe).unwrap();
    let mut bus = Ram::from_executable(&exe);
    // Exact arguments emitted by game sound initialization at 0x8015CD10..20.
    let base = 0x8014_8a50;
    let mut cpu = Cpu::new(profile.sequence_table_setup);
    cpu.set_register(4, base);
    cpu.set_register(5, 2);
    cpu.set_register(6, 4);
    cpu.set_register(29, exe.stack_pointer().unwrap());
    cpu.set_register(31, 0x8000_1000);
    cpu.run_until(&mut bus, 0x8000_1000, 10_000).unwrap();
    assert_eq!(bus.read(0x8019_0b88, Width::Half).unwrap(), 2);
    assert_eq!(bus.read(0x8019_0b8a, Width::Half).unwrap(), 4);
    for handle in 0..2 {
        let expected = base + handle * 4 * 0xac;
        assert_eq!(
            bus.read(0x8019_0308 + handle * 4, Width::Word).unwrap(),
            expected
        );
        for sequence in 0..4 {
            let state = expected + sequence * 0xac;
            assert_eq!(bus.read(state + 0x90, Width::Word).unwrap(), 0);
            assert_eq!(bus.read(state + 0x3c, Width::Byte).unwrap(), 0xff);
            assert_eq!(bus.read(state, Width::Byte).unwrap(), 0);
        }
    }
}
