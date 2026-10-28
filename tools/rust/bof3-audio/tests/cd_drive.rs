use bof3_audio::{
    codec::edc,
    machine::{
        cd_drive::{Activity, Delivery, Drive, Response},
        cd_host::{Command, Host},
        cd_position::CdPosition,
    },
};

fn command(opcode: u8, parameters: &[u8]) -> Command {
    Command {
        opcode,
        parameters: parameters.to_vec(),
    }
}
fn drive(mode: u8) -> Drive {
    Drive::ready(CdPosition::from_lba(1234).unwrap(), mode, [1, 0]).unwrap()
}
fn start(drive: &mut Drive) {
    assert_eq!(drive.command(&command(6, &[]), 0).unwrap().bytes, [2]);
    assert_eq!(drive.status(), 2);
    assert_eq!(drive.complete(0).unwrap(), None);
    assert_eq!(drive.status(), 0x42);
    assert_eq!(drive.complete(0).unwrap(), None);
    assert_eq!(drive.status(), 0x22);
}
fn sector(lba: i32, subheader: [u8; 4]) -> Vec<u8> {
    let mut raw = vec![0; 2352];
    raw[1..11].fill(255);
    raw[12..15].copy_from_slice(&CdPosition::from_lba(lba).unwrap().bcd());
    raw[15] = 2;
    raw[16..20].copy_from_slice(&subheader);
    raw[20..24].copy_from_slice(&subheader);
    raw[24..2072].fill(0x5a);
    if subheader[2] & 0x20 == 0 {
        let crc = edc::checksum(&raw[16..2072]);
        raw[2072..2076].copy_from_slice(&crc.to_le_bytes());
    }
    raw
}

#[test]
fn configuration_errors_and_unsupported_commands_do_not_mutate_drive() {
    let mut drive = drive(0xe8);
    for cmd in [
        command(2, &[0, 0x6a, 0]),
        command(2, &[0, 0, 0x75]),
        command(1, &[0]),
    ] {
        let before = drive.clone();
        let response = drive.command(&cmd, 0).unwrap();
        assert_eq!(response.interrupt, 5);
        assert_eq!(response.bytes[0], 3);
        assert_eq!(response.bytes[1], if cmd.opcode == 1 { 0x20 } else { 0x10 });
        assert_eq!(drive, before);
    }
    for cmd in [
        command(0x0e, &[0xff]),
        command(0x1c, &[]),
        command(2, &[0, 0, 0]),
    ] {
        let before = drive.clone();
        assert!(drive.command(&cmd, 0).is_err());
        assert_eq!(drive, before);
    }
    assert_eq!(
        drive.command(&command(0x10, &[]), 0).unwrap().bytes,
        [3, 0x80]
    );
    drive.command(&command(0x0d, &[7, 31]), 0).unwrap();
    assert_eq!(
        drive.command(&command(0x0f, &[]), 0).unwrap().bytes,
        [2, 0xe8, 0, 7, 31]
    );
    assert!(drive.complete(0).is_err());
}

#[test]
fn setloc_seek_read_pause_and_restart_have_distinct_boundaries() {
    let mut drive = drive(0);
    let position = CdPosition::from_lba(555).unwrap();
    drive.command(&command(2, &position.bcd()), 0).unwrap();
    assert_eq!(drive.next_lba(), 1234);
    drive.command(&command(0x15, &[]), 0).unwrap();
    assert_eq!(
        drive.activity(),
        Activity::PreparingSeek {
            target: 555,
            physical: false,
            status: 0
        }
    );
    assert!(drive.command(&command(9, &[]), 0).is_err());
    assert!(drive.sector(Some(&sector(555, [1, 0, 8, 0]))).is_err());
    assert_eq!(drive.complete(0).unwrap(), None);
    assert_eq!(drive.next_lba(), 1234);
    assert_eq!(drive.complete(0).unwrap().unwrap().bytes, [2]);
    assert_eq!(drive.next_lba(), 555);
    start(&mut drive);
    drive.sector(Some(&sector(555, [1, 0, 8, 0]))).unwrap();
    // Setloc alone does not redirect the currently running read.
    drive
        .command(&command(2, &CdPosition::from_lba(777).unwrap().bcd()), 0)
        .unwrap();
    assert_eq!(drive.next_lba(), 556);
    drive.sector(Some(&sector(556, [1, 0, 8, 0]))).unwrap();
    drive.command(&command(0x1b, &[]), 0).unwrap();
    assert_eq!(drive.activity(), Activity::PreparingRead { target: 777 });
    drive.complete(0).unwrap();
    drive.complete(0).unwrap();
    let latest = sector(777, [1, 0, 8, 0]);
    drive.sector(Some(&latest)).unwrap();
    assert_eq!(
        drive.command(&command(0x10, &[]), 0).unwrap().bytes,
        latest[12..20]
    );
    assert_eq!(drive.command(&command(9, &[]), 0).unwrap().bytes, [0x22]);
    assert_eq!(drive.status(), 2);
    assert_eq!(
        drive.complete(0).unwrap(),
        Some(Response {
            interrupt: 2,
            bytes: vec![2]
        })
    );
    start(&mut drive);
    assert_eq!(drive.next_lba(), 777); // No Setloc: restart at the retained seek target.
    drive.command(&command(9, &[]), 0).unwrap();
    drive.complete(0).unwrap();
    drive.command(&command(9, &[]), 0).unwrap();
    assert_eq!(drive.status(), 2); // Pausing an already-paused drive is not reading.
}

#[test]
fn filtering_distinguishes_realtime_audio_from_data_and_updates_latest_header() {
    let mut drive = drive(0xe8);
    start(&mut drive);
    let xa = sector(1234, [1, 0, 0xe4, 0]); // EOF remains XA data.
    assert_eq!(
        drive.sector(Some(&xa)).unwrap(),
        Delivery::Xa(xa[16..].to_vec())
    );
    let other = sector(1235, [1, 1, 0x64, 0]);
    assert_eq!(drive.sector(Some(&other)).unwrap(), Delivery::Filtered);
    assert_eq!(
        drive.command(&command(0x10, &[]), 0).unwrap().bytes,
        other[12..20]
    );
    // File/channel filtering must not reject ordinary data on first delivery.
    let data = sector(1236, [7, 31, 8, 0]);
    assert_eq!(
        drive.sector(Some(&data)).unwrap(),
        Delivery::Data {
            bytes: data[12..].to_vec(),
            response: Response {
                interrupt: 1,
                bytes: vec![0x22]
            },
        }
    );
    drive.command(&command(0x0e, &[0x08]), 0).unwrap();
    assert_eq!(
        drive.sector(Some(&sector(1237, [1, 0, 0x64, 0]))).unwrap(),
        Delivery::Filtered
    );
    let data = sector(1238, [7, 31, 8, 0]);
    let Delivery::Data { bytes, .. } = drive.sector(Some(&data)).unwrap() else {
        panic!()
    };
    assert_eq!(bytes, data[24..2072]);
    drive.command(&command(0x0e, &[0]), 0).unwrap();
    assert!(matches!(
        drive.sector(Some(&sector(1239, [1, 0, 0x64, 0]))).unwrap(),
        Delivery::Data { .. }
    ));
}

#[test]
fn malformed_or_unsupported_sector_does_not_advance_position_or_header() {
    let mut drive = drive(0xe8);
    start(&mut drive);
    let raw = sector(1234, [1, 0, 8, 0]);
    let mut cases = vec![raw[..2336].to_vec(), sector(1235, [1, 0, 8, 0])];
    for at in [1, 15, 20, 2072, 24] {
        let mut bad = raw.clone();
        bad[at] ^= 1;
        cases.push(bad);
    }
    let mut form2 = sector(1234, [1, 0, 0x64, 0]);
    form2[2351] = 1;
    cases.push(form2);
    for bad in cases {
        let before = drive.clone();
        assert!(drive.sector(Some(&bad)).is_err());
        assert_eq!(drive, before);
    }
}

#[test]
fn drive_responses_use_existing_host_irq_and_drain_contract() {
    let mut drive = drive(0);
    let mut host = Host::new([128, 0, 128, 0]).unwrap();
    host.write(0, 1).unwrap();
    host.write(2, 0x1f).unwrap();
    host.write(0, 0).unwrap();
    host.write(1, 0x0f).unwrap();
    let cmd = host.service_fifo().unwrap();
    let response = drive.command(&cmd, 0).unwrap();
    host.stage_response(&response.bytes).unwrap();
    assert!(!host.irq_line());
    host.raise_interrupt(response.interrupt).unwrap();
    assert!(host.irq_line());
    host.write(1, 1).unwrap();
    assert!(host.service_fifo().is_none());
    let bytes: Vec<_> = (0..5).map(|_| host.read(1).unwrap()).collect();
    assert_eq!(bytes, [2, 0, 0, 1, 0]);
    host.write(0, 1).unwrap();
    host.write(3, 0x1f).unwrap();
    assert!(!host.irq_line());
}

#[test]
fn init_resets_positions_and_preserves_mode_mute_filter_and_header() {
    let mut drive = Drive::ready(CdPosition::from_lba(100).unwrap(), 0xe8, [7, 31]).unwrap();
    drive.command(&command(6, &[]), 0).unwrap();
    drive.complete(0).unwrap();
    drive.complete(0).unwrap();
    let header = sector(100, [7, 31, 8, 0]);
    drive.sector(Some(&header)).unwrap();
    drive.command(&command(0x0b, &[]), 0).unwrap();
    let target = CdPosition::from_lba(300).unwrap();
    drive.command(&command(2, &target.bcd()), 0).unwrap();
    let before = drive.clone();
    let bad = drive.command(&command(0x0a, &[0]), 0).unwrap();
    assert_eq!((bad.interrupt, bad.bytes), (5, vec![0x23, 0x20]));
    assert_eq!(drive, before);

    let ack = drive.command(&command(0x0a, &[]), 0).unwrap();
    assert_eq!((ack.interrupt, ack.bytes), (3, vec![0x22]));
    assert_eq!(drive.activity(), Activity::Resetting);
    assert!(drive.muted());
    assert_eq!(
        drive.command(&command(0x10, &[]), 0).unwrap().bytes,
        header[12..20]
    );
    assert_eq!(
        drive.command(&command(0x0f, &[]), 0).unwrap().bytes,
        [2, 0xe8, 0, 7, 31]
    );
    let pending = drive.clone();
    assert!(drive.command(&command(6, &[]), 0).is_err());
    assert!(drive.command(&command(0x0a, &[]), 0).is_err());
    assert!(drive.sector(Some(&[0; 2352])).is_err());
    assert_eq!(drive, pending);

    let completed = drive.complete(0).unwrap().unwrap();
    assert_eq!((completed.interrupt, completed.bytes), (2, vec![2]));
    assert_eq!(drive.activity(), Activity::Paused);
    assert_eq!(drive.next_lba(), 0);
    assert!(drive.complete(0).is_err());
    drive.command(&command(6, &[]), 0).unwrap();
    assert_eq!(drive.activity(), Activity::PreparingRead { target: 0 });
}
