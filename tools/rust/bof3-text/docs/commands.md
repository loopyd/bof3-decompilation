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

## `coverage [--root DIR] [--strict]`

Emits a deterministic JSON report for every regular file under the complete
extracted tree, default `out/extracted`. Each file has its relative path, size and
SHA-256. EMI reports partition headers, payloads and padding into known text,
structurally identified nontext, or unresolved ranges. Payload exclusions retain
the inventory's source records. The report also pins the inventory's SHA-256.
Symlinks, special files, non-UTF-8 paths and unreadable inputs fail rather than
disappearing. Paths retain their actual components; a literal backslash in a Linux
filename is not a directory separator.

`disc_membership` compares each original US file with the paths, sizes and hashes
in `config/original-us-disc.json`, compiled into the binary. It pins the
qualification's SHA-256, release and source-project hash, lists missing and changed
files, and counts additional inputs. Matching every listed original file clears
the physical-membership blocker. Additional files keep their existing range and
derivative classifications. Membership does not establish their roles or text
coverage. The harness rebuilds the binary when this qualification changes.

The byte-identical PCM WAV export of original US audio track 2 is
`verified-audio-export`, with its whole file classified as known nontext. This
requires the pinned size and SHA-256; changed or arbitrary WAV files stay
unresolved. See [the format contract](formats.md#cdda-audio-export).

With the exact original `SLUS_004.22` present, coverage also recognizes the
validated encoded audio bodies in the three pinned US XA files as
`partially-verified-xa`. Only those bodies become known nontext; surrounding
bytes and other sectors stay unresolved. See [XA body coverage](formats.md#xa-encoded-audio-bodies).

Supported US executable layouts are pinned outside their editable allocations.
Recognition establishes build identity, not complete text coverage. Other builds remain unrecognized;
EU and JP support is explicitly unverified. Unknown consumers, nonzero container
gaps and unverified non-EMI ranges stay unresolved. Supported executable ASCII
allocations carry their pointer and consumer evidence; other executable bytes
remain unresolved. Matching unpacked payload sidecars and
matching extraction manifests are marked as derivatives of their EMI archive,
so duplicate files do not count as independent sources.

`--strict` prints the report and exits 1 while coverage is incomplete. Executable
and remaining consumer evidence remain open, so the current report always marks
full US completion false. Successful bank parsing, matching physical membership
or an empty heuristic search cannot close those gaps. Runtime acceptance is separate.
The command requires the payload inventory named by `--payloads`; run `prepare`
first to create the default inventory, or name the crate's policy explicitly.

## `analyze INPUT`

Emits structured JSON for each verified bank row, including its archive and payload
offsets, encoded byte size, fixed capacity, decoded text and shared row numbers.
Commands carry their byte offset, opcode and operand bytes. Terminator offsets
exclude operand bytes. Nonempty strings following terminators are listed as
`trailing_string_segments`; a consumer trace establishes which are choice options.
Empty slots have zero capacity and no byte offset. Analysis reads original bytes;
`validate` checks edited document syntax and `pack` enforces edited byte capacity.
Supported executable, record and default character inputs emit
`bof3.text-analysis/v1` reports, retaining their source metadata and `strings` or
`names`. `source_schema` identifies the extraction format. For EMI inputs,
`inline_entries` lists embedded allocation reports beside the bank `entries`.
`monster_entries` lists encounter-template names with their source and copy
consumer metadata.
Each literal allocation reports `encoded_size`, `max_encoded_size`,
`required_terminator_bytes`, `required_length_bytes`, `remaining_capacity`,
whole-allocation hexadecimal `original_bytes` and an empty `commands` list,
alongside its offsets, capacity and any pointer references. Encoded size counts
the text's bytes. A required NUL reserves one byte for executable and
NUL-terminated inline strings. Counted inline labels reserve one trailing length byte and require no
terminator; packing updates that byte from the encoded text.
Fixed-width names may use their entire field. `encoding` distinguishes ASCII
from the game's literal character table. These reports are diagnostic; use
`extract` for editable documents and `validate` for proposed edits.

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
fingerprint and the entire original archive SHA-256, and an extent that runs past its subfile is refused as unbounded.

Output is always JSON — there is no second codec — and every object carries a
`source` identifying the subfile and its class. Packing is driven by the
document's own recorded `source`.

Each supported US `SLUS_004.22` or `LOGO/LOGO.EXE` executable exports one
`bof3.executable-text/v17` document. Re-extract older documents. Use the default
`--entry all` and omit `--class` and `--latch`; EMI selectors are refused for
executable input.
LOGO supports kernel, CD, GPU and MDEC diagnostics. Use `analyze SOURCE --json` for
supported allocations. Its direct printf literals refuse percent characters;
CD labels and image-operation names passed as `%s` data allow them. Other LOGO
text and runtime console execution remain unverified.
Keep each row's `consumers`, `format_directives` and output metadata unchanged.
The shared CD `?` row retains twelve pointer references. BIOS puts accepts
literal percent bytes; direct printf literals refuse them. Shared-word label
references retain empty `calls` arrays where the downstream use is unproven.
BIOS formats allow literal edits within capacity while retaining every original
directive exactly and in order. GPU callback formats record the word and its
initial BIOS stub; their reference calls are indirect. The ResetGraph address
and GPU timeout diagnostics record direct BIOS printf calls. Image-operation
names pass through a helper as `%s` data and allow literal percent characters.
Analysis leaves formatted size unbounded. Runtime callback targets remain unverified.
Only the two LOGO MDEC rows with `ascii-bios-printf-tab-format` also accept tabs;
other executable codecs refuse them. Their sync names pass as `%s` data.
See [formats.md](formats.md#executable-ascii) for supported allocations and limits.

For the supported US `GAME.EMI`, extraction writes one `bof3.record-text/v1`
document of equipment and ability names. Use `--entry all` or `--entry 0` and omit
`--class` and `--latch`. These fixed-width names use literal characters without
dialogue commands. See [formats.md](formats.md#record-names).

For supported US `START.EMI` and `STATUS.EMI`, select `--entry 8` for START or
`--entry 0` for STATUS, without class or latch selectors. Extraction writes one
`bof3.inline-text/v19` document with ten counted labels, eight default names,
68 ordinary keyboard cells and four name-entry menu labels. Re-extract older
inline documents. Default `--entry all` selects banks.
See [formats.md](formats.md#default-character-names).

For COMMU02 menu labels, select `--entry 8` in `ETC/COMMU02.EMI` or
`--entry 7` in `ETC/COMMU02B.EMI`, below `BIN`. The inline document contains
fifteen game literal fields, one ASCII separator, eight ASCII formats and 68
keyboard cells. Edit only `text` using each row's codec. Fixed fields use their
full byte capacity; terminated fields reserve one NUL byte. Each
`game-keyboard-character` cell requires one letter, digit, space or one of
`! ? - , .`. Empty cells, control cells and the other keyboard pages are protected.
Single-argument decimal formats retain one signed word and at most eleven output
bytes. The shared `%d` and three-argument `%d%d%d` formats must retain their original
text within two and six encoded bytes, respectively. Fallback formats refuse
percent sequences. Packing preserves all pointer words and unrelated bytes.
Each archive needs its own document and output.
See [inline formats](formats.md#inline-overlay-text) for field limits.

For COMMU00 default names, select `--entry 0` in `ETC/COMMU00.EMI` or
`--entry 14` in `WORLD04/AREA175.EMI` through `AREA185.EMI`, below `BIN`.
The inline document contains sixty five-byte literal fields. Edit only `text`;
retain all allocations and metadata. Packing preserves each record's four
attribute bytes and refuses name growth beyond five encoded bytes. Each archive
requires its own document and output. See [inline formats](formats.md#inline-overlay-text)
for the cross-overlay consumers and remaining runtime limits.

For encounter-template monster names, use `analyze` to find `monster_entries`,
then select the entry explicitly with `extract --entry N`. The output uses
`bof3.monster-text/v1`. Omit class and latch selectors. Names accept up to eight
literal bytes; see [formats.md](formats.md#monster-names). Default `--entry all`
continues to select banks; `extract-corpus` exports both.

For supported embedded overlay text, select the code entry explicitly:

```sh
bin/harness text extract out/extracted/BIN/BATTLE/BATTLE.EMI --entry 15 -o out/battle-categories.json
```

This emits `bof3.inline-text/v19`; omit class and latch selectors. `analyze` lists
the supported entry in each archive. Default `--entry all` selects banks in
archives containing both layouts; `extract-corpus` exports both.
The original BATTLE archive also has supported message records in entry 3.
Use `--entry 3` to extract them. Its appended row 1 permits literals and
`{glyph(0xNN)}` within five encoded bytes. Other command-bearing and untraced
records remain excluded. SISYOU entry 0 supports four master-menu stat labels,
the status labels `Pois` and `Conf`, and three ASCII numeric formats; select it
with `--entry 0`. See [formats.md](formats.md#inline-overlay-text).

## `extract-corpus [--root DIR] -o|--output DIR`

Exports every currently verified dialogue and battle bank beneath `DIR`, preserving
archive-relative paths as `ARCHIVE.EMI/ENTRY.json`. The default source root is
`out/extracted`. The destination must be new, outside the input tree, with an
existing parent. An existing destination is refused. Every document is parsed back
and its unchanged bank must reproduce the original payload before export.

`manifest.json` records each exported source, archive SHA-256 and round-trip result.
Paths retain their actual components, including literal backslashes in Unix
filenames. Input paths must be UTF-8; unrepresentable inputs are refused before
the destination is created.
An archive failure is recorded and makes the command exit 1. Outputs from a failed
run remain for diagnosis; use a new destination for the next run. This operation
also exports equipment and ability names as `GAME.EMI/records.json`. It covers the supported
banks, name fields and supported executable ASCII strings. Executable inputs are
discovered by their PS-X EXE header. Documents use their relative source path
plus `.json`, such as `SLUS_004.22.json` or `LOGO/LOGO.EXE.json`, and round-trip before
publication. Supported inline strings use `ARCHIVE.EMI/ENTRY.json` alongside
bank documents. START's labels and default names use `START.EMI/8.json`, and
STATUS's use `STATUS.EMI/0.json`, at their archive-relative paths. Use `coverage`
to inspect other consumers and gaps.
Encounter-template names use that same path shape, with class `monster_names`
in the manifest. Each whole archive must round-trip before publication.

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
Executable ASCII validation checks its explicit allocations, character set and
capacity. It emits a JSON summary; `--commands` has no extra effect because this
codec has no dialogue commands. `pack` also checks the original source and references.

Equipment and ability name documents use `bof3.record-text/v1`. Validation checks their full
record inventory, literal character encoding and twelve-byte field capacity.
The JSON summary reports `names`; `--commands` adds nothing for this codec.
Monster documents use `bof3.monster-text/v1` and report `names`, with an eight-byte
field capacity. All eight records and their allocation identities are required.
AREA008 entry 13 includes three decimal formats with separate encoded and
formatted output limits. Use `analyze` to inspect `max_formatted_size` before
editing. The deferred masked-byte format has an unverified final renderer and
retains `renderers: []`. See [inline formats](formats.md#inline-overlay-text)
for its supported arguments and limits.

AREA030, AREA089 and AREA129 entry 4 each include two numeric formats and two
twelve-byte fixed labels. The formats accept one unsigned sixteen-bit decimal
argument, at most three encoded ASCII bytes and five formatted bytes. Their
draw limits remain three and five characters respectively. Fixed labels use
all twelve bytes with literal game encoding. See [inline formats](formats.md#inline-overlay-text)
for source addresses and the limits of the consumer evidence.

BATE and SHISU entry 0 each include `%3d/%3d` with two unsigned byte arguments.
The `ascii-u8-decimal-pair-format` codec preserves their order, at most seven
encoded bytes, and at most seven formatted bytes. Widths use 1..4 with optional
zero padding; output growth and other conversions are refused. Each archive
also exposes `%2d` with one unsigned byte argument and `%02d %02d` with two.
Their encoded limits are three and nine bytes; formatted limits are three and
seven bytes. An adjacent `ascii-literal` label accepts three printable ASCII
bytes, using `0x20` for spaces. It does not interpret format conversions or command
markup. Each allocation reserves a NUL byte. Runtime rendering remains unverified.

BATE and SHISU also expose `%02d` with one unsigned byte argument, at most four
encoded bytes and three formatted bytes. Their `%7d` rows accept one signed
32-bit argument, at most three encoded bytes and eleven formatted bytes. Only
this signed codec permits decimal widths through 7. Its `renderers` array retains
both proven draw functions. The adjacent `-------` fallback accepts one to seven
printable ASCII bytes and refuses every percent sequence because its formatter
receives no arguments. All these allocations reserve NUL.

SHOP entry 0 includes an ASCII clock format with three unsigned byte arguments.
Preserve their order, at most nineteen encoded bytes, and at most sixteen
formatted bytes. Decimal widths use 1..4 with optional zero padding. Its traced
consumer copies the result to a stack buffer; retain `renderers: []` because
rendering is unverified. The filename and Shift-JIS header formats remain unsupported.
The `%d` at `0x801D0C1C` retains exactly that format within two encoded bytes.
Its unsigned argument is at most 2550; the codec allows five formatted bytes.
Preserve `renderers: []` because only formatting and the eight-byte text-slot
copy are qualified. Empty `ascii-format-literal` replacements are refused to
prevent stale scratch output.

Inline documents use `bof3.inline-text/v19`; validation checks all allocations,
per-string renderers and codecs, references and capacity. Terminated or counted
rows reserve one NUL or length byte; fixed literal rows use their full capacity.
It reports `strings`. Re-extract older inline or character-only documents.
`--commands` counts glyph commands; literal-only layouts retain an empty list.
The `ascii-decimal-format` codec retains exactly one `%3d`; only its surrounding
printable ASCII may change within capacity. Bounded decimal codecs preserve
their argument count and enforce both encoded and formatted limits.

## `query SOURCE [-e|--entry N] [--grep TEXT] [-i|--ignore-case] [--commands] [--class ...] [--windows PATH] [--no-windows]`

Without `--class`, queries include verified inline allocations, equipment and
ability names, default character names and encounter-template names alongside
banks. `--entry` accepts a supported entry, a comma-separated list or `all`.
For either supported US executable, omit entry and class selectors to query its
ASCII allocations. For example:

```sh
bin/harness text query out/extracted/BIN/ETC/SISYOU.EMI --entry 0 --grep Pwr --json
bin/harness text query out/extracted/SLUS_004.22 --grep "Exception" --json
```

Literal results use `bof3.literal-query/v1`, retain the source schema and identity,
and report allocation metadata in `matches`. Literal-only rows use `reading: literal`.
The battle append row uses `reading: command-aware`; glyph commands and their
operands are transparent to matching.
The default string anchor covers the allocation, including padding and any NUL;
the match anchor covers only matched source bytes. `--commands` counts glyph
commands in the append row and reports an empty usage list for literal-only fields.
Class selectors retain their bank or heuristic scope.
Corpus `search` also includes these verified literal allocations. Use the complete
extraction root to include executable text.

Lists rows, optionally filtered by a **command-transparent** text match, showing
`bank.row`, the bank-relative byte offset, span length and decoded text. When a match is
found the row also reports the match's own byte offset and length and which reading
produced it. `--commands` prints a command-usage histogram instead. Both modes support
`--json`.

Control commands and their operands are transparent to `--grep`, so a phrase the game
breaks across a command is found (`--grep "it'll be a good"` matches
`it'll{nl}be a good crop`) and **raw command bytes are never matchable**: `--grep end`
does not match a `{end}` terminator, and `--grep 0x81` does not match an operand. Token
syntax is therefore not searchable. Use `--commands` for command usage. `-i`
folds ASCII case.

**Bank and heuristic `query` and `search` use one matcher.** Both route through `search::find_in_instance`, so the
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

```sh
bin/harness text verify --root out/extracted
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

```sh
bin/harness text map --root out/extracted
```

A range is `text` only when it parses as a bank section, reproduces byte-identically
and belongs to a supported bank class. Everything else has role `data`, including
other text layouts; this role does not prove the absence of text. Text-like runs
inside data remain candidates. The report does not encode the EU knowledgebase's
ctype taxonomy. Use `coverage --strict` for supported literal layouts, original
disc membership and unresolved source ranges.

## `probe ARCHIVE.EMI [--round-trip]`

Probes **every** subfile of an archive for text-bank structure — a pointer table whose rows tile
the string region — **without consulting class membership**, so an unknown load argument is judged
on the same terms as `dialogue`. `--round-trip` also rebuilds each parsed section and reports
whether it reproduces the payload byte-for-byte.

```sh
bin/harness text probe out/extracted/BIN/WORLD00/AREA000.EMI --round-trip
```

The report includes the entry, load argument, bank and row counts, and reproduction
result. A successful probe supplies structural evidence for further investigation;
promotion also requires original consumer evidence. A failed probe does not rule
out text stored in another layout.

## `search [--index PATH] [--root DIR] [--grep TEXT] [-i|--ignore-case] [--class ...] [--script S] [--language L] [-e|--entry N] [--archive TEXT] [--limit N] [--match-offset] [--windows PATH] [--no-windows] [--verify-entries]`

Searches the **retained corpus index** (`build-index`) instead of one archive at a time,
using the same command-transparent matcher as `query`.

```sh
bin/harness text build-index --root out/extracted
bin/harness text search --root out/extracted --grep EXCEPTION
```

Filters: `--class`, `--script` (e.g. `Latin`), `--language` (e.g. `eng`; only guesses the
detector called reliable are stored), `-e/--entry`, `--archive` (path substring), and
`--limit` (0 reports every hit). Results are ordered by relative source path,
optional EMI entry and file offset before the display limit is applied. Literal
rows remain independent. `--class` selects bank or heuristic rows and excludes
verified literals. Executable results have no EMI entry. Use `--root out/extracted`
for a complete extraction tree; the default root is `out/extracted/BIN`.

Freshness is decided by the **content digest**, never by the stat manifest: a manifest entry
keeps a size and a whole-second mtime that can both survive a content change, and a search whose
hits were all served from the index would otherwise answer from stale data without noticing.
A stale or missing index is **regenerated and then used**, and the run reports that (the `--json`
output carries `"regenerated": true`). `build-index --check` is the refusing mode for a caller
that wants to detect staleness without writing.

Freshness also binds the loaded ownership registry, payload inventory, vocabulary,
readability settings and verified literal codec schemas through `inputs.filters_hash`.
Changing their contents
or using `--no-windows`, `--no-payloads`, `--no-readable` or `--no-vocabulary`
invalidates the index. Older index schemas are rebuilt. Use a
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
instance against its own source and proves its readings or literal document reproduce,
reporting `verified N of M` and failing closed on any mismatch. `--limit` is ignored
in this mode, so the reported count covers every selected row.

Bank and heuristic hits reproduce their readings from the owning archive before
reporting offsets. Literal hits re-extract the complete source-bound document and
compare its identities, codecs and allocations with the index. Literal results
retain `source_schema`, original source hash, consumer and allocation metadata,
and use `class: null`. Glyph commands and their operands remain unsearchable.

## `build-index [--root DIR] [--out PATH] [--check] [--force] [--windows PATH] [--no-windows]`

Builds the retained **corpus text index**: one artifact recording where text lives across
many archives, so a corpus query is a scan of that file rather than of the disc.

```sh
bin/harness text build-index --root out/extracted
bin/harness text build-index --root out/extracted --check
```

Each instance records its class, source (`archive`, `entry`, `address`), byte extent
(`offset`, `length`) and the **command-transparent reading** — the same field `--grep`
matches. `bank`/`number` identify a banked row; a non-banked subfile contributes the
scanner's latches instead. The header carries digests over archive paths, sizes and
contents and the index-building filter policy. The `bof3.text-index/v2` artifact
also retains verified inline, record-name, monster-name and executable documents
in `literals`. Counts include each source schema separately. Executable inputs
are recognized by their PS-X EXE header, and their bytes participate in freshness.
Paths must be UTF-8; symlinks and special files are refused.

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
2. The required `source_sha256` matches the entire original archive, the source
   path resolves to that archive, and entry, load address and class match. The
   required `fingerprint` also matches the original row extents.
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

For either supported US executable, pass that executable as `--original` and its
ASCII document as `--text`. The tool checks the pinned layout, complete source
SHA-256, canonical source path, complete allocation list and unchanged pointer
references. Text must fit the original ASCII allocation with a final NUL.
Executable input uses `--entry all` and refuses class or latch selectors.

For the supported `GAME.EMI`, packing accepts the name document from extraction.
It checks complete source identity and every fixed-width field, writes at most
twelve encoded literal bytes per name and NUL-pads shorter names. Other record
bytes remain unchanged. Use `--entry all` or `--entry 0` without class or latch
selectors. This layout does not support relocation or dialogue commands in names.

For inline text, pack its document against the same original archive, with
`--entry all` or the recorded entry and no class or latch selector. Full archive
identity, pinned payload layout, entry offset and all pointer references must
match. Terminated rows reserve one byte for NUL; counted rows reserve one byte
for their trailing length. START and STATUS default names accept all five bytes
of their fixed fields. Each archive copy needs its own document and output.

For monster names, pack the extracted document against the same original
archive, using `--entry all` or its recorded entry without class or latch
selectors. Each name may use eight encoded literal bytes. Shorter names are
NUL-padded. Packing preserves the remaining encounter and stat fields.

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
