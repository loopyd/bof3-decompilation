use bof3_audio::{
    driver::retirement::{Before, Retirement},
    machine::{
        bus::{Bus, Width},
        cpu::Cpu,
        executable::Executable,
        firmware::{Image, ROM_BYTES},
        interconnect::Interconnect,
    },
};

const BASE: u32 = 0x80010000;
const DATA: u32 = 0x80020000;

fn fixture(words: &[u32]) -> (Executable, Cpu, Interconnect) {
    let mut bytes = vec![0; 0x800];
    bytes[..8].copy_from_slice(b"PS-X EXE");
    bytes[0x10..0x14].copy_from_slice(&BASE.to_le_bytes());
    bytes[0x18..0x1c].copy_from_slice(&BASE.to_le_bytes());
    bytes[0x1c..0x20].copy_from_slice(&((words.len() * 4) as u32).to_le_bytes());
    for word in words {
        bytes.extend(word.to_le_bytes());
    }
    let exe = Executable::from_bytes(bytes).unwrap();
    let mut bus = Interconnect::from_executable(&exe);
    bus.write(DATA, Width::Word, 0x44332211).unwrap();
    let mut cpu = Cpu::new(BASE);
    cpu.set_register(4, DATA);
    cpu.set_register(2, 0xaabbccdd);
    (exe, cpu, bus)
}

#[test]
fn retired_memory_uses_pre_step_registers_across_a_load_delay() {
    // lw a0,0(a1); lw v0,0(a0); nop
    let (exe, mut cpu, mut bus) = fixture(&[0x8ca40000, 0x8c820000, 0]);
    cpu.set_register(5, DATA + 4);
    bus.write(DATA + 4, Width::Word, DATA + 8).unwrap();
    bus.write(DATA + 8, Width::Word, 0x55667788).unwrap();
    cpu.step(&mut bus).unwrap();
    let before = Before::capture(&cpu);
    let step = cpu.step(&mut bus).unwrap();
    assert_eq!(cpu.register(4), DATA + 8);
    assert_eq!(step.scheduled_load.unwrap().value, 0x44332211);
    let mut evidence = Retirement::default();
    evidence.observe(&exe, &before, &step).unwrap();
    assert_eq!(
        evidence.memory.keys().next().unwrap().effective_address,
        DATA
    );
}

#[test]
fn byte_lanes_match_actual_partial_stores_and_merged_loads() {
    for opcode in [
        0x20u32, 0x24, 0x21, 0x25, 0x23, 0x22, 0x26, 0x28, 0x29, 0x2b, 0x2a, 0x2e,
    ] {
        for lane in 0..4 {
            if matches!(opcode, 0x21 | 0x25 | 0x29) && lane % 2 != 0
                || matches!(opcode, 0x23 | 0x2b) && lane != 0
            {
                continue;
            }
            let word = opcode << 26 | 4 << 21 | 2 << 16 | lane;
            let (exe, mut cpu, mut bus) = fixture(&[word, 0]);
            let before = Before::capture(&cpu);
            let step = cpu.step(&mut bus).unwrap();
            let mut evidence = Retirement::default();
            evidence.observe(&exe, &before, &step).unwrap();
            let access = evidence.memory.keys().next().unwrap();
            assert_eq!(access.effective_address, DATA + lane);
            assert_eq!(access.word_address, DATA);
            let merged = matches!(opcode, 0x22 | 0x26 | 0x2a | 0x2e);
            assert_eq!(access.bus_address, if merged { DATA } else { DATA + lane });
            assert_eq!(
                access.bus_width,
                match opcode {
                    0x20 | 0x24 | 0x28 => 1,
                    0x21 | 0x25 | 0x29 => 2,
                    _ => 4,
                }
            );
            let lanes = match opcode {
                0x20 | 0x24 | 0x28 => 1 << lane,
                0x21 | 0x25 | 0x29 => 3 << lane,
                0x23 | 0x2b => 15,
                0x22 | 0x2a => [1, 3, 7, 15][lane as usize],
                _ => [15, 14, 12, 8][lane as usize],
            };
            assert_eq!(access.byte_lanes, lanes, "opcode {opcode:x} lane {lane}");
            if opcode >= 0x28 {
                assert_eq!(access.operation, "write");
                let actual = bus.read(DATA, Width::Word).unwrap().to_le_bytes();
                let original = 0x44332211u32.to_le_bytes();
                for byte in 0..4 {
                    assert_eq!(actual[byte] != original[byte], lanes & (1 << byte) != 0);
                }
            } else {
                assert_eq!(access.operation, "read");
                assert_eq!(bus.read(DATA, Width::Word).unwrap(), 0x44332211);
            }
        }
    }
}

#[test]
fn bios_and_kernel_instructions_are_recorded_from_successful_steps() {
    let (exe, mut cpu, mut bus) = fixture(&[0x8c820000, 0]);
    let mut rom = vec![0; ROM_BYTES];
    rom[..4].copy_from_slice(&0x8c820000u32.to_le_bytes());
    bus.attach_firmware(Image::from_bytes(rom).unwrap())
        .unwrap();
    bus.write(0x80003000, Width::Word, 0x8c820000).unwrap();
    let mut evidence = Retirement::default();
    for (pc, region) in [
        (BASE, "executable"),
        (0xbfc00000, "bios_rom"),
        (0x80003000, "other_ram"),
    ] {
        cpu = if pc == BASE { cpu } else { Cpu::new(pc) };
        cpu.set_register(4, DATA);
        let before = Before::capture(&cpu);
        let step = cpu.step(&mut bus).unwrap();
        evidence.observe(&exe, &before, &step).unwrap();
        assert!(evidence
            .instructions
            .keys()
            .any(|i| i.pc == pc && i.region == region));
    }
    assert_eq!(evidence.memory.len(), 3);
    assert!(evidence.report()["limitations"].is_array());
}

#[test]
fn mismatched_boundaries_or_changed_original_words_leave_no_evidence() {
    let (exe, mut cpu, mut bus) = fixture(&[0x8c820000, 0]);
    let wrong = Before::capture(&Cpu::new(BASE + 4));
    let before = Before::capture(&cpu);
    let step = cpu.step(&mut bus).unwrap();
    let mut evidence = Retirement::default();
    assert!(evidence.observe(&exe, &wrong, &step).is_err());
    let mut changed = step;
    changed.instruction = 0;
    assert!(evidence.observe(&exe, &before, &changed).is_err());
    assert!(evidence.instructions.is_empty() && evidence.memory.is_empty());
}
