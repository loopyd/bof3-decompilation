# Commands

```
bof3-text [-v]... [-n|--no-color] [--json] <MODE> ...
```

Global flags may appear before or after the mode. `-nc` is accepted as a spelling
of `--no-color`.

| Flag | Meaning |
| --- | --- |
| `-v`, `-vv`, `-vvv`, `-vvvv` | Log verbosity: info, debug, trace, trace ([logging.md](logging.md)) |
| `-n`, `--no-color`, `-nc` | Disable coloured output (also `NO_COLOR`) |
| `--json` | Emit JSON for command output **and** for log records |

Exit status: `0` success, `1` a tool error (unsupported data, refused write,
malformed document, capacity overflow), `2` a usage error from the argument parser.

## `prepare [--root DIR] [--targets DIR] [--out-dir DIR]`

Prepares corpus search from extracted US archives without existing generated text
artifacts. Defaults are `out/extracted/BIN`, `config/targets/emi` and `out`.
The command derives vocabulary directly from verified bank rows, copies the
crate's payload-inventory policy only if the destination is absent, rebuilds the
classified-window registry, and builds or reuses a fresh filtered index. Invalid
bank data, inventory or reviewed target records fail rather than disabling filters.
The bundled inventory retains the US calibration measurements and source records;
those measurements are not recomputed for a different corpus.

Outputs live under `OUT_DIR/text-payloads`, `text-vocabulary`, `text-windows` and
`text-index`. An existing payload inventory is preserved; other generated
artifacts may be refreshed. `--json` emits one object naming the artifacts,
archive/instance counts and whether the index was regenerated. Filter opt-outs
and global inventory/vocabulary overrides are refused by this command.

For an alternate output directory, subsequent search must name its artifacts:

```sh
bin/harness text prepare --out-dir /tmp/bof3-search
bin/harness text search --grep McNeil \
  --index /tmp/bof3-search/text-index/index.json \
  --windows /tmp/bof3-search/text-windows/registry.json \
  --payloads /tmp/bof3-search/text-payloads/inventory.json \
  --vocabulary /tmp/bof3-search/text-vocabulary/vocabulary.json
```

`index`, `extract`, `validate`, `pack`, `probe`, `verify`, `payloads`, `windows`
and `vocabulary` do not load an unrelated generated vocabulary at dispatch.
Search/scan/readability commands still require their actual filter inputs.

## `index ARCHIVE.EMI [--class CLASS] [-e|--entry N]`

Lists every text subfile (EMI entries whose TOC load argument is a known text bank),
with its class, and whether it parses.

```
ENTRY  OFFSET     RAM_PTR    SIZE       CLASS     BANKS  ROWS   USED   OPTIONS  STATUS
11     0x000b8000 0x80010000 9170       dialogue 1      256    130    2        ok
```

`--class` and `-e/--entry` restrict the listing, and an absent class is refused with the
classes the archive does contain. `--json` emits `{"archive": ..., "entries": [...]}` where each entry carries
`entry`, `offset`, `ram_ptr`, `size`, `class`, `banks`, `rows`, `used`, `options`
and a `status` string. `options` counts rows whose extent carries a choice option
list after its `{end}`.

## `extract ARCHIVE.EMI -o|--output OUT [-e|--entry N] [--class CLASS] [--latch ENTRY:OFFSET+LENGTH]`

Writes the selected text subfile as an editable document and reports the counts.
`--entry` is only needed when an archive holds more than one text subfile (US
archives hold exactly one dialogue subfile). `--class` restricts the selection; an
absent class is refused with the classes the archive does contain. The destination
is created and never overwritten.

`--latch ENTRY:OFFSET+LENGTH` extracts one raw-text instance (class `heuristic`)
found by `scan` instead of a whole subfile; `OFFSET` and `LENGTH` accept decimal or
`0x`-prefixed hex. The instance is written with an explicit `latch` extent and a span
fingerprint, and an extent that runs past its subfile is refused as unbounded.

Output is always JSON — there is no second codec — and every object carries a
`source` identifying the subfile and its class. Packing is driven by the
document's own recorded `source`, so no format detection happens at pack time.

## `scan ARCHIVE.EMI [--min-run N] [--class CLASS] [-e|--entry N] [--windows PATH] [--no-windows]`

Three flags are **global** and therefore apply to every mode rather than being repeated in each
section: `--payloads PATH` (default `out/text-payloads/inventory.json`) names the payload inventory,
`--no-payloads` searches without the exclusion, and `--no-readable` reports every candidate the
scanner forms instead of only those carrying a word — both opt-outs are for comparison only. A missing inventory is an
error on the searching path, not "nothing is excluded".

Every mode that latches raw text — `scan`, `build-index`, `map` and `search` when it regenerates the
index — takes `--windows PATH` (default `out/text-windows/registry.json`) and `--no-windows`. A
registered window is a **barrier**: its bytes are never latched, a run that would straddle it closes
at its start and a fresh run begins at its end, so unclassified bytes on either side stay latchable
as separate runs and no unclassified byte is dropped. Each mode reports what the check did — barriers
applied, bytes skipped, latches removed and latches split — in its summary and in `--json`, so a
removal is never silent. `--no-windows` exists for comparison and should not be used in a gate.


Latches candidate raw-text runs in **every** subfile, not just the text banks, and
reports each one's offset, length, decoded preview, decode ratio, letter count and
distinct-byte count. `--min-run` sets the minimum decoded characters (default 12).

`--class` restricts scanning to subfiles of that class and `-e/--entry` to one subfile.

These are **heuristic leads, not consumer-verified text**: the `--json` output
carries `"heuristic": true` and an explicit note. Discovery is bounded (a fixed run
cap, a gap tolerance and ratio floors), so a missing latch is not evidence of
absence.

## `validate DOCUMENT [--commands]`

Checks the document's header, bank structure, row numbering and every command
token, without needing the original archive. `--commands` adds one summary line
per non-empty row (`b<bank>.<row>`, character count, commands used). `--json`
emits the same facts as an object.

Validation is structural: it cannot know whether an edit still fits the original
allocation. `pack` performs that check.

## `query ARCHIVE.EMI [-e|--entry N] [--grep TEXT] [-i|--ignore-case] [--commands] [--class ...] [--windows PATH] [--no-windows]`

Lists rows, optionally filtered by a **command-transparent** text match, showing
`bank.row`, the bank-relative byte offset, span length and decoded text. When a match is
found the row also reports the match's own byte offset and length and which reading
produced it. `--commands` prints a command-usage histogram instead. Both modes support
`--json`.

Control commands and their operands are transparent to `--grep`, so a phrase the game
breaks across a command is found (`--grep "it'll be a good"` matches
`it'll{nl}be a good crop`) and **raw command bytes are never matchable**: `--grep end`
does not match a `{end}` terminator, and `--grep 0x81` does not match an operand. Token
syntax is therefore no longer searchable — use `--commands` for command usage. `-i`
folds ASCII case. See `out/text-format/search-semantics.md`.

**`query` and `search` use one matcher.** Both route through `search::find_in_instance`, so the
same needle answers the same way in either mode: a command-derived break may be skipped or read as
a single space, and token names and operand bytes are never matchable. `--class heuristic` searches
the decoded raw-text instances with that same matcher; matches are reported as heuristic leads, not
verified text.

**A hit's extent is a true archive byte range.** `match_offset` is an offset in the archive file and
`match_length` is its length **in bytes**, in every mode, so the bytes at
`match_offset .. match_offset + match_length` are the literals that matched. The pre-existing `offset`
field keeps its per-mode base (bank-relative for a banked row, payload-relative for a raw-text run).

**A hit is anchored on its string, not on its match, unless you ask otherwise.** Every mode reports
two anchors as offsets in the archive file:

| Field | Meaning |
| --- | --- |
| `string_offset`, `string_length` | The containing **string** — the row extent for a banked row, or the raw-text run for a heuristic hit |
| `match_offset`, `match_length` | The matched literals themselves |

The human line prints the containing string by default, and `--match-offset` switches it to the match
start; the offset and its length always describe the same one of the two, so the pair is never mixed.
`--json` always carries both, so the match start is never lost. In a row where the match begins
part-way in — `WORLD00/AREA000.EMI#11` row 50, whose extent starts at `0xb8c65` and whose
`heroes, eh? Saved the village` match starts at `0xb8c91` — the two anchors differ and each range
decodes to what it claims.

## `verify [--root DIR]`

Round-trips every text subfile of a **known** class across the corpus and reports the result. This
is the tool's own gate: it proves the bytes it claims to understand are reproduced exactly.

```
$ bin/harness text verify
verified out/extracted/BIN: 244 of 244 text subfile(s) over 880 archive(s) round-trip byte-identically
  battle     44/44
  dialogue   200/200
```

Subfiles of an unknown class are not judged here — `probe` is the mode for those. Any failure is
listed with its archive, entry and reason (a size mismatch, a parse error, a rebuild error) and the
exit status is non-zero, so an offender is never hidden behind a summary count.

## `windows [--root DIR] [--targets DIR] [--out PATH] [--json]`

Builds the classified-window registry: the byte ranges an existing owner already accounts for, so
heuristic latching does not run inside them. Windows come from the sources that already know — the
**parse** (a known-class subfile's whole payload, its pointer table and every row extent, read from the
crate's own parse rather than re-derived), the **payload inventory** (each identified subfile as a
`known_payload` window naming its record and check), and the **reviewed records** (the `textbin`
subsegments the boundary review produced, each cross-checked against its target's load address). Nothing
is inferred from a byte pattern: a window exists only because a parse, the inventory or a reviewed record
says so, and a window with no named owner is refused rather than invented.

## `vocabulary [--index PATH] [--out PATH]`

Derives the vocabulary the readability rule attests words against, from the verified rows in the index:
ASCII words of three or more letters, lowercased, with a sources block recording where they came from.
Documentation is deliberately **not** scanned — the project's own prose holds identifiers (`FFF`, `xxx`)
that would let byte noise pass attestation — and the document shape is fixed, so the artifact is
byte-identical across regenerations.

## `readability [--text TEXT]... [--index PATH]`

Applies the **actual** readability rule — the shape thresholds and the vocabulary corroboration, composed
in `filters.rs` — either to the text given with `--text` or to every verified row's reading in the index,
and reports how many rows are shaped, readable and attested. It exists so a calibration can measure the
real thresholds rather than a re-implementation of them.

## `payloads [--root DIR]`

Lists the payloads the inventory identifies, one row per identified subfile, with the family, the
magic, the record that names it and the structural check that confirmed it. This is what
`bin/harness text windows` reads to register `known_payload` windows, so the identification lives in
one place (`filters.rs`) and the checks are not re-implemented.

## `map [--root DIR] [--out PATH] [--windows PATH] [--no-windows]`

Writes a **read-only** text-versus-data map of every subfile in every archive: role, byte range,
the evidence used and an `unidentified` marker. It proposes roles and changes nothing — no
`splat.yaml`, `target.toml` or any segment config is touched.

```
$ bin/harness text map
mapped out/text-format/segment-map.json: 6344 subfile(s) over 880 archive(s)
  verified text   244
  data            6100
  identified data 1139 (1139 by magic)
  unidentified    4961
  candidates      12884 text-like run(s) in 2200 data subfile(s) (candidates only)
  in text banks   311 run(s) belonging to the verified text itself
  this report proposes roles and changes nothing
```

A range is `text` only when it parses as a section **and** reproduces byte-identically **and** its
class has evidence; everything else is `data`. Text-like runs inside data are listed as candidates,
never as text ranges. The report deliberately does **not** encode the EU-knowledgebase ctype
taxonomy, so it asserts only what the US bytes show. See `out/text-format/segment-map.md`.

## `probe ARCHIVE.EMI [--round-trip]`

Probes **every** subfile of an archive for text-bank structure — a pointer table whose rows tile
the string region — **without consulting class membership**, so an unknown load argument is judged
on the same terms as `dialogue`. `--round-trip` also rebuilds each parsed section and reports
whether it reproduces the payload byte-for-byte.

```
$ bin/harness text probe out/extracted/BIN/WORLD00/AREA000.EMI --round-trip
probe …AREA000.EMI: 1 of 16 subfile(s) parse as a text section, 1 reproduce byte-identically
  #11 0x80010000 dialogue banks=1 rows=256 used=130 round-trip=identical
```

This is the structural evidence a class promotion may rest on, and its positive control is
built in: it parses and reproduces the two consumer-verified classes, and reports 0 of 6,100
subfiles for every other load argument in the corpus. See
`out/text-format/class-promotion.md`.

## `search [--index PATH] [--root DIR] [--grep TEXT] [-i|--ignore-case] [--class ...] [--script S] [--language L] [-e|--entry N] [--archive TEXT] [--limit N] [--match-offset] [--windows PATH] [--no-windows] [--verify-entries]`

Searches the **retained corpus index** (`build-index`) instead of one archive at a time,
using the same command-transparent matcher as `query`.

```
$ bin/harness text search --grep "it'll be a good"
search out/text-index/index.json: 1 of 46057 indexed instance(s) matched
  WORLD00/AREA000.EMI#11@0x80010000 +0xb8228 "It looks like it'll{nl}be a good crop this…
```

Filters: `--class`, `--script` (e.g. `Latin`), `--language` (e.g. `eng`; only guesses the
detector called reliable are stored), `-e/--entry`, `--archive` (path substring), and
`--limit` (0 reports every hit).

Freshness is decided by the **content digest**, never by the stat manifest: a manifest entry
keeps a size and a whole-second mtime that can both survive a content change, and a search whose
hits were all served from the index would otherwise answer from stale data without noticing.
A stale or missing index is **regenerated and then used**, and the run reports that (the `--json`
output carries `"regenerated": true`). `build-index --check` is the refusing mode for a caller
that wants to detect staleness without writing.

Freshness also binds the loaded ownership registry, payload inventory, vocabulary
and readability settings through `inputs.filters_hash`. Changing their contents
or using `--no-windows`, `--no-payloads`, `--no-readable` or `--no-vocabulary`
invalidates the index. Older indexes without this field are rebuilt. Use a
separate `--index` path for comparison runs to avoid rebuilding the default index
when switching back to normal search.

**A command may be skipped or read as a space, whichever makes the needle fit.** The tool does not
reconstruct a command's displayed width, so a command-derived break is transparent in the sense of
being *either* zero-width *or* a single space: `heroes{/color}, eh? Saved{nl}the village` answers
`heroes, eh? Saved the village`, `Savedthe` and `Saved the`. Literal bytes must still match exactly,
so token names and operand bytes are never matchable.

**Readings are whitespace-collapsed but never trimmed.** The game's space is a literal byte, so a
separator at the start or end of an instance's reading is data: it is kept, and a literal space
therefore survives a window boundary instead of being deleted.

**Matching is per subfile, and a needle never spans two rows.** Each banked row is matched on its
own, so a phrase assembled from the end of one row and the start of the next does not match. The
one exception is a raw-text run the scanner had to split at its window cap: consecutive `heuristic`
instances of the same subfile whose extents **abut** are joined before matching, and only those, so
a phrase straddling that edge is found while independent rows stay independent. A hit reports the
true byte range of the match, mapped through per-character byte marks of the same representation the
index was matched on.

`--verify-entries` is an integrity check rather than a search: it resolves every selected
instance against its own archive and proves both readings reproduce, reporting `verified N of M`
and failing closed on any mismatch.

Because the artifact stores readings but no per-character byte marks, each hit is resolved
against its owning archive, and that resolution is **verified**: if the archive does not
reproduce the entry's readings, `search` fails closed instead of printing a stale offset.

## `build-index [--root DIR] [--out PATH] [--check] [--force] [--windows PATH] [--no-windows]`

Builds the retained **corpus text index**: one artifact recording where text lives across
many archives, so a corpus query is a scan of that file rather than of the disc.

```
$ bin/harness text build-index
indexed out/text-index/index.json: 46057 instance(s) over 880 archive(s), 0 skipped
  battle     26936
  dialogue   19121
```

Each instance records its class, source (`archive`, `entry`, `address`), byte extent
(`offset`, `length`) and the **command-transparent reading** — the same field `--grep`
matches. `bank`/`number` identify a banked row; a non-banked subfile contributes the
scanner's latches instead. The header carries digests over archive paths, sizes and
contents and the index-building filter policy.

`--check` reports whether an existing index still matches the inputs and exits non-zero when
it does not, so a stale index can never be used silently. Without `--check` a stale or
missing index is rebuilt; a fresh one is left alone unless `--force` is given. Two runs over
unchanged inputs are byte-identical (no timestamp is stored).

Decoded text with `{token}` spellings is deliberately **not** stored: it is the one form that
must never be searched, and it dominated the artifact. A hit is rendered by re-reading that
single archive.

## `pack --original ARCHIVE.EMI --text DOCUMENT -o|--output OUT.EMI [-e|--entry N] [--class CLASS] [--latch ENTRY:OFFSET+LENGTH]`

Rebuilds the archive with the edited document and refuses to overwrite its own
source. Checks performed before writing:

1. The document parses and every row is syntactically valid.
2. The document's `block_fingerprint`, when present, matches the block it is
   being packed against — this catches a document extracted from a different
   archive or entry.
3. Bank count and per-bank row counts match the original.
4. Each edited row fits its original span; a row with no allocated span cannot
   receive text.
5. Rows sharing a span are edited identically, and partially overlapping rows are
   left unedited.

The offset tables are re-emitted verbatim and only row spans are rewritten, so an
unedited document reproduces the original archive byte-for-byte.

A raw-text (`heuristic`) document takes a different path: it is patched in place at
its recorded `latch` extent, and any edit that changes the encoded byte length is
refused, because no allocation is proven for raw text.

## Common idioms

```sh
# search a whole area for a phrase
bin/harness text query out/extracted/BIN/WORLD00/AREA000.EMI --grep McNeil

# see which control tokens an area uses
bin/harness text query out/extracted/BIN/WORLD00/AREA000.EMI --commands

# machine-readable inventory of every area
for e in out/extracted/BIN/WORLD00/AREA*.EMI; do
  bin/harness text index "$e" --json
done
```
