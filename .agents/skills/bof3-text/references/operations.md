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

For the US executable, use `extract out/extracted/SLUS_004.22 -o out/exe-text.json`
or `analyze out/extracted/SLUS_004.22`. Its `bof3.executable-text/v1` document
uses ASCII, not dialogue tokens. Edit only each string's `text`, retain all
allocations and references, then pack to a new executable:

```sh
bin/harness text pack --original out/extracted/SLUS_004.22 \
  --text out/exe-text.json -o out/edited-exe.bin
```

Omit EMI selectors. Support is limited to the source-traced exception-screen
strings. Other executable ranges
and runtime acceptance remain unresolved. See the tool's
[format contract](../../../../tools/rust/bof3-text/docs/formats.md#executable-ascii).

`extract out/extracted/BIN/ETC/GAME.EMI -o out/equipment-names.json` exports
supported equipment and ability name records through `bof3.record-text/v1`. Edit only each
record's `text`, using literal characters and at most twelve encoded bytes.
Retain every record and its identity. Pack against that same `GAME.EMI` into a
new output, with `--entry all` or `--entry 0` and no class or latch selector.
The tool preserves stats and other record bytes. These fields do not use
dialogue command markup. Other name families require separate evidence.

`extract` also supports eight default character names in US START and STATUS,
using `bof3.character-text/v1`. Edit only `text`, with at most five literal bytes.
Use `--entry all` or the source's record entry: 8 for START and 0 for STATUS.
Omit class and latch selectors. Each copy needs its own freshly extracted
document and separate output; editing one does not update the other or a save.

For embedded battle categories, use `analyze` to find the supported `inline_entries`
and select that code entry explicitly with `extract --entry N`. The document
uses `bof3.inline-text/v1`. Edit only `text`, retain all strings and references,
and reserve one encoded byte for NUL within each allocation. Pack against the
same archive with its recorded entry or `all`, without class or latch selectors.
Each matching archive payload needs its own source-bound document and output.
Corpus export includes these documents alongside bank documents.

Analysis reports archive offsets, encoded size, original capacity, command operands
and shared rows. Terminators inside operands are not string boundaries. Trailing
string segments are structural observations; trace the consumer before naming
option semantics. `query` and corpus `search` match displayed literals across
commands. Their default offset is the containing string; `--match-offset` selects
the actual match.

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
