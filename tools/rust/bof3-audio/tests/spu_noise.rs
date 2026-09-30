use bof3_audio::{digest::sha256_hex, machine::spu_noise::Noise};

#[test]
fn published_noise_all_clocks_match_independent_integer_oracle() {
    // Python oracle: parity = bit_count(level & 0x9c00) % 2 ^ 1,
    // integer-division period and while-negative refill. Samples precede ticks.
    let mut bytes = Vec::with_capacity(64 * 65536 * 2);
    for clock in 0..64 {
        let mut noise = Noise::from_published_state(0, 0).unwrap();
        for _ in 0..65536 {
            bytes.extend(noise.level().to_le_bytes());
            noise.tick(clock).unwrap();
            assert!((0..=0x20000).contains(&noise.timer()));
        }
    }
    assert_eq!(
        sha256_hex(&bytes),
        "734409073f749d57a47b078e194d6f9377bd137aff6d3a635f9d3aa843f363c7"
    );
}

#[test]
fn noise_refill_changes_phase_without_extra_lfsr_ticks_or_implicit_reset() {
    let mut noise = Noise::from_published_state(0, 0).unwrap();
    for expected in [
        0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4094, 8189, 16378, 32756, -24, -47, -93,
        -185,
    ] {
        assert_eq!(noise.level(), expected);
        noise.tick(63).unwrap();
    }
    let mut noise = Noise::from_published_state(0x1234, 0x20000).unwrap();
    noise.tick(0).unwrap();
    assert_eq!((noise.level(), noise.timer()), (0x1234, 0x1fffc));
    noise.tick(63).unwrap(); // Clock changes preserve the existing countdown.
    assert_eq!((noise.level(), noise.timer()), (0x1234, 0x1fff5));
    assert!(noise.tick(64).is_err());
    assert_eq!((noise.level(), noise.timer()), (0x1234, 0x1fff5));
    for timer in [-1, 0x20001, i32::MAX] {
        assert!(Noise::from_published_state(0, timer).is_err());
    }
}
