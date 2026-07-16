use std::fs;
use std::path::{Path, PathBuf};
use std::time::{SystemTime, UNIX_EPOCH};

pub fn fixture() -> Vec<u8> {
    let mut bytes = Vec::new();
    bytes.extend_from_slice(&2_u32.to_le_bytes());
    bytes.extend_from_slice(&1_u32.to_le_bytes());
    bytes.extend_from_slice(b"MATH_TBL");
    for (data, file_type, ram_ptr) in [
        (&b"ABCDx"[..], 6_u16, 0x8010_0000_u32),
        (&b"yz"[..], 0_u16, 0),
    ] {
        bytes.extend_from_slice(&(data.len() as u32).to_le_bytes());
        bytes.extend_from_slice(&ram_ptr.to_le_bytes());
        let mut first = [0; 4];
        first[..data.len().min(4)].copy_from_slice(&data[..data.len().min(4)]);
        bytes.extend_from_slice(&first);
        bytes.extend_from_slice(&file_type.to_le_bytes());
        bytes.extend_from_slice(&0_u16.to_le_bytes());
    }
    bytes.resize(0x800, 0);
    bytes.extend_from_slice(b"ABCDx");
    bytes.resize(0x1000, 0);
    bytes.extend_from_slice(b"yz");
    bytes.resize(0x1800, 0);
    bytes
}

pub fn temp_root(label: &str) -> PathBuf {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .expect("system clock must follow Unix epoch")
        .as_nanos();
    std::env::temp_dir().join(format!("emi-ex-v2-{label}-{unique}"))
}

pub fn remove_temp_root(path: &Path) {
    let temp = std::env::temp_dir();
    let name = path.file_name().and_then(|name| name.to_str());
    assert_eq!(path.parent(), Some(temp.as_path()));
    assert!(name.is_some_and(|name| name.starts_with("emi-ex-v2-")));
    fs::remove_dir_all(path).unwrap();
}
