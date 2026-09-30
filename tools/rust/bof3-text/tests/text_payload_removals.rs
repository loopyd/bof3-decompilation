//! Payload-removal accounting: the recorded figures must be reproducible, and retired ones must stay out.
//!
//! Replaces: `tools/python/tests/text/test_payload_removals.py`.
//!
//! An excluded payload is never parsed, so the tool cannot count what it removed at run time: the numbers
//! are measured once and recorded in the inventory. This re-derives them from the corpus, and fails if the
//! evidence quotes a figure the artifacts no longer support.

use std::collections::BTreeMap;
use std::path::PathBuf;
use std::process::Command;
use std::sync::Arc;

/// Figures the artifacts no longer support. Each was true of an earlier pipeline and survived into prose
/// at least once; a guard that only checked the presence of current figures did not catch them.
const RETIRED: [&str; 14] = [
    "12,721",
    "1,725",
    "10,806",
    "5,054",
    "46,097",
    "56,863",
    "11,970",
    "786",
    "1,603",
    "58,941",
    "58941",
    "20,416,048",
    "14,387,239",
    "10,967,663",
];

/// Phrases that mark a figure as historical rather than current, so the guard does not fire on an honest
/// record of an earlier state.
const HISTORICAL: [&str; 9] = [
    "superseded",
    "retired",
    "earlier",
    "void",
    "historical",
    "reconstruction",
    "at the time",
    "recorded then",
    "pre-readability",
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

fn json(args: &[&str]) -> serde_json::Value {
    let result = Command::new(env!("CARGO_BIN_EXE_bof3-text"))
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

fn rows(args: &[&str]) -> Vec<serde_json::Value> {
    json(args)["results"]
        .as_array()
        .cloned()
        .unwrap_or_default()
}

/// Per-entry latch counts for one archive, with the payload exclusion optionally disabled.
fn counts(archive: &str, readability: &[&str], without_payloads: bool) -> BTreeMap<usize, usize> {
    let path = corpus().join(archive);
    let mut args: Vec<String> = vec![
        "scan".to_string(),
        path.to_string_lossy().into_owned(),
        "--json".to_string(),
        "--no-windows".to_string(),
    ];
    if without_payloads {
        args.push("--no-payloads".to_string());
    }
    args.extend(readability.iter().map(|value| (*value).to_string()));
    let borrowed: Vec<&str> = args.iter().map(String::as_str).collect();
    json(&borrowed)["results"]
        .as_array()
        .cloned()
        .unwrap_or_default()
        .iter()
        .map(|record| {
            let entry = record["source"]
                .as_str()
                .unwrap_or_default()
                .split('#')
                .nth(1)
                .and_then(|rest| rest.split('@').next())
                .and_then(|value| value.parse::<usize>().ok())
                .unwrap_or_default();
            (entry, record["latches"].as_array().map_or(0, Vec::len))
        })
        .collect()
}

#[test]
fn the_recorded_removals_are_reproducible() {
    let root = corpus().to_string_lossy().into_owned();
    let payloads = rows(&["payloads", "--root", &root]);
    let mut families: BTreeMap<String, BTreeMap<usize, String>> = BTreeMap::new();
    for row in &payloads {
        let archive = row["archive"].as_str().unwrap_or_default().to_string();
        let entry = row["entry"].as_u64().unwrap_or_default() as usize;
        // the pairing family carries no magic of its own; name it the way the inventory does
        let magic = row["magic"].as_str().unwrap_or_default();
        let magic = if magic.is_empty() {
            "pBAV+ct7".to_string()
        } else {
            magic.to_string()
        };
        families.entry(archive).or_default().insert(entry, magic);
    }
    let families = Arc::new(families);
    let archives: Vec<String> = families.keys().cloned().collect();

    let measure = |readability: Vec<&'static str>| -> BTreeMap<String, usize> {
        let workers = 8usize;
        let mut totals: BTreeMap<String, usize> = BTreeMap::new();
        std::thread::scope(|scope| {
            let handles: Vec<_> = archives
                .chunks(archives.len().div_ceil(workers).max(1))
                .map(|slice| {
                    let families = Arc::clone(&families);
                    let readability = readability.clone();
                    scope.spawn(move || {
                        let mut local: BTreeMap<String, usize> = BTreeMap::new();
                        for archive in slice {
                            let with_exclusion = counts(archive, &readability, false);
                            let without = counts(archive, &readability, true);
                            for (entry, magic) in &families[archive] {
                                let lost = without.get(entry).copied().unwrap_or(0).saturating_sub(
                                    with_exclusion.get(entry).copied().unwrap_or(0),
                                );
                                if lost > 0 {
                                    *local.entry(magic.clone()).or_default() += lost;
                                }
                            }
                        }
                        local
                    })
                })
                .collect();
            for handle in handles {
                for (magic, count) in handle.join().expect("a measurement worker failed") {
                    *totals.entry(magic).or_default() += count;
                }
            }
        });
        totals
    };

    let standalone = measure(vec!["--no-readable"]);
    let marginal = measure(Vec::new());
    let document: serde_json::Value = serde_json::from_slice(
        &std::fs::read(inventory()).expect("the inventory should be readable"),
    )
    .expect("the inventory should be JSON");
    let empty: Vec<serde_json::Value> = Vec::new();
    for entry in document["exclusions"]
        .as_array()
        .unwrap_or(&empty)
        .iter()
        .chain(
            document["identified_without_signature"]
                .as_array()
                .unwrap_or(&empty)
                .iter(),
        )
    {
        let magic = match entry["magic"].as_str() {
            Some(value) if !value.is_empty() => value.to_string(),
            _ => "pBAV+ct7".to_string(),
        };
        let measured = &entry["measured"];
        assert_eq!(
            standalone.get(&magic).copied().unwrap_or(0),
            measured["removed"].as_u64().unwrap_or(0) as usize,
            "{magic}: standalone removals differ from the recorded figure"
        );
        assert_eq!(
            marginal.get(&magic).copied().unwrap_or(0),
            measured["removed_marginal"].as_u64().unwrap_or(0) as usize,
            "{magic}: marginal removals differ from the recorded figure"
        );
    }
}

#[test]
fn the_evidence_quotes_no_retired_figures() {
    let evidence = repo_root().join("out").join("text-format");
    let mut problems: Vec<String> = Vec::new();
    for entry in std::fs::read_dir(&evidence).expect("the evidence directory should list") {
        let path = entry.expect("a readable entry").path();
        if path.extension().is_none_or(|kind| kind != "md") {
            continue;
        }
        let name = path
            .file_name()
            .map(|name| name.to_string_lossy().into_owned())
            .unwrap_or_default();
        let text = std::fs::read_to_string(&path).expect("an evidence file should be readable");
        for line in text.lines() {
            let lowered = line.to_lowercase();
            if HISTORICAL.iter().any(|marker| lowered.contains(marker)) {
                continue;
            }
            for figure in RETIRED {
                if lowered.contains(figure) {
                    problems.push(format!("{name}: {figure}"));
                }
            }
        }
    }
    assert!(
        problems.is_empty(),
        "retired figures are still quoted: {problems:?}"
    );
}

#[test]
fn no_current_claim_contradicts_the_candidate_count() {
    // A current-state claim about candidate runs has to be zero. Markdown emphasis hides a number from a
    // naive pattern, which is how a `**21** candidate runs` claim once survived the first version of this
    // guard, so emphasis is stripped before matching.
    let evidence = repo_root().join("out").join("text-format");
    let mut problems: Vec<String> = Vec::new();
    for entry in std::fs::read_dir(&evidence).expect("the evidence directory should list") {
        let path = entry.expect("a readable entry").path();
        if path.extension().is_none_or(|kind| kind != "md") {
            continue;
        }
        let name = path
            .file_name()
            .map(|name| name.to_string_lossy().into_owned())
            .unwrap_or_default();
        let text = std::fs::read_to_string(&path).expect("an evidence file should be readable");
        for (number, line) in text.lines().enumerate() {
            let lowered = line.to_lowercase();
            if HISTORICAL.iter().any(|marker| lowered.contains(marker)) {
                continue;
            }
            let plain: String = lowered
                .chars()
                .filter(|character| !matches!(character, '*' | '`' | '_'))
                .collect();
            let words: Vec<&str> = plain.split_whitespace().collect();
            // The claim shape the Python guard used: a number, then candidate/heuristic, then a run or
            // instance noun. Requiring the third word keeps legitimate prose like "removed 472 candidates"
            // from being read as a claim about the candidate set.
            for triple in words.windows(3) {
                let (count, noun, kind) = (triple[0], triple[1], triple[2]);
                let digits = count.replace(',', "");
                let is_count = !digits.is_empty() && digits.chars().all(|c| c.is_ascii_digit());
                let names_candidates =
                    noun.starts_with("candidate") || noun.starts_with("heuristic");
                let names_runs = matches!(kind, "run" | "runs" | "instance" | "instances");
                if is_count && names_candidates && names_runs {
                    let value: usize = digits.parse().unwrap_or(0);
                    if value != 0 {
                        problems.push(format!(
                            "{name}:{}: claims {count} {noun} {kind} where the artifacts report 0",
                            number + 1
                        ));
                    }
                }
            }
        }
    }
    assert!(problems.is_empty(), "{problems:?}");
    // the guard must know the retired set, so a silently emptied list is itself a failure
    assert!(
        RETIRED.len() >= 10,
        "the retired-figure list should not shrink"
    );
}
