use bof3_audio::{
    driver::trace::Trace,
    machine::{
        bus::{Bus, Width},
        cpu::Cpu,
        executable::Executable,
        interconnect::Interconnect,
    },
};

const BASE: u32 = 0x80010000;

fn fixture() -> (Executable, Cpu, Interconnect) {
    let words: [u32; 3] = [0x8c82fffc, 0x03200008, 0]; // lw v0,-4(a0); jr t9; nop
    let mut bytes = vec![0; 0x800];
    bytes[..8].copy_from_slice(b"PS-X EXE");
    bytes[0x10..0x14].copy_from_slice(&BASE.to_le_bytes());
    bytes[0x18..0x1c].copy_from_slice(&BASE.to_le_bytes());
    bytes[0x1c..0x20].copy_from_slice(&12u32.to_le_bytes());
    for word in words {
        bytes.extend(word.to_le_bytes());
    }
    let executable = Executable::from_bytes(bytes).unwrap();
    let bus = Interconnect::from_executable(&executable);
    let mut cpu = Cpu::new(BASE);
    cpu.set_register(4, 0x80020004);
    cpu.set_register(25, BASE + 8);
    (executable, cpu, bus)
}

#[test]
fn pre_execution_observation_retains_addresses_and_leaves_guest_unchanged() {
    let (exe, mut cpu, mut bus) = fixture();
    let mut trace = Trace::default();
    let ram = bus.ram().bytes().to_vec();
    trace.observe_cpu(&exe, "load", &cpu, &bus).unwrap();
    assert_eq!(cpu.pc(), BASE);
    assert_eq!(cpu.instructions(), 0);
    assert_eq!(cpu.register(4), 0x80020004);
    assert_eq!(bus.ram().bytes(), ram);
    assert_eq!(trace.memory[&(BASE, 0x80020000, 4, "read")], 1);
    cpu.step(&mut bus).unwrap();
    trace.observe_cpu(&exe, "jump", &cpu, &bus).unwrap();
    assert_eq!(trace.indirect[&(BASE + 4, BASE + 8)], 1);
    assert_eq!(trace.pcs.len(), 2);
    assert!(trace.services.is_empty());
}

#[test]
fn changed_original_instruction_is_rejected_before_recording_it() {
    let (exe, cpu, mut bus) = fixture();
    bus.write(BASE, Width::Word, 0).unwrap();
    let mut trace = Trace::default();
    assert!(trace
        .observe_cpu(&exe, "changed", &cpu, &bus)
        .unwrap_err()
        .to_string()
        .contains("original instruction changed"));
    assert!(trace.pcs.is_empty());
}

#[test]
fn host_call_boundaries_do_not_invent_kernel_reentry_edges() {
    let (exe, cpu, bus) = fixture();
    let mut trace = Trace::default();
    let mut service = Cpu::new(0xb0);
    service.set_register(9, 0x17);
    trace.observe_cpu(&exe, "kernel", &service, &bus).unwrap();
    assert_eq!(trace.services[&("kernel".into(), 0xb0, 0x17)], 1);
    trace.begin_call();
    trace.observe_cpu(&exe, "host call", &cpu, &bus).unwrap();
    assert!(trace.reentries.is_empty());
    trace.observe_cpu(&exe, "kernel", &service, &bus).unwrap();
    trace.observe_cpu(&exe, "guest return", &cpu, &bus).unwrap();
    assert_eq!(trace.reentries[&(0xb0, BASE)], 1);
}
