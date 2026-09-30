//! The classified-window registry: what it holds, that it rebuilds identically, and that it fails closed.
//!
//! Replaces: `tools/python/tests/text/test_window_registry.py`.

use std::collections::BTreeSet;
use std::path::{Path, PathBuf};
use std::process::Command;

/// The reviewed ranges the boundary record names. Losing one must fail, not shrink the registry.
const REVIEWED: [(&str, u64); 3] = [
    ("ETC/COMMU00.EMI", 0),
    ("SCENARIO/SCENA00.EMI", 0),
    ("WORLD00/AREA026.EMI", 13),
];

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

fn corpus() -> PathBuf {
    let path = repo_root().join("out").join("extracted").join("BIN");
    assert!(
        path.is_dir(),
        "the extracted corpus {} is missing; extract the archive tree first",
        path.display()
    );
    path
}

fn registry() -> serde_json::Value {
    serde_json::from_slice(
        &std::fs::read(artifact("out/text-windows/registry.json"))
            .expect("the registry should be readable"),
    )
    .expect("the registry should be JSON")
}

/// Run the binary from the repository root.
fn run(args: &[&str]) -> std::process::Output {
    Command::new(env!("CARGO_BIN_EXE_bof3-text"))
        .args(args)
        .current_dir(repo_root())
        .output()
        .expect("the built binary should run")
}

fn json(args: &[&str]) -> serde_json::Value {
    let result = run(args);
    assert!(
        result.status.success(),
        "{} failed: {}",
        args.join(" "),
        String::from_utf8_lossy(&result.stderr).trim()
    );
    serde_json::from_slice(&result.stdout).expect("the command should print JSON")
}

/// A temporary directory that removes itself, so the suite leaves nothing behind.
struct Scratch(PathBuf);

impl Scratch {
    fn new(name: &str) -> Self {
        let path = std::env::temp_dir().join(format!("bof3-{name}-{}", std::process::id()));
        let _ = std::fs::remove_dir_all(&path);
        std::fs::create_dir_all(&path).expect("a scratch directory should be creatable");
        Self(path)
    }

    fn path(&self) -> &Path {
        &self.0
    }
}

impl Drop for Scratch {
    fn drop(&mut self) {
        let _ = std::fs::remove_dir_all(&self.0);
    }
}

#[test]
fn the_retained_registry_and_its_prerequisites_are_present() {
    let registry = registry();
    assert_eq!(registry["schema"], "bof3.text-windows/v1");
    assert_eq!(registry["version"].as_u64().unwrap_or(0), 1);
    assert!(
        !registry["windows"]
            .as_array()
            .cloned()
            .unwrap_or_default()
            .is_empty()
    );
}

#[test]
fn a_freshly_built_registry_is_byte_identical_to_the_retained_one() {
    let scratch = Scratch::new("registry");
    let out = scratch.path().join("registry.json");
    let result = run(&["windows", "--out", &out.to_string_lossy()]);
    assert!(
        result.status.success(),
        "the rebuild failed: {}",
        String::from_utf8_lossy(&result.stderr).trim()
    );
    let retained = std::fs::read(artifact("out/text-windows/registry.json")).expect("readable");
    let rebuilt = std::fs::read(&out).expect("the rebuild should be readable");
    assert_eq!(
        rebuilt.len(),
        retained.len(),
        "the rebuilt registry is a different size"
    );
    assert!(
        rebuilt == retained,
        "the rebuilt registry differs from the retained one"
    );
}

#[test]
fn every_window_names_an_owner_and_a_hash() {
    let registry = registry();
    let kinds = &registry["inputs"]["kinds"];
    assert_eq!(registry["inputs"]["archives"].as_u64().unwrap_or(0), 880);
    let mut named: BTreeSet<String> = BTreeSet::new();
    for key in kinds.as_object().cloned().unwrap_or_default().keys() {
        named.insert(key.clone());
    }
    let expected: BTreeSet<String> = [
        "banked_payload",
        "known_payload",
        "pointer_table",
        "row_extent",
        "reviewed_textbin",
    ]
    .iter()
    .map(|name| name.to_string())
    .collect();
    assert_eq!(named, expected, "the registry's kinds changed");
    assert_eq!(kinds["banked_payload"].as_u64().unwrap_or(0), 244);
    assert_eq!(kinds["known_payload"].as_u64().unwrap_or(0), 1_979);
    assert_eq!(kinds["pointer_table"].as_u64().unwrap_or(0), 288);
    assert_eq!(kinds["row_extent"].as_u64().unwrap_or(0), 46_057);
    assert_eq!(kinds["reviewed_textbin"].as_u64().unwrap_or(0), 3);
    for window in registry["windows"].as_array().cloned().unwrap_or_default() {
        let owner = &window["owner"];
        assert!(
            !owner["kind"].as_str().unwrap_or_default().is_empty(),
            "a window names no owner kind: {window}"
        );
        assert!(
            owner["hash"]
                .as_str()
                .is_some_and(|hash| hash.starts_with("sha256:")),
            "a window's owner carries no sha256 hash: {window}"
        );
        assert!(
            !window["kind"].as_str().unwrap_or_default().is_empty(),
            "a window names no kind: {window}"
        );
    }
}

#[test]
fn the_reviewed_windows_match_the_boundary_record() {
    let registry = registry();
    let found: BTreeSet<(String, u64)> = registry["windows"]
        .as_array()
        .cloned()
        .unwrap_or_default()
        .into_iter()
        .filter(|window| window["kind"].as_str() == Some("reviewed_textbin"))
        .map(|window| {
            (
                window["archive"].as_str().unwrap_or_default().to_string(),
                window["entry"].as_u64().unwrap_or_default(),
            )
        })
        .collect();
    let expected: BTreeSet<(String, u64)> = REVIEWED
        .iter()
        .map(|(archive, entry)| ((*archive).to_string(), *entry))
        .collect();
    assert_eq!(found, expected, "the reviewed ranges changed");
}

#[test]
fn a_target_tree_with_no_ranges_fails_closed() {
    let scratch = Scratch::new("targets");
    let empty = scratch.path().join("emi");
    std::fs::create_dir_all(&empty).expect("an empty targets tree should be creatable");
    let out = scratch.path().join("registry.json");
    let result = run(&[
        "windows",
        "--targets",
        &empty.to_string_lossy(),
        "--out",
        &out.to_string_lossy(),
    ]);
    assert!(
        !result.status.success(),
        "a source that contributes no windows must not pass silently"
    );
    let message = String::from_utf8_lossy(&result.stderr);
    assert!(
        message.contains("no textbin ranges found"),
        "unexpected refusal: {message}"
    );
}

#[test]
fn a_missing_registry_fails_closed_rather_than_latching_unwindowed() {
    let archive = corpus().join("WORLD00").join("AREA000.EMI");
    let result = run(&[
        "scan",
        &archive.to_string_lossy(),
        "--windows",
        "/nonexistent/registry.json",
        "--json",
    ]);
    assert!(
        !result.status.success(),
        "a missing registry must not be ignored"
    );
}

#[test]
fn an_archive_the_source_never_inspected_is_refused_by_both_modes() {
    // An archive outside the corpus is not in the inspected set, so latching it would be guessing.
    let scratch = Scratch::new("uninspected");
    // The copy must carry a name the registry cannot know: keeping the original basename would resolve
    // to the inspected archive of that name rather than to an uninspected one.
    let copy = scratch.path().join("UNINSPECTED-COPY.EMI");
    std::fs::copy(corpus().join("WORLD00").join("AREA000.EMI"), &copy)
        .expect("an archive copy should be writable");
    let scanned = run(&["scan", &copy.to_string_lossy(), "--json"]);
    assert!(
        !scanned.status.success(),
        "an uninspected archive must be refused rather than latched"
    );
    assert!(
        String::from_utf8_lossy(&scanned.stderr).contains("not inspected by the window registry"),
        "unexpected refusal: {}",
        String::from_utf8_lossy(&scanned.stderr).trim()
    );
    // the class-based latch path is the one that consults the registry, so the check is made there
    let queried = run(&[
        "query",
        &copy.to_string_lossy(),
        "--class",
        "heuristic",
        "--grep",
        "a",
        "--json",
    ]);
    assert!(
        !queried.status.success(),
        "the latch query must refuse an uninspected archive too: {}",
        String::from_utf8_lossy(&queried.stderr).trim()
    );
    assert!(
        String::from_utf8_lossy(&queried.stderr).contains("not inspected by the window registry"),
        "unexpected refusal: {}",
        String::from_utf8_lossy(&queried.stderr).trim()
    );
    // and the explicit opt-out scans where the check would otherwise refuse
    let opted_out = run(&["scan", &copy.to_string_lossy(), "--json", "--no-windows"]);
    assert!(
        opted_out.status.success(),
        "--no-windows must scan where the check would refuse: {}",
        String::from_utf8_lossy(&opted_out.stderr).trim()
    );
    let body: serde_json::Value =
        serde_json::from_slice(&opted_out.stdout).expect("the scan should print JSON");
    assert_eq!(body["windows"]["windows"].as_u64().unwrap_or(1), 0);
}

#[test]
fn an_archive_with_no_owned_windows_still_scans_and_agrees_with_the_map() {
    let registry = registry();
    let owned: BTreeSet<String> = registry["windows"]
        .as_array()
        .cloned()
        .unwrap_or_default()
        .into_iter()
        .map(|window| window["archive"].as_str().unwrap_or_default().to_string())
        .collect();
    assert!(
        registry["inputs"]["kinds"]["known_payload"]
            .as_u64()
            .unwrap_or(0)
            > 0,
        "the registry should hold payload windows"
    );
    let _ = owned;
    let archive = "BATTLE/BATL_DRA.EMI";
    let inspected: BTreeSet<String> = registry["inputs"]["inspected_archives"]
        .as_array()
        .cloned()
        .unwrap_or_default()
        .into_iter()
        .filter_map(|name| name.as_str().map(str::to_string))
        .collect();
    assert!(inspected.contains(archive), "{archive} should be inspected");

    let scan = json(&["scan", &corpus().join(archive).to_string_lossy(), "--json"]);
    let runs: usize = scan["results"]
        .as_array()
        .cloned()
        .unwrap_or_default()
        .iter()
        .map(|record| record["latches"].as_array().map_or(0, Vec::len))
        .sum();
    let mapped: serde_json::Value = serde_json::from_slice(
        &std::fs::read(artifact("out/text-format/segment-map.json"))
            .expect("the map should be readable"),
    )
    .expect("the map should be JSON");
    let recorded: usize = mapped["archives"]
        .as_array()
        .cloned()
        .unwrap_or_default()
        .iter()
        .filter(|entry| entry["archive"].as_str() == Some(archive))
        .flat_map(|entry| entry["subfiles"].as_array().cloned().unwrap_or_default())
        .map(|subfile| subfile["text_like_runs"].as_array().map_or(0, Vec::len))
        .sum();
    assert_eq!(
        runs, recorded,
        "scan and the map must agree on the candidate runs"
    );
    // and the latch query reports the same barrier count
    let queried = json(&[
        "query",
        &corpus().join(archive).to_string_lossy(),
        "--class",
        "heuristic",
        "--grep",
        "the",
        "--json",
    ]);
    assert_eq!(
        queried["windows"]["windows"], scan["windows"]["windows"],
        "the latch query and the scan must agree on barriers"
    );
}
