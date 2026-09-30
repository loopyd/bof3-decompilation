use bof3_audio::interchange::midi::{Event, Message, Midi, Track};
use std::io::Cursor;

fn chunk(id: &[u8; 4], payload: &[u8], bytes: &mut Vec<u8>) {
    bytes.extend(id);
    bytes.extend((payload.len() as u32).to_be_bytes());
    bytes.extend(payload);
}

fn raw(format: u16, ppqn: u16, tracks: &[&[u8]]) -> Vec<u8> {
    let mut bytes = Vec::new();
    let header: Vec<_> = [format, tracks.len() as u16, ppqn]
        .into_iter()
        .flat_map(u16::to_be_bytes)
        .collect();
    chunk(b"MThd", &header, &mut bytes);
    for track in tracks {
        chunk(b"MTrk", track, &mut bytes);
    }
    bytes
}

fn channel(tick: u64, status: u8, data: &[u8]) -> Event {
    Event {
        tick,
        message: Message::Channel {
            status,
            data: data.to_vec(),
        },
    }
}

fn meta(tick: u64, kind: u8, data: &[u8]) -> Event {
    Event {
        tick,
        message: Message::Meta {
            kind,
            data: data.to_vec(),
        },
    }
}

#[test]
fn unchanged_file_and_unedited_tracks_preserve_all_encoding_and_opaque_data() {
    let first = [
        0x80, 0, 0xff, 3, 0x80, 1, b'x', 0, 0xc0, 7, 0, 8, 127, 0xb0, 7, 100, 0, 10, 64, 0, 0xff,
        0x7f, 3, 0, 255, 127, 0, 0xf0, 3, 0x43, 1, 0xf7, 0, 0xf7, 2, 0xf8, 0xfa, 0, 0xff, 0x2f, 0,
    ];
    let second = [0, 0x90, 60, 100, 96, 60, 0, 0, 0xff, 0x2f, 0];
    let assemble = |ppqn: u8, second: &[u8]| {
        let mut bytes = Vec::new();
        chunk(b"MThd", &[0, 1, 0, 2, 0, ppqn, 0xde, 0xad], &mut bytes);
        chunk(b"BEFR", &[0, 3, 9], &mut bytes);
        chunk(b"MTrk", &first, &mut bytes);
        chunk(b"BTWN", &[255], &mut bytes);
        chunk(b"MTrk", second, &mut bytes);
        chunk(b"AFTR", &[7, 8], &mut bytes);
        bytes
    };
    let original = assemble(96, &second);
    let mut midi = Midi::from_bytes(&original).unwrap();
    assert_eq!(midi.to_bytes().unwrap(), original);
    assert_eq!(midi.tracks[0].events[2], channel(0, 0xc0, &[8]));
    assert_eq!(midi.tracks[0].events[4], channel(127, 0xb0, &[10, 64]));
    midi.ppqn = 48;
    midi.tracks[1].events[0] = channel(0, 0x90, &[60, 101]);
    let edited = midi.to_bytes().unwrap();
    assert_eq!(
        edited,
        assemble(48, &[0, 0x90, 60, 101, 96, 0x90, 60, 0, 0, 0xff, 0x2f, 0])
    );
    let read = Midi::from_bytes(&edited).unwrap();
    assert_eq!(read.tracks[0].events, midi.tracks[0].events);
    assert_eq!(read.tracks[1].events, midi.tracks[1].events);
}

#[test]
fn format_one_tempo_map_and_performance_are_accepted_by_independent_rustysynth() {
    let conductor = Track::new(vec![
        meta(0, 3, b"BOF3 synthetic conductor"),
        meta(0, 0x58, &[4, 2, 24, 8]),
        meta(0, 0x51, &[7, 0xa1, 0x20]),
        meta(96, 0x51, &[15, 0x42, 0x40]),
        meta(192, 0x2f, &[]),
    ]);
    let performance = Track::new(vec![
        channel(0, 0xc0, &[5]),
        channel(0, 0xb0, &[7, 100]),
        channel(0, 0x90, &[60, 100]),
        channel(96, 0xe0, &[0, 65]),
        channel(96, 0x90, &[60, 0]),
        channel(96, 0xc0, &[6]),
        channel(96, 0x90, &[67, 90]),
        channel(192, 0x80, &[67, 64]),
        meta(192, 0x2f, &[]),
    ]);
    let midi = Midi::new(1, 96, vec![conductor, performance]).unwrap();
    let bytes = midi.to_bytes().unwrap();
    let independent = rustysynth::MidiFile::new(&mut Cursor::new(&bytes)).unwrap();
    assert!((independent.get_length() - 1.5).abs() < 1e-12);
    let decoded = Midi::from_bytes(&bytes).unwrap();
    assert_eq!(decoded.format, 1);
    for (a, b) in decoded.tracks.iter().zip(&midi.tracks) {
        assert_eq!(a.events, b.events);
    }
    // An actual parsed tempo edit changes independent timing; preservation data
    // must not override it. The second quarter note becomes 0.25 seconds.
    let mut edited = decoded;
    edited.tracks[0].events[3] = meta(96, 0x51, &[3, 0xd0, 0x90]);
    let independent =
        rustysynth::MidiFile::new(&mut Cursor::new(edited.to_bytes().unwrap())).unwrap();
    assert!((independent.get_length() - 0.75).abs() < 1e-12);
}

#[test]
fn malformed_chunks_messages_running_status_and_unrepresentable_edits_fail() {
    let good = raw(0, 96, &[&[0, 0xc0, 5, 0, 0xff, 0x2f, 0]]);
    for len in 0..good.len() {
        assert!(
            Midi::from_bytes(&good[..len]).is_err(),
            "accepted prefix {len}"
        );
    }
    for track in [
        vec![],
        vec![0, 60, 100],
        vec![0, 0x90, 60, 128],
        vec![0, 0xf1, 0],
        vec![0x80; 5],
        vec![0, 0xff, 0x80, 0],
        vec![0, 0xff, 0x2f, 1, 0],
        vec![0, 0xff, 0x2f, 0, 0],
        vec![0, 0xff, 1, 0x81, 0],
        vec![0, 0xc0, 5, 0, 0xff, 1, 0, 0, 6, 0, 0xff, 0x2f, 0],
        vec![0, 0xc0, 5, 0, 0xf7, 0, 0, 6, 0, 0xff, 0x2f, 0],
    ] {
        assert!(
            Midi::from_bytes(&raw(0, 96, &[&track])).is_err(),
            "accepted {track:02x?}"
        );
    }
    for (format, division) in [(2, 96), (0, 0), (0, 0xe250)] {
        assert!(Midi::from_bytes(&raw(format, division, &[&[0, 0xff, 0x2f, 0]])).is_err());
    }
    assert!(Midi::from_bytes(&raw(0, 96, &[&[0, 0xff, 0x2f, 0], &[0, 0xff, 0x2f, 0]])).is_err());
    let mut wrong_count = good.clone();
    wrong_count[11] = 2;
    assert!(Midi::from_bytes(&wrong_count).is_err());
    let mut wrong_length = good.clone();
    wrong_length[4..8].copy_from_slice(&u32::MAX.to_be_bytes());
    assert!(Midi::from_bytes(&wrong_length).is_err());
    let parsed = Midi::from_bytes(&good).unwrap();
    for message in [
        Message::Channel {
            status: 0xc0,
            data: vec![1, 2],
        },
        Message::Channel {
            status: 0x90,
            data: vec![1, 128],
        },
        Message::Meta {
            kind: 0x2f,
            data: vec![],
        },
        Message::SysEx {
            status: 0xf1,
            data: vec![],
        },
    ] {
        let mut edited = parsed.clone();
        edited.tracks[0].events[0].message = message;
        assert!(edited.to_bytes().is_err());
    }
    let mut edited = parsed.clone();
    edited.tracks[0].events[0].tick = 1;
    assert!(edited.to_bytes().is_err());
    let mut edited = parsed.clone();
    edited.tracks[0].events[1].tick = 0x1000_0000;
    assert!(edited.to_bytes().is_err());
    let mut edited = parsed;
    edited.tracks.push(Track::new(vec![meta(0, 0x2f, &[])]));
    assert!(edited.to_bytes().is_err());
}

#[test]
fn vlq_boundaries_and_every_channel_message_shape_round_trip() {
    let mut tick = 0;
    let mut events = Vec::new();
    for delta in [0, 127, 128, 16383, 16384, 2097151, 2097152, 0x0fff_ffff] {
        tick += delta;
        for status in 0x80..=0xef {
            let data = if matches!(status >> 4, 12 | 13) {
                vec![127]
            } else {
                vec![0, 127]
            };
            events.push(channel(tick, status, &data));
        }
    }
    events.push(meta(tick, 0x2f, &[]));
    let midi = Midi::new(0, 32767, vec![Track::new(events.clone())]).unwrap();
    let parsed = Midi::from_bytes(&midi.to_bytes().unwrap()).unwrap();
    assert_eq!(parsed.tracks[0].events, events);
}

#[test]
fn unknown_meta_and_extended_known_meta_remain_opaque_across_edits() {
    let mut midi = Midi::new(
        0,
        48,
        vec![Track::new(vec![
            meta(0, 0x51, &[7, 0xa1, 0x20, 99]),
            meta(0, 0x70, &[0, 255, 42]),
            meta(48, 0x2f, &[]),
        ])],
    )
    .unwrap();
    let bytes = midi.to_bytes().unwrap();
    midi = Midi::from_bytes(&bytes).unwrap();
    midi.tracks[0].events[2].tick = 96;
    let parsed = Midi::from_bytes(&midi.to_bytes().unwrap()).unwrap();
    assert_eq!(parsed.tracks[0].events[..2], midi.tracks[0].events[..2]);
    assert_eq!(parsed.tracks[0].events[2].tick, 96);
}
