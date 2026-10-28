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
fn volume() -> bof3_audio::machine::cd_audio::Volume {
    bof3_audio::machine::cd_audio::Volume::new([128, 0, 128, 0]).unwrap()
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
fn init_preserves_mute_queued_audio_data_cursor_and_volume() {
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
    assert!(!reset.xa_predictors_reset && reset.audio_selection_released);
    assert!(drive.muted());
    assert_eq!(bus.cd_host().unwrap().volume().active(), volume);
    assert_eq!(bus.cd_audio().unwrap().statistics().queued_frames, frames);
    assert_eq!(bus.read(0x1f801802, Width::Byte).unwrap(), 1);
    drive.complete(0).unwrap();
    assert_eq!(
        bus.enqueue_cd_audio(&sector(1, 1)).unwrap(),
        Admission::Muted
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
        0,
    )
    .unwrap()
}

#[test]
fn reset_rebinds_channel_and_format_with_fresh_history_after_validation() {
    let mut q = queue(0, 0);
    q.sector(&sector(0, 0), false, &volume()).unwrap();
    for _ in 0..31 {
        q.consume();
    }
    for _ in 0..2 {
        assert!(matches!(
            q.sector(&sector(0, 0), false, &volume()).unwrap(),
            Admission::Queued { .. }
        ));
    }
    assert!(matches!(
        q.sector(&sector(0, 0), false, &volume()).unwrap(),
        Admission::Pending { .. }
    ));
    let discarded = q.statistics().queued_frames + q.statistics().pending_frames;
    assert_eq!(q.reset(), discarded);
    assert_eq!(q.statistics().pending_frames, 0);
    assert_eq!(q.preview(), [0; 2]);
    let before = q.statistics();
    let mut invalid = sector(1, 1);
    invalid[5] = 3;
    assert!(q.sector(&invalid, false, &volume()).is_err());
    assert_eq!(q.statistics(), before);
    let mut fresh = queue(2, 5);
    let bytes = sector(2, 5);
    assert_eq!(
        q.sector(&bytes, false, &volume()).unwrap(),
        fresh.sector(&bytes, false, &volume()).unwrap()
    );
    while fresh.statistics().queued_frames != 0 {
        assert_eq!(q.preview(), fresh.preview());
        q.consume();
        fresh.consume();
    }
    assert_eq!(q.statistics().reset_discarded_frames, discarded as u64);
    assert_eq!(q.statistics().resets, 1);
    assert!(q.sector(&sector(3, 5), false, &volume()).is_err()); // No mid-stream rebind.
}

#[test]
fn setloc_and_seek_preserve_audio_and_data_buffers() {
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
    assert!(!setloc.xa_predictors_reset);
    assert_eq!(bus.cd_audio().unwrap().statistics().queued_frames, frames);
    let seek = apply(&mut bus, &mut drive, 0x16, &[]);
    assert!(!seek.xa_predictors_reset);
    assert_eq!(bus.cd_audio().unwrap().statistics().queued_frames, frames);
    bus.write(0x1f801803, Width::Byte, 0x80).unwrap();
    assert_eq!(bus.read(0x1f801802, Width::Byte).unwrap(), 7);
    let seeking = drive.activity();
    let muted = apply(&mut bus, &mut drive, 0x0b, &[]);
    assert_eq!(muted.response.bytes, [2]);
    assert!(!muted.xa_predictors_reset);
    assert!(drive.muted());
    assert_eq!(drive.activity(), seeking);
    drive.complete(0).unwrap();
    drive.complete(0).unwrap();
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
            },
            0
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
    assert!(drive.muted() && !response.xa_predictors_reset);
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
fn pause_preserves_buffers_and_each_read_resets_only_predictors() {
    let (mut bus, mut drive) = context();
    apply(&mut bus, &mut drive, 0x1b, &[]);
    drive.complete(0).unwrap();
    drive.complete(0).unwrap();
    bus.enqueue_cd_audio(&sector(0, 0)).unwrap();
    bus.present_cd_data(&[7; 2048]).unwrap();
    let stats = bus.cd_audio().unwrap().statistics();
    for (index, explicit_target) in [false, true].into_iter().enumerate() {
        if explicit_target {
            apply(
                &mut bus,
                &mut drive,
                2,
                &CdPosition::from_lba(100).unwrap().bcd(),
            );
        }
        assert!(apply(&mut bus, &mut drive, 0x1b, &[]).xa_predictors_reset);
        drive.complete(0).unwrap();
        drive.complete(0).unwrap();
        let mut expected = stats;
        expected.predictor_resets += index as u64 + 1;
        assert_eq!(bus.cd_audio().unwrap().statistics(), expected);
    }
    let pause = apply(&mut bus, &mut drive, 9, &[]);
    assert_eq!(pause.response.bytes, [0x22]);
    assert!(!pause.xa_predictors_reset);
    assert_eq!(
        bus.cd_audio().unwrap().statistics().queued_frames,
        stats.queued_frames
    );
    bus.write(0x1f801803, Width::Byte, 0x80).unwrap();
    assert_eq!(bus.read(0x1f801802, Width::Byte).unwrap(), 7);
    drive.complete(0).unwrap();
    assert_eq!(drive.activity(), Activity::Paused);
}
