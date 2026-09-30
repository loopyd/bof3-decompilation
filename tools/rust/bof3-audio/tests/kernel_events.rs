use bof3_audio::machine::{
    bus::{Bus, Ram, Width},
    cpu::Cpu,
    events::{Table, BUSY, CALLBACK, DISABLED, POLLING, READY, RECORD_BYTES},
    kernel::{Call, Kernel},
};

const BASE: u32 = 0x8000_8000;
const RETURN: u32 = 0x8000_1000;

fn configure(ram: &mut Ram, slots: u32) {
    ram.write(0x120, Width::Word, BASE).unwrap();
    ram.write(0x124, Width::Word, slots * RECORD_BYTES).unwrap();
    for slot in 0..slots {
        for word in 0..7 {
            ram.write(
                BASE + slot * RECORD_BYTES + word * 4,
                Width::Word,
                if word == 1 { 0 } else { 0xfeed_0000 + word },
            )
            .unwrap();
        }
    }
}

fn invoke(
    kernel: &mut Kernel,
    cpu: &mut Cpu,
    ram: &mut Ram,
    service: u32,
    args: [u32; 4],
) -> bof3_audio::Result<Call> {
    cpu.resume_at(0xb0);
    cpu.set_register(9, service);
    cpu.set_register(31, RETURN);
    for (i, arg) in args.into_iter().enumerate() {
        cpu.set_register(i + 4, arg);
    }
    kernel.dispatch(cpu, ram)
}

fn state(ram: &mut Ram, slot: u32) -> u32 {
    ram.read(BASE + slot * RECORD_BYTES + 4, Width::Word)
        .unwrap()
}

#[test]
fn allocation_reuses_lowest_free_slot_and_preserves_opaque_record_words() {
    let mut ram = Ram::default();
    configure(&mut ram, 2);
    let table = Table::read(&mut ram).unwrap();
    assert_eq!(table.free_slot(&mut ram).unwrap(), 0);
    assert_eq!(table.free_slot(&mut ram).unwrap(), 0); // query does not reserve
    let a = table.open(&mut ram, 0xf000_0003, 0x10, POLLING, 0).unwrap();
    let b = table
        .open(&mut ram, 0xf000_0009, 2, CALLBACK, 0x8001_0000)
        .unwrap();
    assert_eq!((a, b), (0xf100_0000, 0xf100_0001));
    assert_eq!(state(&mut ram, 0), DISABLED);
    assert_eq!(table.free_slot(&mut ram).unwrap(), u32::MAX);
    assert_eq!(table.open(&mut ram, 1, 2, POLLING, 0).unwrap(), u32::MAX);
    for slot in 0..2 {
        for word in [5, 6] {
            assert_eq!(
                ram.read(BASE + slot * RECORD_BYTES + word * 4, Width::Word)
                    .unwrap(),
                0xfeed_0000 + word
            );
        }
    }
    table.close(&mut ram, a).unwrap();
    assert_eq!(ram.read(BASE, Width::Word).unwrap(), 0xf000_0003);
    assert_eq!(table.open(&mut ram, 0x1234, 7, POLLING, 0).unwrap(), a);
    // Descriptor consumers use low 16 bits, not invented generation counters.
    table.enable(&mut ram, 0xbeef_0000, true).unwrap();
    assert_eq!(state(&mut ram, 0), BUSY);
}

#[test]
fn delivery_requires_both_identifiers_and_enabled_state_and_test_consumes_once() {
    let mut ram = Ram::default();
    configure(&mut ram, 5);
    let table = Table::read(&mut ram).unwrap();
    for (class, spec, enabled) in [
        (3, 16, true),
        (3, 16, true),
        (3, 32, true),
        (4, 16, true),
        (3, 16, false),
    ] {
        let handle = table.open(&mut ram, class, spec, POLLING, 0).unwrap();
        table.enable(&mut ram, handle, enabled).unwrap();
    }
    assert_eq!(table.delivery(3, 16).next_callback(&mut ram).unwrap(), None);
    assert_eq!(
        (0..5).map(|i| state(&mut ram, i)).collect::<Vec<_>>(),
        [READY, READY, BUSY, BUSY, DISABLED]
    );
    assert!(table.test(&mut ram, 0xf100_0000).unwrap());
    assert!(!table.test(&mut ram, 0xf100_0000).unwrap());
    table.undeliver(&mut ram, 3, 16).unwrap();
    assert_eq!(state(&mut ram, 1), BUSY);
    assert_eq!(table.delivery(3, 16).next_callback(&mut ram).unwrap(), None);
    table.enable(&mut ram, 0xf100_0000, false).unwrap();
    assert!(!table.test(&mut ram, 0xf100_0000).unwrap());
    table.close(&mut ram, 0xf100_0000).unwrap();
    table.enable(&mut ram, 0xf100_0000, true).unwrap();
    assert_eq!(state(&mut ram, 0), 0); // enable does not allocate a free slot
}

#[test]
fn delivery_cursor_yields_callbacks_after_prior_polling_events() {
    let mut ram = Ram::default();
    configure(&mut ram, 3);
    let table = Table::read(&mut ram).unwrap();
    for (mode, handler) in [(POLLING, 0), (CALLBACK, 0x8010_1234), (CALLBACK, 0)] {
        let id = table.open(&mut ram, 3, 16, mode, handler).unwrap();
        table.enable(&mut ram, id, true).unwrap();
    }
    let mut delivery = table.delivery(3, 16);
    assert_eq!(delivery.next_callback(&mut ram).unwrap(), Some(0x8010_1234));
    assert_eq!(delivery.next_callback(&mut ram).unwrap(), None);
    assert_eq!(state(&mut ram, 0), READY);
    assert_eq!(state(&mut ram, 2), BUSY); // null callback has no ready flag
}

#[test]
fn wait_yields_and_resumes_only_after_delivery_even_if_disabled_or_table_relocated() {
    let mut ram = Ram::default();
    configure(&mut ram, 2);
    let mut cpu = Cpu::new(RETURN);
    let mut kernel = Kernel::default();
    invoke(&mut kernel, &mut cpu, &mut ram, 8, [3, 16, POLLING, 0]).unwrap();
    let id = cpu.register(2);
    let call = invoke(&mut kernel, &mut cpu, &mut ram, 10, [id, 0, 0, 0]).unwrap();
    assert!(!call.waiting);
    assert_eq!((cpu.pc(), cpu.register(2)), (RETURN, 0)); // initially disabled
    invoke(&mut kernel, &mut cpu, &mut ram, 12, [id, 0, 0, 0]).unwrap();
    cpu.set_register(2, 0x1234);
    let call = invoke(&mut kernel, &mut cpu, &mut ram, 10, [id, 0, 0, 0]).unwrap();
    assert!(call.waiting);
    assert_eq!((cpu.pc(), cpu.register(2)), (0xb0, 0x1234));
    ram.write(BASE + 4, Width::Word, DISABLED).unwrap();
    assert!(kernel.dispatch(&mut cpu, &mut ram).unwrap().waiting);
    ram.write(BASE + 4, Width::Word, 0).unwrap();
    assert!(kernel.dispatch(&mut cpu, &mut ram).unwrap().waiting);
    // In-progress WaitEvent retained a record pointer; it does not look up
    // the event descriptor afresh on every host scheduler boundary.
    ram.write(0x120, Width::Word, BASE + 0x1000).unwrap();
    ram.write(BASE + 4, Width::Word, READY).unwrap();
    assert!(!kernel.dispatch(&mut cpu, &mut ram).unwrap().waiting);
    assert_eq!((cpu.pc(), cpu.register(2)), (RETURN, 1));
    assert_eq!(state(&mut ram, 0), BUSY);
}

#[test]
fn guest_table_errors_unknown_modes_and_out_of_range_handles_do_not_mutate_ram() {
    let mut ram = Ram::default();
    assert!(Table::read(&mut ram).is_err());
    for (base, size) in [
        (BASE + 1, 28),
        (BASE, 27),
        (0x1f80_1c00, 28),
        (0x801f_fff0, 28),
    ] {
        ram.write(0x120, Width::Word, base).unwrap();
        ram.write(0x124, Width::Word, size).unwrap();
        assert!(Table::read(&mut ram).is_err());
    }
    configure(&mut ram, 1);
    let table = Table::read(&mut ram).unwrap();
    let before = ram.bytes().to_vec();
    assert!(table.open(&mut ram, 3, 16, 0x3000, 0).is_err());
    assert!(table.close(&mut ram, 0xf100_0001).is_err());
    assert!(table.enable(&mut ram, 0xf100_ffff, true).is_err());
    assert!(table.test(&mut ram, 0xffff_ffff).is_err());
    assert_eq!(ram.bytes(), before);
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_event_wrappers_execute_with_explicit_guest_table_context() {
    use bof3_audio::machine::{executable::Executable, profile::Profile};
    let exe =
        Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    Profile::identify(&exe).unwrap();
    let mut ram = Ram::from_executable(&exe);
    // Explicit synthetic allocation, not recovered BIOS bootstrap state.
    configure(&mut ram, 22);
    let mut kernel = Kernel::default();
    for (entry, args, expected) in [
        (0x8017_ed3c, [3, 16, POLLING, 0], Some(0xf100_0000)),
        (0x8017_ed7c, [0xf100_0000, 0, 0, 0], Some(1)),
        (0x8017_ed2c, [3, 16, 0, 0], None),
        (0x8017_ed6c, [0xf100_0000, 0, 0, 0], Some(1)),
        (0x8017_ed6c, [0xf100_0000, 0, 0, 0], Some(0)),
        (0x8017_ed4c, [0xf100_0000, 0, 0, 0], Some(1)),
    ] {
        let mut cpu = Cpu::new(entry);
        cpu.set_register(31, RETURN);
        cpu.set_register(16, 0xabcdef);
        for (i, arg) in args.into_iter().enumerate() {
            cpu.set_register(4 + i, arg);
        }
        let mut returned = false;
        for _ in 0..10 {
            if cpu.pc() == RETURN {
                returned = true;
                break;
            }
            if Kernel::is_call(cpu.pc()) {
                assert!(!kernel.dispatch(&mut cpu, &mut ram).unwrap().waiting);
            } else {
                cpu.step(&mut ram).unwrap();
            }
        }
        assert!(returned, "entry={entry:08x}");
        assert_eq!(cpu.register(16), 0xabcdef);
        if let Some(value) = expected {
            assert_eq!(cpu.register(2), value, "entry={entry:08x}");
        }
    }
    assert_eq!(state(&mut ram, 0), 0);
}
