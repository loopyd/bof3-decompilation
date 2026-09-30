use bof3_audio::{
    digest::sha256_hex,
    driver::retirement::{Before, Retirement},
    machine::{
        adsr, boot,
        bus::{Bus, Ram, Width},
        cpu::{Cpu, FaultKind},
        executable::Executable,
        firmware::{Image, ROM_BYTES},
        interconnect::Interconnect,
        profile::Profile,
        spu_clock, spu_reverb, spu_sample,
        spu_voice_ports::DisableModel,
        thread_context,
    },
};

#[path = "reference/runtime.rs"]
mod reference;

fn media() -> (Vec<u8>, Executable) {
    (
        std::fs::read(std::env::var_os("BOF3_AUDIO_BIOS").unwrap()).unwrap(),
        Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap(),
    )
}

#[test]
#[ignore = "requires BOF3_AUDIO_BIOS, BOF3_AUDIO_EXE and BOF3_AUDIO_REDUX target capture directory"]
fn us_handoff_matches_independent_redux_ram_and_register_capture() {
    let (rom, exe) = media();
    let root = std::path::PathBuf::from(std::env::var_os("BOF3_AUDIO_REDUX").unwrap());
    let (expected, ram) = reference::capture(&root, &rom, exe.bytes());
    assert_eq!(expected.pc, exe.header().entry_pc);
    let machine = boot::load_us(Image::from_bytes(rom).unwrap(), &exe, 3_000_000).unwrap();
    reference::compare(
        &machine.cpu,
        machine.bus.ram().bytes(),
        &expected,
        &ram,
        "US EXE entry before first instruction",
    );
}

#[test]
fn unknown_bios_is_rejected_before_any_execution() {
    let mut bytes = vec![0; 0x804];
    bytes[..8].copy_from_slice(b"PS-X EXE");
    for (offset, value) in [(0x10, 0x80010000u32), (0x18, 0x80010000), (0x1c, 4)] {
        bytes[offset..offset + 4].copy_from_slice(&value.to_le_bytes());
    }
    let exe = Executable::from_bytes(bytes).unwrap();
    let result = boot::load_us(
        Image::from_bytes(vec![0; ROM_BYTES]).unwrap(),
        &exe,
        3_000_000,
    );
    assert!(result
        .err()
        .unwrap()
        .to_string()
        .contains("unsupported BIOS"));
}

#[test]
#[ignore = "requires BOF3_AUDIO_BIOS and BOF3_AUDIO_EXE"]
fn us_handoff_preserves_bios_ram_registers_and_loads_only_the_declared_payload() {
    let (rom, exe) = media();
    let mut before = Interconnect::from_ram(Ram::default());
    before
        .attach_firmware(Image::from_bytes(rom.clone()).unwrap())
        .unwrap();
    let mut cpu = Cpu::pcsx_redux_reset();
    cpu.run_until(&mut before, boot::SHELL_ENTRY, 3_000_000)
        .unwrap();
    assert_eq!(cpu.instructions(), 2_695_618);
    let mut machine =
        boot::load_us(Image::from_bytes(rom.clone()).unwrap(), &exe, 2_695_618).unwrap();
    let start = 0x96800;
    let end = start + exe.text().len();
    assert_eq!(
        &machine.bus.ram().bytes()[..start],
        &before.ram().bytes()[..start]
    );
    assert_eq!(&machine.bus.ram().bytes()[start..end], exe.text());
    assert_eq!(
        &machine.bus.ram().bytes()[end..],
        &before.ram().bytes()[end..]
    );
    assert_eq!(
        machine.evidence.shell_ram_sha256,
        sha256_hex(before.ram().bytes())
    );
    assert_eq!(
        machine.evidence.loaded_ram_sha256,
        sha256_hex(machine.bus.ram().bytes())
    );
    assert_eq!(machine.evidence.syscall_exceptions, 0);
    assert_eq!(machine.evidence.shell_return, 0xbfc0702c);
    assert_eq!(machine.cpu.pc(), exe.header().entry_pc);
    for register in 0..32 {
        assert_eq!(
            machine.cpu.register(register),
            if register == 29 {
                0x801ffff0
            } else {
                cpu.register(register)
            }
        );
    }
    assert_eq!(machine.cpu.cop0().status(), cpu.cop0().status());
    assert_eq!((machine.cpu.hi(), machine.cpu.lo()), (cpu.hi(), cpu.lo()));
    assert_eq!(machine.bus.read(0xfffe0130, Width::Word).unwrap(), 0x1e988);
    thread_context::Context::current(&mut machine.bus).unwrap();
    let step = machine.cpu.step(&mut machine.bus).unwrap();
    assert_eq!(step.instruction, before_word(&exe, step.pc));
    for limit in [0, 1, 10_000_001] {
        assert!(boot::load_us(Image::from_bytes(rom.clone()).unwrap(), &exe, limit).is_err());
    }
    let mut changed = exe.bytes().to_vec();
    changed[0x804] ^= 1;
    assert!(boot::load_us(
        Image::from_bytes(rom).unwrap(),
        &Executable::from_bytes(changed).unwrap(),
        3_000_000
    )
    .is_err());
}

fn before_word(exe: &Executable, pc: u32) -> u32 {
    let offset = (pc - exe.header().load_address) as usize;
    u32::from_le_bytes(exe.text()[offset..offset + 4].try_into().unwrap())
}

#[test]
#[ignore = "requires BOF3_AUDIO_BIOS and BOF3_AUDIO_EXE; real BIOS kernel, no HLE calls"]
fn original_callback_and_sound_initializers_return_using_bios_created_kernel_state() {
    let (rom, exe) = media();
    let profile = Profile::identify(&exe).unwrap();
    for (entry, instructions, exception_count) in
        [(0x801748e4, 5281, 2), (profile.sound_initialize, 67615, 4)]
    {
        let (mut machine, actual_instructions, exceptions) = execute_initializer(&rom, &exe, entry);
        let cpu = &machine.cpu;
        assert_eq!(cpu.pc(), 0x80010000);
        assert_eq!(actual_instructions, instructions);
        assert_eq!(exceptions, exception_count);
        assert_eq!(cpu.register(29), 0x801ffff0);
        thread_context::Context::current(&mut machine.bus).unwrap();
        if entry == profile.sound_initialize {
            assert_eq!(cpu.register(2), 0);
            assert_eq!(machine.bus.read(0x80190b88, Width::Half).unwrap(), 2);
            assert_eq!(machine.bus.read(0x80190b8a, Width::Half).unwrap(), 4);
            for handle in 0..2 {
                assert_eq!(
                    machine
                        .bus
                        .read(0x80190308 + handle * 4, Width::Word)
                        .unwrap(),
                    0x80148a50 + handle * 4 * 0xac
                );
            }
            assert_eq!(machine.bus.read(0x80184440, Width::Word).unwrap(), 5);
            assert_eq!(machine.bus.read(0x80184444, Width::Word).unwrap(), 0);
            assert_eq!(machine.bus.read(0x1f801dac, Width::Half).unwrap(), 4);
        }
    }
}

fn execute_initializer(rom: &[u8], exe: &Executable, entry: u32) -> (boot::Machine, u64, u64) {
    let mut machine =
        boot::load_us(Image::from_bytes(rom.to_vec()).unwrap(), exe, 3_000_000).unwrap();
    let start = machine.cpu.instructions();
    let mut cpu = machine.cpu;
    cpu.resume_at(entry);
    for register in 4..8 {
        cpu.set_register(register, 0);
    }
    cpu.set_register(31, 0x80010000);
    machine
        .bus
        .configure_spu_voices(
            spu_sample::Model::EmulatorReference,
            adsr::Model::EmulatorReference,
        )
        .unwrap();
    machine
        .bus
        .configure_spu_disable(DisableModel::EmulatorReference)
        .unwrap();
    machine
        .bus
        .configure_spu_reverb(spu_reverb::Model::EmulatorReference)
        .unwrap();
    let mut clock = spu_clock::Clock::new(spu_clock::Model::EmulatorReference);
    let mut exceptions = 0;
    let mut retirement = Retirement::default();
    for _ in 0..100_000 {
        if cpu.pc() == 0x80010000 {
            break;
        }
        let before = Before::capture(&cpu);
        match cpu.step(&mut machine.bus) {
            Ok(step) => retirement.observe(exe, &before, &step).unwrap(),
            Err(fault) if fault.kind == FaultKind::Syscall => {
                cpu.enter_exception(&fault).unwrap();
                exceptions += 1;
            }
            Err(fault) => panic!("{fault}"),
        }
        clock.advance(&mut machine.bus, 2).unwrap();
    }
    let accesses: Vec<_> = retirement
        .memory
        .iter()
        .filter(|(access, _)| {
            let address = access.bus_address & 0x1fffffff;
            (0x1f801c00..0x1f801d80).contains(&address) && access.operation == "read"
                || (0x18dbf6..=0x18e0a2).contains(&address)
                    && (address - 0x18dbf6) % 52 == 0
                    && access.operation == "write"
                || address == 0x190c1c
        })
        .map(|(access, count)| serde_json::json!({"access": access, "count": count}))
        .collect();
    println!(
        "{}",
        serde_json::json!({"schema":"bof3.initialization-accesses/v1", "entry":entry, "accesses":accesses})
    );
    let instructions = cpu.instructions() - start;
    machine.cpu = cpu;
    (machine, instructions, exceptions)
}

#[test]
#[ignore = "requires BOF3_AUDIO_BIOS, BOF3_AUDIO_EXE and BOF3_AUDIO_REDUX_INITIALIZATION with callback/sound captures"]
fn original_initializers_match_independent_redux_ram_and_registers() {
    let (rom, exe) = media();
    let root =
        std::path::PathBuf::from(std::env::var_os("BOF3_AUDIO_REDUX_INITIALIZATION").unwrap());
    let profile = Profile::identify(&exe).unwrap();
    for (name, entry) in [
        ("callback", 0x801748e4),
        ("sound", profile.sound_initialize),
    ] {
        let (expected, ram) = reference::capture(&root.join(name), &rom, exe.bytes());
        let (machine, _, _) = execute_initializer(&rom, &exe, entry);
        reference::compare(
            &machine.cpu,
            machine.bus.ram().bytes(),
            &expected,
            &ram,
            name,
        );
    }
}
