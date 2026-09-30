use bof3_audio::machine::{
    bus::{Bus, Ram, Width},
    cpu::{Cpu, FaultKind},
    exception_chains::Table,
    executable::Executable,
    kernel::Kernel,
    profile::Profile,
};

const BASE: u32 = 0x8000_3000;
const RETURN: u32 = 0x8000_1000;

fn word(ram: &mut Ram, address: u32, value: u32) {
    ram.write(address, Width::Word, value).unwrap();
}
fn read(ram: &mut Ram, address: u32) -> u32 {
    ram.read(address, Width::Word).unwrap()
}
fn table(ram: &mut Ram) -> Table {
    word(ram, 0x100, BASE);
    word(ram, 0x104, 32);
    for priority in 0..4 {
        word(ram, BASE + priority * 8, 0);
        word(ram, BASE + priority * 8 + 4, 0xabcd0000 + priority);
    }
    Table::read(ram).unwrap()
}
fn node(ram: &mut Ram, address: u32) {
    for (i, value) in [0xfeed_face, 0x8001_2340, 0x8005_6780, 0x1122_3344]
        .into_iter()
        .enumerate()
    {
        word(ram, address + i as u32 * 4, value);
    }
}

#[test]
fn four_priority_chains_keep_exact_aliases_and_preserve_detached_nodes() {
    let mut ram = Ram::default();
    let chains = table(&mut ram);
    for priority in 0..4 {
        let first = 0xa000_4000 + priority * 32;
        let second = 0x0000_4010 + priority * 32;
        node(&mut ram, first);
        node(&mut ram, second);
        chains.enqueue(&mut ram, priority, first).unwrap();
        chains.enqueue(&mut ram, priority, second).unwrap();
        assert_eq!(read(&mut ram, BASE + priority * 8), second);
        assert_eq!(read(&mut ram, second), first);
        assert_eq!(read(&mut ram, first), 0);
        let before = ram.bytes().to_vec();
        assert!(chains
            .dequeue_head(&mut ram, priority, first)
            .unwrap_err()
            .to_string()
            .contains("non-head"));
        assert_eq!(ram.bytes(), before);
        assert_eq!(
            chains.dequeue_head(&mut ram, priority, second).unwrap(),
            second
        );
        assert_eq!(read(&mut ram, second), first); // removal does not zero next
        assert_eq!(
            chains.dequeue_head(&mut ram, priority, first).unwrap(),
            first
        );
        assert_eq!(read(&mut ram, BASE + priority * 8), 0);
        assert_eq!(
            read(&mut ram, BASE + priority * 8 + 4),
            0xabcd0000 + priority
        );
        for address in [first, second] {
            assert_eq!(read(&mut ram, address + 4), 0x8001_2340);
            assert_eq!(read(&mut ram, address + 8), 0x8005_6780);
            assert_eq!(read(&mut ram, address + 12), 0x1122_3344);
        }
    }
}

fn invoke(
    kernel: &mut Kernel,
    cpu: &mut Cpu,
    ram: &mut Ram,
    service: u32,
    priority: u32,
    address: u32,
) -> bof3_audio::Result<()> {
    cpu.resume_at(0xa000_00c0);
    cpu.set_register(9, service);
    cpu.set_register(4, priority);
    cpu.set_register(5, address);
    cpu.set_register(31, RETURN);
    let call = kernel.dispatch(cpu, ram)?;
    assert!(!call.waiting);
    assert!(call.callback.is_none());
    Ok(())
}

#[test]
fn kernel_services_observe_guest_edits_and_descriptor_rebinding() {
    let mut ram = Ram::default();
    table(&mut ram);
    let mut cpu = Cpu::new(RETURN);
    let mut kernel = Kernel::default();
    let address = 0x8000_4000;
    node(&mut ram, address);
    invoke(&mut kernel, &mut cpu, &mut ram, 2, 3, address).unwrap();
    assert_eq!((cpu.pc(), cpu.register(2)), (RETURN, 0));
    // A BIOS pointer comparison is raw equality, not physical alias equality.
    assert!(invoke(&mut kernel, &mut cpu, &mut ram, 3, 3, 0xa000_4000).is_err());
    assert_eq!((cpu.pc(), cpu.register(2)), (0xa000_00c0, 0));
    word(&mut ram, 0x100, 0x8000_5000);
    assert!(invoke(&mut kernel, &mut cpu, &mut ram, 3, 3, address).is_err());
    word(&mut ram, 0x8000_5018, address);
    word(&mut ram, address, 0x8000_4100);
    node(&mut ram, 0x8000_4100);
    word(&mut ram, 0x8000_4100, 0);
    invoke(&mut kernel, &mut cpu, &mut ram, 3, 3, address).unwrap();
    assert_eq!((cpu.pc(), cpu.register(2)), (RETURN, address));
    assert_eq!(read(&mut ram, 0x8000_5018), 0x8000_4100);
    assert_eq!(read(&mut ram, BASE + 24), address); // original table untouched
}

#[test]
fn malformed_descriptors_nodes_cycles_and_duplicates_fail_before_mutation() {
    let mut ram = Ram::default();
    assert!(Table::read(&mut ram).is_err());
    for (base, size) in [
        (BASE + 1, 32),
        (BASE, 24),
        (BASE, 40),
        (0x801ffff0, 32),
        (0x1f801c00, 32),
        (0x100, 32),
    ] {
        word(&mut ram, 0x100, base);
        word(&mut ram, 0x104, size);
        assert!(Table::read(&mut ram).is_err());
    }
    let chains = table(&mut ram);
    node(&mut ram, 0x80004000);
    chains.enqueue(&mut ram, 0, 0x80004000).unwrap();
    for (priority, address) in [
        (4, 0x5000),
        (0, 0),
        (0, 0x5001),
        (0, 0x1f801c00),
        (0, 0x801ffff4),
        (0, BASE),
        (0, 0x80000100),
        (0, 0xa0004000),
        (0, 0x4004),
    ] {
        let before = ram.bytes().to_vec();
        assert!(chains.enqueue(&mut ram, priority, address).is_err());
        assert_eq!(ram.bytes(), before);
    }
    for next in [0xa0004000, 0x80004004, 0x1f801c00] {
        word(&mut ram, 0x80004000, next);
        let before = ram.bytes().to_vec();
        assert!(chains.dequeue_head(&mut ram, 0, 0x80004000).is_err());
        assert!(chains.enqueue(&mut ram, 0, 0x5000).is_err());
        assert_eq!(ram.bytes(), before);
    }
}

#[test]
fn traversal_limit_stops_corrupt_guest_state_without_host_recursion_or_writes() {
    let mut ram = Ram::default();
    let chains = table(&mut ram);
    word(&mut ram, BASE, 0x10000);
    for i in 0..4097 {
        word(
            &mut ram,
            0x10000 + i * 16,
            if i == 4096 { 0 } else { 0x10010 + i * 16 },
        );
    }
    let before = ram.bytes().to_vec();
    assert!(chains
        .enqueue(&mut ram, 0, 0x30000)
        .unwrap_err()
        .to_string()
        .contains("4096-node"));
    assert_eq!(ram.bytes(), before);
    // The maximum is inclusive for an existing chain, but a successful insert
    // must never create a chain that the next operation cannot traverse.
    word(&mut ram, 0x10000 + 4095 * 16, 0);
    let before = ram.bytes().to_vec();
    assert!(chains
        .enqueue(&mut ram, 0, 0x30000)
        .unwrap_err()
        .to_string()
        .contains("would exceed"));
    assert_eq!(ram.bytes(), before);
    assert_eq!(chains.dequeue_head(&mut ram, 0, 0x10000).unwrap(), 0x10000);
    chains.enqueue(&mut ram, 0, 0x30000).unwrap();
    assert_eq!(chains.dequeue_head(&mut ram, 0, 0x30000).unwrap(), 0x30000);
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_bios_wrappers_and_registered_head_removal_execute_unchanged() {
    let exe =
        Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    Profile::identify(&exe).unwrap();
    let mut ram = Ram::from_executable(&exe);
    table(&mut ram); // Synthetic table location; not asserted as original BIOS boot.
    let mut kernel = Kernel::default();
    let mut total = 0;
    for (entry, argument, expected) in [
        (0x8017f39c, 0x80004000, 0),
        (0x8017f3ac, 0x80004000, 0x80004000),
    ] {
        let mut cpu = Cpu::new(entry);
        cpu.set_register(4, 2);
        cpu.set_register(5, argument);
        cpu.set_register(31, RETURN);
        while cpu.pc() != RETURN {
            if Kernel::is_call(cpu.pc()) {
                kernel.dispatch(&mut cpu, &mut ram).unwrap();
            } else {
                cpu.step(&mut ram).unwrap();
            }
            total += 1;
            assert!(total < 50);
        }
        assert_eq!(cpu.register(2), expected);
    }
    // Original caller requests priority 1, node 0x8018DB40. Only its valid head
    // context is supplied; the routine and critical-section wrappers are intact.
    word(&mut ram, BASE + 8, 0x8018db40);
    word(&mut ram, 0x8018db40, 0x80004000);
    let mut cpu = Cpu::new(0x8017f27c);
    cpu.set_register(29, exe.stack_pointer().unwrap());
    cpu.set_register(31, RETURN);
    cpu.set_register(16, 0x12345678);
    cpu.cop0_mut().write(12, 0x401).unwrap();
    while cpu.pc() != RETURN {
        assert!(total < 100);
        if Kernel::is_call(cpu.pc()) {
            kernel.dispatch(&mut cpu, &mut ram).unwrap();
        } else {
            match cpu.step(&mut ram) {
                Ok(_) => {}
                Err(fault) if fault.kind == FaultKind::Syscall => {
                    kernel.dispatch_syscall(&mut cpu, &fault).unwrap();
                }
                Err(fault) => panic!("{fault}"),
            }
        }
        total += 1;
    }
    assert_eq!(read(&mut ram, BASE + 8), 0x80004000);
    assert_eq!(read(&mut ram, 0x8018db40), 0x80004000);
    assert_eq!(cpu.register(2), 1);
    assert_eq!(cpu.register(16), 0x12345678);
    assert_eq!(cpu.register(29), exe.stack_pointer().unwrap());
    assert_eq!(cpu.cop0().status() & 0x401, 0x401);
    eprintln!("Original US enqueue/dequeue wrappers and priority-1 removal: {total} CPU/kernel transitions; synthetic ExCB context, no dispatch/timing acceptance");
}
