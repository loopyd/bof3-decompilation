//! The payload inventory must still describe the corpus it claims to describe.
//!
//! Replaces: `tools/python/tests/text/test_payload_inventory.py`.
//!
//! The inventory excludes payloads from text searching, so its two structural checks and its counts are
//! re-derived here from original archive bytes — independently of the crate's own checks, which is what
//! makes this a guard rather than a restatement.

use std::collections::BTreeMap;
use std::path::PathBuf;
use std::sync::Arc;

fn repo_root() -> PathBuf {
    let mut candidate = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
    while candidate.pop() {
        if candidate.join("out").is_dir() && candidate.join("tools/rust/bof3-text").is_dir() {
            return candidate;
        }
    }
    panic!(
        "could not locate the repository root from {}",
        env!("CARGO_MANIFEST_DIR")
    );
}

fn artifact(relative: &str) -> PathBuf {
    let path = repo_root().join(relative);
    assert!(
        path.is_file(),
        "{} is missing; build the text artifacts before running this suite",
        path.display()
    );
    path
}

fn json(relative: &str) -> serde_json::Value {
    serde_json::from_slice(&std::fs::read(artifact(relative)).expect("the artifact is readable"))
        .unwrap_or_else(|error| panic!("{relative} should be JSON: {error}"))
}

fn inventory() -> serde_json::Value {
    json("out/text-payloads/inventory.json")
}

fn map() -> serde_json::Value {
    json("out/text-format/segment-map.json")
}

/// Every subfile the map records, as `(archive path, offset, size, bytes)`.
fn subfiles() -> Vec<(PathBuf, usize, usize, Arc<Vec<u8>>)> {
    let corpus = repo_root().join("out").join("extracted").join("BIN");
    let mut cache: BTreeMap<String, Arc<Vec<u8>>> = BTreeMap::new();
    let mut found = Vec::new();
    for archive in map()["archives"].as_array().cloned().unwrap_or_default() {
        let name = archive["archive"].as_str().unwrap_or_default().to_string();
        let data = cache.entry(name.clone()).or_insert_with(|| {
            let path = corpus.join(&name);
            Arc::new(
                std::fs::read(&path).unwrap_or_else(|error| panic!("{}: {error}", path.display())),
            )
        });
        for subfile in archive["subfiles"].as_array().cloned().unwrap_or_default() {
            let offset = subfile["offset"].as_u64().unwrap_or(0) as usize;
            let size = subfile["size"].as_u64().unwrap_or(0) as usize;
            found.push((corpus.join(&name), offset, size, Arc::clone(data)));
        }
    }
    assert!(
        found.len() > 1_000,
        "the map should record the corpus subfiles"
    );
    found
}

/// The VAB header arithmetic, re-derived: `numProg` is a u16 LE at +0x12 and the header size is exact.
fn vab_header_size(data: &[u8], offset: usize, size: usize) -> bool {
    if data.get(offset..offset + 4) != Some(b"pBAV") || size < 0x20 {
        return false;
    }
    let programs = u16::from_le_bytes([data[offset + 0x12], data[offset + 0x13]]) as usize;
    0x20 + 128 * 16 + programs * 16 * 32 + 512 == size
}

/// The SEQ event chain, re-derived: track 0's 19-byte header and up to three 13-byte headers must consume
/// the subfile exactly.
fn seq_chain_consumes(data: &[u8], offset: usize, size: usize) -> bool {
    if data.get(offset..offset + 4) != Some(b"pQES") {
        return false;
    }
    let end = offset + size;
    let mut cursor = offset
        + 19
        + u32::from_be_bytes([
            data[offset + 15],
            data[offset + 16],
            data[offset + 17],
            data[offset + 18],
        ]) as usize;
    let mut tracks = 1;
    while cursor < end {
        if tracks >= 4 || cursor + 13 > end {
            return false;
        }
        cursor += 13
            + u32::from_be_bytes([
                data[cursor + 9],
                data[cursor + 10],
                data[cursor + 11],
                data[cursor + 12],
            ]) as usize;
        tracks += 1;
    }
    cursor == end
}

#[test]
fn the_inventory_parses_and_names_a_record_for_every_entry() {
    let inventory = inventory();
    assert_eq!(inventory["schema"], "bof3.payload-inventory/v1");
    let exclusions = inventory["exclusions"]
        .as_array()
        .cloned()
        .unwrap_or_default();
    assert!(
        !exclusions.is_empty(),
        "the inventory must name the audio and music families"
    );
    for entry in exclusions {
        let records = entry["records"].as_array().cloned().unwrap_or_default();
        assert!(
            !records.is_empty(),
            "{} is excluded without a named record",
            entry["magic"]
        );
        for record in records {
            let name = record.as_str().unwrap_or_default().to_string();
            let (path, _, line) = match name.split_once(':') {
                Some((path, line)) => (path.to_string(), (), line.to_string()),
                None => (name.clone(), (), String::new()),
            };
            assert!(repo_root().join(&path).is_file(), "{name} does not exist");
            if !line.is_empty() {
                let text = std::fs::read_to_string(repo_root().join(&path)).expect("readable");
                let count = text.lines().count();
                let quoted: usize = line.parse().unwrap_or(0);
                assert!(quoted > 0 && quoted <= count, "{name} is out of range");
            }
        }
        let kind = entry["check"]["kind"].as_str().unwrap_or_default();
        assert!(
            matches!(kind, "vab_header_size" | "seq_event_chain"),
            "{} has no implemented check ({kind})",
            entry["magic"]
        );
    }
}

#[test]
fn every_exclusion_still_holds_across_the_corpus() {
    let inventory = inventory();
    let corpus = subfiles();
    for entry in inventory["exclusions"]
        .as_array()
        .cloned()
        .unwrap_or_default()
    {
        let magic = entry["magic"].as_str().unwrap_or_default().to_string();
        let head = entry["magic"]
            .as_str()
            .unwrap_or_default()
            .as_bytes()
            .to_vec();
        let check = match entry["check"]["kind"].as_str().unwrap_or_default() {
            "vab_header_size" => vab_header_size as fn(&[u8], usize, usize) -> bool,
            _ => seq_chain_consumes,
        };
        let mut found = 0usize;
        let mut passed = 0usize;
        for (_, offset, size, data) in &corpus {
            if data.get(*offset..offset + head.len()) != Some(head.as_slice()) {
                continue;
            }
            found += 1;
            if check(data.as_slice(), *offset, *size) {
                passed += 1;
            }
        }
        let measured = &entry["measured"];
        assert_eq!(
            found as u64,
            measured["payloads"].as_u64().unwrap_or(u64::MAX),
            "{magic}: found {found}"
        );
        assert_eq!(
            passed as u64,
            measured["passed"].as_u64().unwrap_or(u64::MAX),
            "{magic}: {passed}/{found} pass the check"
        );
    }
}

#[test]
fn the_inventory_reconciles_with_the_segment_map() {
    let inventory = inventory();
    let totals = &map()["totals"];
    for entry in inventory["exclusions"]
        .as_array()
        .cloned()
        .unwrap_or_default()
    {
        let magic = entry["magic"].as_str().unwrap_or_default();
        let counted = totals
            .get(format!("magic:{magic}"))
            .and_then(serde_json::Value::as_u64)
            .unwrap_or(0);
        assert_eq!(
            counted,
            entry["measured"]["payloads"].as_u64().unwrap_or(u64::MAX),
            "the map counts {counted} of {magic}"
        );
    }
}

#[test]
fn no_third_repeated_ascii_signature_has_appeared() {
    // Two families are identified; a third repeated ASCII-looking signature would mean the census in the
    // evidence is stale and that something may be excludable but is not yet recorded.
    let mut seen: BTreeMap<Vec<u8>, usize> = BTreeMap::new();
    for (_, offset, _, data) in subfiles() {
        let head = &data[offset..offset + 4];
        if head.iter().all(|byte| (0x20..0x7f).contains(byte)) {
            *seen.entry(head.to_vec()).or_default() += 1;
        }
    }
    let repeated: BTreeMap<Vec<u8>, usize> =
        seen.into_iter().filter(|(_, count)| *count >= 3).collect();
    let mut expected: BTreeMap<Vec<u8>, usize> = BTreeMap::new();
    expected.insert(b"pBAV".to_vec(), 1_020);
    expected.insert(b"pQES".to_vec(), 119);
    assert_eq!(repeated, expected, "the ASCII signature census changed");
}
