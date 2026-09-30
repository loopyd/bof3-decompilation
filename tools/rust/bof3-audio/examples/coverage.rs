//! Local auxiliary corpus audit; original dispatch only, not PCM fidelity.
use bof3_audio::{
    digest::sha256_hex,
    machine::{bank, cues, effects, executable::Executable, firmware::Image},
    Result,
};
use emi_ex_v2::image::ArchiveImage;
use serde_json::{json, Value};
use std::{fs, path::Path};

fn files(root: &Path, paths: &mut Vec<std::path::PathBuf>) -> Result<()> {
    for entry in fs::read_dir(root)? {
        let entry = entry?;
        let kind = entry.file_type()?;
        if kind.is_dir() {
            files(&entry.path(), paths)?;
        } else if kind.is_file()
            && entry
                .path()
                .extension()
                .is_some_and(|v| v.eq_ignore_ascii_case("emi"))
        {
            paths.push(entry.path());
        }
    }
    Ok(())
}

fn audit(exe: &Executable, bios: &Image, archive: &ArchiveImage, aux: usize) -> Result<Value> {
    let header = (0..aux)
        .rfind(|&i| archive.entries()[i].file_type == 6)
        .ok_or("auxiliary entry has no preceding VH")?;
    let end = (header + 1..archive.entries().len())
        .find(|&i| archive.entries()[i].file_type == 6)
        .unwrap_or(archive.entries().len());
    let bodies: Vec<_> = (header + 1..end)
        .filter(|&i| archive.entries()[i].file_type == 7)
        .collect();
    if bodies.len() != 1 {
        return Err(format!("VH {header} owns {} bodies", bodies.len()).into());
    }
    let mut prepared = bank::prepare(
        exe,
        Image::from_bytes(bios.bytes().to_vec())?,
        archive,
        &bank::Options {
            header_entry: header,
            body_entry: bodies[0],
            sequence_entry: None,
            layout: None,
        },
        &mut |_, _| Ok(()),
    )?;
    let table = cues::stage(&mut prepared, archive, aux)?;
    prepared.execution.call(0x8015ce70, [0; 4], 100_000)?;
    let mut rows = Vec::new();
    // One fresh bank per table, rows in file order, no host status writes,
    // no output clock or status poll. This is not a gameplay schedule.
    for row in 0..table.records.len() {
        let mut notes = Vec::new();
        let before = prepared.execution.cpu.instructions();
        let result = cues::dispatch(&mut prepared, &table, row, &mut |e| {
            if e.cpu.pc() == effects::KEY_ON {
                notes.push([
                    e.cpu.register(4),
                    e.cpu.register(5),
                    e.cpu.register(6),
                    e.cpu.register(7),
                ]);
            }
            Ok(())
        });
        let executed = prepared.execution.cpu.instructions() - before;
        let failed = result.is_err();
        rows.push(json!({
            "row": row, "record": table.records[row], "instructions": executed,
            "notes": notes, "error": result.err().map(|e| e.to_string()),
        }));
        // A runtime failure may leave partially changed guest state. Stop this
        // table; validation failures execute nothing and permit later rows.
        if failed && executed != 0 {
            break;
        }
    }
    Ok(json!({
        "header": header, "body": bodies[0], "slot": table.game_bank_id,
        "address": table.address, "capacity": table.capacity,
        "expected_rows": table.records.len(), "rows": rows,
    }))
}

fn main() -> Result<()> {
    let args: Vec<_> = std::env::args_os().skip(1).collect();
    if args.len() != 3 {
        return Err("usage: coverage US_EXE BIOS CORPUS_ROOT".into());
    }
    let exe_bytes = fs::read(&args[0])?;
    let exe_sha256 = sha256_hex(&exe_bytes);
    let exe = Executable::from_bytes(exe_bytes)?;
    let bios = Image::from_bytes(fs::read(&args[1])?)?;
    let root = Path::new(&args[2]);
    let mut paths = Vec::new();
    files(root, &mut paths)?;
    paths.sort();
    let mut tables = Vec::new();
    let mut archives = Vec::new();
    for path in paths {
        let relative = path.strip_prefix(root)?.to_string_lossy();
        let bytes = fs::read(&path)?;
        let sha256 = sha256_hex(&bytes);
        let archive = match ArchiveImage::from_bytes(bytes) {
            Ok(archive) => archive,
            Err(error) => {
                archives
                    .push(json!({"path": relative, "sha256": sha256, "error": error.to_string()}));
                continue;
            }
        };
        for (aux, entry) in archive.entries().iter().enumerate() {
            if entry.file_type != 8 {
                continue;
            }
            let result = audit(&exe, &bios, &archive, aux);
            let (report, error) = match result {
                Ok(report) => (Some(report), None),
                Err(error) => (None, Some(error.to_string())),
            };
            tables.push(json!({"path": relative, "sha256": sha256, "aux": aux,
                "report": report, "error": error}));
            if tables.len() % 100 == 0 {
                eprintln!("audited {} auxiliary tables", tables.len());
            }
        }
    }
    let prepared = tables.iter().filter(|t| t["error"].is_null()).count();
    let mut visited = 0;
    let mut rejected = 0;
    let mut skipped = 0;
    for table in &tables {
        if let Some(rows) = table["report"]["rows"].as_array() {
            visited += rows.len();
            rejected += rows.iter().filter(|r| !r["error"].is_null()).count();
            skipped += table["report"]["expected_rows"].as_u64().unwrap() as usize - rows.len();
        }
    }
    let passed = !tables.is_empty()
        && prepared == tables.len()
        && rejected == 0
        && skipped == 0
        && archives.is_empty();
    println!(
        "{}",
        serde_json::to_string_pretty(&json!({
            "schema": "bof3.audio.cue-coverage/v1", "exe_sha256": exe_sha256,
            "bios_sha256": bios.sha256(), "tables": tables, "archive_errors": archives,
            "dispatch_coverage_passed": passed,
            "summary": {"prepared_tables": prepared, "visited_rows": visited,
                "rejected_rows": rejected, "skipped_rows": skipped},
            "scope": "fresh bank per table; rows in order; no output clock or status polls",
            "pcm_fidelity_validated": false, "pruning_authorized": false,
        }))?
    );
    if !passed {
        return Err("cue corpus coverage incomplete; see JSON report".into());
    }
    Ok(())
}
