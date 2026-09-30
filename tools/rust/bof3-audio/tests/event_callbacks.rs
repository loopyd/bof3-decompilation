use bof3_audio::machine::{
    bus::{Bus, Ram, Width},
    cpu::Cpu,
    events::{Table, BUSY, CALLBACK, POLLING, READY},
    kernel::Kernel,
};
const STOP: u32 = 0x8001_0000;
const FIRST: u32 = 0x8001_1000;
const SECOND: u32 = 0x8001_2000;
const STACK: u32 = 0x8002_0000;

fn words(ram: &mut Ram, address: u32, code: &[u32]) {
    for (i, word) in code.iter().enumerate() {
        ram.write(address + i as u32 * 4, Width::Word, *word)
            .unwrap();
    }
}
fn fixture() -> (Cpu, Ram, Kernel, Table) {
    let mut ram = Ram::default();
    ram.write(0x120, Width::Word, 0x8000_8000).unwrap();
    ram.write(0x124, Width::Word, 8 * 28).unwrap();
    let table = Table::read(&mut ram).unwrap();
    let mut cpu = Cpu::new(0xb0);
    cpu.set_register(4, 1);
    cpu.set_register(5, 16);
    cpu.set_register(9, 7);
    cpu.set_register(29, STACK);
    cpu.set_register(31, STOP);
    ram.write(STACK, Width::Word, 0xdead_beef).unwrap();
    (cpu, ram, Kernel::default(), table)
}
fn event(table: &Table, ram: &mut Ram, class: u32, mode: u32, handler: u32, enabled: bool) -> u32 {
    let id = table.open(ram, class, 16, mode, handler).unwrap();
    table.enable(ram, id, enabled).unwrap();
    id
}
// Returns false for a pending BIOS wait. Every instruction and HLE transition
// consumes the same caller-supplied bound; callbacks never execute in a host loop.
fn run(
    cpu: &mut Cpu,
    ram: &mut Ram,
    kernel: &mut Kernel,
    limit: usize,
) -> bof3_audio::Result<bool> {
    for _ in 0..limit {
        if cpu.pc() == STOP {
            if kernel.pending_callbacks() != 0 {
                return Err("nonlocal callback return".into());
            }
            return Ok(true);
        }
        if Kernel::is_call(cpu.pc()) {
            if kernel.dispatch(cpu, ram)?.waiting {
                return Ok(false);
            }
        } else {
            cpu.step(ram)?;
        }
    }
    Err("test execution limit".into())
}

#[test]
fn callbacks_run_in_order_and_observe_live_later_event_changes() {
    let (mut cpu, mut ram, mut kernel, table) = fixture();
    event(&table, &mut ram, 1, CALLBACK, FIRST, true);
    event(&table, &mut ram, 1, CALLBACK, SECOND, false);
    let skipped = event(&table, &mut ram, 1, CALLBACK, SECOND + 0x100, true);
    // Enable record 1 and close record 2, then redirect the table descriptor.
    // This delivery retains its table range but reads later records live.
    words(
        &mut ram,
        FIRST,
        &[
            0x3c08_8001,
            0x2409_2000,
            0xad09_8020,
            0xad00_803c,
            0x3c0a_8003,
            0xac0a_0120,
            0x03e0_0008,
            0,
        ],
    );
    // Write result and the callback argument home area (in its return delay slot).
    words(
        &mut ram,
        SECOND,
        &[0x2408_002a, 0xac08_5000, 0x03e0_0008, 0xafa8_0000],
    );
    words(&mut ram, SECOND + 0x100, &[0x0000_000d]); // would fault if wrongly preselected
    assert!(run(&mut cpu, &mut ram, &mut kernel, 100).unwrap());
    assert_eq!(ram.read(0x5000, Width::Word).unwrap(), 42);
    assert_eq!(ram.read(STACK, Width::Word).unwrap(), 0xdead_beef);
    assert_eq!(ram.read(STACK - 16, Width::Word).unwrap(), 42);
    assert_eq!(
        ram.read(table.address(skipped).unwrap() + 4, Width::Word)
            .unwrap(),
        0
    );
    assert_eq!((cpu.register(29), cpu.register(31)), (STACK, STOP));
}

fn nested_code(class: u16) -> Vec<u32> {
    vec![
        0x27bd_ffe8,
        0xafbf_0014,
        0x2404_0000 | u32::from(class),
        0x2405_0010,
        0x2409_0007,
        0x0c00_002c,
        0,
        0x8fbf_0014,
        0x27bd_0018,
        0x03e0_0008,
        0,
    ]
}

#[test]
fn guest_nested_delivery_returns_to_outer_callback_and_then_original_caller() {
    let (mut cpu, mut ram, mut kernel, table) = fixture();
    event(&table, &mut ram, 1, CALLBACK, FIRST, true);
    event(&table, &mut ram, 2, CALLBACK, SECOND, true);
    words(&mut ram, FIRST, &nested_code(2));
    words(
        &mut ram,
        SECOND,
        &[0x2408_007b, 0xac08_5000, 0x03e0_0008, 0],
    );
    assert!(run(&mut cpu, &mut ram, &mut kernel, 100).unwrap());
    assert_eq!(ram.read(0x5000, Width::Word).unwrap(), 123);
    assert_eq!(kernel.pending_callbacks(), 0);
    assert_eq!((cpu.register(29), cpu.register(31)), (STACK, STOP));
}

#[test]
fn callback_wait_can_yield_and_resume_without_losing_delivery() {
    let (mut cpu, mut ram, mut kernel, table) = fixture();
    event(&table, &mut ram, 1, CALLBACK, FIRST, true);
    let polled = event(&table, &mut ram, 2, POLLING, 0, true);
    // Tail-call WaitEvent with the existing callback return address.
    words(
        &mut ram,
        FIRST,
        &[0x3c04_f100, 0x3484_0001, 0x2409_000a, 0x0800_002c, 0],
    );
    assert!(!run(&mut cpu, &mut ram, &mut kernel, 100).unwrap());
    assert_eq!(kernel.pending_callbacks(), 1);
    assert_eq!(cpu.pc() & 0x1fff_ffff, 0xb0);
    assert_eq!(table.delivery(2, 16).next_callback(&mut ram).unwrap(), None);
    assert_eq!(
        ram.read(table.address(polled).unwrap() + 4, Width::Word)
            .unwrap(),
        READY
    );
    assert!(run(&mut cpu, &mut ram, &mut kernel, 100).unwrap());
    assert_eq!(
        ram.read(table.address(polled).unwrap() + 4, Width::Word)
            .unwrap(),
        BUSY
    );
    assert_eq!(cpu.register(2), 1);
}

#[test]
fn bad_callbacks_and_recursive_delivery_fail_explicitly() {
    for (handler, stack, code, expected) in [
        (FIRST + 1, STACK, vec![0], "unaligned"),
        (FIRST, 0, vec![0], "guest stack"),
        (
            FIRST,
            STACK,
            vec![0x2610_0001, 0x03e0_0008, 0],
            "callee-saved",
        ),
        (FIRST, STACK, nested_code(1), "nesting limit"),
        (FIRST, STACK, vec![0x0000_000d], "Break"),
    ] {
        let (mut cpu, mut ram, mut kernel, table) = fixture();
        cpu.set_register(29, stack);
        event(&table, &mut ram, 1, CALLBACK, handler, true);
        words(&mut ram, FIRST, &code);
        let error = run(&mut cpu, &mut ram, &mut kernel, 4096)
            .unwrap_err()
            .to_string();
        assert!(error.contains(expected), "{error}");
        assert!(kernel.pending_callbacks() > 0);
    }
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_game_registration_routine_executes_as_a_guest_callback() {
    use bof3_audio::machine::{executable::Executable, profile::Profile};
    let exe =
        Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    Profile::identify(&exe).unwrap();
    let (mut cpu, _, mut kernel, _) = fixture();
    let mut ram = Ram::from_executable(&exe);
    ram.write(0x120, Width::Word, 0x8000_8000).unwrap();
    ram.write(0x124, Width::Word, 8 * 28).unwrap();
    let table = Table::read(&mut ram).unwrap();
    // Synthetic event wiring: this does not claim the game registers this routine
    // as a BIOS handler. The supplied function body and its callees stay original.
    event(&table, &mut ram, 1, CALLBACK, 0x8016_1dc8, true);
    assert!(run(&mut cpu, &mut ram, &mut kernel, 2000).unwrap());
    for (slot, entry) in [
        (0x8018_db58, 0x8016_a974),
        (0x8018_db5c, 0x8016_aa5c),
        (0x8018_db60, 0x8016_a4a4),
        (0x8018_db64, 0x8016_a79c),
        (0x8018_db68, 0x8016_a55c),
    ] {
        assert_eq!(ram.read(slot, Width::Word).unwrap(), entry);
    }
}
