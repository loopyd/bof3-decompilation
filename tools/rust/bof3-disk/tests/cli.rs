#[allow(dead_code)]
mod support;

use std::fs;
use std::process::Command;

#[test]
fn extract_discovers_single_image_and_uses_v1_default_paths() {
    let root = support::temp_root("cli-default-extract");
    let disk = root.join("disk");
    fs::create_dir_all(&disk).unwrap();
    fs::write(disk.join("game.iso"), support::fixture(false, false)).unwrap();

    let output = Command::new(env!("CARGO_BIN_EXE_bof3-disk"))
        .arg("extract")
        .current_dir(&root)
        .output()
        .unwrap();
    assert!(
        output.status.success(),
        "stderr: {}",
        String::from_utf8_lossy(&output.stderr)
    );
    assert_eq!(
        fs::read(root.join("build/extracted/HELLO.TXT")).unwrap(),
        b"hello"
    );
    assert!(root.join("build/game.xml").is_file());

    support::remove_temp_root(&root);
}

#[test]
fn rebuild_writes_the_documented_synthetic_iso() {
    let root = support::temp_root("cli-rebuild");
    let input = root.join("input");
    fs::create_dir_all(&input).unwrap();
    fs::write(input.join("HELLO.TXT"), b"hello").unwrap();
    let output = root.join("synthetic.iso");

    let result = Command::new(env!("CARGO_BIN_EXE_bof3-disk"))
        .args(["rebuild", "-i"])
        .arg(&input)
        .args(["-o"])
        .arg(&output)
        .output()
        .unwrap();

    assert!(
        result.status.success(),
        "stderr: {}",
        String::from_utf8_lossy(&result.stderr)
    );
    assert_eq!(
        bof3_disk_v2::Image::open(&output)
            .unwrap()
            .entries()
            .unwrap()
            .into_iter()
            .filter(|entry| !entry.is_directory)
            .map(|entry| entry.path)
            .collect::<Vec<_>>(),
        vec![std::path::PathBuf::from("HELLO.TXT")]
    );
    support::remove_temp_root(&root);
}

#[test]
fn extract_rejects_multiple_images_at_the_highest_priority_extension() {
    let root = support::temp_root("cli-multiple-images");
    let disk = root.join("disk");
    fs::create_dir_all(&disk).unwrap();
    fs::write(disk.join("a.iso"), []).unwrap();
    fs::write(disk.join("b.ISO"), []).unwrap();
    fs::write(disk.join("fallback.bin"), []).unwrap();

    let output = Command::new(env!("CARGO_BIN_EXE_bof3-disk"))
        .arg("extract")
        .current_dir(&root)
        .output()
        .unwrap();
    assert!(!output.status.success());
    assert!(String::from_utf8_lossy(&output.stderr).contains("multiple ISO images found"));

    support::remove_temp_root(&root);
}
