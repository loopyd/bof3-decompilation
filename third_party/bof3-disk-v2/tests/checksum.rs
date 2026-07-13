#[allow(dead_code)]
mod support;

use bof3_disk_v2::checksum;
use std::fs;
use std::time::{SystemTime, UNIX_EPOCH};

#[test]
fn hashes_supported_images_and_deduplicates_identical_content() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let root = std::env::temp_dir().join(format!("bof3-disk-v2-checksum-{unique}"));
    let disk = root.join("disk");
    fs::create_dir_all(&disk).unwrap();
    fs::write(disk.join("a.bin"), b"abc").unwrap();
    fs::write(disk.join("copy.iso"), b"abc").unwrap();
    fs::write(disk.join("ignored.txt"), b"abc").unwrap();

    let rows = checksum::scan(&disk, &root).unwrap();
    assert_eq!(rows.len(), 1);
    assert_eq!(rows[0].path, "disk/a.bin");
    assert_eq!(rows[0].size, 3);
    assert_eq!(rows[0].md5, "900150983cd24fb0d6963f7d28e17f72");
    assert_eq!(
        rows[0].sha256,
        "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"
    );

    let manifest = root.join("checksums.json");
    checksum::write(&manifest, &rows).unwrap();
    let decoded = checksum::read(&manifest).unwrap();
    assert_eq!(decoded, rows);
    assert!(checksum::verify(&disk, &root, &decoded).unwrap().is_empty());

    assert_eq!(root.parent(), Some(std::env::temp_dir().as_path()));
    assert!(root
        .file_name()
        .and_then(|name| name.to_str())
        .is_some_and(|name| name.starts_with("bof3-disk-v2-checksum-")));
    fs::remove_dir_all(root).unwrap();
}
