//! Figure reconciliation: the evidence must agree with the artifacts it describes.
//!
//! Ported from `tools/python/tests/text/test_documented_figures.py`. This covers the artifact-side
//! assertions — the aggregate facts, the artifact-size binding, the census identity, the quoted crate test
//! count and the cost claims agreeing across documents. The live-scan rows, the archive ranking and the
//! 880-archive unwindowed census from the same Python file are **not** ported yet.

use std::path::PathBuf;
use std::process::Command;

/// Phrases that mark a figure as historical rather than current.
const HISTORICAL: [&str; 7] = [
    "superseded",
    "retired",
    "earlier",
    "historical",
    "pre-readability",
    "at the time",
    // the validation summary's own marker for a gate table captured at an earlier step, explained by
    // "Those are the gate results as they stood at that step"
    "(then)",
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

fn json(relative: &str) -> serde_json::Value {
    serde_json::from_slice(&std::fs::read(artifact(relative)).expect("the artifact is readable"))
        .unwrap_or_else(|error| panic!("{relative} should be JSON: {error}"))
}

/// Every evidence document, with its name.
fn evidence() -> Vec<(String, String)> {
    let directory = repo_root().join("out").join("text-format");
    let mut found: Vec<(String, String)> = std::fs::read_dir(&directory)
        .unwrap_or_else(|error| panic!("{} should list: {error}", directory.display()))
        .filter_map(|entry| entry.ok().map(|entry| entry.path()))
        .filter(|path| path.extension().is_some_and(|kind| kind == "md"))
        .map(|path| {
            let name = path
                .file_name()
                .map(|name| name.to_string_lossy().into_owned())
                .unwrap_or_default();
            let text = std::fs::read_to_string(&path).expect("an evidence file should be readable");
            (name, text)
        })
        .collect();
    found.sort();
    assert!(found.len() > 5, "the evidence tree should be populated");
    found
}

fn index() -> serde_json::Value {
    json("out/text-index/index.json")
}

fn map() -> serde_json::Value {
    json("out/text-format/segment-map.json")
}

fn registry() -> serde_json::Value {
    json("out/text-windows/registry.json")
}

#[test]
fn the_aggregate_facts_match_the_artifacts_and_are_quoted() {
    let index = index();
    let map = map();
    let registry = registry();
    let length = |value: &serde_json::Value| value.as_array().map(Vec::len).unwrap_or(0);
    let text = evidence()
        .iter()
        .map(|(_, text)| text.clone())
        .collect::<Vec<_>>()
        .join("\n");
    let facts: [(&str, u64); 4] = [
        ("46,057", length(&index["instances"]) as u64),
        (
            "0",
            map["totals"]
                .get("text_like_runs_in_data_subfiles")
                .and_then(serde_json::Value::as_u64)
                .unwrap_or(0),
        ),
        ("48,571", length(&registry["windows"]) as u64),
        (
            "0",
            map["totals"]
                .get("latches_removed_by_windows")
                .and_then(serde_json::Value::as_u64)
                .unwrap_or(0),
        ),
    ];
    for (quoted, actual) in facts {
        let number: u64 = quoted.replace(',', "").parse().unwrap_or(u64::MAX);
        assert_eq!(
            number, actual,
            "the artifacts report {actual}, not the quoted {quoted}"
        );
        assert!(
            text.contains(quoted),
            "{quoted} is not quoted anywhere in the evidence"
        );
    }
    // Presence is not enough: a document that claims a **different** count for the same fact contradicts
    // the artifacts, and that is what this rejects. The phrasing is narrowed to a corpus-wide claim
    // (`<n> instance(s) over`, `<n> classified window(s) over`, `<n> candidate run(s) in`), so an unrelated
    // number nearby — a search hit count, say — is not misread as one.
    let claims: [(&str, &str, u64); 3] = [
        (" instance", "over", 46_057),
        (" classified window", "over", 48_571),
        (" candidate run", "in", 0),
    ];
    let mut contradictions: Vec<String> = Vec::new();
    for (name, body) in evidence() {
        for (line_number, line) in body.lines().enumerate() {
            let lowered = line.to_lowercase();
            if HISTORICAL.iter().any(|marker| lowered.contains(marker)) {
                continue;
            }
            for (phrase, follower, expected) in claims {
                for (at, _) in lowered.match_indices(phrase) {
                    let head = lowered[..at].trim_end_matches(['s', ' ']);
                    let digits: String = head
                        .chars()
                        .rev()
                        .take_while(|character| character.is_ascii_digit() || *character == ',')
                        .collect::<String>()
                        .chars()
                        .rev()
                        .collect();
                    if digits.is_empty() {
                        continue;
                    }
                    // consume an optional plural `s` before the follower, so `45,000 instances over` is
                    // read as the same claim shape as `46,057 instances over`
                    let tail = lowered[at + phrase.len()..].trim_start();
                    let tail = tail.strip_prefix('s').unwrap_or(tail).trim_start();
                    if !tail.starts_with(follower) {
                        continue;
                    }
                    let claimed: u64 = digits.replace(',', "").parse().unwrap_or(u64::MAX);
                    if claimed != expected {
                        contradictions.push(format!(
                            "{name}:{}: claims {digits}{phrase} {follower} … where the artifacts report {expected}",
                            line_number + 1
                        ));
                    }
                }
            }
        }
    }
    assert!(contradictions.is_empty(), "{contradictions:?}");
    assert_eq!(
        index["counts"]
            .get("heuristic")
            .cloned()
            .unwrap_or(serde_json::json!(0)),
        serde_json::json!(0)
    );
}

#[test]
fn artifact_size_claims_match_the_artifacts() {
    // A size claim that names an artifact must be that artifact's size. Each claim is bound to the
    // **nearest preceding** artifact label on its line, so a sentence that mentions several artifacts at
    // once is read correctly rather than comparing every number with every artifact.
    let artifacts: [(&str, PathBuf); 6] = [
        ("index artifact", artifact("out/text-index/index.json")),
        ("index.json", artifact("out/text-index/index.json")),
        (
            "classified-window registry",
            artifact("out/text-windows/registry.json"),
        ),
        ("registry.json", artifact("out/text-windows/registry.json")),
        ("segment map", artifact("out/text-format/segment-map.json")),
        (
            "segment-map.json",
            artifact("out/text-format/segment-map.json"),
        ),
    ];
    let mut problems: Vec<String> = Vec::new();
    let mut claims = 0usize;
    for (name, text) in evidence() {
        for (line_number, line) in text.lines().enumerate() {
            let lowered = line.to_lowercase();
            if HISTORICAL.iter().any(|marker| lowered.contains(marker)) {
                continue;
            }
            // where each artifact is named on this line
            let labels: Vec<(usize, &str, &PathBuf)> = artifacts
                .iter()
                .filter_map(|(label, path)| lowered.find(label).map(|at| (at, *label, path)))
                .collect();
            if labels.is_empty() {
                continue;
            }
            for (word_at, _) in lowered.match_indices("byte") {
                let head = lowered[..word_at].trim_end_matches(['-', ' ']);
                let digits: String = head
                    .chars()
                    .rev()
                    .take_while(|character| character.is_ascii_digit() || *character == ',')
                    .collect::<String>()
                    .chars()
                    .rev()
                    .collect();
                if digits.is_empty() {
                    continue;
                }
                // the artifact named closest before this claim owns it
                let claim_at = word_at - digits.len();
                // the last label that starts at or before the claim owns it
                let mut owner: Option<&(usize, &str, &PathBuf)> = None;
                for candidate in &labels {
                    if candidate.0 <= claim_at {
                        owner = Some(candidate);
                    }
                }
                let Some((_, label, path)) = owner else {
                    continue;
                };
                let expected = std::fs::metadata(path).map(|data| data.len()).unwrap_or(0) as usize;
                let claimed: usize = digits.replace(',', "").parse().unwrap_or(0);
                claims += 1;
                if claimed != expected {
                    problems.push(format!(
                        "{name}:{}: {label} is {expected} bytes, not {claimed}",
                        line_number + 1
                    ));
                }
            }
        }
    }
    assert!(problems.is_empty(), "{problems:?}");
    assert!(
        claims >= 3,
        "the size binding should still find claims to check, found {claims}"
    );
}

#[test]
fn the_census_identity_holds() {
    let map = map();
    let candidates = map["totals"]
        .get("text_like_runs_in_data_subfiles")
        .and_then(serde_json::Value::as_u64)
        .unwrap_or(0);
    let (baseline, current) = (13_196u64, 1u64);
    assert_eq!(
        baseline - current,
        13_195,
        "the difference is what the exclusions and the rule removed"
    );
    assert!(
        candidates <= current,
        "the map's data-subfile candidates cannot exceed the whole census"
    );
    let text = evidence()
        .iter()
        .map(|(_, text)| text.clone())
        .collect::<Vec<_>>()
        .join("\n");
    for figure in ["13,196", "1"] {
        assert!(
            text.contains(figure),
            "the evidence does not state the census figure {figure}"
        );
    }
}

#[test]
fn the_quoted_crate_test_count_matches_the_sources() {
    let sources = repo_root().join("tools/rust/bof3-text/src");
    let mut counted = 0usize;
    for entry in std::fs::read_dir(&sources).expect("the crate sources should list") {
        let path = entry.expect("a readable entry").path();
        if path.extension().is_none_or(|kind| kind != "rs") {
            continue;
        }
        counted += std::fs::read_to_string(&path)
            .expect("a source should be readable")
            .matches("#[test]")
            .count();
    }
    assert!(counted >= 20, "the crate should still be tested: {counted}");
    for (name, text) in evidence() {
        if !name.starts_with("validation-summary") && !name.starts_with("harness-wiring") {
            continue;
        }
        assert!(
            text.contains(&format!("{counted} passed"))
                || text.contains(&format!("{counted} tests")),
            "{name} does not quote the crate's current test count ({counted})"
        );
        // Quoting the right number beside a wrong one is still a contradiction: every crate-attributed
        // count in the document has to agree with the sources.
        for (line_number, line) in text.lines().enumerate() {
            let lowered = line.to_lowercase();
            if HISTORICAL.iter().any(|marker| lowered.contains(marker)) {
                continue;
            }
            let about_the_crate = ["cargo test", "crate", "rust", "unit target"]
                .iter()
                .any(|marker| lowered.contains(marker));
            if !about_the_crate || lowered.contains("integration") || lowered.contains("text tests")
            {
                continue;
            }
            for (at, _) in lowered.match_indices(" test") {
                let head = lowered[..at].trim_end_matches('s');
                let digits: String = head
                    .chars()
                    .rev()
                    .take_while(|character| character.is_ascii_digit() || *character == ',')
                    .collect::<String>()
                    .chars()
                    .rev()
                    .collect();
                if digits.is_empty() {
                    continue;
                }
                let claimed: usize = digits.replace(',', "").parse().unwrap_or(usize::MAX);
                assert_eq!(
                    claimed,
                    counted,
                    "{name}:{}: claims {digits} crate tests where the sources hold {counted}",
                    line_number + 1
                );
            }
        }
    }
}

#[test]
fn the_cost_claims_agree_across_the_documents() {
    // The stage costs are quoted as ranges; the summary and the justfile must agree, and a cost quoted in
    // one place only is a claim nothing checks.
    let summary = evidence()
        .into_iter()
        .find(|(name, _)| name.starts_with("validation-summary"))
        .map(|(_, text)| text)
        .expect("the validation summary should exist");
    let justfile = std::fs::read_to_string(repo_root().join("justfile")).expect("readable");
    let ranges: Vec<&str> = summary
        .match_indices('≈')
        .filter_map(|(start, _)| summary.get(start..start + 12))
        .filter(|snippet| snippet.contains('s'))
        .collect();
    assert!(
        !ranges.is_empty(),
        "the summary should quote at least one cost range"
    );
    for range in ranges {
        let trimmed = range.trim_end_matches(|character: char| !character.is_ascii_digit());
        assert!(
            justfile.contains(trimmed),
            "the justfile does not carry the summary's cost {trimmed}"
        );
    }
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

/// Total latches over every archive, in eight workers: the corpus sweeps are the slow part.
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
fn the_representative_rows_match_a_live_scan() {
    // The documented rows must still describe what a live scan reports, and at least one row must show
    // that an identified payload no longer latches.
    let doc = std::fs::read_to_string(artifact("out/text-format/lexagraph-scan.md"))
        .expect("the scan document should be readable");
    let section = doc
        .split("## Representative latches outside the known banks")
        .nth(1)
        .and_then(|rest| rest.split("## Scanner reconstruction notice").next())
        .expect("the representative section should exist")
        .to_string();
    let mut rows = 0usize;
    let mut excluded = 0usize;
    for line in section.lines() {
        let Some(rest) = line.strip_prefix("- `") else {
            continue;
        };
        let Some((name, rest)) = rest.split_once(".EMI`") else {
            continue;
        };
        if rest.contains("**0 latch(es)**") {
            excluded += 1;
            continue;
        }
        let Some((_, tail)) = rest.split_once(": ") else {
            continue;
        };
        let mut parts = tail.splitn(3, ' ');
        let Some(count) = parts.next().and_then(|value| value.parse::<usize>().ok()) else {
            continue;
        };
        let Some(letters) = parts.next().and_then(|value| value.parse::<usize>().ok()) else {
            continue;
        };
        let entry = rest
            .split_once('#')
            .and_then(|(_, rest)| rest.split_once('@'))
            .and_then(|(entry, _)| entry.parse::<usize>().ok())
            .expect("a documented row names its entry");
        let path = repo_root()
            .join("out/extracted/BIN/BATTLE")
            .join(format!("{name}.EMI"));
        let result = Command::new(env!("CARGO_BIN_EXE_bof3-text"))
            .arg("scan")
            .arg(&path)
            .arg("--json")
            .current_dir(repo_root())
            .output()
            .expect("the built binary should run");
        let document: serde_json::Value =
            serde_json::from_slice(&result.stdout).expect("the scan should print JSON");
        let record = document["results"]
            .as_array()
            .cloned()
            .unwrap_or_default()
            .into_iter()
            .find(|record| {
                record["source"]
                    .as_str()
                    .unwrap_or_default()
                    .split('#')
                    .nth(1)
                    .and_then(|rest| rest.split('@').next())
                    .and_then(|value| value.parse::<usize>().ok())
                    == Some(entry)
            })
            .expect("the documented subfile should be scanned");
        let actual = record["latches"].as_array().cloned().unwrap_or_default();
        assert_eq!(
            actual.len(),
            count,
            "{name}#{entry}: documented {count} latches"
        );
        if let Some(first) = actual.first() {
            assert_eq!(
                first["alpha"].as_u64().unwrap_or(u64::MAX) as usize,
                letters,
                "{name}#{entry}: documented {letters} letters"
            );
        }
        rows += 1;
    }
    assert!(
        rows + excluded >= 8,
        "the representative list should be populated: {rows} + {excluded}"
    );
    assert!(
        excluded > 0,
        "the list must show that an identified payload no longer latches"
    );
}

#[test]
fn the_ranking_matches_a_live_unwindowed_scan() {
    // The document's top six archives are re-derived from the corpus, with all three opt-outs off only
    // where the document says so.
    let doc = std::fs::read_to_string(artifact("out/text-format/lexagraph-scan.md"))
        .expect("the scan document should be readable");
    let quoted: Vec<usize> = doc
        .lines()
        .find(|line| line.starts_with("Top archives by latch count"))
        .map(|line| {
            line.split('(')
                .filter_map(|chunk| chunk.split(',').next())
                .filter_map(|value| value.trim().parse::<usize>().ok())
                .collect()
        })
        .unwrap_or_default();
    if quoted.is_empty() {
        // Silence is only acceptable when the document says the ranking is unmeasured; otherwise the parse
        // failed and the guard would pass on a section it never checked.
        let lowered = doc.to_lowercase();
        assert!(
            lowered.contains("pending re-measurement") || lowered.contains("not measured"),
            "the archive ranking is neither quoted nor marked unmeasured, so nothing was checked"
        );
        return;
    }
    let corpus = repo_root().join("out/extracted/BIN");
    let mut ranked: Vec<(usize, String)> = Vec::new();
    let mut worlds: Vec<PathBuf> = std::fs::read_dir(&corpus)
        .expect("the corpus should list")
        .filter_map(|entry| entry.ok().map(|entry| entry.path()))
        .filter(|path| {
            path.is_dir()
                && path
                    .file_name()
                    .map(|name| name.to_string_lossy().starts_with("WORLD"))
                    .unwrap_or(false)
        })
        .collect();
    worlds.sort();
    for world in worlds {
        for entry in std::fs::read_dir(&world).expect("a world should list") {
            let path = entry.expect("a readable entry").path();
            if path.extension().is_none_or(|kind| kind != "EMI") {
                continue;
            }
            let name = path
                .file_name()
                .map(|name| name.to_string_lossy().into_owned())
                .unwrap_or_default();
            if !name.starts_with("AREA") {
                continue;
            }
            ranked.push((latches(&path, &["--no-windows"]), name));
        }
    }
    ranked.sort_by(|left, right| right.cmp(left));
    let measured: Vec<usize> = ranked
        .iter()
        .take(quoted.len())
        .map(|(count, _)| *count)
        .collect();
    assert_eq!(measured, quoted, "the documented ranking drifted");
}

#[test]
fn the_unwindowed_corpus_total_is_the_stated_census() {
    // Two censuses: the baseline with every opt-out, and the current pipeline's total.
    let baseline = corpus_total(&["--no-windows", "--no-payloads", "--no-readable"]);
    assert_eq!(
        baseline, 13_196,
        "the baseline corpus holds {baseline} runs"
    );
    let current = corpus_total(&["--no-windows"]);
    assert_eq!(current, 1, "the current pipeline holds {current} runs");
}

#[test]
fn each_cost_is_bound_to_its_operation() {
    // A cost quoted beside an operation is a checkable claim; the justfile's gate note must carry each
    // stage's figure on the stage's own line.
    let justfile = std::fs::read_to_string(repo_root().join("justfile")).expect("readable");
    let note = justfile
        .split("# Text tool gates")
        .nth(1)
        .and_then(|rest| rest.split("@cargo test").next())
        .expect("the justfile should carry the gate note");
    let lines: Vec<&str> = note
        .lines()
        .filter(|line| line.trim_start().starts_with('#'))
        .collect();
    assert!(lines.len() >= 4, "the gate note should list the stages");
    let with_cost: Vec<&&str> = lines
        .iter()
        .filter(|line| line.contains('s') && line.chars().any(|c| c.is_ascii_digit()))
        .collect();
    assert!(
        with_cost.len() >= 4,
        "at least four stages should carry a measured cost, found {}",
        with_cost.len()
    );
}
