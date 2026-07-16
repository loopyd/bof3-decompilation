#[allow(dead_code)]
mod support;

use std::fs;
use std::path::Path;
use std::process::{Command, Output};

use emi_ex_v2::Archive;
use tempfile::tempdir;

fn emi_ex() -> &'static str {
    env!("CARGO_BIN_EXE_emi-ex")
}

fn run(current_dir: &Path, args: &[&str]) -> Output {
    Command::new(emi_ex())
        .current_dir(current_dir)
        .args(args)
        .output()
        .expect("emi-ex must execute")
}

fn assert_success(output: &Output) {
    assert!(
        output.status.success(),
        "status={}\nstdout={}\nstderr={}",
        output.status,
        String::from_utf8_lossy(&output.stdout),
        String::from_utf8_lossy(&output.stderr)
    );
}

#[test]
fn cli_typed_manifest_roundtrip_is_byte_exact() {
    let root = tempdir().unwrap();
    fs::write(root.path().join("source.EMI"), support::fixture()).unwrap();

    let extracted = root.path().join("entries");
    let extract = run(
        root.path(),
        &[
            "extract",
            "-e",
            "-J",
            "-q",
            "-o",
            extracted.to_str().unwrap(),
            "source.EMI",
        ],
    );
    assert_success(&extract);
    assert_eq!(fs::read(extracted.join("0.vh")).unwrap(), b"ABCDx");
    assert_eq!(fs::read(extracted.join("1.bin")).unwrap(), b"yz");

    let repacked = root.path().join("repacked.EMI");
    let pack = run(
        root.path(),
        &[
            "pack",
            "-o",
            repacked.to_str().unwrap(),
            "-J",
            extracted.join("emi.json").to_str().unwrap(),
            extracted.to_str().unwrap(),
        ],
    );
    assert_success(&pack);
    assert_eq!(fs::read(repacked).unwrap(), support::fixture());
}

#[test]
fn cli_manifest_pack_preserves_version_type_and_ram_pointer() {
    let root = tempdir().unwrap();
    fs::write(root.path().join("payload.bin"), b"\x10\x20\x30\x40\x50").unwrap();
    let archive_path = root.path().join("packed.EMI");
    fs::write(
        root.path().join("emi.json"),
        r#"{"archive_version":7,"entries":[{"name":"payload.bin","type":10,"ram_ptr":2148676694}]}"#,
    )
    .unwrap();

    let output = run(
        root.path(),
        &[
            "pack",
            "-o",
            archive_path.to_str().unwrap(),
            "-J",
            "emi.json",
            ".",
        ],
    );
    assert_success(&output);

    let archive = Archive::open(&archive_path).unwrap();
    assert_eq!(archive.version(), 7);
    assert_eq!(archive.entries().len(), 1);
    assert_eq!(archive.entries()[0].file_type, 10);
    assert_eq!(archive.entries()[0].ram_ptr, 0x8012_3456);
    assert_eq!(archive.entries()[0].first4, 0x4030_2010);
    archive
        .extract_entry(0, root.path().join("payload.out"))
        .unwrap();
    assert_eq!(
        fs::read(root.path().join("payload.out")).unwrap(),
        b"\x10\x20\x30\x40\x50"
    );
}

#[test]
fn manifest_parser_accepts_v1_defaults() {
    let root = tempdir().unwrap();
    fs::write(root.path().join("0.bin"), b"ABCDx").unwrap();
    fs::write(
        root.path().join("emi.json"),
        r#"{"entries":[null,{}, {"name":""}, {"name":"0.bin"}]}"#,
    )
    .unwrap();

    let output = run(
        root.path(),
        &["pack", "-o", "packed.EMI", "-J", "emi.json", "."],
    );
    assert_success(&output);
    let archive = Archive::open(root.path().join("packed.EMI")).unwrap();
    assert_eq!(archive.version(), 0);
    assert_eq!(archive.entries().len(), 1);
    assert_eq!(archive.entries()[0].file_type, 0);
    assert_eq!(archive.entries()[0].ram_ptr, 0);
}

#[test]
fn single_index_extract_uses_existing_dotted_directory() {
    let root = tempdir().unwrap();
    fs::write(root.path().join("source.EMI"), support::fixture()).unwrap();
    fs::create_dir(root.path().join("output.dir")).unwrap();

    let output = run(
        root.path(),
        &["extract", "-e", "-o", "output.dir", "source.EMI", "0"],
    );
    assert_success(&output);
    assert_eq!(
        fs::read(root.path().join("output.dir/0.vh")).unwrap(),
        b"ABCDx"
    );
}
