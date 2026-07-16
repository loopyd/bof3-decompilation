use std::fs;
use std::path::{Path, PathBuf};
use std::time::{SystemTime, UNIX_EPOCH};

const ISO_SECTOR_SIZE: usize = 2048;
const RAW_SECTOR_SIZE: usize = 2352;
const RAW_USER_DATA_OFFSET: usize = 24;
const RAW_XA_SIZE: usize = 2336;

fn record(extent: u32, size: u32, flags: u8, name: &[u8]) -> Vec<u8> {
    let length = 33 + name.len() + usize::from(name.len().is_multiple_of(2));
    let mut bytes = vec![0; length];
    bytes[0] = length as u8;
    bytes[2..6].copy_from_slice(&extent.to_le_bytes());
    bytes[6..10].copy_from_slice(&extent.to_be_bytes());
    bytes[10..14].copy_from_slice(&size.to_le_bytes());
    bytes[14..18].copy_from_slice(&size.to_be_bytes());
    bytes[25] = flags;
    bytes[28..30].copy_from_slice(&1_u16.to_le_bytes());
    bytes[30..32].copy_from_slice(&1_u16.to_be_bytes());
    bytes[32] = name.len() as u8;
    bytes[33..33 + name.len()].copy_from_slice(name);
    bytes
}

fn xa_record(extent: u32, size: u32, name: &[u8]) -> Vec<u8> {
    let mut bytes = record(extent, size, 0, name);
    bytes.resize(bytes.len() + 14, 0);
    bytes[0] = bytes.len() as u8;
    let system_use = 33 + name.len() + usize::from(name.len().is_multiple_of(2));
    bytes[system_use..system_use + 2].copy_from_slice(&0x1234_u16.to_be_bytes());
    bytes[system_use + 2..system_use + 4].copy_from_slice(&0x5678_u16.to_be_bytes());
    bytes[system_use + 4..system_use + 6].copy_from_slice(&0x1055_u16.to_be_bytes());
    bytes[system_use + 6..system_use + 8].copy_from_slice(b"XA");
    bytes[system_use + 8] = 1;
    bytes
}

pub fn fixture(raw: bool, corrupt_both_endian: bool) -> Vec<u8> {
    let mut sectors = vec![[0_u8; ISO_SECTOR_SIZE]; 24];
    sectors[16][0] = 1;
    sectors[16][1..6].copy_from_slice(b"CD001");
    sectors[16][6] = 1;
    let root = record(20, 2048, 2, &[0]);
    sectors[16][156..156 + root.len()].copy_from_slice(&root);
    let mut file = record(21, 5, 0, b"HELLO.TXT;1");
    if corrupt_both_endian {
        file[6] ^= 1;
    }
    let records = [
        record(20, 2048, 2, &[0]),
        record(20, 2048, 2, &[1]),
        file,
        record(22, 2048, 2, b"DIR"),
    ];
    let mut offset = 0;
    for value in records {
        sectors[20][offset..offset + value.len()].copy_from_slice(&value);
        offset += value.len();
    }
    let nested = [
        record(22, 2048, 2, &[0]),
        record(20, 2048, 2, &[1]),
        record(23, 3, 0, b"DATA.BIN;1"),
    ];
    offset = 0;
    for value in nested {
        sectors[22][offset..offset + value.len()].copy_from_slice(&value);
        offset += value.len();
    }
    sectors[21][..5].copy_from_slice(b"hello");
    sectors[23][..3].copy_from_slice(&[0, 1, 255]);
    if !raw {
        return sectors.into_iter().flatten().collect();
    }
    let mut output = vec![0; sectors.len() * RAW_SECTOR_SIZE];
    for (index, sector) in sectors.iter().enumerate() {
        let start = index * RAW_SECTOR_SIZE + RAW_USER_DATA_OFFSET;
        output[start..start + ISO_SECTOR_SIZE].copy_from_slice(sector);
    }
    output
}

pub fn xa_fixture() -> Vec<u8> {
    let mut bytes = fixture(true, false);
    let directory = 20 * RAW_SECTOR_SIZE + RAW_USER_DATA_OFFSET;
    bytes[directory..directory + ISO_SECTOR_SIZE].fill(0);
    let records = [
        record(20, 2048, 2, &[0]),
        record(20, 2048, 2, &[1]),
        xa_record(24, (RAW_XA_SIZE * 2) as u32, b"STREAM.XA;1"),
        record(22, 2048, 2, b"DIR"),
    ];
    let mut offset = directory;
    for record in records {
        bytes[offset..offset + record.len()].copy_from_slice(&record);
        offset += record.len();
    }
    bytes.resize(26 * RAW_SECTOR_SIZE, 0);
    for sector_index in 0..2 {
        let sector = (24 + sector_index) * RAW_SECTOR_SIZE;
        for (index, byte) in bytes[sector + 16..sector + 16 + RAW_XA_SIZE]
            .iter_mut()
            .enumerate()
        {
            *byte = ((sector_index * RAW_XA_SIZE + index) % 251) as u8;
        }
    }
    bytes
}

pub fn temp_root(label: &str) -> PathBuf {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .expect("system clock must follow Unix epoch")
        .as_nanos();
    std::env::temp_dir().join(format!("bof3-disk-v2-{label}-{unique}"))
}

pub fn remove_temp_root(path: &Path) {
    let temp = std::env::temp_dir();
    let name = path.file_name().and_then(|name| name.to_str());
    assert_eq!(path.parent(), Some(temp.as_path()));
    assert!(name.is_some_and(|name| name.starts_with("bof3-disk-v2-")));
    fs::remove_dir_all(path).unwrap();
}
