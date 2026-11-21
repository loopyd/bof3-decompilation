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
root includes the executable and non-EMI files, not only `BIN`. Completion requires
source, encoding and reference evidence, safe packing and byte preservation.
Rendering observations are optional and do not close unresolved source ranges.
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

For SHOP's card-choice labels, extract AFLDKWA entry 0 and FIRST entry 11 with
`--class battle` into separate version 3 bank documents. In bank 0, row 29 is
`Save`, row 153 is the prompt, and rows 126 through 134 share `Memory card`.
Edit all nine shared rows identically. Pack each document against its own source
archive into a separate output. Editing one alias leaves the other unchanged;
the active supplier remains unresolved. See the
[card-choice allocation contract](../../../../tools/rust/bof3-text/docs/formats.md#card-choice-bank-aliases)
for capacities and consumers.
Controlled checks verify the edited prompt, title and slot labels. The edits
also display through ordinary diary interaction after loading a game-created save.
The original-label route has created and reloaded one save. Selected value-panel
calls, the unique supplier and persistence of the edited labels remain unverified.

For the title rows in AREA175 through AREA185, select entry 11 and use a version 3
bank document. Keep each archive's source identity separate. Follow the
[sound-test title contract](../../../../tools/rust/bof3-text/docs/formats.md#sound-test-title-lookup).
The active bank, selection limit and edited rendering remain unverified.

For the main US executable, use `extract out/extracted/SLUS_004.22 -o out/exe-text.json`.
Its `bof3.executable-text/v18` document uses ASCII. Re-extract older documents.
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
helper `0x801D7414`. Other executable ranges remain unresolved. A rendering claim
requires an observation of that output path.
See the tool's [format contract](../../../../tools/rust/bof3-text/docs/formats.md#executable-ascii).

`extract out/extracted/BIN/ETC/GAME.EMI --layout record-names -o out/equipment-names.json` exports
supported equipment and ability name records through `bof3.record-text/v1`. Edit only each
record's `text`, using literal characters and at most twelve encoded bytes.
Retain every record and its identity. Pack against that same `GAME.EMI` into a
new output, with `--entry all` or `--entry 0` and no class or latch selector.
The tool preserves stats and other record bytes. These fields do not use
dialogue command markup. Other name families require separate evidence.

The same GAME entry exposes five category labels with `--entry 0 --layout inline`.
Retain each `game-literal` allocation, its NUL byte and pointer reference. An
omitted layout selector is ambiguous and fails before writing. Discovery, analysis
and corpus export include both layouts. To edit both, pack one layout and extract
the other document from that output before packing it against the new source.
Edited category rendering and natural selector bounds remain unverified.

US START entry 8 and STATUS entry 0 each expose ten counted labels, eight default
names, 68 ordinary keyboard cells, four name-entry menu labels, fifteen stat
labels, one numeric format, five item-category labels, four skill-category labels,
three equipment-category labels, fifteen fixed-width settings fields, six
configuration captions, nine sort captions, fifteen command captions and five
panel captions through
`bof3.inline-text/v58`.
Select that entry without class or latch selectors.
Settings fields retain their recorded glyph limits, count bytes and positions.
Configuration, sort and command captions use `game-literal`. Reserve one NUL
byte within their exact capacities. Preserve alignment and the following tables.
Sort and command captions also retain pointer references. These captions have
source proof; edited rendering and visual fit remain unverified.
All fifteen edited Config fields render in a controlled fixture and through
ordinary menus after a fresh BIOS boot with STATUS-only edits and an unchanged
game-created save. Observed ordinary reader and draw calls return; individual
call pixels, other draw paths, general bounds and Manual/Auto remain unverified.
Packed START media verifies the four edited menu labels and the first ordinary
keyboard cell edited from `A` to `Z` during new-game name entry. Selection renders
`Z` in the entered name. START confirmation copies the normalized five-byte name
to the main character record at `0x80144968`. Other keyboard cells, pages,
STATUS name-entry contexts and memory-card persistence remain unverified.
Re-extract older inline documents. Use a separate document and output for each
archive. Editing one does not update the other or a save. Counted formation labels
use separate groups for one, two and three party members. Controlled observations
verify all ten counted formation labels and the five
edited Items category labels. WEAPON, ARMOR, OPTION and VITAL use separate
controlled records with categories 1 through 4. A preinstruction capture binds
ITEM at category 0. Controlled Ability checks verify first-glyph edits in HEAL,
ASSIST, ATTACK and SKILL, with preinstruction pointers bound for HEAL and SKILL.
Controlled Equipment checks verify first-glyph edits in WEAPON, ARMOR and OPTION
at selectors 0, 1 and 4.

Ordinary Items, Ability and Equipment navigation after loading a game-created
save displays those original category labels. Ordinary Equipment uses selectors
0, 1 and 5. Each selected category call reaches `0x8014F800` with the expected
return address and text pointer at `SP + 0x10`, verifying its JAL and delay store.
Row reads and completed rendering for those individual invocations, general
selector bounds and the unique START or STATUS supplier remain unverified.
See the [record groups](../../../../tools/rust/bof3-text/docs/formats.md#default-character-names)
before choosing a duplicate label.

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

Each matching COMMU00 payload also exposes the shared `%d` field through
`ascii-i32-decimal-format`. Its three-byte allocation permits only the original
format plus NUL. The eleven-byte formatted bound covers five full-word arguments
as well as two unsigned bytes. Retain renderer `0x8014F800`. With the original
AREA175 through AREA185 dialogue bank resident, selectors `0x55` and `0x56`
expand its formatted slots through the shared dialogue draw loop. Active-bank
publication and observed numeric output remain unverified.

For embedded overlay text, use `analyze` to find the supported `inline_entries`
and select that code entry explicitly with `extract --entry N`. The document
uses `bof3.inline-text/v58`. MAGIC003 entry 3 exposes three `uppercase-glyphs`
rows. Write A-Z, `!` and spaces literally; `{byte(0xNN)}` stores one raw glyph
byte. Spaces encode as `0xFF`. End each caption with `{nul}` for the reserved
`0x00`; `{nul}` alone is empty. Preserve the recorded byte and nonblank-glyph
limits, centering and tails.
Use `analyze` for glyph indices and UVs; token names are excluded from text search.
The shared battle layout includes six five-byte fields for Data, Pick, Best, DATA,
BEST and Eqip, plus a four-byte Use field. Keep their row tails intact. Re-extract older
inline documents. Edit only `text`. Retain all strings, per-string renderers,
codecs and references. Reserve one byte
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

The five item-category labels use `game-literal` with capacities of five, seven,
six, seven and six bytes including NUL. Preserve their pointer references and
nine alignment bytes.

COMMU03 entry 6 exposes three ASCII audio-menu labels and three game-glyph fields.
The `ascii-literal` labels allow fifteen, five and seven bytes including NUL,
with printable ASCII and `0x20` spaces. Retain their overlay-local renderer
`0x801D1714` and pointer references. The `game-literal` fields allow three, one
and one encoded glyphs plus NUL, use renderer `0x8014FC90` and retain empty
pointer-reference arrays. Packing preserves the four alignment bytes after the
single-glyph fields. Controlled checks verify the three edited ASCII labels.
Edited game-glyph rendering, natural producers, other labels and audio output
remain unverified.
`ascii-decimal-format` rows retain exactly one `%3d` and allow only surrounding
printable ASCII within capacity. Other percent sequences are refused.
AREA008 entry 13 also exposes three bounded decimal formats. Preserve their
ordered arguments and both encoded and formatted limits. The signed pair permits
at most nine output bytes; the unsigned byte and masked seven-bit rows permit
three. Optional widths use 1..4 with optional zero padding; other conversions are
refused. Retain `0x8014F800` for the masked row's deferred slot renderer.
Active-bank publication and observed rendering remain unverified. See the
[inline format contract](../../../../tools/rust/bof3-text/docs/formats.md)
for argument bounds and allocation sizes.
COMMU01 entry 7 exposes `Culture` and two footer formats. The signed-byte format
permits four formatted bytes before NUL; the unsigned-byte format permits three.
The latter's allocation fits only `%d` plus NUL. Retain each codec and empty
reference list, and inspect encoded and formatted bounds with `analyze`.
Twenty list labels use separate eight-byte `game-fixed-literal` fields. They
reserve no NUL; shorter or empty edits zero-fill the remaining bytes. Preserve
the following kind table. The inline format contract lists capacities and
consumers, including unproven list-count and kind-index producer bounds.
Natural selection, visual fit and edited rendering remain unverified.

SCENA17 entry 0 exposes credits literal runs through `ascii-credits-literal`.
Use only ASCII letters and spaces, retain exact byte widths and pad shorter
wording with spaces. Runs reserve no NUL and cannot contain commands. Preserve
references, codecs and renderer metadata. Command-prefixed parent strings have
protected pointers that do not target their later literal runs. Natural entry,
whole-roll bounds, visual fit and edited rendering remain unverified.

SCENA10 entry 0 exposes `%02d` and `*%2d` with five-byte allocations including
NUL. Preserve their unsigned-word and unsigned-byte field codecs, output bounds
of five and four bytes before NUL, and per-row renderers. The first draw retains
a two-character limit. The adjacent `%s` field and padding remain protected.
Natural selection and edited rendering remain unverified.

SCENA12 entry 0 exposes `%03d` with an unsigned-byte output bound of three.
BOSS025 entry 13 exposes `%2d` with a signed-word bound of eleven and the
`turns left` literal, also present in the identical BOSS027 entry 16 payload.
Each archive needs its own source-bound document. Bounds exclude NUL. The
wrapping subtraction has no proved
0..21 range. Preserve per-field codecs, empty reference lists and all adjacent
bytes. Use the inline format contract for capacities and consumers. Natural
selection and edited rendering remain unverified.

COMMU05 entry 7 exposes four literal `faeries`/`faery` labels and two decimal
formats. The pair codec bounds a signed byte plus one and an unsigned byte to
eight formatted bytes; the single unsigned-byte format permits three. Both
bounds exclude NUL. Retain the original spelling, per-field codec and empty
reference lists. The inline format contract lists capacities, consumers and
protected padding. Natural selection and edited rendering remain unverified.

AREA030, AREA089 and AREA129 entry 4 each expose five numeric formats, one
nonempty digit literal, two fixed twelve-byte labels and seven packed menu labels.
The menu labels are `Gear`, `Data`, `Rule`, `ROD`, `LURE`, `EQUIP` and `GUIDE`.
Use `game-fixed-literal` within each field's recorded capacity, without reserving
NUL. Shorter edits zero-fill the field. Preserve padding and empty reference lists.
Format arguments include signed and unsigned halfwords plus one; `%d/6` can receive 65536. Use `analyze` for each
encoded and formatted bound. The formats at `0x801D0C38` and `0x801D0C8C` retain
three- and five-character draw limits. Fixed labels use literal game characters
in all twelve bytes. Runtime rendering remains unverified.

The same payloads include twelve fishing help-text fields. Their
`game-exact-literal` codec requires the recorded encoded byte length. Pad
shorter wording with game spaces; the reveal loop ignores NUL. Preserve both
references for the shared rod-and-lure field, all thirteen table pointers,
counts and original terminators. `Wind reel...` has no terminator before its
next field. The line-broke field keeps its count of 23 and following NUL while
allowing edits only to its 22 literal bytes. Valid table-row and initialized-work
conditions, addresses and rendering limits are in the same format reference.

SHISU entry 0 exposes fifteen command labels through `game-literal`. SHOP entry 0
exposes twenty-two command labels, four sort captions and three stat captions.
Its `Ink` caption permits three body bytes and NUL in a four-byte field.
Retain NUL and inspect each field's encoded limit with `analyze`. Stat records
permit three body bytes and preserve two trailing padding bytes. Commands require
SHISU group 0..4 or SHOP group 0..8; their callers do not enforce those bounds.
Retain valid task, record and returning-helper conditions. Eight SHISU labels
and SHOP's `Bttn` remain unsupported. Shorter or empty edits zero-fill the field.
Preserve pointers, alignment and selection records. Natural entry, producer
bounds, visual fit and edited rendering remain unverified.

BATE and SHISU entry 0 each expose `%3d` through `ascii-u16-decimal-format`.
Retain one decimal argument, at most three encoded bytes plus NUL, and at most
five formatted characters. Its eighteen formatter sites per overlay use unsigned
byte or halfword arguments, then draw through `0x80150098`. Controlled BATE checks
verify width-two decimal rendering on one stat panel. Other paths and SHISU
rendering remain unverified.

BATE and SHISU also expose `%3d/   ` and `   /%3d` through
`ascii-u16-decimal-field-format`. Keep one unsigned-halfword decimal argument,
seven encoded bytes plus NUL, and at most nine formatted characters per field.
Keep separator positions consistent across the separately colored pair. BATE's
`%7d` and `%4d`, and both overlays' `%8d`, retain one signed 32-bit argument,
three encoded bytes plus NUL and at most eleven formatted characters. Their
eight-dash fallbacks accept one to eight ASCII spaces or dashes plus NUL;
empty text and percent sequences are refused. Preserve earlier indices,
alignment, code and tables. Edited rendering is unverified.

Their two-row `1` and `2` labels use `ascii-digit-glyph-literal`: one digit,
ASCII space or `:` plus NUL within two bytes. Empty text is refused; following
bytes remain protected. Edited display is unverified.

BATE, SHISU and SHOP entry 0, START entry 8 and STATUS entry 0 also expose
`Pois`, `Conf`, `EQUIP`, `EXP` and `NEXT` through `game-literal`. Their capacities
including NUL are 5, 5, 6, 4 and 5 bytes. Shorter or empty edits zero-fill the
field; spaces encode as `0xFF` and commands are refused. Retain empty pointer
references and preserve all alignment and the EXP/NEXT boundary. Valid records,
returning helpers and renderer resources remain required. Natural activation
and edited display remain unverified.

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
reserve one NUL byte. Controlled BATE equipment-screen checks cover `WEAPON`,
`ARMOR`, `OPTION` and the `%3d/%3d` format. The `OPTION` observation uses an
upstream cursor fixture; its natural availability remains unverified. Observed
calls bind `WEAPON`, `ARMOR` and `OPTION` to table `0x801D8EB8`. Other fields
and SHISU rendering remain unverified.
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
The filename format remains unsupported.
The ten Shift-JIS save-title digit strings accept exactly one full-width digit
`０` through `９`. Each three-byte allocation reserves NUL; its fourth alignment
byte and pointer word stay unchanged. Keep `renderers: []`. The title format uses
`shift-jis-title-format`: retain seven plain `%s` conversions and exactly 64 encoded bytes, followed by NUL. Use
full-width Latin letters, digits, ideographic spaces, parentheses and full stop.
Pad shorter wording with ideographic spaces. The original formatter and header
copy establish this fixed width statically. Keep all argument selectors and
`renderers: []`; this format does not use the game's ordinary text renderers.
SHOP's `%d` at `0x801D0C1C` retains exactly `%d` within two encoded bytes and
uses the five-byte unsigned 16-bit output bound. Retain renderer `0x8014F800`
through the original system bank's row 162 and its slot-zero command.
Active-bank publication and observed output remain unverified. Refuse empty
`ascii-format-literal` replacements, which can expose stale scratch bytes after
the initial NUL.
Controlled SHOP party-change checks cover the split HP/AP fields. Edit both
halves together and keep their separators in the same formatted character
column, since both draw at the same coordinates. Identical format-pair
allocations remain indistinguishable. Ordinary Buy-list checks verify the edited
price suffix; Sell quantity previews verify the edited price and total suffixes.
Other quantity paths, larger quantities, purchase effects and sale effects remain
unverified.
Corpus export includes these documents alongside bank documents.
Battle rows 1 and 3 accept `{glyph(0xNN)}`, with a nonzero operand and at most
five encoded bytes plus NUL. Preserve each record's other seven bytes.
BATTLE rows 4 and 5, `Defend` and `Charge`, use `game-literal`
with thirteen-byte allocations, including NUL. Preserve their `renderers: []`;
only the indexed bounded copy is qualified. Natural selectors, message
publication and rendering remain unverified for rows 3, 4 and 5. Row 3 also
retains `renderers: []`; its icon identities remain unverified.
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

BATL_END entry 0 exposes six decimal formats and one literal glyph. Retain each
recorded codec and its encoded and formatted bounds. The byte-plus-one format
retains an argument in 1..256 and at most three output bytes before NUL. The two
`%d` formats reach `0x8014F800` through bank command `0x07` and 32-byte dynamic
buffers. Runtime observations cover width edits for remaining EXP, next level
and current money, plus an edit to the currency glyph, in a controlled fixture
with zero rewards. Controlled EXP-pool and item-list fixtures also verify the
level-3 stat preview, an edited stat label, an item name edit and the
item-count format. These previews stop before stored level and stat updates or
inventory grant. Ordinary reward availability, normal cleanup and other
battle-results text remain unverified.
See the [format contract](../../../../tools/rust/bof3-text/docs/formats.md#battle-results-text).

The results ingress tests rising input at `0x80145AA8`; display waits test held
input at `0x80145AA4`. Releasing the native override does not clear these guest
samples. Release before the wait and reach the post-sample boundary at
`0x8014AB68` with both fields zero before resuming into the wait.

For encounter-template monster names, find `monster_entries` with `analyze`
and extract the selected entry explicitly. The `bof3.monster-text/v1` document
must retain all eight records, including empty names. Edit only `text`, using
at most eight literal bytes per name. Pack against the same source archive
with its recorded entry or `all`, without class or latch selectors. Shorter
names are NUL-padded; encounter data and stats remain unchanged. Corpus export
includes each matching entry. Controlled runtime checks verify the
`Gary` label from `AREA024.EMI` entry 11 edited to `Zary` in the first battle. Other
names, encounter families, regions and battle or substitution contexts remain
unverified.

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
results separate from optional rendering observations. To establish a rendering
claim, use disposable images and bounded emulator missions to observe the edited
consumer, including choices and substitutions when affected. A launch or
successful pack is not a rendering result.

Checked states from the current software GPU preserve VRAM and GP1 controls but
omit the GP0 drawing area and offset. After restoring, let the original game
reissue that environment before assessing pixels. Record the environment
commands and the scene's GPU ingress in the same observation.
