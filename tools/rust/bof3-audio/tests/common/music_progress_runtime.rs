use super::{fixture, prepare, BASE, DATA};
use bof3_audio::machine::{
    bus::{Bus, Width},
    cpu::Cpu,
};

#[test]
#[ignore = "requires BOF3_AUDIO_EXE; observes original loop/end instructions"]
fn original_handlers_report_infinite_boundaries_without_counting_finite_loops() {
    use bof3_audio::machine::{
        execution::Execution,
        interconnect::Interconnect,
        music_progress::{Progress, Stop},
        spu_clock,
    };
    let mut ram = fixture();
    let record = prepare(&mut ram, 1, 2, &[0; 32]);
    ram.write(record + 0x3c, Width::Byte, 255).unwrap();
    ram.write(record + 0x46, Width::Half, 1).unwrap();
    ram.write(0x8018_e264, Width::Byte, 0).unwrap();
    let mut execution = Execution::new(
        Cpu::new(0),
        Interconnect::from_ram(ram),
        spu_clock::Model::EmulatorReference,
    );
    execution.cpu.set_register(29, 0x801f_ff00);
    let mut progress = Progress::new(1, 2, record, Some(2)).unwrap();
    let mut other = Progress::new(0, 0, BASE, Some(1)).unwrap();
    let call =
        |execution: &mut Execution, progress: &mut Progress, other: &mut Progress, entry, args| {
            execution
                .call_observed(entry, args, 2000, &mut |e| {
                    progress.observe(e)?;
                    other.observe(e)
                })
                .unwrap();
        };
    call(
        &mut execution,
        &mut progress,
        &mut other,
        0x8016_a278,
        [1, 2, 20, 0],
    );
    assert_eq!(progress.loop_starts, 1);
    assert_eq!(progress.saved_loop_cursor, Some(DATA + 1));
    // The finite branch jumps too, but must not consume the host's loop limit.
    execution.bus.write(record + 0x28, Width::Byte, 2).unwrap();
    call(
        &mut execution,
        &mut progress,
        &mut other,
        0x8016_a278,
        [1, 2, 30, 0],
    );
    assert_eq!(execution.bus.read(record + 0x28, Width::Byte).unwrap(), 1);
    assert_eq!(progress.total_infinite_traversals, 0);
    execution
        .bus
        .write(record + 0x28, Width::Byte, 127)
        .unwrap();
    for count in 1..=2 {
        call(
            &mut execution,
            &mut progress,
            &mut other,
            0x8016_a278,
            [1, 2, 30, 0],
        );
        assert_eq!(progress.infinite_traversals, count);
        assert_eq!(progress.boundary.is_some(), count == 2);
        assert_eq!(execution.bus.read(record + 0x28, Width::Byte).unwrap(), 127);
        assert_eq!(
            execution.bus.read(record + 4, Width::Word).unwrap(),
            DATA + 1
        );
    }
    assert_eq!(
        progress.boundary.as_ref().unwrap().reason,
        Stop::InfiniteLoopLimit
    );
    assert_eq!(progress.boundary.as_ref().unwrap().pc, 0x8016a374);
    let mut end = Progress::new(1, 2, record, Some(2)).unwrap();
    call(
        &mut execution,
        &mut end,
        &mut other,
        0x8016_cf1c,
        [1, 2, 0, 0],
    );
    assert_eq!(end.end_markers, 1);
    assert_eq!(end.boundary.unwrap().reason, Stop::EndMarker);
    assert_eq!(execution.bus.read(record + 0x48, Width::Half).unwrap(), 1);
    assert_eq!(other.loop_starts, 0);
    assert_eq!(other.total_infinite_traversals, 0);
    assert_eq!(other.end_markers, 0);
    assert!(other.boundary.is_none());
}
