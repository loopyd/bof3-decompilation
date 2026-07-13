#[allow(dead_code)]
mod support;

use bof3_disk_v2::{metadata, Entry, Image};
use std::fs;

#[test]
fn emits_project_and_lba_metadata_from_iso_records() {
    let root = support::temp_root("metadata");
    fs::create_dir_all(&root).unwrap();
    let image_path = root.join("fixture.iso");
    fs::write(&image_path, support::fixture(false, false)).unwrap();
    let mut image = Image::open(&image_path).unwrap();
    let entries = image.entries().unwrap();

    let project = root.join("fixture.xml");
    metadata::write_project(&project, &entries, "fixture.iso").unwrap();
    let xml = fs::read_to_string(project).unwrap();
    assert!(xml.contains("source=\"HELLO.TXT\""));
    assert!(xml.contains("source=\"DIR/DATA.BIN\""));

    let lba = root.join("disc_lba.json");
    metadata::write_lba_json(
        &lba,
        &entries,
        std::path::Path::new("raw"),
        std::path::Path::new("extract"),
    )
    .unwrap();
    let json = fs::read_to_string(lba).unwrap();
    assert_eq!(json, include_str!("fixtures/disc_lba_basic.json"));

    support::remove_temp_root(&root);
}

#[test]
fn maps_emi_manifests_like_v1_inside_and_outside_bin() {
    let root = support::temp_root("metadata-emi-manifest");
    fs::create_dir_all(&root).unwrap();
    let output = root.join("disc_lba.json");
    let entries = [
        Entry {
            path: "BIN/ETC/GAME.EMI".into(),
            lba: 30,
            size: 2048,
            is_directory: false,
            is_xa: false,
            xa: None,
        },
        Entry {
            path: "ROOT.EMI".into(),
            lba: 31,
            size: 2048,
            is_directory: false,
            is_xa: false,
            xa: None,
        },
    ];

    metadata::write_lba_json(
        &output,
        &entries,
        std::path::Path::new("raw"),
        std::path::Path::new("extract"),
    )
    .unwrap();
    let json = fs::read_to_string(output).unwrap();
    assert!(json.contains(r#""manifest_path": "raw/BIN/ETC/GAME/emi.json""#));
    assert!(json.contains(r#""manifest_path": "extract/ROOT/emi.json""#));

    support::remove_temp_root(&root);
}
