use bof3_audio::sequence::clock::Clock;

#[test]
fn tick_batches_zero_delays_and_observes_quantum_changes() {
    let mut clock = Clock {
        delay: 0,
        quantum: 16,
        slow_counter: -1,
    };
    let mut emitted = Vec::new();
    let mut delays = [0, 10, 20].into_iter();
    assert_eq!(
        clock
            .tick(3, |clock| {
                let delay = delays.next().unwrap();
                emitted.push(delay);
                clock.delay = delay;
                if delay == 10 {
                    clock.quantum = 25;
                }
                Ok(())
            })
            .unwrap(),
        3
    );
    assert_eq!(emitted, [0, 10, 20]);
    assert_eq!(clock.delay, 5);
    assert_eq!(clock.quantum, 25);
}

#[test]
fn zero_delay_cycles_and_callback_failures_propagate_without_success() {
    let mut clock = Clock {
        delay: 0,
        quantum: 16,
        slow_counter: -1,
    };
    let mut calls = 0;
    let error = clock
        .tick(8, |_| {
            calls += 1;
            Ok(())
        })
        .unwrap_err();
    assert_eq!(calls, 8);
    assert!(error.to_string().contains("exceeded 8 events"));
    assert!(clock
        .tick(8, |_| Err("unsupported sequence event".into()))
        .unwrap_err()
        .to_string()
        .contains("unsupported sequence event"));
}
