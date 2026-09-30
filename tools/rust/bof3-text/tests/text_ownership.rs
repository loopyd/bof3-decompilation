//! Corpus-backed text behaviour, ported from the Python suite.
//!
//! These drive the built binary exactly as the Python tests did, so the behaviour is asserted in the
//! crate's own suite and the crate stays the single owner of the tooling. A missing corpus or artifact is
//! an **actionable failure**, never a silent skip.
//!
//! Replaces: `tools/python/tests/text/test_payload_ownership.py` and
//! `tools/python/tests/text/test_payload_removals.py`.

use std::path::PathBuf;
use std::process::Command;

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

fn binary() -> PathBuf {
    PathBuf::from(env!("CARGO_BIN_EXE_bof3-text"))
}

fn corpus() -> PathBuf {
    let path = repo_root().join("out").join("extracted").join("BIN");
    assert!(
        path.is_dir(),
        "the extracted corpus {} is missing; extract the archive tree first",
        path.display()
    );
    path
}

fn inventory() -> PathBuf {
    let path = repo_root()
        .join("out")
        .join("text-payloads")
        .join("inventory.json");
    assert!(
        path.is_file(),
        "the payload inventory {} is missing; build it with `bin/harness text payloads`",
        path.display()
    );
    path
}

fn output(args: &[&str]) -> std::process::Output {
    Command::new(binary())
        .args(args)
        .current_dir(repo_root())
        .output()
        .expect("the built binary should run")
}

fn json(args: &[&str]) -> serde_json::Value {
    let result = output(args);
    assert!(
        result.status.success(),
        "{} failed: {}",
        args.join(" "),
        String::from_utf8_lossy(&result.stderr).trim()
    );
    serde_json::from_slice(&result.stdout).expect("the command should print JSON")
}

fn rows(args: &[&str]) -> Vec<serde_json::Value> {
    json(args)["results"]
        .as_array()
        .cloned()
        .unwrap_or_default()
}

/// Latches in one archive, with the window check off so only the payload policy decides.
fn latches(archive: &str, extra: &[&str]) -> usize {
    let path = corpus().join(archive);
    let mut args: Vec<String> = vec![
        "scan".into(),
        path.to_string_lossy().into_owned(),
        "--json".into(),
        "--no-windows".into(),
    ];
    args.extend(extra.iter().map(|value| (*value).to_string()));
    let borrowed: Vec<&str> = args.iter().map(String::as_str).collect();
    json(&borrowed)["results"]
        .as_array()
        .map(|records| {
            records
                .iter()
                .map(|record| record["latches"].as_array().map_or(0, Vec::len))
                .sum()
        })
        .unwrap_or(0)
}

#[test]
fn the_payloads_command_knows_nothing_without_the_inventory() {
    // identification is the inventory's decision, so switching it off reports nothing rather than
    // discovering payloads of its own.
    let root = corpus().to_string_lossy().into_owned();
    let none = rows(&["payloads", "--root", &root, "--no-payloads"]);
    assert!(
        none.is_empty(),
        "{} rows reported without the inventory",
        none.len()
    );
    let all = rows(&["payloads", "--root", &root]);
    assert!(all.len() > 1_000, "only {} payloads reported", all.len());
}

#[test]
fn every_identified_row_names_its_record() {
    let root = corpus().to_string_lossy().into_owned();
    let all = rows(&["payloads", "--root", &root]);
    let unnamed: Vec<&serde_json::Value> = all
        .iter()
        .filter(|row| row["records"].as_array().is_none_or(Vec::is_empty))
        .collect();
    assert!(unnamed.is_empty(), "{} rows name no record", unnamed.len());
    for row in &all {
        assert!(!row["family"].as_str().unwrap_or_default().is_empty());
        let named = !row["magic"].as_str().unwrap_or_default().is_empty()
            || !row["check"].as_str().unwrap_or_default().is_empty();
        assert!(named, "a row names neither a magic nor a check: {row}");
    }
}

#[test]
fn removing_the_pairing_entry_removes_it_from_both_paths() {
    // The inventory is the single source, so deleting the pairing entry must disable the exclusion in the
    // `payloads` report **and** in the PreFilter, not just one of them.
    let root = repo_root();
    let source = inventory();
    let mut document: serde_json::Value =
        serde_json::from_slice(&std::fs::read(&source).expect("the inventory should be readable"))
            .expect("the inventory should be JSON");
    document["identified_without_signature"] = serde_json::json!([]);
    let stripped = root
        .join("out")
        .join("text-payloads")
        .join("inventory.withheld.json");
    std::fs::write(
        &stripped,
        serde_json::to_vec_pretty(&document).expect("serialisable"),
    )
    .expect("the stripped copy should be writable");

    let corpus_path = corpus().to_string_lossy().into_owned();
    let stripped_path = stripped.to_string_lossy().into_owned();
    let bodies = |path: &str| -> usize {
        rows(&["payloads", "--root", &corpus_path, "--payloads", path])
            .into_iter()
            .filter(|row| row["check"].as_str() == Some("vab_body_pairing"))
            .count()
    };
    assert_eq!(
        bodies(&stripped_path),
        0,
        "the pairing entry was not removed"
    );
    let all = bodies(&source.to_string_lossy());
    assert!(all > 0, "the inventory identifies no paired bodies");

    // and the PreFilter stops barring the body: with the registry windows off and the readability rule
    // off, only the inventory decides what may be latched, so the body's runs appear again.
    let archive = "BATTLE/BATL_RET.EMI";
    let with_inventory = latches(archive, &["--no-readable"]);
    let without = latches(archive, &["--payloads", &stripped_path, "--no-readable"]);
    assert!(
        without > with_inventory,
        "removing the pairing entry must stop the PreFilter barring the body: {with_inventory} with it, {without} without"
    );
    let _ = std::fs::remove_file(&stripped);
}

// NOTE: `test_payload_removals.py`'s reproducibility measurement is not ported yet. Its 880-archive
// sweep needs a shared-families (Arc) rework to run in parallel, and a serial version takes minutes; the
// Python guard still covers the behaviour, so the crate keeps the three controls it can prove.
