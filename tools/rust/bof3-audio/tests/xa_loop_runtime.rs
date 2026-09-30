//! Original XA scheduler under direct, supplied-kernel, and booted-ROM dispatch.
//! Drive responses, GPU status, sector phase, and SPU inputs remain explicit.
#[path = "support/transport.rs"]
mod transport;
use bof3_audio::{
    machine::{
        bus::{Bus, Width},
        cd_drive::{Activity, Delivery},
    },
    xa::cue,
};
use transport::machine::{reading_fixture, reading_fixture_with_irq, Dispatch, Proof, Vsync};

#[test]
#[ignore = "requires BOF3_AUDIO_EXE; original XA initializer/scheduler/SDK with functional drive"]
fn original_xa_scheduler_reaches_reading_through_sdk_callbacks() {
    let (fixture, cue) = reading_fixture(0x2000);
    assert_eq!(
        fixture.commands,
        vec![
            (14, vec![0xc8]),
            (2, cue.runtime_start_bcd.to_vec()),
            (22, vec![]),
            (13, vec![cue.filter_file, cue.channel]),
            (2, cue.runtime_start_bcd.to_vec()),
            (27, vec![]),
        ]
    );
    eprintln!(
        "XA scheduler reaches reading in {} explicit fixture transitions",
        fixture.transitions
    );
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE and BOF3_AUDIO_TRACK; original cue restart with pending audio/data and changed XA channel"]
fn original_next_cue_discards_old_buffers_and_rebinds_selected_stream() {
    use bof3_audio::{
        archive::disc::DiscImage,
        machine::{
            cd_audio::{Model, XaAudio},
            cd_queue::Queue,
        },
        xa::{Arithmetic, Histories, Stream},
    };
    let (mut fixture, cue) = reading_fixture(0x2000);
    let track = std::path::PathBuf::from(std::env::var_os("BOF3_AUDIO_TRACK").unwrap());
    let mut disc = DiscImage::open(&track).unwrap();
    let first = disc.read_sector(cue.runtime_start_lba).unwrap();
    fixture
        .bus
        .inner
        .configure_cd_audio(
            Queue::new(
                Stream {
                    file: first[16],
                    channel: first[17],
                    coding: first[19],
                },
                Arithmetic::SplitFloor,
                Histories::default(),
                Model::EmulatorReference,
                bof3_audio::machine::cd_queue::Model::EmulatorReference,
            )
            .unwrap(),
        )
        .unwrap();
    // Retain one selected audio block and one acknowledged, unrequested data
    // block from the original disc before requesting a new cue.
    for lba in cue.runtime_start_lba..cue.runtime_start_lba + 6 {
        let raw = disc.read_sector(lba).unwrap();
        match fixture.drive.sector(&raw).unwrap() {
            Delivery::Xa(bytes) => {
                fixture.bus.inner.enqueue_cd_audio(&bytes).unwrap();
            }
            Delivery::Data { bytes, response } => {
                fixture
                    .bus
                    .inner
                    .deliver_cd_data(response.bytes[0], &bytes)
                    .unwrap();
                fixture.call(0x80177264, &[], true);
            }
            Delivery::Filtered => {}
        }
    }
    let old = fixture
        .bus
        .inner
        .cd_audio()
        .unwrap()
        .statistics()
        .queued_frames;
    assert_eq!(old, 4704);
    fixture.call(cue::START, &[0x2001], false);
    for _ in 0..20 {
        fixture.call(cue::TICK, &[], false);
        if fixture.state() == 5 {
            break;
        }
    }
    assert_eq!(fixture.state(), 5);
    assert_eq!(fixture.drive.activity(), Activity::Reading);
    let stats = fixture.bus.inner.cd_audio().unwrap().statistics();
    assert_eq!(stats.queued_frames, 0);
    assert_eq!(stats.reset_discarded_frames, old as u64);
    assert_eq!(stats.resets, 2); // SeekP and the subsequent located ReadS.
    fixture.bus.write(0x1f801800, Width::Byte, 0).unwrap();
    assert!(fixture.bus.write(0x1f801803, Width::Byte, 0x80).is_err());
    assert!(fixture.commands.iter().any(|c| *c == (13, vec![1, 1])));
    let mut checked = false;
    for _ in 0..16 {
        let raw = disc.read_sector(fixture.drive.next_lba() as u32).unwrap();
        if let Delivery::Xa(bytes) = fixture.drive.sector(&raw).unwrap() {
            assert_eq!([bytes[0], bytes[1]], [1, 1]);
            let mut fresh = XaAudio::new(
                Stream {
                    file: bytes[0],
                    channel: bytes[1],
                    coding: bytes[3],
                },
                Arithmetic::SplitFloor,
                Histories::default(),
                Model::EmulatorReference,
            )
            .unwrap();
            let expected = fresh.sector(&bytes).unwrap();
            fixture.bus.inner.enqueue_cd_audio(&bytes).unwrap();
            assert_eq!(
                fixture
                    .bus
                    .inner
                    .cd_audio()
                    .unwrap()
                    .statistics()
                    .queued_frames,
                expected.len()
            );
            transport::configure_spu(&mut fixture.bus.inner);
            assert_eq!(
                fixture.bus.inner.cd_host().unwrap().volume().active(),
                [128, 0, 128, 0]
            );
            for frame in expected {
                assert_eq!(
                    fixture.bus.inner.step_spu_cd_output([0; 2]).unwrap(),
                    frame.map(|v| ((i32::from(v) >> 1) >> 1) as i16)
                );
            }
            assert_eq!(
                fixture
                    .bus
                    .inner
                    .cd_audio()
                    .unwrap()
                    .statistics()
                    .queued_frames,
                0
            );
            checked = true;
            break;
        }
    }
    assert!(checked);
    eprintln!("Original cue restart 0x2000 -> 0x2001: 4,704 queued frames and pending data cleared, two command-coupled resets, channel 1 rebound without patching game state.");
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE and BOF3_AUDIO_TRACK; original XA scheduler through raw VOICE delivery and pause"]
fn original_xa_scheduler_polls_raw_media_and_completes_pause() {
    run_cue_schedules(Dispatch::Direct, &[0x2000], false, 0);
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE and BOF3_AUDIO_TRACK; original SDK exception hook with explicit kernel RAM"]
fn original_xa_scheduler_uses_cop0_and_installed_sdk_exception_hook() {
    run_cue_schedules(Dispatch::Kernel, &[0x2000], false, 0);
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE and BOF3_AUDIO_TRACK; original S_XA/MAGIC/VOICE instruction closure with explicit kernel inputs"]
fn original_xa_stream_families_have_bounded_instruction_closures() {
    run_cue_schedules(Dispatch::Kernel, &[0, 0x1000, 0x2000], true, 0);
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE, BOF3_AUDIO_BIOS and BOF3_AUDIO_TRACK; original US ROM initialization and exception dispatch"]
fn original_xa_scheduler_uses_booted_rom_kernel_state() {
    let expected = run_cue_schedules(Dispatch::Kernel, &[0, 0x1000, 0x2000], false, 0);
    let observed = run_cue_schedules(Dispatch::Firmware, &[0, 0x1000, 0x2000], true, 0);
    assert_eq!(
        observed, expected,
        "ROM and supplied kernel observable XA outcomes"
    );
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE, BOF3_AUDIO_BIOS and BOF3_AUDIO_TRACK; full original CD reset and raw XA playback"]
fn original_cd_reset_establishes_readiness_before_xa_playback() {
    let expected = run_cue_schedules(Dispatch::Firmware, &[0, 0x1000, 0x2000], false, 0);
    let observed = run_cue_schedules(Dispatch::Initialized, &[0, 0x1000, 0x2000], true, 0);
    assert_eq!(observed.len(), expected.len());
    for (mut actual, expected) in observed.into_iter().zip(expected) {
        assert_eq!(
            actual.commands[..3],
            [(1, vec![]), (0x0a, vec![]), (0x0c, vec![])]
        );
        actual.commands.drain(..3);
        assert_eq!(
            actual, expected,
            "full reset and callback-only XA outcomes after explicit initialization commands"
        );
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE, BOF3_AUDIO_BIOS and BOF3_AUDIO_TRACK; original XA GPUSTAT dependency comparison"]
fn original_audio_vsync_calls_do_not_depend_on_gpu_status_bits() {
    let expected = run_cue_schedules(Dispatch::Initialized, &[0, 0x1000, 0x2000], false, 0);
    let observed = run_cue_schedules(Dispatch::Initialized, &[0, 0x1000, 0x2000], true, u32::MAX);
    assert_eq!(
        observed, expected,
        "GPU input must not change the observed audio outcomes"
    );
}

fn run_cue_schedules(
    dispatch: Dispatch,
    packed_ids: &[u16],
    observed: bool,
    gpu: u32,
) -> Vec<transport::Outcome> {
    run_cue_comparison(
        dispatch,
        packed_ids,
        observed,
        Vsync::Original(gpu),
        &mut Vec::new(),
    )
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE, BOF3_AUDIO_BIOS and BOF3_AUDIO_TRACK; fixture-only VSync substitution through original ROM interrupts"]
fn negative_vsync_candidate_preserves_full_reset_and_interrupt_execution() {
    let mut original_proof = Vec::new();
    let original = run_cue_comparison(
        Dispatch::Initialized,
        &[0, 0x1000, 0x2000],
        true,
        Vsync::Original(0x14802000),
        &mut original_proof,
    );
    let mut candidate_proof = Vec::new();
    let candidate = run_cue_comparison(
        Dispatch::Initialized,
        &[0, 0x1000, 0x2000],
        false,
        Vsync::Candidate,
        &mut candidate_proof,
    );
    assert_eq!(
        candidate, original,
        "commands, scheduler and PCM must agree"
    );
    assert_eq!(
        candidate_proof, original_proof,
        "PC sequence, call-return registers, RAM and IRQ boundaries must agree"
    );
    println!(
        "{}",
        serde_json::json!({
        "schema": "bof3.audio.vsync-candidate/v1",
        "profile": "exe/slus_004_22",
        "exe_sha256": bof3_audio::digest::sha256_hex(&std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap()),
        "bios_sha256": bof3_audio::digest::sha256_hex(&std::fs::read(std::env::var_os("BOF3_AUDIO_BIOS").unwrap()).unwrap()),
        "fixture_substitutions": [
            {"address":0x80174720u32,"file_offset":0xde720,"original":0x8c500000u32,"candidate":0},
            {"address":0x80174724u32,"file_offset":0xde724,"original":0x8c620000u32,"candidate":0},
            ],
            "outcomes": candidate, "execution": candidate_proof,
            "gpu_and_timer_counter_available": false,
            "production_pruning_authorized": false,
            "limitations": "Explicit command responses, sector schedules and dry SPU; CPU timing and broader caller domains remain unproven",
        })
    );
}

fn run_cue_comparison(
    dispatch: Dispatch,
    packed_ids: &[u16],
    observed: bool,
    vsync: Vsync,
    proofs: &mut Vec<Proof>,
) -> Vec<transport::Outcome> {
    let mut outcomes = Vec::new();
    let exception_irqs = dispatch != Dispatch::Direct;
    use bof3_audio::{
        archive::disc::DiscImage,
        machine::{
            cd_audio::Model,
            cd_queue::{Admission, Queue},
        },
        xa::{Arithmetic, Histories, Stream},
    };
    let track = std::path::PathBuf::from(std::env::var_os("BOF3_AUDIO_TRACK").unwrap());
    let mut disc = DiscImage::open(&track).unwrap();
    for &packed_id in packed_ids {
        for sectors_per_tick in [1, 4] {
            let (mut fixture, cue) = reading_fixture_with_irq(packed_id, dispatch, observed, vsync);
            assert_eq!(
                disc.files()[cue.disc_path].lba + cue.sector_start as u32,
                cue.runtime_start_lba
            );
            assert_eq!(
                fixture.bus.inner.cd_host().unwrap().volume().active(),
                [128, 0, 128, 0]
            );
            transport::configure_spu(&mut fixture.bus.inner);
            let mut configured_audio = false;
            let mut signal = false;
            let mut pcm = Vec::new();
            let (mut selected, mut data, mut displaced, mut frames, mut sectors) = (0, 0, 0, 0, 0);
            let mut stop_lba = None;
            let mut states = vec![5];
            for _ in 0..2000 {
                for _ in 0..sectors_per_tick {
                    if fixture.drive.activity() == Activity::Reading {
                        let lba = fixture.drive.next_lba() as u32;
                        let raw = disc.read_sector(lba).unwrap();
                        if let Some(evidence) = &mut fixture.evidence {
                            evidence.observe_sector(lba, &raw);
                        }
                        match fixture.drive.sector(&raw).unwrap() {
                            Delivery::Xa(bytes) => {
                                assert_eq!([bytes[0], bytes[1]], [cue.filter_file, cue.channel]);
                                if !configured_audio {
                                    fixture
                                    .bus
                                    .inner
                                    .configure_cd_audio(
                                        Queue::new(
                                            Stream {
                                                file: bytes[0],
                                                channel: bytes[1],
                                                coding: bytes[3],
                                            },
                                            Arithmetic::SplitFloor,
                                            Histories::default(),
                                            Model::EmulatorReference,
                                            bof3_audio::machine::cd_queue::Model::EmulatorReference,
                                        )
                                        .unwrap(),
                                    )
                                    .unwrap();
                                    configured_audio = true;
                                }
                                let Admission::Queued { frames: added } =
                                    fixture.bus.inner.enqueue_cd_audio(&bytes).unwrap()
                                else {
                                    panic!("unexpected XA queue drop or mute")
                                };
                                frames += added;
                                selected += 1;
                            }
                            Delivery::Data { bytes, response } => {
                                assert_eq!(response.interrupt, 1);
                                displaced += fixture
                                    .bus
                                    .inner
                                    .deliver_cd_data(response.bytes[0], &bytes)
                                    .unwrap();
                                if exception_irqs {
                                    fixture.call(0x8017ee1c, &[], false);
                                } else {
                                    fixture.call(0x80177264, &[], true);
                                }
                                data += 1;
                            }
                            Delivery::Filtered => {}
                        }
                        sectors += 1;
                        // Explicit steady double-speed sector phase, independent
                        // of the one/four-sector game scheduler call ordering.
                        for _ in 0..294 {
                            let frame = fixture.bus.inner.step_spu_cd_output([0; 2]).unwrap();
                            signal |= frame != [0; 2];
                            for sample in frame {
                                pcm.extend_from_slice(&sample.to_le_bytes());
                            }
                        }
                    }
                }
                fixture.call(cue::TICK, &[], false);
                let state = fixture.state();
                if states.last() != Some(&state) {
                    states.push(state);
                }
                if state == 6 && stop_lba.is_none() {
                    stop_lba = Some(fixture.bus.read(0x80146814, Width::Word).unwrap());
                }
                if fixture.state() == 0 {
                    break;
                }
            }
            eprintln!("XA functional fixture ({sectors_per_tick} sectors/tick): {states:?}, {sectors} raw sectors, {selected} selected XA, {data} data, {displaced} displaced bytes, {frames} resampled frames, {} GetlocL commands", fixture.commands.iter().filter(|c| c.0 == 0x10).count());
            assert_eq!(fixture.state(), 0);
            if exception_irqs {
                assert!(fixture.irq_entries > data);
                assert_eq!(fixture.irq_entries, fixture.irq_returns);
                assert!(!fixture.bus.inner.interrupts().pending());
                println!("COP0 -> original SDK hook -> ReturnFromException: {} complete interrupts ({sectors_per_tick} sectors/tick)", fixture.irq_entries);
            }
            assert_eq!(states, [5, 6, 7, 0]);
            assert!(stop_lba.unwrap() >= cue.runtime_stop_threshold_lba);
            assert_eq!(
                fixture.bus.inner.cd_host().unwrap().volume().active(),
                [0; 4]
            );
            assert_eq!(fixture.drive.activity(), Activity::Paused);
            assert!(fixture.commands.iter().any(|c| c.0 == 0x10));
            assert!(fixture.commands.iter().any(|c| c.0 == 9));
            assert!(selected > 0 && frames > 0);
            if cue.stream == 1 {
                assert_eq!(data, 0, "first MAGIC cue contains audio sectors only");
            } else {
                assert!(data > 0);
            }
            let audio = fixture.bus.inner.cd_audio().unwrap().statistics();
            assert_eq!(audio.consumed_frames, frames as u64);
            assert_eq!(
                (
                    audio.queued_frames,
                    audio.dropped_sectors,
                    audio.empty_frames
                ),
                (0, 0, 0)
            );
            assert!(signal);
            assert_eq!(displaced, data.saturating_sub(1) * 2048);
            let outcome = transport::Outcome {
                cue: packed_id,
                sectors_per_tick,
                commands: fixture.commands.clone(),
                states,
                sectors,
                selected,
                data,
                frames,
                stop_lba: stop_lba.unwrap(),
                pcm_sha256: bof3_audio::digest::sha256_hex(&pcm),
            };
            if let Some(evidence) = &fixture.evidence {
                evidence.report(&cue, &outcome, fixture.bus.gpu_reads);
            }
            if matches!(vsync, Vsync::Candidate) {
                assert_eq!(fixture.bus.gpu_reads, 0);
            }
            proofs.push(fixture.proof());
            outcomes.push(outcome);
        }
    }
    outcomes
}
