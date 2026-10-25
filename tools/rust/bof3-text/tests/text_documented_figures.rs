//! Reconcile retained text artifacts and live scans against the US corpus census.

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

fn json(relative: &str) -> serde_json::Value {
    let path = repo_root().join(relative);
    let bytes = std::fs::read(&path)
        .unwrap_or_else(|error| panic!("{} should be readable: {error}", path.display()));
    serde_json::from_slice(&bytes)
        .unwrap_or_else(|error| panic!("{relative} should be JSON: {error}"))
}

fn counter(totals: &serde_json::Value, name: &str) -> u64 {
    match totals.get(name) {
        None => 0,
        Some(value) => value
            .as_u64()
            .unwrap_or_else(|| panic!("{name} should be an unsigned integer")),
    }
}

#[test]
fn the_aggregate_facts_match_the_artifacts() {
    let index = json("out/text-index/index.json");
    let map = json("out/text-format/segment-map.json");
    let registry = json("out/text-windows/registry.json");
    let length = |value: &serde_json::Value| {
        value
            .as_array()
            .expect("the artifact field should be an array")
            .len()
    };
    assert_eq!(length(&index["instances"]), 46_057);
    assert_eq!(
        counter(&map["totals"], "text_like_runs_in_data_subfiles"),
        0
    );
    assert_eq!(length(&registry["windows"]), 48_571);
    assert_eq!(
        map["totals"]["latches_removed_by_windows"],
        serde_json::json!(0)
    );
    assert_eq!(counter(&index["counts"], "heuristic"), 0);
}

#[test]
fn the_data_subfile_candidates_fit_the_current_census() {
    let map = json("out/text-format/segment-map.json");
    let candidates = counter(&map["totals"], "text_like_runs_in_data_subfiles");
    assert!(
        candidates <= 1,
        "the map's data-subfile candidates cannot exceed the whole census"
    );
}

/// The latch count one archive's scan reports.
fn latches(path: &PathBuf, flags: &[&str]) -> usize {
    let result = Command::new(env!("CARGO_BIN_EXE_bof3-text"))
        .arg("scan")
        .arg(path)
        .arg("--json")
        .args(flags)
        .current_dir(repo_root())
        .output()
        .expect("the built binary should run");
    assert!(
        result.status.success(),
        "{} failed: {}",
        path.display(),
        String::from_utf8_lossy(&result.stderr).trim()
    );
    let document: serde_json::Value =
        serde_json::from_slice(&result.stdout).expect("the scan should print JSON");
    document["results"]
        .as_array()
        .cloned()
        .unwrap_or_default()
        .iter()
        .map(|record| record["latches"].as_array().map_or(0, Vec::len))
        .sum()
}

/// Total latches over every archive, in eight workers.
fn corpus_total(flags: &[&str]) -> usize {
    let root = repo_root();
    let mut archives: Vec<PathBuf> = Vec::new();
    let mut pending = vec![root.join("out/extracted/BIN")];
    while let Some(directory) = pending.pop() {
        for entry in std::fs::read_dir(&directory).expect("the corpus should list") {
            let path = entry.expect("a readable entry").path();
            if path.is_dir() {
                pending.push(path);
            } else if path.extension().is_some_and(|kind| kind == "EMI") {
                archives.push(path);
            }
        }
    }
    archives.sort();
    assert!(archives.len() > 100, "the corpus should be present");
    let mut total = 0usize;
    std::thread::scope(|scope| {
        let handles: Vec<_> = archives
            .chunks(archives.len().div_ceil(8).max(1))
            .map(|slice| {
                scope.spawn(move || slice.iter().map(|path| latches(path, flags)).sum::<usize>())
            })
            .collect();
        for handle in handles {
            total += handle.join().expect("a scan worker failed");
        }
    });
    total
}

#[test]
fn the_unwindowed_corpus_total_is_the_stated_census() {
    let baseline = corpus_total(&["--no-windows", "--no-payloads", "--no-readable"]);
    assert_eq!(
        baseline, 13_196,
        "the baseline corpus holds {baseline} runs"
    );
    let current = corpus_total(&["--no-windows"]);
    assert_eq!(current, 1, "the current pipeline holds {current} runs");
}
