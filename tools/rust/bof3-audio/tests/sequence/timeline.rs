use bof3_audio::{
    sequence::events::Kind,
    sequence::timeline::{self, Limits, Stop},
};

fn stream(count: u8) -> Vec<u8> {
    vec![
        0, 0xb0, 99, 20, 0, 98, count, 0, 0xc1, 8, 5, 0xb2, 99, 30, 7, 0xff, 0x2f, 0,
    ]
}

#[test]
fn finite_loops_use_post_end_delay_and_preserve_dynamic_running_channel() {
    for count in [0, 1, 2, 3, 126] {
        let trace = timeline::trace(&stream(count), &Limits::default()).unwrap();
        let traversals = u64::from(count.max(1));
        assert_eq!(trace.stop, Stop::EndMarker);
        assert_eq!(trace.final_tick, traversals * 12);
        assert_eq!(trace.steps.len(), 2 + traversals as usize * 3);
        let counts: Vec<_> = trace
            .steps
            .iter()
            .filter(|s| s.source_cursor == 5)
            .collect();
        assert_eq!(counts[0].status, 0xb0);
        assert!(counts[1..].iter().all(|s| s.status == 0xb2));
        assert!(counts.iter().all(|s| !s.explicit_status));
        let programs: Vec<_> = trace
            .steps
            .iter()
            .filter(|s| matches!(s.kind, Kind::Program { .. }))
            .collect();
        assert_eq!(
            programs.iter().map(|s| s.tick).collect::<Vec<_>>(),
            (0..traversals).map(|i| i * 12).collect::<Vec<_>>()
        );
        assert_eq!(trace.source_trailing_bytes, 1);
    }
}

#[test]
fn infinite_loops_stop_at_requested_boundary_without_inventing_eot_execution() {
    let trace = timeline::trace(&stream(127), &Limits::default()).unwrap();
    assert_eq!(
        trace.stop,
        Stop::InfiniteLoopLimit {
            source_cursor: 11,
            traversals: 2
        }
    );
    assert_eq!(trace.final_tick, 10);
    assert_eq!(trace.steps.len(), 7);
    assert!(trace.steps.iter().all(|s| s.kind != Kind::End));
    let last = trace.steps.last().unwrap();
    assert!(last.jumped);
    assert_eq!(
        (last.next_cursor, last.consumed_delta, last.next_delay),
        (5, Some(7), 0)
    );
    assert_eq!(last.loops.remaining, 127); // Rendering policy did not alter game state.
}

#[test]
fn overwritten_starts_and_unmatched_starts_do_not_create_a_loop_stack() {
    let bytes = [
        0, 0xb0, 99, 20, 0, 98, 127, 0, 99, 20, 0, 0xc0, 7, 3, 0xb0, 99, 30, 0, 0xff, 0x2f,
    ];
    let trace = timeline::trace(&bytes, &Limits::default()).unwrap();
    assert_eq!(trace.final_tick, 6);
    assert_eq!(
        trace.steps.iter().filter(|s| s.source_cursor == 8).count(),
        1
    );
    assert_eq!(
        trace.steps.iter().filter(|s| s.source_cursor == 11).count(),
        2
    );
    assert_eq!(trace.steps.last().unwrap().loops.start_cursor, 11);
    let unmatched = [2, 0xb0, 99, 20, 3, 98, 127, 4, 0xff, 0x2f];
    let trace = timeline::trace(&unmatched, &Limits::default()).unwrap();
    assert_eq!(trace.stop, Stop::EndMarker);
    assert_eq!(trace.final_tick, 9);
}

#[test]
fn zero_delay_cycles_budgets_unsupported_controllers_and_delay_overflow_fail() {
    let zero = [0, 0xb0, 99, 20, 0, 98, 127, 0, 99, 30, 0, 0xff, 0x2f];
    let limits = Limits {
        max_events: 3,
        infinite_traversals: 1000,
        ..Limits::default()
    };
    assert!(timeline::trace(&zero, &limits)
        .unwrap_err()
        .to_string()
        .contains("exceeded 3 events"));
    assert!(timeline::trace(
        &stream(2),
        &Limits {
            max_ticks: 1,
            ..Limits::default()
        }
    )
    .is_err());
    for controller in [1, 6, 64, 98] {
        assert!(
            timeline::trace(&[0, 0xb0, controller, 1, 0, 0xff, 0x2f], &Limits::default()).is_err()
        );
    }
    assert!(
        timeline::trace(&[0xff, 0xff, 0xff, 0x7f, 0xff, 0x2f], &Limits::default())
            .unwrap_err()
            .to_string()
            .contains("signed runtime delay")
    );
    assert!(timeline::trace(
        &stream(127),
        &Limits {
            infinite_traversals: 0,
            ..Limits::default()
        }
    )
    .is_err());
}
