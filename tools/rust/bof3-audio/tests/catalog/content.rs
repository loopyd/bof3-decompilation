use bof3_audio::{
    catalog::loader, catalog::model::AssetData, catalog::model::Catalog,
    machine::executable::Executable,
};
use std::{
    fs,
    path::PathBuf,
    sync::atomic::{AtomicU64, Ordering},
};

struct Directory(PathBuf);
impl Directory {
    fn new() -> Self {
        static NEXT: AtomicU64 = AtomicU64::new(0);
        let path = std::env::temp_dir().join(format!(
            "bof3-bank-content-{}-{}",
            std::process::id(),
            NEXT.fetch_add(1, Ordering::Relaxed)
        ));
        fs::create_dir(&path).unwrap();
        Self(path)
    }
    fn write(&self, name: &str, bytes: &[u8]) {
        fs::write(self.0.join(name), bytes).unwrap();
    }
}
impl Drop for Directory {
    fn drop(&mut self) {
        fs::remove_dir_all(&self.0).unwrap();
    }
}

fn archive(slot: u32, volume: u8, sample_units: u16, bodies: &[Vec<u8>], padding: u8) -> Vec<u8> {
    let mut vh = vec![0; 0xa20];
    vh[..4].copy_from_slice(b"pBAV");
    vh[4..8].copy_from_slice(&7u32.to_le_bytes());
    vh[0x16..0x18].copy_from_slice(&1u16.to_le_bytes());
    vh[0x18] = volume;
    vh[0x822..0x824].copy_from_slice(&sample_units.to_le_bytes());
    let mut entries = vec![(6u16, vh)];
    entries.extend(bodies.iter().cloned().map(|body| (7u16, body)));
    let mut emi = vec![0; 2048];
    emi[..4].copy_from_slice(&(entries.len() as u32).to_le_bytes());
    emi[8..16].copy_from_slice(b"MATH_TBL");
    for (index, (kind, data)) in entries.into_iter().enumerate() {
        let toc = 16 + index * 16;
        emi[toc..toc + 4].copy_from_slice(&(data.len() as u32).to_le_bytes());
        emi[toc + 4..toc + 8].copy_from_slice(&slot.to_le_bytes());
        emi[toc + 12..toc + 14].copy_from_slice(&kind.to_le_bytes());
        emi.extend(data);
        emi.resize(emi.len().div_ceil(2048) * 2048, padding);
    }
    emi
}

#[test]
fn body_hash_covers_declared_payload_including_unused_bytes_but_not_archive_padding() {
    let directory = Directory::new();
    directory.write("a.EMI", &archive(0, 127, 2, &[vec![0; 32]], 0xa5));
    directory.write("padding.EMI", &archive(0, 127, 2, &[vec![0; 32]], 0x5a));
    let mut body = vec![0; 32];
    body[31] = 1; // Outside the declared sample, still part of the preserved VB payload.
    directory.write("changed.EMI", &archive(0, 127, 2, &[body], 0xa5));
    let catalog = Catalog::read(Some(&directory.0), &[]).unwrap();
    let hash = |name: &str| {
        catalog
            .sources
            .iter()
            .find(|s| s.source == name)
            .unwrap()
            .entries[1]
            .body_sha256
            .as_deref()
            .unwrap()
    };
    assert_eq!(hash("a.EMI"), hash("padding.EMI"));
    assert_ne!(hash("a.EMI"), hash("changed.EMI"));
    for asset in &catalog.assets {
        if let AssetData::Bank { content, .. } = &asset.data {
            assert!(
                content.is_none(),
                "inventory alone must not resolve loader associations"
            );
        }
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE original US loader profile"]
fn shared_payloads_keep_distinct_game_ids_and_reject_ambiguous_or_short_bodies() {
    let directory = Directory::new();
    for (name, slot, volume, units, bodies, padding) in [
        ("a.EMI", 0, 127, 2, vec![vec![0; 16]], 0xa5),
        ("same.EMI", 1, 127, 2, vec![vec![0; 16]], 0xa5),
        ("padding.EMI", 0, 127, 2, vec![vec![0; 16]], 0x5a),
        ("body.EMI", 0, 127, 2, vec![vec![1; 16]], 0xa5),
        ("header.EMI", 0, 126, 2, vec![vec![0; 16]], 0xa5),
        ("short.EMI", 0, 127, 4, vec![vec![0; 16]], 0xa5),
        ("missing.EMI", 0, 127, 2, vec![], 0xa5),
        (
            "multiple.EMI",
            0,
            127,
            2,
            vec![vec![0; 16], vec![0; 16]],
            0xa5,
        ),
    ] {
        directory.write(name, &archive(slot, volume, units, &bodies, padding));
    }
    let exe =
        Executable::from_bytes(fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    let mut catalog = Catalog::read(Some(&directory.0), &[]).unwrap();
    let report = loader::resolve(&mut catalog, &exe).unwrap();
    assert_eq!(report.shared_banks.len(), 1);
    assert_eq!(
        report.shared_banks[0].banks,
        ["a.EMI#entry=0", "padding.EMI#entry=0", "same.EMI#entry=0"]
    );
    let a = catalog
        .select("audio", Some("bank"), "a.EMI#entry=0")
        .unwrap();
    let same = catalog
        .select("audio", Some("bank"), "same.EMI#entry=0")
        .unwrap();
    assert_eq!(a.game_bank_id, Some(0));
    assert_eq!(same.game_bank_id, Some(1));
    assert!(catalog.select("audio", Some("bank"), "0").is_err());
    for source in ["short.EMI", "missing.EMI", "multiple.EMI"] {
        let asset = catalog
            .select("audio", Some("bank"), &format!("{source}#entry=0"))
            .unwrap();
        assert!(matches!(&asset.data, AssetData::Bank { content: None, .. }));
        assert!(report
            .unresolved
            .iter()
            .any(|message| message.starts_with(source)));
    }
    assert!(report
        .unresolved
        .iter()
        .any(|message| message.contains("require 32 bytes") && message.contains("contains 16")));
    // Re-resolving an edited body must clear its prior content identity and grouping.
    directory.write("same.EMI", &archive(1, 127, 2, &[vec![1; 16]], 0xa5));
    let mut changed = Catalog::read(Some(&directory.0), &[]).unwrap();
    let changed_report = loader::resolve(&mut changed, &exe).unwrap();
    let group = changed_report
        .shared_banks
        .iter()
        .find(|g| g.banks.iter().any(|id| id == "same.EMI#entry=0"))
        .unwrap();
    assert_eq!(group.banks, ["body.EMI#entry=0", "same.EMI#entry=0"]);
    loader::resolve(&mut changed, &exe).unwrap();
    for asset in changed.assets {
        if let AssetData::Bank {
            content: Some(content),
            ..
        } = asset.data
        {
            assert_eq!(content.declared_sample_bytes, 16);
            assert_eq!(content.body_bytes, 16);
        }
    }
}
