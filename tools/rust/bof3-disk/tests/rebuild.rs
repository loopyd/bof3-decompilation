#[allow(dead_code)]
mod support;

use bof3_disk_v2::{rebuild, Image};
use sha2::{Digest, Sha256};
use std::fs;

fn fixture_input(root: &std::path::Path) {
    let fixture = std::path::Path::new(env!("CARGO_MANIFEST_DIR")).join("tests/fixtures/rebuild");
    fs::create_dir_all(root).unwrap();
    for name in ["ALPHA.TXT", "ZETA.BIN"] {
        fs::copy(fixture.join(name), root.join(name)).unwrap();
    }
}

#[test]
fn rebuilds_sorted_top_level_files_to_a_byte_stable_cooked_iso() {
    let root = support::temp_root("rebuild");
    let input = root.join("input");
    fixture_input(&input);
    let first = root.join("first.iso");
    let second = root.join("second.iso");

    rebuild::iso(&input, &first).unwrap();
    rebuild::iso(&input, &second).unwrap();

    let first_bytes = fs::read(&first).unwrap();
    assert_eq!(first_bytes, fs::read(&second).unwrap());
    assert_eq!(first_bytes.len(), 21 * 2_048);
    assert_eq!(
        Sha256::digest(&first_bytes)
            .iter()
            .map(|byte| format!("{byte:02x}"))
            .collect::<String>(),
        "107da5ec42bab15f6e58bcc754223e690ba323dd87005e5ea04faadc6bb61779"
    );

    let mut image = Image::open(&first).unwrap();
    let entries = image.entries().unwrap();
    assert_eq!(
        entries
            .iter()
            .filter(|entry| !entry.is_directory)
            .map(|entry| (entry.path.clone(), entry.lba, entry.size))
            .collect::<Vec<_>>(),
        vec![("ALPHA.TXT".into(), 19, 6), ("ZETA.BIN".into(), 20, 3),]
    );
    Image::open(&first)
        .unwrap()
        .extract(root.join("out"))
        .unwrap();
    assert_eq!(fs::read(root.join("out/ALPHA.TXT")).unwrap(), b"hello\n");
    assert_eq!(fs::read(root.join("out/ZETA.BIN")).unwrap(), [0, 1, 255]);
    support::remove_temp_root(&root);
}

#[test]
fn rebuild_rejects_unsupported_input_shapes() {
    let root = support::temp_root("rebuild-invalid");
    let input = root.join("input");
    fs::create_dir_all(input.join("DIR")).unwrap();
    fs::write(input.join("lower.txt"), b"no").unwrap();
    assert!(rebuild::iso(&input, &root.join("out.iso")).is_err());
    support::remove_temp_root(&root);
}
