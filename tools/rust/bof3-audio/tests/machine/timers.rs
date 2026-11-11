use bof3_audio::machine::{interrupts::Interrupts, timers::Timers};

#[test]
fn target_reset_has_two_zero_clocks_and_flags_clear_only_on_mode_read() {
    let mut timers = Timers::default();
    let mut irq = Interrupts::default();
    timers.write(0, 8, 1, &mut irq).unwrap();
    timers.write(0, 4, 8 | 16 | 64, &mut irq).unwrap();
    for expected in [0, 1, 0, 0, 1] {
        timers.advance_cpu(1, &mut irq).unwrap();
        assert_eq!(timers.read(0, 0).unwrap(), expected);
    }
    assert_eq!(irq.status() & (1 << 4), 1 << 4);
    assert_ne!(timers.read(0, 4).unwrap() & (1 << 11), 0);
    assert_eq!(timers.read(0, 4).unwrap() & (1 << 11), 0);
}

#[test]
fn one_shot_suppresses_later_irqs_without_stopping_counter() {
    let mut timers = Timers::default();
    let mut irq = Interrupts::default();
    timers.write(0, 8, 1, &mut irq).unwrap();
    timers.write(0, 4, 8 | 16, &mut irq).unwrap();
    timers.advance_cpu(2, &mut irq).unwrap();
    assert_eq!(irq.status(), 1 << 4);
    irq.acknowledge(0);
    timers.advance_cpu(3, &mut irq).unwrap();
    assert_eq!(timers.read(0, 0).unwrap(), 1);
    assert_eq!(irq.status(), 0);
}

#[test]
fn toggle_mode_only_rising_output_edges_reach_interrupt_controller() {
    let mut timers = Timers::default();
    let mut irq = Interrupts::default();
    timers.write(0, 8, 1, &mut irq).unwrap();
    timers.write(0, 4, 8 | 16 | 64 | 128, &mut irq).unwrap();
    timers.advance_cpu(2, &mut irq).unwrap();
    assert_eq!(irq.status(), 1 << 4);
    assert_eq!(timers.read(0, 4).unwrap() & 0x400, 0);
    irq.acknowledge(0);
    timers.advance_cpu(3, &mut irq).unwrap();
    assert_eq!(irq.status(), 0);
    assert_eq!(timers.read(0, 4).unwrap() & 0x400, 0x400);
    timers.advance_cpu(3, &mut irq).unwrap();
    assert_eq!(irq.status(), 1 << 4);
}

#[test]
fn clock_sources_prescaling_and_blank_gate_inputs_are_independent() {
    let mut timers = Timers::default();
    let mut irq = Interrupts::default();
    timers.write(0, 4, 0x100, &mut irq).unwrap();
    timers.write(1, 4, 0x100, &mut irq).unwrap();
    timers.write(2, 4, 0x200, &mut irq).unwrap();
    timers.advance_cpu(16, &mut irq).unwrap();
    assert_eq!(timers.read(0, 0).unwrap(), 0);
    assert_eq!(timers.read(1, 0).unwrap(), 0);
    assert_eq!(timers.read(2, 0).unwrap(), 1);
    timers.advance_dotclocks(10, &mut irq).unwrap();
    assert_eq!(timers.read(0, 0).unwrap(), 9);
    for _ in 0..3 {
        timers.set_hblank(true, &mut irq).unwrap();
        timers.set_hblank(false, &mut irq).unwrap();
    }
    assert_eq!(timers.read(1, 0).unwrap(), 2);
    timers.write(2, 4, 1, &mut irq).unwrap(); // Sync mode 0 stops timer 2.
    timers.advance_cpu(100, &mut irq).unwrap();
    assert_eq!(timers.read(2, 0).unwrap(), 0);
}

#[test]
fn sync_mode_three_waits_for_first_blank_then_runs_freely() {
    let mut timers = Timers::default();
    let mut irq = Interrupts::default();
    timers.write(0, 4, 7, &mut irq).unwrap();
    timers.advance_cpu(20, &mut irq).unwrap();
    assert_eq!(timers.read(0, 0).unwrap(), 0);
    timers.set_hblank(true, &mut irq).unwrap();
    timers.set_hblank(false, &mut irq).unwrap();
    timers.advance_cpu(20, &mut irq).unwrap();
    assert_eq!(timers.read(0, 0).unwrap(), 19);
}

#[test]
fn ffff_irq_and_wrap_are_separate_from_target_reset() {
    let mut timers = Timers::default();
    let mut irq = Interrupts::default();
    timers.write(2, 4, 32, &mut irq).unwrap();
    timers.advance_cpu(1, &mut irq).unwrap();
    timers.write(2, 0, 0xfffe, &mut irq).unwrap();
    timers.advance_cpu(1, &mut irq).unwrap();
    assert_eq!(timers.read(2, 0).unwrap(), 0xffff);
    assert_eq!(irq.status(), 1 << 6);
    assert_ne!(timers.read(2, 4).unwrap() & (1 << 12), 0);
    timers.advance_cpu(1, &mut irq).unwrap();
    assert_eq!(timers.read(2, 0).unwrap(), 0);
    timers.advance_cpu(1, &mut irq).unwrap();
    assert_eq!(timers.read(2, 0).unwrap(), 1);
}

#[test]
fn unverified_target_zero_reset_timing_is_explicit() {
    let mut timers = Timers::default();
    let mut irq = Interrupts::default();
    timers.write(0, 4, 8, &mut irq).unwrap();
    assert!(timers
        .advance_cpu(1, &mut irq)
        .unwrap_err()
        .to_string()
        .contains("target-zero"));
}
