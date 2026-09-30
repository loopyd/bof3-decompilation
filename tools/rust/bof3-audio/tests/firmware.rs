use bof3_audio::{
    digest::sha256_hex,
    machine::{
        bus::{Bus, Ram, Width},
        cpu::{Cpu, FaultKind},
        executable::RAM_BYTES,
        firmware::{Image, ROM_BYTES},
        interconnect::Interconnect,
    },
};
fn put(bytes: &mut [u8], at: usize, instructions: &[u32]) {
    for (i, word) in instructions.iter().enumerate() {
        bytes[at + i * 4..at + i * 4 + 4].copy_from_slice(&word.to_le_bytes());
    }
}

#[test]
fn post_display_is_a_byte_write_sink_without_readback_or_irq_effects() {
    let mut bus = Interconnect::from_ram(Ram::default());
    assert_eq!(bus.post_status(), None);
    for (address, value) in [(0x1f80_2041, 15), (0x9f80_2041, 0x123), (0xbf80_2041, 7)] {
        bus.write(address, Width::Byte, value).unwrap();
        assert_eq!(bus.post_status(), Some(value as u8));
        assert!(bus.read(address, Width::Byte).is_err());
        for width in [Width::Half, Width::Word] {
            assert!(bus.write(address, width, 0).is_err());
        }
        assert!(bus.write_masked(address & !3, 0, 2).is_err());
        assert_eq!(bus.post_status(), Some(value as u8));
    }
    for address in [0x1f80_2040, 0x1f80_2042, 0x3f80_2041] {
        assert!(bus.write(address, Width::Byte, 0).is_err());
    }
    assert_eq!(bus.post_status(), Some(7));
    assert_eq!(bus.interrupts().status(), 0);
}

#[test]
fn absent_expansion_rom_requires_configured_window_and_preserves_ram() {
    let mut bus = Interconnect::from_ram(Ram::default());
    assert!(bus.read(0x1f00_0084, Width::Byte).is_err());
    bus.write(0x1f80_1000, Width::Word, 0x1f00_0000).unwrap();
    assert!(bus.read(0x1f00_0084, Width::Byte).is_err());
    bus.write(0x1f80_1008, Width::Word, 0x0013_243f).unwrap();
    let ram_hash = sha256_hex(bus.ram().bytes());
    for base in [0x1f00_0000, 0x9f00_0000, 0xbf00_0000] {
        for (width, expected) in [
            (Width::Byte, 255),
            (Width::Half, 65535),
            (Width::Word, u32::MAX),
        ] {
            for offset in [0, 4, 0x84, 0x80000 - width.bytes() as u32] {
                assert_eq!(bus.read(base + offset, width).unwrap(), expected);
                assert!(bus.write(base + offset, width, 0).is_err());
            }
        }
        assert!(bus.read(base + 1, Width::Half).is_err());
        assert!(bus.read(base + 2, Width::Word).is_err());
        assert!(bus.read(base + 0x80000, Width::Byte).is_err());
        assert!(bus.write_masked(base + 4, 0, 15).is_err());
    }
    assert!(bus.read(0x3f00_0084, Width::Byte).is_err());
    assert_eq!(sha256_hex(bus.ram().bytes()), ram_hash);
}

#[test]
fn image_preserves_all_bytes_and_accepts_only_explicit_aligned_rom_windows() {
    let bytes: Vec<_> = (0..ROM_BYTES).map(|i| (i / 256 + i) as u8).collect();
    let hash = sha256_hex(&bytes);
    let image = Image::from_bytes(bytes.clone()).unwrap();
    assert_eq!(image.bytes(), bytes);
    assert_eq!(image.sha256(), hash);
    for base in [0x1fc0_0000, 0x9fc0_0000, 0xbfc0_0000] {
        for width in [Width::Byte, Width::Half, Width::Word] {
            for offset in [0, 0x120, ROM_BYTES - width.bytes()] {
                let mut expected = [0; 4];
                expected[..width.bytes()].copy_from_slice(&bytes[offset..offset + width.bytes()]);
                assert_eq!(
                    image.read(base + offset as u32, width).unwrap(),
                    u32::from_le_bytes(expected)
                );
            }
        }
        assert!(image.read(base + 1, Width::Half).is_err());
        assert!(image.read(base + 2, Width::Word).is_err());
        assert!(image.read(base - 1, Width::Byte).is_err());
        assert!(image.read(base + ROM_BYTES as u32, Width::Byte).is_err());
    }
    for address in [0x3fc0_0000, 0x7fc0_0000, 0xdfc0_0000, 0xffc0_0000] {
        assert!(Image::offset(address).is_none());
        assert!(image.read(address, Width::Word).is_err());
    }
    for len in [0, ROM_BYTES - 1, ROM_BYTES + 1, 1024 * 1024] {
        assert!(Image::from_bytes(vec![0; len]).is_err());
    }
}

#[test]
fn attached_rom_cannot_be_replaced_or_mutated_and_raw_ram_is_not_device_state() {
    let mut raw = vec![0x5a; RAM_BYTES];
    put(&mut raw, RAM_BYTES - 4, &[0x1234_5678]);
    let ram = Ram::from_bytes(raw.clone()).unwrap();
    assert_eq!(ram.bytes(), raw);
    assert!(Ram::from_bytes(vec![0; RAM_BYTES - 1]).is_err());
    assert!(Ram::from_bytes(vec![0; RAM_BYTES + 1]).is_err());
    let mut bus = Interconnect::from_ram(ram);
    assert!(bus
        .read(0xbfc0_0000, Width::Word)
        .unwrap_err()
        .detail
        .contains("not attached"));
    let mut image = vec![0; ROM_BYTES];
    put(&mut image, 0, &[0x1122_3344]);
    bus.attach_firmware(Image::from_bytes(image).unwrap())
        .unwrap();
    assert!(bus
        .attach_firmware(Image::from_bytes(vec![0; ROM_BYTES]).unwrap())
        .is_err());
    for base in [0x1fc0_0000, 0x9fc0_0000, 0xbfc0_0000] {
        for width in [Width::Byte, Width::Half, Width::Word] {
            assert!(bus.write(base, width, u32::MAX).is_err());
        }
        assert!(bus.write_masked(base, u32::MAX, 15).is_err());
        assert_eq!(bus.read(base, Width::Word).unwrap(), 0x1122_3344);
    }
    assert_eq!(bus.read(0xa01f_fffc, Width::Word).unwrap(), 0x1234_5678);
    assert_eq!(bus.read(0x1f80_1070, Width::Word).unwrap(), 0);
    assert!(bus.read(0x1f80_1800, Width::Byte).is_err());
    bus.write(0xa000_0000, Width::Word, 7).unwrap();
    assert_eq!(bus.read(0x8000_0000, Width::Word).unwrap(), 7);
}

#[test]
fn redux_cpu_reset_seed_runs_retail_bios_memory_configuration() {
    let mut cpu = Cpu::pcsx_redux_reset();
    assert_eq!(cpu.pc(), 0xbfc0_0000);
    assert_eq!(cpu.cop0().status(), 0x1090_0000);
    assert_eq!((cpu.hi(), cpu.lo(), cpu.instructions()), (0, 0, 0));
    assert!((0..32).all(|r| cpu.register(r) == 0));
    let mut ordinary = Cpu::new(0x1000);
    ordinary.cop0_mut().write(12, 0x1090_0000).unwrap();
    assert_eq!(ordinary.cop0().status(), 0x1010_0000);
    assert!(Interconnect::from_ram(Ram::default())
        .read(0x0060_0120, Width::Word)
        .is_err());
    // Initial instructions of the verified US ROM; copied constants/opcodes
    // exercise device stores through CPU execution, not direct fixture writes.
    let mut rom = vec![0; ROM_BYTES];
    put(
        &mut rom,
        0,
        &[
            0x3c08_0013,
            0x3508_243f,
            0x3c01_1f80,
            0xac28_1010,
            0,
            0x2408_0b88,
            0x3c01_1f80,
            0xac28_1060,
            0,
            0x4080_6000, // mtc0 zero,sr (ordinary guest write removes reset bits)
        ],
    );
    let mut bus = Interconnect::from_ram(Ram::default());
    bus.attach_firmware(Image::from_bytes(rom).unwrap())
        .unwrap();
    cpu.run_until(&mut bus, 0xbfc0_0028, 10).unwrap();
    assert_eq!(cpu.cop0().status(), 0);
    assert_eq!(bus.read(0xbf80_1010, Width::Word).unwrap(), 0x0013_243f);
    assert_eq!(bus.read(0x1f80_1060, Width::Word).unwrap(), 0x0b88);
    bus.write(0x8000_0120, Width::Word, 0xaabb_ccdd).unwrap();
    assert_eq!(bus.read(0xa060_0120, Width::Word).unwrap(), 0xaabb_ccdd);
    for (address, value) in [(0x1f80_1010, 0x13243f), (0x1f80_1060, 0xb88)] {
        for width in [Width::Byte, Width::Half] {
            assert!(bus.write(address, width, value).is_err());
            assert!(bus.read(address, width).is_err());
        }
        assert!(bus.write(address, Width::Word, 0).is_err());
        assert_eq!(bus.read(address, Width::Word).unwrap(), value);
    }
    assert!(bus.write(0x1f80_1060, Width::Word, 0x888).is_err());
    assert!(bus.write(0x1f80_1010, Width::Word, 0x0014_243f).is_err());
    bus.write(0xbf80_1020, Width::Word, 0x31125).unwrap();
    assert_eq!(bus.read(0x1f80_1020, Width::Word).unwrap(), 0x1125);
    bus.write(0x1f80_1018, Width::Word, 0x20843).unwrap();
    assert_eq!(bus.read(0xbf80_1018, Width::Word).unwrap(), 0x20843);
    bus.write_masked(0x8060_0120, 0x1122_3344, 5).unwrap();
    assert_eq!(bus.read(0x0020_0120, Width::Word).unwrap(), 0xaa22_cc44);
    for address in [0x0080_0120, 0x8080_0120, 0xa080_0120, 0xc060_0120] {
        assert!(bus.read(address, Width::Word).is_err());
    }
    for (address, value) in [
        (0x1f80_1000, 0x1f00_0000),
        (0x1f80_1004, 0x1f80_2000),
        (0x1f80_1008, 0x0013_243f),
        (0x1f80_100c, 0x3022),
        (0x1f80_101c, 0x70777),
    ] {
        bus.write(address, Width::Word, value).unwrap();
        assert_eq!(bus.read(address | 0xa000_0000, Width::Word).unwrap(), value);
        assert!(bus.write(address, Width::Word, value ^ 0x100).is_err());
        assert_eq!(bus.read(address, Width::Word).unwrap(), value);
    }
    assert_eq!(bus.read(0x1f00_0000, Width::Byte).unwrap(), 255);
    for address in [0x1f08_0000, 0x1f80_2000, 0x1fa0_0000] {
        assert!(bus.read(address, Width::Byte).is_err());
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_BIOS pointing to the verified US SCPH-5501 ROM"]
fn original_us_bios_executes_reset_memory_and_cache_setup_to_spu_setup() {
    let bytes = std::fs::read(std::env::var_os("BOF3_AUDIO_BIOS").unwrap()).unwrap();
    assert_eq!(
        sha256_hex(&bytes),
        "11052b6499e466bbf0a709b1f9cb6834a9418e66680387912451e971cf8a1fef"
    );
    let mut bus = Interconnect::from_ram(Ram::default());
    bus.attach_firmware(Image::from_bytes(bytes).unwrap())
        .unwrap();
    let mut cpu = Cpu::pcsx_redux_reset();
    cpu.run_until(&mut bus, 0xbfc0_0234, 87).unwrap();
    assert_eq!(cpu.instructions(), 87);
    for (port, value) in [
        (0x1000, 0x1f00_0000),
        (0x1004, 0x1f80_2000),
        (0x1008, 0x0013_243f),
        (0x1010, 0x0013_243f),
        (0x1014, 0x2009_31e1),
        (0x1018, 0x0002_0843),
        (0x101c, 0x0007_0777),
        (0x1020, 0x1125),
        (0x1060, 0xb88),
    ] {
        assert_eq!(bus.read(0x1f80_0000 | port, Width::Word).unwrap(), value);
    }
    assert_eq!(cpu.register(1), 0xfffe_0000);
    assert_eq!(cpu.register(9), 0x804);
    assert_eq!(cpu.cop0().status(), 0x1090_0000);
    cpu.run_until(&mut bus, 0xbfc0_1a74, 17378 - 87).unwrap();
    assert_eq!(cpu.instructions(), 17378);
    assert_eq!(cpu.cop0().status(), 0);
    assert_eq!(bus.read(0xfffe_0130, Width::Word).unwrap(), 0x1e988);
    assert_eq!(cpu.register(4), 15);
    assert_eq!(cpu.register(14), 0x1f80_0000);
    assert_eq!(bus.post_status(), None);
    let mut exceptions = 0;
    for _ in 17378..3_000_000 {
        if cpu.pc() == 0x8005_429c {
            break;
        }
        match cpu.step(&mut bus) {
            Ok(_) => (),
            Err(fault) if fault.kind == FaultKind::Syscall => {
                cpu.enter_exception(&fault).unwrap();
                exceptions += 1;
            }
            Err(fault) => panic!("{fault}"),
        }
    }
    assert_eq!(cpu.pc(), 0x8005_429c);
    assert_eq!(exceptions, 1);
    assert_eq!(cpu.instructions(), 2_727_265);
    assert_eq!(bus.post_status(), Some(7));
    assert_eq!(cpu.register(16), 0x1f80_1c00);
    assert_eq!(cpu.register(2), 0xffff);
    let fault = cpu.step(&mut bus).unwrap_err();
    assert!(fault.to_string().contains("SPU voice arithmetic models"));
    assert_eq!(cpu.instructions(), 2_727_265);
}

#[test]
#[ignore = "requires BOF3_AUDIO_BIOS pointing to verified US SCPH-5501 ROM"]
fn original_us_bios_uploads_spu_data_with_explicit_transfer_clock() {
    use bof3_audio::machine::{adsr, spu_clock, spu_reverb, spu_sample, spu_voice_ports};
    let bytes = std::fs::read(std::env::var_os("BOF3_AUDIO_BIOS").unwrap()).unwrap();
    assert_eq!(
        sha256_hex(&bytes),
        "11052b6499e466bbf0a709b1f9cb6834a9418e66680387912451e971cf8a1fef"
    );
    let mut bus = Interconnect::from_ram(Ram::default());
    bus.attach_firmware(Image::from_bytes(bytes).unwrap())
        .unwrap();
    bus.configure_spu_voices(
        spu_sample::Model::EmulatorReference,
        adsr::Model::EmulatorReference,
    )
    .unwrap();
    bus.configure_spu_disable(spu_voice_ports::DisableModel::EmulatorReference)
        .unwrap();
    bus.configure_spu_reverb(spu_reverb::Model::EmulatorReference)
        .unwrap();
    let mut clock = spu_clock::Clock::new(spu_clock::Model::EmulatorReference);
    let mut cpu = Cpu::pcsx_redux_reset();
    let mut exceptions = 0;
    for _ in 0..20_000_000 {
        if cpu.pc() == 0x8005_a4e8 {
            break;
        }
        match cpu.step(&mut bus) {
            Ok(_) => (),
            Err(fault) if fault.kind == FaultKind::Syscall => {
                cpu.enter_exception(&fault).unwrap();
                exceptions += 1;
            }
            Err(fault) => panic!("{fault}"),
        }
        clock.advance(&mut bus, 2).unwrap();
    }
    assert_eq!(cpu.pc(), 0x8005_a4e8);
    assert_eq!(cpu.instructions(), 19_247_628);
    assert_eq!(exceptions, 4);
    assert_eq!(clock.ticks(), 38_495_264);
    assert_eq!(clock.halfwords(), 54_328);
    assert_eq!(bus.spu_transfer().fifo_halfwords(), 0);
    // Regression snapshot of the original ROM's writes under this explicit
    // clock model, not independent hardware or rendered-audio evidence.
    assert_eq!(
        sha256_hex(bus.spu_transfer().ram()),
        "35eff894304fb8c68065a11b9942b685a660fd39cbad4f31815c12afd4332416"
    );
    assert_eq!(bus.post_status(), Some(7));
    assert_eq!(cpu.register(2), 0x1f80_1814);
    assert!(cpu
        .step(&mut bus)
        .unwrap_err()
        .to_string()
        .contains("unimplemented I/O read"));
}

#[test]
fn ram_bios_vector_and_rom_routine_execute_without_hle_interception() {
    let mut ram = Ram::default();
    // Synthetic RAM A0 vector dispatches directly to a supplied ROM function.
    for (i, op) in [0x3c08_bfc0, 0x3508_0200, 0x0100_0008, 0]
        .into_iter()
        .enumerate()
    {
        ram.write(0xa0 + i as u32 * 4, Width::Word, op).unwrap();
    }
    let mut rom = vec![0; ROM_BYTES];
    put(&mut rom, 0x200, &[0x2402_0072, 0x03e0_0008, 0x2442_0001]);
    let mut bus = Interconnect::from_ram(ram);
    bus.attach_firmware(Image::from_bytes(rom).unwrap())
        .unwrap();
    let mut cpu = Cpu::new(0xa0);
    cpu.set_register(31, 0x8000_1000);
    cpu.run_until(&mut bus, 0x8000_1000, 7).unwrap();
    assert_eq!(cpu.register(2), 0x73);
    assert_eq!(cpu.instructions(), 7);
}

#[test]
fn syscall_can_enter_rom_exception_vector_and_return_through_guest_rfe() {
    let mut ram = Ram::default();
    ram.write(0x1000, Width::Word, 0x0000_000c).unwrap();
    let mut rom = vec![0; ROM_BYTES];
    put(
        &mut rom,
        0x180,
        &[
            0x401a_7000, // mfc0 k0, EPC
            0,           // load delay
            0x275a_0004, // advance past syscall
            0x0340_0008, // jr k0
            0x4200_0010, // rfe in jump delay
        ],
    );
    let mut bus = Interconnect::from_ram(ram);
    bus.attach_firmware(Image::from_bytes(rom).unwrap())
        .unwrap();
    let mut cpu = Cpu::new(0x8000_1000);
    cpu.cop0_mut().write(12, (1 << 22) | 0x401).unwrap();
    let fault = cpu.step(&mut bus).unwrap_err();
    assert_eq!(fault.kind, FaultKind::Syscall);
    cpu.enter_exception(&fault).unwrap();
    assert_eq!(cpu.pc(), 0xbfc0_0180);
    cpu.run_until(&mut bus, 0x8000_1004, 5).unwrap();
    assert_eq!(cpu.cop0().status(), (1 << 22) | 0x401);
}
