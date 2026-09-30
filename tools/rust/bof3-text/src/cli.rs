//! Command line interface for the dialogue text tool.

use std::fs;
use std::io::Write;
use std::path::{Path, PathBuf};

use clap::{ArgAction, Parser, Subcommand};
use log::{debug, info};

use crate::extract;
use crate::logging;
use crate::models::{Error, Result, Source, TextClass};
use crate::pack;
use crate::query;
use crate::syntax::{self, Document};

/// BOF3 text banks (dialogue and battle/system): index, extract, validate, query, pack.
#[derive(Debug, Parser)]
#[command(name = "bof3-text", version, about, long_about = None)]
pub struct Cli {
    /// Payload inventory: families known not to be text, skipped before they are parsed
    #[arg(
        long,
        global = true,
        default_value = "out/text-payloads/inventory.json"
    )]
    pub payloads: PathBuf,
    /// Search without the payload exclusion (for comparison only)
    #[arg(long, global = true)]
    pub no_payloads: bool,
    /// Report every candidate the scanner forms, without the readability floor (comparison only)
    #[arg(long, global = true)]
    pub no_readable: bool,
    /// Vocabulary the readability rule checks its words against
    #[arg(
        long,
        global = true,
        default_value = "out/text-vocabulary/vocabulary.json"
    )]
    pub vocabulary: PathBuf,
    /// Report words without checking them against the vocabulary (comparison only)
    #[arg(long, global = true)]
    pub no_vocabulary: bool,

    /// Increase log verbosity: -v info, -vv debug, -vvv/-vvvv trace
    #[arg(short = 'v', long = "verbose", action = ArgAction::Count, global = true)]
    pub verbose: u8,
    /// Disable coloured output (also honours NO_COLOR)
    #[arg(short = 'n', long = "no-color", global = true)]
    pub no_color: bool,
    /// Emit JSON for command output and for log records
    #[arg(long = "json", global = true)]
    pub json: bool,
    #[command(subcommand)]
    pub mode: Mode,
}

#[derive(Debug, Subcommand)]
pub enum Mode {
    /// Prepare corpus search artifacts from extracted US archives
    Prepare {
        /// Directory containing extracted archives
        #[arg(long, default_value = "out/extracted/BIN")]
        root: PathBuf,
        /// Target configuration containing reviewed textbin records
        #[arg(long, default_value = "config/targets/emi")]
        targets: PathBuf,
        /// Parent directory for the text artifact directories
        #[arg(long, default_value = "out")]
        out_dir: PathBuf,
    },
    /// Latch candidate raw-text runs in every subfile (heuristic leads)
    Scan {
        /// Archive to scan
        archive: PathBuf,
        /// Minimum decoded characters that can latch a run
        #[arg(long, default_value_t = crate::lexagraph::MIN_RUN)]
        min_run: usize,
        /// Restrict to subfiles of one text class; other subfiles are the heuristic ones
        #[arg(long, value_parser = class_value_parser())]
        class: Option<String>,
        /// Restrict to one TOC entry index
        #[arg(short = 'e', long)]
        entry: Option<usize>,
        /// Classified-window registry; a window is a barrier latching must leave alone
        #[arg(long, default_value = "out/text-windows/registry.json")]
        windows: PathBuf,
        /// Latch without the classified-window check (for comparison only)
        #[arg(long)]
        no_windows: bool,
    },
    /// List the payloads the inventory identifies, per archive, for the registry generator
    Payloads {
        /// Directory of extracted archives
        #[arg(long, default_value = "out/extracted/BIN")]
        root: PathBuf,
    },
    /// Build the classified-window registry: the byte ranges an existing owner already accounts for
    Windows {
        /// Directory of extracted archives
        #[arg(long, default_value = "out/extracted/BIN")]
        root: PathBuf,
        /// Target configuration root holding the reviewed records
        #[arg(long, default_value = "config/targets/emi")]
        targets: PathBuf,
        /// Registry to write
        #[arg(long, default_value = "out/text-windows/registry.json")]
        out: PathBuf,
        /// Print the summary as JSON instead of a table
        #[arg(long)]
        json: bool,
    },
    /// Derive the readability vocabulary from the verified rows (the artifact the rule reads)
    Vocabulary {
        /// Index holding the verified rows the words come from
        #[arg(long, default_value = "out/text-index/index.json")]
        index: PathBuf,
        /// Where to write the vocabulary
        #[arg(long, default_value = "out/text-vocabulary/vocabulary.json")]
        out: PathBuf,
    },
    /// Apply the readability rule to text and report the verdicts (a calibration diagnostic)
    Readability {
        /// Text to judge, repeatable. With none, every verified row's reading from the index.
        #[arg(long = "text")]
        text: Vec<String>,
        /// Index holding the verified rows to judge when no text is given
        #[arg(long, default_value = "out/text-index/index.json")]
        index: PathBuf,
    },
    /// Map text-versus-data roles across every archive (read-only report, no config edits)
    Map {
        /// Directory walked recursively for `*.EMI` archives
        #[arg(long, default_value = "out/extracted/BIN")]
        root: PathBuf,
        /// Report destination
        #[arg(long, default_value = "out/text-format/segment-map.json")]
        out: PathBuf,
        /// Classified-window registry; a window is a barrier latching must leave alone
        #[arg(long, default_value = "out/text-windows/registry.json")]
        windows: PathBuf,
        /// Latch without the classified-window check (for comparison only)
        #[arg(long)]
        no_windows: bool,
    },
    /// Round-trip every text subfile of a known class across the corpus
    Verify {
        /// Directory walked recursively for `*.EMI` archives
        #[arg(long, default_value = "out/extracted/BIN")]
        root: PathBuf,
    },
    /// Probe every subfile of an archive for text-bank structure, whatever its class
    Probe {
        /// Archive to probe
        archive: PathBuf,
        /// Also try an unedited round trip and report whether it reproduces the payload
        #[arg(long)]
        round_trip: bool,
    },
    /// Search the retained corpus text index over every archive
    Search {
        /// Index artifact to query
        #[arg(long, default_value = "out/text-index/index.json")]
        index: PathBuf,
        /// Directory the index was built from, for the freshness check
        #[arg(long, default_value = "out/extracted/BIN")]
        root: PathBuf,
        /// Text to find. Control commands and their operands are transparent, so a
        /// phrase may span a command and raw command bytes are never matched
        #[arg(long)]
        grep: Option<String>,
        /// Match ASCII letters case-insensitively
        #[arg(short = 'i', long)]
        ignore_case: bool,
        /// Restrict to one text class
        #[arg(long, value_parser = class_value_parser())]
        class: Option<String>,
        /// Restrict to one Unicode script, e.g. `Latin`
        #[arg(long)]
        script: Option<String>,
        /// Restrict to one language code reported as reliable, e.g. `eng`
        #[arg(long)]
        language: Option<String>,
        /// Restrict to one TOC entry index
        #[arg(short = 'e', long)]
        entry: Option<usize>,
        /// Restrict to archives whose relative path contains this text
        #[arg(long)]
        archive: Option<String>,
        /// Report at most this many hits; 0 reports every hit
        #[arg(long, default_value_t = 0)]
        limit: usize,
        /// Report the match start instead of the start of the containing string
        #[arg(long)]
        match_offset: bool,
        /// Classified-window registry, used when the index has to be regenerated
        #[arg(long, default_value = "out/text-windows/registry.json")]
        windows: PathBuf,
        /// Regenerate without the classified-window check (for comparison only)
        #[arg(long)]
        no_windows: bool,
        /// Resolve every selected instance against its archive and prove it reproduces;
        /// an integrity check for the index, not a text search
        #[arg(long)]
        verify_entries: bool,
    },
    /// Build or check the retained corpus text index over many archives
    BuildIndex {
        /// Directory walked recursively for `*.EMI` archives
        #[arg(long, default_value = "out/extracted/BIN")]
        root: PathBuf,
        /// Index artifact to write, or to check when `--check` is given
        #[arg(long, default_value = "out/text-index/index.json")]
        out: PathBuf,
        /// Only report whether the existing index still matches the inputs; never write
        #[arg(long)]
        check: bool,
        /// Rebuild even when the index is already fresh
        #[arg(long)]
        force: bool,
        /// Classified-window registry; a window is a barrier latching must leave alone
        #[arg(long, default_value = "out/text-windows/registry.json")]
        windows: PathBuf,
        /// Latch without the classified-window check (for comparison only)
        #[arg(long)]
        no_windows: bool,
    },
    /// List every text subfile of an EMI archive
    Index {
        /// Archive to inspect
        archive: PathBuf,
        /// Restrict to one text class
        #[arg(long, value_parser = class_value_parser())]
        class: Option<String>,
        /// Restrict to one TOC entry index
        #[arg(short = 'e', long)]
        entry: Option<usize>,
    },
    /// Write text objects as JSON: one archive, many entries, or all of them
    Extract {
        /// Archive to read
        archive: PathBuf,
        /// Destination file (single entry) or directory (many)
        #[arg(short = 'o', long = "output")]
        output: PathBuf,
        /// Entry index, comma-separated list, or `all`
        #[arg(short = 'e', long = "entry", default_value = "all")]
        entry: String,
        /// Restrict to one text class
        #[arg(long, value_parser = class_value_parser())]
        class: Option<String>,
        /// Raw-text instance extent `ENTRY:OFFSET+LENGTH` (heuristic class only)
        #[arg(long)]
        latch: Option<String>,
    },
    /// Check one JSON text-object document
    Validate {
        /// Document to check
        document: PathBuf,
        /// Print one summary line per non-empty row
        #[arg(long)]
        commands: bool,
    },
    /// List or search the rows of a text subfile
    Query {
        /// Archive to read
        archive: PathBuf,
        /// Entry index, comma-separated list, or `all`
        #[arg(short = 'e', long = "entry", default_value = "all")]
        entry: String,
        /// Restrict to one text class
        #[arg(long, value_parser = class_value_parser())]
        class: Option<String>,
        /// Text to find. Control commands and their operands are transparent, so a
        /// phrase may span a command and raw command bytes are never matched
        #[arg(long)]
        grep: Option<String>,
        /// Match ASCII letters case-insensitively
        #[arg(short = 'i', long)]
        ignore_case: bool,
        /// Print a command-usage histogram instead of rows
        #[arg(long)]
        commands: bool,
        /// Report the match start instead of the start of the containing string
        #[arg(long)]
        match_offset: bool,
        /// Classified-window registry; a window is a barrier latching must leave alone
        #[arg(long, default_value = "out/text-windows/registry.json")]
        windows: PathBuf,
        /// Latch without the classified-window check (for comparison only)
        #[arg(long)]
        no_windows: bool,
    },
    /// Repack edited text objects into a separate EMI output
    Pack {
        /// Original archive the documents came from
        #[arg(long)]
        original: PathBuf,
        /// Edited document (single entry) or directory of `<entry>.json` files
        #[arg(long)]
        text: PathBuf,
        /// Destination archive (created; never overwritten)
        #[arg(short = 'o', long = "output")]
        output: PathBuf,
        /// Entry index, comma-separated list, or `all`
        #[arg(short = 'e', long = "entry", default_value = "all")]
        entry: String,
        /// Restrict to one text class
        #[arg(long, value_parser = class_value_parser())]
        class: Option<String>,
        /// Raw-text instance extent `ENTRY:OFFSET+LENGTH` (heuristic class only)
        #[arg(long)]
        latch: Option<String>,
    },
}

/// Which entries a command acts on.
#[derive(Debug)]
enum Selection {
    One(usize),
    Many(Vec<usize>),
}

/// Argument vector with the `-nc` spelling of `--no-color` normalised.
pub fn normalise_args(args: Vec<std::ffi::OsString>) -> Vec<std::ffi::OsString> {
    args.into_iter()
        .map(|arg| {
            if arg == "-nc" {
                std::ffi::OsString::from("--no-color")
            } else {
                arg
            }
        })
        .collect()
}

pub fn main(argv: Vec<std::ffi::OsString>) -> i32 {
    let cli = match Cli::try_parse_from(normalise_args(argv)) {
        Ok(cli) => cli,
        Err(error) => {
            let _ = error.print();
            return if error.use_stderr() { 2 } else { 0 };
        }
    };
    logging::init(cli.verbose, cli.no_color, cli.json);
    match run(&cli) {
        Ok(()) => 0,
        Err(error) => {
            eprintln!("{} {error}", logging::failure("error:"));
            1
        }
    }
}

fn run(cli: &Cli) -> Result<()> {
    match &cli.mode {
        Mode::Prepare {
            root,
            targets,
            out_dir,
        } => run_prepare(cli, root, targets, out_dir),
        Mode::Payloads { root } => run_payloads(cli, root),
        Mode::Windows {
            root,
            targets,
            out,
            json,
        } => run_windows(root, targets, out, &cli.payloads, *json, false),
        Mode::Vocabulary { index, out } => run_vocabulary(index, out),
        Mode::Readability { text, index } => run_readability(cli, text, index),
        Mode::Map {
            root,
            out,
            windows,
            no_windows,
        } => run_map(
            cli,
            root,
            out,
            &window_index(windows, *no_windows)?,
            &known_payloads(&cli.payloads, cli.no_payloads)?,
            &vocabulary(&cli.vocabulary, cli.no_vocabulary)?,
        ),
        Mode::Verify { root } => run_verify(cli, root),
        Mode::Probe {
            archive,
            round_trip,
        } => run_probe(cli, archive, *round_trip),
        Mode::Search {
            index,
            root,
            grep,
            ignore_case,
            class,
            script,
            language,
            entry,
            archive,
            limit,
            match_offset,
            windows,
            no_windows,
            verify_entries,
        } => run_search(
            cli,
            index,
            root,
            &SearchQuery {
                needle: grep.as_deref(),
                ignore_case: *ignore_case,
                class: resolve_class(class.as_deref())?,
                script: script.as_deref(),
                language: language.as_deref(),
                entry: *entry,
                archive: archive.as_deref(),
                limit: *limit,
                match_offset: *match_offset,
                verify_entries: *verify_entries,
            },
            &window_index(windows, *no_windows)?,
            &known_payloads(&cli.payloads, cli.no_payloads)?,
            &vocabulary(&cli.vocabulary, cli.no_vocabulary)?,
        ),
        Mode::BuildIndex {
            root,
            out,
            check,
            force,
            windows,
            no_windows,
        } => {
            let windows = window_index(windows, *no_windows)?;
            let payloads = known_payloads(&cli.payloads, cli.no_payloads)?;
            let vocabulary = vocabulary(&cli.vocabulary, cli.no_vocabulary)?;
            let filters = crate::filters::compose(
                &windows,
                &payloads,
                &vocabulary,
                crate::filters::Readability::of_opt_out(cli.no_readable),
            );
            run_build_index(cli, root, out, *check, *force, &filters)
        }
        Mode::Index {
            archive,
            class,
            entry,
        } => run_index(cli, archive, class.as_deref(), *entry),
        Mode::Scan {
            archive,
            min_run,
            class,
            entry,
            windows,
            no_windows,
        } => {
            let windows = window_index(windows, *no_windows)?;
            let payloads = known_payloads(&cli.payloads, cli.no_payloads)?;
            let vocabulary = vocabulary(&cli.vocabulary, cli.no_vocabulary)?;
            let filters = crate::filters::compose(
                &windows,
                &payloads,
                &vocabulary,
                crate::filters::Readability::of_opt_out(cli.no_readable),
            );
            run_scan(cli, archive, *min_run, class.as_deref(), *entry, &filters)
        }
        Mode::Extract {
            archive,
            output,
            entry,
            class,
            latch,
        } => run_extract(
            cli,
            archive,
            output,
            entry,
            class.as_deref(),
            latch.as_deref(),
        ),
        Mode::Validate { document, commands } => run_validate(cli, document, *commands),
        Mode::Query {
            archive,
            entry,
            grep,
            ignore_case,
            commands,
            class,
            match_offset,
            windows,
            no_windows,
        } => run_query(
            cli,
            archive,
            &QueryOptions {
                entry_spec: entry,
                needle: grep.as_deref(),
                commands: *commands,
                class_filter: class.as_deref(),
                ignore_case: *ignore_case,
                match_offset: *match_offset,
            },
            &window_index(windows, *no_windows)?,
            &known_payloads(&cli.payloads, cli.no_payloads)?,
            &vocabulary(&cli.vocabulary, cli.no_vocabulary)?,
        ),
        Mode::Pack {
            original,
            text,
            output,
            entry,
            class,
            latch,
        } => run_pack(
            cli,
            original,
            text,
            output,
            entry,
            class.as_deref(),
            latch.as_deref(),
        ),
    }
}

/// A `--latch ENTRY:OFFSET+LENGTH` selector.
fn parse_latch(spec: &str) -> Result<(usize, syntax::Extent)> {
    let (entry, rest) = spec
        .split_once(':')
        .ok_or_else(|| Error::Invalid(format!("latch {spec:?} must be ENTRY:OFFSET+LENGTH")))?;
    let (offset, length) = rest
        .split_once('+')
        .ok_or_else(|| Error::Invalid(format!("latch {spec:?} must be ENTRY:OFFSET+LENGTH")))?;
    let number = |text: &str, what: &str| -> Result<usize> {
        let value = text.trim();
        let parsed = match value.strip_prefix("0x") {
            Some(hex) => usize::from_str_radix(hex, 16),
            None => value.parse::<usize>(),
        };
        parsed.map_err(|_| Error::Invalid(format!("latch {what} {value:?} is not a number")))
    };
    Ok((
        number(entry, "entry")?,
        syntax::Extent {
            offset: number(offset, "offset")?,
            length: number(length, "length")?,
        },
    ))
}

/// Bound a latch extent inside an archive, refusing precisely when it cannot be.
fn latch_span(
    data: &[u8],
    index: usize,
    extent: syntax::Extent,
) -> Result<(crate::models::Entry, &[u8])> {
    if extent.length == 0 {
        return Err(Error::Invalid(
            "latch length must be at least one byte".into(),
        ));
    }
    let entries = crate::lexagraph::entries(data)?;
    let (entry, payload) = entries
        .into_iter()
        .find(|(entry, _)| entry.index == index)
        .ok_or_else(|| {
            Error::Invalid(format!(
                "archive has no subfile #{index}; run `index` or `scan` to list them"
            ))
        })?;
    let end = extent
        .offset
        .checked_add(extent.length)
        .ok_or_else(|| Error::Invalid("latch extent overflows".into()))?;
    if end > payload.len() {
        let size = payload.len();
        return Err(Error::Invalid(format!(
            "latch #{index}:{:#x}+{} runs past subfile #{index} size {size:#x}; the instance cannot be bounded",
            extent.offset, extent.length
        )));
    }
    Ok((entry, &payload[extent.offset..end]))
}

/// Extract one bounded raw-text instance as a JSON document.
fn extract_latch(cli: &Cli, archive: &Path, output: &Path, data: &[u8], spec: &str) -> Result<()> {
    let (index, extent) = parse_latch(spec)?;
    let (entry, span) = latch_span(data, index, extent)?;
    let text = crate::command::deserialize(span);
    let source = Source::new(
        archive_label(archive),
        Some(entry.index),
        entry.ram_ptr,
        TextClass::Heuristic,
    );
    let document = syntax::Document {
        version: crate::models::DOCUMENT_VERSION,
        source,
        fingerprint: Some(syntax::span_fingerprint(span)),
        banks: vec![vec![syntax::DocumentRow {
            number: 1,
            text: Some(text),
        }]],
        latch: Some(extent),
    };
    syntax::check_row(document.banks[0][0].text.as_deref().unwrap_or_default()).map_err(
        |error| Error::Invalid(format!("latched instance is not representable: {error}")),
    )?;
    let body = syntax::render_json(&document)?;
    write_output(output, body.as_bytes(), archive)?;
    info!(
        "latched {} ({} bytes) -> {}",
        document.source,
        extent.length,
        output.display()
    );
    if !cli.json {
        println!(
            "{} raw-text instance {} ({} byte(s)) -> {}",
            logging::success("extracted"),
            document.source,
            extent.length,
            output.display()
        );
    }
    Ok(())
}

/// Pack a raw-text instance back in place. No allocation is proven for raw text,
/// so an edit must occupy exactly the bytes the instance came from.
fn pack_latch(
    cli: &Cli,
    original: &Path,
    document_path: &Path,
    output: &Path,
    data: &[u8],
    document: syntax::Document,
    latch_spec: Option<&str>,
) -> Result<()> {
    let Some(extent) = document.latch else {
        return invalid(format!(
            "--latch was given but document {} is not a raw-text instance; re-extract it with --latch",
            document_path.display()
        ));
    };
    let Some(index) = document.source.entry else {
        return invalid(format!(
            "raw-text document {} carries no subfile number in its source",
            document_path.display()
        ));
    };
    if let Some(spec) = latch_spec {
        let (stated_index, stated) = parse_latch(spec)?;
        if stated != extent || stated_index != index {
            return invalid(format!(
                "--latch {spec} names subfile #{stated_index} offset {:#x} length {}, but document {} declares subfile #{index} offset {:#x} length {}",
                stated.offset,
                stated.length,
                document_path.display(),
                extent.offset,
                extent.length
            ));
        }
    }
    let (entry, span) = latch_span(data, index, extent)?;
    if document.source.class != TextClass::Heuristic {
        return invalid(format!(
            "document {} is {} text, not a raw-text instance",
            document_path.display(),
            document.source.class
        ));
    }
    if document.source.address != entry.ram_ptr {
        return invalid(format!(
            "document {} came from address {:#010x} but subfile #{index} is at {:#010x}",
            document_path.display(),
            document.source.address,
            entry.ram_ptr
        ));
    }
    if let Some(stated) = &document.fingerprint {
        let actual = syntax::span_fingerprint(span);
        if *stated != actual {
            return invalid(format!(
                "document fingerprint {stated} does not match this instance's {actual}; re-extract before editing"
            ));
        }
    }
    let text = document
        .banks
        .first()
        .and_then(|bank| bank.first())
        .and_then(|row| row.text.as_deref())
        .unwrap_or_default();
    let encoded = crate::command::serialize(text)?;
    if encoded.len() != extent.length {
        return invalid(format!(
            "raw-text instance {} has no proven allocation: {} byte(s) available but the edit encodes to {}; only a same-length edit can be packed",
            document.source,
            extent.length,
            encoded.len()
        ));
    }
    let mut packed = data.to_vec();
    let start = entry.offset + extent.offset;
    packed[start..start + extent.length].copy_from_slice(&encoded);
    write_output(output, &packed, original)?;
    info!("patched {} ({} byte(s))", document.source, extent.length);
    if cli.json {
        println!(
            "{}",
            serde_json::to_string_pretty(&serde_json::json!({
                "source": document.source.to_string(),
                "extent": {"offset": extent.offset, "length": extent.length},
                "output": archive_label(output),
                "bytes": packed.len(),
            }))
            .map_err(|error| Error::Invalid(error.to_string()))?
        );
    } else {
        println!(
            "{} raw-text instance {} -> {} ({} bytes, unchanged size)",
            logging::success("packed"),
            document.source,
            output.display(),
            packed.len()
        );
    }
    Ok(())
}

/// Query raw-text instances: latches are heuristic leads, reported as such.
fn query_latches(
    cli: &Cli,
    archive: &Path,
    data: &[u8],
    options: &QueryOptions<'_>,
    windows: &crate::windows::WindowIndex,
    payloads: &crate::filters::KnownPayload,
    vocabulary: &crate::filters::Vocabulary,
) -> Result<()> {
    let mut hits = Vec::new();
    let mut total = 0usize;
    let mut tally = crate::lexagraph::Tally::default();
    let key = window_key(windows, &archive_label(archive))?
        .unwrap_or("")
        .to_string();
    let mut preceding: Vec<&[u8]> = Vec::new();
    for (entry, payload) in crate::lexagraph::entries(data)? {
        let input = crate::models::PreFilterInput {
            archive: &key,
            entry: entry.index,
            payload,
            class: TextClass::from_load_argument(entry.ram_ptr),
            // The container's earlier payloads, so the pairing rule works on this path too.
            preceding: &preceding,
        };
        let filters = crate::filters::compose(
            windows,
            payloads,
            vocabulary,
            crate::filters::Readability::of_opt_out(cli.no_readable),
        );
        let (latches, subfile) = filters.scan_bytes(&input, crate::lexagraph::MIN_RUN);
        preceding.push(payload);
        tally.add(&subfile);
        total += latches.len();
        // Abutting windows of a run the scanner had to split are matched as one group, exactly as
        // corpus search matches them, so a phrase across the split is not lost here.
        let mut groups: Vec<Vec<crate::lexagraph::Latch>> = Vec::new();
        for latch in latches {
            let continues = groups.last().is_some_and(|group| {
                let last = group.last().expect("non-empty");
                last.offset + last.length == latch.offset
            });
            if continues {
                groups.last_mut().expect("checked").push(latch);
            } else {
                groups.push(vec![latch]);
            }
        }
        for group in groups {
            let first = &group[0];
            let span: usize = group.iter().map(|latch| latch.length).sum();
            let Some(needle) = options.needle else {
                hits.push(serde_json::json!({
                    "source": Source::new(
                        archive_label(archive),
                        Some(entry.index),
                        entry.ram_ptr,
                        TextClass::Heuristic,
                    ).to_string(),
                    "offset": first.offset,
                    "length": span,
                    "instances": group.len(),
                    "ratio": first.ratio,
                    "text": crate::command::deserialize(&payload[first.offset..first.offset + span]),
                }));
                continue;
            };
            let spans: Vec<(usize, usize)> = group
                .iter()
                .map(|latch| (latch.offset, latch.length))
                .collect();
            // (start, exclusive end) from the group matcher; the published length is the span
            let Some((start, end)) =
                crate::search::find_in_group(payload, &spans, needle, options.ignore_case)
            else {
                continue;
            };
            // The string begins where a walk **back** from the match through the group's windows
            // reaches, not where the group's first window happens to be; its end is the group's own
            // recorded end, so the whole recorded string is reported.
            let group_end = first.offset + span;
            let begin =
                crate::lexagraph::walk_back_to_chain_start(&spans, start).unwrap_or(first.offset);
            let anchored = (begin, group_end - begin);
            hits.push(serde_json::json!({
                "source": Source::new(
                    archive_label(archive),
                    Some(entry.index),
                    entry.ram_ptr,
                    TextClass::Heuristic,
                ).to_string(),
                "offset": first.offset,
                "length": span,
                "string_offset": entry.offset + anchored.0,
                "string_length": anchored.1,
                "instances": group.len(),
                "ratio": first.ratio,
                "text": crate::command::deserialize(&payload[first.offset..first.offset + span]),
                "reading": "command-aware",
                "match_offset": entry.offset + start,
                "match_length": end - start,
                "anchor": if options.match_offset { "match" } else { "string" },
                "reported_offset": if options.match_offset {
                    entry.offset + start
                } else {
                    entry.offset + anchored.0
                },
                "reported_length": if options.match_offset { end - start } else { anchored.1 },
            }));
        }
    }
    info!(
        "{} of {total} heuristic latch(es) matched in {}",
        hits.len(),
        archive.display()
    );
    if cli.json {
        println!(
            "{}",
            serde_json::to_string_pretty(&serde_json::json!({
                "windows": tally,
                "archive": archive_label(archive),
                "heuristic": true,
                "note": "matches are heuristic leads, not consumer-verified text",
                "scanned": total,
                "matches": hits,
            }))
            .map_err(|error| Error::Invalid(error.to_string()))?
        );
        return Ok(());
    }
    println!(
        "{} {}: {} of {total} heuristic latch(es) matched",
        logging::heading("query"),
        archive.display(),
        hits.len()
    );
    for hit in hits.iter().take(20) {
        let (anchor, span) = if options.match_offset {
            (
                hit["match_offset"]
                    .as_u64()
                    .or_else(|| hit["string_offset"].as_u64())
                    .unwrap_or(0),
                hit["match_length"].as_u64().unwrap_or(0),
            )
        } else {
            (
                hit["string_offset"].as_u64().unwrap_or(0),
                hit["string_length"].as_u64().unwrap_or(0),
            )
        };
        println!("  {} +{anchor:#x} {span}B {}", hit["source"], hit["text"]);
    }
    Ok(())
}

fn read(path: &Path, label: &str) -> Result<Vec<u8>> {
    fs::read(path).map_err(|error| Error::Invalid(format!("{label} {}: {error}", path.display())))
}

fn read_json_document(path: &Path) -> Result<Document> {
    let bytes = read(path, "document")?;
    let text = String::from_utf8(bytes)
        .map_err(|_| Error::Invalid(format!("document {} is not valid UTF-8", path.display())))?;
    debug!("loading {}", path.display());
    syntax::parse_json(&text)
}

/// Publish an output file without ever replacing an existing path.
fn write_output(path: &Path, bytes: &[u8], protected: &Path) -> Result<()> {
    let over_source = path == protected
        || matches!(
            (path.canonicalize(), protected.canonicalize()),
            (Ok(destination), Ok(source)) if destination == source
        );
    if over_source {
        return invalid("refusing to write over the source archive");
    }
    // Create the destination's directory, exactly as the index and map modes do, so a
    // documented example such as `-o out/text/AREA000.json` works from a clean tree instead of
    // failing on a missing parent. This does not weaken the create-new guarantee below.
    if let Some(parent) = path.parent() {
        if !parent.as_os_str().is_empty() {
            fs::create_dir_all(parent)
                .map_err(|error| Error::Invalid(format!("{}: {error}", parent.display())))?;
        }
    }
    let mut file = fs::OpenOptions::new()
        .write(true)
        .create_new(true)
        .open(path)
        .map_err(|error| match error.kind() {
            std::io::ErrorKind::AlreadyExists => Error::Invalid(format!(
                "refusing to overwrite existing output: {} (choose a new path)",
                path.display()
            )),
            _ => Error::Invalid(format!("{}: {error}", path.display())),
        })?;
    file.write_all(bytes)
        .map_err(|error| Error::Invalid(format!("{}: {error}", path.display())))
}

/// Accepted `--class` values, derived from the enum so that the accepted set, the help text
/// and the parser can never disagree.
fn class_value_parser() -> clap::builder::PossibleValuesParser {
    clap::builder::PossibleValuesParser::new(TextClass::ALL.iter().map(|class| class.name()))
}

/// Resolve an optional `--class` selector.
fn resolve_class(value: Option<&str>) -> Result<Option<TextClass>> {
    match value {
        None => Ok(None),
        Some(text) => TextClass::parse(text).map(Some).ok_or_else(|| {
            Error::Invalid(format!(
                "unknown text class {text:?}; expected dialogue, battle or heuristic"
            ))
        }),
    }
}

fn invalid<T>(message: impl Into<String>) -> Result<T> {
    Err(Error::Invalid(message.into()))
}

fn archive_label(path: &Path) -> String {
    path.to_string_lossy().replace('\\', "/")
}

fn parse_selection(spec: &str) -> Result<Option<Selection>> {
    let trimmed = spec.trim();
    if trimmed.eq_ignore_ascii_case("all") || trimmed.is_empty() {
        return Ok(None);
    }
    let mut indices = Vec::new();
    for part in trimmed.split(',') {
        let part = part.trim();
        let index: usize = part
            .parse()
            .map_err(|_| Error::Invalid(format!("invalid --entry value {part:?}")))?;
        indices.push(index);
    }
    if indices.is_empty() {
        return invalid("--entry selected no entries");
    }
    Ok(Some(if indices.len() == 1 {
        Selection::One(indices[0])
    } else {
        Selection::Many(indices)
    }))
}

/// Resolve the selection against an archive into `(entry, class, payload)`,
/// optionally filtered to one text class.
fn select<'a>(
    data: &'a [u8],
    spec: &str,
    class_filter: Option<TextClass>,
) -> Result<Vec<(crate::models::Entry, TextClass, &'a [u8])>> {
    let selected = select_class(data, spec)?;
    if let Some(wanted) = class_filter {
        let filtered: Vec<_> = selected
            .into_iter()
            .filter(|(_, class, _)| *class == wanted)
            .collect();
        if filtered.is_empty() {
            return invalid(format!(
                "archive has no {wanted} text subfile (banked at {}); run `index` to see the classes present",
                wanted
                    .load_address()
                    .map(|a| format!("{a:#010x}"))
                    .unwrap_or_else(|| "no bank".into())
            ));
        }
        return Ok(filtered);
    }
    Ok(selected)
}

/// Resolve the entry selection without a class filter.
fn select_class<'a>(
    data: &'a [u8],
    spec: &str,
) -> Result<Vec<(crate::models::Entry, TextClass, &'a [u8])>> {
    match parse_selection(spec)? {
        None => extract::all_entries(data),
        Some(Selection::One(index)) => {
            let (entry, class, payload) = extract::select_entry(data, Some(index))?;
            Ok(vec![(entry, class, payload)])
        }
        Some(Selection::Many(indices)) => {
            let mut selected = Vec::new();
            for index in indices {
                let (entry, class, payload) = extract::select_entry(data, Some(index))?;
                selected.push((entry, class, payload));
            }
            Ok(selected)
        }
    }
}

/// The `source` property for one selected subfile.
fn source_for(
    archive: &Path,
    entry: crate::models::Entry,
    class: crate::models::TextClass,
    multiple: bool,
) -> Source {
    Source::new(
        archive_label(archive),
        if multiple { Some(entry.index) } else { None },
        entry.ram_ptr,
        class,
    )
}

/// Derive the vocabulary the readability rule attests words against, from the verified rows.
///
/// The tokeniser and the document shape are the ones the harness's Python builder used, byte for byte,
/// so moving the derivation into the crate changes the artifact not at all: **ASCII words of three or
/// more letters, lowercased**, from the dialogue and battle rows' readings. Documentation is deliberately
/// not scanned — the project's own prose contains identifiers that let byte noise pass attestation — and
/// the sources block records that choice.
fn run_prepare(cli: &Cli, root: &Path, targets: &Path, out_dir: &Path) -> Result<()> {
    if cli.no_payloads
        || cli.no_readable
        || cli.no_vocabulary
        || cli.payloads != Path::new("out/text-payloads/inventory.json")
        || cli.vocabulary != Path::new("out/text-vocabulary/vocabulary.json")
    {
        return invalid(
            "prepare uses all filters and its --out-dir artifact paths; use search or build-index for filter comparisons",
        );
    }
    let root = anchored(root);
    let archives = crate::textindex::archives(&root)?;
    if archives.is_empty() {
        return invalid(
            "no extracted EMI archives found; supply --root with the extracted US corpus",
        );
    }
    // Derive vocabulary only from structurally verified bank rows. No heuristic
    // scan or existing generated vocabulary/index participates in this bootstrap.
    let mut readings = Vec::new();
    for archive in &archives {
        let data = read(archive, "archive")?;
        for (entry, payload) in crate::lexagraph::entries(&data)? {
            if TextClass::from_load_argument(entry.ram_ptr).is_none() {
                continue;
            }
            let section = extract::parse_section(payload).map_err(|error| {
                Error::Invalid(format!("{}#{}: {error}", archive.display(), entry.index))
            })?;
            for bank in section.banks {
                for row in bank.rows {
                    if row.span.is_some() {
                        readings.push(crate::search::readings(&row.encoded).0.text);
                    }
                }
            }
        }
    }
    let vocabulary_path = out_dir.join("text-vocabulary/vocabulary.json");
    write_vocabulary(
        readings.iter().map(String::as_str),
        &root,
        &vocabulary_path,
        false,
    )?;
    let payload_path = out_dir.join("text-payloads/inventory.json");
    if let Some(parent) = payload_path.parent() {
        fs::create_dir_all(parent).map_err(|error| Error::Invalid(error.to_string()))?;
    }
    match fs::OpenOptions::new()
        .write(true)
        .create_new(true)
        .open(&payload_path)
    {
        Ok(mut file) => file
            .write_all(include_bytes!("../data/payload-inventory.json"))
            .map_err(|error| Error::Invalid(error.to_string()))?,
        Err(error) if error.kind() == std::io::ErrorKind::AlreadyExists => {}
        Err(error) => return invalid(format!("{}: {error}", payload_path.display())),
    }
    let windows_path = out_dir.join("text-windows/registry.json");
    run_windows(&root, targets, &windows_path, &payload_path, false, true)?;
    let vocabulary = vocabulary(&vocabulary_path, false)?;
    let payloads = known_payloads(&payload_path, false)?;
    let windows = window_index(&windows_path, false)?;
    let index_path = out_dir.join("text-index/index.json");
    let (index, regenerated) = fresh_index(
        &index_path,
        &root,
        &windows,
        &payloads,
        &vocabulary,
        crate::filters::Readability::On,
    )?;
    if !index.skipped.is_empty() {
        return invalid(format!(
            "text preparation skipped subfiles: {}",
            index.skipped.join("; ")
        ));
    }
    if cli.json {
        println!(
            "{}",
            serde_json::json!({
                "root": root, "index": index_path, "windows": windows_path,
                "payloads": payload_path, "vocabulary": vocabulary_path,
                "archives": index.inputs.archives, "instances": index.instances.len(),
                "regenerated": regenerated,
            })
        );
    } else {
        println!(
            "prepared {}: {} instance(s) over {} archive(s)",
            index_path.display(),
            index.instances.len(),
            index.inputs.archives
        );
    }
    Ok(())
}

fn run_vocabulary(index_path: &Path, out: &Path) -> Result<()> {
    let index = crate::textindex::load(index_path)?;
    write_vocabulary(
        index
            .instances
            .iter()
            .filter(|instance| matches!(instance.class, TextClass::Dialogue | TextClass::Battle))
            .map(|instance| instance.reading.as_str()),
        index_path,
        out,
        true,
    )
}

fn write_vocabulary<'a>(
    readings: impl IntoIterator<Item = &'a str>,
    source: &Path,
    out: &Path,
    report: bool,
) -> Result<()> {
    let mut words: std::collections::BTreeSet<String> = std::collections::BTreeSet::new();
    let mut verified = 0usize;
    for reading in readings {
        verified += 1;
        let mut current = String::new();
        for character in reading.chars() {
            if character.is_ascii_alphabetic() {
                current.push(character.to_ascii_lowercase());
            } else {
                if current.len() >= 3 {
                    words.insert(std::mem::take(&mut current));
                } else {
                    current.clear();
                }
            }
        }
        if current.len() >= 3 {
            words.insert(current);
        }
    }
    if verified == 0 {
        return Err(Error::Invalid(format!(
            "{}: no verified rows to derive a vocabulary from; build the index first",
            source.display()
        )));
    }
    if words.is_empty() {
        return Err(Error::Invalid(format!(
            "{}: the verified rows yielded no words; refusing an empty vocabulary",
            source.display()
        )));
    }
    // Written in the shape the artifact already had (one-space indent, keys in order), so the file stays
    // byte-identical and the rule's attestations cannot shift underneath it.
    let mut text = String::from("{\n \"schema\": \"bof3.text-vocabulary/v1\",\n \"sources\": {\n");
    text.push_str(&format!("  \"bank_words\": {},\n", words.len()));
    text.push_str("  \"document_words_only\": 0,\n  \"documents\": 0,\n");
    text.push_str(&format!(
        "  \"verified_instances\": {verified}\n }},\n \"words\": [\n"
    ));
    let last = words.len() - 1;
    for (position, word) in words.iter().enumerate() {
        let comma = if position == last { "" } else { "," };
        text.push_str(&format!("  \"{word}\"{comma}\n"));
    }
    text.push_str(" ]\n}\n");
    if let Some(parent) = out.parent() {
        std::fs::create_dir_all(parent)
            .map_err(|error| Error::Invalid(format!("{}: {error}", parent.display())))?;
    }
    std::fs::write(out, text)
        .map_err(|error| Error::Invalid(format!("{}: {error}", out.display())))?;
    info!(
        "{}: {} word(s) from {verified} verified row(s)",
        out.display(),
        words.len()
    );
    if !report {
        return Ok(());
    }
    let shown = out
        .strip_prefix(repository_root())
        .map(|path| path.to_path_buf())
        .unwrap_or_else(|_| out.to_path_buf());
    // The sentence the pre-port builder printed, word for word, including its two zeroes: the builder
    // never scanned documentation, and the artifact's sources block records that choice.
    println!(
        "{}: {} word(s) ({} from {verified} verified instances, 0 only from 0 documents)",
        shown.display(),
        words.len(),
        words.len()
    );
    Ok(())
}

/// Judge one piece of text with the **actual** readability rule, so a calibration measures the real
/// thresholds rather than a re-implementation of them.
fn judge_text(text: &str, vocabulary: &crate::filters::Vocabulary) -> crate::filters::Verdict {
    let mut distinct = std::collections::BTreeSet::new();
    let mut characters = 0usize;
    let mut alpha = 0usize;
    for character in text.chars() {
        distinct.insert(character);
        if !character.is_whitespace() {
            characters += 1;
        }
        if character.is_alphabetic() {
            alpha += 1;
        }
    }
    let candidate = crate::models::PostFilterInput {
        encoded: text.as_bytes(),
        length: text.len(),
        printable: characters,
        unknown: 0,
        distinct: distinct.len(),
        characters,
        alpha,
        literal: text,
        min_run: crate::lexagraph::MIN_RUN,
    };
    crate::filters::readability_verdict(&candidate, text, Some(vocabulary))
}

/// Report the readability verdict for text, or for every verified row when none is given.
fn run_readability(cli: &Cli, text: &[String], index_path: &Path) -> Result<()> {
    let vocabulary = vocabulary(&cli.vocabulary, cli.no_vocabulary)?;
    let mut rows: Vec<(String, crate::filters::Verdict)> = Vec::new();
    if text.is_empty() {
        let index = crate::textindex::load(index_path)?;
        if index.instances.is_empty() {
            return Err(Error::Invalid(format!(
                "{}: no instances to judge",
                index_path.display()
            )));
        }
        for instance in &index.instances {
            if !matches!(instance.class, TextClass::Dialogue | TextClass::Battle) {
                continue;
            }
            rows.push((
                instance.reading.clone(),
                judge_text(&instance.reading, &vocabulary),
            ));
        }
        if rows.is_empty() {
            return Err(Error::Invalid(format!(
                "{}: no verified rows to judge",
                index_path.display()
            )));
        }
    } else {
        for line in text {
            rows.push((line.clone(), judge_text(line, &vocabulary)));
        }
    }
    let shaped = rows.iter().filter(|(_, verdict)| verdict.shaped).count();
    let readable = rows.iter().filter(|(_, verdict)| verdict.readable).count();
    let attested = rows.iter().filter(|(_, verdict)| verdict.attested).count();
    let failures: Vec<serde_json::Value> = rows
        .iter()
        .filter(|(_, verdict)| !verdict.readable)
        .take(20)
        .map(|(text, verdict)| {
            serde_json::json!({
                "text": text.chars().take(60).collect::<String>(),
                "shaped": verdict.shaped,
                "attested": verdict.attested,
            })
        })
        .collect();
    info!(
        "{} row(s) judged: {shaped} shaped, {readable} readable, {attested} attested",
        rows.len()
    );
    println!(
        "{}",
        serde_json::to_string_pretty(&serde_json::json!({
            "rows": rows.len(),
            "shaped": shaped,
            "readable": readable,
            "attested": attested,
            "unshaped": rows.len() - shaped,
            "unattested": shaped - readable,
            "failures": failures,
        }))
        .map_err(|error| Error::Invalid(error.to_string()))?
    );
    Ok(())
}

/// SHA-256 of a byte slice, in lowercase hex.
///
/// Implemented here rather than imported: the registry's owner records carry `sha256:` digests, and the
/// crate takes no dependency for a hash this small.
fn sha256_hex(bytes: &[u8]) -> String {
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

/// The `sha256:` digest of a file, the form the registry's owner records use.
fn sha256_of(path: &Path) -> Result<String> {
    let bytes = std::fs::read(path)
        .map_err(|error| Error::Invalid(format!("{}: {error}", path.display())))?;
    Ok(format!("sha256:{}", sha256_hex(&bytes)))
}

/// Every archive under a root, in a stable order.
fn archives_under(root: &Path) -> Result<Vec<PathBuf>> {
    let mut found = Vec::new();
    let mut pending = vec![root.to_path_buf()];
    while let Some(directory) = pending.pop() {
        for entry in std::fs::read_dir(&directory)
            .map_err(|error| Error::Invalid(format!("{}: {error}", directory.display())))?
        {
            let path = entry
                .map_err(|error| Error::Invalid(error.to_string()))?
                .path();
            if path.is_dir() {
                pending.push(path);
            } else if path.extension().is_some_and(|extension| extension == "EMI") {
                found.push(path);
            }
        }
    }
    found.sort();
    Ok(found)
}

/// Report the payloads the inventory identifies, so the window registry can name them too.
/// The repository root, found the way artifacts are anchored: by walking up from the working directory
/// to the ancestor that holds both the build output and the crate.
fn repository_root() -> std::path::PathBuf {
    let Ok(cwd) = std::env::current_dir() else {
        return std::path::PathBuf::from(".");
    };
    for ancestor in cwd.ancestors().take(9) {
        if ancestor.join("out").is_dir() && ancestor.join("tools/rust/bof3-text").is_dir() {
            return ancestor.to_path_buf();
        }
    }
    cwd
}

/// The `(offset, kind, name)` triples of one splat file.
///
/// A splat entry is the line `- - <offset>`; the lines beneath it name the kind and the symbol.
fn splat_segments(splat: &Path) -> Vec<(usize, String, String)> {
    let Ok(text) = std::fs::read_to_string(splat) else {
        return Vec::new();
    };
    let mut segments: Vec<(usize, String, String)> = Vec::new();
    let mut current: Option<usize> = None;
    let mut body: Vec<String> = Vec::new();
    let flush = |current: &Option<usize>,
                 body: &Vec<String>,
                 segments: &mut Vec<(usize, String, String)>| {
        if let Some(offset) = current {
            segments.push((
                *offset,
                body.first().cloned().unwrap_or_default(),
                body.get(1).cloned().unwrap_or_default(),
            ));
        }
    };
    for line in text.lines() {
        let trimmed = line.trim_start();
        if let Some(rest) = trimmed.strip_prefix("- - ") {
            if let Ok(offset) = rest.trim().parse::<usize>() {
                flush(&current, &body, &mut segments);
                current = Some(offset);
                body.clear();
                continue;
            }
        }
        if current.is_some() {
            if let Some(value) = trimmed.strip_prefix("- ") {
                let value = value
                    .trim()
                    .trim_matches(|character| character == '\'' || character == '"');
                if !value.is_empty() {
                    body.push(value.to_string());
                }
            }
        }
    }
    flush(&current, &body, &mut segments);
    segments
}

/// The `(archive, entry, load_address)` a target manifest names.
fn target_facts(target: &Path) -> Result<(String, usize, u64)> {
    let path = target.join("target.toml");
    let text = std::fs::read_to_string(&path)
        .map_err(|error| Error::Invalid(format!("{}: {error}", path.display())))?;
    let mut disc: Option<String> = None;
    let mut load: Option<u64> = None;
    for line in text.lines() {
        // A multi-member manifest declares its own archive first and its member tables after it,
        // and member tables carry their own unindented `disc_id`/`load_address` lines. The first
        // well-formed match is therefore this target's identity; later ones belong to members.
        if disc.is_none() {
            if let Some(rest) = line.strip_prefix("disc_id") {
                if let Some(value) = rest.trim().strip_prefix('=') {
                    disc = Some(value.trim().trim_matches('"').to_string());
                }
            }
        }
        if load.is_none() {
            if let Some(rest) = line.strip_prefix("load_address") {
                if let Some(value) = rest.trim().strip_prefix('=') {
                    let value = value.trim();
                    let hex = value
                        .strip_prefix("0x")
                        .or_else(|| value.strip_prefix("0X"));
                    load = hex.and_then(|hex| u64::from_str_radix(hex, 16).ok());
                }
            }
        }
    }
    let (Some(identity), Some(load)) = (disc, load) else {
        return Err(Error::Invalid(format!(
            "{}: no disc_id or load_address",
            path.display()
        )));
    };
    let identity = identity
        .strip_prefix("BIN/")
        .unwrap_or(&identity)
        .to_string();
    let (archive, entry) = identity.rsplit_once('#').unwrap_or((identity.as_str(), ""));
    if archive.is_empty() || entry.is_empty() || !entry.chars().all(|c| c.is_ascii_digit()) {
        return Err(Error::Invalid(format!(
            "{}: disc_id is not `BIN/PATH.EMI#N`",
            path.display()
        )));
    }
    Ok((archive.to_string(), entry.parse().unwrap_or(0), load))
}

/// The reviewed `textbin` ranges, each bounded by the entry after it and naming its record.
///
/// A reviewed range must also name the runtime address its own offset and load address imply, so a
/// record that drifts from its address fails rather than registering a wrong window.
fn reviewed_windows(repo: &Path, targets: &Path) -> Result<Vec<serde_json::Value>> {
    let mut splats: Vec<std::path::PathBuf> = Vec::new();
    let mut pending = vec![targets.to_path_buf()];
    while let Some(directory) = pending.pop() {
        let entries = std::fs::read_dir(&directory)
            .map_err(|error| Error::Invalid(format!("{}: {error}", directory.display())))?;
        for entry in entries {
            let path = entry
                .map_err(|error| Error::Invalid(error.to_string()))?
                .path();
            if path.is_dir() {
                pending.push(path);
            } else if path.file_name().is_some_and(|name| name == "splat.yaml") {
                splats.push(path);
            }
        }
    }
    splats.sort();
    let mut windows = Vec::new();
    for splat in splats {
        let segments = splat_segments(&splat);
        for (position, (offset, kind, name)) in segments.iter().enumerate() {
            if kind != "textbin" {
                continue;
            }
            let Some((next, _, _)) = segments.get(position + 1) else {
                return Err(Error::Invalid(format!(
                    "{}: textbin at {offset} has no following entry to bound it",
                    splat.display()
                )));
            };
            if *next <= *offset {
                return Err(Error::Invalid(format!(
                    "{}: textbin at {offset} has a non-positive length",
                    splat.display()
                )));
            }
            let parent = splat.parent().unwrap_or(targets);
            let (archive, entry, load) = target_facts(parent)?;
            if name.starts_with("T_") {
                let expected = format!("T_{:08X}", load + *offset as u64);
                if name != &expected {
                    return Err(Error::Invalid(format!(
                        "{}: {name} does not match load_address + offset ({expected})",
                        splat.display()
                    )));
                }
            }
            let record = splat
                .strip_prefix(repo)
                .unwrap_or(&splat)
                .to_string_lossy()
                .replace('\\', "/");
            windows.push(serde_json::json!({
                "archive": archive,
                "entry": entry,
                "start": offset,
                "length": next - offset,
                "kind": "reviewed_textbin",
                "owner": {
                    "kind": "reviewed-record",
                    "record": record,
                    "evidence": "out/text-format/boundary-review.md",
                    "hash": sha256_of(&splat)?,
                },
            }));
        }
    }
    if windows.is_empty() {
        return Err(Error::Invalid(format!(
            "{}: no textbin ranges found; refusing to contribute no windows",
            targets.display()
        )));
    }
    // Every reviewed range the boundary record names must be present, so losing one is a failure rather
    // than a smaller registry.
    let expected: Vec<(&str, usize)> = vec![
        ("ETC/COMMU00.EMI", 0),
        ("SCENARIO/SCENA00.EMI", 0),
        ("WORLD00/AREA026.EMI", 13),
    ];
    let found: Vec<(String, usize)> = windows
        .iter()
        .map(|window| {
            (
                window["archive"].as_str().unwrap_or_default().to_string(),
                window["entry"].as_u64().unwrap_or_default() as usize,
            )
        })
        .collect();
    let missing: Vec<String> = expected
        .iter()
        .filter(|pair| !found.contains(&(pair.0.to_string(), pair.1)))
        .map(|pair| format!("{}#{}", pair.0, pair.1))
        .collect();
    if !missing.is_empty() {
        return Err(Error::Invalid(format!(
            "{}: reviewed ranges missing for {missing:?}",
            targets.display()
        )));
    }
    Ok(windows)
}

/// The pointer-table and row-extent windows of every subfile that parses.
fn parsed_windows(
    repo: &Path,
    root: &Path,
    archives: &[std::path::PathBuf],
) -> Result<(Vec<serde_json::Value>, Vec<String>)> {
    let record = "tools/rust/bof3-text/src/extract.rs";
    let owner = serde_json::json!({
        "kind": "parse",
        "record": record,
        "hash": sha256_of(&repo.join(record))?,
    });
    let mut windows = Vec::new();
    let mut inspected = Vec::new();
    for path in archives {
        let data = read(path, "archive")?;
        let name = path
            .strip_prefix(root)
            .map_err(|_| Error::Invalid(format!("{}: outside {}", path.display(), root.display())))?
            .to_string_lossy()
            .replace('\\', "/");
        inspected.push(name.clone());
        for (entry, payload) in crate::lexagraph::entries(&data)? {
            let class = TextClass::from_load_argument(entry.ram_ptr);
            windows.extend(windows_for_subfile(
                &name,
                entry.index,
                entry.size,
                class,
                payload,
                &owner,
            )?);
        }
    }
    Ok((windows, inspected))
}

/// The windows one subfile contributes through the parse.
///
/// A subfile whose class is known is itself a classified window — its class owns the whole payload,
/// including the unused slot space — and its pointer table and row extents follow. A known-class subfile
/// whose windows cannot be read is an **unavailable** ownership source, not an empty one, so it is
/// refused rather than skipped; a subfile with no class simply contributes nothing.
fn windows_for_subfile(
    name: &str,
    entry: usize,
    size: u32,
    class: Option<TextClass>,
    payload: &[u8],
    owner: &serde_json::Value,
) -> Result<Vec<serde_json::Value>> {
    let mut windows = Vec::new();
    match crate::extract::parse_section(payload) {
        Ok(section) => {
            windows.push(serde_json::json!({
                "archive": name,
                "entry": entry,
                "start": 0,
                "length": size,
                "kind": "banked_payload",
                "owner": owner,
            }));
            for bank in &section.banks {
                if bank.pointer_bytes > 0 {
                    windows.push(serde_json::json!({
                        "archive": name, "entry": entry,
                        "start": bank.start, "length": bank.pointer_bytes,
                        "kind": "pointer_table", "owner": owner,
                    }));
                }
                for row in &bank.rows {
                    let Some((start, end)) = row.span else {
                        continue;
                    };
                    windows.push(serde_json::json!({
                        "archive": name, "entry": entry,
                        "start": bank.start + start, "length": end - start,
                        "kind": "row_extent", "owner": owner,
                    }));
                }
            }
        }
        Err(error) => {
            if class.is_some() {
                return Err(Error::Invalid(format!(
                    "{name}#{entry}: known class produced no windows (parse error: {error})"
                )));
            }
        }
    }
    Ok(windows)
}

// The window helpers it exercises sit above it in this file, which is where they belong.
#[allow(clippy::items_after_test_module)]
#[cfg(test)]
mod window_tests {
    use super::*;

    #[test]
    fn a_known_class_with_no_windows_is_refused_and_an_unknown_one_is_silent() {
        let owner = serde_json::json!({"kind": "parse", "record": "x", "hash": "sha256:0"});
        let garbage = [0u8; 8];
        let refused = windows_for_subfile(
            "A/B.EMI",
            11,
            8,
            Some(TextClass::Dialogue),
            &garbage,
            &owner,
        );
        let message = refused
            .expect_err("a known-class subfile with no windows must be refused")
            .to_string();
        assert!(
            message.contains("known class produced no windows"),
            "{message}"
        );
        let silent = windows_for_subfile("A/B.EMI", 11, 8, None, &garbage, &owner)
            .expect("an unclassed subfile contributes nothing without failing");
        assert!(
            silent.is_empty(),
            "an unclassed subfile contributes no windows"
        );
    }
}

/// Build the classified-window registry from the sources that already know: the parse, the payload
/// inventory and the reviewed records. Nothing is inferred from a byte pattern.
fn run_windows(
    root_arg: &Path,
    targets_arg: &Path,
    out_arg: &Path,
    payload_path: &Path,
    as_json: bool,
    quiet: bool,
) -> Result<()> {
    let repo = repository_root();
    let root = anchored(root_arg);
    let root = root.as_path();
    let targets = anchored(targets_arg);
    let targets = targets.as_path();
    let archives = archives_under(root)?;
    if archives.is_empty() {
        return Err(Error::Invalid(format!(
            "{}: no archives found; refusing to build an empty registry",
            root.display()
        )));
    }
    let (mut windows, inspected) = parsed_windows(&repo, root, &archives)?;
    let payload_path = anchored(payload_path);
    let payloads = crate::filters::load_payloads(&payload_path, false)?;
    let rows = identified_payloads(root, &payloads)?;
    if rows.is_empty() {
        return Err(Error::Invalid(
            "no identified payloads; refusing a registry without them".to_string(),
        ));
    }
    let digest = sha256_of(&payload_path)?;
    for row in &rows {
        let records = row["records"].as_array().cloned().unwrap_or_default();
        let Some(record) = records.first().and_then(|value| value.as_str()) else {
            return Err(Error::Invalid(format!(
                "{}#{}: identified payload names no record; a known_payload window may not be registered without one",
                row["archive"].as_str().unwrap_or_default(),
                row["entry"]
            )));
        };
        // The registry records the magic and the check that confirmed it together — `pBAV vab_header_size`
        // — which is the form the artifact carried before the port and the form the `payloads` mode's two
        // separate fields combine into.
        // The label the artifact has carried since before the port: the magic and the check that
        // confirmed it, joined by a space. The pairing family has no magic of its own, so its label
        // begins with that space — a quirk of the artifact, reproduced rather than tidied, so the port
        // and the pre-port generator agree byte for byte.
        let magic = row["magic"].as_str().unwrap_or_default();
        let kind = row["check"].as_str().unwrap_or_default();
        let check = format!("{magic} {kind}");
        windows.push(serde_json::json!({
            "archive": row["archive"],
            "entry": row["entry"],
            "start": 0,
            "length": row["size"],
            "kind": "known_payload",
            "owner": {
                "kind": "payload-inventory",
                "record": record,
                "check": check,
                "hash": digest,
            },
        }));
    }
    windows.extend(reviewed_windows(&repo, targets)?);
    windows.sort_by_key(|window| {
        (
            window["archive"].as_str().unwrap_or_default().to_string(),
            window["entry"].as_u64().unwrap_or_default(),
            window["start"].as_u64().unwrap_or_default(),
            window["length"].as_u64().unwrap_or_default(),
            window["kind"].as_str().unwrap_or_default().to_string(),
        )
    });
    let mut counts: std::collections::BTreeMap<String, usize> = std::collections::BTreeMap::new();
    let mut covered: std::collections::BTreeMap<String, u64> = std::collections::BTreeMap::new();
    for window in &windows {
        let kind = window["kind"].as_str().unwrap_or_default().to_string();
        *counts.entry(kind.clone()).or_default() += 1;
        *covered.entry(kind).or_default() += window["length"].as_u64().unwrap_or_default();
    }
    let inputs = serde_json::json!({
        "root": root_arg.to_string_lossy(),
        "targets": targets_arg.to_string_lossy(),
        "archives": archives.len(),
        "inspected_archives": inspected,
        "windows": windows.len(),
        "kinds": counts,
        "bytes": covered,
    });
    let destination = anchored(out_arg);
    if let Some(parent) = destination.parent() {
        std::fs::create_dir_all(parent)
            .map_err(|error| Error::Invalid(format!("{}: {error}", parent.display())))?;
    }
    let mut text = Vec::new();
    let mut serializer = serde_json::Serializer::with_formatter(
        &mut text,
        serde_json::ser::PrettyFormatter::with_indent(b" "),
    );
    let document = serde_json::json!({
        "schema": "bof3.text-windows/v1",
        "version": 1,
        "inputs": inputs,
        "windows": windows,
    });
    use serde::Serialize;
    document
        .serialize(&mut serializer)
        .map_err(|error| Error::Invalid(error.to_string()))?;
    text.push(b'\n');
    std::fs::write(&destination, text)
        .map_err(|error| Error::Invalid(format!("{}: {error}", destination.display())))?;
    info!(
        "{}: {} classified window(s) over {} archive(s)",
        destination.display(),
        document["windows"].as_array().map(Vec::len).unwrap_or(0),
        archives.len()
    );
    if quiet {
        return Ok(());
    }
    if as_json {
        // The same one-space, key-sorted shape the registry document uses, so the summary a caller reads
        // is byte-identical to the one the pre-port generator printed.
        let mut buffer = Vec::new();
        let mut serializer = serde_json::Serializer::with_formatter(
            &mut buffer,
            serde_json::ser::PrettyFormatter::with_indent(b" "),
        );
        use serde::Serialize;
        document["inputs"]
            .serialize(&mut serializer)
            .map_err(|error| Error::Invalid(error.to_string()))?;
        buffer.push(b'\n');
        print!("{}", String::from_utf8_lossy(&buffer));
        return Ok(());
    }
    // Name the registry the way the caller named it: a path inside the repository is shown relative to it,
    // exactly as the pre-port generator showed it.
    let shown = destination
        .strip_prefix(&repo)
        .map(|path| path.to_path_buf())
        .unwrap_or_else(|_| destination.clone());
    println!(
        "{}: {} classified window(s) over {} archive(s)",
        shown.display(),
        document["windows"].as_array().map(Vec::len).unwrap_or(0),
        archives.len()
    );
    let no_kinds = serde_json::Map::new();
    let kinds = document["inputs"]["kinds"].as_object().unwrap_or(&no_kinds);
    for (kind, count) in kinds {
        // Plain integers, not Values: a `serde_json::Value`'s own Display ignores the width, which would
        // silently collapse the column the pre-port table had.
        let count = count.as_u64().unwrap_or(0);
        let bytes = document["inputs"]["bytes"][kind].as_u64().unwrap_or(0);
        println!("  {kind:<16} {count:>6}  {bytes:>10} bytes");
    }
    Ok(())
}

/// The payloads the inventory identifies over a corpus, one row per identified subfile.
///
/// The one implementation of that identification: `payloads` prints these rows and the window registry
/// turns them into `known_payload` windows, so the two cannot disagree about what is identified.
fn identified_payloads(
    root: &Path,
    payloads: &crate::filters::KnownPayload,
) -> Result<Vec<serde_json::Value>> {
    let base = root;
    let mut results = Vec::new();
    for path in archives_under(root)? {
        let data = read(&path, "archive")?;
        let archive = path
            .strip_prefix(base)
            .map_err(|_| Error::Invalid(format!("{}: outside {}", path.display(), base.display())))?
            .to_string_lossy()
            .replace('\\', "/");
        let mut preceding: Vec<&[u8]> = Vec::new();
        for (entry, payload) in crate::lexagraph::entries(&data)? {
            let identified = payloads
                .identify(payload)
                .or_else(|| payloads.identify_paired(&preceding, payload));
            preceding.push(payload);
            let Some(family) = identified else {
                continue;
            };
            results.push(serde_json::json!({
                "archive": archive,
                "entry": entry.index,
                "offset": entry.offset,
                "size": entry.size,
                "family": family.family,
                "magic": family.magic,
                "format": family.format,
                "records": family.records,
                "check": family.check.kind,
            }));
        }
    }
    Ok(results)
}

fn run_payloads(cli: &Cli, root: &Path) -> Result<()> {
    // The inventory is the only identification policy: this command reports what the filters decide,
    // including the pairing rule and its named records, and `--no-payloads` therefore reports nothing
    // rather than discovering payloads of its own.
    let payloads = known_payloads(&cli.payloads, cli.no_payloads)?;
    let results = identified_payloads(root, &payloads)?;
    info!(
        "{} identified payload(s) over the corpus in {}",
        results.len(),
        root.display()
    );
    println!(
        "{}",
        serde_json::to_string_pretty(&serde_json::json!({
            "schema": "bof3.identified-payloads/v1",
            "results": results,
        }))
        .map_err(|error| Error::Invalid(error.to_string()))?
    );
    Ok(())
}

/// Anchor a relative artifact path (registry, payload inventory, vocabulary) at the repository root.
///
/// The defaults are relative, so a caller running the tool from a subdirectory — which the tests and
/// the generator both do — would otherwise fail closed on a file that exists. The path is tried as
/// given first, then against each ancestor of the working directory, and is returned unchanged when
/// nothing matches so the caller still fails closed with the path it was given.
fn anchored(path: &Path) -> std::path::PathBuf {
    if path.is_absolute() || path.exists() {
        return path.to_path_buf();
    }
    let Ok(cwd) = std::env::current_dir() else {
        return path.to_path_buf();
    };
    for ancestor in cwd.ancestors().skip(1).take(8) {
        let candidate = ancestor.join(path);
        if candidate.exists() {
            return candidate;
        }
    }
    path.to_path_buf()
}

/// The recorded per-family removal counts, from the inventory.
///
/// An excluded payload is never parsed, so these are the measurements recorded when the inventory was
/// built, reported **once per run** rather than counted per subfile.
fn payload_totals(payloads: &crate::filters::KnownPayload) -> serde_json::Value {
    serde_json::json!(
        payloads
            .totals()
            .into_iter()
            .map(
                |(family, magic, removed, marginal, basis)| serde_json::json!({
                    "family": family,
                    "magic": magic,
                    "candidates_removed": removed,
                    "candidates_removed_marginal": marginal,
                    "basis": basis,
                })
            )
            .collect::<Vec<_>>()
    )
}

/// Print the recorded per-family removal counts, one line each.
fn print_payload_totals(payloads: &crate::filters::KnownPayload) {
    for (family, magic, removed, marginal, _) in payloads.totals() {
        println!(
            "  payload exclusion `{magic}` ({family}): {removed} candidate(s) removed on its own, \
{marginal} more once unreadable runs are gone (recorded measurements; the payload is never parsed)"
        );
    }
}

/// Load the vocabulary the readability rule checks against, failing closed when it is absent.
fn vocabulary(path: &Path, disabled: bool) -> Result<crate::filters::Vocabulary> {
    let path = anchored(path);
    let path = path.as_path();
    let words = crate::filters::load_vocabulary(path, disabled)?;
    info!(
        "{} vocabulary word(s) loaded from {}",
        words.len(),
        path.display()
    );
    Ok(words)
}

/// Load the payload inventory, or none when the caller explicitly opts out.
///
/// A missing inventory is an error on the searching path rather than "nothing is excluded": these
/// families are the whole reason the exclusion exists, and silently searching known audio would be a
/// quiet regression.
fn known_payloads(path: &Path, disabled: bool) -> Result<crate::filters::KnownPayload> {
    let path = anchored(path);
    let path = path.as_path();
    let payloads = crate::filters::load_payloads(path, disabled)?;
    info!(
        "{} identified payload family(ies) loaded from {}",
        payloads.len(),
        path.display()
    );
    Ok(payloads)
}

/// Load the classified-window registry, or none when the caller opts out or it is absent.
fn window_index(path: &Path, disabled: bool) -> Result<crate::windows::WindowIndex> {
    let path = anchored(path);
    let path = path.as_path();
    let index = crate::filters::load_windows(path, disabled)?;
    info!(
        "{} classified window(s) loaded from {}",
        index.len(),
        path.display()
    );
    Ok(index)
}

/// The registry key for one archive, refusing only an archive the source never inspected.
///
/// An inspected archive with no owned windows latches unwindowed — 634 of the 880 corpus archives
/// own no windows and must keep working — while an archive absent from the inspected set is an
/// unavailable ownership source rather than an unclassified one.
fn window_key<'a>(
    windows: &'a crate::windows::WindowIndex,
    label: &str,
) -> Result<Option<&'a str>> {
    // An explicit opt-out is a choice, not an unavailable ownership source: validate nothing.
    if windows.disabled() {
        return Ok(None);
    }
    match windows.key_for(label)? {
        Some(key) => Ok(Some(key)),
        None if windows.inspected(label) => Ok(None),
        None => Err(Error::Invalid(format!(
            "{label}: not inspected by the window registry; rebuild it with `bin/harness text windows`, or pass --no-windows"
        ))),
    }
}

fn run_scan(
    cli: &Cli,
    archive: &Path,
    min_run: usize,
    class_filter: Option<&str>,
    entry_filter: Option<usize>,
    filters: &crate::filters::ScanFilters<'_>,
) -> Result<()> {
    let data = read(archive, "archive")?;
    let wanted = resolve_class(class_filter)?;
    let label = archive_label(archive);
    // The registry keys archives corpus-relative; a caller may pass a longer path.
    let key = window_key(filters.windows, &label)?.unwrap_or("");
    let mut scanned = crate::lexagraph::scan_archive_excluding(&data, min_run, key, filters)?;
    let mut tally = crate::lexagraph::Tally::default();
    for (_, _, subfile) in &scanned {
        tally.add(subfile);
    }
    scanned.retain(|(entry, _, _)| {
        entry_filter.is_none_or(|index| entry.index == index)
            && wanted
                .is_none_or(|class| TextClass::from_load_argument(entry.ram_ptr) == Some(class))
    });
    let total: usize = scanned.iter().map(|(_, latches, _)| latches.len()).sum();
    info!(
        "{} candidate raw-text run(s) across {} subfile(s) in {} ({} window(s) skipped, {} removed, {} split)",
        total,
        scanned.len(),
        archive.display(),
        tally.windows,
        tally.latches_removed,
        tally.latches_split
    );
    if cli.json {
        let results: Vec<serde_json::Value> = scanned
            .iter()
            .map(|(entry, latches, subfile)| {
                serde_json::json!({
                    "source": Source::new(
                        archive_label(archive),
                        Some(entry.index),
                        entry.ram_ptr,
                        TextClass::Heuristic,
                    ).to_string(),
                    "offset": entry.offset,
                    "size": entry.size,
                    "windows_detail": crate::lexagraph::details(&label, entry.index, subfile),
                    "latches": latches,
                })
            })
            .collect();
        println!(
            "{}",
            serde_json::to_string_pretty(&serde_json::json!({
                "archive": archive_label(archive),
                "heuristic": true,
                "windows": tally,
                "payload_totals": payload_totals(filters.payloads),
                "note": "latches are leads from heuristics, not consumer-verified text",
                "totals": {"subfiles": scanned.len(), "latches": total},
                "results": results,
            }))
            .map_err(|error| Error::Invalid(error.to_string()))?
        );
        return Ok(());
    }
    println!(
        "{} {}: {} heuristic latch(es) in {} subfile(s)",
        logging::heading("scan"),
        archive.display(),
        total,
        scanned.len()
    );
    print_payload_totals(filters.payloads);
    for (entry, latches, _) in &scanned {
        for latch in latches.iter().take(3) {
            println!(
                "  #{} +{:#06x} {}B ratio={:.2} {}",
                entry.index, latch.offset, latch.length, latch.ratio, latch.preview
            );
        }
    }
    Ok(())
}

/// Round-trip every subfile of a verified class across the corpus.
///
/// The gate for this tool: it proves the bytes it claims to understand are reproduced exactly,
/// and it lists offenders with their archive, entry and reason instead of hiding them behind a
/// summary count.
fn run_verify(cli: &Cli, root: &Path) -> Result<()> {
    let mut per_class: std::collections::BTreeMap<String, (usize, usize)> =
        std::collections::BTreeMap::new();
    let mut failures: Vec<String> = Vec::new();
    let mut archives = 0usize;
    for path in crate::textindex::archives(root)? {
        let data = read(&path, "archive")?;
        archives += 1;
        let name = crate::textindex::relative_archive(root, &path);
        for (entry, payload) in crate::lexagraph::entries(&data)? {
            let Some(class) = TextClass::from_load_argument(entry.ram_ptr) else {
                continue;
            };
            let counts = per_class.entry(class.name().to_string()).or_default();
            counts.0 += 1;
            let section = match extract::parse_section(payload) {
                Ok(section) => section,
                Err(error) => {
                    failures.push(format!("{name}#{}: {error}", entry.index));
                    continue;
                }
            };
            let source = source_for(&path, entry, class, true);
            let document = extract::document(&section, &source);
            match pack::rebuild(&section, &document) {
                Ok(rebuilt) if rebuilt == payload => counts.1 += 1,
                Ok(rebuilt) => failures.push(format!(
                    "{name}#{}: rebuilt {} byte(s) for a {} byte payload",
                    entry.index,
                    rebuilt.len(),
                    payload.len()
                )),
                Err(error) => failures.push(format!("{name}#{}: {error}", entry.index)),
            }
        }
    }
    let total: usize = per_class.values().map(|(total, _)| *total).sum();
    let identical: usize = per_class.values().map(|(_, identical)| *identical).sum();
    info!(
        "{identical} of {total} text subfile(s) over {archives} archive(s) round-trip byte-identically"
    );
    if cli.json {
        println!(
            "{}",
            serde_json::to_string_pretty(&serde_json::json!({
                "root": root.display().to_string(),
                "archives": archives,
                "subfiles": total,
                "identical": identical,
                "by_class": per_class.iter().map(|(class, (total, identical))| {
                    serde_json::json!({"class": class, "subfiles": total, "identical": identical})
                }).collect::<Vec<_>>(),
                "failures": failures,
            }))
            .map_err(|error| Error::Invalid(error.to_string()))?
        );
    } else {
        println!(
            "{} {}: {identical} of {total} text subfile(s) over {archives} archive(s) round-trip byte-identically",
            if failures.is_empty() {
                logging::success("verified")
            } else {
                logging::heading("FAILED")
            },
            root.display()
        );
        for (class, (total, identical)) in &per_class {
            println!("  {class:<10} {identical}/{total}");
        }
        for failure in failures.iter().take(10) {
            println!("  {failure}");
        }
    }
    if !failures.is_empty() {
        return invalid(format!(
            "{} of {total} text subfile(s) did not round-trip; the first failures are listed above",
            failures.len()
        ));
    }
    Ok(())
}

/// Map text-versus-data roles across the corpus into a read-only report.
fn run_map(
    cli: &Cli,
    root: &Path,
    out: &Path,
    windows: &crate::windows::WindowIndex,
    payloads: &crate::filters::KnownPayload,
    vocabulary: &crate::filters::Vocabulary,
) -> Result<()> {
    let report = crate::segmentmap::build(
        root,
        windows,
        payloads,
        vocabulary,
        crate::filters::Readability::of_opt_out(cli.no_readable),
    )?;
    crate::segmentmap::write(&report, out)?;
    let total = |key: &str| report.totals.get(key).copied().unwrap_or(0);
    info!(
        "mapped {} subfile(s) over {} archive(s) into {}",
        total("subfiles"),
        total("archives"),
        out.display()
    );
    if cli.json {
        println!(
            "{}",
            serde_json::to_string_pretty(&serde_json::json!({
                "report": out.display().to_string(),
                "root": root.display().to_string(),
                "totals": report.totals,
            }))
            .map_err(|error| Error::Invalid(error.to_string()))?
        );
        return Ok(());
    }
    println!(
        "{} {}: {} subfile(s) over {} archive(s)",
        logging::success("mapped"),
        out.display(),
        total("subfiles"),
        total("archives")
    );
    println!("  verified text   {}", total("verified_text_subfiles"));
    println!("  data            {}", total("data_subfiles"));
    println!(
        "  identified data {} ({} by magic)",
        total("data_subfiles") - total("unidentified_data_subfiles"),
        total("magic:pBAV") + total("magic:pQES")
    );
    println!("  unidentified    {}", total("unidentified_data_subfiles"));
    println!(
        "  candidates      {} text-like run(s) in {} data subfile(s) (candidates only)",
        total("text_like_runs_in_data_subfiles"),
        total("subfiles_with_text_like_runs")
    );
    println!(
        "  in text banks   {} run(s) belonging to the verified text itself",
        total("text_like_runs_in_text_subfiles")
    );
    for (key, value) in &report.totals {
        if let Some(what) = key.strip_prefix("magic:") {
            println!("  magic           {value:>6}  {what}");
        }
    }
    println!("  this report proposes roles and changes nothing");
    Ok(())
}

/// Probe one archive for text-bank structure, independent of any known class.
///
/// This is the structural evidence a class promotion rests on: a candidate's subfile either
/// parses as a section (a pointer table whose rows tile the string region) and reproduces
/// itself on an unedited round trip, or it does not. Class membership is *not* consulted, so an
/// unknown load argument is probed on the same terms as `dialogue`.
fn run_probe(cli: &Cli, archive: &Path, round_trip: bool) -> Result<()> {
    let data = read(archive, "archive")?;
    let mut results = Vec::new();
    let mut parsed = 0usize;
    let mut reproduced = 0usize;
    for (entry, payload) in crate::lexagraph::entries(&data)? {
        let class = TextClass::from_load_argument(entry.ram_ptr);
        let mut record = serde_json::json!({
            "entry": entry.index,
            "offset": entry.offset,
            "address": format!("{:#010x}", entry.ram_ptr),
            "size": entry.size,
            "class": class,
        });
        match extract::parse_section(payload) {
            Ok(section) => {
                parsed += 1;
                record["parsed"] = serde_json::json!(true);
                record["banks"] = serde_json::json!(section.banks.len());
                record["rows"] = serde_json::json!(section.rows());
                record["used"] = serde_json::json!(section.used());
                record["options"] = serde_json::json!(extract::option_rows(&section));
                // The classified windows this parse owns: the bank's pointer table and every row
                // extent. Emitted so the corpus window registry can be built from the parse rather
                // than from a re-derived guess.
                let windows: Vec<serde_json::Value> = section
                    .banks
                    .iter()
                    .map(|bank| {
                        serde_json::json!({
                            "bank": bank.index,
                            "start": bank.start,
                            "size": bank.size,
                            "pointer_bytes": bank.pointer_bytes,
                            "rows": bank
                                .rows
                                .iter()
                                .filter_map(|row| {
                                    row.span.map(|(start, end)| {
                                        serde_json::json!({"start": start, "length": end - start})
                                    })
                                })
                                .collect::<Vec<_>>(),
                        })
                    })
                    .collect();
                record["windows"] = serde_json::json!(windows);
                if round_trip {
                    // The class is irrelevant to rebuilding; a stand-in keeps this a probe of
                    // structure rather than of any class claim.
                    let source =
                        source_for(archive, entry, class.unwrap_or(TextClass::Heuristic), true);
                    let document = extract::document(&section, &source);
                    match pack::rebuild(&section, &document) {
                        Ok(rebuilt) if rebuilt == payload => {
                            reproduced += 1;
                            record["round_trip"] = serde_json::json!(true);
                        }
                        Ok(rebuilt) => {
                            record["round_trip"] = serde_json::json!(false);
                            record["rebuilt_bytes"] = serde_json::json!(rebuilt.len());
                        }
                        Err(error) => {
                            record["round_trip"] = serde_json::json!(false);
                            record["round_trip_error"] = serde_json::json!(error.to_string());
                        }
                    }
                }
            }
            Err(error) => {
                record["parsed"] = serde_json::json!(false);
                record["error"] = serde_json::json!(error.to_string());
            }
        }
        results.push(record);
    }
    info!(
        "{}: {} of {} subfile(s) parse{}",
        archive.display(),
        parsed,
        results.len(),
        if round_trip {
            format!(", {reproduced} reproduce byte-identically")
        } else {
            String::new()
        }
    );
    if cli.json {
        println!(
            "{}",
            serde_json::to_string_pretty(&serde_json::json!({
                "archive": archive_label(archive),
                "subfiles": results.len(),
                "parsed": parsed,
                "reproduced": if round_trip { serde_json::json!(reproduced) } else { serde_json::Value::Null },
                "results": results,
            }))
            .map_err(|error| Error::Invalid(error.to_string()))?
        );
        return Ok(());
    }
    println!(
        "{} {}: {} of {} subfile(s) parse as a text section{}",
        logging::heading("probe"),
        archive.display(),
        parsed,
        results.len(),
        if round_trip {
            format!(", {reproduced} reproduce byte-identically")
        } else {
            String::new()
        }
    );
    for record in results.iter().filter(|record| record["parsed"] == true) {
        println!(
            "  #{} {} {} banks={} rows={} used={}{}",
            record["entry"],
            record["address"].as_str().unwrap_or(""),
            record["class"].as_str().unwrap_or("-"),
            record["banks"],
            record["rows"],
            record["used"],
            if round_trip {
                match record["round_trip"].as_bool() {
                    Some(true) => " round-trip=identical".to_string(),
                    Some(false) => " round-trip=DIFFERS".to_string(),
                    None => String::new(),
                }
            } else {
                String::new()
            }
        );
    }
    Ok(())
}

/// Everything `search` selects on.
struct SearchQuery<'a> {
    needle: Option<&'a str>,
    ignore_case: bool,
    class: Option<TextClass>,
    script: Option<&'a str>,
    language: Option<&'a str>,
    entry: Option<usize>,
    archive: Option<&'a str>,
    limit: usize,
    /// Report the match start rather than the containing string's start.
    match_offset: bool,
    /// Resolve every selected instance and verify it, instead of reporting text hits.
    verify_entries: bool,
}

/// Load the corpus index, regenerating it when it does not describe the current inputs.
///
/// Freshness is decided by archive content and selection-policy digests. The cheap stat manifest is
/// still recorded and is used only to describe *what* changed, because its whole-second mtime and
/// size can both be preserved while content changes — and a search that read no hit's archive
/// would then answer from a stale index without noticing. Verifying content costs about a second
/// over the 260 MB corpus, which is the price of the guarantee.
///
/// A stale index is **regenerated and then used**, not refused: the requirement is that a stale
/// index regenerates rather than silently answering from it. `build-index --check` remains the
/// refusing mode for a caller that wants to detect staleness without writing.
fn fresh_index(
    index_path: &Path,
    root: &Path,
    windows: &crate::windows::WindowIndex,
    payloads: &crate::filters::KnownPayload,
    vocabulary: &crate::filters::Vocabulary,
    readability: crate::filters::Readability,
) -> Result<(crate::textindex::Index, bool)> {
    let existing = crate::textindex::load(index_path).ok();
    let current = crate::textindex::current_hash(root)?;
    let filters_hash =
        crate::filters::compose(windows, payloads, vocabulary, readability).fingerprint()?;
    if let Some(index) = existing {
        if index.is_fresh(&current, &filters_hash) {
            return Ok((index, false));
        }
        let detail = crate::textindex::manifest_diff(&index.inputs.manifest, &current.manifest, 3);
        info!(
            "{} is stale ({}); regenerating it",
            index_path.display(),
            if index.inputs.filters_hash != filters_hash {
                "selection policy changed".to_string()
            } else if detail.is_empty() {
                format!("content digest {} -> {}", index.inputs.hash, current.hash)
            } else {
                detail.join("; ")
            }
        );
    } else {
        info!("{} is missing; building it", index_path.display());
    }
    let rebuilt = crate::textindex::build(root, windows, payloads, vocabulary, readability)?;
    crate::textindex::write(&rebuilt, index_path)?;
    info!(
        "regenerated {}: {} instance(s) over {} archive(s)",
        index_path.display(),
        rebuilt.instances.len(),
        rebuilt.inputs.archives
    );
    Ok((rebuilt, true))
}

/// Search the retained corpus index.
///
/// The index is trusted only while it still describes the inputs: a stale manifest is refused
/// with a rebuild instruction instead of quietly answering from an outdated artifact. Because
/// the artifact stores readings but not per-character byte marks, a hit is resolved against
/// its own archive, and that resolution is verified rather than assumed.
fn run_search(
    cli: &Cli,
    index_path: &Path,
    root: &Path,
    query: &SearchQuery<'_>,
    windows: &crate::windows::WindowIndex,
    payloads: &crate::filters::KnownPayload,
    vocabulary: &crate::filters::Vocabulary,
) -> Result<()> {
    let (index, regenerated) = fresh_index(
        index_path,
        root,
        windows,
        payloads,
        vocabulary,
        crate::filters::Readability::of_opt_out(cli.no_readable),
    )?;
    let needle = query.needle.filter(|_| !query.verify_entries);

    // 1. Filter, keeping the index's own order.
    let mut selected: Vec<&crate::textindex::Instance> = Vec::new();
    for instance in &index.instances {
        if query.class.is_some_and(|class| instance.class != class) {
            continue;
        }
        if query
            .script
            .is_some_and(|script| instance.script.as_deref() != Some(script))
        {
            continue;
        }
        if query
            .language
            .is_some_and(|language| instance.language.as_deref() != Some(language))
        {
            continue;
        }
        if query.entry.is_some_and(|entry| instance.entry != entry) {
            continue;
        }
        if query
            .archive
            .is_some_and(|archive| !instance.archive.contains(archive))
        {
            continue;
        }
        selected.push(instance);
    }

    // 2. Group consecutive instances of one subfile. A phrase may straddle the edge between two
    //    windows of the same subfile, so each group is matched as one joined reading.
    let mut groups: Vec<Vec<&crate::textindex::Instance>> = Vec::new();
    for instance in &selected {
        let continues = groups.last().is_some_and(|group| {
            let previous = group[group.len() - 1];
            previous.archive == instance.archive
                && previous.entry == instance.entry
                && crate::search::continues_run(
                    (previous.class, previous.offset, previous.length),
                    (instance.class, instance.offset),
                )
        });
        if continues {
            groups.last_mut().expect("checked").push(instance);
        } else {
            groups.push(vec![instance]);
        }
    }

    // 3. Decide which groups matched.
    let mut hits: Vec<(
        Vec<&crate::textindex::Instance>,
        Option<String>,
        Option<&'static str>,
    )> = Vec::new();
    for group in &groups {
        match needle {
            None => hits.push((group.clone(), None, None)),
            Some(needle) => {
                let spaced = crate::search::join_instance_readings(
                    &group
                        .iter()
                        .map(|instance| {
                            (instance.offset, instance.length, instance.reading.as_str())
                        })
                        .collect::<Vec<_>>(),
                );
                let bare = crate::search::join_instance_readings(
                    &group
                        .iter()
                        .map(|instance| {
                            (
                                instance.offset,
                                instance.length,
                                instance
                                    .joined
                                    .as_deref()
                                    .unwrap_or(instance.reading.as_str()),
                            )
                        })
                        .collect::<Vec<_>>(),
                );
                if let Some((start, end)) =
                    crate::search::find_in_reading_pair(&spaced, &bare, needle, query.ignore_case)
                {
                    let matched: String = spaced.chars().skip(start).take(end - start).collect();
                    hits.push((group.clone(), Some(matched), Some("command-aware")));
                }
            }
        }
    }
    let total = hits.len();
    // Integrity mode reports how many *instances* it resolved, which is what was verified; a hit
    // covers a group of consecutive windows, so the group count would understate the evidence.
    let verified_instances: usize = hits.iter().map(|(group, _, _)| group.len()).sum();
    let selected_instances = selected.len();
    if query.limit > 0 {
        hits.truncate(query.limit);
    }

    // 4. Resolve every hit against its own archive, verifying rather than trusting the index.
    let mut cache: Vec<(PathBuf, Vec<u8>)> = Vec::new();
    let mut results = Vec::new();
    for (group, matched, reading) in &hits {
        let first = group[0];
        let last = group[group.len() - 1];
        let archive_path = root.join(&first.archive);
        if !cache.iter().any(|(path, _)| *path == archive_path) {
            cache.push((archive_path.clone(), read(&archive_path, "archive")?));
        }
        let data = &cache
            .iter()
            .find(|(path, _)| *path == archive_path)
            .expect("just cached")
            .1;
        let entries = crate::lexagraph::entries(data)?;
        let (entry, payload) = entries
            .into_iter()
            .find(|(entry, _)| entry.index == first.entry)
            .ok_or_else(|| {
                Error::Invalid(format!(
                    "{} has no subfile #{}, which the index records",
                    first.archive, first.entry
                ))
            })?;
        // Each instance must still reproduce from the archive.
        for instance in group {
            let begin = instance.offset.checked_sub(entry.offset).ok_or_else(|| {
                Error::Invalid(format!(
                    "index offset {} precedes subfile #{} of {}",
                    instance.offset, instance.entry, instance.archive
                ))
            })?;
            let finish = begin + instance.length;
            if finish > payload.len() {
                return invalid(format!(
                    "index entry {}#{} at {} runs past its subfile; rebuild the index",
                    instance.archive, instance.entry, instance.offset
                ));
            }
            let (spaced, joined) = crate::search::readings(&payload[begin..finish]);
            let fresh_joined = (joined.text != spaced.text).then(|| joined.text.clone());
            if spaced.text != instance.reading
                || fresh_joined.as_deref() != instance.joined.as_deref()
            {
                return invalid(format!(
                    "index entry {}#{} at {} does not reproduce from the archive (readings differ); rebuild the index",
                    instance.archive, instance.entry, instance.offset
                ));
            }
        }
        // The group's whole byte span is the unit a boundary-spanning phrase lives in.
        let group_start = first.offset - entry.offset;
        let group_end = last.offset - entry.offset + last.length;
        // Resolve through the same group matcher the other modes use, so all three agree.
        let spans: Vec<(usize, usize)> = group
            .iter()
            .map(|instance| (instance.offset - entry.offset, instance.length))
            .collect();
        let decoded = crate::command::deserialize(&payload[group_start..group_end]);
        let mut match_offset = None;
        let mut match_length = None;
        if let Some(needle) = needle {
            // `find_in_group` returns an inclusive start and an **exclusive end**; the length is
            // their difference, not the end offset.
            let Some((relative, end)) =
                crate::search::find_in_group(payload, &spans, needle, query.ignore_case)
            else {
                return invalid(format!(
                    "index hit {}#{} at {} is not reproducible in the archive; rebuild the index",
                    first.archive, first.entry, first.offset
                ));
            };
            match_offset = Some(entry.offset + relative);
            match_length = Some(end - relative);
        }
        // The string begins where a walk **back** from the match through the group's abutting
        // instances reaches, rather than at the first instance the grouping happened to record.
        let begin = match_offset
            .and_then(|offset| {
                crate::lexagraph::walk_back_to_chain_start(&spans, offset - entry.offset)
            })
            .unwrap_or(group_start);
        let anchored = (begin, group_end - begin);
        let anchor = if query.match_offset {
            "match"
        } else {
            "string"
        };
        let reported_offset = if query.match_offset {
            match_offset
        } else {
            Some(entry.offset + anchored.0)
        };
        let reported_length = if query.match_offset {
            match_length.unwrap_or(anchored.1)
        } else {
            anchored.1
        };
        results.push(serde_json::json!({
            "source": format!("{}#{}@{}", first.archive, first.entry, first.address),
            "archive": first.archive,
            "entry": first.entry,
            "address": first.address,
            "class": first.class,
            "instances": group.len(),
            "offset": first.offset,
            "length": group_end - group_start,
            "string_offset": entry.offset + anchored.0,
            "string_length": anchored.1,
            "match_offset": match_offset,
            "match_length": match_length,
            "anchor": anchor,
            "reported_offset": reported_offset,
            "reported_length": reported_length,
            "matched": matched,
            "reading": reading,
            "script": first.script,
            "language": first.language,
            "text": decoded,
        }));
    }
    info!(
        "{} of {} indexed instance(s) matched ({} shown) from {}",
        total,
        index.instances.len(),
        results.len(),
        index_path.display()
    );
    if query.verify_entries {
        if cli.json {
            println!(
                "{}",
                serde_json::to_string_pretty(&serde_json::json!({
                    "index": index_path.display().to_string(),
                    "root": root.display().to_string(),
                    "verified": verified_instances,
                    "groups": results.len(),
                    "of_selected": selected_instances,
                    "indexed": index.instances.len(),
                    "regenerated": regenerated,
                }))
                .map_err(|error| Error::Invalid(error.to_string()))?
            );
        } else {
            println!(
                "{} {}: {} of {} selected instance(s) reproduced from their archives ({} window group(s))",
                logging::success("verified"),
                index_path.display(),
                verified_instances,
                selected_instances,
                results.len()
            );
        }
        return Ok(());
    }
    if cli.json {
        println!(
            "{}",
            serde_json::to_string_pretty(&serde_json::json!({
                "index": index_path.display().to_string(),
                "root": root.display().to_string(),
                "indexed": index.instances.len(),
                "regenerated": regenerated,
                "matched": total,
                "shown": results.len(),
                "results": results,
            }))
            .map_err(|error| Error::Invalid(error.to_string()))?
        );
        return Ok(());
    }
    println!(
        "{} {}: {} of {} indexed instance(s) matched",
        logging::heading("search"),
        index_path.display(),
        total,
        index.instances.len()
    );
    for hit in results
        .iter()
        .take(if query.limit > 0 { query.limit } else { 20 })
    {
        let anchor = if query.match_offset {
            hit["match_offset"]
                .as_u64()
                .or_else(|| hit["string_offset"].as_u64())
        } else {
            hit["string_offset"]
                .as_u64()
                .or_else(|| hit["offset"].as_u64())
        };
        println!(
            "  {} +{:#x} {}",
            hit["source"].as_str().unwrap_or(""),
            anchor.unwrap_or(0),
            hit["text"].as_str().unwrap_or("")
        );
    }
    Ok(())
}

fn run_build_index(
    cli: &Cli,
    root: &Path,
    out: &Path,
    check: bool,
    force: bool,
    filters: &crate::filters::ScanFilters<'_>,
) -> Result<()> {
    let current = crate::textindex::current_hash(root)?;
    let filters_hash = filters.fingerprint()?;
    if check {
        let index = crate::textindex::load(out)?;
        let fresh = index.is_fresh(&current, &filters_hash);
        info!(
            "{} is {} ({} instance(s), {} archive(s))",
            out.display(),
            if fresh { "fresh" } else { "stale" },
            index.instances.len(),
            index.inputs.archives
        );
        if cli.json {
            println!(
                "{}",
                serde_json::to_string_pretty(&serde_json::json!({
                    "index": out.display().to_string(),
                    "fresh": fresh,
                    "indexed": {"archives": index.inputs.archives, "instances": index.instances.len(), "hash": index.inputs.hash, "filters_hash": index.inputs.filters_hash},
                    "current": {"archives": current.archives, "hash": current.hash, "filters_hash": filters_hash},
                }))
                .map_err(|error| Error::Invalid(error.to_string()))?
            );
        } else if fresh {
            println!(
                "{} {}: inputs match ({} instance(s) over {} archive(s))",
                logging::success("fresh"),
                out.display(),
                index.instances.len(),
                index.inputs.archives
            );
        } else {
            println!(
                "{} {}: inputs changed (archive {} -> {}; filters {} -> {}); rebuild it",
                logging::heading("stale"),
                out.display(),
                index.inputs.hash,
                current.hash,
                index.inputs.filters_hash,
                filters_hash
            );
        }
        if fresh {
            return Ok(());
        }
        return invalid("the index is stale; rebuild it with `build-index` (or --force)");
    }
    if !force && out.is_file() {
        if let Ok(existing) = crate::textindex::load(out) {
            if existing.is_fresh(&current, &filters_hash) {
                info!("{} is already fresh; nothing to do", out.display());
                if cli.json {
                    println!(
                        "{}",
                        serde_json::to_string_pretty(&serde_json::json!({
                            "index": out.display().to_string(),
                            "rebuilt": false,
                            "inputs": existing.inputs,
                            "counts": existing.counts,
                            "instances": existing.instances.len(),
                        }))
                        .map_err(|error| Error::Invalid(error.to_string()))?
                    );
                } else {
                    println!(
                        "{} {} already fresh: {} instance(s) over {} archive(s)",
                        logging::success("index"),
                        out.display(),
                        existing.instances.len(),
                        existing.inputs.archives
                    );
                }
                return Ok(());
            }
        }
    }
    let index = crate::textindex::build(
        root,
        filters.windows,
        filters.payloads,
        filters.vocabulary,
        filters.readability(),
    )?;
    crate::textindex::write(&index, out)?;
    info!(
        "indexed {} instance(s) over {} archive(s) into {}",
        index.instances.len(),
        index.inputs.archives,
        out.display()
    );
    for (class, count) in &index.counts {
        info!("  {class}: {count}");
    }
    for entry in &index.skipped {
        info!("  skipped {entry}");
    }
    if cli.json {
        println!(
            "{}",
            serde_json::to_string_pretty(&serde_json::json!({
                "index": out.display().to_string(),
                "rebuilt": true,
                "inputs": index.inputs,
                "counts": index.counts,
                "skipped": index.skipped,
                "instances": index.instances.len(),
            }))
            .map_err(|error| Error::Invalid(error.to_string()))?
        );
    } else {
        println!(
            "{} {}: {} instance(s) over {} archive(s), {} skipped",
            logging::success("indexed"),
            out.display(),
            index.instances.len(),
            index.inputs.archives,
            index.skipped.len()
        );
        for (class, count) in &index.counts {
            println!("  {class:<10} {count}");
        }
        println!(
            "  windows    {} barrier(s), {} byte(s) skipped, {} latch(es) removed, {} split",
            index.inputs.windows.windows,
            index.inputs.windows.bytes_skipped,
            index.inputs.windows.latches_removed,
            index.inputs.windows.latches_split
        );
    }
    Ok(())
}

fn run_index(
    cli: &Cli,
    archive: &Path,
    class_filter: Option<&str>,
    entry_filter: Option<usize>,
) -> Result<()> {
    let data = read(archive, "archive")?;
    let wanted = resolve_class(class_filter)?;
    let mut entries = query::index(&data)?;
    if wanted.is_some() || entry_filter.is_some() {
        entries.retain(|entry| {
            wanted.is_none_or(|class| entry.class == class)
                && entry_filter.is_none_or(|index| entry.entry == index)
        });
        if entries.is_empty() {
            return invalid(match (wanted, entry_filter) {
                (Some(class), Some(index)) => {
                    format!("archive has no {class} text subfile at entry #{index}")
                }
                (Some(class), None) => format!("archive has no {class} text subfile"),
                (None, Some(index)) => format!("archive has no text subfile at entry #{index}"),
                (None, None) => "archive has no matching text subfile".to_string(),
            });
        }
    }
    if entries.is_empty() {
        return invalid("archive has no text subfile (load argument 0x80010000 or 0x8001A000)");
    }
    info!("{} text subfile(s) in {}", entries.len(), archive.display());
    if cli.json {
        let value = serde_json::json!({
            "archive": archive_label(archive),
            "entries": entries,
        });
        println!(
            "{}",
            serde_json::to_string_pretty(&value)
                .map_err(|error| Error::Invalid(error.to_string()))?
        );
        return Ok(());
    }
    println!(
        "{}",
        logging::heading(&format!(
            "{:<6} {:<10} {:<10} {:<10} {:<9} {:<6} {:<6} {:<6} {:<8} {}",
            "ENTRY",
            "OFFSET",
            "RAM_PTR",
            "SIZE",
            "CLASS",
            "BANKS",
            "ROWS",
            "USED",
            "OPTIONS",
            "STATUS"
        ))
    );
    for entry in &entries {
        let status = if entry.status == "ok" {
            logging::success(&entry.status)
        } else {
            logging::failure(&entry.status)
        };
        println!(
            "{:<6} {:#010x} {:<10} {:<10} {:<9} {:<6} {:<6} {:<6} {:<8} {}",
            entry.entry,
            entry.offset,
            entry.ram_ptr,
            entry.size,
            entry.class,
            entry.banks,
            entry.rows,
            entry.used,
            entry.options,
            status
        );
    }
    Ok(())
}

fn run_extract(
    cli: &Cli,
    archive: &Path,
    output: &Path,
    entry_spec: &str,
    class_filter: Option<&str>,
    latch: Option<&str>,
) -> Result<()> {
    let data = read(archive, "archive")?;
    if let Some(spec) = latch {
        return extract_latch(cli, archive, output, &data, spec);
    }
    let selected = select(&data, entry_spec, resolve_class(class_filter)?)?;
    let multiple = selected.len() > 1;
    if multiple {
        fs::create_dir_all(output)
            .map_err(|error| Error::Invalid(format!("{}: {error}", output.display())))?;
    }
    for (entry, class, payload) in &selected {
        let section = extract::parse_section(payload)?;
        let source = source_for(archive, *entry, *class, multiple);
        let document = extract::document(&section, &source);
        let destination = if multiple {
            output.join(format!("{}.json", entry.index))
        } else {
            output.to_path_buf()
        };
        let body = syntax::render_json(&document)?;
        write_output(&destination, body.as_bytes(), archive)?;
        info!(
            "{source}: {} bank(s), {} rows, {} used, {} bytes -> {}",
            section.banks.len(),
            section.rows(),
            section.used(),
            section.size,
            destination.display()
        );
        if !cli.json {
            println!(
                "{} {source} ({} bank(s), {} rows, {} used) -> {}",
                logging::success("extracted"),
                section.banks.len(),
                section.rows(),
                section.used(),
                destination.display()
            );
        }
    }
    Ok(())
}

fn run_validate(cli: &Cli, path: &Path, commands: bool) -> Result<()> {
    let document = read_json_document(path)?;
    info!(
        "{}: {} bank(s), {} rows, {} non-empty",
        path.display(),
        document.banks.len(),
        document.rows(),
        document.used()
    );
    let summaries = if commands {
        document.summaries()
    } else {
        Vec::new()
    };
    if cli.json {
        let value = serde_json::json!({
            "document": archive_label(path),
            "source": document.source.to_string(),
            "class": document.source.class,
            "fingerprint": document.fingerprint,
            "banks": document.bank_sizes(),
            "rows": document.rows(),
            "used": document.used(),
            "summaries": summaries,
        });
        println!(
            "{}",
            serde_json::to_string_pretty(&value)
                .map_err(|error| Error::Invalid(error.to_string()))?
        );
        return Ok(());
    }
    println!(
        "{} {}: {} bank(s), {} rows, {} non-empty, from {}",
        logging::success("ok"),
        path.display(),
        document.banks.len(),
        document.rows(),
        document.used(),
        document.source
    );
    for summary in summaries {
        println!("{summary}");
    }
    Ok(())
}

/// Everything one `query` invocation selects and how it reports a hit.
struct QueryOptions<'a> {
    entry_spec: &'a str,
    needle: Option<&'a str>,
    commands: bool,
    class_filter: Option<&'a str>,
    ignore_case: bool,
    match_offset: bool,
}

fn run_query(
    cli: &Cli,
    archive: &Path,
    options: &QueryOptions<'_>,
    windows: &crate::windows::WindowIndex,
    payloads: &crate::filters::KnownPayload,
    vocabulary: &crate::filters::Vocabulary,
) -> Result<()> {
    let QueryOptions {
        entry_spec,
        needle,
        commands,
        class_filter,
        ignore_case,
        match_offset,
    } = *options;
    let data = read(archive, "archive")?;
    if resolve_class(class_filter)? == Some(TextClass::Heuristic) {
        return query_latches(cli, archive, &data, options, windows, payloads, vocabulary);
    }
    let selected = select(&data, entry_spec, resolve_class(class_filter)?)?;
    let mut report = Vec::new();
    for (entry, class, payload) in &selected {
        let section = extract::parse_section(payload)?;
        let source = source_for(archive, *entry, *class, selected.len() > 1).to_string();
        if commands {
            let usage: Vec<serde_json::Value> = query::command_usage(&section)
                .iter()
                .map(|(name, count)| serde_json::json!({"command": name, "count": count}))
                .collect();
            info!("{source}: {} distinct command(s)", usage.len());
            if cli.json {
                report.push(serde_json::json!({"source": source, "usage": usage}));
            } else {
                println!(
                    "{} {source}: {} distinct command(s)",
                    logging::heading("usage"),
                    usage.len()
                );
                for (name, count) in query::command_usage(&section) {
                    println!("  {{{name}}}\t{count}");
                }
            }
            continue;
        }
        let matches = query::query(&section, needle, ignore_case, entry.offset, match_offset);
        info!(
            "{source}: {} of {} rows match",
            matches.len(),
            section.rows()
        );
        if cli.json {
            report.push(serde_json::json!({
                "source": source,
                "matched": matches.len(),
                "rows": section.rows(),
                "matches": query::query_json(&matches)?,
            }));
            continue;
        }
        println!(
            "{} {source}: {} of {} rows match",
            logging::heading("query"),
            matches.len(),
            section.rows()
        );
        for hit in matches {
            // The anchor and the length describe the same thing: the containing string by default,
            // the match itself under `--match-offset`.
            let (anchor, span) = if options.match_offset {
                (
                    hit.match_offset.or(hit.string_offset).unwrap_or(hit.offset),
                    hit.match_length.unwrap_or(hit.length),
                )
            } else {
                (hit.string_offset.unwrap_or(hit.offset), hit.string_length)
            };
            println!(
                "b{}.{}\t+{anchor:#x}\t{span}B{}\t{}",
                hit.bank,
                hit.number,
                if hit.options { "+opt" } else { "" },
                hit.text
            );
        }
    }
    if cli.json {
        println!(
            "{}",
            serde_json::to_string_pretty(&serde_json::json!({ "results": report }))
                .map_err(|error| Error::Invalid(error.to_string()))?
        );
    }
    Ok(())
}

fn run_pack(
    cli: &Cli,
    original: &Path,
    text_path: &Path,
    output: &Path,
    entry_spec: &str,
    class_filter: Option<&str>,
    latch: Option<&str>,
) -> Result<()> {
    if output == original {
        return invalid("pack refuses to overwrite the original archive");
    }
    let data = read(original, "archive")?;
    if latch.is_some() || text_path.is_file() {
        let document = read_json_document(text_path)?;
        if document.latch.is_some() || latch.is_some() {
            return pack_latch(cli, original, text_path, output, &data, document, latch);
        }
    }
    let selected = select(&data, entry_spec, resolve_class(class_filter)?)?;
    let multiple = selected.len() > 1;
    let mut packed = data.clone();
    let mut areas = Vec::new();
    for (entry, class, payload) in &selected {
        let section = extract::parse_section(payload)?;
        let document_path = if multiple {
            text_path.join(format!("{}.json", entry.index))
        } else {
            text_path.to_path_buf()
        };
        let document = read_json_document(&document_path)?;
        let expected = source_for(original, *entry, *class, multiple);
        if document.source.address != expected.address || document.source.class != expected.class {
            return invalid(format!(
                "document {} came from {} but is being packed as {}",
                document_path.display(),
                document.source,
                expected
            ));
        }
        if let Some(stated) = &document.fingerprint {
            let actual = extract::fingerprint(&section);
            if *stated != actual {
                return invalid(format!(
                    "document fingerprint {stated} does not match this block's {actual}; re-extract before editing"
                ));
            }
        }
        let rebuilt = pack::rebuild(&section, &document)?;
        packed = pack::replace_payload(&packed, *entry, &rebuilt)?;
        info!(
            "packed {} entry {} ({} bytes)",
            document.source,
            entry.index,
            rebuilt.len()
        );
        areas.push(document.source.to_string());
    }
    write_output(output, &packed, original)?;
    if cli.json {
        println!(
            "{}",
            serde_json::to_string_pretty(&serde_json::json!({
                "sources": areas,
                "output": archive_label(output),
                "bytes": packed.len(),
            }))
            .map_err(|error| Error::Invalid(error.to_string()))?
        );
    } else {
        println!(
            "{} {} -> {} ({} bytes, unchanged size)",
            logging::success("packed"),
            areas.join(", "),
            output.display(),
            packed.len()
        );
    }
    Ok(())
}
