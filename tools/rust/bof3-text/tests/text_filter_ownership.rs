//! Filter logic and thresholds must live in exactly one place.
//!
//! Replaces: `tools/python/tests/text/test_filter_ownership.py`.
//!
//! `filters.rs` owns every pre-filter, every post-filter and every threshold; `models.rs` declares the
//! contracts. These checks fail if a threshold is declared or a floor is compared anywhere else, so the
//! consolidation cannot quietly come undone as filters are added.

use std::path::{Path, PathBuf};

const FILTERS: &str = "filters.rs";
const THRESHOLDS: [&str; 7] = [
    "MIN_RUN",
    "GAP_TOLERANCE",
    "MAX_RUN",
    "MIN_RATIO",
    "MIN_ALPHA",
    "MIN_DISTINCT",
    "PREVIEW",
];
const FLOORS: [&str; 3] = ["MIN_RATIO", "MIN_ALPHA", "MIN_DISTINCT"];

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

fn source(name: &str) -> String {
    let path = repo_root().join("tools/rust/bof3-text/src").join(name);
    std::fs::read_to_string(&path)
        .unwrap_or_else(|error| panic!("{} should be readable: {error}", path.display()))
}

/// Every crate source, so a rule can be checked everywhere it might hide.
fn sources() -> Vec<(String, String)> {
    let directory = repo_root().join("tools/rust/bof3-text/src");
    let mut found: Vec<(String, String)> = std::fs::read_dir(&directory)
        .unwrap_or_else(|error| panic!("{} should list: {error}", directory.display()))
        .filter_map(|entry| entry.ok().map(|entry| entry.path()))
        .filter(|path| path.extension().is_some_and(|kind| kind == "rs"))
        .map(|path| {
            let name = path
                .file_name()
                .map(Path::new)
                .and_then(Path::file_name)
                .map(|name| name.to_string_lossy().into_owned())
                .unwrap_or_default();
            let text = std::fs::read_to_string(&path).expect("a source should be readable");
            (name, text)
        })
        .collect();
    found.sort();
    assert!(found.len() > 5, "the crate's sources should be found");
    found
}

#[test]
fn every_threshold_is_declared_only_in_filters() {
    for (name, text) in sources() {
        for threshold in THRESHOLDS {
            assert!(
                !text.contains(&format!("pub const {threshold}")) || name == FILTERS,
                "{name} declares the threshold {threshold}; thresholds live in {FILTERS}"
            );
        }
    }
}

#[test]
fn the_floor_thresholds_are_compared_only_in_filters() {
    for (name, text) in sources() {
        if name == FILTERS {
            continue;
        }
        for threshold in FLOORS {
            assert!(
                !text.contains(threshold),
                "{name} still compares the floor threshold {threshold}"
            );
        }
    }
}

#[test]
fn the_filter_types_and_implementations_live_where_they_belong() {
    let filters = source(FILTERS);
    let models = source("models.rs");
    for declaration in [
        "pub struct Floors",
        "impl PostFilter for Floors",
        "pub struct WindowBarrier",
        "impl PreFilter for WindowBarrier",
        "pub fn scan_admitted",
        "pub fn merge_windows",
    ] {
        assert!(
            filters.contains(declaration),
            "{declaration} is not in {FILTERS}"
        );
    }
    for declaration in [
        "pub trait PreFilter",
        "pub trait PostFilter",
        "pub struct PreFilterInput",
        "pub struct PostFilterInput",
        "pub struct AdmittedRanges",
        "pub struct Refusal",
    ] {
        assert!(
            models.contains(declaration),
            "{declaration} is not declared in models.rs"
        );
    }
}

#[test]
fn the_reporting_filters_implement_their_trait() {
    let filters = source(FILTERS);
    let models = source("models.rs");
    for declaration in [
        "impl PostFilter for Floors",
        "impl PostFilter for Readable",
        "impl PreFilter for WindowBarrier",
        "impl PreFilter for KnownPayload",
    ] {
        assert!(
            filters.contains(declaration),
            "{declaration} is not in {FILTERS}"
        );
    }
    assert!(models.contains("pub trait PostFilter") && models.contains("pub trait PreFilter"));
    // admission is a filtering decision, so the scanner must not make it
    let lexagraph = source("lexagraph.rs");
    assert!(
        !lexagraph.contains("characters >= min_run"),
        "admission still happens in the scanner"
    );
}

#[test]
fn the_scanner_holds_no_filter_policy() {
    // The scanner forms runs; the filters decide policy. A composite admission rule in the scanner is a
    // second owner of the policy, which this rejects. Only the production part counts: the scanner's own
    // test module quotes the vocabulary API inside its assertions.
    let lexagraph = source("lexagraph.rs");
    let production = lexagraph
        .split("#[cfg(test)]")
        .next()
        .unwrap_or_default()
        .to_string();
    for expression in [
        "corroborates",
        "is_disabled",
        "READABLE.keeps",
        "MIN_LETTER_SHARE",
    ] {
        assert!(
            !production.contains(expression),
            "{expression} appears in the scanner; the composite rule belongs in {FILTERS}"
        );
    }
    assert!(
        source(FILTERS).contains("pub fn readability_verdict"),
        "the composite readability verdict must be implemented in {FILTERS}"
    );
}

#[test]
fn the_barrier_has_one_implementation() {
    let lexagraph = source("lexagraph.rs");
    assert!(
        !lexagraph.contains("fn merge_windows"),
        "the barrier's merge lives in {FILTERS}"
    );
    assert!(
        !lexagraph.contains("fn scan_windows"),
        "the barrier's segmentation lives in {FILTERS}"
    );
    assert!(
        lexagraph.contains("filters::ScanFilters"),
        "the archive scan must go through the composed filters rather than a barrier of its own"
    );
}
