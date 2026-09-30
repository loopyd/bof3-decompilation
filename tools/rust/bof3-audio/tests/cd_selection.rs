use bof3_audio::{
    archive::disc::DiscImage,
    machine::{
        bus::Ram,
        cd_audio::{Model as Resampling, XaAudio},
        cd_drive::{Delivery, Drive},
        cd_host::Command,
        cd_position::CdPosition,
        cd_queue::{Admission, Model, Queue},
        interconnect::Interconnect,
    },
    xa::{Arithmetic, Histories, Stream},
};

fn stream(channel: u8) -> Stream {
    Stream {
        file: 1,
        channel,
        coding: 0,
    }
}

fn queue() -> Queue {
    Queue::new(
        stream(0),
        Arithmetic::SplitFloor,
        Histories::default(),
        Resampling::EmulatorReference,
        Model::EmulatorReference,
    )
    .unwrap()
}

fn audio() -> XaAudio {
    XaAudio::new(
        stream(0),
        Arithmetic::SplitFloor,
        Histories::default(),
        Resampling::EmulatorReference,
    )
    .unwrap()
}

fn sector(channel: u8, flags: u8, value: u8) -> Vec<u8> {
    let mut bytes = vec![0; 2336];
    bytes[..8].copy_from_slice(&[1, channel, flags, 0, 1, channel, flags, 0]);
    for group in bytes[8..2312].as_chunks_mut::<128>().0 {
        group[..16].fill(0x18);
        group[16..].fill(value);
    }
    bytes
}

fn raw(lba: i32, bytes: &[u8]) -> Vec<u8> {
    let mut raw = vec![0; 2352];
    raw[1..11].fill(255);
    raw[12..15].copy_from_slice(&CdPosition::from_lba(lba).unwrap().bcd());
    raw[15] = 2;
    raw[16..].copy_from_slice(bytes);
    raw
}

fn command(opcode: u8, parameters: &[u8]) -> Command {
    Command {
        opcode,
        parameters: parameters.to_vec(),
    }
}

fn drain(queue: &mut Queue) -> Vec<[i16; 2]> {
    let mut frames = Vec::new();
    while queue.statistics().queued_frames != 0 {
        frames.push(queue.preview());
        queue.consume();
    }
    frames
}

#[test]
fn automatic_selection_ignores_unrelated_eof_and_releases_matching_eof() {
    let mut drive = Drive::ready(CdPosition::from_lba(100).unwrap(), 0x40, [9, 9]).unwrap();
    drive.command(&command(6, &[])).unwrap();
    drive.complete().unwrap();
    // 255 does not claim automatic selection. EOR is not EOF. A mismatched
    // channel's EOF must not release the already selected channel.
    for (index, (channel, flags, selected)) in [
        (255, 0x64, false),
        (2, 0x65, true),
        (3, 0xe4, false),
        (3, 0x64, false),
        (2, 0xe4, true),
        (3, 0x64, true),
        (2, 0x64, false),
    ]
    .into_iter()
    .enumerate()
    {
        let bytes = sector(channel, flags, 0x21);
        let delivery = drive.sector(&raw(100 + index as i32, &bytes)).unwrap();
        assert_eq!(
            delivery,
            if selected {
                Delivery::Xa(bytes)
            } else {
                Delivery::Filtered
            }
        );
    }
    // Setfilter releases the automatic latch even when mode filtering is off.
    drive.command(&command(0x0d, &[1, 7])).unwrap();
    assert!(matches!(
        drive.sector(&raw(107, &sector(4, 0x64, 0))).unwrap(),
        Delivery::Xa(_)
    ));
    // Filter configuration can explicitly select channel 255.
    drive.command(&command(0x0e, &[0x48])).unwrap();
    drive.command(&command(0x0d, &[1, 255])).unwrap();
    assert!(matches!(
        drive.sector(&raw(108, &sector(255, 0x64, 0))).unwrap(),
        Delivery::Xa(_)
    ));
}

#[test]
fn eof_rebinding_keeps_predictors_filter_history_and_tail_for_each_admission() {
    for admission in 0..3 {
        let mut q = queue();
        let mut reference = audio();
        let first = sector(0, 0x64, 0x12);
        q.sector(&first, false).unwrap();
        let first_frames = reference.sector(&first).unwrap();
        if admission != 2 {
            assert_eq!(drain(&mut q), first_frames);
        }
        let eof = sector(0, 0xe4, 0x76);
        let result = q.sector(&eof, admission == 1).unwrap();
        match admission {
            0 => {
                assert!(matches!(result, Admission::Queued { .. }));
                assert_eq!(drain(&mut q), reference.sector(&eof).unwrap());
            }
            1 => {
                assert_eq!(result, Admission::Muted);
                // Obtain the predictor-only reference via the public decoder
                // and resampler below; no private release API is the oracle.
            }
            _ => {
                assert!(matches!(result, Admission::Dropped { .. }));
                assert_eq!(drain(&mut q), first_frames);
            }
        }
        let next = sector(1, 0x64, 0x34);
        q.sector(&next, false).unwrap();
        let normalized = sector(0, 0x64, 0x34);
        let expected = if admission == 1 {
            use bof3_audio::{machine::cd_audio::Resampler, xa::Decoder};
            let mut decoder =
                Decoder::new(stream(0), Arithmetic::SplitFloor, Histories::default()).unwrap();
            let mut resampler =
                Resampler::new(decoder.format(), Resampling::EmulatorReference).unwrap();
            resampler
                .process(&decoder.decode_sector(&first).unwrap())
                .unwrap();
            decoder.decode_sector(&eof).unwrap();
            resampler
                .process(&decoder.decode_sector(&normalized).unwrap())
                .unwrap()
        } else {
            reference.sector(&normalized).unwrap()
        };
        assert_ne!(
            expected,
            audio().sector(&normalized).unwrap(),
            "must distinguish retained history from reset"
        );
        assert_eq!(drain(&mut q), expected);
        assert!(q.sector(&sector(2, 0x64, 0), false).is_err());
    }
}

#[test]
fn rejected_eof_and_unsupported_coding_preserve_selection_and_audio_state() {
    let mut q = queue();
    q.sector(&sector(0, 0x64, 0x12), false).unwrap();
    let before = q.statistics();
    let mut bad = sector(0, 0xe4, 0);
    bad[8 + 17 * 128 + 4] = 0xf8;
    assert!(q.sector(&bad, false).is_err());
    assert_eq!(q.statistics(), before);
    assert!(q.sector(&sector(1, 0x64, 0), false).is_err());
    q.sector(&sector(0, 0xe4, 0), false).unwrap(); // dropped, but releases selection
    let before = q.statistics();
    let mut changed = sector(1, 0x64, 0);
    changed[3] = 0x40;
    changed[7] = 0x40;
    assert!(q
        .sector(&changed, false)
        .unwrap_err()
        .to_string()
        .contains("emphasis"));
    assert_eq!(q.statistics(), before);
    assert!(matches!(
        q.sector(&sector(1, 0x64, 0), false).unwrap(),
        Admission::Dropped { .. }
    ));
    // A dropped first sector still acquires selection; another channel fails.
    assert!(q.sector(&sector(2, 0x64, 0), false).is_err());
}

#[test]
fn setfilter_releases_only_selection_and_invalid_parameters_do_not_release_it() {
    let mut bus = Interconnect::from_ram(Ram::default());
    bus.configure_cd_host([128, 0, 128, 0]).unwrap();
    bus.configure_cd_audio(queue()).unwrap();
    let mut drive = Drive::ready(CdPosition::from_lba(100).unwrap(), 0x48, [1, 0]).unwrap();
    let first = sector(0, 0x64, 0x12);
    bus.enqueue_cd_audio(&first).unwrap();
    let before = bus.cd_audio().unwrap().statistics();
    let invalid = bus
        .apply_cd_drive_command(&mut drive, &command(0x0d, &[1]))
        .unwrap();
    assert!(!invalid.audio_selection_released);
    assert_eq!(invalid.response.interrupt, 5);
    assert!(bus.enqueue_cd_audio(&sector(1, 0x64, 0)).is_err());
    let effect = bus
        .apply_cd_drive_command(&mut drive, &command(0x0d, &[1, 1]))
        .unwrap();
    assert!(effect.audio_selection_released);
    assert!(!effect.audio_reset);
    assert_eq!(effect.discarded_audio_frames, 0);
    assert_eq!(bus.cd_audio().unwrap().statistics(), before);
    // Stream identity changes without clearing the already queued block.
    assert!(matches!(
        bus.enqueue_cd_audio(&sector(1, 0x64, 0x76)).unwrap(),
        Admission::Dropped { .. }
    ));
    assert_eq!(
        bus.cd_audio().unwrap().statistics().queued_frames,
        before.queued_frames
    );
}

#[test]
fn selection_release_preserves_tail_and_continuous_decoder_history() {
    let mut q = queue();
    let mut reference = audio();
    let first = sector(0, 0x64, 0x12);
    q.sector(&first, false).unwrap();
    let expected = reference.sector(&first).unwrap();
    let before = q.statistics();
    q.release_selection();
    assert_eq!(q.statistics(), before);
    assert_eq!(drain(&mut q), expected);
    q.sector(&sector(1, 0x64, 0x34), false).unwrap();
    let normalized = sector(0, 0x64, 0x34);
    let expected = reference.sector(&normalized).unwrap();
    assert_ne!(expected, audio().sector(&normalized).unwrap());
    assert_eq!(drain(&mut q), expected);
}

#[test]
#[ignore = "requires BOF3_AUDIO_TRACK; whole raw VOICE extent automatic selection"]
fn raw_voice_automatic_selection_decodes_complete_first_stream() {
    let path = std::path::PathBuf::from(std::env::var_os("BOF3_AUDIO_TRACK").unwrap());
    let mut disc = DiscImage::open(&path).unwrap();
    let entry = &disc.files()["BIN/SCE_XA/VOICE.STR"];
    let start = entry.lba;
    let count = entry.sector_count();
    let mut drive =
        Drive::ready(CdPosition::from_lba(start as i32).unwrap(), 0xc0, [0, 0]).unwrap();
    drive.command(&command(6, &[])).unwrap();
    drive.complete().unwrap();
    let mut q = queue();
    q.reset();
    let mut selected = 0;
    let mut eof = 0;
    let mut channels = std::collections::BTreeSet::new();
    let mut frames = 0;
    let mut reference = audio();
    let mut eof_at = None;
    for index in 0..count {
        let bytes = disc.read_sector(start + index as u32).unwrap();
        if let Delivery::Xa(bytes) = drive.sector(&bytes).unwrap() {
            selected += 1;
            eof += usize::from(bytes[2] & 0x80 != 0);
            if bytes[2] & 0x80 != 0 {
                eof_at = Some(index);
            }
            channels.insert((bytes[0], bytes[1]));
            assert!(matches!(
                q.sector(&bytes, false).unwrap(),
                Admission::Queued { .. }
            ));
            let actual = drain(&mut q);
            assert_eq!(actual, reference.sector(&bytes).unwrap());
            frames += actual.len();
        }
    }
    println!("Raw VOICE selection: {count} sectors, {selected} selected, {eof} EOF at sector {eof_at:?}, {channels:?}, {frames} frames; explicit drain per sector, no timing claim");
    // This corpus has no cross-channel handoff after the selected EOF. The
    // synthetic tests above deliberately exercise that separate requirement.
    assert_eq!((count, selected, eof, frames), (3536, 71, 1, 333984));
    assert_eq!(channels.into_iter().collect::<Vec<_>>(), [(1, 0)]);
    assert_eq!(q.statistics().dropped_sectors, 0);
}
