//! Original US low-level SPU initialization, with explicit untimed device service.
use bof3_audio::machine::{
    adsr,
    bus::{Bus, Width},
    cpu::Cpu,
    executable::Executable,
    interconnect::Interconnect,
    profile::Profile,
    spu_sample,
    spu_voice_ports::DisableModel,
};

fn initialize(bus: &mut Interconnect, stack: u32, argument: u32) -> u32 {
    let mut cpu = Cpu::new(0x8016_8374);
    cpu.set_register(4, argument);
    cpu.set_register(16, 0x1234_5678);
    cpu.set_register(17, 0x8765_4321);
    cpu.set_register(29, stack);
    cpu.set_register(31, 0x8000_1000);
    let mut count = 0;
    while cpu.pc() != 0x8000_1000 {
        assert!(
            count < 50_000,
            "initializer safety limit at {:08x}",
            cpu.pc()
        );
        cpu.step(bus)
            .unwrap_or_else(|e| panic!("instruction {count}: {e}"));
        // Deliberately transaction-based: not one hardware transfer per CPU
        // instruction. Do not schedule voice frames or infer key-on latency.
        bus.service_spu(1).unwrap();
        count += 1;
    }
    assert_eq!(cpu.register(2), 0);
    assert_eq!(cpu.register(16), 0x1234_5678);
    assert_eq!(cpu.register(17), 0x8765_4321);
    assert_eq!(cpu.register(29), stack);
    count
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_hardware_initializer_uploads_dummy_block_and_sets_all_voice_registers() {
    let exe = Executable::from_bytes(
        std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").expect("set BOF3_AUDIO_EXE")).unwrap(),
    )
    .unwrap();
    Profile::identify(&exe).unwrap();
    let mut bus = Interconnect::from_executable(&exe);
    bus.configure_spu_voices(spu_sample::Model::Published, adsr::Model::Published)
        .unwrap();
    let stack = exe.stack_pointer().unwrap();
    let count = initialize(&mut bus, stack, 0);
    assert_eq!(bus.read(0x1f80_1daa, Width::Half).unwrap(), 0xc000);
    assert_eq!(bus.read(0x1f80_1dac, Width::Half).unwrap(), 4);
    for voice in 0..24 {
        for (offset, value) in [(0, 0), (2, 0), (4, 0x3fff), (6, 0x200), (8, 0), (10, 0)] {
            assert_eq!(
                bus.read(0x1f80_1c00 + voice * 16 + offset, Width::Half)
                    .unwrap(),
                value
            );
        }
    }
    assert_eq!(
        &bus.spu_transfer().ram()[0x1000..0x1010],
        &bus.ram().bytes()[0x183af0..0x183b00]
    );
    let dummy =
        bof3_audio::codec::adpcm::decode_sample(&bus.spu_transfer().ram()[0x1000..0x1010]).unwrap();
    // Predictor zero, shift seven, packed nibbles 7/0: this is a dummy loop,
    // not zero PCM. The initializer silences voices through gain/ADSR settings.
    assert_eq!(dummy.pcm, [224, 0].repeat(14));
    assert_eq!(
        dummy.termination,
        bof3_audio::codec::adpcm::Termination::EndRepeat
    );
    assert_eq!(bus.spu_transfer().fifo_halfwords(), 0);
    assert_eq!(bus.dma().priority(), 0x076f_4321);
    assert!(bus.spu_transfer().ram()[..0x1000].iter().all(|b| *b == 0));
    assert!(bus.spu_transfer().ram()[0x1010..].iter().all(|b| *b == 0));

    // Nonzero argument skips the voice/sample reset, but still clears master
    // and wet gain, reverb sends, and the SDK's ten-halfword staging buffer.
    bus.configure_spu_disable(DisableModel::EmulatorReference)
        .unwrap();
    bus.write(0x1f801c04, Width::Half, 0x1234).unwrap();
    bus.write(0x1f801d90, Width::Word, 0x24).unwrap();
    bus.write(0x1f801d94, Width::Word, 0x42).unwrap();
    bus.write(0x1f801d98, Width::Word, 0xffff).unwrap();
    bus.write(0x1f801db0, Width::Word, 0x43211234).unwrap();
    for i in 0..10 {
        bus.write(0x8018e230 + i * 2, Width::Half, 0x9876).unwrap();
    }
    let before = bus.spu_transfer().ram().to_vec();
    let warm_count = initialize(&mut bus, stack, 1);
    assert_eq!(bus.read(0x1f801c04, Width::Half).unwrap(), 0x1234);
    assert_eq!(bus.read(0x1f801d90, Width::Word).unwrap(), 0x24);
    assert_eq!(bus.read(0x1f801d94, Width::Word).unwrap(), 0x42);
    assert_eq!(bus.read(0x1f801db0, Width::Word).unwrap(), 0x43211234);
    assert_eq!(bus.read(0x1f801d98, Width::Word).unwrap(), 0);
    assert_eq!(bus.spu_transfer().ram(), before);
    for i in 0..10 {
        assert_eq!(bus.read(0x8018e230 + i * 2, Width::Half).unwrap(), 0);
    }
    let repeat_count = initialize(&mut bus, stack, 0);
    assert_eq!(repeat_count, count);
    assert_eq!(bus.read(0x1f801c04, Width::Half).unwrap(), 0x3fff);
    for register in [
        0x1f801d80, 0x1f801d84, 0x1f801d90, 0x1f801d94, 0x1f801db0, 0x1f801db4,
    ] {
        assert_eq!(bus.read(register, Width::Word).unwrap(), 0);
    }
    assert_eq!(bus.spu_transfer().ram(), before);
    eprintln!("Original US SPU initializer: {count}/{warm_count}/{repeat_count} cold/warm/repeated instructions; register/transfer evidence only, no CPU/SPU clock or audio acceptance");
}
