#[allow(dead_code)]
mod support;

use std::fs;

use emi_ex_v2::{
    glob_match, guess_type, pack, pack_folder, pack_manifest, Archive, PackEntry, PackFolderOptions,
};
use tempfile::tempdir;

#[test]
fn packs_and_roundtrips_entries_byte_exactly() {
    let root = tempdir().unwrap();
    let first = root.path().join("first.vh");
    let second = root.path().join("second.bin");
    fs::write(&first, b"ABCDx").unwrap();
    fs::write(&second, b"yz").unwrap();
    let output = root.path().join("roundtrip.emi");
    pack(
        &output,
        &[
            PackEntry {
                path: first,
                file_type: 6,
                ram_ptr: 0x8010_0000,
            },
            PackEntry {
                path: second,
                file_type: 0,
                ram_ptr: 0,
            },
        ],
        1,
    )
    .unwrap();

    assert_eq!(fs::read(&output).unwrap(), support::fixture());
    let archive = Archive::open(output).unwrap();
    assert_eq!(archive.entries().len(), 2);
}

#[test]
fn repacks_an_extracted_manifest() {
    let root = tempdir().unwrap();
    let source = root.path().join("source.emi");
    fs::write(&source, support::fixture()).unwrap();
    let archive = Archive::open(&source).unwrap();
    let extracted = root.path().join("extracted");
    archive.extract_all(&extracted, true).unwrap();
    let manifest = extracted.join("emi.json");
    fs::write(&manifest, archive.manifest_json(true)).unwrap();
    let repacked = root.path().join("repacked.emi");
    pack_manifest(&repacked, &manifest).unwrap();
    assert_eq!(fs::read(repacked).unwrap(), support::fixture());
}

#[test]
fn type_guessing_matches_v1_extensions() {
    for (name, expected) in [
        ("image.TIM", Some(3)),
        ("header.VH", Some(6)),
        ("body.vb", Some(7)),
        ("music.MID", Some(10)),
        ("data.dat", Some(0)),
        ("README", Some(0)),
        ("unknown.txt", None),
    ] {
        assert_eq!(guess_type(name), expected, "{name}");
    }
}

#[test]
fn v1_globs_are_case_sensitive_and_support_star_and_question() {
    assert!(glob_match("a?c*", "abc.bin"));
    assert!(glob_match("*.vh", "voice.vh"));
    assert!(!glob_match("*.vh", "voice.VH"));
    assert!(!glob_match("a?c", "ac"));
}

#[test]
fn folder_pack_filters_sorts_and_applies_unknown_fallback() {
    let root = tempdir().unwrap();
    let folder = root.path().join("input");
    fs::create_dir(&folder).unwrap();
    fs::write(folder.join("b.foo"), b"second").unwrap();
    fs::write(folder.join("a.vh"), b"first").unwrap();
    fs::write(folder.join("skip.bin"), b"skip").unwrap();
    fs::create_dir(folder.join("nested")).unwrap();
    fs::write(folder.join("nested/ignored.vb"), b"ignored").unwrap();

    let output = root.path().join("folder.EMI");
    pack_folder(
        &output,
        &folder,
        &PackFolderOptions {
            default_type: Some(10),
            include_patterns: vec!["a.*".into(), "b.*".into()],
            exclude_patterns: vec!["skip*".into()],
        },
    )
    .unwrap();

    let archive = Archive::open(output).unwrap();
    assert_eq!(archive.entries().len(), 2);
    assert_eq!(archive.entries()[0].file_type, 6);
    assert_eq!(archive.entries()[1].file_type, 10);
    archive
        .extract_entry(0, root.path().join("first.out"))
        .unwrap();
    archive
        .extract_entry(1, root.path().join("second.out"))
        .unwrap();
    assert_eq!(fs::read(root.path().join("first.out")).unwrap(), b"first");
    assert_eq!(fs::read(root.path().join("second.out")).unwrap(), b"second");
}

#[test]
fn packs_empty_aligned_and_cross_sector_payloads() {
    let root = tempfile::tempdir().unwrap();
    let sizes = [0_usize, 0x800, 0x801];
    let entries: Vec<_> = sizes
        .iter()
        .enumerate()
        .map(|(index, size)| {
            let path = root.path().join(format!("{index}.bin"));
            fs::write(&path, vec![index as u8; *size]).unwrap();
            PackEntry {
                path,
                file_type: 0,
                ram_ptr: 0,
            }
        })
        .collect();
    let output = root.path().join("edges.EMI");
    pack(&output, &entries, 3).unwrap();
    let archive = Archive::open(&output).unwrap();
    assert_eq!(archive.version(), 3);
    assert_eq!(
        archive
            .entries()
            .iter()
            .map(|entry| entry.size)
            .collect::<Vec<_>>(),
        [0, 0x800, 0x801]
    );
    for (index, size) in sizes.iter().enumerate() {
        let path = root.path().join(format!("out-{index}"));
        archive.extract_entry(index, &path).unwrap();
        assert_eq!(fs::metadata(path).unwrap().len(), *size as u64);
    }
}
