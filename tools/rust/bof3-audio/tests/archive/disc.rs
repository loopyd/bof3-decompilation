use bof3_audio::{archive::disc::DiscImage, digest::sha256_hex};
use std::{
    path::PathBuf,
    sync::atomic::{AtomicU64, Ordering},
};

struct Fixture(PathBuf);
impl Fixture {
    fn new(bytes: &[u8]) -> Self {
        static NEXT: AtomicU64 = AtomicU64::new(0);
        let path = std::env::temp_dir().join(format!(
            "bof3-disc-{}-{}.bin",
            std::process::id(),
            NEXT.fetch_add(1, Ordering::Relaxed)
        ));
        std::fs::write(&path, bytes).unwrap();
        Self(path)
    }
}
impl Drop for Fixture {
    fn drop(&mut self) {
        std::fs::remove_file(&self.0).unwrap();
    }
}

fn write_both32(bytes: &mut [u8], at: usize, value: u32) {
    bytes[at..at + 4].copy_from_slice(&value.to_le_bytes());
    bytes[at + 4..at + 8].copy_from_slice(&value.to_be_bytes());
}

fn record(name: &[u8], lba: u32, bytes: u32, directory: bool) -> Vec<u8> {
    let mut result = vec![0; (33 + name.len()).div_ceil(2) * 2];
    result[0] = result.len() as u8;
    write_both32(&mut result, 2, lba);
    write_both32(&mut result, 10, bytes);
    result[25] = if directory { 2 } else { 0 };
    result[32] = name.len() as u8;
    result[33..33 + name.len()].copy_from_slice(name);
    result
}

fn image() -> Vec<u8> {
    let mut result = vec![0; 32 * 2352];
    for sector in result.as_chunks_mut::<2352>().0.iter_mut() {
        sector[1..11].fill(255);
        sector[15] = 2;
    }
    let pvd = &mut result[16 * 2352 + 24..][..2048];
    pvd[..7].copy_from_slice(b"\x01CD001\x01");
    pvd[128..132].copy_from_slice(&[0, 8, 8, 0]);
    pvd[156..190].copy_from_slice(&record(&[0], 20, 2048, true));
    let file = record(b"SOUND.STR;1", 21, 10 * 2048, false);
    result[20 * 2352 + 24..][..file.len()].copy_from_slice(&file);
    for index in 21..31 {
        let sector = &mut result[index * 2352..][..2352];
        sector[16..20].copy_from_slice(&[1, (index % 2) as u8, 0x64, 0]);
        sector[20..24].copy_from_slice(&[1, (index % 2) as u8, 0x64, 0]);
        sector[24..].fill(index as u8);
    }
    result
}

#[test]
fn xa_extent_uses_iso_logical_blocks_and_retains_every_raw_payload_byte() {
    let bytes = image();
    let fixture = Fixture::new(&bytes);
    let mut disc = DiscImage::open(&fixture.0).unwrap();
    let entry = &disc.files()["SOUND.STR"];
    assert_eq!(entry.logical_bytes, 20480);
    assert_eq!(entry.sector_count(), 10);
    let stream = disc.read_xa("SOUND.STR").unwrap();
    assert_eq!(stream.bytes().len(), 23360);
    for index in 0..10 {
        assert_eq!(
            stream.sector(index).unwrap(),
            &bytes[(21 + index) * 2352 + 16..(22 + index) * 2352]
        );
    }
    assert_eq!(stream.bytes().last(), Some(&30));
    assert!(disc
        .read_file("SOUND.STR")
        .unwrap_err()
        .to_string()
        .contains("form-2"));
    assert!(disc.read_xa("missing").is_err());
}

#[test]
fn malformed_disc_extents_directory_cycles_and_endian_conflicts_fail() {
    let mut cases = Vec::new();
    let mut bytes = image();
    bytes.pop();
    cases.push(bytes);
    let mut bytes = image();
    bytes[16 * 2352 + 24 + 130] ^= 1;
    cases.push(bytes);
    let mut bytes = image();
    write_both32(&mut bytes, 20 * 2352 + 24 + 2, 32);
    cases.push(bytes);
    let mut bytes = image();
    bytes[20 * 2352 + 24 + 32] = 250;
    cases.push(bytes);
    let mut bytes = image();
    bytes[20 * 2352 + 24 + 25] = 0x80;
    cases.push(bytes);
    let mut bytes = image();
    let cycle = record(b"LOOP", 20, 2048, true);
    bytes[20 * 2352 + 24..][..cycle.len()].copy_from_slice(&cycle);
    cases.push(bytes);
    for bytes in cases {
        let fixture = Fixture::new(&bytes);
        assert!(DiscImage::open(&fixture.0).is_err());
    }
    let mut bytes = image();
    bytes[30 * 2352 + 20] ^= 1;
    let fixture = Fixture::new(&bytes);
    let mut disc = DiscImage::open(&fixture.0).unwrap();
    assert!(disc
        .read_xa("SOUND.STR")
        .unwrap_err()
        .to_string()
        .contains("subheader"));
}

#[test]
#[ignore = "requires original US Mode-2 data track in BOF3_AUDIO_TRACK"]
fn original_disc_stream_extents_include_audio_missing_from_older_extractions() {
    let path = PathBuf::from(std::env::var_os("BOF3_AUDIO_TRACK").expect("set BOF3_AUDIO_TRACK"));
    let mut disc = DiscImage::open(&path).unwrap();
    assert_eq!(disc.files().len(), 887);
    assert_eq!(
        sha256_hex(&disc.read_file("SLUS_004.22").unwrap()),
        bof3_audio::machine::profile::US_EXE_SHA256
    );
    for (path, count, digest) in [
        (
            "BIN/BMAG_XA/MAGIC00.STR",
            15408,
            "9916db03238af4469843db0e71891ab6680e8a3a9e755bcb2d96e2dfbf92d3fa",
        ),
        (
            "BIN/SCE_XA/S_XA00.STR",
            40416,
            "4dd8d034d4524aa907be793da1450f9f85cb67cd837ba412f1e328b719e0a37d",
        ),
        (
            "BIN/SCE_XA/VOICE.STR",
            3536,
            "f4f9d8ce9a2849501e8cd44dfcfee85aad4bc92cd9ff5113acde43c9362c65cc",
        ),
        (
            "LOGO/CAPCOM30.STR",
            1155,
            "c496080be2e134dba766e89a52bacbae7616239e1e26fe9d0c1242a92d12e915",
        ),
    ] {
        let image = disc.read_xa(path).unwrap();
        assert_eq!(image.sectors().len(), count);
        assert_eq!(image.bytes().len(), count * 2336);
        assert_eq!(sha256_hex(image.bytes()), digest);
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_TRACK and original BOF3_AUDIO_CORPUS"]
fn verification_reports_truncation_even_when_all_compared_bytes_are_equal() {
    let corpus =
        PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").expect("set BOF3_AUDIO_CORPUS"));
    let prior = corpus.join("BIN/SCE_XA/VOICE.STR");
    let output = std::process::Command::new(env!("CARGO_BIN_EXE_bof3-audio"))
        .args(["verify", "--mode", "audio", "--archive"])
        .arg(&prior)
        .arg("--against")
        .arg(&prior)
        .arg("--json")
        .output()
        .unwrap();
    assert_eq!(output.status.code(), Some(1));
    let report: serde_json::Value = serde_json::from_slice(&output.stdout).unwrap();
    assert_eq!(report["byte_equality"]["equal"], true);
    assert_eq!(
        report["media_completeness"]["status"],
        "known_truncated_disc_extent"
    );
    assert_eq!(report["media_completeness"]["missing_audio_sectors"], 0);
    let path = PathBuf::from(std::env::var_os("BOF3_AUDIO_TRACK").expect("set BOF3_AUDIO_TRACK"));
    let full = DiscImage::open(&path)
        .unwrap()
        .read_xa("BIN/SCE_XA/VOICE.STR")
        .unwrap();
    let image = bof3_audio::archive::MediaImage::Xa(full);
    let report =
        bof3_audio::verify::Report::inspect("complete.STR".into(), &image, Some(image.bytes()));
    assert!(report.byte_equality.unwrap().equal);
    assert_eq!(
        report.media_completeness.unwrap().status,
        "known_complete_disc_extent"
    );
    assert!(bof3_audio::xa::reference::identify(&"0".repeat(64)).is_none());
}
