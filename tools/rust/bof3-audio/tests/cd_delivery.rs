use bof3_audio::machine::{
    bus::{Bus, Ram, Width},
    cd_data::Model,
    cd_host::Host,
    interconnect::Interconnect,
};

fn host() -> Host {
    let mut host = Host::new([128, 0, 128, 0]).unwrap();
    host.configure_data(Model::EmulatorReference).unwrap();
    host.write(0, 1).unwrap();
    host.write(2, 0x1f).unwrap();
    host.write(0, 0).unwrap();
    host
}
fn drain(host: &mut Host) {
    assert_eq!(host.read(1).unwrap(), 0x22);
    host.write(0, 1).unwrap();
    host.write(3, 0x1f).unwrap();
    host.write(0, 0).unwrap();
}

#[test]
fn serialized_int1_selects_latest_unrequested_block() {
    let mut host = host();
    assert_eq!(host.deliver_data(0x22, &[1; 2048]).unwrap(), 0);
    assert!(host.irq_line());
    assert!(!host.data_ready());
    drain(&mut host);
    assert_eq!(host.deliver_data(0x22, &[2; 2340]).unwrap(), 2048);
    drain(&mut host);
    host.write(3, 0x80).unwrap();
    assert_eq!(host.read_data(4).unwrap(), 0x02020202);
    for _ in 1..585 {
        host.read_data(4).unwrap();
    }
    assert_eq!(host.deliver_data(0x22, &[3; 2048]).unwrap(), 0);
}

#[test]
fn active_or_partial_reads_are_not_silently_replaced() {
    let mut host = host();
    host.deliver_data(0x22, &[1; 2048]).unwrap();
    drain(&mut host);
    host.write(3, 0x80).unwrap();
    assert!(host.deliver_data(0x22, &[2; 2048]).is_err());
    assert_eq!(host.read_data(1).unwrap(), 1);
    assert!(host.deliver_data(0x22, &[2; 2048]).is_err());
    assert!(!host.irq_line());
    assert_eq!(host.read_data(1).unwrap(), 1);
    host.write(3, 0).unwrap(); // Explicit release/rewind before new selection.
    assert_eq!(host.deliver_data(0x22, &[2; 2048]).unwrap(), 2048);
}

#[test]
fn pending_host_work_and_bad_blocks_preserve_data_and_response() {
    for early_ack in [false, true] {
        let mut host = host();
        host.deliver_data(0x22, &[1; 2048]).unwrap();
        if early_ack {
            host.write(0, 1).unwrap();
            host.write(3, 0x1f).unwrap();
            host.write(0, 0).unwrap();
        }
        assert!(host.deliver_data(0x22, &[2; 2048]).is_err());
        drain(&mut host);
        assert!(host.deliver_data(0x22, &[2; 2047]).is_err());
        host.write(3, 0x80).unwrap();
        assert_eq!(host.read_data(4).unwrap(), 0x01010101);
    }
    let mut host = host();
    host.write(1, 1).unwrap();
    assert!(host.deliver_data(0x22, &[1; 2048]).is_err());
    host.take_command().unwrap();
    assert!(host.deliver_data(0x22, &[1; 2048]).is_err());
    host.stage_response(&[2]).unwrap();
    host.read(1).unwrap();
    assert!(host.deliver_data(0x22, &[1; 2048]).is_err());
    host.raise_interrupt(3).unwrap();
}

#[test]
fn interconnect_delivery_sets_irq_and_preserves_failure_state() {
    let mut bus = Interconnect::from_ram(Ram::default());
    bus.configure_cd_host([128, 0, 128, 0]).unwrap();
    bus.configure_cd_data(Model::EmulatorReference).unwrap();
    bus.write(0x1f801800, Width::Byte, 1).unwrap();
    bus.write(0x1f801802, Width::Byte, 0x1f).unwrap();
    assert_eq!(bus.deliver_cd_data(0x22, &[7; 2048]).unwrap(), 0);
    assert_eq!(bus.read(0x1f801070, Width::Word).unwrap() & 4, 4);
    assert!(bus.deliver_cd_data(0x22, &[8; 2048]).is_err());
    assert_eq!(bus.read(0x1f801801, Width::Byte).unwrap(), 0x22);
    bus.write(0x1f801803, Width::Byte, 0x1f).unwrap();
    bus.write(0x1f801800, Width::Byte, 0).unwrap();
    bus.write(0x1f801803, Width::Byte, 0x80).unwrap();
    assert_eq!(bus.read(0x1f801802, Width::Half).unwrap(), 0x0707);
}
