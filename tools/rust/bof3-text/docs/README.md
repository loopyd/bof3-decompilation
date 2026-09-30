# bof3-text

Owner-local documentation for the BOF3 text tool
(`tools/rust/bof3-text`, binary `bof3-text`, harness entry point
`bin/harness text`).

It indexes, scans, extracts, validates, queries, searches and repacks the text subfiles of a
BOF3 EMI archive as editable JSON text objects, each carrying the `source` and text class it
came from, and it builds a retained corpus index so a search covers every archive at once.
Three classes are modelled: `dialogue` and `battle` are located structurally, and `heuristic`
marks a raw-text instance found by scanning — a lead, not verified text. Original archives are
never modified.

## Documentation

| Document | Covers |
| --- | --- |
| [getting-started.md](getting-started.md) | Build, publish, run, harness integration, environment overrides |
| [commands.md](commands.md) | Every mode, flag, output shape and exit status |
| [formats.md](formats.md) | Dialogue subfile layout, terminators, character and command tables, the JSON codec and the `source` property |
| [logging.md](logging.md) | `-v`..`-vvvv` levels, colour control, JSON logs |
| [development.md](development.md) | Module layout, invariants, how to extend safely, validation commands |

The repository-wide format authority remains
[`docs/specs/formats/dialogue-text.md`](../../../../docs/specs/formats/dialogue-text.md);
these documents describe the tool that implements it.

## Quick start

```sh
# after extracting the US disc, prepare the corpus search artifacts
bin/harness text prepare

# search every extracted archive; phrases can cross game control codes
bin/harness text search --grep "it'll be a good" -i --limit 20

# restrict results and retain source locations in JSON
bin/harness text search --grep McNeil -i --class dialogue --json

# list the text subfiles in an archive
bin/harness text index out/extracted/BIN/WORLD00/AREA000.EMI

# extract to an editable JSON document, edit it, repack to a new archive
bin/harness text extract out/extracted/BIN/WORLD00/AREA000.EMI -o out/text/AREA000.json
$EDITOR out/text/AREA000.json
bin/harness text validate out/text/AREA000.json
bin/harness text pack --original out/extracted/BIN/WORLD00/AREA000.EMI \
    --text out/text/AREA000.json -o out/text/AREA000.EMI
```

Corpus search uses `out/extracted/BIN` and refreshes its retained index when
archive bytes, ownership windows, payload inventory, vocabulary or filter
settings change. Each hit names its archive, entry and containing string's byte
range; `--match-offset` selects the matching phrase's range. See
[commands.md](commands.md) for filters and [getting-started.md](getting-started.md)
for the Rust build and required local inputs.

`--json` switches both the command output and the log records to JSON, so the
same works with machine-readable documents:

```sh
bin/harness text extract out/extracted/BIN/WORLD00/AREA000.EMI -o out/text/AREA000.json --json
bin/harness text pack --original out/extracted/BIN/WORLD00/AREA000.EMI \
    --text out/text/AREA000.json -o out/text/AREA000.EMI
```

## Guarantees

- All **244** verified text subfiles parse, extract and repack **byte-identically** when
  unedited: `dialogue` (200 US area subfiles) and `battle` (44 two-bank battle/system
  subfiles). `bin/harness text verify` asserts this across the corpus in about half a second,
  and `just check` runs it.
- **Every filter and every threshold lives in `filters.rs`.** The contracts are declared in
  `models.rs`: a `PreFilter` answers which byte ranges may be latched at all, and a `PostFilter`
  decides whether a produced candidate is worth reporting. The classified-window barrier and the
  payload inventory are the pre-filters; the scanner's quality floors and the readability rule are the
  post-filters, and no consumer applies a filter of its own.
- **Identified non-text payloads are never searched.** `bin/harness text payloads` lists the payloads
  the inventory identifies — `pBAV` (audio) and `pQES` (music) today — each named by an in-repo record
  and confirmed by a structural check; they are excluded both as `known_payload` windows and by the
  pre-filter, and the removals are tallied per family.
- **A reported result carries a word.** After command tokens are stripped, a candidate must contain a
  word of at least three letters; candidates that decode but carry no word are dropped and counted.
- **A hit is anchored on its containing string, not on its match.** `query`, `search` and the
  raw-text query report the start of the containing row or run (the string object) by default, and
  `--match-offset` reports the match start instead; `--json` carries both anchors, each an offset in
  the archive file.
- **Heuristic latching never runs inside a classified window.** `bin/harness text windows` builds a
  registry of the ranges an existing owner already accounts for — a known-class subfile's whole
  payload, its pointer tables and row extents, and the reviewed `textbin` ranges — and the scanner
  treats each as a barrier: its bytes are never latched, no unclassified byte is dropped, and each
  latching mode reports what the check removed.
- **Search matches what a player reads and never matches raw bytes.** Control commands and
  their operands are transparent to `--grep`, so a phrase the game breaks across a command is
  found, while `--grep end` cannot match a `{end}` terminator and `--grep 0x81` cannot match an
  operand. Token syntax is therefore deliberately not searchable.
- The **corpus index** is deterministic (two runs over unchanged inputs are byte-identical) and
  carries both a cheap stat manifest and a content digest. Freshness is decided by the **content
  digest**, and a stale or missing index is **regenerated before it is used** — never answered from,
  and never left for the caller to rebuild (`search` reports `"regenerated": true`).
  `build-index --check` is the mode that refuses when a caller wants detection without a write, and
  `search --verify-entries` proves every indexed instance still reproduces from its archive.
- **No class is promoted on vocabulary or frequency.** `probe` parses every subfile
  independently of class, and it refutes every non-verified candidate: 0 of 6,100 subfiles
  outside `dialogue`/`battle` parse as a text-bank section, versus 200/200 and 44/44 for those
  two. Those ranges are graphics, palette, encounter and audio containers.
- Outputs are create-new: an existing destination is refused, and the source archive can never
  be reached through a different spelling, symlink or hard link. The destination's directory is
  created when missing.
- An edit must fit its row's extent — which also holds a choice row's option list; overflow, a
  row with no extent, and shared extents edited differently are refused rather than guessed.
- Raw-text instances round-trip byte-identically and edit in place, but only at the same byte
  length: no allocation is proven for raw text, so a size-changing edit is refused rather than
  relocated.
- A scan latch is a candidate: `scan` reports its measurements and labels it `heuristic`, and
  never promotes a lead to a verified class.
