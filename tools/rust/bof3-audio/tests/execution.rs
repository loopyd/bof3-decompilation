use bof3_audio::machine::{
    bus::{Bus, Ram, Width},
    cpu::Cpu,
    execution::Execution,
    interconnect::Interconnect,
    spu_clock,
};

fn fixture() -> Execution {
    let mut ram = Ram::default();
    // addu v0,a0,a1; jr ra; addiu v0,v0,1
    for (i, word) in [0x00851021, 0x03e00008, 0x24420001].into_iter().enumerate() {
        ram.write(0x2000 + i as u32 * 4, Width::Word, word).unwrap();
    }
    Execution::new(
        Cpu::new(0),
        Interconnect::from_ram(ram),
        spu_clock::Model::EmulatorReference,
    )
}

#[test]
fn calls_keep_guest_state_and_reject_invalid_requests_before_entering() {
    let mut execution = fixture();
    execution.cpu.set_register(29, 0x80010000);
    for (entry, limit) in [
        (0x2001, 10),
        (0x80001000, 10),
        (0x2000, 0),
        (0x2000, 10_000_001),
    ] {
        assert!(execution.call(entry, [0; 4], limit).is_err());
        assert_eq!(execution.cpu.pc(), 0);
        assert_eq!(execution.clock.ticks(), 0);
    }
    let call = execution.call(0x2000, [10, 20, 0, 0], 3).unwrap();
    assert_eq!(
        (
            call.result,
            call.instructions,
            call.syscalls,
            call.interrupts
        ),
        (31, 3, 0, 0)
    );
    assert_eq!(execution.cpu.register(29), 0x80010000);
    assert_eq!(execution.clock.ticks(), 6);
    assert_eq!(
        execution.call(0x2000, [40, 50, 0, 0], 3).unwrap().result,
        91
    );
    assert_eq!(execution.clock.ticks(), 12);
    assert!(execution.call(0x2000, [1; 4], 1).is_err());
    assert_eq!(execution.cpu.pc(), 0x2004);
}

#[test]
fn pending_irq_runs_guest_vector_and_rfe_before_resuming_the_call() {
    let mut execution = fixture();
    // Synthetic RAM exception handler acknowledges I_STAT, loads EPC and RFE.
    for (i, word) in [
        0x3c081f80, 0xa5001070, 0x401a7000, 0, 0x03400008, 0x42000010,
    ]
    .into_iter()
    .enumerate()
    {
        execution
            .bus
            .write(0x80 + i as u32 * 4, Width::Word, word)
            .unwrap();
    }
    execution.cpu.cop0_mut().write(12, 0x401).unwrap();
    execution.bus.interrupts_mut().set_mask(1);
    execution.bus.set_vblank(true);
    let mut observed = Vec::new();
    let call = execution
        .call_observed(0x2000, [5, 6, 0, 0], 20, &mut |state| {
            observed.push(state.cpu.pc());
            Ok(())
        })
        .unwrap();
    assert_eq!(
        observed,
        [
            0x80000080, 0x80000084, 0x80000088, 0x8000008c, 0x80000090, 0x80000094, 0x2000, 0x2004,
            0x2008
        ]
    );
    assert_eq!(call.result, 12);
    assert_eq!(call.interrupts, 1);
    assert_eq!(call.instructions, 9);
    assert_eq!(execution.cpu.cop0().status(), 0x401);
    assert_eq!(execution.cpu.cop0().read(14).unwrap(), 0x2000);
    assert!(!execution.bus.interrupts().pending());
}
