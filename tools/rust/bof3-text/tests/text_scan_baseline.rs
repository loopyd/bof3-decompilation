//! A retained behaviour baseline for the whole corpus.
//!
//! Replaces: `tools/python/tests/text/test_scan_baseline.py`.
//!
//! The filter consolidation claimed to change no behaviour, and a claim like that is only checkable if the
//! previous behaviour was retained in a form a later change can be compared against. These files are that
//! form: the SHA-256 of every archive's `scan --json` output, with and without the window check, over the
//! 880-archive corpus. Regenerating them and comparing is a mechanical proof that the scanning pipeline
//! still produces byte-identical results for every archive.

use std::path::{Path, PathBuf};
use std::process::Command;

/// The baseline files and the scan configuration each was captured with.
const BASELINES: [(&str, &[&str]); 2] = [
    ("per-archive-scan-digests.txt", &["--no-windows"]),
    ("per-archive-scan-digests-windowed.txt", &[]),
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

/// Every archive under a directory, in a stable order.
fn archives(root: &Path) -> Vec<PathBuf> {
    let mut found = Vec::new();
    let mut pending = vec![root.to_path_buf()];
    while let Some(directory) = pending.pop() {
        for entry in std::fs::read_dir(&directory)
            .unwrap_or_else(|error| panic!("{}: {error}", directory.display()))
        {
            let path = entry.expect("a readable directory entry").path();
            if path.is_dir() {
                pending.push(path);
            } else if path.extension().is_some_and(|kind| kind == "EMI") {
                found.push(path);
            }
        }
    }
    found.sort();
    found
}

/// The retained `digest  path` lines of one baseline file.
fn retained(name: &str) -> Vec<(String, String)> {
    let path = repo_root().join("out").join("text-format").join(name);
    let text = std::fs::read_to_string(&path).unwrap_or_else(|error| {
        panic!(
            "the retained baseline {} is missing ({error}); capture it before comparing",
            path.display()
        )
    });
    let mut rows: Vec<(String, String)> = text
        .lines()
        .filter(|line| !line.trim().is_empty())
        .map(|line| {
            let (digest, file) = line
                .split_once("  ")
                .expect("a baseline line is `digest  path`");
            (digest.to_string(), file.to_string())
        })
        .collect();
    assert!(!rows.is_empty(), "the retained baseline {name} is empty");
    rows.sort();
    rows
}

/// The SHA-256 of one archive's scan output, in the script's digest form.
fn digest(archive: &Path, flags: &[&str]) -> String {
    let result = Command::new(env!("CARGO_BIN_EXE_bof3-text"))
        .arg("scan")
        .arg(archive)
        .arg("--json")
        .args(flags)
        .current_dir(repo_root())
        .output()
        .expect("the built binary should run");
    assert!(
        result.status.success(),
        "{} failed: {}",
        archive.display(),
        String::from_utf8_lossy(&result.stderr).trim()
    );
    hex(&result.stdout)
}

/// SHA-256 in lowercase hex — the form the baseline files use.
fn hex(bytes: &[u8]) -> String {
    const K: [u32; 64] = [
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4,
        0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe,
        0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f,
        0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
        0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc,
        0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
        0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116,
        0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7,
        0xc67178f2,
    ];
    let mut state: [u32; 8] = [
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab,
        0x5be0cd19,
    ];
    let mut message = bytes.to_vec();
    let bit_length = (bytes.len() as u64).wrapping_mul(8);
    message.push(0x80);
    while message.len() % 64 != 56 {
        message.push(0);
    }
    message.extend_from_slice(&bit_length.to_be_bytes());
    for chunk in message.chunks(64) {
        let mut schedule = [0u32; 64];
        for (position, word) in schedule.iter_mut().take(16).enumerate() {
            *word = u32::from_be_bytes([
                chunk[4 * position],
                chunk[4 * position + 1],
                chunk[4 * position + 2],
                chunk[4 * position + 3],
            ]);
        }
        for position in 16..64 {
            let small0 = schedule[position - 15].rotate_right(7)
                ^ schedule[position - 15].rotate_right(18)
                ^ (schedule[position - 15] >> 3);
            let small1 = schedule[position - 2].rotate_right(17)
                ^ schedule[position - 2].rotate_right(19)
                ^ (schedule[position - 2] >> 10);
            schedule[position] = schedule[position - 16]
                .wrapping_add(small0)
                .wrapping_add(schedule[position - 7])
                .wrapping_add(small1);
        }
        let mut working = state;
        for position in 0..64 {
            let big1 = working[4].rotate_right(6)
                ^ working[4].rotate_right(11)
                ^ working[4].rotate_right(25);
            let choose = (working[4] & working[5]) ^ ((!working[4]) & working[6]);
            let temp1 = working[7]
                .wrapping_add(big1)
                .wrapping_add(choose)
                .wrapping_add(K[position])
                .wrapping_add(schedule[position]);
            let big0 = working[0].rotate_right(2)
                ^ working[0].rotate_right(13)
                ^ working[0].rotate_right(22);
            let majority =
                (working[0] & working[1]) ^ (working[0] & working[2]) ^ (working[1] & working[2]);
            let temp2 = big0.wrapping_add(majority);
            working = [
                temp1.wrapping_add(temp2),
                working[0],
                working[1],
                working[2],
                working[3].wrapping_add(temp1),
                working[4],
                working[5],
                working[6],
            ];
        }
        for (slot, value) in state.iter_mut().zip(working.iter()) {
            *slot = slot.wrapping_add(*value);
        }
    }
    state.iter().map(|word| format!("{word:08x}")).collect()
}

#[test]
fn every_archive_scans_byte_identically_to_the_retained_baseline() {
    let root = repo_root();
    let found = archives(&corpus());
    assert!(found.len() > 100, "only {} archives found", found.len());
    for (name, flags) in BASELINES {
        let expected = retained(name);
        assert_eq!(
            expected.len(),
            found.len(),
            "{name}: the corpus changed ({} retained, {} found)",
            expected.len(),
            found.len()
        );
        // 880 archives × 2 modes serialised takes minutes; eight workers keep the comparison honest and
        // the suite usable.
        let workers = 8usize;
        let mut observed: Vec<(String, String)> = Vec::new();
        // Relative names are computed before the workers start, so a worker only needs a slice.
        let targets: Vec<(PathBuf, String)> = found
            .iter()
            .map(|archive| {
                let relative = archive
                    .strip_prefix(&root)
                    .unwrap_or(archive)
                    .to_string_lossy()
                    .replace('\\', "/");
                (archive.clone(), relative)
            })
            .collect();
        std::thread::scope(|scope| {
            let handles: Vec<_> = targets
                .chunks(targets.len().div_ceil(workers).max(1))
                .map(|slice| {
                    scope.spawn(move || {
                        slice
                            .iter()
                            .map(|(archive, relative)| (digest(archive, flags), relative.clone()))
                            .collect::<Vec<_>>()
                    })
                })
                .collect();
            for handle in handles {
                observed.extend(handle.join().expect("a scan worker failed"));
            }
        });
        observed.sort();
        let drifted: Vec<&(String, String)> = expected
            .iter()
            .zip(observed.iter())
            .filter(|(want, got)| want != got)
            .map(|(_, got)| got)
            .collect();
        assert!(
            drifted.is_empty(),
            "{name}: {} archive(s) no longer scan identically, e.g. {:?}",
            drifted.len(),
            drifted.iter().take(3).collect::<Vec<_>>()
        );
    }
}
