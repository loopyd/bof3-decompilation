use bof3_audio::{
    machine::{
        adsr,
        bus::{Bus, Ram, Width},
        cd_audio::{Model as Resampling, Resampler, XaAudio},
        cd_queue::{Admission, Model, Queue},
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
fn backlog_drop_preserves_histories_and_ten_frame_boundary() {
    let mut queue = queue();
    let mut direct = reference();
    let expected_first = direct.sector(&sector(0x12)).unwrap();
    assert_eq!(
        queue.sector(&sector(0x12), false).unwrap(),
        Admission::Queued {
            frames: expected_first.len()
        }
    );
    assert_eq!(
        queue.sector(&sector(0x76), false).unwrap(),
        Admission::Dropped {
            buffered_frames: expected_first.len()
        }
    );
    let mut actual = Vec::new();
    while queue.statistics().queued_frames > 11 {
        actual.push(queue.preview());
        queue.consume();
    }
    assert_eq!(
        queue.sector(&sector(0x65), false).unwrap(),
        Admission::Dropped {
            buffered_frames: 11
        }
    );
    actual.push(queue.preview());
    queue.consume();
    let second = direct.sector(&sector(0x34)).unwrap();
    assert_eq!(
        queue.sector(&sector(0x34), false).unwrap(),
        Admission::Queued {
            frames: second.len()
        }
    );
    actual.extend(drain(&mut queue));
    assert_eq!(
        actual,
        expected_first.into_iter().chain(second).collect::<Vec<_>>()
    );
    assert_eq!(queue.statistics().dropped_sectors, 2);
    assert_eq!(queue.statistics().admitted_sectors, 2);
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
    let first = filter
        .process(&decoder.decode_sector(&sector(0x12)).unwrap())
        .unwrap();
    queue.sector(&sector(0x12), false).unwrap();
    for _ in 0..first.len() - 10 {
        queue.consume();
    }
    let old = queue.preview();
    assert_eq!(queue.sector(&sector(0x67), true).unwrap(), Admission::Muted);
    decoder.decode_sector(&sector(0x67)).unwrap(); // No interpolation while muted.
    assert_eq!(queue.preview(), old);
    assert_eq!(drain(&mut queue), first[first.len() - 10..]);
    let next = filter
        .process(&decoder.decode_sector(&sector(0x34)).unwrap())
        .unwrap();
    queue.sector(&sector(0x34), false).unwrap();
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
        assert!(queue.sector(&invalid, muted).is_err());
        assert_eq!(queue.statistics(), before);
    }
    queue.sector(&sector(0x12), false).unwrap();
    let before = queue.statistics();
    assert!(queue.sector(&invalid, false).is_err()); // Validate even dropped input.
    assert_eq!(queue.statistics(), before);
    assert_eq!(
        drain(&mut queue),
        reference().sector(&sector(0x12)).unwrap()
    );
}

#[test]
fn buffered_frames_use_current_matrix_and_capture_post_drive_pre_spu_gain() {
    let mut bus = bus(queue());
    bus.enqueue_cd_audio(&sector(0x12)).unwrap();
    let reference = reference().sector(&sector(0x12)).unwrap();
    for (index, raw) in reference.iter().copied().take(300).enumerate() {
        if index == 100 {
            // Halve both drive channels after audio is queued.
            port(&mut bus, 2, 2, 64);
            port(&mut bus, 3, 1, 64);
            port(&mut bus, 3, 3, 0x20);
        }
        if index == 150 {
            port(&mut bus, 3, 3, 1);
        } // Mute future admission, not tail.
        let gain = if index < 100 { 128 } else { 64 };
        let cd = raw.map(|v| ((i32::from(v) * gain) >> 7) as i16);
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
        .command(&bof3_audio::machine::cd_host::Command {
            opcode: 0x1b,
            parameters: vec![],
        })
        .unwrap();
    drive.complete().unwrap();
    let mut bus = bus(queue());
    let mut direct = reference();
    let mut expected = std::collections::VecDeque::new();
    let mut selected = 0;
    let mut nonzero = false;
    for lba in start..start + 256 {
        let raw = disc.read_sector(lba).unwrap();
        if let Delivery::Xa(bytes) = drive.sector(&raw).unwrap() {
            assert!(matches!(
                bus.enqueue_cd_audio(&bytes).unwrap(),
                Admission::Queued { .. }
            ));
            expected.extend(direct.sector(&bytes).unwrap());
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
    eprintln!("Raw VOICE queue/SPU: 256 sectors, 16 selected XA, 75,264 output frames; no drops/underruns at the supplied steady phase. This is not CPU timing or independent hardware PCM validation.");
}
