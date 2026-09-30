use bof3_audio::{
    machine::{
        bus::{Bus, Ram, Width},
        cd_audio::Model as Resampling,
        cd_drive::{Activity, AppliedCommand, Drive},
        cd_host::Command,
        cd_position::CdPosition,
        cd_queue::{Admission, Model, Queue},
        interconnect::Interconnect,
    },
    xa::{Arithmetic, Histories, Stream},
};

fn stream(channel: u8, coding: u8) -> Stream {
    Stream {
        file: 1,
        channel,
        coding,
    }
}
fn queue(channel: u8, coding: u8) -> Queue {
    Queue::new(
        stream(channel, coding),
        Arithmetic::SplitFloor,
        Histories::default(),
        Resampling::EmulatorReference,
        Model::EmulatorReference,
    )
    .unwrap()
}
fn sector(channel: u8, coding: u8) -> Vec<u8> {
    let mut bytes = vec![0; 2336];
    bytes[..8].copy_from_slice(&[1, channel, 0x64, coding, 1, channel, 0x64, coding]);
    for group in 0..18 {
        let at = 8 + group * 128;
        bytes[at..at + 16].fill(0x18);
        bytes[at + 16..at + 128].fill(0x21);
    }
    bytes
}
fn context() -> (Interconnect, Drive) {
    let mut bus = Interconnect::from_ram(Ram::default());
    bus.configure_cd_host([128, 0, 128, 0]).unwrap();
    bus.configure_cd_audio(queue(0, 0)).unwrap();
    bus.configure_cd_data(bof3_audio::machine::cd_data::Model::EmulatorReference)
        .unwrap();
    let drive = Drive::ready(CdPosition::from_lba(100).unwrap(), 0xc8, [1, 0]).unwrap();
    (bus, drive)
}

#[test]
fn init_unmutes_and_preserves_queued_audio_data_cursor_and_volume() {
    let (mut bus, mut drive) = context();
    let Admission::Queued { frames } = bus.enqueue_cd_audio(&sector(0, 0)).unwrap() else {
        panic!("expected queued audio");
    };
    let data: Vec<u8> = (0..2048).map(|i| (i % 256) as u8).collect();
    bus.present_cd_data(&data).unwrap();
    bus.write(0x1f801800, Width::Byte, 0).unwrap();
    bus.write(0x1f801803, Width::Byte, 0x80).unwrap();
    assert_eq!(bus.read(0x1f801802, Width::Byte).unwrap(), 0);
    apply(&mut bus, &mut drive, 0x0b, &[]);
    let volume = bus.cd_host().unwrap().volume().active();
    let reset = apply(&mut bus, &mut drive, 0x0a, &[]);
    assert_eq!(reset.response.bytes, [2]);
    assert!(!reset.audio_reset && reset.audio_selection_released);
    assert_eq!(
        (reset.discarded_audio_frames, reset.discarded_data_bytes),
        (0, 0)
    );
    assert!(!drive.muted());
    assert_eq!(bus.cd_host().unwrap().volume().active(), volume);
    assert_eq!(bus.cd_audio().unwrap().statistics().queued_frames, frames);
    assert_eq!(bus.read(0x1f801802, Width::Byte).unwrap(), 1);
    drive.complete().unwrap();
    assert_eq!(
        bus.enqueue_cd_audio(&sector(1, 1)).unwrap(),
        Admission::Dropped {
            buffered_frames: frames
        }
    );
}
fn apply(
    bus: &mut Interconnect,
    drive: &mut Drive,
    opcode: u8,
    parameters: &[u8],
) -> AppliedCommand {
    bus.apply_cd_drive_command(
        drive,
        &Command {
            opcode,
            parameters: parameters.to_vec(),
        },
    )
    .unwrap()
}

#[test]
fn reset_rebinds_channel_and_format_with_fresh_history_after_validation() {
    let mut q = queue(0, 0);
    q.sector(&sector(0, 0), false).unwrap();
    for _ in 0..31 {
        q.consume();
    }
    let discarded = q.statistics().queued_frames;
    assert_eq!(q.reset(), discarded);
    assert_eq!(q.preview(), [0; 2]);
    let before = q.statistics();
    let mut invalid = sector(1, 1);
    invalid[5] = 3;
    assert!(q.sector(&invalid, false).is_err());
    assert_eq!(q.statistics(), before);
    let mut fresh = queue(2, 5);
    let bytes = sector(2, 5);
    assert_eq!(
        q.sector(&bytes, false).unwrap(),
        fresh.sector(&bytes, false).unwrap()
    );
    while fresh.statistics().queued_frames != 0 {
        assert_eq!(q.preview(), fresh.preview());
        q.consume();
        fresh.consume();
    }
    assert_eq!(q.statistics().reset_discarded_frames, discarded as u64);
    assert_eq!(q.statistics().resets, 1);
    assert!(q.sector(&sector(3, 5), false).is_err()); // No mid-stream rebind.
}

#[test]
fn setloc_preserves_buffers_and_seek_atomically_discards_selected_state() {
    let (mut bus, mut drive) = context();
    bus.enqueue_cd_audio(&sector(0, 0)).unwrap();
    let frames = bus.cd_audio().unwrap().statistics().queued_frames;
    bus.present_cd_data(&[7; 2048]).unwrap();
    let setloc = apply(
        &mut bus,
        &mut drive,
        2,
        &CdPosition::from_lba(200).unwrap().bcd(),
    );
    assert!(!setloc.audio_reset);
    assert_eq!(bus.cd_audio().unwrap().statistics().queued_frames, frames);
    let seek = apply(&mut bus, &mut drive, 0x16, &[]);
    assert!(seek.audio_reset);
    assert_eq!(
        (seek.discarded_audio_frames, seek.discarded_data_bytes),
        (frames, 2048)
    );
    assert_eq!(bus.cd_audio().unwrap().statistics().queued_frames, 0);
    assert!(bus.write(0x1f801803, Width::Byte, 0x80).is_err());
    let seeking = drive.activity();
    let muted = apply(&mut bus, &mut drive, 0x0b, &[]);
    assert_eq!(muted.response.bytes, [0x42]);
    assert!(!muted.audio_reset);
    assert!(drive.muted());
    assert_eq!(drive.activity(), seeking);
    drive.complete().unwrap();
    assert_eq!(drive.next_lba(), 200);
    assert_eq!(drive.activity(), Activity::Paused);
}

#[test]
fn invalid_seek_and_active_dma_failure_preserve_drive_and_queues() {
    let (mut bus, mut drive) = context();
    bus.enqueue_cd_audio(&sector(0, 0)).unwrap();
    bus.present_cd_data(&[7; 2048]).unwrap();
    let state = drive.clone();
    let stats = bus.cd_audio().unwrap().statistics();
    assert_eq!(
        apply(&mut bus, &mut drive, 2, &[0, 0x60, 0])
            .response
            .interrupt,
        5
    );
    assert_eq!(drive, state);
    bus.write(0x1f8010b4, Width::Word, 512).unwrap();
    bus.write(0x1f8010b8, Width::Word, 0x11000000).unwrap();
    assert!(bus
        .apply_cd_drive_command(
            &mut drive,
            &Command {
                opcode: 0x15,
                parameters: vec![]
            }
        )
        .is_err());
    assert_eq!(drive, state);
    assert_eq!(bus.cd_audio().unwrap().statistics(), stats);
    bus.write(0x1f801803, Width::Byte, 0x80).unwrap();
    assert_eq!(bus.read(0x1f801802, Width::Byte).unwrap(), 7);
}

#[test]
fn command_mute_and_host_mute_are_independent_and_keep_existing_output() {
    let (mut bus, mut drive) = context();
    let response = apply(&mut bus, &mut drive, 0x0b, &[]);
    assert_eq!(response.response.bytes, [2]);
    assert!(drive.muted() && !response.audio_reset);
    assert_eq!(
        bus.enqueue_cd_audio(&sector(0, 0)).unwrap(),
        Admission::Muted
    );
    apply(&mut bus, &mut drive, 0x0c, &[]);
    assert!(!drive.muted());
    bus.write(0x1f801800, Width::Byte, 3).unwrap();
    bus.write(0x1f801803, Width::Byte, 1).unwrap();
    assert_eq!(
        bus.enqueue_cd_audio(&sector(0, 0)).unwrap(),
        Admission::Muted
    );
    bus.write(0x1f801803, Width::Byte, 0).unwrap();
    assert!(matches!(
        bus.enqueue_cd_audio(&sector(0, 0)).unwrap(),
        Admission::Queued { .. }
    ));
    let before = bus.cd_audio().unwrap().statistics();
    apply(&mut bus, &mut drive, 0x0b, &[]);
    assert_eq!(bus.cd_audio().unwrap().statistics(), before);
}

#[test]
fn pause_discards_audio_but_keeps_data_and_read_continuation_does_not_reset() {
    let (mut bus, mut drive) = context();
    apply(&mut bus, &mut drive, 0x1b, &[]);
    drive.complete().unwrap();
    bus.enqueue_cd_audio(&sector(0, 0)).unwrap();
    bus.present_cd_data(&[7; 2048]).unwrap();
    let stats = bus.cd_audio().unwrap().statistics();
    for explicit_target in [false, true] {
        if explicit_target {
            apply(
                &mut bus,
                &mut drive,
                2,
                &CdPosition::from_lba(100).unwrap().bcd(),
            );
        }
        assert!(!apply(&mut bus, &mut drive, 0x1b, &[]).audio_reset);
        assert_eq!(bus.cd_audio().unwrap().statistics(), stats);
    }
    let pause = apply(&mut bus, &mut drive, 9, &[]);
    assert_eq!(pause.response.bytes, [0x22]);
    assert!(pause.audio_reset);
    assert_eq!(pause.discarded_audio_frames, stats.queued_frames);
    assert_eq!(pause.discarded_data_bytes, 0);
    bus.write(0x1f801803, Width::Byte, 0x80).unwrap();
    assert_eq!(bus.read(0x1f801802, Width::Byte).unwrap(), 7);
    drive.complete().unwrap();
    assert_eq!(drive.activity(), Activity::Paused);
}
