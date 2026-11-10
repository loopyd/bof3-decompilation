use bof3_audio::machine::{
    bus::{Bus, Ram, Width},
    cpu::clock::Clock,
    interconnect::Interconnect,
};

fn half(bus: &mut Interconnect, address: u32, value: u16) {
    bus.write(address, Width::Half, u32::from(value)).unwrap();
}

fn fixture() -> (Interconnect, Clock) {
    let mut bus = Interconnect::from_ram(Ram::default());
    half(&mut bus, 0x1f801dac, 4);
    (bus, Clock::default())
}

#[test]
fn elapsed_time_is_partition_invariant_and_does_not_delay_manual_writes() {
    fn execute(steps: &[u32]) -> (Vec<u8>, u64) {
        let (mut bus, mut clock) = fixture();
        clock.advance(1_000_003).unwrap();
        half(&mut bus, 0x1f801da6, 0xffff);
        for value in 1..=32 {
            half(&mut bus, 0x1f801da8, value);
        }
        half(&mut bus, 0x1f801daa, 0x10);
        assert_eq!(bus.spu_transfer().current_address(), 56);
        for &ticks in steps {
            clock.advance(ticks).unwrap();
        }
        assert_eq!(clock.ticks(), 1_000_515);
        assert_eq!(bus.spu_transfer().current_address(), 56);
        (bus.spu_transfer().ram().to_vec(), clock.ticks())
    }
    assert_eq!(execute(&[512]), execute(&[1; 512]));
}

#[test]
fn stopped_mode_access_is_immediate_and_bad_ram_control_fails_at_the_port() {
    let (mut bus, mut clock) = fixture();
    half(&mut bus, 0x1f801da8, 0xabcd);
    assert_eq!(&bus.spu_transfer().ram()[..2], &[0xcd, 0xab]);
    clock.advance(1000).unwrap();
    half(&mut bus, 0x1f801dac, 0);
    half(&mut bus, 0x1f801daa, 0x10);
    assert!(bus.write(0x1f801da8, Width::Half, 0x1234).is_err());
    assert_eq!(clock.ticks(), 1000);
    assert_eq!(bus.spu_transfer().current_address(), 2);
    half(&mut bus, 0x1f801dac, 4);
    half(&mut bus, 0x1f801da8, 0x1234);
    assert_eq!(&bus.spu_transfer().ram()[..4], &[0xcd, 0xab, 0x34, 0x12]);
}

#[test]
fn dma_read_payload_is_immediate_but_busy_waits_for_a_branch_callback() {
    let (mut bus, mut clock) = fixture();
    for value in 0x8000..0x8020 {
        half(&mut bus, 0x1f801da8, value);
    }
    half(&mut bus, 0x1f801da6, 0);
    for (address, value) in [
        (0x1f801014, 0x220931e1),
        (0x1f8010f0, 0x076d4321),
        (0x1f8010c0, 0x10000),
        (0x1f8010c4, 0x10010),
        (0x1f8010c8, 0x01000200),
    ] {
        bus.write(address, Width::Word, value).unwrap();
    }
    for word in 0..16 {
        assert_eq!(
            bus.read(0x10000 + word * 4, Width::Word).unwrap(),
            (0x8001 + word * 2) << 16 | (0x8000 + word * 2)
        );
    }
    clock.advance(511).unwrap();
    bus.advance_devices(clock.ticks(), false)
        .unwrap()
        .require_ready()
        .unwrap();
    assert_ne!(bus.read(0x1f8010c8, Width::Word).unwrap() & (1 << 24), 0);
    clock.advance(1).unwrap();
    bus.advance_devices(clock.ticks(), true)
        .unwrap()
        .require_ready()
        .unwrap();
    assert_eq!(bus.read(0x1f8010c8, Width::Word).unwrap(), 0x200);
}
