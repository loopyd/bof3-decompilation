//! The readability rule's two-way calibration, ported from the Python suite.
//!
//! Replaces: `tools/python/tests/text/test_readability_calibration.py`.
//!
//! Both directions are measured through the **rule's own diagnostic** (`readability`), so the numbers are
//! the rule's answer rather than a re-implementation's:
//!
//! * **keep** — every verified row whose literal text is mostly letters and carries a word is retained
//!   (the scoped positive control), and the rule declines exactly the rows that carry no word together
//!   with the symbol-dominated ones;
//! * **reject** — the junk the request names is refused, and no data-subfile candidate is reported at all.

use std::collections::BTreeSet;
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

fn artifact(relative: &str) -> PathBuf {
    let path = repo_root().join(relative);
    assert!(
        path.is_file(),
        "{} is missing; build the text artifacts before running this suite",
        path.display()
    );
    path
}

/// Run the binary from the repository root, failing with its own message when it refuses.
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

/// Judge text with the actual rule and return its verdict counts.
fn judge(text: &[&str]) -> serde_json::Value {
    let mut args: Vec<&str> = vec!["readability", "--json"];
    for line in text {
        args.push("--text");
        args.push(line);
    }
    json(&args)
}

fn rows() -> Vec<serde_json::Value> {
    let index: serde_json::Value = serde_json::from_slice(
        &std::fs::read(artifact("out/text-index/index.json"))
            .expect("the index should be readable"),
    )
    .expect("the index should be JSON");
    index["instances"]
        .as_array()
        .cloned()
        .unwrap_or_default()
        .into_iter()
        .filter(|instance| {
            matches!(
                instance["class"].as_str(),
                Some("dialogue") | Some("battle")
            )
        })
        .collect()
}

/// The ASCII words of three or more letters in a reading — the tokeniser the vocabulary uses.
fn words(text: &str) -> BTreeSet<String> {
    let mut found = BTreeSet::new();
    let mut current = String::new();
    for character in text.chars() {
        if character.is_ascii_alphabetic() {
            current.push(character.to_ascii_lowercase());
        } else {
            if current.len() >= 3 {
                found.insert(std::mem::take(&mut current));
            } else {
                current.clear();
            }
        }
    }
    if current.len() >= 3 {
        found.insert(current);
    }
    found
}

#[test]
fn the_thresholds_keep_the_verified_text_that_is_text() {
    // keep: the control set is every verified row whose literal text is mostly letters and carries a word.
    // The rule retains all of it, and corroboration rejects none of it, because the vocabulary is derived
    // from these rows.
    let all = rows();
    assert_eq!(all.len(), 46_057, "the verified rows changed");
    let without_a_word: Vec<&serde_json::Value> = all
        .iter()
        .filter(|row| words(row["reading"].as_str().unwrap_or_default()).is_empty())
        .collect();
    assert_eq!(without_a_word.len(), 782, "word-less rows changed");

    let judged = json(&["readability", "--json"]);
    assert_eq!(judged["rows"].as_u64().unwrap_or(0), 46_057);
    assert_eq!(
        judged["shaped"], judged["readable"],
        "corroboration must not reject a verified row"
    );
    assert_eq!(
        judged["shaped"].as_u64().unwrap_or(0),
        45_212,
        "control set changed"
    );
    assert_eq!(
        judged["unshaped"].as_u64().unwrap_or(0),
        845,
        "declines changed"
    );
    assert_eq!(
        judged["unshaped"].as_u64().unwrap_or(0) as usize - without_a_word.len(),
        63,
        "the remaining declines are symbol-dominated rows"
    );
    // and every row the rule declines carries no word at all, or is symbol-dominated
    for failure in judged["failures"].as_array().cloned().unwrap_or_default() {
        let text = failure["text"].as_str().unwrap_or_default();
        let carries_a_word = !words(text).is_empty();
        assert!(
            !carries_a_word || failure["shaped"] == serde_json::json!(false),
            "the rule declined a row that carries a word and is shaped: {text:?}"
        );
    }
}

#[test]
fn the_rule_keeps_every_verified_word_in_its_vocabulary() {
    // the derivation the keep direction rests on: the vocabulary is built from these rows, so no row's
    // word may fall outside it.
    let vocabulary: serde_json::Value = serde_json::from_slice(
        &std::fs::read(artifact("out/text-vocabulary/vocabulary.json"))
            .expect("the vocabulary should be readable"),
    )
    .expect("the vocabulary should be JSON");
    let known: BTreeSet<String> = vocabulary["words"]
        .as_array()
        .cloned()
        .unwrap_or_default()
        .into_iter()
        .filter_map(|word| word.as_str().map(str::to_string))
        .collect();
    assert!(
        known.len() > 1_000,
        "the vocabulary is too small to attest anything"
    );
    let mut uncovered = 0usize;
    for row in rows() {
        let reading = row["reading"].as_str().unwrap_or_default();
        for word in words(reading) {
            if !known.contains(&word) {
                uncovered += 1;
            }
        }
    }
    assert_eq!(
        uncovered, 0,
        "{uncovered} verified word(s) are outside the vocabulary"
    );
}

#[test]
fn the_thresholds_reject_the_junk_they_are_meant_to() {
    // reject: literal junk of the kinds the corpus produces. Markup is judged as what it decodes to, so
    // `{end}` is not a fair input here — in the pipeline it decodes to a command and contributes no
    // literal text at all.
    let junk = judge(&[
        "????",
        "qqqq",
        "cddddddddddddd",
        "z;;;; ;drrddz;",
        "ZZZZ………□M□V……",
        "←zzzz○↓→→xxxxxyy",
    ]);
    assert_eq!(junk["rows"].as_u64().unwrap_or(0), 6);
    assert_eq!(
        junk["readable"].as_u64().unwrap_or(0),
        0,
        "junk passed the rule: {}",
        junk["failures"]
    );
    // a shaped row is still rejected when nothing corroborates it, and ordinary text passes both
    let shaped_only = judge(&["cddddddddddddd"]);
    assert_eq!(
        shaped_only["failures"][0]["shaped"],
        serde_json::json!(true)
    );
    assert_eq!(
        shaped_only["failures"][0]["attested"],
        serde_json::json!(false)
    );
    assert_eq!(judge(&["the village"])["readable"].as_u64().unwrap_or(0), 1);
}

#[test]
fn no_data_subfile_candidate_is_reported() {
    // the readability outcome on this corpus: nothing unreadable is reported, so the heuristic class is
    // empty and the game's text is carried by the parsed rows.
    let index: serde_json::Value = serde_json::from_slice(
        &std::fs::read(artifact("out/text-index/index.json"))
            .expect("the index should be readable"),
    )
    .expect("the index should be JSON");
    assert_eq!(
        index["counts"]
            .get("heuristic")
            .cloned()
            .unwrap_or(serde_json::json!(0)),
        serde_json::json!(0),
        "un-corroborated candidates are still reported"
    );
}
