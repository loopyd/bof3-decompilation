use bof3_audio::machine::{
    bus::{Bus, Ram, Width},
    cpu::Cpu,
    exception_calls::{Environment, MissingHook},
    kernel::Kernel,
};
const TABLE: u32 = 0x8000_3000;
const NODE: u32 = 0x8000_4000;
const TCB: u32 = 0x8000_5000;
const CODE: u32 = 0x8000_1000;
const STACK: u32 = 0x8001_0000;
fn word(ram: &mut impl Bus, a: u32, v: u32) {
    ram.write(a, Width::Word, v).unwrap();
}
fn read(ram: &mut impl Bus, a: u32) -> u32 {
    ram.read(a, Width::Word).unwrap()
}
fn code(ram: &mut impl Bus, a: u32, words: &[u32]) {
    for (i, &v) in words.iter().enumerate() {
        word(ram, a + i as u32 * 4, v);
    }
}
fn fixture(ram: &mut impl Bus) {
    for (a, v) in [
        (0x100, TABLE),
        (0x104, 32),
        (0x108, 0x8000_6000),
        (0x10c, 4),
        (0x110, TCB),
        (0x114, 0xc0),
        (0x8000_6000, TCB),
        (TCB, 0x4000),
    ] {
        word(ram, a, v);
    }
    for i in 0..8 {
        word(ram, TABLE + i * 4, 0);
    }
}
fn interrupted_cpu() -> Cpu {
    let mut cpu = Cpu::new(CODE);
    for r in 1..32 {
        cpu.set_register(r, 0xabcd_0000 + r as u32);
    }
    cpu.set_register(29, 0x801f_0000);
    cpu.cop0_mut().write(12, 0x401).unwrap();
    assert!(cpu.take_interrupt(true));
    cpu
}
fn environment(missing_hook: MissingHook) -> Environment {
    Environment {
        stack_top: STACK,
        global_pointer: 0,
        missing_hook,
    }
}
fn node(ram: &mut impl Bus, a: u32, next: u32, first: u32, second: u32) {
    code(ram, a, &[next, second, first, 0x1122_3344]);
}
fn logger(ram: &mut impl Bus, a: u32, id: u16, result: u16) {
    code(
        ram,
        a,
        &[
            0x3c08_8000,
            0x8d09_2000,
            0,
            0x2529_0001,
            0xad09_2000,
            0x0009_4880,
            0x0109_4021,
            0x240a_0000 | u32::from(id),
            0xad0a_2000,
            0xad04_2100,
            0x03e0_0008,
            0x2402_0000 | u32::from(result),
        ],
    );
}
fn run(kernel: &mut Kernel, cpu: &mut Cpu, ram: &mut impl Bus) -> bof3_audio::Result<usize> {
    for i in 0..2000 {
        if !kernel.pending_exception() {
            return Ok(i);
        }
        if Kernel::is_call(cpu.pc()) {
            let call = kernel.dispatch(cpu, ram)?;
            if call.waiting {
                return Err("unexpected wait in fixture".into());
            }
        } else {
            cpu.step(ram)?;
        }
    }
    Err("exception fixture exceeded transition budget".into())
}
fn logs(ram: &mut impl Bus) -> Vec<u32> {
    (1..=read(ram, 0x2000))
        .map(|i| read(ram, 0x2000 + i * 4))
        .collect()
}

#[test]
fn priority_order_conditional_handlers_and_default_return_restore_interrupted_thread() {
    let mut ram = Ram::default();
    fixture(&mut ram);
    let mut cpu = interrupted_cpu();
    let saved: Vec<_> = (0..32).map(|r| cpu.register(r)).collect();
    for (i, result) in [
        (1, 0),
        (2, 0),
        (3, 7),
        (4, 0),
        (5, 0),
        (6, 9),
        (7, 1),
        (8, 0),
    ] {
        logger(&mut ram, 0x8000_8000 + i * 0x100, i as u16, result);
    }
    node(&mut ram, NODE, NODE + 16, 0x8000_8100, 0x8000_8200);
    node(&mut ram, NODE + 16, 0, 0x8000_8300, 0x8000_8400);
    node(&mut ram, NODE + 32, 0, 0, 0x8000_8500);
    node(&mut ram, NODE + 48, 0, 0x8000_8600, 0);
    node(&mut ram, NODE + 64, 0, 0x8000_8700, 0x8000_8800);
    for (i, head) in [NODE, NODE + 32, NODE + 48, NODE + 64]
        .into_iter()
        .enumerate()
    {
        word(&mut ram, TABLE + i as u32 * 8, head);
    }
    let mut kernel = Kernel::default();
    kernel
        .begin_exception(
            &mut cpu,
            &mut ram,
            environment(MissingHook::ReturnFromException),
        )
        .unwrap();
    run(&mut kernel, &mut cpu, &mut ram).unwrap();
    assert_eq!(logs(&mut ram), [1, 3, 4, 6, 7, 8]);
    assert_eq!(read(&mut ram, 0x2100 + 3 * 4), 7);
    assert_eq!(read(&mut ram, 0x2100 + 6 * 4), 1);
    assert_eq!(cpu.pc(), CODE);
    for (r, &v) in saved.iter().enumerate() {
        assert_eq!(cpu.register(r), if r == 26 { CODE } else { v });
    }
    assert_eq!(cpu.cop0().status(), 0x401);
    assert!(!kernel.pending_exception());
}

#[test]
fn second_pointer_is_captured_but_links_and_later_heads_are_read_after_guest_changes() {
    let mut ram = Ram::default();
    fixture(&mut ram);
    let mut cpu = interrupted_cpu();
    node(&mut ram, NODE, NODE + 16, 0x8000_8000, 0x8000_8100);
    node(&mut ram, NODE + 16, 0, 0x8000_8200, 0);
    node(&mut ram, NODE + 32, 0, 0x8000_8300, 0);
    word(&mut ram, TABLE, NODE);
    // Clear cached second field and next link, install a later head, rebind
    // the descriptor. Dispatch must retain its captured table and second pointer.
    code(
        &mut ram,
        0x8000_8000,
        &[
            0x3c08_8000,
            0xad00_4004,
            0xad00_4000,
            0x3509_4020,
            0xad09_3008,
            0x3509_7000,
            0xad09_0100,
            0x03e0_0008,
            0x2402_0005,
        ],
    );
    logger(&mut ram, 0x8000_8100, 10, 0);
    logger(&mut ram, 0x8000_8200, 99, 0);
    logger(&mut ram, 0x8000_8300, 20, 0);
    let mut kernel = Kernel::default();
    kernel
        .begin_exception(
            &mut cpu,
            &mut ram,
            environment(MissingHook::ReturnFromException),
        )
        .unwrap();
    run(&mut kernel, &mut cpu, &mut ram).unwrap();
    assert_eq!(logs(&mut ram), [10, 20]);
    assert_eq!(read(&mut ram, 0x2104), 5);
    assert_eq!(read(&mut ram, 0x100), 0x8000_7000);
}

#[test]
fn hook_entry_is_a_live_jump_buffer_and_missing_hook_policy_is_explicit() {
    let mut ram = Ram::default();
    fixture(&mut ram);
    let mut kernel = Kernel::default();
    let mut setup = Cpu::new(0xb0);
    setup.set_register(9, 0x19);
    setup.set_register(4, 0x8000_7000);
    setup.set_register(31, CODE);
    kernel.dispatch(&mut setup, &mut ram).unwrap();
    // RA, SP, FP, S0..S7, GP, in the BIOS setjmp order.
    code(
        &mut ram,
        0x8000_7000,
        &[
            0x8000_8000,
            STACK - 4,
            0x100,
            0x101,
            0x102,
            0x103,
            0x104,
            0x105,
            0x106,
            0x107,
            0x108,
            0x109,
        ],
    );
    code(
        &mut ram,
        0x8000_8000,
        &[
            0x3c08_8000,
            0xad02_2200,
            0xad10_2204,
            0xad1c_2208,
            0x2409_0017,
            0x0800_002c,
            0,
        ],
    );
    let mut cpu = interrupted_cpu();
    let call = kernel
        .begin_exception(&mut cpu, &mut ram, environment(MissingHook::Reject))
        .unwrap();
    assert_eq!(call.callback, Some(0x8000_8000));
    assert!(kernel.pending_exception());
    run(&mut kernel, &mut cpu, &mut ram).unwrap();
    assert_eq!(
        [
            read(&mut ram, 0x2200),
            read(&mut ram, 0x2204),
            read(&mut ram, 0x2208)
        ],
        [1, 0x101, 0x109]
    );
    assert_eq!(cpu.pc(), CODE);
    let mut missing = Kernel::default();
    let mut cpu = interrupted_cpu();
    assert!(missing
        .begin_exception(&mut cpu, &mut ram, environment(MissingHook::Reject))
        .unwrap_err()
        .to_string()
        .contains("no registered exit hook"));
    assert!(missing.pending_exception());
}

#[test]
fn explicit_nonlocal_exception_return_discards_only_owned_event_continuations() {
    use bof3_audio::machine::events::{Table as Events, CALLBACK};
    for early_return in [false, true] {
        let mut ram = Ram::default();
        fixture(&mut ram);
        let mut kernel = Kernel::default();
        let mut cpu = interrupted_cpu();
        word(&mut ram, 0x120, 0x8000_6100);
        word(&mut ram, 0x124, 0x1c);
        let events = Events::read(&mut ram).unwrap();
        let handle = events.open(&mut ram, 3, 16, CALLBACK, 0x8000_8100).unwrap();
        events.enable(&mut ram, handle, true).unwrap();
        node(&mut ram, NODE, 0, 0x8000_8000, 0x8000_8200);
        word(&mut ram, TABLE, NODE);
        code(
            &mut ram,
            0x8000_8000,
            &[
                0x27bd_ffe8,
                0xafbf_0014,
                0x2404_0003,
                0x2405_0010,
                0x2409_0007,
                0x0c00_002c,
                0,
                0x8fbf_0014,
                0,
                0x27bd_0018,
                0x03e0_0008,
                0x2402_0001,
            ],
        );
        if early_return {
            code(&mut ram, 0x8000_8100, &[0x2409_0017, 0x0800_002c, 0]);
        } else {
            logger(&mut ram, 0x8000_8100, 1, 0);
        }
        logger(&mut ram, 0x8000_8200, 99, 0);
        kernel
            .begin_exception(
                &mut cpu,
                &mut ram,
                environment(MissingHook::ReturnFromException),
            )
            .unwrap();
        if !early_return {
            run(&mut kernel, &mut cpu, &mut ram).unwrap();
            assert_eq!(logs(&mut ram), [1, 99]);
            assert_eq!(cpu.pc(), CODE);
            assert_eq!(kernel.pending_callbacks(), 0);
            continue;
        }
        for _ in 0..30 {
            if Kernel::is_call(cpu.pc()) && cpu.register(9) == 0x17 {
                break;
            }
            if Kernel::is_call(cpu.pc()) {
                kernel.dispatch(&mut cpu, &mut ram).unwrap();
            } else {
                cpu.step(&mut ram).unwrap();
            }
        }
        assert_eq!(kernel.pending_callbacks(), 1);
        assert_eq!(cpu.register(9), 0x17);
        word(&mut ram, TCB + 0x88, CODE + 1);
        assert!(kernel.dispatch(&mut cpu, &mut ram).is_err());
        assert!(kernel.pending_exception());
        assert_eq!(kernel.pending_callbacks(), 1);
        word(&mut ram, TCB + 0x88, CODE);
        kernel.dispatch(&mut cpu, &mut ram).unwrap();
        assert_eq!(cpu.pc(), CODE);
        assert!(!kernel.pending_exception());
        assert_eq!(kernel.pending_callbacks(), 0);
        assert!(logs(&mut ram).is_empty());
    }
}

#[test]
fn malformed_graphs_abi_failures_nesting_and_limits_fail_without_claiming_completion() {
    for mode in 0..5 {
        let mut ram = Ram::default();
        fixture(&mut ram);
        let mut kernel = Kernel::default();
        let mut cpu = interrupted_cpu();
        word(&mut ram, TABLE, NODE);
        match mode {
            0 => node(&mut ram, NODE, NODE, 0, 0),
            1 => node(&mut ram, NODE, 0, 0x8000_8001, 0),
            2 => {
                node(&mut ram, NODE, 0, 0x8000_8000, 0);
                code(&mut ram, 0x8000_8000, &[0x03e0_0008, 0x2410_0001]);
            }
            3 => {
                word(&mut ram, TABLE, 0x8001_0000);
                for i in 0..4097 {
                    node(
                        &mut ram,
                        0x8001_0000 + i * 16,
                        if i == 4096 { 0 } else { 0x8001_0010 + i * 16 },
                        0,
                        0,
                    );
                }
            }
            _ => {
                node(&mut ram, NODE, 0, 0x8000_8000, 0);
                code(&mut ram, 0x8000_8000, &[0xffff_ffff]);
            }
        }
        let started = kernel.begin_exception(
            &mut cpu,
            &mut ram,
            environment(MissingHook::ReturnFromException),
        );
        if started.is_ok() {
            assert!(kernel
                .begin_exception(
                    &mut cpu,
                    &mut ram,
                    environment(MissingHook::ReturnFromException)
                )
                .is_err());
            assert!(run(&mut kernel, &mut cpu, &mut ram).is_err());
        }
        assert!(kernel.pending_exception());
        assert_ne!(cpu.pc(), CODE);
    }
    let mut ram = Ram::default();
    fixture(&mut ram);
    let mut kernel = Kernel::default();
    let mut cpu = interrupted_cpu();
    let memory = ram.bytes().to_vec();
    let before = format!("{cpu:?}");
    assert!(kernel
        .begin_exception(
            &mut cpu,
            &mut ram,
            Environment {
                stack_top: 3,
                ..environment(MissingHook::ReturnFromException)
            }
        )
        .is_err());
    assert_eq!(ram.bytes(), memory);
    assert_eq!(format!("{cpu:?}"), before);
    assert!(!kernel.pending_exception());
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE; original US handlers with explicit synthetic device state"]
fn original_us_priority_one_handlers_check_vblank_and_clear_sio_control() {
    use bof3_audio::machine::{bus::BusError, executable::Executable, profile::Profile};
    struct Device {
        ram: Ram,
        mask: u32,
        status: u32,
        reads: Vec<u32>,
        clears: usize,
    }
    impl Bus for Device {
        fn read(&mut self, a: u32, w: Width) -> Result<u32, BusError> {
            if w == Width::Word && matches!(a, 0x1f80_1070 | 0x1f80_1074) {
                self.reads.push(a);
                return Ok(if a == 0x1f80_1070 {
                    self.status
                } else {
                    self.mask
                });
            }
            self.ram.read(a, w)
        }
        fn write(&mut self, a: u32, w: Width, v: u32) -> Result<(), BusError> {
            if a == 0x1f80_104a && w == Width::Half && v == 0 {
                self.clears += 1;
                return Ok(());
            }
            self.ram.write(a, w, v)
        }
        fn write_masked(&mut self, a: u32, v: u32, lanes: u8) -> Result<(), BusError> {
            self.ram.write_masked(a, v, lanes)
        }
    }
    let exe =
        Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    Profile::identify(&exe).unwrap();
    for (mask, status) in [(2, 2), (3, 2), (3, 3)] {
        let mut bus = Device {
            ram: Ram::from_executable(&exe),
            mask,
            status,
            reads: Vec::new(),
            clears: 0,
        };
        fixture(&mut bus);
        node(&mut bus, NODE, 0, 0x8017_f31c, 0x8017_f2b4);
        word(&mut bus, TABLE + 8, NODE);
        let mut cpu = interrupted_cpu();
        let mut kernel = Kernel::default();
        kernel
            .begin_exception(
                &mut cpu,
                &mut bus,
                environment(MissingHook::ReturnFromException),
            )
            .unwrap();
        run(&mut kernel, &mut cpu, &mut bus).unwrap();
        assert_eq!(bus.clears, usize::from(mask & status & 1 != 0));
        assert_eq!(
            bus.reads,
            if mask & 1 != 0 {
                vec![0x1f80_1074, 0x1f80_1070]
            } else {
                vec![0x1f80_1074]
            }
        );
        assert_eq!(cpu.pc(), CODE);
        assert_eq!(cpu.cop0().status(), 0x401);
    }
}
