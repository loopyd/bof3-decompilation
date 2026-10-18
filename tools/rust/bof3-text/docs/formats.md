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
Each US archive contains exactly one; `index` reports its payload offset and whether
it parses.

## Executable ASCII

`extract`, `analyze`, `validate` and `pack` also support the US executable's
exception-screen ASCII strings and the `FILE NOT FOUND` diagnostic with its
three-ID hexadecimal format. `extract-corpus` exports
them when the complete source root includes `SLUS_004.22`. Literal strings use
the pointer words at `0x8017F470..0x8017F53C`, with an exclusive upper bound.
The consumer at `0x8014B3C4` passes these pointers to renderer `0x80150098`.
The renderer reads bytes until NUL, advances on ASCII space and handles newline
`0x0A`. These bytes use ASCII rather than the dialogue character table.

The JSON schema is `bof3.executable-text/v3`. Re-extract older documents before
editing. It contains `source`, mandatory `source_sha256`, `build`, `load_address`,
`consumers`, `renderer`, `formatter` and `strings`.
Each string retains its `address`, whole-file `file_offset`, original `capacity`
including NUL, immutable `consumer` and `codec`, typed `references` and editable `text`.
Exception literal rows use `ascii-literal` and `pointer-word` references with an `address`.
Shared pointers appear on one allocation. Retain every allocation and edit only
`text`.

For literal rows, packing accepts printable ASCII and newline, reserves one byte
for NUL and refuses growth beyond the original string, excluding alignment
padding. A shortened string is NUL-padded inside its allocation. Unchanged strings
preserve every byte. Reference edits, relocation and unproven exception-table
indices are refused.

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

Extract the selected entry as `bof3.inline-text/v4`. Its eighteen strings include
the ten counted labels at `0x801D0C7C`, with stride 20, followed by these eight
names at indices 10 through 17. The label consumer calls `0x8014FC90` at
`0x801DCF50`, using the same count convention as the BATE, SHISU and SHOP tables.
Names use `game-fixed-literal`, have capacity five and may occupy all five bytes.
Shorter names are NUL-padded. Retain all eighteen allocations and edit only
`text`. Re-extract former character-only documents before editing or packing.

Recognition pins the selected payload outside its label and name allocations.
Packing binds the complete archive SHA-256 and canonical path and checks every
allocation. Stats, the other twelve bytes in each label record and all other
archive bytes remain unchanged. The copies remain separate sources. Changing
START does not update STATUS or an existing save's live names. Character renaming,
other initializers and observed
rendering remain separate requirements.

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
pointer tables and code remain outside the editable allocations. Other BATE
strings and observed rendering remain unverified.

US `SHISU.EMI` entry 0 loads at `0x801D0C00`. Its category tables at
`0x801D9E70`, `0x801D9EA4` and `0x801D9EDC` contain five, four and five labels.
Calls at `0x801D7F2C`, `0x801D85A8` and `0x801D8E34` pass selected strings to
`0x8014F800`. US `SHOP.EMI` entry 0 uses the same load address, with tables at
`0x801E5B48`, `0x801E5B7C` and `0x801E5BB4` and calls at `0x801DCA2C`,
`0x801DD0A8` and `0x801DD934`. Each archive has its own source-bound layout.
These `game-literal` fields also retain their original string length plus NUL,
with capacities of five to seven bytes. Slot padding, pointers and code remain
protected.

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

`bof3.inline-text/v4` retains `source`, `source_sha256`, `layout`, `entry`,
`entry_offset`, `load_address`, `consumers` and `strings`. Each string
has `index`, `address`, whole-file `file_offset`, `payload_offset`, `capacity`,
`renderer`, `codec`, pointer-word `references` and editable `text`. Retain all
fields and edit only `text`. Allocations exclude alignment padding. Terminated
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

The `US master labels and numeric formats` layout supports `Pwr`, `Def`, `Int`
and `Agl` in SISYOU entry 0, loaded at `0x801D0C00`. Their four-byte allocations start at
`0x801D4038`, `0x801D403C`, `0x801D4040` and `0x801D4044`. Each accepts up to
three literal bytes followed by NUL. Calls at `0x801D1500`, `0x801D1574`,
`0x801D15DC` and `0x801D1638` pass these strings directly to `0x8014F800`.
Their `references` are empty because the pinned code supplies the addresses.
It also supports `Pois` at `0x801D4048` and `Conf` at `0x801D4050`.
These five-byte allocations accept four literal bytes and NUL; the three
alignment bytes after each remain protected. The selected status pointer reaches
the call at `0x801D1920`, which uses renderer `0x8014FC90`. Each row records its
own renderer. The document's `consumers` lists the calls for the whole layout.
The same layout includes ASCII numeric formats `%3d`, `%3d/   ` and `   /%3d`
at `0x801D0C04`, `0x801D0C08` and `0x801D0C10`, with four-, eight- and
eight-byte allocations including NUL. Nine calls pass these formats and one
zero-extended numeric value to `sprintf` at `0x8017E3F4`; the resulting ASCII
buffer is drawn by `0x80150098`. Keep the `%3d` conversion and edit only the
surrounding printable ASCII within capacity. This preserves the number and type
of arguments and the original maximum formatted length. The conversion is part
of the format text, not a dialogue command.
Other master-menu labels and observed rendering remain unverified.

## Bank layout

```
u16 pointer_bytes                 ; byte size of the offset array, including itself
u16 offsets[pointer_bytes / 2]    ; bank-relative pointers
strings[]                         ; begins at byte pointer_bytes
```

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

This is the US (`SLUS_004.22`) table. For other regional builds the evidenced
extension is a single-byte **accented-Latin** range around `0x97`-`0xAA` (supplied to
the reference tool as `--extra-table 9A=à 9B=ò …`), which is the multilingual (EU)
surface and is still Latin script. **No kanji or two-byte table is evidenced**: the
pinned reference decoder has no double-byte path and the JP build is unexamined, so
treat "the game also supports a kanji table" as unproven
(`out/text-format/decoder-provenance.md`).

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

`source` is `PATH/ARCHIVE.EMI[#ENTRY]@0xADDRESS`, with an explicit `entry` even for a single text subfile, and carries the `class` (`dialogue`, `battle`, or
`heuristic` for a raw-text instance). Escaping is JSON's own, so literals need no
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
