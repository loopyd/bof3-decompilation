mod support;

use bof3_disk_v2::{Error, Image};
use std::fs;

fn assert_extracts_fixture(raw: bool) {
    let root = support::temp_root(if raw { "raw" } else { "iso" });
    fs::create_dir_all(&root).unwrap();
    let image_path = root.join(if raw { "fixture.bin" } else { "fixture.iso" });
    fs::write(&image_path, support::fixture(raw, false)).unwrap();
    let paths = Image::open(&image_path)
        .unwrap()
        .extract(root.join("out"))
        .unwrap();
    assert_eq!(paths.len(), 2);
    assert_eq!(fs::read(root.join("out/HELLO.TXT")).unwrap(), b"hello");
    assert_eq!(
        fs::read(root.join("out/DIR/DATA.BIN")).unwrap(),
        [0, 1, 255]
    );
    support::remove_temp_root(&root);
}

#[test]
fn extracts_cooked_iso_fixture_byte_exactly() {
    assert_extracts_fixture(false);
}

#[test]
fn extracts_raw_mode2_fixture_byte_exactly() {
    let root = support::temp_root("raw-license");
    fs::create_dir_all(&root).unwrap();
    let image_path = root.join("fixture.bin");
    fs::write(&image_path, support::fixture(true, false)).unwrap();
    Image::open(&image_path)
        .unwrap()
        .extract(root.join("out"))
        .unwrap();
    assert_eq!(fs::read(root.join("out/HELLO.TXT")).unwrap(), b"hello");
    assert_eq!(
        fs::metadata(root.join("out/license_data.dat"))
            .unwrap()
            .len(),
        28_032
    );
    support::remove_temp_root(&root);
}

#[test]
fn rejects_disagreeing_both_endian_fields() {
    let root = support::temp_root("bad-endian");
    fs::create_dir_all(&root).unwrap();
    let image = root.join("fixture.iso");
    fs::write(&image, support::fixture(false, true)).unwrap();
    assert!(matches!(
        Image::open(&image).unwrap().extract(root.join("out")),
        Err(Error::InvalidImage(_))
    ));
    support::remove_temp_root(&root);
}

#[test]
fn extracts_xa_entries_as_2336_bytes_per_sector() {
    let root = support::temp_root("xa");
    fs::create_dir_all(&root).unwrap();
    let image = root.join("fixture.bin");
    fs::write(&image, support::xa_fixture()).unwrap();

    let mut parsed = Image::open(&image).unwrap();
    let entry = parsed
        .entries()
        .unwrap()
        .into_iter()
        .find(|entry| entry.path == std::path::Path::new("STREAM.XA"))
        .unwrap();
    assert!(entry.is_xa);
    assert_eq!(entry.xa.unwrap().group_id, 0x1234);
    assert_eq!(entry.xa.unwrap().user_id, 0x5678);
    assert_eq!(entry.xa.unwrap().attributes, 0x1055);
    assert_eq!(entry.xa.unwrap().file_number, 1);
    Image::open(&image)
        .unwrap()
        .extract(root.join("out"))
        .unwrap();
    let payload = fs::read(root.join("out/STREAM.XA")).unwrap();
    assert_eq!(payload.len(), 4_672);
    assert_eq!(payload[4_671], (4_671 % 251) as u8);

    support::remove_temp_root(&root);
}

#[test]
fn extracts_multifile_cue_audio_track_as_wav() {
    let root = support::temp_root("cue-audio");
    fs::create_dir_all(&root).unwrap();
    fs::write(root.join("disc_track01.bin"), support::fixture(true, false)).unwrap();
    fs::write(root.join("disc_track02.bin"), [0_u8; 2352]).unwrap();
    fs::write(
        root.join("disc.cue"),
        "FILE \"disc_track01.bin\" BINARY\n  TRACK 01 MODE2/2352\n    INDEX 01 00:00:00\nFILE \"disc_track02.bin\" BINARY\n  TRACK 02 AUDIO\n    INDEX 01 00:00:00\n",
    )
    .unwrap();
    let paths = Image::open(root.join("disc.cue"))
        .unwrap()
        .extract(root.join("out"))
        .unwrap();
    assert!(paths.iter().any(|path| path.ends_with("track02.wav")));
    let wav = hound::WavReader::open(root.join("out/track02.wav")).unwrap();
    assert_eq!(wav.duration(), 588);
    support::remove_temp_root(&root);
}

#[test]
#[should_panic]
fn cleanup_rejects_non_test_directories() {
    support::remove_temp_root(std::path::Path::new("out"));
}
