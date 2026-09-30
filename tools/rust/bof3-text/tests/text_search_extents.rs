//! Public-output regression: a hit extent must be a true byte range in the original archive.
//!
//! Replaces: `tools/python/tests/text/test_search_extents.py`.
//!
//! These assertions come from the original corpus rather than from internal helpers, because the defects
//! they cover — an exclusive end offset published as a length, and a base-relative offset published as a
//! text-file offset — lived in the callers. Every expectation is derived from an artifact (the index's
//! recorded row, the probe's payload base, the registry's row extents) rather than restated.

use std::path::{Path, PathBuf};
use std::process::Command;
use std::sync::atomic::{AtomicUsize, Ordering};

const MIXED: &str = "WORLD00/AREA000.EMI";
const MIXED_NEEDLE: &str = "heroes, eh? Saved the village";
/// `heroes {/color} , <space> eh ? <space> Saved {nl} the <space> village`
const MIXED_BYTES: &[u8] = b"heroes\x06<\xffeh\\\xffSaved\x01the\xffvillage";
const TIMED_ENTRY: u64 = 11;
const STRING_OFFSET: u64 = 756_837;
const STRING_LENGTH: u64 = 139;
const MATCH_OFFSET: u64 = 756_881;
const MATCH_LENGTH: u64 = 30;

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

/// The built tool, refused with what to do rather than skipped: these tests gate public output.
fn require_binary() -> PathBuf {
    let path = binary();
    assert!(
        path.is_file(),
        "the text tool binary is missing at {}: build it with `just setup`, or any `bin/harness text` command",
        path.display()
    );
    path
}

fn corpus() -> PathBuf {
    let path = repo_root().join("out").join("extracted").join("BIN");
    assert!(
        path.is_dir(),
        "the extracted US corpus is missing at {}: extract it with `bin/harness media disc extract`",
        path.display()
    );
    path
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

fn index() -> serde_json::Value {
    serde_json::from_slice(&std::fs::read(artifact("out/text-index/index.json")).expect("readable"))
        .expect("the index should be JSON")
}

/// A scratch directory that removes itself, unique per call so parallel tests cannot collide.
struct Scratch(PathBuf);

impl Scratch {
    fn new(tag: &str) -> Self {
        static NEXT: AtomicUsize = AtomicUsize::new(0);
        let path = std::env::temp_dir().join(format!(
            "bof3-extents-{tag}-{}-{}",
            std::process::id(),
            NEXT.fetch_add(1, Ordering::SeqCst)
        ));
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

/// Run the tool from the repository root and parse its JSON, failing with its own message.
fn run(args: &[&str]) -> serde_json::Value {
    require_binary();
    let result = Command::new(binary())
        .args(args)
        .current_dir(repo_root())
        .output()
        .expect("the built binary should run");
    assert!(
        result.status.success(),
        "{} failed: {}",
        args.join(" "),
        String::from_utf8_lossy(&result.stderr).trim()
    );
    serde_json::from_slice(&result.stdout).expect("the command should print JSON")
}

/// Run the tool and return its raw stdout.
fn text(args: &[&str]) -> String {
    require_binary();
    let result = Command::new(binary())
        .args(args)
        .current_dir(repo_root())
        .output()
        .expect("the built binary should run");
    assert!(
        result.status.success(),
        "{} failed: {}",
        args.join(" "),
        String::from_utf8_lossy(&result.stderr).trim()
    );
    String::from_utf8_lossy(&result.stdout).into_owned()
}

/// The last line of the human output, which carries the anchor.
fn last_line(args: &[&str]) -> String {
    text(args)
        .trim()
        .lines()
        .last()
        .unwrap_or_default()
        .to_string()
}

/// The complete recorded **row** extent containing an archive-absolute offset, derived from the index.
///
/// The parse records every verified string as a row with an archive-absolute offset and length, so the
/// expectation comes from the artifact rather than being restated.
fn recorded_row(archive: &Path, archive_offset: u64) -> (u64, u64) {
    let name = archive
        .strip_prefix(corpus())
        .unwrap_or(archive)
        .to_string_lossy()
        .replace('\\', "/");
    for instance in index()["instances"].as_array().cloned().unwrap_or_default() {
        if instance["archive"].as_str() != Some(name.as_str()) {
            continue;
        }
        let offset = instance["offset"].as_u64().unwrap_or(0);
        let length = instance["length"].as_u64().unwrap_or(0);
        if offset <= archive_offset && archive_offset < offset + length {
            return (offset, length);
        }
    }
    panic!("{}: no recorded row contains {archive_offset}", name);
}

fn probe(archive: &Path) -> serde_json::Value {
    let result = Command::new(binary())
        .args(["probe", &archive.to_string_lossy(), "--json"])
        .current_dir(repo_root())
        .output()
        .expect("the built binary should run");
    assert!(
        result.status.success(),
        "probe {} failed: {}",
        archive.display(),
        String::from_utf8_lossy(&result.stderr).trim()
    );
    serde_json::from_slice(&result.stdout).expect("probe should print JSON")
}

/// The payload offset of one entry, from the probe.
fn payload_base(archive: &Path, entry: u64) -> u64 {
    probe(archive)["results"]
        .as_array()
        .cloned()
        .unwrap_or_default()
        .iter()
        .find(|record| record["entry"].as_u64() == Some(entry))
        .and_then(|record| record["offset"].as_u64())
        .unwrap_or_else(|| panic!("{}: entry {entry} was not probed", archive.display()))
}

/// The `(entry, payload base)` containing an archive-absolute offset, from the probe.
fn containing_subfile(archive: &Path, offset: u64) -> (u64, u64) {
    probe(archive)["results"]
        .as_array()
        .cloned()
        .unwrap_or_default()
        .iter()
        .find(|record| {
            let start = record["offset"].as_u64().unwrap_or(0);
            start <= offset && offset < start + record["size"].as_u64().unwrap_or(0)
        })
        .map(|record| {
            (
                record["entry"].as_u64().unwrap_or(0),
                record["offset"].as_u64().unwrap_or(0),
            )
        })
        .unwrap_or_else(|| panic!("{}: no subfile contains {offset}", archive.display()))
}

/// Decode one byte range through the tool's own extractor and return the text it yields.
fn decode_range(archive: &Path, entry: u64, base: u64, offset: u64, length: u64) -> String {
    let scratch = Scratch::new("latch");
    let out = scratch.path().join("range.json");
    let latch = format!("{entry}:{}+{length}", offset - base);
    let result = Command::new(binary())
        .args([
            "extract",
            &archive.to_string_lossy(),
            "--entry",
            &entry.to_string(),
            "--latch",
            &latch,
            "--output",
            &out.to_string_lossy(),
        ])
        .current_dir(repo_root())
        .output()
        .expect("the built binary should run");
    assert!(
        result.status.success(),
        "extract {latch} failed: {}",
        String::from_utf8_lossy(&result.stderr).trim()
    );
    let document: serde_json::Value =
        serde_json::from_slice(&std::fs::read(&out).expect("the extract should be readable"))
            .expect("the extract should be JSON");
    document["banks"][0][0]["text"]
        .as_str()
        .unwrap_or_default()
        .to_string()
}

/// Command tokens dropped and whitespace removed, which is how a reading matches a needle.
fn squashed(text: &str) -> String {
    let mut out = String::new();
    let mut depth = 0usize;
    for character in text.chars() {
        match character {
            '{' => depth += 1,
            '}' if depth > 0 => depth -= 1,
            _ if depth == 0 && !character.is_whitespace() => out.push(character),
            _ => {}
        }
    }
    out
}

/// The match a query reports for the mixed archive's entry 11.
fn row_hit(needle: &str, extra: &[&str]) -> serde_json::Value {
    let archive = corpus().join(MIXED);
    let mut args: Vec<String> = vec![
        "query".into(),
        archive.to_string_lossy().into_owned(),
        "--entry".into(),
        TIMED_ENTRY.to_string(),
        "--grep".into(),
        needle.to_string(),
        "--json".into(),
    ];
    args.extend(extra.iter().map(|value| (*value).to_string()));
    let borrowed: Vec<&str> = args.iter().map(String::as_str).collect();
    run(&borrowed)["results"][0]["matches"][0].clone()
}

/// The search hit for the mixed archive's entry 11.
fn search_hit(needle: &str, extra: &[&str]) -> serde_json::Value {
    let mut args: Vec<String> = vec![
        "search".into(),
        "--archive".into(),
        MIXED.into(),
        "--entry".into(),
        "11".into(),
        "--grep".into(),
        needle.to_string(),
        "--json".into(),
    ];
    args.extend(extra.iter().map(|value| (*value).to_string()));
    let borrowed: Vec<&str> = args.iter().map(String::as_str).collect();
    run(&borrowed)["results"][0].clone()
}

fn raw(relative: &str) -> Vec<u8> {
    let path = corpus().join(relative);
    assert!(
        path.is_file(),
        "{} is missing from the corpus",
        path.display()
    );
    std::fs::read(&path).expect("an archive should be readable")
}

fn number(value: &serde_json::Value, key: &str) -> u64 {
    value[key]
        .as_u64()
        .unwrap_or_else(|| panic!("{key} is missing from {value}"))
}

#[test]
fn prerequisites_are_present() {
    require_binary();
    corpus();
    assert!(
        corpus().join(MIXED).is_file(),
        "{MIXED} is missing from the corpus"
    );
}

#[test]
fn search_reports_the_true_extent_of_a_mixed_command_phrase() {
    let document = run(&[
        "search",
        "--archive",
        MIXED,
        "--entry",
        "11",
        "--grep",
        MIXED_NEEDLE,
        "--json",
    ]);
    assert_eq!(document["matched"].as_u64().unwrap_or(0), 1);
    let hit = document["results"][0].clone();
    let raw = raw(MIXED);
    let start = number(&hit, "match_offset") as usize;
    let length = number(&hit, "match_length") as usize;
    let span = &raw[start..start + length];
    assert_eq!(span, MIXED_BYTES, "{span:?}");
    assert!(
        number(&hit, "match_length") <= number(&hit, "length"),
        "an extent cannot exceed its instance"
    );
}

#[test]
fn query_reports_the_same_archive_absolute_extent_as_search() {
    let searched = search_hit(MIXED_NEEDLE, &[]);
    let queried = row_hit(MIXED_NEEDLE, &[]);
    assert_eq!(queried["match_offset"], searched["match_offset"]);
    assert_eq!(queried["match_length"], searched["match_length"]);
}

#[test]
fn a_query_reports_a_true_extent_across_a_boundary_space() {
    let hit = row_hit(MIXED_NEEDLE, &[]);
    let raw = raw(MIXED);
    let string_offset = number(&hit, "string_offset");
    let string_length = number(&hit, "string_length");
    let match_offset = number(&hit, "match_offset");
    let match_length = number(&hit, "match_length");
    assert!(string_offset < raw.len() as u64);
    assert!(string_offset <= match_offset, "the match precedes the run");
    assert!(
        match_offset + match_length <= string_offset + string_length,
        "the match must sit inside the run"
    );
    assert!(string_offset + string_length <= raw.len() as u64);
    let archive = corpus().join(MIXED);
    let (entry, base) = containing_subfile(&archive, string_offset);
    let whole = decode_range(&archive, entry, base, string_offset, string_length);
    assert_eq!(
        whole,
        hit["text"].as_str().unwrap_or_default(),
        "the whole range must decode to the text the hit reports"
    );
    let first_word = MIXED_NEEDLE
        .split(',')
        .next()
        .unwrap_or_default()
        .to_lowercase();
    assert!(squashed(&whole).to_lowercase().contains(&first_word));
}

#[test]
fn a_fused_spelling_is_not_matched() {
    let document = run(&[
        "query",
        &corpus()
            .join("WORLD01")
            .join("AREA070.EMI")
            .to_string_lossy(),
        "--class",
        "heuristic",
        "--grep",
        "KLK5\u{2022}",
        "--json",
    ]);
    let matches = &document["matches"];
    assert!(
        matches.as_array().is_some_and(|list| list.is_empty()),
        "a fused spelling matched: {matches}"
    );
}

#[test]
fn query_reports_the_string_anchor_by_default_and_the_match_on_request() {
    let default = row_hit(MIXED_NEEDLE, &[]);
    assert_eq!(number(&default, "string_offset"), STRING_OFFSET);
    assert_eq!(number(&default, "string_length"), STRING_LENGTH);
    assert_eq!(number(&default, "match_offset"), MATCH_OFFSET);
    assert_eq!(number(&default, "match_length"), MATCH_LENGTH);
    let raw = raw(MIXED);
    let start = number(&default, "match_offset") as usize;
    let length = number(&default, "match_length") as usize;
    assert_eq!(&raw[start..start + length], MIXED_BYTES);
    let string_start = number(&default, "string_offset") as usize;
    assert_eq!(&raw[string_start..string_start + 3], b"\x90Me");
}

#[test]
fn search_reports_the_same_two_anchors() {
    let hit = search_hit(MIXED_NEEDLE, &[]);
    assert_eq!(number(&hit, "string_offset"), STRING_OFFSET);
    assert_eq!(number(&hit, "string_length"), STRING_LENGTH);
    assert_eq!(number(&hit, "match_offset"), MATCH_OFFSET);
    assert_eq!(number(&hit, "match_length"), MATCH_LENGTH);
}

#[test]
fn the_human_line_switches_anchor_with_the_flag() {
    let archive = corpus().join(MIXED);
    let path = archive.to_string_lossy().into_owned();
    let default = last_line(&["query", &path, "--entry", "11", "--grep", MIXED_NEEDLE]);
    let flagged = last_line(&[
        "query",
        &path,
        "--entry",
        "11",
        "--grep",
        MIXED_NEEDLE,
        "--match-offset",
    ]);
    let string_anchor = format!("+{STRING_OFFSET:#x}");
    let match_anchor = format!("+{MATCH_OFFSET:#x}");
    assert!(
        default.contains(&string_anchor) && default.contains(&format!("{STRING_LENGTH}B")),
        "the default line is not anchored on the run: {default}"
    );
    assert!(
        flagged.contains(&match_anchor) && flagged.contains(&format!("{MATCH_LENGTH}B")),
        "the flagged line is not anchored on the match: {flagged}"
    );
}

#[test]
fn a_query_anchors_on_its_run_and_on_the_match() {
    let hit = row_hit(MIXED_NEEDLE, &[]);
    assert_ne!(
        number(&hit, "string_offset"),
        number(&hit, "match_offset"),
        "the run and the match must differ"
    );
    assert!(
        number(&hit, "string_length") > number(&hit, "match_length"),
        "a run must be longer than its match"
    );
    assert_eq!(hit["anchor"].as_str(), Some("string"));
    assert_eq!(
        number(&hit, "reported_offset"),
        number(&hit, "string_offset")
    );
    let flagged = row_hit(MIXED_NEEDLE, &["--match-offset"]);
    assert_eq!(
        flagged["anchor"].as_str(),
        Some("match"),
        "the flag must switch the anchor"
    );
    assert_eq!(
        number(&flagged, "reported_offset"),
        number(&flagged, "match_offset")
    );
}

#[test]
fn query_offsets_are_correct_in_the_second_bank_of_a_two_bank_subfile() {
    let archive = corpus().join("BATTLE").join("BATTLE.EMI");
    let queried = run(&[
        "query",
        &archive.to_string_lossy(),
        "--entry",
        "11",
        "--grep",
        "Restores",
        "--json",
    ])["results"]
        .as_array()
        .cloned()
        .unwrap_or_default()
        .iter()
        .flat_map(|result| result["matches"].as_array().cloned().unwrap_or_default())
        .next()
        .expect("the second bank should hold the match");
    let searched = run(&[
        "search",
        "--archive",
        "BATTLE/BATTLE.EMI",
        "--entry",
        "11",
        "--class",
        "battle",
        "--grep",
        "Restores",
        "--json",
    ])["results"][0]
        .clone();
    assert_eq!(number(&queried, "bank"), 1);
    assert_eq!(
        queried["match_offset"], searched["match_offset"],
        "a bank-1 offset must include the bank start"
    );
    let bytes = raw("BATTLE/BATTLE.EMI");
    let start = number(&queried, "match_offset") as usize;
    assert_eq!(&bytes[start..start + 8], b"Restores");
}

#[test]
fn the_full_string_range_decodes_to_the_row_and_the_match_range_to_the_needle() {
    let archive = corpus().join(MIXED);
    let hit = row_hit(MIXED_NEEDLE, &[]);
    let base = payload_base(&archive, 11);
    let string_offset = number(&hit, "string_offset");
    let string_length = number(&hit, "string_length");
    let match_offset = number(&hit, "match_offset");
    let match_length = number(&hit, "match_length");
    let whole = decode_range(&archive, 11, base, string_offset, string_length);
    assert_eq!(
        whole,
        hit["text"].as_str().unwrap_or_default(),
        "the complete string range must decode to the reported row"
    );
    assert!(
        whole.len() > MIXED_NEEDLE.len(),
        "the row is longer than the needle it contains"
    );
    let matched = decode_range(&archive, 11, base, match_offset, match_length);
    assert_eq!(
        squashed(&matched),
        squashed(MIXED_NEEDLE),
        "{matched:?} is not the needle"
    );
    let relative = match_offset - string_offset;
    let inner = decode_range(&archive, 11, base, string_offset + relative, match_length);
    assert_eq!(squashed(&inner), squashed(MIXED_NEEDLE));
    assert!(match_offset + match_length <= string_offset + string_length);
}

#[test]
fn json_selects_the_anchor_the_flag_asks_for_in_both_modes() {
    let default = row_hit(MIXED_NEEDLE, &[]);
    let flagged = row_hit(MIXED_NEEDLE, &["--match-offset"]);
    assert_eq!(
        (
            default["anchor"].as_str(),
            number(&default, "reported_offset"),
            number(&default, "reported_length"),
        ),
        (Some("string"), STRING_OFFSET, STRING_LENGTH)
    );
    assert_eq!(
        (
            flagged["anchor"].as_str(),
            number(&flagged, "reported_offset"),
            number(&flagged, "reported_length"),
        ),
        (Some("match"), MATCH_OFFSET, MATCH_LENGTH)
    );
    let searched = search_hit(MIXED_NEEDLE, &[]);
    let searched_flagged = search_hit(MIXED_NEEDLE, &["--match-offset"]);
    assert_eq!(
        (
            searched["anchor"].as_str(),
            number(&searched, "reported_offset")
        ),
        (Some("string"), STRING_OFFSET)
    );
    assert_eq!(
        (
            searched_flagged["anchor"].as_str(),
            number(&searched_flagged, "reported_offset"),
        ),
        (Some("match"), MATCH_OFFSET)
    );
}

#[test]
fn the_json_reports_both_full_ranges_and_follows_the_flag() {
    let hit = row_hit(MIXED_NEEDLE, &[]);
    let raw = raw(MIXED);
    let string_offset = number(&hit, "string_offset");
    let string_length = number(&hit, "string_length");
    let match_offset = number(&hit, "match_offset");
    let match_length = number(&hit, "match_length");
    let whole = &raw[string_offset as usize..(string_offset + string_length) as usize];
    let matched = &raw[match_offset as usize..(match_offset + match_length) as usize];
    assert_eq!(
        whole.len() as u64,
        string_length,
        "the run range must be the whole run"
    );
    assert_eq!(
        (string_offset, string_length),
        recorded_row(&corpus().join(MIXED), match_offset),
        "the anchor is the beginning of the complete recorded row"
    );
    assert_eq!(
        whole[(match_offset - string_offset) as usize],
        matched[0],
        "the match sits inside the run"
    );
    let flagged = row_hit(MIXED_NEEDLE, &["--match-offset"]);
    assert_eq!(flagged["anchor"].as_str(), Some("match"));
    assert_eq!(
        number(&flagged, "match_offset"),
        number(&flagged, "reported_offset")
    );
}

#[test]
fn a_missing_registry_fails_closed_rather_than_latching_unwindowed() {
    let result = Command::new(binary())
        .args([
            "scan",
            &corpus().join(MIXED).to_string_lossy(),
            "--windows",
            "/nonexistent/registry.json",
            "--json",
        ])
        .current_dir(repo_root())
        .output()
        .expect("the built binary should run");
    assert!(
        !result.status.success(),
        "an unavailable ownership source must not become no barriers"
    );
    assert!(
        String::from_utf8_lossy(&result.stderr).contains("is missing"),
        "unexpected refusal: {}",
        String::from_utf8_lossy(&result.stderr).trim()
    );
}

#[test]
fn every_latching_mode_reports_what_the_exclusions_removed() {
    // A subfile whose whole payload is identified is never parsed, so it reports the bytes it skipped
    // rather than a per-run removal count; the counts remain available from the registry path.
    let document = run(&[
        "scan",
        &corpus()
            .join("WORLD00")
            .join("AREA026.EMI")
            .to_string_lossy(),
        "--json",
    ]);
    let tally = &document["windows"];
    assert!(
        tally.get("payloads").is_some(),
        "the tally does not report payloads"
    );
    assert!(
        tally.get("readable_dropped").is_some(),
        "the tally does not report readable_dropped"
    );
    assert!(tally["bytes_skipped"].as_u64().is_some());
    let per_window: u64 = tally["per_window"]
        .as_array()
        .cloned()
        .unwrap_or_default()
        .iter()
        .map(|entry| entry["latches_removed"].as_u64().unwrap_or(0))
        .sum();
    assert_eq!(per_window, tally["latches_removed"].as_u64().unwrap_or(0));
}

#[test]
fn the_banked_full_range_is_a_registered_row_extent_and_identical_across_modes() {
    let registry: serde_json::Value = serde_json::from_slice(
        &std::fs::read(artifact("out/text-windows/registry.json")).expect("readable"),
    )
    .expect("the registry should be JSON");
    let windows = registry["windows"].as_array().cloned().unwrap_or_default();
    let rows: Vec<(u64, u64)> = windows
        .iter()
        .filter(|window| {
            window["archive"].as_str() == Some(MIXED)
                && window["entry"].as_u64() == Some(11)
                && window["kind"].as_str() == Some("row_extent")
        })
        .map(|window| {
            (
                window["start"].as_u64().unwrap_or(0),
                window["length"].as_u64().unwrap_or(0),
            )
        })
        .collect();
    let bases: Vec<u64> = windows
        .iter()
        .filter(|window| {
            window["archive"].as_str() == Some(MIXED)
                && window["entry"].as_u64() == Some(11)
                && window["kind"].as_str() == Some("pointer_table")
        })
        .map(|window| window["start"].as_u64().unwrap_or(0))
        .collect();
    let queried = row_hit(MIXED_NEEDLE, &[]);
    let searched = search_hit(MIXED_NEEDLE, &[]);
    let bytes = raw(MIXED);
    // the reported full range must be one of the registered row extents, at its own payload base
    let payload_base = payload_base(&corpus().join(MIXED), 11);
    let pair = (
        number(&queried, "string_offset") - payload_base,
        number(&queried, "string_length"),
    );
    assert!(
        rows.contains(&pair),
        "{pair:?} is not a registered row extent"
    );
    let queried_range = &bytes[number(&queried, "string_offset") as usize
        ..(number(&queried, "string_offset") + number(&queried, "string_length")) as usize];
    let searched_range = &bytes[number(&searched, "string_offset") as usize
        ..(number(&searched, "string_offset") + number(&searched, "string_length")) as usize];
    assert_eq!(
        queried_range, searched_range,
        "the two modes must report identical bytes"
    );
    assert_eq!(number(&queried, "string_length"), 139);
    assert_eq!(number(&searched, "string_length"), 139);
    assert!(
        !bases.is_empty(),
        "the pointer table must be registered too"
    );
}

#[test]
fn the_full_run_range_bounds_the_match_and_follows_the_flag() {
    let hit = row_hit(MIXED_NEEDLE, &[]);
    let bytes = raw(MIXED);
    let string_offset = number(&hit, "string_offset");
    let string_length = number(&hit, "string_length");
    let match_offset = number(&hit, "match_offset");
    let match_length = number(&hit, "match_length");
    let run_bytes = &bytes[string_offset as usize..(string_offset + string_length) as usize];
    let match_bytes = &bytes[match_offset as usize..(match_offset + match_length) as usize];
    assert_eq!(
        run_bytes.len() as u64,
        string_length,
        "the whole run, not a sample"
    );
    assert_eq!(
        (string_offset, string_length),
        recorded_row(&corpus().join(MIXED), match_offset),
        "the anchor is the beginning of the complete recorded row"
    );
    let archive = corpus().join(MIXED);
    let (entry, base) = containing_subfile(&archive, string_offset);
    let whole = decode_range(&archive, entry, base, string_offset, string_length);
    assert_eq!(
        whole,
        hit["text"].as_str().unwrap_or_default(),
        "the complete run must decode to the text the hit reports"
    );
    let matched = decode_range(&archive, entry, base, match_offset, match_length);
    assert!(
        squashed(&whole)
            .to_lowercase()
            .contains(&squashed(&matched).to_lowercase()),
        "{matched:?} is not part of the run's text"
    );
    let relative = (match_offset - string_offset) as usize;
    assert_eq!(
        &run_bytes[relative..relative + match_length as usize],
        match_bytes
    );

    let path = archive.to_string_lossy().into_owned();
    let plain = last_line(&["query", &path, "--entry", "11", "--grep", MIXED_NEEDLE]);
    let flagged = last_line(&[
        "query",
        &path,
        "--entry",
        "11",
        "--grep",
        MIXED_NEEDLE,
        "--match-offset",
    ]);
    assert_ne!(plain, flagged, "the human line must follow the flag");
}

#[test]
fn no_data_subfile_candidate_is_reported() {
    // The readability rule reports only a run every word of which the game writes, and on this corpus no
    // data-subfile candidate qualifies. Asserted so a change in the candidate set is noticed.
    assert_eq!(
        index()["counts"]
            .get("heuristic")
            .cloned()
            .unwrap_or(serde_json::json!(0)),
        serde_json::json!(0)
    );
}

#[test]
fn the_human_line_switches_in_search_too() {
    let plain = last_line(&[
        "search",
        "--archive",
        MIXED,
        "--entry",
        "11",
        "--grep",
        MIXED_NEEDLE,
    ]);
    let flagged = last_line(&[
        "search",
        "--archive",
        MIXED,
        "--entry",
        "11",
        "--grep",
        MIXED_NEEDLE,
        "--match-offset",
    ]);
    assert!(plain.contains(&format!("+{STRING_OFFSET:#x}")), "{plain}");
    assert!(
        flagged.contains(&format!("+{MATCH_OFFSET:#x}")),
        "{flagged}"
    );
}

#[test]
fn a_basename_archive_argument_resolves_to_the_same_barriers() {
    // From inside the world directory the archive is named by its bare file name; the registry must resolve
    // it, and the result must match the absolute-path invocation exactly.
    let absolute = run(&[
        "scan",
        &corpus()
            .join("WORLD00")
            .join("AREA026.EMI")
            .to_string_lossy(),
        "--entry",
        "11",
        "--json",
        "--no-payloads",
    ]);
    let relative = Command::new(binary())
        .args([
            "scan",
            "AREA026.EMI",
            "--entry",
            "11",
            "--json",
            "--windows",
            &artifact("out/text-windows/registry.json").to_string_lossy(),
            "--no-payloads",
        ])
        .current_dir(corpus().join("WORLD00"))
        .output()
        .expect("the built binary should run");
    assert!(
        relative.status.success(),
        "{}",
        String::from_utf8_lossy(&relative.stderr).trim()
    );
    let bare: serde_json::Value =
        serde_json::from_slice(&relative.stdout).expect("the scan should print JSON");
    assert_eq!(bare["windows"]["windows"], absolute["windows"]["windows"]);
    let count = |document: &serde_json::Value| -> usize {
        document["results"]
            .as_array()
            .cloned()
            .unwrap_or_default()
            .iter()
            .map(|record| record["latches"].as_array().map_or(0, Vec::len))
            .sum()
    };
    assert_eq!(count(&bare), count(&absolute));
    assert_eq!(count(&bare), 0);
}

#[test]
fn the_complete_string_range_decodes_to_the_reported_string_in_all_three_modes() {
    let archive = corpus().join(MIXED);
    let banked = row_hit(MIXED_NEEDLE, &[]);
    let base = payload_base(&archive, 11);
    assert_eq!(
        decode_range(
            &archive,
            11,
            base,
            number(&banked, "string_offset"),
            number(&banked, "string_length"),
        ),
        banked["text"].as_str().unwrap_or_default()
    );
    let searched = search_hit(MIXED_NEEDLE, &[]);
    assert_eq!(
        decode_range(
            &archive,
            11,
            base,
            number(&searched, "string_offset"),
            number(&searched, "string_length"),
        ),
        searched["text"].as_str().unwrap_or_default()
    );
    // and the same complete-range property holds for a query hit found by containment
    let row = row_hit(MIXED_NEEDLE, &[]);
    let (entry, containing_base) = containing_subfile(&archive, number(&row, "string_offset"));
    assert_eq!(
        decode_range(
            &archive,
            entry,
            containing_base,
            number(&row, "string_offset"),
            number(&row, "string_length"),
        ),
        row["text"].as_str().unwrap_or_default()
    );
}
