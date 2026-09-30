use bof3_audio::sequence::loops::LoopState;

#[test]
fn finite_counts_infinite_jump_and_single_saved_cursor_are_explicit() {
    let mut state = LoopState::new(1);
    state.apply(99, 20, 100, 3).unwrap();
    state.apply(98, 2, 200, 4).unwrap();
    let jump = state.apply(99, 30, 300, 5).unwrap();
    assert_eq!((jump.cursor, jump.delay, jump.jumped), (100, 5, true));
    // Encountering the count again after a jump must not reload it.
    state.apply(98, 2, 200, 4).unwrap();
    let end = state.apply(99, 30, 300, 5).unwrap();
    assert_eq!((end.cursor, end.delay, end.jumped), (300, 5, false));
    assert!(!state.count_active);
    state.apply(99, 20, 400, 8).unwrap();
    state.apply(6, 127, 500, 9).unwrap();
    state.apply(99, 20, 600, 2).unwrap();
    state.apply(98, 1, 700, 3).unwrap();
    let jump = state.apply(99, 30, 800, 12).unwrap();
    assert_eq!((jump.cursor, jump.delay, jump.jumped), (600, 0, true));
    assert_eq!(state.remaining, 127);
    assert!(state.count_pending);
}

#[test]
fn unsupported_nrpn_contexts_fail_without_state_changes() {
    let mut state = LoopState::new(5);
    for (controller, value) in [(6, 127), (98, 127), (99, 40), (7, 100), (99, 128)] {
        let before = state.clone();
        assert!(state.apply(controller, value, 9, 0).is_err());
        assert_eq!(state, before);
    }
    // A zero-count end does not jump or invent a start marker.
    let end = state.apply(99, 30, 15, 2).unwrap();
    assert_eq!((end.cursor, end.delay, end.jumped), (15, 2, false));
}
