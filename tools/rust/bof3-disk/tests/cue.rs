use std::path::Path;

use bof3_disk_v2::cue::{extract_audio_tracks, format_timecode, parse, TrackMode};
use std::fs;
use tempfile::tempdir;

#[test]
fn parses_multifile_data_and_audio_tracks() {
    let cue = parse(
        "FILE \"game_track01.bin\" BINARY\r\n\
         TRACK 01 MODE2/2352\r\n\
         INDEX 01 00:00:00\r\n\
         FILE \"game_track02.bin\" BINARY\r\n\
         TRACK 02 AUDIO\r\n\
         INDEX 00 00:00:00\r\n\
         INDEX 01 00:02:00\r\n",
        Path::new("disc"),
    )
    .unwrap();
    assert_eq!(cue.tracks.len(), 2);
    assert_eq!(cue.tracks[0].mode, TrackMode::Mode2_2352);
    assert_eq!(cue.tracks[0].file, Path::new("disc/game_track01.bin"));
    assert_eq!(cue.tracks[1].mode, TrackMode::Audio);
    assert_eq!(cue.tracks[1].index00, Some(0));
    assert_eq!(cue.tracks[1].index01, 150);
    assert_eq!(format_timecode(cue.tracks[1].index01), "00:02:00");
}

#[test]
fn rejects_track_without_index01() {
    assert!(parse(
        "FILE game.bin BINARY\nTRACK 01 MODE2/2352\nINDEX 00 00:00:00\n",
        Path::new("."),
    )
    .is_err());
}

#[test]
fn extracts_cdda_to_standard_stereo_wav() {
    let root = tempdir().unwrap();
    let audio = root.path().join("track.bin");
    let mut sector = Vec::with_capacity(2352);
    for index in 0..1176_i16 {
        sector.extend_from_slice(&index.to_le_bytes());
    }
    fs::write(&audio, sector).unwrap();
    let sheet = parse(
        "FILE \"track.bin\" BINARY\nTRACK 02 AUDIO\nINDEX 01 00:00:00\n",
        root.path(),
    )
    .unwrap();
    let paths = extract_audio_tracks(&sheet, &root.path().join("out")).unwrap();
    assert_eq!(paths.len(), 1);
    let mut wav = hound::WavReader::open(&paths[0]).unwrap();
    assert_eq!(wav.spec().channels, 2);
    assert_eq!(wav.spec().sample_rate, 44_100);
    assert_eq!(wav.samples::<i16>().count(), 1176);
}
