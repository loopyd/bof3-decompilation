use bof3_audio::machine::{
    bus::{Bus, BusError, Ram, Width},
    cpu::{Cpu, FaultKind},
};

fn immediate(opcode: u32, rs: u32, rt: u32, value: i16) -> u32 {
    opcode << 26 | rs << 21 | rt << 16 | u32::from(value as u16)
}
fn special(function: u32, rs: u32, rt: u32, rd: u32) -> u32 {
    rs << 21 | rt << 16 | rd << 11 | function
}
fn program(words: &[u32]) -> (Cpu, Ram) {
    let mut bus = Ram::default();
    for (index, &word) in words.iter().enumerate() {
        bus.write(0x1000 + index as u32 * 4, Width::Word, word)
            .unwrap();
    }
    (Cpu::new(0x8000_1000), bus)
}

#[test]
fn taken_and_untaken_branches_both_execute_the_delay_slot() {
    for (opcode, equal, target) in [
        (4, true, 0x1010),
        (4, false, 0x1008),
        (5, true, 0x1008),
        (5, false, 0x1010),
    ] {
        let (mut cpu, mut ram) = program(&[immediate(opcode, 1, 2, 3), immediate(9, 3, 3, 7)]);
        cpu.set_register(1, 10);
        cpu.set_register(2, if equal { 10 } else { 11 });
        cpu.step(&mut ram).unwrap();
        assert_eq!(cpu.pc(), 0x8000_1004);
        let slot = cpu.step(&mut ram).unwrap();
        assert!(slot.in_delay_slot);
        assert_eq!(cpu.register(3), 7);
        assert_eq!(cpu.pc(), 0x8000_0000 | target);
    }
}

#[test]
fn link_is_visible_in_the_slot_and_returns_after_it() {
    let (mut cpu, mut ram) = program(&[(3 << 26) | (0x1020 >> 2), special(0x21, 31, 0, 8)]);
    ram.write(0x1020, Width::Word, special(8, 31, 0, 0))
        .unwrap();
    ram.write(0x1024, Width::Word, immediate(9, 9, 9, 1))
        .unwrap();
    cpu.run_until(&mut ram, 0x8000_1008, 4).unwrap();
    assert_eq!(cpu.register(31), 0x8000_1008);
    assert_eq!(cpu.register(8), 0x8000_1008);
    assert_eq!(cpu.register(9), 1);
}

#[test]
fn conditional_link_writes_ra_even_when_not_taken() {
    let (mut cpu, mut ram) = program(&[immediate(1, 1, 16, 4), 0]); // bltzal
    cpu.set_register(1, 1);
    cpu.step(&mut ram).unwrap();
    assert_eq!(cpu.register(31), 0x8000_1008);
    cpu.step(&mut ram).unwrap();
    assert_eq!(cpu.pc(), 0x8000_1008);
}

#[test]
fn load_reads_old_value_in_next_instruction_then_commits() {
    let (mut cpu, mut ram) = program(&[
        immediate(0x23, 0, 1, 0x2000),
        special(0x21, 1, 0, 2),
        special(0x21, 1, 0, 3),
    ]);
    ram.write(0x2000, Width::Word, 42).unwrap();
    cpu.set_register(1, 7);
    cpu.step(&mut ram).unwrap();
    assert_eq!(cpu.register(1), 7);
    let step = cpu.step(&mut ram).unwrap();
    assert_eq!(step.committed_load.unwrap().value, 42);
    assert_eq!(cpu.register(2), 7);
    cpu.step(&mut ram).unwrap();
    assert_eq!(cpu.register(3), 42);
}

#[test]
fn intervening_write_cancels_pending_load() {
    let (mut cpu, mut ram) = program(&[immediate(0x23, 0, 1, 0x2000), immediate(9, 1, 1, 2), 0]);
    ram.write(0x2000, Width::Word, 42).unwrap();
    cpu.set_register(1, 7);
    for _ in 0..3 {
        cpu.step(&mut ram).unwrap();
    }
    assert_eq!(cpu.register(1), 9);
}

#[test]
fn a_second_load_to_the_same_register_replaces_the_pending_value() {
    let (mut cpu, mut ram) = program(&[
        immediate(0x23, 0, 1, 0x2000),
        immediate(0x23, 0, 1, 0x2004),
        special(0x21, 1, 0, 2),
        special(0x21, 1, 0, 3),
    ]);
    cpu.set_register(1, 7);
    ram.write(0x2000, Width::Word, 42).unwrap();
    ram.write(0x2004, Width::Word, 99).unwrap();
    cpu.run_until(&mut ram, 0x8000_1010, 4).unwrap();
    assert_eq!(cpu.register(2), 7);
    assert_eq!(cpu.register(3), 99);
}

#[test]
fn fault_does_not_undo_a_load_from_the_previous_instruction() {
    let (mut cpu, mut ram) = program(&[immediate(0x23, 0, 1, 0x2000), 0x0d]);
    ram.write(0x2000, Width::Word, 42).unwrap();
    cpu.step(&mut ram).unwrap();
    assert_eq!(cpu.register(1), 0);
    assert_eq!(cpu.step(&mut ram).unwrap_err().kind, FaultKind::Break);
    assert_eq!(cpu.register(1), 42);
}

#[test]
fn loads_sign_extend_but_unsigned_comparison_immediates_also_sign_extend() {
    for (opcode, expected) in [
        (0x20, 0xffff_ff80),
        (0x24, 0x80),
        (0x21, 0xffff_8080),
        (0x25, 0x8080),
    ] {
        let (mut cpu, mut ram) =
            program(&[immediate(opcode, 0, 1, 0x2000), 0, immediate(11, 1, 2, -1)]);
        ram.write(0x2000, Width::Word, 0x8080).unwrap();
        cpu.run_until(&mut ram, 0x8000_100c, 3).unwrap();
        assert_eq!(cpu.register(1), expected);
        assert_eq!(cpu.register(2), 1);
    }
}

#[test]
fn unaligned_pairs_merge_for_every_offset_and_order() {
    for offset in 0..4 {
        for reverse in [false, true] {
            let mut words = [immediate(0x22, 1, 2, 3), immediate(0x26, 1, 2, 0), 0];
            if reverse {
                words.swap(0, 1);
            }
            let (mut cpu, mut ram) = program(&words);
            cpu.set_register(1, 0x2000 + offset);
            cpu.set_register(2, 0xaaaa_aaaa);
            for index in 0..8 {
                ram.write(0x2000 + index, Width::Byte, 0x10 + index)
                    .unwrap();
            }
            cpu.run_until(&mut ram, 0x8000_100c, 3).unwrap();
            assert_eq!(
                cpu.register(2),
                u32::from_le_bytes(std::array::from_fn(|i| 0x10 + offset as u8 + i as u8))
            );
        }
    }
}

#[test]
fn unaligned_stores_preserve_surrounding_bytes_without_reads() {
    for offset in 0..4 {
        let (mut cpu, mut ram) = program(&[immediate(0x2a, 1, 2, 3), immediate(0x2e, 1, 2, 0)]);
        cpu.set_register(1, 0x2000 + offset);
        cpu.set_register(2, 0x4433_2211);
        ram.write(0x2000, Width::Word, 0xaaaaaaaa).unwrap();
        ram.write(0x2004, Width::Word, 0xaaaaaaaa).unwrap();
        cpu.run_until(&mut ram, 0x8000_1008, 2).unwrap();
        let mut expected = [0xaa; 8];
        expected[offset as usize..offset as usize + 4].copy_from_slice(&[0x11, 0x22, 0x33, 0x44]);
        assert_eq!(&ram.bytes()[0x2000..0x2008], expected);
    }
}

#[test]
fn multiply_and_divide_use_hardware_edge_results() {
    for (function, a, b, hi, lo) in [
        (0x18, 0xffff_fff9, 3, 0xffff_ffff, 0xffff_ffeb),
        (0x19, 0xffff_ffff, 2, 1, 0xffff_fffe),
        (0x1a, 0xffff_fff9, 3, 0xffff_ffff, 0xffff_fffe),
        (0x1a, 7, 0, 7, 0xffff_ffff),
        (0x1a, 0xffff_fff9, 0, 0xffff_fff9, 1),
        (0x1a, 0x8000_0000, 0xffff_ffff, 0, 0x8000_0000),
        (0x1b, 0x8000_0000, 0, 0x8000_0000, 0xffff_ffff),
    ] {
        let (mut cpu, mut ram) = program(&[
            special(function, 1, 2, 0),
            special(0x10, 0, 0, 3),
            special(0x12, 0, 0, 4),
        ]);
        cpu.set_register(1, a);
        cpu.set_register(2, b);
        cpu.run_until(&mut ram, 0x8000_100c, 3).unwrap();
        assert_eq!((cpu.register(3), cpu.register(4)), (hi, lo));
    }
}

#[test]
fn overflow_and_unmapped_or_unaligned_access_fail_precisely() {
    for (instruction, expected) in [
        (special(0x20, 1, 2, 3), FaultKind::Overflow),
        (
            immediate(0x23, 0, 3, 1),
            FaultKind::Alignment {
                address: 1,
                store: false,
            },
        ),
        (
            immediate(0x29, 0, 3, 1),
            FaultKind::Alignment {
                address: 1,
                store: true,
            },
        ),
        (0x4200_0001, FaultKind::UnsupportedInstruction),
        (0x0c, FaultKind::Syscall),
    ] {
        let (mut cpu, mut ram) = program(&[instruction]);
        cpu.set_register(1, 0x7fff_ffff);
        cpu.set_register(2, 1);
        cpu.set_register(3, 99);
        let fault = cpu.step(&mut ram).unwrap_err();
        assert_eq!(fault.kind, expected);
        assert_eq!(fault.pc, 0x8000_1000);
        assert_eq!(fault.instruction, Some(instruction));
        assert_eq!(cpu.register(3), 99);
    }
    let mut cpu = Cpu::new(0xbfc0_0000);
    assert!(matches!(
        cpu.step(&mut Ram::default()).unwrap_err().kind,
        FaultKind::Bus(_)
    ));
}

#[test]
fn delay_slot_fault_and_runaway_are_not_silently_skipped() {
    let (mut cpu, mut ram) = program(&[immediate(4, 0, 0, -1), 0x0d]);
    cpu.step(&mut ram).unwrap();
    let fault = cpu.step(&mut ram).unwrap_err();
    assert_eq!(fault.pc, 0x8000_1004);
    assert!(fault.in_delay_slot);
    assert_eq!(fault.kind, FaultKind::Break);
    let (mut cpu, mut ram) = program(&[immediate(4, 0, 0, -1), 0]);
    assert_eq!(
        cpu.run_until(&mut ram, 0x8000_1010, 100).unwrap_err().kind,
        FaultKind::InstructionLimit(100)
    );
    assert_eq!(cpu.instructions(), 100);
}

struct StoreProbe {
    ram: Ram,
    writes: Vec<(u32, Width, u32)>,
    masked: Vec<(u32, u32, u8)>,
}
impl Bus for StoreProbe {
    fn read(&mut self, address: u32, width: Width) -> Result<u32, BusError> {
        assert!(address < 0x8000_2000, "unexpected data read during store");
        self.ram.read(address, width)
    }
    fn write(&mut self, address: u32, width: Width, value: u32) -> Result<(), BusError> {
        self.writes.push((address, width, value));
        Ok(())
    }
    fn write_masked(&mut self, address: u32, value: u32, lanes: u8) -> Result<(), BusError> {
        self.masked.push((address, value, lanes));
        Ok(())
    }
}

#[test]
fn mmio_receives_full_register_and_single_masked_store_transaction() {
    let (mut cpu, ram) = program(&[immediate(0x29, 1, 2, 0), immediate(0x2a, 1, 2, 1)]);
    let mut bus = StoreProbe {
        ram,
        writes: vec![],
        masked: vec![],
    };
    cpu.set_register(1, 0x1f80_10a0);
    cpu.set_register(2, 0x4433_2211);
    cpu.run_until(&mut bus, 0x8000_1008, 2).unwrap();
    assert_eq!(bus.writes, [(0x1f80_10a0, Width::Half, 0x4433_2211)]);
    assert_eq!(bus.masked, [(0x1f80_10a0, 0x0000_4433, 3)]);
}

#[test]
fn writes_to_zero_are_discarded_but_loads_still_access_the_bus() {
    let (mut cpu, mut ram) = program(&[immediate(9, 0, 0, 1), immediate(0x23, 1, 0, 0)]);
    cpu.set_register(0, 99);
    cpu.set_register(1, 0x1f80_1000);
    cpu.step(&mut ram).unwrap();
    assert_eq!(cpu.register(0), 0);
    assert!(matches!(
        cpu.step(&mut ram).unwrap_err().kind,
        FaultKind::Bus(_)
    ));
}
