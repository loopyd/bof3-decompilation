mod support;

use emi_ex_v2::image::ArchiveImage;

fn opaque_fixture() -> Vec<u8> {
    let mut bytes = support::fixture();
    bytes[0x1e..0x20].copy_from_slice(&0x2e2e_u16.to_le_bytes());
    bytes[0x18..0x1c].copy_from_slice(b"OLD!");
    bytes[0x30..0x800].fill(0xa5);
    bytes[0x805..0x1000].fill(0xc3);
    bytes[0x1002..0x1800].fill(0x3c);
    bytes.extend_from_slice(b"opaque trailer\0\xff");
    bytes
}

#[test]
fn preserves_opaque_regions_and_stale_cached_word() {
    let bytes = opaque_fixture();
    let image = ArchiveImage::from_bytes(bytes.clone()).unwrap();
    assert_eq!(image.entries()[0].table_padding, 0x2e2e);
    assert_eq!(image.replace_entries(&[]).unwrap().bytes(), bytes);
    assert_eq!(
        image
            .replace_entries(&[(0, b"ABCDx"), (1, b"yz")])
            .unwrap()
            .bytes(),
        bytes
    );
}

#[test]
fn edits_only_payload_size_and_cached_word() {
    let bytes = opaque_fixture();
    let image = ArchiveImage::from_bytes(bytes.clone()).unwrap();
    let result = image.replace_entries(&[(0, b"new payload")]).unwrap();
    let mut expected = bytes.clone();
    expected[0x10..0x14].copy_from_slice(&11_u32.to_le_bytes());
    expected[0x18..0x1c].copy_from_slice(b"new ");
    expected[0x800..0x80b].copy_from_slice(b"new payload");
    assert_eq!(result.bytes(), expected);
    assert_eq!(image.bytes(), bytes);
    assert_eq!(result.entry(1).unwrap(), b"yz");
}

#[test]
fn rejects_invalid_conflicting_and_relocating_edits_without_mutation() {
    let bytes = opaque_fixture();
    let image = ArchiveImage::from_bytes(bytes.clone()).unwrap();
    assert!(image.replace_entries(&[(2, b"x")]).is_err());
    assert!(image.replace_entries(&[(0, b"x"), (0, b"y")]).is_err());
    assert!(image.replace_entries(&[(0, &vec![0; 0x801])]).is_err());
    assert!(image.replace_entries(&[(0, b"")]).is_err());
    assert_eq!(image.bytes(), bytes);
}

#[test]
fn rejects_every_truncation_before_last_payload_end() {
    let bytes = opaque_fixture();
    for length in 0..0x1002 {
        assert!(
            ArchiveImage::from_bytes(bytes[..length].to_vec()).is_err(),
            "length {length}"
        );
    }
    assert!(ArchiveImage::from_bytes(bytes[..0x1002].to_vec()).is_ok());
}

#[test]
fn bounds_entry_count_before_allocating() {
    let mut bytes = opaque_fixture();
    bytes[..4].copy_from_slice(&u32::MAX.to_le_bytes());
    assert!(ArchiveImage::from_bytes(bytes).is_err());
}

#[test]
fn file_and_snapshot_parsers_agree_and_snapshot_survives_source_edits() {
    let root = support::temp_root("snapshot");
    std::fs::create_dir_all(&root).unwrap();
    let path = root.join("source.emi");
    let bytes = opaque_fixture();
    std::fs::write(&path, &bytes).unwrap();
    let archive = emi_ex_v2::Archive::open(&path).unwrap();
    let image = ArchiveImage::from_bytes(std::fs::read(&path).unwrap()).unwrap();
    assert_eq!(archive.entries(), image.entries());
    assert_eq!(archive.version(), image.version());
    std::fs::write(&path, b"changed source").unwrap();
    assert_eq!(image.bytes(), bytes);
    support::remove_temp_root(&root);
}
