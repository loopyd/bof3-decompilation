use bof3_audio::machine::{
    bus::{Bus, BusError, Ram, Width},
    cpu::Cpu,
    executable::Executable,
    kernel::Kernel,
    profile::Profile,
    thread_context::Context,
};
const PCB: u32 = 0x8000_3000;
const TCB: u32 = 0x8000_4000;
const CODE: u32 = 0x8000_1000;
fn word(ram: &mut Ram, address: u32, value: u32) {
    ram.write(address, Width::Word, value).unwrap();
}
fn read(ram: &mut Ram, address: u32) -> u32 {
    ram.read(address, Width::Word).unwrap()
}
fn tables(ram: &mut Ram) {
    for (a, v) in [
        (0x108, PCB),
        (0x10c, 4),
        (0x110, TCB),
        (0x114, 0x180),
        (PCB, TCB),
    ] {
        word(ram, a, v);
    }
    for i in 0..96 {
        word(ram, TCB + i * 4, 0xdead_beef);
    }
    word(ram, TCB, 0x4000);
    word(ram, TCB + 0xc0, 0x4000);
}
fn seeded(ram: &mut Ram) -> Cpu {
    let mut cpu = Cpu::new(CODE);
    for i in 1..32 {
        cpu.set_register(i, 0x1234_0000 + i as u32);
    }
    word(ram, CODE, 0x0100_0011); // mthi t0
    word(ram, CODE + 4, 0x0120_0013); // mtlo t1
    cpu.step(ram).unwrap();
    cpu.step(ram).unwrap();
    cpu.cop0_mut().write(12, 0x401).unwrap();
    cpu
}
fn return_call(kernel: &mut Kernel, cpu: &mut Cpu, ram: &mut impl Bus) -> bof3_audio::Result<()> {
    cpu.resume_at(0xb0);
    cpu.set_register(9, 0x17);
    cpu.set_register(31, 0x8000_9998);
    let result = kernel.dispatch(cpu, ram)?;
    assert_eq!(result.name, "ReturnFromException");
    Ok(())
}

#[test]
fn exception_context_preserves_opaque_words_and_restores_live_guest_registers() {
    let mut ram = Ram::default();
    tables(&mut ram);
    let mut cpu = seeded(&mut ram);
    let expected: Vec<_> = (0..32).map(|i| cpu.register(i)).collect();
    let (hi, lo) = (cpu.hi(), cpu.lo());
    assert!(cpu.take_interrupt(true));
    let cause = cpu.cop0().read(13).unwrap();
    let mut kernel = Kernel::default();
    assert_eq!(kernel.save_exception(&mut cpu, &mut ram).unwrap(), TCB);
    for i in [4, 8, 0x70, 0x9c, 0xbc] {
        assert_eq!(read(&mut ram, TCB + i), 0xdead_beef);
    }
    assert_eq!(read(&mut ram, TCB), 0x4000);
    assert_eq!(read(&mut ram, TCB + 0x88), CODE + 8);
    assert_eq!(read(&mut ram, TCB + 0x94), 0x404);
    assert_eq!(read(&mut ram, TCB + 0x98), cause);
    for i in 1..32 {
        cpu.set_register(i, 0);
    }
    // A guest handler can edit the stored return PC/registers, as a syscall does.
    word(&mut ram, TCB + 8 + 2 * 4, 99);
    word(&mut ram, TCB + 0x88, CODE + 0x20);
    word(&mut ram, TCB + 0x98, 0xffff_ffff); // diagnostic cause is not restored
    return_call(&mut kernel, &mut cpu, &mut ram).unwrap();
    for (i, &value) in expected.iter().enumerate() {
        assert_eq!(
            cpu.register(i),
            match i {
                2 => 99,
                26 => CODE + 0x20,
                _ => value,
            },
            "r{i}"
        );
    }
    assert_eq!((cpu.pc(), cpu.hi(), cpu.lo()), (CODE + 0x20, hi, lo));
    assert_eq!(cpu.cop0().status(), 0x401);
    assert_eq!(cpu.cop0().read(13).unwrap(), cause);
    word(&mut ram, CODE + 0x20, 0x2402_0007);
    cpu.step(&mut ram).unwrap();
    assert_eq!(cpu.register(2), 7);
}

#[test]
fn branch_delay_exception_restarts_saved_branch_and_loads_are_committed() {
    let mut ram = Ram::default();
    tables(&mut ram);
    word(&mut ram, CODE, 0x1000_0003); // beq zero,zero,CODE+16
    word(&mut ram, CODE + 4, 0x0000_000c); // syscall in delay slot
    let mut cpu = Cpu::new(CODE);
    cpu.cop0_mut().write(12, 0x401).unwrap();
    cpu.step(&mut ram).unwrap();
    let fault = cpu.step(&mut ram).unwrap_err();
    cpu.enter_exception(&fault).unwrap();
    let mut kernel = Kernel::default();
    kernel.save_exception(&mut cpu, &mut ram).unwrap();
    assert_eq!(read(&mut ram, TCB + 0x88), CODE);
    assert_eq!(read(&mut ram, TCB + 0x98) & 0xc000_007c, 0xc000_0020);
    return_call(&mut kernel, &mut cpu, &mut ram).unwrap();
    assert_eq!(cpu.pc(), CODE);
    word(&mut ram, CODE + 4, 0x2403_0055);
    cpu.step(&mut ram).unwrap();
    cpu.step(&mut ram).unwrap();
    assert_eq!((cpu.pc(), cpu.register(3)), (CODE + 16, 0x55));
    word(&mut ram, CODE + 16, 0x8c02_2000); // lw v0,0x2000(zero)
    word(&mut ram, 0x2000, 0x1122_3344);
    cpu.step(&mut ram).unwrap();
    assert!(cpu.take_interrupt(true));
    kernel.save_exception(&mut cpu, &mut ram).unwrap();
    assert_eq!(read(&mut ram, TCB + 16), 0x1122_3344);
}

#[test]
fn current_pointer_and_descriptors_are_reread_and_aliases_are_validated() {
    let mut ram = Ram::default();
    tables(&mut ram);
    let mut cpu = seeded(&mut ram);
    assert!(cpu.take_interrupt(true));
    let mut kernel = Kernel::default();
    kernel.save_exception(&mut cpu, &mut ram).unwrap();
    let bytes: Vec<_> = (0..0xc0 / 4).map(|i| read(&mut ram, TCB + i * 4)).collect();
    for (i, &value) in bytes.iter().enumerate() {
        word(&mut ram, TCB + 0xc0 + i as u32 * 4, value);
    }
    word(&mut ram, TCB + 0xc0 + 16, 777);
    word(&mut ram, PCB, 0xa000_40c0);
    word(&mut ram, 0x108, 0xa000_3000);
    word(&mut ram, 0x110, 0x4000);
    assert_eq!(Context::current(&mut ram).unwrap().address(), 0xa000_40c0);
    return_call(&mut kernel, &mut cpu, &mut ram).unwrap();
    assert_eq!(cpu.register(2), 777);
    for (address, value) in [
        (0x108, 0),
        (0x108, 0x110),
        (0x10c, 8),
        (0x110, PCB),
        (0x114, 0xc1),
        (0x114, 0x20_0000),
        (PCB, TCB + 4),
        (PCB, TCB + 0x180),
        (TCB, 0x1000),
    ] {
        tables(&mut ram);
        word(&mut ram, address, value);
        assert!(Context::current(&mut ram).is_err(), "{address:x}={value:x}");
    }
}

#[test]
fn invalid_return_and_read_fault_leave_cpu_unchanged_and_cop2_capture_rejects() {
    let mut ram = Ram::default();
    tables(&mut ram);
    let mut cpu = seeded(&mut ram);
    assert!(cpu.take_interrupt(true));
    let context = Context::current(&mut ram).unwrap();
    context.save_exception(&mut cpu, &mut ram).unwrap();
    let before = format!("{cpu:?}");
    for pc in [CODE + 1, 0xbfc0_0000, 0x8020_0000] {
        word(&mut ram, TCB + 0x88, pc);
        assert!(context.restore(&mut cpu, &mut ram).is_err());
        assert_eq!(format!("{cpu:?}"), before);
    }
    word(&mut ram, TCB + 0x88, CODE + 8);
    struct Failing<'a>(&'a mut Ram);
    impl Bus for Failing<'_> {
        fn read(&mut self, a: u32, w: Width) -> Result<u32, BusError> {
            if a == TCB + 0x94 {
                return Err(BusError {
                    address: a,
                    detail: "injected read failure".into(),
                });
            }
            self.0.read(a, w)
        }
        fn write(&mut self, a: u32, w: Width, v: u32) -> Result<(), BusError> {
            self.0.write(a, w, v)
        }
        fn write_masked(&mut self, a: u32, v: u32, lanes: u8) -> Result<(), BusError> {
            self.0.write_masked(a, v, lanes)
        }
    }
    assert!(context.restore(&mut cpu, &mut Failing(&mut ram)).is_err());
    assert_eq!(format!("{cpu:?}"), before);
    word(&mut ram, CODE + 8, 0x4a00_0000);
    let memory = ram.bytes().to_vec();
    assert!(context
        .save_exception(&mut cpu, &mut ram)
        .unwrap_err()
        .to_string()
        .contains("COP2"));
    assert_eq!(ram.bytes(), memory);
    cpu.resume_at(CODE);
    assert!(context.save_exception(&mut cpu, &mut ram).is_err());
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE; original US ReturnFromException wrapper and explicit guest TCB"]
fn original_us_wrapper_restores_current_thread_without_returning_to_its_caller() {
    let exe =
        Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    Profile::identify(&exe).unwrap();
    let mut ram = Ram::from_executable(&exe);
    tables(&mut ram);
    let mut cpu = seeded(&mut ram);
    assert!(cpu.take_interrupt(true));
    let mut kernel = Kernel::default();
    kernel.save_exception(&mut cpu, &mut ram).unwrap();
    let registers: Vec<_> = (0..32).map(|i| cpu.register(i)).collect();
    cpu.resume_at(0x8017_eddc);
    cpu.set_register(31, 0x8000_9998);
    cpu.run_until(&mut ram, 0xb0, 3).unwrap();
    assert_eq!(cpu.register(9), 0x17);
    kernel.dispatch(&mut cpu, &mut ram).unwrap();
    assert_eq!(cpu.pc(), CODE + 8);
    for (i, &value) in registers.iter().enumerate() {
        if i != 26 {
            assert_eq!(cpu.register(i), value, "r{i}");
        }
    }
    assert_eq!(cpu.instructions(), 5);
}

#[test]
fn exception_return_does_not_discard_pending_event_or_wait_continuations() {
    use bof3_audio::machine::events::{Table, CALLBACK, POLLING};
    for callback in [false, true] {
        let mut ram = Ram::default();
        tables(&mut ram);
        let mut cpu = seeded(&mut ram);
        assert!(cpu.take_interrupt(true));
        let mut kernel = Kernel::default();
        kernel.save_exception(&mut cpu, &mut ram).unwrap();
        word(&mut ram, 0x120, 0x8000_6000);
        word(&mut ram, 0x124, 0x1c);
        let events = Table::read(&mut ram).unwrap();
        let handle = events
            .open(
                &mut ram,
                3,
                16,
                if callback { CALLBACK } else { POLLING },
                CODE + 0x40,
            )
            .unwrap();
        events.enable(&mut ram, handle, true).unwrap();
        cpu.set_register(29, 0x8001_0000);
        cpu.set_register(31, CODE + 0x20);
        cpu.set_register(9, if callback { 7 } else { 0x0a });
        cpu.set_register(4, if callback { 3 } else { handle });
        cpu.set_register(5, 16);
        cpu.resume_at(0xb0);
        let call = kernel.dispatch(&mut cpu, &mut ram).unwrap();
        assert_eq!(call.waiting, !callback);
        assert_eq!(kernel.pending_callbacks(), usize::from(callback));
        assert!(return_call(&mut kernel, &mut cpu, &mut ram)
            .unwrap_err()
            .to_string()
            .contains("cannot be unwound"));
        cpu.resume_at(0x8000_0080);
        let memory = ram.bytes().to_vec();
        assert!(kernel.save_exception(&mut cpu, &mut ram).is_err());
        assert_eq!(ram.bytes(), memory);
        assert_eq!(kernel.pending_callbacks(), usize::from(callback));
    }
}
