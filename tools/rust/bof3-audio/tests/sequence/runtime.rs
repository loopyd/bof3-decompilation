//! Bounded original-US execution. Synthetic RAM contexts are not playback traces.
use bof3_audio::machine::{
    bus::{Bus, Ram, Width},
    cpu::Cpu,
    executable::Executable,
    profile::Profile,
};

const BASE: u32 = 0x8002_0000;
const DATA: u32 = 0x8001_0000;
const RETURN: u32 = 0x8000_1000;
#[path = "../common/timing_runtime.rs"]
mod timing_runtime;

fn call(ram: &mut Ram, entry: u32, args: &[u32], stop: u32) -> Cpu {
    let mut cpu = Cpu::new(entry);
    for (index, value) in args.iter().enumerate() {
        cpu.set_register(4 + index, *value);
    }
    cpu.set_register(29, 0x801f_f000);
    cpu.set_register(31, RETURN);
    cpu.run_until(ram, stop, 2000).unwrap();
    cpu
}

fn fixture() -> Ram {
    let exe =
        Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    let profile = Profile::identify(&exe).unwrap();
    let mut ram = Ram::from_executable(&exe);
    call(
        &mut ram,
        profile.sequence_table_setup,
        &[BASE, 2, 4],
        RETURN,
    );
    // Real game callback registration, reached by game initialization at 0x8015CDD8.
    call(&mut ram, 0x8016_1dc8, &[], RETURN);
    for (slot, entry) in [
        (0x8018_db58, 0x8016_a974),
        (0x8018_db5c, 0x8016_aa5c),
        (0x8018_db60, 0x8016_a4a4),
        (0x8018_db64, 0x8016_a79c),
        (0x8018_db68, 0x8016_a55c),
    ] {
        assert_eq!(ram.read(slot, Width::Word).unwrap(), entry);
    }
    ram
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_sep_initializer_assigns_programs_by_channel() {
    let mut ram = fixture();
    for (handle, sequence) in [(0, 0), (1, 3)] {
        let state = prepare(&mut ram, handle, sequence, &[0, 0xff, 0x2f]);
        call(
            &mut ram,
            0x8016b4b8,
            &[handle, sequence, 0, DATA],
            0x8016b5a0,
        );
        for channel in 0..16 {
            assert_eq!(
                ram.read(state + 0x2c + channel, Width::Byte).unwrap(),
                channel
            );
            assert_eq!(ram.read(state + 0x17 + channel, Width::Byte).unwrap(), 64);
            assert_eq!(
                ram.read(state + 0x4e + channel * 2, Width::Half).unwrap(),
                127
            );
        }
    }
}

fn prepare(ram: &mut Ram, handle: u32, sequence: u32, data: &[u8]) -> u32 {
    let state = BASE + (handle * 4 + sequence) * 0xac;
    for (i, b) in data.iter().enumerate() {
        ram.write(DATA + i as u32, Width::Byte, u32::from(*b))
            .unwrap();
    }
    ram.write(state + 4, Width::Word, DATA).unwrap();
    ram.write(state + 0x80, Width::Word, 0).unwrap();
    state
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_delta_reader_consumes_vlq_and_accumulates_tenfold_units() {
    let mut ram = fixture();
    for handle in 0..2 {
        for sequence in 0..4 {
            for (bytes, value) in [
                (vec![0], 0),
                (vec![127], 127),
                (vec![0x81, 0], 128),
                (vec![0x80, 0], 0),
                (vec![0xff, 0xff, 0x7f], 0x1f_ffff),
                (vec![0xff, 0xff, 0xff, 0x7f], 0x0fff_ffff),
            ] {
                let state = prepare(&mut ram, handle, sequence, &bytes);
                ram.write(state + 0x80, Width::Word, 123).unwrap();
                let cpu = call(&mut ram, 0x8016_aad4, &[handle, sequence], RETURN);
                assert_eq!(cpu.register(2), value * 10);
                assert_eq!(
                    ram.read(state + 4, Width::Word).unwrap(),
                    DATA + bytes.len() as u32
                );
                assert_eq!(
                    ram.read(state + 0x80, Width::Word).unwrap(),
                    123 + value * 10
                );
            }
        }
    }
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_dispatcher_preserves_running_channel_program_and_note_arguments() {
    let mut ram = fixture();
    for channel in 0..16u8 {
        let state = prepare(&mut ram, 1, 2, &[0xc0 | channel, 7, 0x81, 1, 9, 0]);
        call(&mut ram, 0x8016_d0e0, &[1, 2], RETURN);
        assert_eq!(
            ram.read(state + 0x2c + u32::from(channel), Width::Byte)
                .unwrap(),
            7
        );
        assert_eq!(ram.read(state + 0x88, Width::Word).unwrap(), 1290);
        assert_eq!(ram.read(state + 4, Width::Word).unwrap(), DATA + 4);
        call(&mut ram, 0x8016_d0e0, &[1, 2], RETURN);
        assert_eq!(
            ram.read(state + 0x2c + u32::from(channel), Width::Byte)
                .unwrap(),
            9
        );
        assert_eq!(ram.read(state + 4, Width::Word).unwrap(), DATA + 6);
        for velocity in [0, 100] {
            prepare(&mut ram, 1, 2, &[0x90 | channel, 60, velocity, 3]);
            // Stop at the actual note callback. Its voice allocation is outside this check.
            let cpu = call(&mut ram, 0x8016_d0e0, &[1, 2], 0x8016_a974);
            assert_eq!(
                (
                    cpu.register(4),
                    cpu.register(5),
                    cpu.register(6),
                    cpu.register(7)
                ),
                (1, 2, 60, u32::from(velocity))
            );
            assert_eq!(
                ram.read(state + 0x12, Width::Byte).unwrap(),
                u32::from(channel)
            );
            assert_eq!(ram.read(state + 4, Width::Word).unwrap(), DATA + 4);
            assert_eq!(ram.read(state + 0x88, Width::Word).unwrap(), 30);
        }
    }
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_pitch_callback_uses_only_high_byte_and_tempo_truncates_to_integer_bpm() {
    let mut ram = fixture();
    for low in 0..128u8 {
        let state = prepare(&mut ram, 0, 0, &[0xe3, low, 65, 0]);
        ram.write(state + 0x2f, Width::Byte, 12).unwrap();
        ram.write(state + 0x4c, Width::Half, 4).unwrap();
        let cpu = call(&mut ram, 0x8016_d0e0, &[0, 0], 0x8017_28e0);
        assert_eq!(
            (
                cpu.register(4),
                cpu.register(5),
                cpu.register(6),
                cpu.register(7)
            ),
            (0, 4, 12, 65)
        );
        assert_eq!(ram.read(state + 4, Width::Word).unwrap(), DATA + 3);
    }
    for tempo in [400_000u32, 500_000, 500_001, 666_667] {
        let b = tempo.to_be_bytes();
        let state = prepare(&mut ram, 0, 0, &[0xff, 0x51, b[1], b[2], b[3], 2]);
        ram.write(state + 0x4a, Width::Half, 48).unwrap();
        ram.write(0x8019_0300, Width::Word, 60).unwrap();
        call(&mut ram, 0x8016_d0e0, &[0, 0], RETURN);
        assert_eq!(
            ram.read(state + 0x8c, Width::Word).unwrap(),
            60_000_000 / tempo
        );
        assert_eq!(ram.read(state + 0x88, Width::Word).unwrap(), 20);
        assert_eq!(ram.read(state + 4, Width::Word).unwrap(), DATA + 6);
    }
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_loop_controllers_match_sequence_wide_state_and_jump_delays() {
    use bof3_audio::sequence::loops::LoopState;
    let mut ram = fixture();
    for count_controller in [6, 98] {
        for count in [0, 1, 2, 3, 126, 127] {
            // Each case starts with a fresh sequence context. Controller 6 calls
            // the original program-info getter; an unopened bank returns early.
            let state = BASE;
            for offset in 0..0xac {
                ram.write(state + offset, Width::Byte, 0).unwrap();
            }
            let mut model = LoopState::new(DATA as usize);
            let mut cursor = DATA;
            let mut events = vec![
                (99, 20, 3),
                (count_controller, count, 5),
                // A second start overwrites the one saved cursor, without a stack
                // or resetting the active count. Its count event cannot reload it.
                (99, 20, 4),
                (count_controller, 17, 6),
            ];
            for _ in 0..130 {
                events.push((99, 30, 7));
                events.push((count_controller, count, 2));
            }
            for (controller, value, delta) in events {
                // Alternate channels: loop bookkeeping belongs to the sequence.
                let channel = ((cursor / 4) & 15) as u8;
                for (i, value) in [0xb0 | channel, controller, value, delta]
                    .iter()
                    .enumerate()
                {
                    ram.write(cursor + i as u32, Width::Byte, u32::from(*value))
                        .unwrap();
                }
                ram.write(state + 4, Width::Word, cursor).unwrap();
                let expected = model
                    .apply(controller, value, cursor as usize + 4, u32::from(delta))
                    .unwrap();
                call(&mut ram, 0x8016_d0e0, &[0, 0], RETURN);
                assert_eq!(
                    ram.read(state + 4, Width::Word).unwrap() as usize,
                    expected.cursor,
                    "controller {controller}, value {value}, model {model:?}"
                );
                assert_eq!(
                    ram.read(state + 0x88, Width::Word).unwrap(),
                    expected.delay * 10
                );
                assert_eq!(
                    ram.read(state + 0xc, Width::Word).unwrap() as usize,
                    model.start_cursor
                );
                assert_eq!(
                    ram.read(state + 0x16, Width::Byte).unwrap(),
                    u32::from(model.selector.unwrap())
                );
                assert_eq!(
                    ram.read(state + 0x27, Width::Byte).unwrap(),
                    u32::from(model.count_pending)
                );
                assert_eq!(
                    ram.read(state + 0x10, Width::Byte).unwrap(),
                    u32::from(model.count_active)
                );
                assert_eq!(
                    ram.read(state + 0x28, Width::Byte).unwrap(),
                    u32::from(model.remaining)
                );
                cursor += 4;
            }
        }
    }
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_event_cursor_replays_count_and_program_without_reloading_loop_count() {
    use bof3_audio::{
        sequence::events::Kind,
        sequence::timeline::{self, Limits},
    };
    let mut ram = fixture();
    for count in [0, 1, 2, 3, 127] {
        for offset in 0..0xac {
            ram.write(BASE + offset, Width::Byte, 0).unwrap();
        }
        let stream = [
            0xb0, 99, 20, 0, 98, count, 0, 0xc1, 8, 5, 0xb2, 99, 30, 7, 0xff, 0x2f, 0,
        ];
        let mut encoded = vec![0];
        encoded.extend(stream);
        let timeline = timeline::trace(
            &encoded,
            &Limits {
                infinite_traversals: 3,
                ..Limits::default()
            },
        )
        .unwrap();
        let state = prepare(&mut ram, 0, 0, &stream);
        // Compare every pre-EOT timeline step against unmodified original
        // dispatch, including channel changes on implicit-status loop targets.
        for step in timeline.steps.iter().filter(|s| s.kind != Kind::End) {
            assert_eq!(
                ram.read(state + 4, Width::Word).unwrap(),
                DATA + step.source_cursor as u32 - 1
            );
            call(&mut ram, 0x8016_d0e0, &[0, 0], RETURN);
            assert_eq!(
                ram.read(state + 4, Width::Word).unwrap(),
                DATA + step.next_cursor as u32 - 1
            );
            assert_eq!(
                ram.read(state + 0x88, Width::Word).unwrap(),
                step.next_delay * 10
            );
            assert_eq!(
                ram.read(state + 0x12, Width::Byte).unwrap(),
                u32::from(step.status & 15)
            );
            assert_eq!(
                ram.read(state + 0x28, Width::Byte).unwrap(),
                u32::from(step.loops.remaining)
            );
        }
        // Keep the earlier literal cursor/count assertions as an independent
        // expected-value check rather than replacing them with model equality.
        for offset in 0..0xac {
            ram.write(BASE + offset, Width::Byte, 0).unwrap();
        }
        let state = prepare(&mut ram, 0, 0, &stream);
        call(&mut ram, 0x8016_d0e0, &[0, 0], RETURN);
        assert_eq!(ram.read(state + 4, Width::Word).unwrap(), DATA + 4);
        let traversals = if count == 127 {
            3
        } else {
            u32::from(count.max(1))
        };
        for traversal in 0..traversals {
            // Do not rewrite the cursor between events. Original handlers alone
            // advance it and jump back to the implicit-status count event.
            for (expected_cursor, expected_delay) in [(DATA + 7, 0), (DATA + 10, 50)] {
                call(&mut ram, 0x8016_d0e0, &[0, 0], RETURN);
                assert_eq!(ram.read(state + 4, Width::Word).unwrap(), expected_cursor);
                assert_eq!(ram.read(state + 0x88, Width::Word).unwrap(), expected_delay);
            }
            assert_eq!(ram.read(state + 0x2d, Width::Byte).unwrap(), 8);
            call(&mut ram, 0x8016_d0e0, &[0, 0], RETURN);
            let jumping = count == 127 || traversal + 1 < traversals;
            assert_eq!(
                ram.read(state + 4, Width::Word).unwrap(),
                DATA + if jumping { 4 } else { 14 }
            );
            assert_eq!(
                ram.read(state + 0x88, Width::Word).unwrap(),
                if count == 127 { 0 } else { 70 }
            );
        }
        // End-of-sequence behavior and actual scheduler cadence are not executed.
    }
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_end_marker_matches_restart_stop_and_signed_counter_boundaries() {
    use bof3_audio::{
        sequence::loops::LoopState,
        sequence::termination::{Action, State},
    };
    let mut ram = fixture();
    // No active voices in this synthetic context. The actual release routine
    // executes and returns; no hardware/audio fidelity follows from this check.
    ram.write(0x8018_e264, Width::Byte, 0).unwrap();
    for handle in 0..2 {
        for sequence in 0..4 {
            for play_count in [0i16, 1, 2, 32767, -1] {
                for completed in [0u16, 1, 32767, 65535] {
                    for quantum in [-1i16, 16] {
                        let address = prepare(&mut ram, handle, sequence, &[0xff, 0x2f, 0x77]);
                        let mut state = State {
                            cursor: DATA as usize + 2,
                            restart_cursor: 0x8001_1000,
                            play_count,
                            completed,
                            elapsed_units: 123,
                            delay_units: 333,
                            tick_quantum: quantum,
                            flags: 0xa5ff,
                            active: true,
                        };
                        let mut loops = LoopState {
                            start_cursor: 0x8001_2000,
                            selector: Some(20),
                            count_pending: true,
                            count_active: true,
                            remaining: 127,
                        };
                        for (offset, width, value) in [
                            (8, Width::Word, state.restart_cursor as u32),
                            (0xc, Width::Word, loops.start_cursor as u32),
                            (0x46, Width::Half, play_count as u16 as u32),
                            (0x48, Width::Half, u32::from(completed)),
                            (0x70, Width::Half, quantum as u16 as u32),
                            (0x80, Width::Word, state.elapsed_units),
                            (0x88, Width::Word, 333),
                            (0x90, Width::Word, state.flags),
                            (0x2b, Width::Byte, 1),
                            (0x27, Width::Byte, 1),
                            (0x10, Width::Byte, 1),
                            (0x28, Width::Byte, 127),
                            (0x3c, Width::Byte, 255),
                        ] {
                            ram.write(address + offset, width, value).unwrap();
                        }
                        let action = state.end(&mut loops);
                        call(&mut ram, 0x8016_d0e0, &[handle, sequence], RETURN);
                        for (offset, width, value) in [
                            (4, Width::Word, state.cursor as u32),
                            (0xc, Width::Word, loops.start_cursor as u32),
                            (0x48, Width::Half, u32::from(state.completed)),
                            (0x80, Width::Word, state.elapsed_units),
                            (0x88, Width::Word, state.delay_units as u32),
                            (0x90, Width::Word, state.flags),
                            (0x2b, Width::Byte, u32::from(state.active)),
                            (0x27, Width::Byte, u32::from(loops.count_pending)),
                            (0x10, Width::Byte, 1),
                            (0x28, Width::Byte, 127),
                        ] {
                            assert_eq!(ram.read(address + offset, width).unwrap(), value,
                                "play={play_count}, completed={completed}, action={action:?}, offset={offset:x}");
                        }
                        if action == Action::Stop {
                            assert_eq!(state.cursor, DATA as usize + 2);
                        }
                    }
                }
            }
        }
    }
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_end_marker_activates_a_distinct_linked_sequence() {
    let mut ram = fixture();
    ram.write(0x8018_e264, Width::Byte, 0).unwrap();
    let current = prepare(&mut ram, 0, 1, &[0xff, 0x2f, 0]);
    let next = BASE + 6 * 0xac; // handle 1, sequence 2
    for (offset, width, value) in [
        (0x46, Width::Half, 1),
        (0x48, Width::Half, 0),
        (0x3c, Width::Byte, 1),
        (0, Width::Byte, 2),
        (0x70, Width::Half, 17),
    ] {
        ram.write(current + offset, width, value).unwrap();
    }
    for (offset, width, value) in [
        (8, Width::Word, 0x8001_3000),
        (0x46, Width::Half, 8),
        (0x48, Width::Half, 9),
        (0x90, Width::Word, 0xa5ff),
        (0x2b, Width::Byte, 0),
        (0x88, Width::Word, 99),
    ] {
        ram.write(next + offset, width, value).unwrap();
    }
    call(&mut ram, 0x8016_d0e0, &[0, 1], RETURN);
    assert_eq!(ram.read(next + 4, Width::Word).unwrap(), 0x8001_3000);
    assert_eq!(ram.read(next + 0x46, Width::Half).unwrap(), 1);
    assert_eq!(ram.read(next + 0x48, Width::Half).unwrap(), 0);
    assert_eq!(
        ram.read(next + 0x90, Width::Word).unwrap(),
        (0xa5ff & !0x30e) | 1
    );
    assert_eq!(ram.read(next + 0x2b, Width::Byte).unwrap(), 1);
    assert_eq!(ram.read(next + 0x88, Width::Word).unwrap(), 99);
    assert_eq!(ram.read(current + 0x2b, Width::Byte).unwrap(), 0);
    assert_eq!(ram.read(current + 0x88, Width::Word).unwrap(), 17);
}

#[test]
#[ignore = "requires original US executable in BOF3_AUDIO_EXE"]
fn original_scheduler_matches_delay_batching_and_slow_counter_updates() {
    use bof3_audio::sequence::clock::Clock;
    let mut ram = fixture();
    let delays = [0u8, 0, 1, 3, 8, 15, 32];
    let stream: Vec<u8> = (0..126)
        .flat_map(|n| [0xc1, n as u8, delays[n % delays.len()]])
        .collect();
    for quantum in [1i16, 16, 100] {
        for initial in [
            0,
            1,
            i32::from(quantum) - 1,
            i32::from(quantum),
            i32::from(quantum) + 1,
            1000,
        ] {
            for slow_counter in [-1i16, 0, 1, 2] {
                let state = prepare(&mut ram, 0, 0, &stream);
                ram.write(state + 0x70, Width::Half, quantum as u16 as u32)
                    .unwrap();
                ram.write(state + 0x6e, Width::Half, slow_counter as u16 as u32)
                    .unwrap();
                ram.write(state + 0x88, Width::Word, initial as u32)
                    .unwrap();
                let mut clock = Clock {
                    delay: initial,
                    quantum,
                    slow_counter,
                };
                let mut emitted = 0;
                for _ in 0..12 {
                    clock
                        .tick(100, |clock| {
                            clock.delay = i32::from(delays[emitted % delays.len()]) * 10;
                            emitted += 1;
                            Ok(())
                        })
                        .unwrap();
                    call(&mut ram, 0x8016_ce0c, &[0, 0], RETURN);
                    assert_eq!(
                        ram.read(state + 4, Width::Word).unwrap(),
                        DATA + emitted as u32 * 3
                    );
                    assert_eq!(
                        ram.read(state + 0x88, Width::Word).unwrap() as i32,
                        clock.delay
                    );
                    assert_eq!(
                        ram.read(state + 0x6e, Width::Half).unwrap() as i16,
                        clock.slow_counter
                    );
                    if emitted > 0 {
                        assert_eq!(
                            ram.read(state + 0x2d, Width::Byte).unwrap(),
                            (emitted - 1) as u32
                        );
                    }
                }
            }
        }
    }
    // A tempo callback changes the quantum while the scheduler is dispatching
    // this same tick. The subsequent comparison must reread the changed value.
    let state = prepare(&mut ram, 0, 0, &[0xff, 0x51, 6, 0x1a, 0x80, 0, 0xc1, 9, 3]);
    ram.write(state + 0x70, Width::Half, 16).unwrap();
    ram.write(state + 0x88, Width::Word, 0).unwrap();
    ram.write(state + 0x4a, Width::Half, 48).unwrap();
    ram.write(0x8019_0300, Width::Word, 60).unwrap();
    call(&mut ram, 0x8016_ce0c, &[0, 0], RETURN);
    assert_eq!(ram.read(state + 0x70, Width::Half).unwrap(), 20);
    assert_eq!(ram.read(state + 0x88, Width::Word).unwrap(), 10);
    assert_eq!(ram.read(state + 4, Width::Word).unwrap(), DATA + 9);
    assert_eq!(ram.read(state + 0x2d, Width::Byte).unwrap(), 9);
}

#[path = "../common/music_progress_runtime.rs"]
mod music_progress_runtime;
