mod support;

use emi_ex_v2::{Archive, Error};
use std::fs;

#[test]
fn parses_and_extracts_fixture() {
    let root = support::temp_root("extract");
    fs::create_dir_all(&root).unwrap();
    let archive_path = root.join("test.emi");
    fs::write(&archive_path, support::fixture()).unwrap();
    let archive = Archive::open(&archive_path).unwrap();
    assert_eq!(archive.version(), 1);
    assert_eq!(archive.entries().len(), 2);
    assert_eq!(archive.entries()[0].ram_ptr, 0x8010_0000);
    archive.extract_all(root.join("out"), true).unwrap();
    assert_eq!(fs::read(root.join("out/0.vh")).unwrap(), b"ABCDx");
    assert_eq!(fs::read(root.join("out/1.bin")).unwrap(), b"yz");
    support::remove_temp_root(&root);
}

#[test]
fn rejects_invalid_magic() {
    let root = support::temp_root("invalid");
    fs::create_dir_all(&root).unwrap();
    let path = root.join("invalid.emi");
    fs::write(&path, [0_u8; 16]).unwrap();
    assert!(matches!(
        Archive::open(&path),
        Err(Error::InvalidArchive("bad magic"))
    ));
    support::remove_temp_root(&root);
}

#[test]
fn manifest_matches_v1_field_order_and_format() {
    let root = support::temp_root("manifest");
    fs::create_dir_all(&root).unwrap();
    let path = root.join("test.emi");
    fs::write(&path, support::fixture()).unwrap();
    let archive = Archive::open(&path).unwrap();
    let expected = "{\n  \"archive_version\": 1,\n  \"entries\": [\n    {\n      \"first4\": 1145258561,\n      \"index\": 0,\n      \"name\": \"0.vh\",\n      \"ram_ptr\": 2148532224,\n      \"size\": 5,\n      \"type\": 6\n    },\n    {\n      \"first4\": 31353,\n      \"index\": 1,\n      \"name\": \"1.bin\",\n      \"ram_ptr\": 0,\n      \"size\": 2,\n      \"type\": 0\n    }\n  ],\n  \"version\": 1\n}";
    assert_eq!(archive.manifest_json(true), expected);
    support::remove_temp_root(&root);
}

#[test]
#[should_panic]
fn cleanup_rejects_non_test_directories() {
    support::remove_temp_root(std::path::Path::new("out"));
}
