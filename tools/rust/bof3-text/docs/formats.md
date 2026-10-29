# Formats

The tool reads the text subfiles of a BOF3 EMI archive and writes a JSON
text-object document. The repository-wide authority is
[`docs/specs/formats/dialogue-text.md`](../../../../docs/specs/formats/dialogue-text.md);
this page describes how the tool represents it.

The editable schemas below belong to extraction, validation and packing.
`analyze` emits diagnostic `bof3.text-analysis/v1` reports; see the
[analysis command](commands.md#analyze-input) for their capacity and byte fields.

## Locating the subfile

The dialogue subfile is the EMI entry whose TOC `load_argument` is `0x80010000`.
Each supported dialogue archive contains one; `index` reports its payload offset and whether
it parses.

## VAB audio payloads

Audio body identification requires an EMI mode-7 entry and the latest preceding
mode-6 header in the same archive. The header must have the exact VAB header size,
at most 128 programs, version 7 and a sample count at `+0x16` no greater than 255.
Only size-table slots zero through that count contribute to the body length,
in eight-byte units;
inactive slots may contain nonzero bytes. The body must consume that length
exactly and be nonempty.

The original US loader selects the header at `0x80162790` and transfers the body
through `0x80162898`, `0x80163418` and `0x80174354` to SPU DMA. The SDK loop at
`0x80173ED0..0x80173F28` uses the active sample count. Matching another entry's
size alone does not identify audio. These checks do not classify container gaps.

## CDDA audio export

Coverage recognizes the exact PCM WAV export of original US audio track 2 by its
whole-file size and SHA-256. The complete 44-byte header describes signed 16-bit
stereo PCM at 44100 Hz. Its only data chunk contains the original audio bytes
from the CUE's `INDEX 01` to the track's end, copied by `extract_audio_tracks` in
[`bof3-disk/src/cue.rs`](../../bof3-disk/src/cue.rs). There are no extra chunks or
trailing bytes. The header and PCM data are known nontext.

The report retains the file's hash and source record. Any different WAV stays
unresolved, including a changed file with the same header. This classification
adds no editable text layout or playback acceptance.

## Raw sprite-bank coverage

For original US `BPLU349.EMI` entry 0, coverage recognizes the image headers,
packed image words, mode-2 token streams and 32-halfword palette records consumed
by `SLUS_004.22@0x8014E6D0`, `0x8014E7F8`, `0x8014E8FC` and `0x80166C48`.
It requires the original archive identity, raw transfer mode, load address
`0x8003B800` and the exact 331,788-byte payload hash. The fixed ranges use payload
coordinates and become file offsets by adding the entry's 2048-byte offset.

Only these serialized graphics ranges become known nontext. Mode-2 four-byte
opaque prefixes, allocation gaps, selection data and all other bytes remain
unresolved. Packed words contain unused bits; classification does not assign
those bits a meaning. It does not establish the absence of rasterized text,
complete CLUTs, runtime selection, upload, residency or display. Other sprite
banks remain unresolved under this rule.

## XA payload coverage

Coverage identifies encoded audio bodies in the original US `S_XA00.STR`,
`MAGIC00.STR`, `VOICE.STR` and `CAPCOM30.STR`. It requires the exact original
`SLUS_004.22` and each source's whole-file size and SHA-256 from
`config/original-us-disc.json`.
The original selector at `0x80163744` establishes the XA consumer family.

Each candidate must have matching subheader copies and Form 2 audio submode
without video or data bits. Coding must specify supported four-bit mono or
stereo audio without emphasis. All eighteen groups must have matching parameter
copies, shifts at most twelve and predictors below four, following the existing
audio decoder's format checks.

For each validated sector index `s`, only `[2336*s+8, 2336*s+2312)` is known
nontext. Every byte in this 2304-byte body is an active parameter, a verified
parameter copy or packed samples. This format classification includes validated
bodies outside the original cue prefixes.

The twenty bytes at `[2336*s+2312, 2336*s+2332)` must also be entirely zero.
Coverage identifies these zero bytes as nontext under the same exact-source
qualification. Their original purpose and consumption remain unverified; this
rule does not establish padding or reusable allocation.

Coverage also identifies empty Form 1 user payloads in these pinned sources.
They require matching subheader copies, no audio, video or data submode bits,
zero coding information and an entirely zero 2048-byte user payload. Only
`[2336*s+8, 2336*s+2056)` becomes known nontext. Nonzero Form 1 payloads stay
unresolved. Empty payloads do not establish padding, unused allocation or their
original purpose and consumer.

The same exact-source qualification identifies duplicated subheaders at `[0,8)`
within each sector. Form 1 correction fields at `[2056,2336)` contain EDC and
ECC P/Q, verified against the [disk sector owner](../../bof3-disk/src/replace/sector.rs).
Form 2 fields at `[2332,2336)` are stored zero and represent absent EDC, as defined
by the [audio EDC owner](../../bof3-audio/src/codec/edc.rs). They are not matching
checksums. Coverage identifies these metadata fields as nontext without adding
an editable layout.

In `CAPCOM30.STR`, the original LOGO reader at `0x801D29B4` and `0x801D29C8`
checks a packet marker and type at the start of each video sector's user payload.
The CD mode and DMA path at `0x801D28A4` bind that reader to the user-payload
start. Coverage identifies only `[2336*s+8, 2336*s+12)` as packet metadata when
the Form 1 video subheader and exact `60 01 01 80` marker/type bytes match.
The remaining packet header stays unresolved.

Coverage also identifies conservative coded prefixes in CAPCOM frames 1 through 221.
Their original LOGO consumer at `0x801D7578` reads fresh input and returns zero
with its next unread halfword in `a0`. Each reviewed prefix excludes the final
eight fetched bytes to cover unread lookahead. The original packet index, count
and frame ID bind each prefix to ordered 2016-byte bodies at sector offset 40.
Coverage admits only the recorded source slices, totaling 1,607,660 bytes.
It leaves 430,516 video-body bytes unresolved, including excluded tails and
frames 222 through 231. These bounds establish neither each frame's logical end
nor whole-movie playback.

Static format identification does not establish original sector selection,
arrival, playback, command completion, predictor history or hardware handling
of parameter copies.
Form 1 user payloads outside the reviewed empty payloads, packet marker/type
fields and reviewed coded prefixes remain unresolved.
Exact playback endpoints and packed-ID caller bounds remain unverified. Changed
sources, other builds and arbitrary files that resemble XA remain unresolved.

## Executable ASCII

`extract`, `analyze`, `validate` and `pack` also support exception-screen ASCII
in the US `SLUS_004.22` executable and the `FILE NOT FOUND` diagnostic with its
three-ID hexadecimal format, plus the BIOS `DiskError: ` prefix and six CD
interrupt labels, 22 CD command names, fixed CD diagnostic literals and BIOS
printf formats and GPU diagnostics. `extract-corpus` exports supported executable
strings under their relative source paths. Exception literal strings use
the pointer words at `0x8017F470..0x8017F53C`, with an exclusive upper bound.
The consumer at `0x8014B3C4` passes these pointers to renderer `0x80150098`.
The renderer reads bytes until NUL, advances on ASCII space and handles newline
`0x0A`. These bytes use ASCII rather than the dialogue character table.

The JSON schema is `bof3.executable-text/v17`. Re-extract older documents before
editing. It contains `source`, mandatory `source_sha256`, `build`, `load_address`,
`consumers` and `strings`.
Each string retains its `address`, whole-file `file_offset`, original `capacity`
including NUL, immutable `consumers`, `codec`, `format_directives` and `output`,
typed `references` and editable `text`. Row `consumers` lists the containing source functions;
document `consumers` is their sorted union. Each `output` is `game-screen`, with
its renderer and optional formatter, or `bios-printf` or `bios-puts`, with its
stub, vector and selector. `bios-printf-callback` records `callback_word`,
`initial_stub`, `vector` and `selector`. Keep output metadata unchanged.
Exception literal rows use `ascii-literal` and `pointer-word` references with an `address`.
Shared pointers appear on one allocation. Retain every allocation and edit only
`text`.

For literal rows, packing accepts printable ASCII and newline, reserves one byte
for NUL and refuses growth beyond the original string, excluding alignment
padding. A shortened string is NUL-padded inside its allocation. Unchanged strings
preserve every byte. Reference edits, relocation and unproven exception-table
indices are refused.

`LOGO/LOGO.EXE` has a separate pinned layout, loaded at `0x801CE000`. It supports
kernel, CD, GPU and MDEC diagnostics. Use `analyze SOURCE --json` to discover their
addresses, capacities, directives and output paths. Direct BIOS printf calls use
stub `0x801D791C` (`A0:3F`). The interrupt formats receive unsigned 16-bit values;
DMA values are full 32-bit words and the MADR index is 0..6. Preserve every
original directive exactly and in order. The CdInit and `DiskError: ` direct
printf literals refuse percent characters. Alignment padding remains protected.

LOGO CD labels `CD_sync`, `CD_ready`, `CD_cw` and `CD_datasync` pass through shared
word `0x801EB4F0` as `%s` data and allow literal percent bytes. Their references
have empty `calls` arrays; each row records its traced consumer, without claiming
reachability through every reader of that word. Command and status table labels
remain unsupported because their getters have unresolved downstream uses. Raw
table indexes also prevent a closed `%s` argument domain or formatted size bound.

LOGO GPU callback rows record word `0x801DA1BC`, initially `0x801D791C`; its runtime
value remains unverified. ResetGraph address and GPU timeout diagnostics call the
BIOS stub directly. Four image-operation names pass through a helper as `%s`
data, allowing literal percent bytes. The `&08x` in the original format at
`0x801CE5F8` is literal text, not an extra directive. The version string at
`0x801CE444` remains unsupported: ResetGraph prints its carrier address rather
than its contents. Runtime console execution and other LOGO consumers remain
unverified.

LOGO MDEC diagnostics call the BIOS stub directly. The two formats at
`0x801CE6B4` and `0x801CE6DC` use `ascii-bios-printf-tab-format`, which also
allows ASCII tab (`0x09`) within their original capacities. Other executable
codecs continue to refuse tabs. Their DMA flags and FIFO status arguments are
individual bits; DMA addresses are full 32-bit words. The bad-option format
receives the original signed word without narrowing. The two sync names pass
through helper `0x801D7414` as `%s` data and allow literal percent bytes.

The `%08X` format at `0x80149990` has a five-byte allocation including NUL and
uses `ascii-u32-hex-format`. Its `address-pair` references retain `upper` and
`lower` instruction addresses and their `calls`. Calls at `0x8014B520`,
`0x8014B5A0`, `0x8014B610` and `0x8014B634` pass one 32-bit value to formatter
`0x8017E3F4`, then draw the result with `0x80150098`.

This codec accepts one entire `%X` or `%x` conversion, optionally with a minimum
width from 1 through 8 and zero padding, such as `%8X` or `%08x`. It refuses
surrounding text, other conversions, additional flags, widths outside this range
and extra arguments. The encoded format must fit four bytes plus NUL; formatted
output remains at most eight bytes plus NUL. `analyze` reports
`max_formatted_size` separately from the encoded allocation size.

The consumer at `0x80166DF4` draws `FILE NOT FOUND` from `0x80149F9C`
through the same ASCII renderer. Its 15-byte allocation includes NUL. The
`address-pair` reference at `0x80166E1C`/`0x80166E20` feeds the draw call at
`0x80166E24`.

The companion `ID %02X/%02X/%02X` at `0x80149FAC` has an 18-byte allocation
including NUL and uses `ascii-u8-triple-hex-format`. Its reference pair at
`0x80166E3C`/`0x80166E40` feeds formatter call `0x80166E44`. Three unsigned
byte loads from scratchpad offsets 0, 1 and 2 supply the arguments in order;
the third passes through stack offset `0x10`. Call `0x80166E58` draws the result.
This codec permits exactly three `%X` or `%x` conversions, optionally with
width 1..2 and zero padding, plus literal ASCII. It refuses other percent
sequences, flags, dynamic widths, reordered arguments and output exceeding
11 bytes plus NUL. The encoded format must fit 17 bytes plus NUL.

The `DiskError: ` prefix at `0x8014A30C` uses `ascii-printf-literal` and a
12-byte allocation, including NUL. It accepts at most eleven printable ASCII
bytes or newlines and refuses every percent character, including `%%`. Its
`address-pair` reference at `0x80175E40`/`0x80175E44` feeds call `0x80175E48`
in the CD interrupt consumer at `0x80175C60`. The output is `bios-printf`:
stub `0x8017E3E4` jumps to BIOS vector `0xA0` with selector `0x3F`.
This is separate from the game-screen renderer. The call requires a positive
CD debug word, which the original executable initializes to zero. Editing the
prefix does not enable that output; runtime acceptance remains unverified.

The six named CD interrupt labels use `ascii-literal`. Their original allocations
include NUL and exclude alignment bytes:

| Address | Original text | Capacity | Pointer word |
| --- | --- | --- | --- |
| `0x8014A2A0` | `DiskError` | 10 | `0x80185838` |
| `0x8014A2AC` | `DataEnd` | 8 | `0x80185834` |
| `0x8014A2B4` | `Acknowledge` | 12 | `0x80185830` |
| `0x8014A2C0` | `Complete` | 9 | `0x8018582C` |
| `0x8014A2CC` | `DataReady` | 10 | `0x80185828` |
| `0x8014A2D8` | `NoIntr` | 7 | `0x80185824` |

Each row records the immediate table consumer `0x8017560C`. It masks the
selector to eight bits and reads the table only for indices below seven.
Separate readers pass labels as the third and fourth `%s` arguments to BIOS
printf at `0x801762F8`, `0x80176580`, `0x801769F0` and `0x801770E8`.
They index the table directly. Percent characters in a label remain literal
argument bytes. The immutable `bios-printf` output describes this known output
path; observed BIOS output and other callers remain unverified.

The 22 named CD command strings use the same literal codec and BIOS output.
Each retains its original bytes plus NUL as its capacity. Use `analyze` to list
their addresses and limits. Their pointer words lie in
`0x801857A4..0x80185814`, with an exclusive upper bound. Immediate consumer
`0x801755D8` masks the selector to eight bits and admits indices below 28.
The shared `?` allocation at `0x8014A1F8` accepts one literal ASCII character
plus NUL. Its twelve immutable pointer words include command indices 15, 18,
23..26 and 28..31, interrupt index 6 and adjacent word `0x80185840`. The checked
lookups retain their bounds; separate printf readers bypass those bounds. This
row records both immediate lookup consumers. The separate `none` return string
has no proven downstream output consumer and remains unsupported.

Separate readers pass command names as `%s` arguments to BIOS printf at
`0x80175E94`, `0x801762F8`, `0x80176580`, `0x801769F0` and `0x801770E8`.
Calls at `0x80176790` and `0x801767E8` use the formats `%s...\n` and
`%s: no param\n`, respectively. These readers bypass the lookup. Observed BIOS
output remains unverified. Editing a name changes its text, without changing command
numbers, pointers, code or debug settings.

Fixed CD diagnostics retain their original capacities and every proven address
pair. Use `analyze` for their addresses, limits and containing functions. The
`CD timeout: ` allocation has four consumers. It and the literals `CDROM: unknown
intr`, `CD_init:`, `CdRead: sector error\n`, `CdRead: Shell open...\n` and
`CdRead: retry...\n` use `bios-puts`: stub `0x8017E33C`, vector `0xB0`, selector `0x3F`. They use
`ascii-literal`, so percent characters remain data. The six direct printf
literals beginning with `CdSearchFile: disc error`, `CD_newmedia:` or
`CD_cachefile:` use `ascii-printf-literal` and refuse every percent character.
Original spelling, including `sarching`, is preserved on unchanged round trips.

The labels `CD_sync`, `CD_ready`, `CD_cw` and `CD_datasync` also use
`ascii-literal`. Their address pairs store pointers in shared word `0x8018B5F0`;
their reference `calls` arrays are empty because intervening calls leave the
store-to-use relationship unproven. Readers at `0x801762B8`, `0x80176540`,
`0x801769B0` and `0x801770AC` load that word as the first `%s` argument to
printf calls `0x801762F8`, `0x80176580`, `0x801769F0` and `0x801770E8`,
respectively. This records a shared-word output path without asserting that
every stored label reaches every reader. BIOS output remains unobserved.

BIOS diagnostic formats use `ascii-bios-printf-format` or the LOGO MDEC
`ascii-bios-printf-tab-format`. Edit only their literal
text within the original capacity, reserving NUL. Each row's immutable
`format_directives` lists the exact original conversions in order. Packing
requires every token unchanged, including case and width, and refuses missing,
extra or other percent sequences, including `%%`. Other rows retain an empty
`format_directives` array. Use `analyze` to inspect addresses and capacities.

The CD formats, ResetGraph address diagnostic at `0x8014A728` and GPU timeout
formats at `0x8014A8DC` and `0x8014A910` pass directly to BIOS printf without an
intermediate game buffer.
Analysis omits `max_formatted_size`; it does not bound the rendered output.
In particular, `%02x` specifies a minimum width, not a two-digit limit.
The format at `0x8014A318` receives word loads for both hex arguments. The
reference pairs for `0x8014A410` and `0x8014A470` converge on call `0x801775E4`.
Other BIOS output paths require their own runtime observations.

GPU diagnostic formats use the same exact-directive codec. Most use
`bios-printf-callback`, recording callback word `0x8018B158`, initially
`0x8017E3E4` (BIOS A0/3F). The reference `calls` are indirect `jalr` sites through
fresh loads of that word. Source binding protects the initial callback word;
it does not prove the callback remains unchanged during execution. No
intermediate formatting buffer is present in these traced paths. Preserve each
row's exact directives, including `%3d`. Analysis reports no formatted output
bound. The direct BIOS formats above use `bios-printf` output. Original spelling,
including `SetGrapQue`, is preserved. `DrawOTagEnv` contains only one conversion:
its original `&08x` is literal text. Callback targets and output through the
callback remain unverified at runtime.

`ClearImage`, `LoadImage`, `StoreImage` and `MoveImage` use `ascii-literal` with
empty `format_directives`. Their reference calls pass the names to helper
`0x8017B438`, which forwards them as `%s` data to the callback. Percent characters
in these names remain literal data. Each edit preserves bytes outside its allocation.

The matrix-stack guard messages at `0x80186888` and `0x801868B9` pass directly
to BIOS printf. They use `ascii-printf-literal`, with empty `format_directives`,
and refuse percent characters. Their address-pair lower instructions execute in
the recorded calls' delay slots. No intermediate formatting buffer is present;
the separate word at `0x801865F8` saves the return address.

SPU timeout, sequence, interrupt, DMA and CD-init diagnostics also pass directly
to BIOS printf. The three SPU wait labels are `ascii-literal` arguments to `%s`;
percent characters remain data. The sequence and CD-init messages use
`ascii-printf-literal` and refuse percent characters. The interrupt formats
retain their exact `%04x` conversions. The VSync timeout message passes directly
to BIOS puts, so percent characters remain literal. These output paths use no
intermediate game buffer; their runtime output remains unobserved.

Build recognition hashes all bytes outside the supported allocations against
the pinned US layout. This permits re-extraction of supported edits while
refusing changes to code, pointers, headers or unrelated data. Each document
additionally binds the complete current source SHA-256 and canonical path.
Source tracing establishes this layout; runtime acceptance and other executable
consumers remain separate coverage requirements.

## Record names

The US `BIN/ETC/GAME.EMI` entry 0 contains inline names for items, key items,
weapons, armor, accessories and abilities. `extract`, `analyze`, `validate`, `pack` and
`extract-corpus` support these fields through `bof3.record-text/v1`.
The equipment resolver at `0x80165D48` computes the record address from category
and index. The COMMU00 entry 0 consumer at `0x801F1A30` copies twelve equipment
name bytes into a separate buffer and appends NUL. The SISYOU entry 0 consumer
at `0x801D3D64` copies twelve ability name bytes from `0x801CA70C + index × 0x14`
and appends NUL. A name can therefore occupy all twelve bytes.
The [equipment contract](../../../../docs/specs/data/equipment.md) describes the
table locations and record strides.

The document retains `source`, mandatory `source_sha256`, `entry`,
`load_address`, `equipment_resolver`, `equipment_name_copy_consumer`,
`ability_name_copy_consumer` and `names`. Each name has
`family`, zero-based `index`, whole-file `file_offset`, `address`, `capacity`
and editable `text`. Retain every record and edit only `text`. Names use the
US literal character table, including `0xFF` for space. They contain no dialogue
commands or markup. A shorter name is NUL-padded to twelve bytes; a full field
needs no terminator inside the record. Overflow and unknown characters fail.

Build recognition pins every archive byte outside these name fields. Packing
also checks the complete current source SHA-256, canonical source path and all
record identities. Stats, flags, prices, offsets and other entries remain
unchanged. Other record families require their own consumer evidence. Runtime
acceptance remains a separate gate.

## Default character names

`START.EMI` entry 8 and `STATUS.EMI` entry 0 contain copies of eight default
character records. Each record has a five-byte name followed by other character
data at stride `0xA4`. The table starts at whole-file offset `0x72914` in START
and `0x1B114` in STATUS; both load to `0x801EB514` within the entry loaded at
`0x801D0C00`. The initializer at entry address `0x801D15C0` copies records into
the mutable array at `0x80144968`. SISYOU entry 0 routine `0x801D39CC` copies five
name bytes and appends NUL. Its draw call at `0x801D1848` passes a record pointer
and a five-byte limit to executable renderer `0x8014F800`.

Extract the selected entry as `bof3.inline-text/v23`. Its first eighteen strings include
the ten counted labels at `0x801D0C7C`, with stride 20, followed by these eight
names at indices 10 through 17. The label consumer calls `0x8014FC90` at
`0x801DCF50`, using the same count convention as the BATE, SHISU and SHOP tables.
Names use `game-fixed-literal`, have capacity five and may occupy all five bytes.
Shorter names are NUL-padded.

The active US name-entry keyboard starts at `0x801ECE7C`. Initialization at
`0x801E4670` sets the page selector at `0x801ED7E8` to two, selecting this page
from three 96-byte pages at `0x801ECDBC`. The draw loop at `0x801E6490` walks
eight rows of twelve cells and calls `0x8014F800` with a one-byte limit at
`0x801E6680`. Selection at `0x801E4AD8` reads the same cell and copies ordinary
characters into the five-byte name buffer at `0x801ED100`.

The next 68 strings expose ordinary cells through `game-keyboard-character`.
Each accepts exactly one letter, digit, space or one of `! ? - , .`. Zero cells,
command bytes one through four and the other two pages stay protected. Strings
86 through 89 are `OK`, `Del`, `Next` and `Back` at `0x801ED190`, `0x801ED194`,
`0x801ED198` and `0x801ED1A0`. Their allocations are three, four, five and five
bytes, including NUL. The pointer table at `0x801ED1B8` supplies them to the
`0x8014FC90` call at `0x801E692C`. Alignment bytes and pointer words stay fixed.
The next fifteen strings expose four stat-label groups through `game-literal`.
The groups start at `0x801EC940`, `0x801EC99C`, `0x801ECB0C` and `0x801ECB2C`.
The first three contain `Pwr`, `Def`, `Int` and `Agl`; the last contains `Pwr`,
`Def` and `Agl`. The first group uses four-byte records and the others use
six-byte records. Each editable allocation is four bytes, including NUL, and
accepts up to three encoded bytes. The two trailing bytes of six-byte records
stay protected. The fixed-address consumers call `0x8014F800` at
`0x801DBC88`, `0x801DBCFC`, `0x801DBD64`, `0x801DBDC0`, `0x801DC90C`,
`0x801DC934`, `0x801DC95C`, `0x801DC978`, `0x801DE6EC` and `0x801DF1AC`.
These stat labels have no pointer-table references. Controlled Equipment member
selection verifies edited labels in the group at `0x801ECB0C`. Identical START
and STATUS payloads leave the active supplier indistinguishable. Other groups and natural menu availability remain unverified.

The final string exposes the four-byte `%3d` allocation at `0x801D0C4C` through
`ascii-u16-decimal-format`. Its eighteen formatter sites read one unsigned byte
or halfword, write to the 64-byte scratch buffer at `0x80145AD4`, then draw through
`0x80150098`. Retain one decimal argument, at most three encoded bytes plus NUL,
and at most five formatted characters. Format widths use 1 through 4. The next
allocation at `0x801D0C50` stays protected. Controlled Equipment member
selection verifies decimal width 2 in this stat panel. Other numeric paths and
values remain unverified.

Retain all 106 allocations and edit only `text`. Re-extract older inline
documents before editing or packing.

Recognition pins the selected payload outside its supported allocations.
Packing binds the complete archive SHA-256 and canonical path and checks every
allocation. Stats, the other twelve bytes in each label record and all other
archive bytes remain unchanged. The copies remain separate sources. Changing
START does not update STATUS or an existing save's live names. Save propagation,
other initializers and edited rendering remain separate requirements.

## Monster names

US encounter templates loaded at `0x800E4000` have a 72-byte prefix followed by
eight records at stride `0x88`. Each record starts with an eight-byte name at
`0x800E4048 + index × 0x88`. The template copy at `0x800AC218` transfers those
eight bytes to the enemy's battle record. Lookup at `0x800AC704` searches the
eight template records. Battle name copy at `0x801DEA18` copies eight bytes
from the live battle record through `0x801501E4`, which appends NUL.

The tool recognizes the 1160-byte entry layout and load argument. The schema
`bof3.monster-text/v1` retains `source`, `source_sha256`, `entry`, `entry_offset`,
`load_address`, `template_copy_consumer`, `template_lookup_consumer`,
`name_copy_consumer` and `names`. Each name uses the record identity fields
above, with `family` equal to `monsters` and `capacity` equal to eight.
Retain all eight records, including empty names, and edit only `text`.
Names use literal game characters without dialogue commands. Full eight-byte
names need no internal NUL; shorter names are NUL-padded.

Packing checks the complete source digest, canonical path, entry offset and
layout before writing. Encounter data, stats and all other bytes stay unchanged.
Different archives remain separate sources. This layout recognition does not
establish support for another region or prove observed rendering.

## Inline overlay text

US `COMMU03.EMI` entry 6 loads at `0x801D0C00`. Its `PLAY      EXIT`, `MONO`
and `STEREO` labels at `0x801D0C1C`, `0x801D0C2C` and `0x801D0C34` use
`ascii-literal`, with capacities of fifteen, five and seven bytes including NUL.
Pointer words at `0x801D190C`, `0x801D1908` and `0x801D1904` supply the strings
to calls at `0x801D1468` and `0x801D1430`. The overlay-local renderer at
`0x801D1714` stops at NUL, advances eight pixels per character and treats
ASCII `0x20` as a space. Edits accept printable ASCII within each allocation.
Pointer words, alignment padding, other labels and arrow glyphs remain protected.
Controlled runtime observations verify all three edited labels. The display
selects `STEREO` when byte `0x80144951` is zero and `MONO` when it is nonzero.
Natural menu and configuration producers, audio output and the other labels'
consumers remain unverified.

The supported US battle code payload loaded at `0x80096800` contains two
five-entry pointer tables at `0x800B6E88` and `0x800B6EC4`. Draw calls at
`0x800B28C8` and `0x800B2F70` pass their selected strings to renderer
`0x8014F800`. These item and ability category labels use literal game characters
and NUL termination. The layout occurs in BATTLE entry 15 and matching payloads
in other archives; use `analyze` to discover the actual entry.

US `BATE.EMI` entry 0 loads at `0x801D0C00`. Its pointer tables at `0x801D8E4C`,
`0x801D8E80` and `0x801D8EB8` contain five, four and five supported category
labels. Calls at `0x801D6F84`, `0x801D7600` and `0x801D7E8C` pass selected
strings to `0x8014F800`. These `game-literal` fields retain their original string
length plus NUL, with capacities of five to seven bytes. Alignment padding,
pointer tables and code remain outside the editable allocations. Controlled runtime
checks cover the equipment-screen `WEAPON` and `ARMOR` category labels. Their
identical allocations remain indistinguishable; other category slots remain
unverified.

US `SHISU.EMI` entry 0 loads at `0x801D0C00`. Its category tables at
`0x801D9E70`, `0x801D9EA4` and `0x801D9EDC` contain five, four and five labels.
Calls at `0x801D7F2C`, `0x801D85A8` and `0x801D8E34` pass selected strings to
`0x8014F800`. US `SHOP.EMI` entry 0 uses the same load address, with tables at
`0x801E5B48`, `0x801E5B7C` and `0x801E5BB4` and calls at `0x801DCA2C`,
`0x801DD0A8` and `0x801DD934`. Each archive has its own source-bound layout.
These `game-literal` fields also retain their original string length plus NUL,
with capacities of five to seven bytes. Slot padding, pointers and code remain
protected.

BATE `0x801D0C20` and SHISU `0x801D0C04` each expose `%3d` through
`ascii-u16-decimal-format`. Each four-byte allocation permits three encoded
bytes and NUL. Eighteen formatter sites per overlay pass one zero-extended byte
or halfword, then draw scratch `0x80145AD4` through `0x80150098`. Preserve one
decimal argument and the five-character output bound. Fixed address calculations
keep pointer-reference lists empty; the following format remains protected.
Controlled BATE observations verify a narrower minimum width on one stat
panel. Other BATE paths and SHISU rendering remain unverified.

BATE and SHISU also expose fifteen stat-label allocations each through
`game-literal`. Each accepts at most three encoded bytes and one NUL.
The first three groups contain `Pwr`, `Def`, `Int` and `Agl`; the last contains
`Pwr`, `Def` and `Agl`.

| Archive | Group bases | Record strides |
| --- | --- | --- |
| BATE | `0x801D8B28`, `0x801D8B84`, `0x801D8CF4`, `0x801D8D14` | 4, 6, 6, 6 |
| SHISU | `0x801D9B4C`, `0x801D9BA8`, `0x801D9D18`, `0x801D9D38` | 4, 6, 6, 6 |

Each allocation covers only its first four bytes. The two trailing bytes in
six-byte records stay protected. Fixed address calculations pass these fields
to `0x8014F800`, so their pointer-reference lists are empty. The comparison loops
select four records; the summary loops select three. All related instructions
remain pinned outside the editable allocations. Controlled BATE observations
verify the group at `0x801D8CF4`. Other groups and SHISU rendering remain unverified.

BATE and SHISU entry 0 also expose `%3d/%3d` at `0x801D0D5C` and
`0x801D0D40`, respectively. Each eight-byte allocation includes NUL. The
`ascii-u8-decimal-pair-format` codec preserves two ordered decimal arguments and
accepts printable ASCII within seven encoded bytes and seven formatted bytes.
Widths use 1..4 with optional zero padding; output growth and other conversions
are refused. BATE calls formatter `0x8017E3F4` at `0x801D6FC0` and `0x801D7EAC`;
SHISU calls it at `0x801D7F68` and `0x801D8E54`. Each call masks its first
argument to eight bits and supplies 32 or 128 as the second argument. The result
at `0x80145AD4` is passed to renderer `0x80150098`. Controlled checks cover the edited
BATE equipment-screen pair; SHISU rendering remains unverified.

Both archives also expose `%2d` and `%02d %02d`. The single-byte format uses
`ascii-u8-decimal-format`, with three encoded bytes and three formatted bytes.
The pair uses `ascii-u8-decimal-pair-format`, with nine encoded bytes and seven
formatted bytes. Their four-byte and ten-byte allocations include NUL. BATE
addresses are `0x801D0D80` and `0x801D0D84`; SHISU addresses are `0x801D0D64`
and `0x801D0D68`. Calls at `0x801D8394`, `0x801D83D0`, `0x801D933C` and
`0x801D9378` load unsigned byte arguments from record offsets 8, 12 and 13.
They format into `0x80145AD4` and pass the result to `0x80150098`.

The four-byte labels at BATE `0x801D0D90` and SHISU `0x801D0D74` originally
contain ASCII `  :` followed by NUL. Their `ascii-literal` codec permits three
printable ASCII bytes and zero-fills unused bytes. Spaces encode as `0x20`;
format conversions and command markup have no special meaning. Calls at
`0x801D841C`, `0x801D8438`, `0x801D93C4` and `0x801D93E0` pass these labels
directly to `0x80150098`. Other formats and runtime rendering remain unverified.

BATE `0x801D0D1C` and SHISU `0x801D0D00` also expose `%02d` in five-byte
allocations. The `ascii-u8-decimal-format` codec preserves one unsigned byte
argument, at most four encoded bytes and three formatted bytes. The callers
load bytes from `0x80144FC0` and `0x80144FC1`, format into `0x80145AD4` and
draw through `0x8014FF0C`.

BATE `0x801D0C44` and SHISU `0x801D0C28` expose `%7d` in four-byte
allocations. Their `ascii-i32-decimal-format` codec preserves one signed
32-bit decimal argument. It allows widths 1..7 with optional zero padding,
within three encoded bytes and eleven formatted bytes. The original formatter
fetches a full word, converts its unsigned magnitude and counts any minus sign
within the field width. Even `-2147483648` fits the eleven-byte output bound.
These strings have proven consumers through both `0x8014FF0C` and `0x80150098`.

The adjacent eight-byte allocations at BATE `0x801D0C48` and SHISU `0x801D0C2C`
hold the fallback `-------`. Their `ascii-format-literal` codec permits one to
seven printable ASCII bytes and refuses every percent sequence. The original calls
pass no formatting arguments and draw the result through `0x80150098`. Packing
reserves NUL in all these allocations. Runtime rendering remains unverified.

US `SHOP.EMI` entry 0 also exposes the ASCII clock format at `0x801D0C68`,
originally `BOF3-%02d/%02d/%02d`. Its twenty-byte allocation includes NUL.
The `ascii-u8-decimal-triple-format` codec preserves three ordered decimal
arguments and accepts printable ASCII within nineteen encoded bytes and sixteen
formatted bytes. Each argument is an unsigned byte. Widths use 1..4 with optional
zero padding; other conversions are refused. Consumer `0x801D5894` loads the
three clock bytes and calls formatter `0x8017E3F4` at `0x801D597C`, then copies
the result to a stack buffer. A rendered consumer is unverified, so `renderers`
remains empty. The filename and Shift-JIS header formats are unsupported.

SHOP's `%d` at `0x801D0C1C` uses `ascii-u16-decimal-format` in a three-byte
allocation including NUL. Its encoded limit permits only `%d`; the codec's
conservative output bound is five bytes. Consumer `0x801D3B98` multiplies an
unsigned byte by ten, so actual output uses at most four bytes. After formatting
into `0x80145AD4`, it copies eight bytes to text slot zero at `0x801490D8` and
appends NUL at offset eight. Preserve `renderers: []`: the active message-bank
binding and rendering during SHOP remain unverified. The padding byte at
`0x801D0C1F` remains protected.

BATE, SHISU and SHOP each also contain ten 20-byte label records, starting at
`0x801D0C50`, `0x801D0C34` and `0x801D0D14`, respectively. Each record begins
with seven literal bytes and a one-byte count. Calls at `0x801D3FB0`,
`0x801D4F58` and `0x801D9A58` pass the selected record and its count to
`0x8014FC90`, which stops at NUL or the supplied count.
These `game-counted-literal` allocations use all eight bytes, accept at most
seven encoded characters, and need no NUL when full. Packing updates the count
and zero-fills unused character bytes. It preserves the remaining 12 bytes of
each record. Glyph commands are refused. Caller selector bounds, other text
and observed rendering remain unverified.

`bof3.inline-text/v23` retains `source`, `source_sha256`, `layout`, `entry`,
`entry_offset`, `load_address`, `consumers` and `strings`. Each string
has `index`, `address`, whole-file `file_offset`, `payload_offset`, `capacity`,
`renderers`, `codec`, pointer-word `references` and editable `text`. Retain all
fields and edit only `text`. The ordered `renderers` array retains every proven
renderer for that allocation; an empty array means no final renderer is verified.
It does not establish that all possible consumers have been found. Allocations
exclude alignment padding. Terminated
rows reserve one byte for NUL, and `game-counted-literal` rows reserve one byte
for the trailing count. Both accept at most `capacity - 1` encoded bytes.
`game-fixed-literal` rows accept all `capacity` bytes without a reserved
terminator. Shorter text leaves zero-filled unused character bytes. These three
literal codecs use the game's character encoding. Counted rows store the encoded
length in the allocation's last byte.
`ascii-decimal-format` rows use printable ASCII and must retain exactly one
`%3d`; all other percent sequences are refused. `game-glyphs` rows also accept
`{glyph(0xNN)}`, encoded as `0x15` followed by one nonzero byte. Glyph names and
displayed meanings are unverified. Other commands and pointer changes are refused.
Re-extract older inline documents before editing or packing.

The `US COMMU02 menu labels` layout covers entry 8 of `BIN/ETC/COMMU02.EMI`
and entry 7 of `COMMU02B.EMI`, loaded at `0x801D0C00`. It exposes eleven
literal allocations through the inline schema:

| Address | Original game text | Codec | Capacity in bytes | Pointer word |
| --- | --- | --- | --- | --- |
| `0x801DAC54` | `HIGH` | `game-fixed-literal` | 4 | None |
| `0x801DAC58` | `LOW` | `game-fixed-literal` | 3 | None |
| `0x801DAC5C` | `SET` | `game-fixed-literal` | 3 | None |
| `0x801DACA8` | `Cash` | `game-fixed-literal` | 4 | None |
| `0x801DACB0` | `Pot` | `game-fixed-literal` | 3 | None |
| `0x801DB010` | `OK` | `game-literal` | 3 | `0x801DB038` |
| `0x801DB014` | `Del` | `game-literal` | 4 | `0x801DB03C` |
| `0x801DB018` | `Next` | `game-literal` | 5 | `0x801DB040` |
| `0x801DB020` | `Back` | `game-literal` | 5 | `0x801DB044` |
| `0x801DB028` | `End` | `game-literal` | 4 | `0x801DB048` |
| `0x801DB034` | `.` | `game-literal` | 2 | `0x801DB054` |

Fixed fields use their full capacity. The other capacities include one NUL byte.
The marker's original byte is `0x3E`, which the game character table maps to `.`.
Consumer `0x801D2CD0` draws the indexed HIGH/LOW/SET fields through `0x8014F800`
with limits four, three and three. Calls `0x801D320C` and `0x801D3334` draw Cash
and Pot with limits four and three. Calls `0x801D9868` and `0x801D98CC` pass
the five menu pointers and the separate marker to `0x8014FC90`, with limits
sixteen and one. Packing preserves pointers, alignment bytes, adjacent currency
glyphs and all other payload bytes.

The same layout also exposes eight ASCII formats. Capacities include NUL and
exclude alignment padding:

| Address | Original bytes before NUL | Capacity | Codec |
| --- | --- | --- | --- |
| `0x801D0C04` | `%7d` | 4 | `ascii-i32-decimal-format` |
| `0x801D0C08` | `%02d` | 5 | `ascii-i32-decimal-format` |
| `0x801D0C10` | `%03d` | 5 | `ascii-i32-decimal-format` |
| `0x801D0C18` | `%04d` | 5 | `ascii-i32-decimal-format` |
| `0x801DAD60` | Three `0x5C` bytes | 4 | `ascii-format-literal` |
| `0x801DAD64` | One `0x5C` byte | 2 | `ascii-format-literal` |
| `0x801D0C24` | `%d%d%d` | 7 | `ascii-u8-decimal-triple-format` |
| `0x801D0C20` | `%d` | 3 | `ascii-i32-decimal-format` |

The formats at `0x801D0C04` through `0x801D0C18` retain one signed 32-bit argument.
Calls load full words from `0x80144F50` or `0x801DB07C`; signed comparisons against 10, 100 or
1000 do not establish a nonnegative or byte-sized domain. The codec permits
widths 1..7 with optional zero padding, within the encoded allocation and eleven
formatted bytes before NUL. Formatter `0x8017E3F4` writes into `0x80145AD4`;
calls `0x801D3254` and `0x801D350C` draw through `0x8014FF0C`.

The two fallback formats use nonempty printable ASCII within three and one encoded bytes.
They refuse every percent sequence because their calls supply no formatting
arguments. Calls `0x801D4BBC`, `0x801D4BF0`, `0x801D4DCC` and `0x801D4E00`
format into the same buffer and draw through `0x8014FF0C`.
Empty `ascii-format-literal` replacements are refused because these renderers
advance past an initial NUL and can consume stale scratch bytes. Nonempty
all-space replacements terminate safely; arbitrary glyph meanings in COMMU02's
numeric atlas remain unverified.

The triple format at `0x801D0C24` retains three ordered unsigned byte arguments.
Calls `0x801D4C44` and `0x801D4D24` load them with `lbu`, format into
`0x80145AD4` and draw through `0x8014FF0C`. Their output needs at most nine
bytes before NUL. Its six encoded bytes leave no room for widths or literals,
so packing requires the original `%d%d%d`. The alignment byte at `0x801D0C2B`
stays unchanged.

The shared `%d` at `0x801D0C20` reserves one NUL byte and must remain exactly `%d`.
Four callers load unsigned bytes and draw through `0x8014FF0C`. Three other calls
write `0x801490D8` or `0x801490F8` before dispatch through `0x80150224`.
Those arguments are byte-masked values or a signed byte plus one. They need at
most four output bytes before NUL, within the codec's conservative eleven-byte
bound. Packing preserves the original format and alignment byte at `0x801D0C23`.

The active US keyboard page at `0x801DAB6C` has eight rows of twelve bytes.
Its 68 ordinary cells use `game-keyboard-character`, with capacity one and no
terminator or padding. Each cell requires exactly one letter, digit, space or
one of `! ? - , .` from the original page. Empty text, command markup and other
characters are refused. The 28 zero or control positions and the preceding
192 bytes of other keyboard pages stay unchanged.

The original selector at `0x801DAAA8` is 2, and both traced initialization paths
store 2. The drawing loop at `0x801D94D0` passes each ordinary cell to
`0x8014F800` with a one-character limit. The selector at `0x801DA114` skips
zero cells, handles control values separately and copies the chosen byte into
`0x801DB080` plus the name cursor at `0x801DA588`. Indexed references remain
empty; the payload pin protects the selector, navigation and draw instructions.
Controlled runtime observations verify edited English keyboard control labels and
one edited character cell. The visible START control accepts an empty name through
the End path. Other command paths, pages, formats and COMMU02 text remain unverified.

Four keyboard menu labels and a separator also use this layout:

| Address | Original text | Capacity including NUL | Codec |
| --- | --- | --- | --- |
| `0x801D0C2C` | `-` | 2 | `ascii-literal` |
| `0x801D0C3C` | `Next` | 5 | `game-literal` |
| `0x801D0C41` | `Back` | 5 | `game-literal` |
| `0x801D0C46` | `Quit` | 5 | `game-literal` |
| `0x801D0C4B` | `End` | 4 | `game-literal` |

Code at `0x801D9520` copies the four adjacent labels to a 20-byte stack region.
Branches select offsets 0, 5, 10 and 15; call `0x801D96CC` draws through
`0x8014F800` with a five-byte count and NUL termination. The separator reaches
ASCII renderer `0x80150098` at call `0x801D4E54`. It retains raw byte `0x2D`,
which differs from the game encoding's hyphen byte `0x3D`. Packing preserves
the copies' boundaries, the trailing alignment byte and the keyboard glyph table.

The `US COMMU00 default names` layout covers entry 0 of `BIN/ETC/COMMU00.EMI`
and entry 14 of `BIN/WORLD04/AREA175.EMI` through `AREA185.EMI`. Each payload
loads at `0x801EEC00`. Its sixty nine-byte records start at `0x801F2700`.
The first five bytes of each record use `game-fixed-literal`; the remaining
four attribute bytes stay unchanged. All five name bytes are available for
literal game characters. Shorter edits zero-fill unused bytes.

The consumers belong to `BIN/ETC/COMMU02.EMI` entry 8, also present as
`COMMU02B.EMI` entry 7, loaded at `0x801D0C00`. Call `0x801D6A1C` selects
a record, then the loop at `0x801D6A24..0x801D6A54` copies its five name bytes
into `0x801DB080`. Call `0x801D693C` uses that buffer, and `0x801D83D0`
passes it to renderer `0x8014F800` with a five-glyph limit. These are
cross-overlay consumers, not instructions inside the name payload. The name
addresses use indexed code addressing and retain empty pointer-reference lists.
Each archive needs its own document and output. Controlled availability verifies
one edited record through the empty-name fallback and its name-panel display.
Identical standalone and embedded payloads leave the supplying archive ambiguous.
Natural availability, other records and save-name propagation remain unverified.

The `US AREA008 decimal formats` layout covers entry 13 of the original US
`BIN/WORLD00/AREA008.EMI`, loaded at `0x801F2C00`. Three ASCII allocations use
formatter `0x8017E3F4`. Their capacities include NUL:

| Address | Original format | Capacity | Codec | Argument bounds | Maximum output bytes before NUL |
| --- | --- | --- | --- | --- | --- |
| `0x801F2C04` | `%02d %02d` | 10 | `ascii-signed-decimal-pair-format` | `[-128, 127]`, `[-426, 423]` | 9 |
| `0x801F2C10` | `%2d` | 4 | `ascii-u8-decimal-format` | `[0, 255]` | 3 |
| `0x801F2C14` | `%d` | 3 | `ascii-u7-decimal-format` | `[0, 127]` | 3 |

These codecs require the same number and order of decimal arguments. They accept
`%d`, an optional minimum width from 1 to 4, and optional zero padding before that
width. Printable ASCII literals are allowed within both the encoded allocation
and the original maximum output bound. Other percent sequences are refused.
Analysis reports `max_formatted_size` separately from encoded size. The three-byte
allocation fits only `%d` plus NUL, so its format cannot change.

Consumer `0x801F3C2C` supplies a signed byte and a second signed byte scaled by
100/30 with truncation toward zero. Consumer `0x801F3D18` supplies an unsigned
byte. Both format into `0x80145AD4` and call renderer `0x8014FF0C`.
Consumers `0x801F437C` and `0x801F43C8` mask an unsigned byte with `0x7F`,
format into `0x801490D8`, then call `0x80150224` with actions `0x54` and `0x56`.
Their final renderer remains unverified; that row records `renderers: []`.
An empty renderer array retains formatter-to-buffer evidence without claiming a
display consumer. Keep this array unchanged when editing. Code references are pinned
outside the allocations; no pointer-word reference is inferred from them.
Observed rendering remains unverified for all three formats.

The `US AREA030 panel text` layout covers entry 4 of the original US
`WORLD00/AREA030.EMI`, `WORLD02/AREA089.EMI` and `WORLD03/AREA129.EMI` archives
below `BIN`. Each payload loads at `0x801D0C00`. Each archive requires its own
source-bound document and edited output.

| Address | Original text | Capacity | Codec |
| --- | --- | --- | --- |
| `0x801D0C38` | `%3d` | 4 | `ascii-u16-decimal-format` |
| `0x801D0C8C` | `%4d` | 4 | `ascii-u16-decimal-format` |
| `0x801E28C4` | `Manillo` | 12 | `game-fixed-literal` |
| `0x801E28D0` | `????????????` | 12 | `game-fixed-literal` |

The numeric formats retain one decimal argument bounded by `[0, 65535]`, at
most five formatted bytes before NUL, and at most three encoded ASCII bytes.
They accept `%d` with an optional width from 1 to 4. Zero padding cannot fit
these allocations. Other conversions, missing arguments and extra literals
that exceed either bound are refused. Consumer `0x801E1004` uses the first
format with both a byte and a masked sixteen-bit result. Consumer `0x801E1320`
uses the second with a masked sixteen-bit result. Both call `0x8014F800`.
The first consumer preserves its three-character draw limit; the second uses
five. The formatter's output bound does not prove that every digit appears
within the first consumer's shorter draw limit.

The two labels use the game's literal encoding and all twelve field bytes.
Consumer `0x801E1004` copies twelve bytes, appends NUL in the destination buffer
and calls `0x8014F800` with a fifteen-character limit. Shorter edits zero-fill
the remaining field bytes. Other labels, dynamic record names and runtime
rendering remain unverified.

Recognition pins the complete selected payload outside these allocations.
Packing additionally binds the complete archive hash, canonical source path,
entry and entry offset. It preserves all other payloads. Each archive replica
remains a separate source. Other embedded strings, different code
layouts and observed rendering remain separate coverage requirements.

The `US battle messages` layout supports eight thirteen-byte literal records
in BATTLE entry 3, loaded at `0x801D0C00`. The table starts at `0x801EB000` with
stride thirteen. Supported table rows are 0, 2, 6, 7, 8, 9, 10 and 11; each
accepts up to twelve literal bytes followed by NUL. The copy consumer at
`0x801DE94C` uses that stride and copies at most twelve bytes through
`0x801501E4`, which appends NUL. Event enqueue at `0x801DE560` stores message
pointers; draw consumer `0x801E9538` forwards them to `0x8014F800`.

Row 1 at `0x801EB00D` uses `game-glyphs` and a six-byte allocation, including NUL.
It accepts at most five encoded bytes and preserves the remaining seven record
bytes. A nonzero latch selects this append at `0x801DEAA4`, after a five-byte
name copy into `0x801492B8`. The call dispatches BIOS A0 service `0x15`, whose
original implementation scans to destination NUL and copies source through NUL
without a capacity argument. Increasing the append beyond its original five
bytes is refused. A zero glyph operand is refused because it would terminate
that copy. Publisher `0x801DE60C` stores the resulting pointer for the draw
consumer. The original append is ` {glyph(0x1B)}{glyph(0x1C)}`.

These records use the same schema and source-binding rules. Their `references` are
empty because their consumers use indexed records or code references rather
than a pointer table. Recognition pins those instructions outside the editable
fields. Document `index` identifies the supported allocation, not its original
table row. Command-bearing row 3 and untraced rows 4 and 5 remain outside
this layout's editable allocations.

The same BATTLE entry exposes three ASCII allocations for its digit-sprite
renderers. Capacities include NUL and exclude alignment bytes:

| Address | Original text | Capacity | Codec | Renderer |
| --- | --- | --- | --- | --- |
| `0x801D0C70` | `:` | 2 | `ascii-digit-glyph-literal` | `0x801D94D4` |
| `0x801D0C74` | `%4d` | 4 | `ascii-u16-decimal-format` | `0x801D94D4` |
| `0x801D0C78` | `%3d` | 4 | `ascii-u16-decimal-format` | `0x801D9684` |

Calls `0x801D9524`, `0x801D9544` and `0x801D96CC` format into `0x80145AD4`
through `0x8017E3F4`. The first receives no arguments and is selected when the
low sixteen bits of the value equal `0xFFFF`. The other calls mask their argument
to sixteen bits. Each decimal format retains one ordered `%d` conversion, with
optional width 1..4, within three encoded and five formatted bytes.

Both renderers skip ASCII spaces and subtract `0x30` from every other byte to
select a sprite. The fallback therefore accepts exactly one space, digit or `:`.
The original `:` selects glyph index ten; its displayed shape is unverified.
Other ASCII and percent sequences are refused. The numeric formats cannot add
literals within their five-byte output bound. Packing preserves the two alignment
bytes at `0x801D0C72` and the adjacent data at `0x801D0C7C`. Runtime rendering
remains unverified.

The `US master labels and numeric formats` layout supports `Pwr`, `Def`, `Int`
and `Agl` in SISYOU entry 0, loaded at `0x801D0C00`. Their four-byte allocations start at
`0x801D4038`, `0x801D403C`, `0x801D4040` and `0x801D4044`. Each accepts up to
three literal bytes followed by NUL. Calls at `0x801D1500`, `0x801D1574`,
`0x801D15DC` and `0x801D1638` pass these strings directly to `0x8014F800`.
Their `references` are empty because the pinned code supplies the addresses.
It also supports `Pois` at `0x801D4048` and `Conf` at `0x801D4050`.
These five-byte allocations accept four literal bytes and NUL; the three
alignment bytes after each remain protected. The selected status pointer reaches
the call at `0x801D1920`, which uses renderer `0x8014FC90`. Each row records all its proven
renderers. The document's `consumers` lists the calls for the whole layout.
The same layout includes ASCII numeric formats `%3d`, `%3d/   ` and `   /%3d`
at `0x801D0C04`, `0x801D0C08` and `0x801D0C10`, with four-, eight- and
eight-byte allocations including NUL. Nine calls pass these formats and one
zero-extended numeric value to `sprintf` at `0x8017E3F4`; the resulting ASCII
buffer is drawn by `0x80150098`. Keep the `%3d` conversion and edit only the
surrounding printable ASCII within capacity. This preserves the number and type
of arguments and the original maximum formatted length. The conversion is part
of the format text, not a dialogue command. Split fields draw at matching
coordinates, so keep their separators in the same formatted character column.
Controlled runtime checks cover the four stat labels, the `%3d` stat path and both
split formats. Separate single-bit character-status fixtures cover `Pois` and
`Conf`. Natural illness acquisition, combined-status behavior and other
master-menu labels remain unverified.

SHOP also exposes these digit literals, dash fallbacks and numeric formats.
Capacities include NUL; formatted bounds exclude it. Digit literals accept
nonempty spaces, digits or `:` within their capacity. The one-character labels
at `0x801D0E58` and `0x801D0E5C` draw directly through `0x8014FF0C`; the
three-character separator at `0x801D0E54` uses `0x80150098`.

| Address | Original | Capacity | Codec | Formatted bound |
| --- | --- | --- | --- | --- |
| `0x801D0CFC` | `--------` | 9 | `ascii-dash-glyph-literal` | 8 |
| `0x801D0D0C` | `-------` | 8 | `ascii-dash-glyph-literal` | 7 |
| `0x801D0E54` | `  :` | 4 | `ascii-digit-glyph-literal` | 3 |
| `0x801D0E58` | `1` | 2 | `ascii-digit-glyph-literal` | 1 |
| `0x801D0E5C` | `2` | 2 | `ascii-digit-glyph-literal` | 1 |
| `0x801D0E60` | `%d` | 3 | `ascii-u8-decimal-format` | 3 |
| `0x801D0E64` | `%3d` | 4 | `ascii-u8-decimal-format` | 3 |
| `0x801D0E68` | `%3d/   ` | 8 | `ascii-u16-decimal-field-format` | 9 |
| `0x801D0E70` | `   /%3d` | 8 | `ascii-u16-decimal-field-format` | 9 |
| `0x801D0E7C` | `%3d/%3d` | 8 | `ascii-u8-decimal-pair-format` | 7 |

Dash fallbacks accept one or more ASCII spaces or `-` glyphs and refuse every
percent sequence. They pass through `sprintf` into the 64-byte scratch buffer at
`0x80145AD4`, then draw through `0x8014FF0C` for `0x801D0CFC` or `0x80150098`
for `0x801D0D0C`. Empty strings are refused because the renderers advance before
testing the next byte for NUL and could consume stale scratch bytes. Nonempty
all-space strings are safe. The three alignment bytes at
`0x801D0D05..0x801D0D08` remain protected.

The row counter at `0x801D0E60` receives values 1..255; `0x801D0E64` receives
an unsigned byte. The two 16-bit fields retain one decimal argument and permit
widths 1..4 with optional zero padding, within seven encoded bytes and nine
formatted bytes. Their `lhu` readers do not establish a smaller numeric domain.
The pair receives an unsigned byte and constant 128. Each format writes to the
64-byte scratch buffer at `0x80145AD4` and draws through `0x80150098`. Existing
16-bit decimal rows retain their five-byte output bound. Other bytes, including
`0x801D0E78..0x801D0E7C`, remain protected. Runtime rendering remains unverified.

Four quantity and price formats each retain one decimal argument. Capacities
include NUL; formatted bounds exclude it.

| Address | Original | Capacity | Codec | Formatted bound |
| --- | --- | --- | --- | --- |
| `0x801D0E84` | `%2d` | 4 | `ascii-u8-decimal-format` | 3 |
| `0x801D0E88` | `%6dZ` | 5 | `ascii-price-decimal-format` | 8 |
| `0x801D0E90` | `%7dZ` | 5 | `ascii-total-decimal-format` | 10 |
| `0x801D0E98` | `*%2d` | 5 | `ascii-u8-decimal-field-format` | 4 |

The first and last formats receive unsigned-byte quantities. The first permits
only `%d`, `%1d`, `%2d` or `%3d`: its three-byte output limit excludes added
literals, and its capacity excludes zero-padding tokens. It draws through digit
renderer `0x8014FF0C`. The other three draw through `0x80150098` and permit
printable ASCII literals around the conversion, within their output limits.
The byte field permits widths 1..4, prices 1..7 and totals 1..9, with optional
zero padding. Each of these three has four encoded bytes available.

The price helper returns at most 3,276,750; multiplying by an unsigned-byte
quantity bounds the total at 835,571,250. These bounds require seven and nine
digits respectively. All four formats use the 64-byte scratch buffer at
`0x80145AD4`. The largest output, including NUL, occupies eleven bytes.
The alignment gaps `0x801D0E8D..0x801D0E90`, `0x801D0E95..0x801D0E98` and
`0x801D0E9D..0x801D0EA0` remain protected. Runtime rendering remains unverified.

Four further SHOP formats use the existing byte-decimal codecs. Capacities
include NUL; formatted bounds exclude it.

| Address | Original | Capacity | Codec | Formatted bound |
| --- | --- | --- | --- | --- |
| `0x801D0DE0` | `%02d` | 5 | `ascii-u8-decimal-format` | 3 |
| `0x801D0E20` | `%3d/%3d` | 8 | `ascii-u8-decimal-pair-format` | 7 |
| `0x801D0E44` | `%2d` | 4 | `ascii-u8-decimal-format` | 3 |
| `0x801D0E48` | `%02d %02d` | 10 | `ascii-u8-decimal-pair-format` | 7 |

Each argument is an unsigned byte. The second argument at `0x801D0E20` is
32 or 128. Single formats retain one conversion and pair formats retain two,
with widths 1..4 and optional zero padding within the encoded and output limits.
The first format draws digits and spaces through `0x8014FF0C`; the others draw
through `0x80150098`. All use the 64-byte scratch buffer at `0x80145AD4`.
The alignment gaps `0x801D0DE5..0x801D0DEC`, `0x801D0E28..0x801D0E2C` and
`0x801D0E52..0x801D0E54` remain protected. Runtime rendering remains unverified.

Five earlier SHOP fields include unsigned 16-bit and signed-word paths.
Capacities include NUL; formatted bounds exclude it.

| Address | Original | Capacity | Codec | Formatted bound |
| --- | --- | --- | --- | --- |
| `0x801D0CE4` | `%3d` | 4 | `ascii-u16-decimal-format` | 5 |
| `0x801D0CE8` | `%3d/   ` | 8 | `ascii-u16-decimal-field-format` | 9 |
| `0x801D0CF0` | `   /%3d` | 8 | `ascii-u16-decimal-field-format` | 9 |
| `0x801D0CF8` | `%8d` | 4 | `ascii-i32-wide-decimal-format` | 11 |
| `0x801D0D08` | `%7d` | 4 | `ascii-i32-decimal-format` | 11 |

The unsigned fields cover 0..65535 and permit widths 1..4. The signed fields
have unmasked word arguments; retain the full signed 32-bit output bound,
including a possible minus sign. The wide codec permits widths 1..8; the existing
signed codec retains widths 1..7. Their capacities exclude zero-padding tokens,
and their eleven-byte output limit excludes added literals. All five retain
exactly one decimal argument.

All results use the 64-byte scratch buffer at `0x80145AD4`; the largest occupies
twelve bytes including NUL. `0x801D0CF8` draws through `0x8014FF0C`, while
`0x801D0D08` uses both `0x8014FF0C` and `0x80150098`. The unsigned fields use
`0x80150098`. The alignment bytes at `0x801D0D05..0x801D0D08` remain
protected. The split 16-bit formats draw at matching coordinates. Edit paired
formats together and keep their separators in the same formatted character
column. Controlled party-change checks cover the edited split HP/AP fields.
The pairs at `0x801D0CE8`/`0x801D0CF0` and `0x801D0E68`/`0x801D0E70`
contain identical strings, so their active supplier remains indistinguishable.
Other numeric paths remain unverified.

## Bank layout

```
u16 offsets[pointer_bytes / 2]    ; offsets[0] is pointer_bytes
strings[]                         ; begins at byte pointer_bytes
```

- The first `u16` is both the first row's offset and the byte size of the entire
  offset array. There is no separate size field before the array.
- `offsets[0] == pointer_bytes`. Pointers are independent and need not increase;
  equal pointers are the same row, and a pointer equal to the bank size is empty.
- A row's bytes are its **extent**: `[offsets[i], next larger pointer)` — or the end
  of the bank for the largest pointer. Extents tile the string region, so nothing is
  orphaned.
- A subfile whose first word is `0x00000008` declares **two banks**: an 8-byte header
  (`u32 8`, `u32 block1_start`), then bank 0, then bank 1 to the end. 44 US
  battle/system text subfiles (`0x8001A000`, sizes 14428/16872) use it and round-trip
  byte-identically; packing preserves the header.

## Commands

| Byte | Token | Operands |
| --- | --- | --- |
| `0x00` | `{end}` | 0 |
| `0x01` | `{nl}` | 0 |
| `0x02` | `{clear}` | 0 |
| `0x03` | `{player}` | 0 |
| `0x04` | `{name(0xNN)}` | 1 |
| `0x05` | `{color(0xNN)}` | 1 |
| `0x06` | `{/color}` | 0 |
| `0x07` | `{item(0xNN)}` | 1 |
| `0x08` | `{byte(0x08)}` (zero US occurrences) | — |
| `0x09` | `{byte(0x09)}` (zero US occurrences) | — |
| `0x0A` | `{sound(0xNN)}` | 1 |
| `0x0B` | `{pause}` | 0 |
| `0x0C` | `{pos(0xNN)}` | 1 |
| `0x0D` | `{anim}` | 0 |
| `0x0E` | `{effect(0xNN)}` | 1, or 2 when the operand is `0x0F` |
| `0x0F` | `{rumble}` | 2 |
| `0x10` | `{fast}` | 0 |
| `0x11` | `{/fast}` | 0 |
| `0x14` | `{choice(0xNN,0xNN,0xNN)}` | 3 |
| `0x16` | `{time(0xNN)}` | 1 |
| `0x20` | `{byte(0x20)}` (zero US occurrences) | — |
| anything else | `{byte(0xNN)}` | — |

Operands are one byte each, written `0xNN`. A command whose operands are cut off by
the extent is emitted as `{byte(0xNN)}`, never as a truncated command. `{byte(0xNN)}` is
accepted for any value, so such text repacks unchanged; no mapped byte in the US corpus
is emitted this way (all 6,516 `{byte}` tokens are unmapped bytes).

## Option lists

Choice options are not in the pointer table. They live in the same row extent, after
the row's `{end}`:

```
This is B3  {nl}Do you want to ride{nl}the lift?  {choice(0x00, 0x0c, 0x13)}Go to B1{end}Go to B2{end}Quit{end}
```

`index` reports how many rows carry such a list (`OPTIONS`), `query` flags them
(`+opt`), and editing them is ordinary text editing — they count against the same
extent, so a longer option list can overrun the row's capacity and is then refused.

## Character table

Literal characters: `0x41`-`0x5A` = `A`-`Z`, `0x61`-`0x7A` = `a`-`z`,
`0x30`-`0x39` = `0`-`9`, `0xFF` = space.

Punctuation: `0x3A`=`(`, `0x3B`=`)`, `0x3C`=`,`, `0x3D`=`-`, `0x3E`=`.`,
`0x3F`=`/`, `0x40`=`=`, `0x5C`=`?`, `0x5D`=`!`, `0x8E`=`'`, `0x8F`=`:`,
`0x90`=`"`, `0x91`=`;`, `0x93`=`%`.

Symbols: `0x5B`=`…`, `0x5E`=`♥`, `0x5F`=`♪`, `0x60`=`Ƶ`, `0x7B`-`0x89` arrows and
shapes, `0x8A`=`©`, `0x8D`=`&`, `0x92`=`•`.

Any byte not in these tables is preserved as `{byte(0xNN)}`; nothing is mapped to a
space or dropped.

This is the US (`SLUS_004.22`) table. EU and JP encodings and consumers remain
unverified. External accented-Latin tables are investigation leads; the US codec
does not establish their regional mapping. A kanji or two-byte table requires
evidence from the Japanese build's own bytes, font table and consumers.

## JSON codec

```json
{
  "schema": "bof3.dialogue-text/v3",
  "version": 3,
  "source": { "archive": "…/AREA000.EMI", "entry": 11, "address": 2147549184, "class": "dialogue" },
  "fingerprint": "…",
  "source_sha256": "…",
  "banks": [[{ "number": 36, "text": " {nl}Do you want to save?{choice(0x00, 0x0c, 0x02)}Yes{end}No{end}" }]]
}
```

The `source` object identifies the archive, original TOC entry, load address and
class: `dialogue`, `battle`, or `heuristic` for a raw-text instance.
Escaping is JSON's own, so literals need no
bespoke rules; the game's control codes stay as `{token}` inside the string.
`--entry` accepts one index, a comma list, or `all`; extraction to a directory writes
`<entry>.json` per subfile.

A raw-text instance additionally carries its extent, and `pack` will only write the
same number of bytes back:

```json
"latch": { "offset": 38407, "length": 656 }
```


## Fingerprint

`fingerprint` is a required 64-bit FNV-style fingerprint over the original row
bytes. `source_sha256` is the required SHA-256 of the entire original archive.
Packing checks both identities and the source path, entry, load address and class
before writing. Version 3 documents are required; re-extract older documents
before editing. The tool does not accept an earlier document schema.

## Retained corpus index

`bof3.text-index/v2`, version 2, stores bank and heuristic readings in `instances`
and verified literal documents in `literals`. Each literal document has a relative
`archive` path and `contents` with the original source schema, source hash, codec,
consumer metadata and allocations. The source path inside `contents` is relative
to the index root. Search uses the literal codec's matcher, including transparent
glyph commands, and never combines adjacent literal allocations.

`inputs.archives` counts EMI files; `inputs.executables` counts PS-X executables.
The content digest and manifest include both. Unknown executable layouts add no
verified rows. `counts` records bank and heuristic classes and literal source
schemas separately. Search's `indexed` count includes both collections' rows.
Version 1 artifacts must be rebuilt; they do not cover verified literal fields.

Corpus hits from literal documents retain `source_schema`, source hash, codec,
references and consumer metadata. They have `class: null`; executable hits omit
`entry`. Their file offsets and capacities use the same allocation anchors as
direct `query`. A class filter selects bank or heuristic results only. Index
integrity checks reproduce each selected literal document against source bytes;
they do not establish full source coverage or runtime rendering.

### Original-US mode-3 VRAM tiles

The payload filter recognizes raw VRAM upload words only when the entire EMI
matches a size and SHA256 in `config/original-us-disc.json`. Renaming an identical
archive preserves qualification. Modified archives and other regions remain
unresolved by this rule. The filter checks the current archive on every scan;
these exclusions are not stored as unconditional registry windows.

In the US executable, `recordEmiPackedDispatch` at `0x80162698` sends mode-3
sectors to ring state 3. The consumer at `0x80163378` passes each complete
`0x800`-byte sector to `LoadImage` as a 32-by-32-word rectangle. The upload
routine at `0x8017CE9C` emits the GPU upload command and transfers the words.
These words are nontext data; this rule does not determine pixel depth, CLUT
selection, or whether the game displays them.

The TOC word at `+4` packs a tile column in bits 24–29, row in bits 16–20,
and row width in bits 8–13. The rule requires a positive width and payload,
whole sectors, a sector count divisible by the row width, and a complete
rectangle within 32 columns and 16 rows of tiles. The payload's first word
must equal the TOC check word at `+8`, and the transfer mode at `+12` must be 3.
Partial sectors, wrapped coordinates, and incomplete rows remain unresolved.
