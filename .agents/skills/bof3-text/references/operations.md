# Text operations

Use `bin/harness text --help` and the selected mode's `--help` for current flags.
The harness refreshes its Rust binary when sources change. `PSX_BOF3_TEXT` selects
an alternate binary; record an override when it affects verification.

## Establish coverage

Run `bin/harness text prepare` after disc extraction to prepare search artifacts.
Then run `bin/harness text coverage --root out/extracted --strict`. Keep its JSON
and exit status together. The report accounts for every file and EMI payload,
pins content hashes, distinguishes verified derivatives, and retains unresolved
ranges. Strict failure is expected while full coverage remains open. The source
root includes the executable and non-EMI files, not only `BIN`.

US bank support does not establish EU or JP encoding. An unrecognized build or
unknown consumer requires investigation before semantic claims or supported writes.
A missing heuristic hit does not prove that a range contains no text.

## Extract and analyze

`bin/harness text extract-corpus --root out/extracted -o out/text-export` exports
verified banks, inline strings, supported record names and US executable ASCII into a new
tree with `manifest.json`. Use an existing parent and a new destination outside
the source tree. Each exported document must parse back and
reproduce its original bytes. Any export failure makes the operation fail; retain
the manifest and inspect failures before claiming coverage.

For a single archive, use `index`, `extract`, `analyze` and `query`. For example:

```sh
bin/harness text analyze out/extracted/BIN/WORLD00/AREA000.EMI
bin/harness text extract out/extracted/BIN/WORLD00/AREA000.EMI -o out/area000.json
```

`analyze` emits diagnostic `bof3.text-analysis/v1` reports. Literal allocations
include encoded size, spare capacity, required terminator bytes and the original
allocation bytes. Use `extract` for documents accepted by `pack`.

For the US executable, use `extract out/extracted/SLUS_004.22 -o out/exe-text.json`.
Its `bof3.executable-text/v3` document uses ASCII. Re-extract older documents.
Edit only each string's `text`; retain its consumer, codec, allocation and typed
reference sites. The exception format permits one `%X` or `%x` conversion with
optional width 1..8 and zero padding, within four bytes. The three-ID format
requires three ordered hex conversions with optional width 1..2 and zero padding;
literal ASCII is allowed within 17 encoded bytes and 11 formatted bytes.
Other percent sequences and output expansion are refused. Pack to a new executable:

```sh
bin/harness text pack --original out/extracted/SLUS_004.22 \
  --text out/exe-text.json -o out/edited-exe.bin
```

Omit EMI selectors. Support covers the source-traced exception screen and
`FILE NOT FOUND` diagnostic. Other executable ranges
and runtime acceptance remain unresolved. See the tool's
[format contract](../../../../tools/rust/bof3-text/docs/formats.md#executable-ascii).

`extract out/extracted/BIN/ETC/GAME.EMI -o out/equipment-names.json` exports
supported equipment and ability name records through `bof3.record-text/v1`. Edit only each
record's `text`, using literal characters and at most twelve encoded bytes.
Retain every record and its identity. Pack against that same `GAME.EMI` into a
new output, with `--entry all` or `--entry 0` and no class or latch selector.
The tool preserves stats and other record bytes. These fields do not use
dialogue command markup. Other name families require separate evidence.

US START entry 8 and STATUS entry 0 each expose ten counted labels and eight
default names through `bof3.inline-text/v8`. Select the entry explicitly and omit
class and latch selectors. Re-extract former character-only documents. Each
archive needs its own document and output; editing one does not update the other
or a save.

For embedded overlay text, use `analyze` to find the supported `inline_entries`
and select that code entry explicitly with `extract --entry N`. The document
uses `bof3.inline-text/v8`. Re-extract older inline documents. Edit only `text`,
retain all strings, per-string renderers, codecs and references. Reserve one byte
for the NUL in terminated rows or the trailing count in `game-counted-literal`
rows. Counted rows accept seven literal bytes. Default names use
`game-fixed-literal` and accept all five bytes without reserving NUL. Packing
zero-fills unused field bytes and updates counted rows' lengths. Pack against the
same archive with its recorded entry or `all`, without class or latch selectors.
Each matching archive payload needs its own source-bound document and output.
`ascii-decimal-format` rows retain exactly one `%3d` and allow only surrounding
printable ASCII within capacity. Other percent sequences are refused.
AREA008 entry 13 also exposes three bounded decimal formats. Preserve their
ordered arguments and both encoded and formatted limits. The signed pair permits
at most nine output bytes; the unsigned byte and masked seven-bit rows permit
three. Optional widths use 1..4 with optional zero padding; other conversions are
refused. The masked row's final renderer is unverified and records `null`.
Retain that value. See the [inline format contract](../../../../tools/rust/bof3-text/docs/formats.md)
for argument bounds and allocation sizes.
AREA030, AREA089 and AREA129 entry 4 each expose two unsigned sixteen-bit decimal
formats and two fixed twelve-byte labels. Numeric output is bounded by five
bytes, but the first format's draw limit remains three characters. Preserve
the formats' arguments and both size bounds; fixed labels accept literal game
characters in all twelve bytes. Runtime rendering remains unverified.
BATE and SHISU entry 0 each include an ASCII format with two ordered unsigned
byte arguments. Preserve both arguments, at most seven encoded bytes, and at
most seven formatted bytes. The original format is `%3d/%3d`; runtime rendering
remains unverified.
SHOP entry 0 also includes an ASCII clock format with three ordered unsigned
byte arguments, at most nineteen encoded bytes and sixteen formatted bytes.
Preserve its `renderer: null`; only formatting and a stack-buffer copy are traced.
The filename and Shift-JIS header formats remain unsupported.
Corpus export includes these documents alongside bank documents.
Battle append row 1 also accepts `{glyph(0xNN)}`, with a nonzero operand and at most
five encoded bytes. Other command-bearing and untraced table rows are refused.
An empty pointer-reference list means indexed or code addressing; retain it and
the consumer metadata. The pinned payload protects those instructions.

For encounter-template monster names, find `monster_entries` with `analyze`
and extract the selected entry explicitly. The `bof3.monster-text/v1` document
must retain all eight records, including empty names. Edit only `text`, using
at most eight literal bytes per name. Pack against the same source archive
with its recorded entry or `all`, without class or latch selectors. Shorter
names are NUL-padded; encounter data and stats remain unchanged. Corpus export
includes each matching entry. Runtime rendering and other regions remain open.

Analysis reports archive offsets, encoded size, original capacity, command operands
and shared rows. Terminators inside operands are not string boundaries. Trailing
string segments are structural observations; trace the consumer before naming
option semantics. `query` and corpus `search` match displayed literals across
commands. Their default offset is the containing string; `--match-offset` selects
the actual match.

Without a class selector, `query` also searches the supported literal layouts,
including executable ASCII. For EMI sources, select one entry, a comma-separated
list or `all`. Omit entry selectors for the executable. Literal JSON results retain
source identities and allocation metadata; their default anchor includes the full
allocation. `--match-offset` selects matched bytes. Corpus `search` also includes
verified literal fields. Use `--root out/extracted` with `build-index` and `search`
to include the executable; the default `BIN` root includes only files below `BIN`.
Literal results retain `source_schema`, codec, allocation and consumer metadata;
`class` is null. A class selector restricts search to banks or heuristic instances.
`--verify-entries` reproduces every selected row, regardless of `--limit`.

## Modify safely

For bank edits, use a freshly extracted version 3 JSON document. Preserve `source`, `fingerprint`
and `source_sha256`. The required SHA-256 binds the entire original archive;
packing also checks its path, entry, load address and class. Re-extract old or
stale documents instead of synthesizing provenance.

Edit row text with the supported command grammar. Keep shared rows identical and
fit the entire encoded row, including its trailing strings, into its original
capacity. Empty slots have no allocation. Unsupported bytes remain `{byte(0xNN)}`;
unknown characters must not be approximated.

```sh
bin/harness text validate out/area000.json
bin/harness text pack --original out/extracted/BIN/WORLD00/AREA000.EMI \
  --text out/area000.json -o out/area000-edited.EMI
```

Outputs are create-new. A raw `--latch ENTRY:OFFSET+LENGTH` document is heuristic
and supports only an exact-length patch. Bank edits retain the original pointer
tables and allocations. Relocation requires a separately evidenced layout and
complete reference ownership.

Re-extract the edited output and confirm intended text. Compare changed byte
ranges against the original allocations and hash the original again. Keep byte
results separate from runtime results. For game-visible acceptance, use disposable
images and bounded emulator missions to observe the edited consumer, including
choices and substitutions when affected. A launch or successful pack is not a
rendering result.
