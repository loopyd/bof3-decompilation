use bof3_audio::machine::{
    bus::{Bus, Width},
    executable::Executable,
    interconnect::Interconnect,
    spu_dma::Channel,
    spu_transfer::{Transfer, RAM_BYTES},
};

fn bus() -> Interconnect {
    let mut bytes = vec![0; 0x804];
    bytes[..8].copy_from_slice(b"PS-X EXE");
    for (at, value) in [(0x10, 0x80010000u32), (0x18, 0x80010000), (0x1c, 4)] {
        bytes[at..at + 4].copy_from_slice(&value.to_le_bytes());
    }
    let mut bus = Interconnect::from_executable(&Executable::from_bytes(bytes).unwrap());
    bus.write(0x1f801014, Width::Word, 0x200931e1).unwrap();
    bus
}

fn write(b: &mut Interconnect, address: u32, value: u32) {
    b.write(address, Width::Word, value).unwrap();
}
fn half(b: &mut Interconnect, address: u32, value: u16) {
    b.write(address, Width::Half, u32::from(value)).unwrap();
}
fn read(b: &mut Interconnect, address: u32) -> u32 {
    b.read(address, Width::Word).unwrap()
}
fn status(b: &mut Interconnect) -> u32 {
    b.read(0x1f801dae, Width::Half).unwrap()
}
fn mode(b: &mut Interconnect, value: u16) {
    half(b, 0x1f801dac, 4);
    half(b, 0x1f801daa, value);
}
fn dma(b: &mut Interconnect, address: u32, blocks: u16, words: u16, control: u32) {
    write(b, 0x1f8010c0, address);
    write(b, 0x1f8010c4, (u32::from(blocks) << 16) | u32::from(words));
    write(b, 0x1f8010c8, control);
}

#[test]
fn manual_fifo_transfer_preserves_order_address_register_and_ram_wrap() {
    let mut b = bus();
    // SB on even SPU addresses writes a halfword; odd SB is ignored.
    b.write(0xbf801da6, Width::Byte, 0xffff).unwrap();
    b.write(0xbf801da7, Width::Byte, 0).unwrap();
    assert_eq!(b.read(0x1f801da6, Width::Half).unwrap(), 0xffff);
    assert_eq!(b.read(0x9f801da7, Width::Byte).unwrap(), 255);
    for n in 0..32 {
        half(&mut b, 0x1f801da8, 0x1000 + n);
    }
    assert!(b.write(0x1f801da8, Width::Half, 999).is_err());
    mode(&mut b, 0x10);
    assert_eq!(status(&mut b) & 0x3f, 0); // Not a scheduled device boundary yet.
    b.service_spu(0).unwrap();
    assert_eq!(status(&mut b), 0x410);
    assert_eq!(b.service_spu(32).unwrap(), 32);
    assert_eq!(status(&mut b), 0x10);
    assert_eq!(b.spu_transfer().fifo_halfwords(), 0);
    assert_eq!(b.spu_transfer().current_address(), 56);
    assert_eq!(b.read(0x1f801da6, Width::Half).unwrap(), 0xffff);
    for n in 0..32usize {
        let at = (RAM_BYTES - 8 + n * 2) % RAM_BYTES;
        assert_eq!(
            &b.spu_transfer().ram()[at..at + 2],
            &(0x1000 + n as u16).to_le_bytes()
        );
    }
    assert!(b.read(0x1f801da8, Width::Half).is_err());
    assert!(b.write(0x1f801da5, Width::Half, 0).is_err());
    assert!(b.write_masked(0x1f801da4, 0, 1).is_err());
}

#[test]
fn dma_write_completion_is_distinct_from_fifo_drain_and_dma_irq_acknowledgment() {
    let mut b = bus();
    for n in 0..32 {
        write(&mut b, 0x10000 + n * 4, 0xa0b00000 + n);
    }
    mode(&mut b, 0x20);
    half(&mut b, 0x1f801da6, 0x1000);
    write(&mut b, 0x1f8010f4, (1 << 23) | (1 << 20));
    half(&mut b, 0x1f801074, 8);
    dma(&mut b, 0x10000, 2, 16, 0x01000201);
    b.service_spu(0).unwrap();
    assert_eq!(b.spu_transfer().fifo_halfwords(), 0); // DPCR disabled.
    assert_eq!(status(&mut b), 0x1a0);
    write(&mut b, 0x1f8010f0, 0x076d4321);
    b.service_spu(0).unwrap();
    assert_eq!(b.spu_transfer().fifo_halfwords(), 32);
    assert_eq!(read(&mut b, 0x1f8010c0), 0x10040);
    assert_eq!(read(&mut b, 0x1f8010c4), 0x10010);
    assert_ne!(read(&mut b, 0x1f8010c8) & (1 << 24), 0);
    assert!(!b.interrupts().pending());
    assert!(b.spu_transfer().ram()[0x8000..0x8080]
        .iter()
        .all(|&x| x == 0));
    b.service_spu(32).unwrap(); // First FIFO drained, second DMA slice queued.
    assert_eq!(read(&mut b, 0x1f8010c4), 16);
    assert_eq!(read(&mut b, 0x1f8010c0), 0x10080);
    assert_eq!(read(&mut b, 0x1f8010c8), 0x201);
    assert_eq!(status(&mut b), 0x420); // SPU still busy after DMA completion.
    assert!(b.interrupts().pending());
    b.service_spu(32).unwrap();
    for n in 0..32usize {
        assert_eq!(
            &b.spu_transfer().ram()[0x8000 + n * 4..0x8004 + n * 4],
            &(0xa0b00000 + n as u32).to_le_bytes()
        );
    }
    assert_eq!(status(&mut b), 0x1a0);
    write(&mut b, 0x1f8010f4, (1 << 23) | (1 << 20) | (1 << 28));
    assert_ne!(b.interrupts().status() & 8, 0); // Controller latch is separate.
    half(&mut b, 0x1f801070, 0);
    assert!(!b.interrupts().pending());
}

#[test]
fn dma_reads_use_full_fifo_requests_and_preserve_little_endian_words() {
    let mut b = bus();
    mode(&mut b, 0x10);
    half(&mut b, 0x1f801da6, 0x200);
    for n in 0..32 {
        half(&mut b, 0x1f801da8, 0x8000 + n);
    }
    b.service_spu(32).unwrap();
    mode(&mut b, 0);
    b.service_spu(0).unwrap();
    half(&mut b, 0x1f801da6, 0x200);
    mode(&mut b, 0x30);
    write(&mut b, 0x1f801014, 0x220931e1);
    write(&mut b, 0x1f8010f0, 0x076d4321);
    dma(&mut b, 0x20002, 1, 16, 0x01000200); // Address low bits ignored on RAM bus only.
    assert_eq!(b.service_spu(31).unwrap(), 31);
    assert_eq!(read(&mut b, 0x20000), 0);
    assert_ne!(read(&mut b, 0x1f8010c8) & (1 << 24), 0);
    b.service_spu(1).unwrap();
    assert_eq!(read(&mut b, 0x1f8010c0), 0x20042);
    for n in 0..16 {
        assert_eq!(
            read(&mut b, 0x20000 + n * 4),
            (0x8001 + n * 2) << 16 | (0x8000 + n * 2)
        );
    }
    assert_eq!(b.spu_transfer().fifo_halfwords(), 0);
    assert_eq!(read(&mut b, 0x1f8010c8), 0x200);
}

#[test]
fn per_slice_interrupts_decrementing_addresses_and_ram_mirrors_are_observable() {
    let mut b = bus();
    write(&mut b, 0x10000, 0x11223344);
    write(&mut b, 0xfffc, 0x55667788);
    mode(&mut b, 0x20);
    write(&mut b, 0x1f8010f0, 0x076d4321);
    write(&mut b, 0x1f8010f4, (1 << 23) | (1 << 20) | (1 << 4));
    dma(&mut b, 0x210000, 2, 1, 0x11000203);
    b.service_spu(0).unwrap();
    assert_eq!(read(&mut b, 0x1f8010c0), 0x20fffc);
    assert_ne!(read(&mut b, 0x1f8010f4) & (1 << 28), 0);
    assert_eq!(read(&mut b, 0x1f8010c8), 0x01000203); // Force-start bit cleared.
    b.service_spu(4).unwrap();
    assert_eq!(
        &b.spu_transfer().ram()[..8],
        &[0x44, 0x33, 0x22, 0x11, 0x88, 0x77, 0x66, 0x55]
    );
    assert_eq!(read(&mut b, 0x1f8010c0), 0x20fff8);
}

#[test]
fn unsupported_execution_and_fifo_capacity_fail_without_false_completion() {
    let mut b = bus();
    for control in [0x01000001, 0x01000401, 0x01000301, 0x21000201] {
        write(&mut b, 0x1f8010c4, 0x10010);
        assert!(b.write(0x1f8010c8, Width::Word, control).is_err());
        assert_eq!(read(&mut b, 0x1f8010c8), 0);
    }
    for size in [0, 17, 65535] {
        write(&mut b, 0x1f8010c4, 0x10000 | size);
        assert!(b.write(0x1f8010c8, Width::Word, 0x01000201).is_err());
    }
    assert!(b.write(0x1f801daa, Width::Half, 0x8040).is_err());
    assert_eq!(b.read(0x1f801daa, Width::Half).unwrap(), 0);
    half(&mut b, 0x1f801dac, 6);
    half(&mut b, 0x1f801daa, 0x20);
    assert!(b.service_spu(100).is_err());
    assert_eq!(b.spu_transfer().current_address(), 0);
    mode(&mut b, 0x20);
    write(&mut b, 0x1f8010f0, 0x076d4321);
    dma(&mut b, 0x800000, 1, 16, 0x01000201);
    assert!(b.service_spu(100).is_err());
    assert_ne!(read(&mut b, 0x1f8010f4) & (1 << 15), 0);
    assert_ne!(read(&mut b, 0x1f8010c8) & (1 << 24), 0);
    assert_eq!(b.spu_transfer().fifo_halfwords(), 0);
    assert!(b.spu_transfer().ram().iter().all(|&v| v == 0));
    let mut t = Transfer::default();
    t.write(12, 4).unwrap();
    t.write(10, 0x10).unwrap();
    t.apply_control().unwrap();
    t.write(8, 42).unwrap();
    assert!(t.write(10, 0).is_err());
    assert_eq!(t.fifo_halfwords(), 1);
    let mut t = Transfer::default();
    t.write(10, 0x30).unwrap();
    assert!(t.write(8, 42).is_err()); // Pending read must not accept write data.
    assert_eq!(t.fifo_halfwords(), 0);
}

#[test]
fn zero_block_count_means_65536_slices_and_cancellation_does_not_complete() {
    let mut c = Channel::default();
    c.write(4, 1).unwrap();
    c.write(8, 0x01000201).unwrap();
    assert_eq!(c.next_word(false), None);
    assert_eq!(c.next_word(true), Some(0));
    assert!(c.finish_word().unwrap());
    assert_eq!(c.read(4).unwrap(), 0xffff0001);
    assert!(c.write(0, 0x10000).is_err());
    c.write(8, 0).unwrap();
    assert!(!c.active());
    assert_eq!(c.next_word(true), None);
    assert!(c.finish_word().is_err());
}

#[test]
fn unstable_read_bus_configuration_and_unsupported_port_accesses_fail() {
    let mut b = bus();
    assert_eq!(read(&mut b, 0xbf801014), 0x200931e1);
    assert!(b.write(0x1f801014, Width::Half, 0x200931e1).is_err());
    assert!(b.write(0x1f801014, Width::Word, 0).is_err());
    assert!(b.read(0x1f801014, Width::Half).is_err());
    assert!(b.write_masked(0x1f801014, 0, 15).is_err());
    assert_eq!(read(&mut b, 0x1f801014), 0x200931e1);
    mode(&mut b, 0x30);
    write(&mut b, 0x1f8010f0, 0x076d4321);
    dma(&mut b, 0x10000, 1, 16, 0x01000200);
    let error = b.service_spu(32).unwrap_err().to_string();
    assert!(error.contains("stable reads require"), "{error}");
    assert_eq!(b.spu_transfer().fifo_halfwords(), 0);
    assert_eq!(read(&mut b, 0x1f8010c4), 0x10010);
    assert_eq!(read(&mut b, 0x1f8010c8), 0x01000200);
    assert_eq!(read(&mut b, 0x1f8010f4), 0);
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_dispatcher_uploads_and_downloads_rounded_blocks_without_code_substitution() {
    use bof3_audio::machine::{cpu::Cpu, profile::Profile};
    let exe = Executable::from_bytes(
        std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").expect("set BOF3_AUDIO_EXE")).unwrap(),
    )
    .unwrap();
    Profile::identify(&exe).unwrap();
    let stack = exe.stack_pointer().unwrap();
    let call = |b: &mut Interconnect, command, arg1, arg2| {
        // exe/slus_004_22@80168960, full EXE offset D2960. Execute the
        // original dispatcher and its original DEV4 helpers, including delays.
        let mut cpu = Cpu::new(0x80168960);
        cpu.set_register(4, command);
        cpu.set_register(5, arg1);
        cpu.set_register(6, arg2);
        cpu.set_register(16, 0x76543210);
        cpu.set_register(29, stack);
        cpu.set_register(31, 0x80001000);
        let instructions = cpu.run_until(b, 0x80001000, 40_000).unwrap();
        assert_eq!(cpu.register(29), stack);
        assert_eq!(cpu.register(16), 0x76543210);
        (cpu.register(2), instructions)
    };
    let mut total_instructions = 0;
    for length in [1u32, 63, 64, 65, 127, 128, 129, 256] {
        for spu_address in [0x20008u32, 0x7fff8] {
            let mut b = Interconnect::from_executable(&exe);
            write(&mut b, 0x1f801014, 0x200931e1);
            half(&mut b, 0x1f801dac, 4);
            write(&mut b, 0x1f8010f0, 0x076d4321);
            write(&mut b, 0x1f8010f4, (1 << 23) | (1 << 20));
            let rounded = length.div_ceil(64) * 64;
            let payload: Vec<u8> = (0..rounded).map(|n| (n * 37 + length) as u8).collect();
            for (n, word) in payload.as_chunks::<4>().0.iter().enumerate() {
                write(&mut b, 0x10000 + n as u32 * 4, u32::from_le_bytes(*word));
            }
            for (command, arg1, arg2) in [(2, spu_address, 0), (1, 0, 0), (3, 0x80010000, length)] {
                let (result, count) = call(&mut b, command, arg1, arg2);
                assert_eq!(result, 0);
                total_instructions += count;
            }
            assert_eq!(read(&mut b, 0x80183b00), 0);
            assert_eq!(read(&mut b, 0x80183b04), 0x80010000);
            assert_eq!(read(&mut b, 0x80183b08), rounded / 64);
            assert_eq!(read(&mut b, 0x1f801014), 0x200931e1);
            assert_eq!(read(&mut b, 0x1f8010c4), (rounded / 64) << 16 | 16);
            assert_eq!(read(&mut b, 0x1f8010c8), 0x01000201);
            assert_eq!(
                b.service_spu(rounded as usize / 2).unwrap(),
                rounded as usize / 2
            );
            assert_eq!(read(&mut b, 0x1f8010c0), 0x10000 + rounded);
            assert_eq!(read(&mut b, 0x1f8010c8), 0x201);
            assert_ne!(read(&mut b, 0x1f8010f4) & (1 << 28), 0);
            for (n, &byte) in payload.iter().enumerate() {
                assert_eq!(
                    b.spu_transfer().ram()[(spu_address as usize + n) % RAM_BYTES],
                    byte
                );
            }
            half(&mut b, 0x1f801daa, 0);
            b.service_spu(0).unwrap();
            for (command, arg1, arg2) in [(2, spu_address, 0), (0, 0, 0), (3, 0x80020000, length)] {
                let (result, count) = call(&mut b, command, arg1, arg2);
                assert_eq!(result, 0);
                total_instructions += count;
            }
            assert_eq!(read(&mut b, 0x80183b00), 1);
            assert_eq!(read(&mut b, 0x1f801014), 0x220931e1);
            assert_eq!(read(&mut b, 0x1f8010c8), 0x01000200);
            b.service_spu(rounded as usize / 2).unwrap();
            assert_eq!(read(&mut b, 0x1f8010c8), 0x200);
            assert_eq!(read(&mut b, 0x1f8010c0), 0x20000 + rounded);
            assert_eq!(
                &b.ram().bytes()[0x20000..0x20000 + rounded as usize],
                payload.as_slice()
            );
            assert_eq!(b.spu_transfer().fifo_halfwords(), 0);
        }
    }
    // Original timeout returns -2 without starting DMA when control never matches.
    let mut b = Interconnect::from_executable(&exe);
    write(&mut b, 0x80183b00, 0);
    let (result, count) = call(&mut b, 3, 0x80010000, 64);
    assert_eq!(result, 0xffff_fffe);
    assert_eq!(read(&mut b, 0x1f8010c8), 0);
    eprintln!("Original US transfer dispatcher: 16 upload/download pairs, {total_instructions} instructions; original timeout {count} instructions; explicit device service, no bus timing or PCM validation");
}
