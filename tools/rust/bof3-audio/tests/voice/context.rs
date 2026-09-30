//! Original-instruction witnesses for the data preceding the note-on pitch table.
//! These deliberately isolate calls; they are not a game boot or PCM reference.
use bof3_audio::{
    digest::sha256_hex, machine::bus::Bus, machine::bus::Ram, machine::bus::Width,
    machine::cpu::Cpu, machine::executable::Executable, machine::profile::Profile,
    machine::profile::US_PITCH_SHA256, soundfont::Zone, voice::tuning::LookupRegion,
    voice::tuning::Reference,
};

fn input() -> (Executable, Profile) {
    let exe =
        Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    let profile = Profile::identify(&exe).unwrap();
    (exe, profile)
}

fn call(ram: &mut Ram, exe: &Executable, entry: u32, args: [u32; 4]) -> u32 {
    let mut cpu = Cpu::new(entry);
    cpu.set_register(29, exe.stack_pointer().unwrap());
    cpu.set_register(31, 0x8000_1000);
    for (i, arg) in args.into_iter().enumerate() {
        cpu.set_register(i + 4, arg);
    }
    cpu.run_until(ram, 0x8000_1000, 200).unwrap();
    cpu.register(2)
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn earlier_table_has_an_original_consumer_and_negative_reads_use_verified_table_bytes() {
    let (exe, profile) = input();
    let mut ram = Ram::from_executable(&exe);
    let mut reference = Reference::from_executable(&exe).unwrap();
    for address in [profile.pitch_table, profile.spu_pitch_table] {
        let start = address as usize - 0x8000_0000;
        assert_eq!(
            sha256_hex(&ram.bytes()[start..start + 386]),
            US_PITCH_SHA256
        );
    }
    for center in [36, 60, 83] {
        for key in [24, 60, 84, 127] {
            for shift in [0, 63, 127, 128, 255] {
                let expected = reference.pitch_register(key, center, shift).unwrap();
                let actual = call(
                    &mut ram,
                    &exe,
                    profile.spu_pitch_entry,
                    [u32::from(center), 0, u32::from(key), u32::from(shift)],
                );
                assert_eq!(actual, u32::from(expected));
            }
        }
    }
    // BGM000 program 2 / tone 0 / key 0, including its signed remainder.
    let alias = reference.key(0, 83, 65, 44100).unwrap();
    assert_eq!(alias.pitch_table_index, Some(-168));
    assert_eq!(alias.pitch_register, 81);
    let lookup = alias.pitch_lookup.unwrap();
    assert_eq!(lookup.runtime_address, 0x8018_430c);
    assert_eq!(lookup.source_file_offset, 0xee30c);
    assert_eq!(lookup.value, 5235);
    assert_eq!(lookup.region, LookupRegion::EarlierSpuPitchTable);
    assert_eq!(lookup.execution_state, "isolated_executable");
    let fit = alias.sf2.as_ref().unwrap();
    assert!(fit.error_cents.abs() < 0.5);
    assert_eq!(fit.target_pcm_rate, 81.0 * 44100.0 / 4096.0);
    assert!(alias.zone(&Zone::new(0)).is_ok());
    // Another declared key from this same tone reaches adjacent data.
    let gap = reference.key(22, 83, 65, 44100).unwrap();
    assert_eq!(gap.pitch_lookup.unwrap().runtime_address, 0x8018_444c);
    assert_eq!(gap.pitch_lookup.unwrap().region, LookupRegion::AdjacentData);
    assert!(gap.sf2.is_none());
    let standard = reference.key(60, 60, 0, 44100).unwrap();
    assert_eq!(
        standard.pitch_lookup.unwrap().region,
        LookupRegion::NoteOnPitchTable
    );
    assert_eq!(standard.pitch_lookup.unwrap().value, 4096);
    assert!(standard.sf2.is_some());
    // A cached call still retains the witnessed lookup, not just the register.
    let again = reference.key(0, 83, 65, 22050).unwrap();
    assert_eq!(again.sf2_sample_rate, 22050);
    assert_eq!(again.pitch_lookup.unwrap().runtime_address, 0x8018_430c);
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_tick_mode_changes_an_adjacent_pitch_lookup_and_its_result() {
    let (exe, profile) = input();
    let mut ram = Ram::from_executable(&exe);
    for (address, width, value) in [
        (0x8018_e7df, Width::Byte, 0),
        (0x8018_e7e4, Width::Byte, 0),
        (0x8018_e25c, Width::Word, 0x8001_0000),
        (0x8001_0004, Width::Byte, 61),
        (0x8001_0005, Width::Byte, 16),
    ] {
        ram.write(address, width, value).unwrap();
    }
    let row = Reference::from_executable(&exe)
        .unwrap()
        .key(0, 61, 16, 44100)
        .unwrap();
    assert_eq!(row.pitch_lookup.unwrap().runtime_address, 0x8018_4440);
    assert_eq!(row.pitch_lookup.unwrap().value, 60);
    assert!(row.sf2.is_none());
    assert_eq!(call(&mut ram, &exe, profile.pitch_entry, [0; 4]), 1);
    assert_eq!(ram.read(0x8018_4440, Width::Word).unwrap(), 60);
    assert_eq!(ram.read(0x8018_4444, Width::Word).unwrap(), 1);
    // Original game initializer calls this entry with a0=1 in its delay slot.
    // We execute that callee alone; preceding initialization is not substituted.
    assert_eq!(
        ram.read(0x8015_cd24, Width::Word).unwrap(),
        0x0c00_0000 | ((profile.set_tick_mode >> 2) & 0x03ff_ffff)
    );
    assert_eq!(ram.read(0x8015_cd28, Width::Word).unwrap(), 0x2404_0001);
    call(&mut ram, &exe, profile.set_tick_mode, [1, 0, 0, 0]);
    assert_eq!(ram.read(0x8018_4440, Width::Word).unwrap(), 5);
    assert_eq!(ram.read(0x8018_4444, Width::Word).unwrap(), 0);
    assert_eq!(call(&mut ram, &exe, profile.pitch_entry, [0; 4]), 0);
    call(&mut ram, &exe, profile.set_tick_mode, [60, 0, 0, 0]);
    assert_eq!(ram.read(0x8018_4440, Width::Word).unwrap(), 60);
    assert_eq!(call(&mut ram, &exe, profile.pitch_entry, [0; 4]), 1);
}
