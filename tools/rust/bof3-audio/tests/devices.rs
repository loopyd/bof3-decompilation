use bof3_audio::machine::{
    bus::{Bus, Ram, Width},
    cpu::Cpu,
    dma::Dma,
    executable::Executable,
    interconnect::Interconnect,
    interrupts::{Interrupts, Source},
    kernel::Kernel,
};

fn executable() -> Executable {
    let mut bytes = vec![0; 0x804];
    bytes[..8].copy_from_slice(b"PS-X EXE");
    for (offset, value) in [(0x10, 0x8001_0000u32), (0x18, 0x8001_0000), (0x1c, 4)] {
        bytes[offset..offset + 4].copy_from_slice(&value.to_le_bytes());
    }
    Executable::from_bytes(bytes).unwrap()
}

#[test]
fn interrupt_sources_latch_rising_edges_even_while_masked() {
    let mut irq = Interrupts::default();
    irq.set_line(Source::Spu, true);
    assert_eq!(irq.status(), 1 << 9);
    assert!(!irq.pending());
    irq.set_mask(1 << 9);
    assert!(irq.pending());
    irq.acknowledge(!(1 << 9));
    assert!(!irq.pending());
    irq.set_line(Source::Spu, true);
    assert_eq!(
        irq.status(),
        0,
        "a held line must not relatch after I_STAT ack"
    );
    irq.set_line(Source::Spu, false);
    irq.set_line(Source::Spu, true);
    assert!(irq.pending());
    irq.set_line(Source::Timer2, true);
    irq.acknowledge(1 << 9);
    assert_eq!(
        irq.status(),
        1 << 9,
        "zero bits acknowledge; one bits preserve"
    );
    irq.set_mask(u32::MAX);
    assert_eq!(irq.mask(), 0x7ff);
}

#[test]
fn dma_completion_latches_only_when_enabled_and_requires_explicit_ack() {
    let mut dma = Dma::default();
    assert_eq!(dma.priority(), 0x0765_4321);
    dma.complete(4).unwrap();
    assert_eq!(dma.interrupt(), 0);
    let enabled = (1 << 23) | (1 << 20);
    dma.set_interrupt(enabled);
    dma.complete(4).unwrap();
    assert_eq!(dma.interrupt(), enabled | (1 << 28) | (1 << 31));
    dma.set_interrupt(1 << 23); // Channel disabled after completion: flag persists.
    assert!(dma.irq_line());
    dma.set_interrupt((1 << 23) | (1 << 28));
    assert!(!dma.irq_line());
    dma.set_interrupt(1 << 15);
    assert!(dma.irq_line());
    assert!(dma.complete(7).is_err());
}

#[test]
fn interconnect_preserves_aliases_scratchpad_and_full_dma_store_values() {
    let mut bus = Interconnect::from_executable(&executable());
    bus.write(0xa001_0000, Width::Word, 0x1234_5678).unwrap();
    assert_eq!(bus.read(0x0001_0000, Width::Word).unwrap(), 0x1234_5678);
    bus.write(0x1f80_0000, Width::Word, 0xaabb_ccdd).unwrap();
    bus.write_masked(0x9f80_0000, 0x1122_3344, 0b0101).unwrap();
    assert_eq!(bus.read(0x1f80_0000, Width::Word).unwrap(), 0xaa22_cc44);
    assert!(bus.read(0xbf80_0000, Width::Word).is_err());
    bus.write(0x1f80_10f0, Width::Half, 0x3333_3333).unwrap();
    assert_eq!(bus.dma().priority(), 0x3333_3333);
    assert!(
        bus.write(0x1f80_10c8, Width::Word, 0x0100_0201).is_err(),
        "DMA4 without a valid block size may not silently start/finish"
    );
}

#[test]
fn dma_force_irq_reaches_controller_only_on_edges() {
    let mut bus = Interconnect::from_executable(&executable());
    bus.write(0x1f80_1074, Width::Half, 8).unwrap();
    bus.write(0x1f80_10f4, Width::Word, 1 << 15).unwrap();
    assert!(bus.interrupts().pending());
    bus.write(0x1f80_1070, Width::Half, 0).unwrap();
    bus.write(0x1f80_10f4, Width::Word, 1 << 15).unwrap();
    assert!(!bus.interrupts().pending());
    bus.write(0x1f80_10f4, Width::Word, 0).unwrap();
    bus.write(0x1f80_10f4, Width::Word, 1 << 15).unwrap();
    assert!(bus.interrupts().pending());
}

#[test]
fn kernel_setjmp_and_longjmp_preserve_exact_context_and_zero_return() {
    let mut cpu = Cpu::new(0xa0);
    let mut bus = Ram::default();
    let mut kernel = Kernel::default();
    for register in 1..32 {
        cpu.set_register(register, 0x10000 + register as u32);
    }
    cpu.set_register(31, 0x8001_0000);
    cpu.set_register(4, 0x8000_2000);
    cpu.set_register(9, 0x13);
    kernel.dispatch(&mut cpu, &mut bus).unwrap();
    assert_eq!(cpu.pc(), 0x8001_0000);
    assert_eq!(cpu.register(2), 0);
    assert_eq!(bus.read(0x2000, Width::Word).unwrap(), 0x8001_0000);
    assert_eq!(bus.read(0x2004, Width::Word).unwrap(), 0x1001d);
    assert_eq!(bus.read(0x202c, Width::Word).unwrap(), 0x1001c);
    for register in 16..32 {
        cpu.set_register(register, 0);
    }
    cpu.set_register(5, 0);
    cpu.set_register(9, 0x14);
    cpu.resume_at(0xa0);
    kernel.dispatch(&mut cpu, &mut bus).unwrap();
    assert_eq!(cpu.pc(), 0x8001_0000);
    assert_eq!(
        cpu.register(2),
        0,
        "PSX longjmp does not normalize zero to one"
    );
    for register in 16..24 {
        assert_eq!(cpu.register(register), 0x10000 + register as u32);
    }
    assert_eq!(cpu.register(29), 0x1001d);
}

#[test]
fn kernel_registration_and_unsupported_calls_never_become_success_stubs() {
    let mut cpu = Cpu::new(0xb0);
    let mut bus = Ram::default();
    let mut kernel = Kernel::default();
    cpu.set_register(9, 0x19);
    cpu.set_register(4, 0x8000_2000);
    cpu.set_register(31, 0x8001_0000);
    kernel.dispatch(&mut cpu, &mut bus).unwrap();
    assert_eq!(kernel.entry_hook(), Some(0x8000_2000));
    cpu.resume_at(0xb0);
    cpu.set_register(9, 0xff);
    cpu.set_register(2, 0x1234);
    assert!(kernel
        .dispatch(&mut cpu, &mut bus)
        .unwrap_err()
        .to_string()
        .contains("unsupported BIOS"));
    assert_eq!(cpu.register(2), 0x1234);
    assert_eq!(cpu.pc(), 0xb0);
    cpu.resume_at(0xa0);
    cpu.set_register(9, 0x13);
    cpu.set_register(4, 0xffff_fff0);
    assert!(kernel.dispatch(&mut cpu, &mut bus).is_err());
}

#[test]
fn kernel_auto_ack_controls_keep_state_and_return_previous_counter_setting() {
    let mut cpu = Cpu::new(0xc0);
    let mut bus = Ram::default();
    let mut kernel = Kernel::default();
    cpu.set_register(31, 0x8001_0000);
    cpu.set_register(9, 0x0a);
    cpu.set_register(4, 3);
    cpu.set_register(5, 0);
    kernel.dispatch(&mut cpu, &mut bus).unwrap();
    assert_eq!(cpu.register(2), 1);
    assert_eq!(kernel.counter_auto_ack()[3], 0);
    cpu.resume_at(0xc0);
    cpu.set_register(5, 1);
    kernel.dispatch(&mut cpu, &mut bus).unwrap();
    assert_eq!(cpu.register(2), 0);
    cpu.resume_at(0xb0);
    cpu.set_register(9, 0x5b);
    cpu.set_register(4, 0);
    kernel.dispatch(&mut cpu, &mut bus).unwrap();
    assert_eq!(kernel.pad_auto_ack(), 0);
    cpu.resume_at(0xc0);
    cpu.set_register(9, 0x0a);
    cpu.set_register(4, 4);
    assert!(kernel.dispatch(&mut cpu, &mut bus).is_err());
    // Removal of the BIOS CD-ROM driver is still unsupported, not a no-op.
    cpu.resume_at(0xa0);
    cpu.set_register(9, 0x72);
    assert!(kernel.dispatch(&mut cpu, &mut bus).is_err());
}
