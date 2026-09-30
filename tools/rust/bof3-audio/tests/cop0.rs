use bof3_audio::machine::{
    bus::{Bus, Ram, Width},
    cpu::{Cpu, FaultKind},
    kernel::Kernel,
};

fn program(words: &[u32]) -> (Cpu, Ram) {
    let mut ram = Ram::default();
    for (index, word) in words.iter().enumerate() {
        ram.write(0x1000 + index as u32 * 4, Width::Word, *word)
            .unwrap();
    }
    (Cpu::new(0x8000_1000), ram)
}

#[test]
fn bios_can_disable_debug_control_but_cannot_enable_unmodeled_breakpoints() {
    for (register, value) in [
        (3, 0x8000_1000),
        (5, 0xa000_2000),
        (9, u32::MAX),
        (11, 0xffff_fffc),
    ] {
        let mut cop0 = bof3_audio::machine::cop0::Cop0::default();
        cop0.write(register, value).unwrap();
        assert_eq!(cop0.read(register).unwrap(), value);
        assert_eq!(cop0.read(7).unwrap(), 0);
    }
    // mtc0 zero,dcic; mfc0 v0,dcic; nop; mtc0 a0,dcic
    let (mut cpu, mut ram) = program(&[0x4080_3800, 0x4002_3800, 0, 0x4084_3800]);
    cpu.set_register(2, u32::MAX);
    cpu.set_register(4, 0x8000_0000);
    cpu.run_until(&mut ram, 0x8000_100c, 3).unwrap();
    assert_eq!(cpu.register(2), 0);
    assert!(matches!(
        cpu.step(&mut ram).unwrap_err().kind,
        FaultKind::UnsupportedSystemControl(_)
    ));
    assert_eq!(cpu.cop0().read(7).unwrap(), 0);
    assert_eq!(cpu.instructions(), 3);
}

#[test]
fn cop0_transfers_keep_load_delay_and_restrict_writable_cause_bits() {
    // mtc0 r1,sr; mfc0 r2,sr; addu r3,r2,zero; addu r4,r2,zero
    let (mut cpu, mut ram) = program(&[0x4081_6000, 0x4002_6000, 0x0040_1821, 0x0040_2021]);
    cpu.set_register(1, 0x401);
    cpu.set_register(2, 0x1234);
    for _ in 0..4 {
        cpu.step(&mut ram).unwrap();
    }
    assert_eq!(cpu.register(3), 0x1234);
    assert_eq!(cpu.register(4), 0x401);
    cpu.cop0_mut().set_external_interrupt(true);
    cpu.cop0_mut().write(13, u32::MAX).unwrap();
    assert_eq!(cpu.cop0().read(13).unwrap(), 0x700);
    cpu.cop0_mut().write(13, 0).unwrap();
    assert_eq!(cpu.cop0().read(13).unwrap(), 0x400);
    assert_eq!(cpu.cop0().read(15).unwrap(), 2);
    assert!(cpu.cop0_mut().write(14, 1).is_err());
    assert_eq!(cpu.cop0().read(7).unwrap(), 0);
    assert!(cpu.cop0().read(0).is_err());
    cpu.cop0_mut().write(6, 0).unwrap();
    assert_eq!(cpu.cop0().read(6).unwrap(), 0);
    assert!(cpu.cop0_mut().write(6, 1).is_err());
}

#[test]
fn exceptions_preserve_branch_direction_epc_target_and_status_stack() {
    for taken in [false, true] {
        // bne r1,zero,target; syscall
        let (mut cpu, mut ram) = program(&[0x1420_0003, 0x0000_000c]);
        cpu.set_register(1, u32::from(taken));
        cpu.cop0_mut().write(12, 0x401).unwrap();
        cpu.step(&mut ram).unwrap();
        let fault = cpu.step(&mut ram).unwrap_err();
        cpu.enter_exception(&fault).unwrap();
        assert_eq!(cpu.pc(), 0x8000_0080);
        assert_eq!(cpu.cop0().read(14).unwrap(), 0x8000_1000);
        assert_eq!(
            cpu.cop0().read(13).unwrap(),
            0x8000_0020 | (u32::from(taken) << 30)
        );
        assert_eq!(
            cpu.cop0().read(6).unwrap(),
            if taken { 0x8000_1010 } else { 0x8000_1008 }
        );
        assert_eq!(cpu.cop0().status(), 0x404);
        // jr r26; rfe: RFE pops status but the jump supplies the return PC.
        ram.write(0x80, Width::Word, 0x0340_0008).unwrap();
        ram.write(0x84, Width::Word, 0x4200_0010).unwrap();
        cpu.set_register(26, 0x8000_2000);
        cpu.step(&mut ram).unwrap();
        cpu.step(&mut ram).unwrap();
        assert_eq!(cpu.pc(), 0x8000_2000);
        assert_eq!(cpu.cop0().status(), 0x401);
    }
}

#[test]
fn alignment_and_interrupt_delivery_keep_distinct_exception_evidence() {
    let (mut cpu, mut ram) = program(&[0x8c01_0001]); // lw r1,1(zero)
    let fault = cpu.step(&mut ram).unwrap_err();
    cpu.enter_exception(&fault).unwrap();
    assert_eq!(cpu.cop0().read(8).unwrap(), 1);
    assert_eq!(cpu.cop0().read(13).unwrap(), 4 << 2);
    assert!(!cpu.take_interrupt(true));
    assert_eq!(cpu.cop0().read(13).unwrap(), (4 << 2) | 0x400);
    cpu.resume_at(0x8000_1000);
    cpu.cop0_mut().write(12, (1 << 22) | 0x401).unwrap();
    assert!(cpu.take_interrupt(true));
    assert_eq!(cpu.pc(), 0xbfc0_0180);
    assert_eq!(cpu.cop0().read(14).unwrap(), 0x8000_1000);
    assert_eq!(cpu.cop0().read(8).unwrap(), 1); // IRQ must not alter BadVaddr
    assert_eq!(cpu.cop0().read(13).unwrap(), 0x400);
    assert!(!cpu.take_interrupt(true)); // entry cleared current IE
    cpu.cop0_mut().write(12, 0x101).unwrap();
    cpu.cop0_mut().write(13, 0x100).unwrap();
    assert!(cpu.take_interrupt(false)); // software request independently masked
}

#[test]
fn critical_sections_preserve_registers_and_report_prior_enable_state() {
    for initial in [0, 1, 0x400, 0x401] {
        let (mut cpu, mut ram) = program(&[0xc, 0xc, 0xc]);
        let mut kernel = Kernel::default();
        for register in 1..32 {
            cpu.set_register(register, 0xabc0 + register as u32);
        }
        cpu.cop0_mut().write(12, initial).unwrap();
        cpu.set_register(4, 1);
        let fault = cpu.step(&mut ram).unwrap_err();
        kernel.dispatch_syscall(&mut cpu, &fault).unwrap();
        assert_eq!(cpu.register(2), u32::from(initial == 0x401));
        assert_eq!(cpu.cop0().status() & 0x401, 0);
        assert_eq!(cpu.pc(), 0x8000_1004);
        let fault = cpu.step(&mut ram).unwrap_err();
        kernel.dispatch_syscall(&mut cpu, &fault).unwrap();
        assert_eq!(cpu.register(2), 0); // already disabled
        cpu.set_register(4, 2);
        cpu.set_register(2, 0x5678);
        let fault = cpu.step(&mut ram).unwrap_err();
        kernel.dispatch_syscall(&mut cpu, &fault).unwrap();
        assert_eq!(cpu.register(2), 0x5678); // exit has no return value
        assert_eq!(cpu.cop0().status() & 0x401, 0x401);
        for register in 1..32 {
            if register != 2 && register != 4 {
                assert_eq!(cpu.register(register), 0xabc0 + register as u32);
            }
        }
    }
}

#[test]
fn unsupported_execution_and_syscalls_do_not_become_success() {
    let (mut cpu, mut ram) = program(&[0x4200_0001]); // unsupported TLB command
    let fault = cpu.step(&mut ram).unwrap_err();
    assert!(cpu.enter_exception(&fault).is_err());
    for status in [2, 1 << 16] {
        cpu.cop0_mut().write(12, status).unwrap();
        assert!(matches!(
            cpu.step(&mut ram).unwrap_err().kind,
            FaultKind::UnsupportedSystemControl(_)
        ));
    }
    for words in [[0xc, 0], [0x1000_0001, 0xc]] {
        let (mut cpu, mut ram) = program(&words);
        cpu.set_register(4, if words[0] == 0xc { 3 } else { 1 });
        if words[0] != 0xc {
            cpu.step(&mut ram).unwrap();
        }
        let fault = cpu.step(&mut ram).unwrap_err();
        assert!(Kernel::default()
            .dispatch_syscall(&mut cpu, &fault)
            .is_err());
        assert_eq!(cpu.pc(), fault.pc);
        assert_eq!(cpu.cop0().status(), 0);
        assert_eq!(cpu.cop0().read(13).unwrap(), 0);
    }
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_game_critical_section_wrappers_execute_through_syscalls() {
    use bof3_audio::machine::{executable::Executable, profile::Profile};
    let exe =
        Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    Profile::identify(&exe).unwrap();
    let mut ram = Ram::from_executable(&exe);
    let mut kernel = Kernel::default();
    let mut cpu = Cpu::new(0x8017_ee1c);
    cpu.set_register(31, 0x8000_1000);
    for (entry, enabled, result) in [
        (0x8017_ee1c, true, 0),
        (0x8017_ee0c, false, 1),
        (0x8017_ee0c, false, 0),
    ] {
        cpu.resume_at(entry);
        let mut returned = false;
        for _ in 0..10 {
            if cpu.pc() == 0x8000_1000 {
                returned = true;
                break;
            }
            if let Err(fault) = cpu.step(&mut ram) {
                kernel.dispatch_syscall(&mut cpu, &fault).unwrap();
            }
        }
        assert!(returned);
        assert_eq!(cpu.cop0().status() & 0x401, if enabled { 0x401 } else { 0 });
        assert_eq!(cpu.register(2), result);
    }
}
