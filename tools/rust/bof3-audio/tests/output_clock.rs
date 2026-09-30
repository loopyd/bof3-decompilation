use bof3_audio::machine::{
    adsr,
    bus::{Bus, Ram, Width},
    interconnect::Interconnect,
    output_clock::{Clock, Model},
    spu_clock, spu_sample,
};

fn fixture() -> (Interconnect, spu_clock::Clock, Clock) {
    let mut bus = Interconnect::from_ram(Ram::default());
    bus.configure_spu_voices(
        spu_sample::Model::EmulatorReference,
        adsr::Model::EmulatorReference,
    )
    .unwrap();
    bus.write(0x1f801dac, Width::Half, 4).unwrap();
    bus.write(0x1f801daa, Width::Half, 0xc000).unwrap();
    (
        bus,
        spu_clock::Clock::new(spu_clock::Model::EmulatorReference),
        Clock::new(Model::PcsxReduxNtsc),
    )
}

#[test]
fn sample_and_vblank_edges_are_partition_invariant() {
    fn run(parts: &[u32]) -> (serde_json::Value, Vec<[i16; 2]>, u16, u64) {
        let (mut bus, mut transfer, mut clock) = fixture();
        bus.write(0x1f801114, Width::Half, 0x100).unwrap();
        for &ticks in parts {
            clock.advance(&mut bus, &mut transfer, ticks).unwrap();
        }
        let pcm = clock.take_frames();
        assert_eq!(
            bus.read(0x1f801110, Width::Half).unwrap(),
            (clock.ticks() / 2146 - 1) as u32
        );
        assert!(pcm.iter().all(|&frame| frame == [0; 2]));
        (
            serde_json::to_value(clock).unwrap(),
            pcm,
            bus.interrupts().status(),
            transfer.ticks(),
        )
    }
    let total = 1_130_017;
    let whole = run(&[total]);
    let mut pieces = vec![997; total as usize / 997];
    pieces.push(total % 997);
    assert_eq!(whole, run(&pieces));
    assert_eq!(whole.1.len(), total as usize / 768);
    assert_eq!(whole.0["vblanks"], 2);
    assert_eq!(whole.3, u64::from(total));

    let (mut bus, mut transfer, mut clock) = fixture();
    clock
        .advance(&mut bus, &mut transfer, 243 * 2146 - 1)
        .unwrap();
    assert_eq!(bus.interrupts().status() & 1, 0);
    clock.advance(&mut bus, &mut transfer, 1).unwrap();
    assert_eq!(bus.interrupts().status() & 1, 1);
    bus.write(0x1f801070, Width::Half, 0).unwrap();
    clock
        .advance(&mut bus, &mut transfer, 263 * 2146 - 1)
        .unwrap();
    assert_eq!(bus.interrupts().status() & 1, 0);
    clock.advance(&mut bus, &mut transfer, 1).unwrap();
    assert_eq!(bus.interrupts().status() & 1, 1);
}

#[test]
fn unsupported_clock_sources_and_undrained_output_fail_explicitly() {
    for (address, mode) in [(0x1f801104, 1), (0x1f801104, 0x100)] {
        let (mut bus, mut transfer, mut clock) = fixture();
        bus.write(address, Width::Half, mode).unwrap();
        assert!(clock
            .advance(&mut bus, &mut transfer, 768)
            .unwrap_err()
            .to_string()
            .contains("HBlank"));
        assert_eq!((clock.ticks(), transfer.ticks()), (0, 0));
    }
    let (mut bus, mut transfer, mut clock) = fixture();
    clock.advance(&mut bus, &mut transfer, 4096 * 768).unwrap();
    assert!(clock
        .advance(&mut bus, &mut transfer, 768)
        .unwrap_err()
        .to_string()
        .contains("queue full"));
    assert_eq!(clock.frames(), 4096);
    assert_eq!(clock.take_frames().len(), 4096);
}
