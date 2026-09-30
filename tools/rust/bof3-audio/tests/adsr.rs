use bof3_audio::machine::adsr::{Adsr, Model, Phase, Registers};

fn start(adsr1: u16, adsr2: u16, model: Model) -> Adsr {
    let mut envelope = Adsr::new(Registers { adsr1, adsr2 }, model);
    envelope.key_on();
    envelope
}

#[test]
fn literal_fast_envelope_trace_covers_all_four_phases() {
    // Register-zero attack adds 7*2048. Decay halves the positive level with
    // floor rounding until <=2048. Sustain increases until explicitly released.
    for model in [Model::Published, Model::EmulatorReference] {
        let mut a = start(0, 0, model);
        for (level, phase) in [
            (14336, Phase::Attack),
            (28672, Phase::Attack),
            (32767, Phase::Decay),
            (16383, Phase::Decay),
            (8191, Phase::Decay),
            (4095, Phase::Decay),
            (2047, Phase::Sustain),
            (16383, Phase::Sustain),
            (30719, Phase::Sustain),
            (32767, Phase::Sustain),
        ] {
            assert_eq!(a.tick(), level);
            assert_eq!(a.phase(), phase);
        }
        a.key_off();
        assert_eq!(a.phase(), Phase::Release);
        assert_eq!(a.tick(), 16383);
        assert_eq!(a.tick(), 0);
        assert_eq!(a.phase(), Phase::Off);
        assert_eq!(a.tick(), 0);
    }
}

#[test]
fn counter_intervals_and_rate_alias_use_accumulation_not_shift_only_delays() {
    for model in [Model::Published, Model::EmulatorReference] {
        let mut a = start(48 << 8, 0, model);
        assert_eq!(a.tick(), 0);
        assert_eq!(a.counter(), 0x4000);
        assert_eq!(a.tick(), 7);
        assert_eq!(a.counter(), 0);
        assert_eq!(a.tick(), 7);
        assert_eq!(a.tick(), 14);
        for rate in [0x6a, 0x76] {
            let mut a = start(rate << 8, 0, model);
            for _ in 0..32767 {
                assert_eq!(a.tick(), 0);
            }
            assert_eq!(a.counter(), 32767);
            assert_eq!(a.tick(), 5);
            assert_eq!(a.counter(), 0);
        }
    }
}

#[test]
fn exponential_growth_boundary_and_slow_rate_disagreements_are_explicit() {
    // One corpus register pair reaches the disputed boundary after eight ticks.
    let mut documented = start(0x89ba, 0x514c, Model::Published);
    let mut emulator = start(0x89ba, 0x514c, Model::EmulatorReference);
    for expected in [3072, 6144, 9216, 12288, 15360, 18432, 21504, 24576] {
        assert_eq!(documented.tick(), expected);
        assert_eq!(emulator.tick(), expected);
    }
    assert_eq!(documented.tick(), 27648);
    assert_eq!(emulator.tick(), 25344);

    let mut documented = start(0x8000, 0, Model::Published);
    let mut emulator = start(0x8000, 0, Model::EmulatorReference);
    documented.write_level(0x6000);
    emulator.write_level(0x6000);
    assert_eq!(documented.tick(), 32767);
    assert_eq!(documented.phase(), Phase::Decay);
    assert_eq!(emulator.tick(), 28160);
    assert_eq!(emulator.phase(), Phase::Attack);

    for model in [Model::Published, Model::EmulatorReference] {
        // Shift 10 divides both delta and counter increment by two above threshold.
        let mut a = start(0x8000 | (40 << 8), 0, model);
        a.write_level(0x6001);
        assert_eq!(a.tick(), 0x6001);
        assert_eq!(a.tick(), 0x6008);
        // Shift 11 divides only the increment by four.
        let mut a = start(0x8000 | (44 << 8), 0, model);
        a.write_level(0x6001);
        for _ in 0..3 {
            assert_eq!(a.tick(), 0x6001);
        }
        assert_eq!(a.tick(), 0x6008);
    }
    let mut documented = start(0x8000 | (108 << 8), 0, Model::Published);
    let mut emulator = start(0x8000 | (108 << 8), 0, Model::EmulatorReference);
    documented.write_level(0x6001);
    emulator.write_level(0x6001);
    for _ in 0..32768 {
        documented.tick();
        emulator.tick();
    }
    assert_eq!(documented.level(), 0x6008);
    assert_eq!(emulator.level(), 0x6001);
}

#[test]
fn exponential_release_rounds_negative_steps_down_and_sustain_zero_stays_active() {
    for model in [Model::Published, Model::EmulatorReference] {
        let mut a = start(0, 32 | 11, model);
        a.write_level(1);
        a.key_off();
        assert_eq!(a.tick(), 0); // (-8 * 1) >> 15 = -1, not zero.
        assert_eq!(a.phase(), Phase::Off);
        let mut a = start(15, 0x4000, model);
        for _ in 0..5 {
            a.tick();
        }
        assert_eq!(a.level(), 0);
        assert_eq!(a.phase(), Phase::Sustain);
        assert_eq!(a.tick(), 0);
        assert_eq!(a.phase(), Phase::Sustain);
        a.force_off();
        assert_eq!(a.phase(), Phase::Off);
    }
}

#[test]
fn all_ones_rates_freeze_without_turning_off_or_saturating() {
    for model in [Model::Published, Model::EmulatorReference] {
        let mut a = start(0x7f00, 31, model);
        a.write_level(1234);
        for _ in 0..40000 {
            assert_eq!(a.tick(), 1234);
        }
        assert_eq!(a.phase(), Phase::Attack);
        a.key_off();
        for _ in 0..40000 {
            assert_eq!(a.tick(), 1234);
        }
        assert_eq!(a.phase(), Phase::Release);
        // A target reached by manual ENVX writing still changes phase.
        a.write_level(0);
        a.tick();
        assert_eq!(a.phase(), Phase::Off);
        let mut a = start(15, 127 << 6, model);
        for _ in 0..4 {
            a.tick();
        }
        assert_eq!(a.phase(), Phase::Sustain);
        assert_eq!(a.level(), 16383);
        for _ in 0..40000 {
            assert_eq!(a.tick(), 16383);
        }
    }
}

#[test]
fn register_writes_retriggering_and_key_off_keep_their_distinct_state_effects() {
    let model = Model::EmulatorReference;
    let mut a = start(0, 12, model);
    a.write_level(100);
    a.key_off();
    assert_eq!(a.tick(), 100);
    assert_eq!(a.counter(), 0x4000);
    a.key_off(); // Duplicate KOFF must not postpone a slow release indefinitely.
    assert_eq!(a.counter(), 0x4000);
    assert_eq!(a.tick(), 92);
    a.tick();
    a.write_registers(Registers {
        adsr1: 0,
        adsr2: 0x2000 | 12,
    });
    assert_eq!(a.counter(), 0);
    assert_eq!(a.phase(), Phase::Release);
    assert_eq!(a.level(), 92);
    assert_eq!(a.registers().adsr2, 0x200c); // Unused register bit is retained.
    a.key_on();
    assert_eq!((a.phase(), a.level(), a.counter()), (Phase::Attack, 0, 0));
    a.write_registers(Registers {
        adsr1: 44 << 8,
        adsr2: 0,
    });
    a.write_level(0x8000);
    assert_eq!(a.tick(), -32761); // Signed ENVX writes are not clamped on input.
    a.key_off();
    assert_eq!(a.tick(), 0);
    assert_eq!(a.phase(), Phase::Off);
    a.write_level(0xffff);
    assert_eq!(a.tick(), -1); // Off voices retain manually written ENVX readback.
    a.force_off();
    assert_eq!(a.level(), 0);
    assert_eq!(a.model(), model);
}

#[test]
#[ignore = "requires BOF3_AUDIO_CORPUS original EMI inputs"]
fn corpus_envelope_probe_keeps_reference_disagreements_visible() {
    use bof3_audio::{catalog::model::AssetData, catalog::model::Catalog};
    use std::collections::BTreeMap;
    let root = std::path::PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap());
    let catalog = Catalog::read(Some(&root), &[]).unwrap();
    let mut cases = BTreeMap::<(u16, u16), usize>::new();
    let mut tones = 0;
    for asset in &catalog.assets {
        let AssetData::Bank { metadata, .. } = &asset.data else {
            continue;
        };
        for program in &metadata.programs {
            for tone in &program.tones {
                *cases.entry((tone.adsr1, tone.adsr2)).or_default() += 1;
                tones += 1;
            }
        }
    }
    assert_eq!(tones, 21112);
    let (mut differing_pairs, mut differing_tones) = (0, 0);
    let mut released = [0, 0];
    let mut first_difference = None;
    for (&(adsr1, adsr2), &weight) in &cases {
        let mut a = start(adsr1, adsr2, Model::Published);
        let mut b = start(adsr1, adsr2, Model::EmulatorReference);
        let mut differs = false;
        // Fixed 250 ms held note followed by 250 ms release. These are model
        // comparisons from VAB registers, not captured game voice trajectories.
        for frame in 0..22050 {
            if frame == 11025 {
                a.key_off();
                b.key_off();
            }
            a.tick();
            b.tick();
            for e in [&a, &b] {
                assert!(e.level() >= 0);
                assert!(e.counter() < 0x8000);
                if e.phase() == Phase::Off {
                    assert_eq!(e.level(), 0);
                }
            }
            if (a.level(), a.phase(), a.counter()) != (b.level(), b.phase(), b.counter()) {
                differs = true;
                if first_difference.is_none() {
                    first_difference = Some((adsr1, adsr2, frame, a.level(), b.level()));
                }
            }
        }
        if differs {
            differing_pairs += 1;
            differing_tones += weight;
        }
        released[0] += usize::from(a.phase() == Phase::Off) * weight;
        released[1] += usize::from(b.phase() == Phase::Off) * weight;
    }
    eprintln!("ADSR corpus: tones={tones} unique_pairs={} differing_pairs={differing_pairs} differing_tones={differing_tones} off_after_probe={released:?} first_difference={first_difference:?}", cases.len());
    assert_eq!(cases.len(), 543);
    assert_eq!((differing_pairs, differing_tones), (125, 395));
    assert_eq!(released, [19595, 19595]);
    assert_eq!(first_difference, Some((0x89ba, 0x514c, 8, 27648, 25344)));
}
