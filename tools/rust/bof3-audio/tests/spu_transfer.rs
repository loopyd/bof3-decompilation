use bof3_audio::machine::{
    bus::{Bus, Width},
    image::executable::Executable,
    interconnect::Interconnect,
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
    bus.write(0x1f801dac, Width::Half, 4).unwrap();
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
fn manual_ports_preserve_order_advance_tsa_and_wrap_in_all_modes() {
    let mut b = bus();
    for control in [0, 0x10, 0x20, 0x30] {
        mode(&mut b, control);
        b.write(0xbf801da6, Width::Byte, 0xffff).unwrap();
        b.write(0xbf801da7, Width::Byte, 0).unwrap();
        assert_eq!(b.read(0x9f801da7, Width::Byte).unwrap(), 255);
        for n in 0..64 {
            half(&mut b, 0x1f801da8, 0x1000 + n);
        }
        assert_eq!(status(&mut b), u32::from(control));
        assert_eq!(b.spu_transfer().current_address(), 120);
        assert_eq!(b.read(0x1f801da6, Width::Half).unwrap(), 15);
        for n in 0..64usize {
            let at = (RAM_BYTES - 8 + n * 2) % RAM_BYTES;
            assert_eq!(
                &b.spu_transfer().ram()[at..at + 2],
                &(0x1000 + n as u16).to_le_bytes()
            );
        }
        half(&mut b, 0x1f801da6, 0xffff);
        for n in 0..64 {
            assert_eq!(b.read(0x1f801da8, Width::Half).unwrap(), 0x1000 + n);
        }
        assert_eq!(b.read(0x1f801da6, Width::Half).unwrap(), 15);
    }
    assert!(b.write(0x1f801da5, Width::Half, 0).is_err());
    assert!(b.write_masked(0x1f801da4, 0, 1).is_err());
}

#[test]
fn dma_write_payload_precedes_busy_completion_and_irq_acknowledgment() {
    let mut b = bus();
    for n in 0..32 {
        write(&mut b, 0x10000 + n * 4, 0xa0b00000 + n);
    }
    mode(&mut b, 0);
    half(&mut b, 0x1f801da6, 0x1000);
    write(&mut b, 0x1f8010f4, (1 << 23) | (1 << 20));
    half(&mut b, 0x1f801074, 8);
    dma(&mut b, 0x10000, 2, 16, 0x01000201);
    assert!(b.spu_transfer().ram()[0x8000..0x8080]
        .iter()
        .all(|&x| x == 0));
    write(&mut b, 0x1f8010f0, 0x076d4321);
    b.advance_devices(1, true).unwrap().require_ready().unwrap();
    assert_eq!(read(&mut b, 0x1f8010c0), 0x10000); // DPCR does not restart DMA.
    write(&mut b, 0x1f8010c8, 0x01000201);
    assert_eq!(read(&mut b, 0x1f8010c0), 0x10080);
    assert_eq!(read(&mut b, 0x1f8010c4), 16);
    assert_eq!(read(&mut b, 0x1f8010c8), 0x01000201);
    assert_eq!(status(&mut b), 0);
    for n in 0..32usize {
        assert_eq!(
            &b.spu_transfer().ram()[0x8000 + n * 4..0x8004 + n * 4],
            &(0xa0b00000 + n as u32).to_le_bytes()
        );
    }
    b.advance_devices(100, false)
        .unwrap()
        .require_ready()
        .unwrap();
    assert!(!b.interrupts().pending());
    assert_ne!(read(&mut b, 0x1f8010c8) & (1 << 24), 0);
    b.advance_devices(101, true)
        .unwrap()
        .require_ready()
        .unwrap();
    assert_eq!(read(&mut b, 0x1f8010c8), 0x201);
    assert!(b.interrupts().pending());
    write(&mut b, 0x1f8010f4, (1 << 23) | (1 << 20) | (1 << 28));
    assert_ne!(b.interrupts().status() & 8, 0);
    half(&mut b, 0x1f801070, 0);
    assert!(!b.interrupts().pending());
}

#[test]
fn dma_reads_copy_immediately_and_preserve_little_endian_halfwords() {
    let mut b = bus();
    mode(&mut b, 0x10);
    half(&mut b, 0x1f801da6, 0x200);
    for n in 0..32 {
        half(&mut b, 0x1f801da8, 0x8000 + n);
    }
    half(&mut b, 0x1f801da6, 0x200);
    mode(&mut b, 0);
    write(&mut b, 0x1f801014, 0x220931e1);
    write(&mut b, 0x1f8010f0, 0x076d4321);
    dma(&mut b, 0x20002, 1, 16, 0x01000200);
    assert_eq!(read(&mut b, 0x1f8010c0), 0x20040);
    for n in 0..16 {
        assert_eq!(
            read(&mut b, 0x20000 + n * 4),
            (0x8001 + n * 2) << 16 | (0x8000 + n * 2)
        );
    }
    assert_eq!(b.read(0x1f801da6, Width::Half).unwrap(), 0x208);
    assert_eq!(read(&mut b, 0x1f8010c8), 0x01000200);
    b.advance_devices(9, true).unwrap().require_ready().unwrap();
    assert_eq!(read(&mut b, 0x1f8010c8), 0x200);
}

#[test]
fn dma_ram_mirrors_and_halfword_observations_preserve_order() {
    let mut b = bus();
    write(&mut b, 0x10000, 0x11223344);
    write(&mut b, 0x10004, 0x55667788);
    mode(&mut b, 0x30); // Native payload copies are independent of SPUCNT mode.
    write(&mut b, 0x1f8010f0, 0x076d4321);
    write(&mut b, 0x1f8010f4, (1 << 23) | (1 << 20) | (1 << 4));
    b.enable_dma_observation(4).unwrap();
    dma(&mut b, 0x210002, 2, 1, 0x01000201);
    assert_eq!(read(&mut b, 0x1f8010c0), 0x210008);
    assert_eq!(read(&mut b, 0x1f8010f4) & (1 << 28), 0);
    assert_eq!(
        &b.spu_transfer().ram()[..8],
        &[0x44, 0x33, 0x22, 0x11, 0x88, 0x77, 0x66, 0x55]
    );
    let observation = b.take_dma_observation().unwrap();
    assert!(!observation.overflowed);
    assert_eq!(observation.accesses.len(), 4);
    for (index, access) in observation.accesses.iter().enumerate() {
        assert_eq!(access.width, Width::Half);
        assert_eq!(access.address, 0x210000 + index as u32 * 2);
        assert_eq!(access.physical_address, 0x10000 + index as u32 * 2);
    }
    b.advance_devices(2, true).unwrap().require_ready().unwrap();
    assert_ne!(read(&mut b, 0x1f8010f4) & (1 << 28), 0);
}

#[test]
fn unsupported_dma_controls_and_payload_ranges_fail_before_copying() {
    let mut b = bus();
    for control in [0x01000001, 0x01000401, 0x01000301, 0x21000201, 0x11000203] {
        write(&mut b, 0x1f8010c4, 0x10010);
        assert!(b.write(0x1f8010c8, Width::Word, control).is_err());
        assert_eq!(read(&mut b, 0x1f8010c8), 0);
    }
    assert!(b.write(0x1f801daa, Width::Half, 0x8040).is_err());
    assert_eq!(b.read(0x1f801daa, Width::Half).unwrap(), 0);
    half(&mut b, 0x1f801dac, 6);
    assert!(b.write(0x1f801da8, Width::Half, 42).is_err());
    assert_eq!(b.spu_transfer().current_address(), 0);
    mode(&mut b, 0);
    write(&mut b, 0x1f8010f0, 0x076d4321);
    write(&mut b, 0x1f8010c0, 0x1ffffc);
    write(&mut b, 0x1f8010c4, 0x10002);
    assert!(b.write(0x1f8010c8, Width::Word, 0x01000201).is_err());
    assert_eq!(read(&mut b, 0x1f8010c8), 0);
    assert!(b.spu_transfer().ram().iter().all(|&v| v == 0));
    let mut t = Transfer::default();
    t.write(12, 4).unwrap();
    t.write(10, 0x30).unwrap();
    t.write(8, 42).unwrap();
    t.write(10, 0).unwrap();
    t.write(6, 0).unwrap();
    assert_eq!(t.read(8).unwrap(), 42);
}

#[test]
fn zero_block_count_copies_nothing_but_advances_native_registers() {
    let mut b = bus();
    write(&mut b, 0x1f8010f0, 0x076d4321);
    half(&mut b, 0x1f801da6, 0x1000);
    dma(&mut b, 0x10000, 0, 1, 0x01000201);
    assert_eq!(read(&mut b, 0x1f8010c0), 0x50000);
    assert_eq!(read(&mut b, 0x1f8010c4), 1);
    assert_eq!(b.spu_transfer().current_address(), 0x8000);
    write(&mut b, 0x1f8010c8, 0);
    assert_eq!(read(&mut b, 0x1f8010c8), 0);
    b.advance_devices(1, true).unwrap().require_ready().unwrap();
    assert_eq!(read(&mut b, 0x1f8010c8), 0);
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
    write(&mut b, 0x1f8010c0, 0x10000);
    write(&mut b, 0x1f8010c4, 0x10010);
    let error = b
        .write(0x1f8010c8, Width::Word, 0x01000200)
        .unwrap_err()
        .to_string();
    assert!(error.contains("stable reads require"), "{error}");
    assert_eq!(read(&mut b, 0x1f8010c4), 0x10010);
    assert_eq!(read(&mut b, 0x1f8010c8), 0);
    assert_eq!(read(&mut b, 0x1f8010f4), 0);
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_dispatcher_uploads_and_downloads_rounded_blocks_without_code_substitution() {
    use bof3_audio::machine::{cpu::Cpu, image::profile::Profile};
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
            assert_eq!(read(&mut b, 0x1f8010c4), 16);
            assert_eq!(read(&mut b, 0x1f8010c8), 0x01000201);
            b.advance_devices(u64::from(rounded), true)
                .unwrap()
                .require_ready()
                .unwrap();
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
            for (command, arg1, arg2) in [(2, spu_address, 0), (0, 0, 0), (3, 0x80020000, length)] {
                let (result, count) = call(&mut b, command, arg1, arg2);
                assert_eq!(result, 0);
                total_instructions += count;
            }
            assert_eq!(read(&mut b, 0x80183b00), 1);
            assert_eq!(read(&mut b, 0x1f801014), 0x220931e1);
            assert_eq!(read(&mut b, 0x1f8010c8), 0x01000200);
            b.advance_devices(u64::from(rounded) * 2, true)
                .unwrap()
                .require_ready()
                .unwrap();
            assert_eq!(read(&mut b, 0x1f8010c8), 0x200);
            assert_eq!(read(&mut b, 0x1f8010c0), 0x20000 + rounded);
            assert_eq!(
                &b.ram().bytes()[0x20000..0x20000 + rounded as usize],
                payload.as_slice()
            );
        }
    }
    // Original timeout returns -2 without starting DMA when control never matches.
    let mut b = Interconnect::from_executable(&exe);
    write(&mut b, 0x80183b00, 0);
    let (result, count) = call(&mut b, 3, 0x80010000, 64);
    assert_eq!(result, 0xffff_fffe);
    assert_eq!(read(&mut b, 0x1f8010c8), 0);
    eprintln!("Original US transfer dispatcher: 16 upload/download pairs, {total_instructions} instructions; original timeout {count} instructions; immediate payload and explicit branch callbacks, no full CPU timing or PCM validation");
}
