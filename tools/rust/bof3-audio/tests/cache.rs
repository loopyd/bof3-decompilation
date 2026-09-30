use bof3_audio::machine::{
    bus::{Bus, Ram, Width},
    cpu::Cpu,
    interconnect::Interconnect,
};

const BCC: u32 = 0xfffe_0130;

fn bus() -> Interconnect {
    let mut bus = Interconnect::from_ram(Ram::default());
    bus.write(BCC, Width::Word, 0x1e988).unwrap();
    bus
}

fn words(bus: &mut Interconnect, address: u32, words: &[u32]) {
    for (i, word) in words.iter().enumerate() {
        bus.write(address + i as u32 * 4, Width::Word, *word)
            .unwrap();
    }
}

#[test]
fn instructions_stay_cached_across_ram_stores_and_aliases_until_tag_flush() {
    let mut bus = bus();
    words(&mut bus, 0x1000, &[0x2402_0001, 0, 0, 0]); // addiu v0,zero,1
    let mut cpu = Cpu::new(0x8000_1000);
    cpu.step(&mut bus).unwrap();
    assert_eq!(cpu.register(2), 1);
    bus.write(0xa000_1000, Width::Word, 0x2402_0002).unwrap();
    assert_eq!(bus.read(0x1000, Width::Word).unwrap(), 0x2402_0002);
    cpu.resume_at(0x1000);
    cpu.step(&mut bus).unwrap();
    assert_eq!(cpu.register(2), 1);
    cpu.resume_at(0xa000_1000);
    cpu.step(&mut bus).unwrap();
    assert_eq!(cpu.register(2), 2);
    bus.write(BCC, Width::Word, 0x804).unwrap();
    bus.set_cache_isolation(true).unwrap();
    bus.write(0x8000_1000, Width::Word, 0).unwrap();
    bus.set_cache_isolation(false).unwrap();
    bus.write(BCC, Width::Word, 0x1e988).unwrap();
    cpu.resume_at(0x1000);
    cpu.step(&mut bus).unwrap();
    assert_eq!(cpu.register(2), 2);
}

#[test]
fn isolated_tag_and_code_access_leave_ram_untouched() {
    let mut bus = bus();
    words(&mut bus, 0x2000, &[0x1122_3344, 2, 3, 4]);
    assert_eq!(bus.fetch(0x2000).unwrap(), 0x1122_3344);
    bus.write(BCC, Width::Word, 0x804).unwrap();
    bus.set_cache_isolation(true).unwrap();
    assert_eq!(bus.read(0x8000_2000, Width::Word).unwrap() & 31, 31);
    assert_eq!(bus.read(0x3000, Width::Word).unwrap() & 31, 15);
    bus.write(0x2000, Width::Word, 0).unwrap();
    assert_eq!(bus.read(0x2000, Width::Word).unwrap() & 31, 16);
    bus.write(BCC, Width::Word, 0x800).unwrap();
    assert_eq!(bus.read(0x2000, Width::Word).unwrap(), 0x1122_3344);
    bus.write(0x2000, Width::Word, 0x5566_7788).unwrap();
    assert_eq!(bus.read(0x2000, Width::Word).unwrap(), 0x5566_7788);
    assert_eq!(bus.read(0xa000_2000, Width::Word).unwrap(), 0x1122_3344);
    bus.write(0xa000_2004, Width::Word, 0x99).unwrap();
    for width in [Width::Byte, Width::Half] {
        assert!(bus.write(0x2000, width, 0).is_err());
        assert!(bus.read(0x2000, width).is_err());
    }
    assert!(bus.write_masked(0x2000, 0, 1).is_err());
    bus.write(BCC, Width::Word, 0x804).unwrap();
    bus.write(0x2000, Width::Word, 15).unwrap();
    bus.set_cache_isolation(false).unwrap();
    assert_eq!(bus.fetch(0x2000).unwrap(), 0x5566_7788);
    assert_eq!(bus.read(0x2000, Width::Word).unwrap(), 0x1122_3344);
    assert_eq!(bus.read(0x2004, Width::Word).unwrap(), 0x99);
}

#[test]
fn refill_starts_at_requested_word_and_invalid_matching_word_refills_all() {
    let mut bus = bus();
    words(&mut bus, 0x2000, &[1, 2, 3, 4]);
    assert_eq!(bus.fetch(0x2008).unwrap(), 3);
    words(&mut bus, 0x2000, &[11, 12, 13, 14]);
    assert_eq!(bus.fetch(0x200c).unwrap(), 4);
    assert_eq!(bus.fetch(0x2000).unwrap(), 11);
    assert_eq!(bus.fetch(0x2008).unwrap(), 13);
    bus.write(BCC, Width::Word, 0x800).unwrap(); // two-word burst on word-zero miss
    words(&mut bus, 0x3000, &[21, 22, 23, 24]);
    assert_eq!(bus.fetch(0x3000).unwrap(), 21);
    words(&mut bus, 0x3000, &[31, 32, 33, 34]);
    assert_eq!(bus.fetch(0x3004).unwrap(), 22);
    assert_eq!(bus.fetch(0x3008).unwrap(), 33);
    assert_eq!(bus.fetch(0x3000).unwrap(), 31);
}

#[test]
fn control_and_scratchpad_modes_reject_unimplemented_access_without_mutation() {
    let mut bus = bus();
    bus.write(0x1f80_0000, Width::Word, 0xabcdef).unwrap();
    assert!(bus.fetch(0x1f80_0000).is_err());
    assert!(bus.fetch(0x1f80_1070).is_err());
    for width in [Width::Byte, Width::Half] {
        assert!(bus.write(BCC, width, 0).is_err());
        assert!(bus.read(BCC, width).is_err());
    }
    assert!(bus.write(BCC, Width::Word, 0x1e888).is_err());
    assert_eq!(bus.read(BCC, Width::Word).unwrap(), 0x1e988);
    bus.write(BCC, Width::Word, 0x804).unwrap();
    assert!(bus.read(0x1f80_0000, Width::Word).is_err());
    assert!(bus.write(0x1f80_0000, Width::Word, 0).is_err());
    assert!(bus.write_masked(0x1f80_0000, 0, 15).is_err());
    bus.write(BCC, Width::Word, 0x1e988).unwrap();
    assert_eq!(bus.read(0x9f80_0000, Width::Word).unwrap(), 0xabcdef);
}

#[test]
fn cpu_isolation_is_confined_to_cpu_transactions_even_after_faults() {
    let mut bus = bus();
    // sw t0,0(t1); illegal opcode. Execute through the uncached RAM alias.
    words(&mut bus, 0x1000, &[0xad28_0000, 0xffff_ffff]);
    bus.write(0x2000, Width::Word, 0x1234).unwrap();
    bus.write(BCC, Width::Word, 0x800).unwrap();
    let mut cpu = Cpu::new(0xa000_1000);
    cpu.set_register(8, 0x5678);
    cpu.set_register(9, 0x2000);
    cpu.cop0_mut().write(12, 1 << 16).unwrap();
    cpu.step(&mut bus).unwrap();
    assert_eq!(bus.read(0x2000, Width::Word).unwrap(), 0x1234);
    assert!(cpu.step(&mut bus).is_err());
    assert_eq!(bus.read(0x2000, Width::Word).unwrap(), 0x1234);
    bus.set_cache_isolation(true).unwrap();
    assert_eq!(bus.read(0x2000, Width::Word).unwrap(), 0x5678);
}

#[test]
#[ignore = "requires BOF3_AUDIO_BIOS pointing to verified US SCPH-5501 ROM"]
fn original_bios_clears_dirty_cache_without_clearing_main_ram() {
    use bof3_audio::{digest::sha256_hex, machine::firmware::Image};
    let rom = std::fs::read(std::env::var_os("BOF3_AUDIO_BIOS").unwrap()).unwrap();
    assert_eq!(
        sha256_hex(&rom),
        "11052b6499e466bbf0a709b1f9cb6834a9418e66680387912451e971cf8a1fef"
    );
    let ram = vec![0x5a; 2 * 1024 * 1024];
    let mut bus = Interconnect::from_ram(Ram::from_bytes(ram.clone()).unwrap());
    bus.attach_firmware(Image::from_bytes(rom).unwrap())
        .unwrap();
    bus.write(BCC, Width::Word, 0x800).unwrap();
    bus.set_cache_isolation(true).unwrap();
    for address in (0x4000..0x5000).step_by(4) {
        bus.write(address, Width::Word, 0xaaaa_aaaa).unwrap();
    }
    bus.write(BCC, Width::Word, 0x804).unwrap();
    for address in (0x4000..0x5000).step_by(16) {
        bus.write(address, Width::Word, 15).unwrap();
    }
    bus.set_cache_isolation(false).unwrap();
    let mut cpu = Cpu::pcsx_redux_reset();
    cpu.run_until(&mut bus, 0xbfc0_0368, 1528).unwrap();
    assert_eq!(cpu.instructions(), 1528);
    assert_eq!(bus.ram().bytes(), ram);
    assert_eq!(cpu.cop0().status(), 0);
    assert_eq!(bus.read(BCC, Width::Word).unwrap(), 0x1e988);
    bus.write(BCC, Width::Word, 0x804).unwrap();
    bus.set_cache_isolation(true).unwrap();
    for address in (0..4096).step_by(16) {
        assert_eq!(bus.read(address, Width::Word).unwrap() & 15, 0);
    }
    bus.write(BCC, Width::Word, 0x800).unwrap();
    for address in (0..4096).step_by(4) {
        assert_eq!(bus.read(address, Width::Word).unwrap(), 0);
    }
}
