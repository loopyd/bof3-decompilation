use bof3_audio::{
    machine::{
        adsr,
        bus::{Bus, Ram, Width},
        cd_audio::{Model as Resampling, Resampler, XaAudio},
        cd_queue::{Admission, Model, Queue, WaitOutcome, CAPACITY},
        interconnect::Interconnect,
        spu_sample,
    },
    xa::{Arithmetic, Decoder, Format, Histories, Stream},
};

fn stream() -> Stream {
    Stream {
        file: 1,
        channel: 0,
        coding: 0,
    }
}
fn volume() -> bof3_audio::machine::cd_audio::Volume {
    bof3_audio::machine::cd_audio::Volume::new([128, 0, 128, 0]).unwrap()
}

fn queue() -> Queue {
    Queue::new(
        stream(),
        Arithmetic::SplitFloor,
        Histories::default(),
        Resampling::EmulatorReference,
        Model::EmulatorReference,
    )
    .unwrap()
}
fn reference() -> XaAudio {
    XaAudio::new(
        stream(),
        Arithmetic::SplitFloor,
        Histories::default(),
        Resampling::EmulatorReference,
    )
    .unwrap()
}
fn sector(value: u8) -> Vec<u8> {
    let mut bytes = vec![0; 2336];
    bytes[..8].copy_from_slice(&[1, 0, 0x64, 0, 1, 0, 0x64, 0]);
    for group in 0..18 {
        let start = 8 + group * 128;
        bytes[start..start + 16].fill(0x18);
        bytes[start + 16..start + 128].fill(value);
    }
    bytes
}
fn drive_gain(frames: Vec<[i16; 2]>) -> Vec<[i16; 2]> {
    frames
        .into_iter()
        .map(|frame| {
            frame.map(|sample| {
                let scaled = (i64::from(sample) * 128 * 33617)
                    .div_euclid(1 << 22)
                    .clamp(-32768, 32767);
                (scaled * 31880).div_euclid(1 << 15) as i16
            })
        })
        .collect()
}

fn drain(queue: &mut Queue) -> Vec<[i16; 2]> {
    let mut frames = Vec::new();
    while queue.statistics().queued_frames != 0 {
        frames.push(queue.preview());
        queue.consume();
    }
    frames
}
fn bus(queue: Queue) -> Interconnect {
    let mut bus = Interconnect::from_ram(Ram::default());
    bus.configure_cd_host([128, 0, 128, 0]).unwrap();
    bus.configure_cd_audio(queue).unwrap();
    bus.configure_spu_voices(spu_sample::Model::Published, adsr::Model::Published)
        .unwrap();
    for (address, value) in [
        (0x1f801dac, 4),
        (0x1f801daa, 0x8011),
        (0x1f801db0, 0x4000),
        (0x1f801db2, 0x4000),
        (0x1f801d80, 0x2000),
        (0x1f801d82, 0x2000),
    ] {
        bus.write(address, Width::Half, value).unwrap();
    }
    bus
}
fn port(bus: &mut Interconnect, bank: u32, offset: u32, value: u32) {
    bus.write(0x1f801800, Width::Byte, bank).unwrap();
    bus.write(0x1f801800 + offset, Width::Byte, value).unwrap();
}

#[test]
fn whole_batch_wait_retries_and_timeout_retains_advanced_histories() {
    let mut queue = queue();
    let mut direct = reference();
    let mut expected = Vec::new();
    for value in [0x12, 0x23, 0x34] {
        let frames = drive_gain(direct.sector(&sector(value)).unwrap());
        assert_eq!(
            queue.sector(&sector(value), false, &volume()).unwrap(),
            Admission::Queued {
                frames: frames.len()
            }
        );
        expected.extend(frames);
    }
    let pending = drive_gain(direct.sector(&sector(0x45)).unwrap());
    let buffered = queue.statistics().queued_frames;
    assert_eq!(
        queue.sector(&sector(0x45), false, &volume()).unwrap(),
        Admission::Pending {
            frames: pending.len(),
            buffered_frames: buffered
        }
    );
    let before = queue.statistics();
    assert!(queue.sector(&sector(0x76), false, &volume()).is_err());
    assert_eq!(queue.statistics(), before);
    // Exact fill is excluded by the native strict inequality.
    let mut actual = Vec::new();
    while queue.statistics().queued_frames + pending.len() > CAPACITY {
        actual.push(queue.preview());
        queue.consume();
    }
    assert!(matches!(
        queue.finish_submission(WaitOutcome::Retry).unwrap(),
        Admission::Pending { .. }
    ));
    actual.push(queue.preview());
    queue.consume();
    // The predicate is checked again at the deadline; available room wins.
    assert_eq!(
        queue.finish_submission(WaitOutcome::TimedOut).unwrap(),
        Admission::Queued {
            frames: pending.len()
        }
    );
    expected.extend(pending);
    assert_eq!(queue.statistics().queued_frames, CAPACITY - 1);
    direct.sector(&sector(0x76)).unwrap(); // Timeout discards output, not histories.
    assert!(matches!(
        queue.sector(&sector(0x76), false, &volume()).unwrap(),
        Admission::Pending { .. }
    ));
    assert_eq!(
        queue.finish_submission(WaitOutcome::TimedOut).unwrap(),
        Admission::Dropped {
            buffered_frames: CAPACITY - 1
        }
    );
    assert_eq!(queue.statistics().pending_frames, 0);
    actual.extend(drain(&mut queue));
    assert_eq!(actual, expected);
    let next = drive_gain(direct.sector(&sector(0x56)).unwrap());
    queue.sector(&sector(0x56), false, &volume()).unwrap();
    assert_eq!(drain(&mut queue), next);
    assert!(queue.finish_submission(WaitOutcome::Retry).is_err());
    assert_eq!(queue.statistics().dropped_sectors, 1);
    assert_eq!(queue.statistics().admitted_sectors, 5);
}

#[test]
fn muted_sector_advances_predictors_only_and_keeps_buffered_tail() {
    let mut queue = queue();
    let mut decoder = Decoder::new(stream(), Arithmetic::SplitFloor, Histories::default()).unwrap();
    let mut filter = Resampler::new(
        Format::from_coding(0).unwrap(),
        Resampling::EmulatorReference,
    )
    .unwrap();
    let mut first = drive_gain(
        filter
            .process(&decoder.decode_sector(&sector(0x12)).unwrap())
            .unwrap(),
    );
    queue.sector(&sector(0x12), false, &volume()).unwrap();
    for _ in 0..2 {
        queue.sector(&sector(0x12), false, &volume()).unwrap();
        first.extend(drive_gain(
            filter
                .process(&decoder.decode_sector(&sector(0x12)).unwrap())
                .unwrap(),
        ));
    }
    // An audible batch would wait at this occupancy; mute still decodes only.
    let old = queue.preview();
    assert_eq!(
        queue.sector(&sector(0x67), true, &volume()).unwrap(),
        Admission::Muted
    );
    decoder.decode_sector(&sector(0x67)).unwrap(); // No interpolation while muted.
    assert_eq!(queue.preview(), old);
    assert_eq!(drain(&mut queue), first);
    let next = drive_gain(
        filter
            .process(&decoder.decode_sector(&sector(0x34)).unwrap())
            .unwrap(),
    );
    queue.sector(&sector(0x34), false, &volume()).unwrap();
    assert_eq!(drain(&mut queue), next);
    assert_eq!(queue.statistics().muted_sectors, 1);
}

#[test]
fn malformed_input_and_empty_output_do_not_advance_decoder_state() {
    let mut queue = queue();
    for _ in 0..30 {
        assert_eq!(queue.preview(), [0; 2]);
        queue.consume();
    }
    assert_eq!(queue.statistics().empty_frames, 30);
    let before = queue.statistics();
    let mut invalid = sector(0x12);
    invalid[8 + 17 * 128 + 4] = 0xff;
    for muted in [false, true] {
        assert!(queue.sector(&invalid, muted, &volume()).is_err());
        assert_eq!(queue.statistics(), before);
    }
    queue.sector(&sector(0x12), false, &volume()).unwrap();
    let before = queue.statistics();
    assert!(queue.sector(&invalid, false, &volume()).is_err()); // Validate input before any admission.
    assert_eq!(queue.statistics(), before);
    assert_eq!(
        drain(&mut queue),
        drive_gain(reference().sector(&sector(0x12)).unwrap())
    );
}

#[test]
fn buffered_frames_retain_admission_matrix_and_capture_post_drive_pre_spu_gain() {
    let mut bus = bus(queue());
    bus.enqueue_cd_audio(&sector(0x12)).unwrap();
    let reference = drive_gain(reference().sector(&sector(0x12)).unwrap());
    for (index, cd) in reference.iter().copied().take(300).enumerate() {
        if index == 100 {
            // Halve both drive channels after audio is queued.
            port(&mut bus, 2, 2, 64);
            port(&mut bus, 3, 1, 64);
            port(&mut bus, 3, 3, 0x20);
        }
        if index == 150 {
            port(&mut bus, 3, 3, 1);
        } // Mute future admission, not tail.
        let expected = cd.map(|v| ((i32::from(v) >> 1) >> 1) as i16);
        assert_eq!(bus.step_spu_cd_output([0; 2]).unwrap(), expected);
        for (channel, sample) in cd.into_iter().enumerate() {
            let at = channel * 1024 + index * 2;
            assert_eq!(&bus.spu_transfer().ram()[at..at + 2], &sample.to_le_bytes());
        }
    }
    assert!(reference[150..300].iter().any(|&f| f != [0; 2]));
    assert_eq!(bus.cd_audio().unwrap().statistics().consumed_frames, 300);
}

#[test]
fn failed_spu_output_retains_queue_and_reconfiguration_is_rejected() {
    let mut bus = bus(queue());
    bus.enqueue_cd_audio(&sector(0x12)).unwrap();
    let before = bus.cd_audio().unwrap().statistics();
    let frame = bus.cd_audio().unwrap().preview();
    bus.write(0x1f801dac, Width::Half, 0).unwrap();
    assert!(bus.step_spu_cd_output([0; 2]).is_err());
    assert_eq!(bus.cd_audio().unwrap().statistics(), before);
    assert_eq!(bus.cd_audio().unwrap().preview(), frame);
    assert!(bus.configure_cd_audio(queue()).is_err());
    bus.write(0x1f801dac, Width::Half, 4).unwrap();
    bus.step_spu_cd_output([0; 2]).unwrap();
    assert_eq!(bus.cd_audio().unwrap().statistics().consumed_frames, 1);
    while matches!(
        bus.enqueue_cd_audio(&sector(0x34)).unwrap(),
        Admission::Queued { .. }
    ) {}
    assert!(matches!(
        bus.finish_cd_audio_submission(WaitOutcome::Retry).unwrap(),
        Admission::Pending { .. }
    ));
    assert!(matches!(
        bus.finish_cd_audio_submission(WaitOutcome::TimedOut)
            .unwrap(),
        Admission::Dropped { .. }
    ));
}

#[test]
#[ignore = "requires BOF3_AUDIO_TRACK; raw VOICE through XA queue and SPU at explicit 150-sector/44100-frame boundaries"]
fn original_voice_queue_matches_direct_samples_at_steady_sector_rate() {
    use bof3_audio::{
        archive::disc::DiscImage,
        machine::{
            cd_drive::{Delivery, Drive},
            cd_position::CdPosition,
        },
    };
    let path = std::path::PathBuf::from(std::env::var_os("BOF3_AUDIO_TRACK").unwrap());
    let mut disc = DiscImage::open(&path).unwrap();
    let start = disc.files()["BIN/SCE_XA/VOICE.STR"].lba;
    let mut drive =
        Drive::ready(CdPosition::from_lba(start as i32).unwrap(), 0xc8, [1, 0]).unwrap();
    drive
        .command(
            &bof3_audio::machine::cd_host::Command {
                opcode: 0x1b,
                parameters: vec![],
            },
            0,
        )
        .unwrap();
    drive.complete(0).unwrap();
    drive.complete(0).unwrap();
    let mut bus = bus(queue());
    let mut direct = reference();
    let mut expected = std::collections::VecDeque::new();
    let mut selected = 0;
    let mut nonzero = false;
    for lba in start..start + 256 {
        let raw = disc.read_sector(lba).unwrap();
        if let Delivery::Xa(bytes) = drive.sector(Some(&raw)).unwrap() {
            assert!(matches!(
                bus.enqueue_cd_audio(&bytes).unwrap(),
                Admission::Queued { .. }
            ));
            expected.extend(drive_gain(direct.sector(&bytes).unwrap()));
            selected += 1;
        }
        for _ in 0..294 {
            // 44100 / 150. Phase is an explicit fixture input.
            let cd = expected.pop_front().expect("reference input exhausted");
            let output = bus.step_spu_cd_output([0; 2]).unwrap();
            assert_eq!(output, cd.map(|v| ((i32::from(v) >> 1) >> 1) as i16));
            nonzero |= output != [0; 2];
        }
    }
    assert_eq!(selected, 16);
    assert!(expected.is_empty() && nonzero);
    let stats = bus.cd_audio().unwrap().statistics();
    assert_eq!(stats.consumed_frames, 75264);
    assert_eq!(
        (
            stats.queued_frames,
            stats.empty_frames,
            stats.dropped_sectors
        ),
        (0, 0, 0)
    );
    eprintln!(
        "Raw VOICE queue/SPU: 256 sectors, 16 selected XA, 75,264 output frames; no drops/underruns at the supplied steady phase. This is not CPU timing or independent hardware PCM validation."
    );
}
