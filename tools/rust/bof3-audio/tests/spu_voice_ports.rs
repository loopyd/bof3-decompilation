use bof3_audio::machine::{
    adsr,
    bus::{Bus, Width},
    cpu::Cpu,
    executable::Executable,
    interconnect::Interconnect,
    profile::Profile,
    spu_sample::Model,
};

fn executable() -> Executable {
    let mut bytes = vec![0; 0x804];
    bytes[..8].copy_from_slice(b"PS-X EXE");
    for (at, value) in [(0x10, 0x80010000u32), (0x18, 0x80010000), (0x1c, 4)] {
        bytes[at..at + 4].copy_from_slice(&value.to_le_bytes());
    }
    Executable::from_bytes(bytes).unwrap()
}
fn half(bus: &mut Interconnect, address: u32, value: u16) {
    bus.write(address, Width::Half, u32::from(value)).unwrap();
}
fn configure(bus: &mut Interconnect) {
    bus.configure_spu_voices(Model::Published, adsr::Model::Published)
        .unwrap();
    // Upload a real ADPCM block through the existing manual FIFO path.
    half(bus, 0x1f801da6, 0);
    half(bus, 0x1f801dac, 4);
    half(bus, 0x1f801daa, 0xc010);
    half(bus, 0x1f801da8, 0x0700);
    for _ in 0..7 {
        half(bus, 0x1f801da8, 0x1111);
    }
    assert_eq!(bus.service_spu(8).unwrap(), 8);
    for v in 0..24 {
        let base = 0x1f801c00 + v * 16;
        half(bus, base, 0x3fff);
        half(bus, base + 2, 0x3fff);
        half(bus, base + 4, 0x1000);
        half(bus, base + 8, 0x000f);
        half(bus, base + 10, 0x1fc0);
    }
}

#[test]
fn endx_write_readback_does_not_modify_voice_loop_end_latches() {
    let mut bus = Interconnect::from_executable(&executable());
    configure(&mut bus);
    bus.write(0x1f801d9c, Width::Word, 0x00800002).unwrap();
    assert_eq!(bus.read(0x1f801d9c, Width::Word).unwrap(), 0x00800002);
    bus.step_spu_voices().unwrap();
    assert_eq!(bus.read(0x1f801d9c, Width::Word).unwrap(), 0);
    half(&mut bus, 0x1f801d88, 2);
    for _ in 0..28 {
        bus.step_spu_voices().unwrap();
    }
    assert_eq!(bus.read(0x1f801d9c, Width::Word).unwrap(), 2);
    half(&mut bus, 0x1f801d9c, 0);
    half(&mut bus, 0x1f801d9e, 0x80);
    assert_eq!(bus.read(0x1f801d9c, Width::Word).unwrap(), 0x800000);
    assert!(bus.write(0x1f801d9d, Width::Half, 0).is_err());
    assert_eq!(bus.read(0x1f801d9c, Width::Word).unwrap(), 0x800000);
    bus.step_spu_voices().unwrap();
    assert_eq!(bus.read(0x1f801d9c, Width::Word).unwrap(), 2);
}

#[test]
fn key_readback_survives_frames_without_retriggering_consumed_keys() {
    let mut bus = Interconnect::from_executable(&executable());
    configure(&mut bus);
    bus.write(0xbf801d88, Width::Word, 0xff800003).unwrap();
    let first = bus.step_spu_voices().unwrap();
    for voice in [0, 1, 23] {
        assert_eq!(first.voices[voice].fetched_address, Some(0));
    }
    assert_eq!(bus.read(0x9f801d88, Width::Word).unwrap(), 0xff800003);
    half(&mut bus, 0x1f801d88, 4);
    assert_eq!(bus.read(0x1f801d88, Width::Word).unwrap(), 0xff800004);
    let next = bus.step_spu_voices().unwrap();
    assert_eq!(next.voices[2].fetched_address, Some(0));
    for voice in [0, 1, 23] {
        assert_eq!(next.voices[voice].fetched_address, None);
    }
    bus.write(0x1f801d8c, Width::Word, 0xaa800007).unwrap();
    bus.step_spu_voices().unwrap();
    assert_eq!(bus.read(0x1f801d8c, Width::Word).unwrap(), 0xaa800007);
    assert_eq!(bus.read(0x1f801d8f, Width::Byte).unwrap(), 0xaa);
    assert!(bus.write(0x1f801d8d, Width::Half, 0).is_err());
    assert!(bus.write_masked(0x1f801d8c, 0, 15).is_err());
    assert_eq!(bus.read(0x1f801d8c, Width::Word).unwrap(), 0xaa800007);
    half(&mut bus, 0x1f801d8c, 0);
    assert_eq!(bus.read(0x1f801d8c, Width::Word).unwrap(), 0xaa800000);
    assert_eq!(bus.read(0x1f801d88, Width::Word).unwrap(), 0xff800004);
}

#[test]
fn mmio_requires_models_handles_widths_and_applies_pending_keys_once() {
    let mut bus = Interconnect::from_executable(&executable());
    assert!(bus.read(0x1f801c00, Width::Half).is_err());
    assert!(bus.step_spu_voices().is_err());
    configure(&mut bus);
    assert!(bus
        .configure_spu_voices(Model::Published, adsr::Model::Published)
        .is_err());
    bus.write(0xbf801c00, Width::Byte, 0x1234).unwrap();
    bus.write(0xbf801c01, Width::Byte, 0xff).unwrap();
    assert_eq!(bus.read(0x9f801c01, Width::Byte).unwrap(), 0x12);
    assert_eq!(bus.read(0x1f801c00, Width::Word).unwrap(), 0x3fff1234);
    assert!(bus.write(0x1f801c02, Width::Word, 0).is_err());
    half(&mut bus, 0x1f801d88, 1);
    half(&mut bus, 0x1f801d88, 2); // latest low-half latch, not an accumulated mask
    half(&mut bus, 0x1f801d8a, 0xff80); // only eight high bits belong to voices
    half(&mut bus, 0x1f801d9a, 0x80);
    let first = bus.step_spu_voices().unwrap();
    assert_eq!(first.voices[0].fetched_address, None);
    assert_eq!(first.voices[1].fetched_address, Some(0));
    assert_eq!(first.voices[23].fetched_address, Some(0));
    assert_eq!(first.reverb_mask, 1 << 23);
    for _ in 1..28 {
        bus.step_spu_voices().unwrap();
    }
    assert_eq!(bus.read(0x1f801d9c, Width::Word).unwrap(), (1 << 23) | 2);
    assert_eq!(bus.read(0x1f801d88, Width::Word).unwrap(), 0xff800002);
    half(&mut bus, 0x1f801d94, 1);
    assert!(bus
        .step_spu_voices()
        .unwrap_err()
        .to_string()
        .contains("noise"));
    bus.write(0x1f801d9c, Width::Half, 0).unwrap();
    assert!(bus.write(0x1f801daa, Width::Half, 0).is_err());
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_register_flush_drives_all_twenty_four_voice_bits_and_reverb_routes() {
    let exe =
        Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    Profile::identify(&exe).unwrap();
    for v in 0..24 {
        let bit = 1u32 << v;
        let mut bus = Interconnect::from_executable(&exe);
        configure(&mut bus);
        assert_eq!(bus.read(0x80184458, Width::Word).unwrap(), 0x1f801c00);
        // Synthetic staged state; original final flush instructions are unchanged.
        for (address, value) in [
            (0x80190c58, 0),
            (0x80190c5a, 0),
            (0x8018db50, bit as u16),
            (0x8018db52, (bit >> 16) as u16),
            (0x8018db54, bit as u16),
            (0x8018db56, (bit >> 16) as u16),
        ] {
            half(&mut bus, address, value);
        }
        // exe/slus_004_22 runtime 0x801709c4..0x80170a34, full EXE 0xda9c4.
        let mut cpu = Cpu::new(0x801709c4);
        cpu.run_until(&mut bus, 0x80170a34, 100).unwrap();
        for address in [0x80190c58, 0x80190c5a, 0x8018db50, 0x8018db52] {
            assert_eq!(bus.read(address, Width::Half).unwrap(), 0);
        }
        assert_eq!(bus.read(0x1f801d98, Width::Word).unwrap(), bit);
        let first = bus.step_spu_voices().unwrap();
        assert_eq!(first.reverb_mask, bit);
        for (i, output) in first.voices.iter().enumerate() {
            assert_eq!(output.fetched_address, (i == v).then_some(0));
            assert_eq!(output.mono, 0); // key-on starts with zero envelope
        }
        let mut signal = false;
        for _ in 1..28 {
            signal |= bus.step_spu_voices().unwrap().voices[v].mono != 0;
        }
        assert!(signal);
        assert_eq!(bus.read(0x1f801d9c, Width::Word).unwrap(), bit);
    }
}
