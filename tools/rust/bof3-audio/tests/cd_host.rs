use bof3_audio::machine::{
    bus::{Bus, Ram, Width},
    cd_host::{Command, Host},
    interconnect::Interconnect,
};

#[test]
fn emulator_reset_has_enabled_interrupts_but_no_seeded_response_or_completion() {
    use bof3_audio::machine::cd_host::Model;
    let mut bus = Interconnect::from_ram(Ram::default());
    bus.configure_cd_host_reset(Model::EmulatorReference)
        .unwrap();
    assert_eq!(bus.cd_host().unwrap().volume().active(), [128, 0, 128, 0]);
    assert!(!bus.cd_host().unwrap().busy());
    assert!(!bus.cd_host().unwrap().irq_line());
    assert_eq!(bus.read(0x1f801800, Width::Byte).unwrap(), 0x18);
    assert_eq!(bus.read(0x1f801803, Width::Byte).unwrap(), 0xff);
    bus.write(0x1f801800, Width::Byte, 1).unwrap();
    assert_eq!(bus.read(0x1f801803, Width::Byte).unwrap(), 0xe0);
    bus.write(0x1f801800, Width::Byte, 0).unwrap();
    bus.write(0x1f801801, Width::Byte, 1).unwrap();
    assert_eq!(bus.take_cd_command().unwrap().unwrap().opcode, 1);
    assert!(bus
        .configure_cd_host_reset(Model::EmulatorReference)
        .is_err());
    assert!(bus.cd_host().unwrap().busy());
    bus.respond_cd(3, &[2]).unwrap();
    assert!(bus.cd_host().unwrap().irq_line());
    assert_eq!(bus.read(0x1f801801, Width::Byte).unwrap(), 2);
}

#[test]
fn command_parameters_busy_and_response_readiness_have_explicit_boundaries() {
    let mut host = Host::new([128, 0, 128, 0]).unwrap();
    assert_eq!(host.read(0).unwrap(), 0x18);
    for i in 0..16 {
        host.write(2, i).unwrap();
    }
    assert_eq!(host.read(0).unwrap(), 0);
    assert!(host.write(2, 17).is_err());
    host.write(1, 0x19).unwrap();
    assert_eq!(host.read(0).unwrap(), 0x80);
    assert!(host.respond(3, &[2]).is_err());
    assert_eq!(
        host.take_command(),
        Some(Command {
            opcode: 0x19,
            parameters: (0..16).collect()
        })
    );
    assert!(host.take_command().is_none());
    assert!(host.busy());
    assert_eq!(host.read(0).unwrap(), 0x98);
    assert!(host.write(1, 1).is_err());
    host.respond(3, &[2, 0x20]).unwrap();
    assert_eq!(host.read(0).unwrap(), 0x38);
    assert!(!host.busy());
    assert_eq!(host.read(1).unwrap(), 2);
    assert_eq!(host.read(0).unwrap() & 0x20, 0x20);
    assert_eq!(host.read(1).unwrap(), 0x20);
    assert_eq!(host.read(0).unwrap() & 0x20, 0);
    for _ in 2..16 {
        assert_eq!(host.read(1).unwrap(), 0);
    }
    assert_eq!(host.read(1).unwrap(), 2);
    assert_eq!(host.read(0).unwrap() & 0x20, 0);
}

#[test]
fn flags_are_numeric_bitmasked_and_acknowledgement_does_not_drain_response() {
    let mut host = Host::new([128, 0, 128, 0]).unwrap();
    host.respond(3, &[2]).unwrap();
    assert!(!host.irq_line());
    host.write(0, 1).unwrap();
    host.write(2, 1).unwrap();
    assert!(host.irq_line()); // IRQ3 & mask1, not 1 << IRQ3.
    host.write(3, 1).unwrap();
    assert_eq!(host.read(3).unwrap(), 0xe2);
    assert!(!host.irq_line());
    host.write(3, 7).unwrap();
    assert!(host.respond(2, &[3]).is_err());
    assert_eq!(host.read(1).unwrap(), 2);
    host.respond(2, &[3]).unwrap();
    assert_eq!(host.read(3).unwrap(), 0xe2);
    host.write(2, 2).unwrap();
    assert!(host.irq_line());
    host.write(0, 2).unwrap();
    assert_eq!(host.read(3).unwrap(), 0xe2); // even bank reads mask
    host.write(0, 3).unwrap();
    assert_eq!(host.read(3).unwrap(), 0xe2); // odd bank reads flags
}

#[test]
fn response_readiness_can_precede_interrupt_without_accepting_an_overlapping_command() {
    let mut bus = Interconnect::from_ram(Ram::default());
    bus.configure_cd_host([128, 0, 128, 0]).unwrap();
    assert!(bus.raise_cd_interrupt(3).is_err());
    bus.write(0x1f801800, Width::Byte, 1).unwrap();
    bus.write(0x1f801802, Width::Byte, 7).unwrap();
    bus.write(0x1f801800, Width::Byte, 0).unwrap();
    bus.write(0x1f801801, Width::Byte, 1).unwrap();
    assert_eq!(bus.take_cd_command().unwrap().unwrap().opcode, 1);
    bus.stage_cd_response(&[2]).unwrap();
    assert!(!bus.cd_host().unwrap().busy());
    assert_eq!(bus.read(0x1f801800, Width::Byte).unwrap() & 0x20, 0x20);
    assert_eq!(bus.interrupts().status() & 4, 0);
    assert_eq!(bus.read(0x1f801801, Width::Byte).unwrap(), 2);
    assert!(bus.write(0x1f801801, Width::Byte, 1).is_err());
    assert!(bus.stage_cd_response(&[3]).is_err());
    bus.raise_cd_interrupt(3).unwrap();
    assert_eq!(bus.interrupts().status() & 4, 4);
    bus.write(0x1f801800, Width::Byte, 1).unwrap();
    bus.write(0x1f801803, Width::Byte, 7).unwrap();
    bus.write(0x1f801800, Width::Byte, 0).unwrap();
    bus.write(0x1f801801, Width::Byte, 1).unwrap();
}

#[test]
fn clearing_parameters_and_audio_register_banks_preserve_independent_state() {
    let mut host = Host::new([128, 0, 128, 0]).unwrap();
    host.write(2, 7).unwrap();
    host.write(0, 0xfd).unwrap();
    host.write(3, 0x40).unwrap();
    assert_eq!(host.read(0).unwrap(), 0x19);
    host.write(0, 2).unwrap();
    host.write(2, 64).unwrap();
    host.write(3, 32).unwrap();
    host.write(0, 3).unwrap();
    host.write(1, 64).unwrap();
    host.write(2, 32).unwrap();
    assert_eq!(host.volume().active(), [128, 0, 128, 0]);
    host.write(3, 0x21).unwrap();
    assert_eq!(host.volume().active(), [64, 32, 64, 32]);
    assert_eq!(host.volume().apply([1000, -1000], false), [250, -250]);
    assert_eq!(host.volume().apply([1000, -1000], true), [0, 0]);
}

#[test]
fn interconnect_irq_is_latched_and_retriggers_only_after_host_line_drops() {
    let mut bus = Interconnect::from_ram(Ram::from_bytes(vec![0; 2 * 1024 * 1024]).unwrap());
    assert!(bus.read(0x1f801800, Width::Byte).is_err());
    bus.configure_cd_host([128, 0, 128, 0]).unwrap();
    assert!(bus.configure_cd_host([0; 4]).is_err());
    bus.write(0xbf801800, Width::Byte, 1).unwrap();
    bus.write(0x1f801802, Width::Byte, 7).unwrap();
    bus.respond_cd(3, &[2]).unwrap();
    assert_eq!(bus.interrupts().status() & 4, 4);
    bus.write(0x1f801070, Width::Word, !4).unwrap();
    assert_eq!(bus.interrupts().status() & 4, 0);
    bus.write(0x1f801802, Width::Byte, 7).unwrap();
    assert_eq!(bus.interrupts().status() & 4, 0);
    assert_eq!(bus.read(0x9f801801, Width::Byte).unwrap(), 2);
    bus.write(0x1f801803, Width::Byte, 7).unwrap();
    bus.respond_cd(2, &[2]).unwrap();
    assert_eq!(bus.interrupts().status() & 4, 4);
    assert!(bus.read(0x1f801800, Width::Word).is_err());
    assert!(bus.write(0x1f801802, Width::Half, 0).is_err());
}

#[test]
fn unsupported_device_paths_and_response_overwrites_fail_without_false_success() {
    let mut host = Host::new([128, 0, 128, 0]).unwrap();
    for (irq, response) in [(0, vec![0]), (6, vec![0]), (3, vec![]), (3, vec![0; 17])] {
        assert!(host.respond(irq, &response).is_err());
    }
    assert_eq!(host.read(0).unwrap(), 0x18);
    assert!(host.read(2).is_err());
    assert!(host.write(3, 0x80).is_err());
    host.respond(3, &[2]).unwrap();
    assert!(host.respond(2, &[3]).is_err());
    assert!(host.write(1, 1).is_err());
    host.write(0, 1).unwrap();
    assert!(host.write(3, 0x87).is_err());
    assert_eq!(host.read(3).unwrap(), 0xe3);
    assert_eq!(host.read(1).unwrap(), 2);
}
