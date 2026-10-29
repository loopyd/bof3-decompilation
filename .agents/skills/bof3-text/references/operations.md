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
The original US track-2 PCM WAV export has a pinned whole-file nontext identity;
changed or arbitrary WAV files stay unresolved.

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

For a single source, use `index`, `extract`, `analyze` and `query`. `index` lists
supported banks and literal layouts, including executable ASCII. Its `rows` count
covers strings or names for literal layouts; bank-only counts are null. Use
`--entry` for an EMI entry and `--class` for a bank class. Executables refuse EMI
selectors. For example:

```sh
bin/harness text analyze out/extracted/BIN/WORLD00/AREA000.EMI
bin/harness text extract out/extracted/BIN/WORLD00/AREA000.EMI -o out/area000.json
```

`analyze` emits diagnostic `bof3.text-analysis/v1` reports. Literal allocations
include encoded size, spare capacity, required terminator bytes and the original
allocation bytes. Use `extract` for documents accepted by `pack`.

For the main US executable, use `extract out/extracted/SLUS_004.22 -o out/exe-text.json`.
Its `bof3.executable-text/v17` document uses ASCII. Re-extract older documents.
Edit only each string's `text`. Retain its consumers, output, codec,
`format_directives`, allocation and typed reference sites. The exception format permits one `%X` or `%x` conversion with
optional width 1..8 and zero padding, within four bytes. The three-ID format
requires three ordered hex conversions with optional width 1..2 and zero padding;
literal ASCII is allowed within 17 encoded bytes and 11 formatted bytes.
Other percent sequences and output expansion are refused. Pack to a new executable:

```sh
bin/harness text pack --original out/extracted/SLUS_004.22 \
  --text out/exe-text.json -o out/edited-exe.bin
```

Omit EMI selectors. Support covers the exception screen, `FILE NOT FOUND`, CD
interrupt and command names, fixed BIOS diagnostics and BIOS formats. `analyze`
reports their capacities, including NUL, and output paths. The shared CD `?` holds
one ASCII character plus NUL and retains twelve pointer references. Literal labels
allow percent bytes passed through `%s` or BIOS puts. Direct printf literals
refuse every percent character. The `DiskError: ` prefix holds at most eleven bytes plus NUL; its
output requires CD debugging, which starts disabled. Shared-word label references
have empty `calls` arrays where intervening calls leave pointer use unproven.
`LOGO/LOGO.EXE` uses the same commands and schema with its own pinned layout.
It supports kernel, CD, GPU and MDEC diagnostics. Discover allocations with `analyze`;
retain exact directives and refuse percent characters in direct printf literals.
CD labels and image-operation names passed as `%s` data allow literal percent
bytes. Other LOGO text and runtime console execution remain unverified. Corpus export discovers
executables by header and keeps their relative paths, adding `.json`.
BIOS formats permit literal edits within capacity but require every original
`format_directives` token unchanged and in order. Other rows retain that array
empty. Analysis reports no formatted size bound for BIOS formats; `%02x` is a
minimum width. GPU callback formats use `bios-printf-callback`, recording the callback
word and its initial BIOS stub. Retain their indirect reference calls and exact
directives, including `%3d`; runtime callback targets remain unverified. The
ResetGraph address and GPU timeout diagnostics record direct BIOS printf calls.
Image-operation names use `ascii-literal` as `%s` data through a helper;
percent characters remain literal. Preserve their helper reference calls.
Matrix-stack guard messages use `ascii-printf-literal` and refuse percent characters.
SPU wait labels are `%s` data; the VSync timeout message uses BIOS puts.
Interrupt formats retain exact `%04x` conversions.
Two LOGO MDEC formats use `ascii-bios-printf-tab-format` and allow ASCII tabs;
other executable codecs refuse them. MDEC sync names pass as `%s` data through
helper `0x801D7414`. Other executable ranges remain unresolved. Each output path requires separate
runtime evidence.
See the tool's [format contract](../../../../tools/rust/bof3-text/docs/formats.md#executable-ascii).

`extract out/extracted/BIN/ETC/GAME.EMI -o out/equipment-names.json` exports
supported equipment and ability name records through `bof3.record-text/v1`. Edit only each
record's `text`, using literal characters and at most twelve encoded bytes.
Retain every record and its identity. Pack against that same `GAME.EMI` into a
new output, with `--entry all` or `--entry 0` and no class or latch selector.
The tool preserves stats and other record bytes. These fields do not use
dialogue command markup. Other name families require separate evidence.

US START entry 8 and STATUS entry 0 each expose ten counted labels, eight default
names, 68 ordinary keyboard cells, four name-entry menu labels, fifteen stat
labels and one numeric format through `bof3.inline-text/v24`. Select that entry
without class or latch selectors. Re-extract older inline documents. Use a
separate document and output for each archive. Editing one does not update the
other or a save.

COMMU02 entry 8 and COMMU02B entry 7 each expose fifteen menu labels, one ASCII
separator, eight ASCII formats and 68 keyboard cells through the inline schema.
Five labels have fixed byte limits; the other labels, separator and formats reserve
one NUL byte. Edit only `text` using each row's codec. Single-argument decimal formats
retain one signed word and an eleven-byte
output bound. The shared `%d` and three-argument `%d%d%d` formats must retain their
original text within two and six encoded bytes, respectively. Fallback formats
refuse percent sequences. Pointer words, padding, adjacent glyphs and unrelated
bytes stay unchanged. Each `game-keyboard-character` cell requires exactly one
letter, digit, space or one of `! ? - , .` from the active US keyboard alphabet.
Empty cells, control cells and the other pages stay protected. Use a separate
document and output for each archive. Controlled runtime checks cover the English
keyboard's control labels and one edited cell. Other COMMU02 text, formats and
pages remain unverified.

COMMU00 entry 0 and AREA175 through AREA185 entry 14 each expose sixty
five-byte default names through the same inline schema. Edit only `text` with
literal game characters. Preserve the four attribute bytes after each field.
Each archive needs its own source-bound document and output. The proven copy
and draw calls belong to the companion COMMU02 overlay. Controlled availability
verifies one edited name through the empty-name fallback. Natural availability,
other names and save propagation remain unverified.

For embedded overlay text, use `analyze` to find the supported `inline_entries`
and select that code entry explicitly with `extract --entry N`. The document
uses `bof3.inline-text/v24`. Re-extract older inline documents. Edit only `text`,
retain all strings, per-string renderers, codecs and references. Reserve one byte
for the NUL in terminated rows or the trailing count in `game-counted-literal`
rows. Counted rows accept seven literal bytes. Default names use
`game-fixed-literal` and accept all five bytes without reserving NUL. Packing
zero-fills unused field bytes and updates counted rows' lengths. Pack against the
same archive with its recorded entry or `all`, without class or latch selectors.
Each matching archive payload needs its own source-bound document and output.
START entry 8 and STATUS entry 0 each expose fifteen stat labels in addition to
the counted labels, default names and keyboard fields. Each `game-literal` stat
allocation permits three encoded bytes plus NUL. Preserve the two trailing bytes
in six-byte records. Each entry also exposes `%3d` through
`ascii-u16-decimal-format`. Retain one decimal argument, at most three encoded
bytes plus NUL and at most five formatted characters. Its eighteen formatter
sites draw through `0x80150098`. Controlled Status and Equipment member
selection verify edited labels in the groups at `0x801EC940` and `0x801ECB0C`,
respectively. Identical START and STATUS payloads leave the active supplier
indistinguishable. Equipment member selection also verifies decimal width 2 in
its stat panel. Other groups, numeric paths and values, and natural menu
availability remain unverified.
COMMU03 entry 6 exposes three audio menu labels through `ascii-literal`, with
capacities of fifteen, five and seven bytes including NUL. Use printable ASCII,
including `0x20` spaces. Retain the overlay-local renderer `0x801D1714` and
pointer references. Controlled runtime checks verify the three edited labels.
Natural menu and configuration producers, other labels, arrow glyphs and audio
output remain unverified.
`ascii-decimal-format` rows retain exactly one `%3d` and allow only surrounding
printable ASCII within capacity. Other percent sequences are refused.
AREA008 entry 13 also exposes three bounded decimal formats. Preserve their
ordered arguments and both encoded and formatted limits. The signed pair permits
at most nine output bytes; the unsigned byte and masked seven-bit rows permit
three. Optional widths use 1..4 with optional zero padding; other conversions are
refused. The masked row's final renderer is unverified and records `renderers: []`.
Retain that empty array. See the [inline format contract](../../../../tools/rust/bof3-text/docs/formats.md)
for argument bounds and allocation sizes.
AREA030, AREA089 and AREA129 entry 4 each expose five numeric formats, one
nonempty digit literal and two fixed twelve-byte labels. Arguments include signed
and unsigned halfwords plus one; `%d/6` can receive 65536. Use `analyze` for each
encoded and formatted bound. The formats at `0x801D0C38` and `0x801D0C8C` retain
three- and five-character draw limits. Fixed labels use literal game characters
in all twelve bytes. Runtime rendering remains unverified.

BATE and SHISU entry 0 each expose `%3d` through `ascii-u16-decimal-format`.
Retain one decimal argument, at most three encoded bytes plus NUL, and at most
five formatted characters. Its eighteen formatter sites per overlay use unsigned
byte or halfword arguments, then draw through `0x80150098`. Controlled BATE checks
verify width-two decimal rendering on one stat panel. Other paths and SHISU
rendering remain unverified.

BATE and SHISU entry 0 each expose fifteen stat labels through `game-literal`.
Each four-byte allocation permits three encoded bytes and NUL; the two trailing
bytes of six-byte records remain protected. Their fixed-address consumers use
`0x8014F800`. Edit each archive separately and retain every allocation and its
empty pointer-reference list. Controlled BATE checks verify the four-label group
at `0x801D8CF4`. Other groups, natural request production and SHISU rendering
remain unverified.

BATE and SHISU entry 0 each include an ASCII format with two ordered unsigned
byte arguments. Preserve both arguments, at most seven encoded bytes, and at
most seven formatted bytes. The original format is `%3d/%3d`. Each archive also exposes `%2d` with one
unsigned byte argument and `%02d %02d` with two. These accept at most three and
nine encoded bytes, respectively, with formatted bounds of three and seven.
The adjacent `ascii-literal` label accepts three printable ASCII bytes, including
spaces as `0x20`, without command or format interpretation. All these fields
reserve one NUL byte. Controlled BATE equipment-screen checks cover the `WEAPON`
and `ARMOR` category labels and the `%3d/%3d` format. Other fields and SHISU
rendering remain unverified.
BATE and SHISU also expose `%02d` with one unsigned byte argument and bounds of
four encoded bytes and three formatted bytes. Their `%7d` rows retain one signed
32-bit argument, three encoded bytes and eleven formatted bytes. Only this codec
permits widths through 7. Preserve both proven addresses in `renderers`. The
adjacent `-------` fallback accepts one to seven printable ASCII bytes and refuses all
percent sequences because its formatter receives no arguments. Reserve NUL.
For SISYOU, keep `%3d` and edit only the surrounding printable ASCII. Its split
fields draw at the same coordinates; keep both separators in the same formatted
character column. Controlled checks cover the `Pwr`, `Def`, `Int` and `Agl`
labels and the split HP/AP fields. Separate character-status fixtures cover
`Pois` and `Conf`; natural illness acquisition remains unverified.
SHOP entry 0 also includes an ASCII clock format with three ordered unsigned
byte arguments, at most nineteen encoded bytes and sixteen formatted bytes.
Preserve its `renderers: []`; only formatting and a stack-buffer copy are traced.
The filename and Shift-JIS header formats remain unsupported.
SHOP's `%d` at `0x801D0C1C` retains exactly `%d` within two encoded bytes and
uses the five-byte unsigned 16-bit output bound. Preserve `renderers: []`;
formatting and the eight-byte text-slot copy are qualified, while the active
message-bank binding is unverified. Refuse empty `ascii-format-literal`
replacements, which can expose stale scratch bytes after the initial NUL.
Controlled SHOP party-change checks cover the split HP/AP fields. Edit both
halves together and keep their separators in the same formatted character
column, since both draw at the same coordinates. Identical format-pair
allocations remain indistinguishable; other SHOP rendering remains unverified.
Corpus export includes these documents alongside bank documents.
Battle append row 1 also accepts `{glyph(0xNN)}`, with a nonzero operand and at most
five encoded bytes. Other command-bearing and untraced table rows are refused.
BATTLE entry 3 also exposes two digit-sprite decimal formats and their fallback.
Decimal rows retain one unsigned sixteen-bit argument, at most three encoded
bytes and five formatted bytes. The one-byte `ascii-digit-glyph-literal` fallback
accepts a space, digit or the original `:` glyph index. Other ASCII and percent
sequences are refused. Runtime observations cover fallback digit `9` beneath
enemy names and width-one decimal rendering at HP/AP value 15 and queued
animation digit 2. Other values, widths and battle contexts remain unverified.
Runtime media must include the copy the game loads; identical BATTLE payloads
occur in multiple US archives with different entry indices.
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
