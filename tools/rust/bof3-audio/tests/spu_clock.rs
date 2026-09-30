use bof3_audio::machine::{
    bus::{Bus, Ram, Width},
    interconnect::Interconnect,
    spu_clock::{Clock, Model},
};

fn half(bus: &mut Interconnect, address: u32, value: u16) {
    bus.write(address, Width::Half, u32::from(value)).unwrap();
}

fn fixture() -> (Interconnect, Clock) {
    let mut bus = Interconnect::from_ram(Ram::default());
    half(&mut bus, 0x1f801dac, 4);
    (bus, Clock::new(Model::EmulatorReference))
}

#[test]
fn transfer_time_is_partition_invariant_and_idle_time_does_not_buy_future_work() {
    fn execute(steps: &[u32]) -> (Vec<u8>, u64) {
        let (mut bus, mut clock) = fixture();
        clock.advance(&mut bus, 1_000_003).unwrap();
        half(&mut bus, 0x1f801da6, 0xffff);
        for value in 1..=32 {
            half(&mut bus, 0x1f801da8, value);
        }
        half(&mut bus, 0x1f801daa, 0x10);
        clock.advance(&mut bus, 0).unwrap();
        assert_eq!(clock.halfwords(), 0);
        for &ticks in steps {
            clock.advance(&mut bus, ticks).unwrap();
        }
        assert_eq!(clock.ticks(), 1_000_003 + 512);
        assert_eq!(clock.halfwords(), 32);
        assert_eq!(bus.spu_transfer().fifo_halfwords(), 0);
        assert_eq!(bus.spu_transfer().current_address(), 56);
        (bus.spu_transfer().ram().to_vec(), clock.halfwords())
    }
    assert_eq!(execute(&[512]), execute(&[1; 512]));
    let (mut bus, mut clock) = fixture();
    clock.advance(&mut bus, 999).unwrap();
    half(&mut bus, 0x1f801daa, 0x10);
    half(&mut bus, 0x1f801da8, 0x1234);
    clock.advance(&mut bus, 15).unwrap();
    assert_eq!(clock.halfwords(), 0);
    assert_ne!(bus.read(0x1f801dae, Width::Half).unwrap() & 0x400, 0);
    clock.advance(&mut bus, 1).unwrap();
    assert_eq!(&bus.spu_transfer().ram()[..2], &[0x34, 0x12]);
    assert_eq!(bus.read(0x1f801dae, Width::Half).unwrap() & 0x400, 0);
    clock.advance(&mut bus, 100).unwrap();
    half(&mut bus, 0x1f801da8, 0x5678);
    clock.advance(&mut bus, 15).unwrap();
    assert_eq!(clock.halfwords(), 1);
    clock.advance(&mut bus, 1).unwrap();
    assert_eq!(clock.halfwords(), 2);
}

#[test]
fn stopped_mode_preserves_fifo_and_bad_control_does_not_advance_time() {
    let (mut bus, mut clock) = fixture();
    half(&mut bus, 0x1f801da8, 0xabcd);
    clock.advance(&mut bus, 1000).unwrap();
    assert_eq!(clock.halfwords(), 0);
    assert_eq!(bus.spu_transfer().fifo_halfwords(), 1);
    half(&mut bus, 0x1f801dac, 0);
    half(&mut bus, 0x1f801daa, 0x10);
    assert!(clock.advance(&mut bus, 16).is_err());
    assert_eq!(clock.ticks(), 1000);
    assert_eq!(bus.spu_transfer().fifo_halfwords(), 1);
    half(&mut bus, 0x1f801dac, 4);
    clock.advance(&mut bus, 16).unwrap();
    assert_eq!(clock.halfwords(), 1);
    assert_eq!(&bus.spu_transfer().ram()[..2], &[0xcd, 0xab]);
}

#[test]
fn dma_read_waits_for_full_fifo_and_preserves_the_last_halfword_boundary() {
    let (mut bus, mut clock) = fixture();
    half(&mut bus, 0x1f801daa, 0x10);
    for value in 0x8000..0x8020 {
        half(&mut bus, 0x1f801da8, value);
    }
    clock.advance(&mut bus, 512).unwrap();
    half(&mut bus, 0x1f801da6, 0);
    half(&mut bus, 0x1f801daa, 0x30);
    for (address, value) in [
        (0x1f801014, 0x220931e1),
        (0x1f8010f0, 0x076d4321),
        (0x1f8010c0, 0x10000),
        (0x1f8010c4, 0x10010),
        (0x1f8010c8, 0x01000200),
    ] {
        bus.write(address, Width::Word, value).unwrap();
    }
    clock.advance(&mut bus, 511).unwrap();
    assert_eq!(bus.spu_transfer().fifo_halfwords(), 31);
    assert_eq!(bus.read(0x10000, Width::Word).unwrap(), 0);
    assert_ne!(bus.read(0x1f8010c8, Width::Word).unwrap() & (1 << 24), 0);
    clock.advance(&mut bus, 1).unwrap();
    assert_eq!(clock.halfwords(), 64);
    assert_eq!(bus.read(0x1f8010c8, Width::Word).unwrap() & (1 << 24), 0);
    for word in 0..16 {
        assert_eq!(
            bus.read(0x10000 + word * 4, Width::Word).unwrap(),
            (0x8001 + word * 2) << 16 | (0x8000 + word * 2)
        );
    }
}
