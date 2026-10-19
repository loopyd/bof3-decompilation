---
type: Format
title: Dialogue text block
description: BOF3 area dialogue subfile layout, pointer banks, command bytecodes, option lists and the editable JSON codec.
tags: [dialogue, text, format]
---

# Dialogue text block

Area dialogue lives in an EMI subfile. This specification describes the supported
US bank layout and codec. The original US file identities are pinned in
[`config/original-us-disc.json`](../../../config/original-us-disc.json).
The [command registry](../../../tools/rust/bof3-text/src/models.rs) defines the
supported token names and operand widths.

External investigation leads include `brisma/bof3-translation-tool`,
`glitch-in-the-herring/bof3-text-extractor` and the BOF3JS
[EU knowledgebase](../bof3-eu/05-features-meshes.md). They do not establish support
for another region. Original source bytes and their consumers determine format
acceptance; codec support alone does not establish in-game behavior.

Use `bin/harness text --help` to discover commands. The
[tool documentation](../../../tools/rust/bof3-text/docs/README.md) describes
extraction, analysis, search, packing and coverage reporting.

## Locating the subfile

The dialogue subfile is the EMI entry whose TOC `load_argument` is `0x80010000`.
The [EMI container](emi.md) rules apply: `0x10`-byte header, `16`-byte TOC
entries from file offset `0x10`, payloads from `0x800`, each payload aligned by
`next = current + ((size + 0x7FF) & ~0x7FF)`.

`WORLD00/AREA000.EMI` entry `11` sits at payload offset `0xB8000`, size `9170`.
Each of the 200 US area archives carries exactly one dialogue subfile.

## Bank layout

```
u16 offsets[pointer_bytes / 2]    ; offsets[0] is pointer_bytes
strings[]                         ; begins at byte pointer_bytes
```

- The first `u16` is both the first row's offset and the byte size of the entire
  offset array. There is no separate size field before the array.
- `pointer_bytes` is even, `>= 4`, and `offsets[0] == pointer_bytes`, so the first
  row begins right after the table.
- Pointers are **independent**: they need not increase, and a later slot may point
  back at an earlier row. Equal pointers are the same row; a pointer equal to the
  bank size is an empty slot.
- A row's bytes are its **extent**: `[offsets[i], next larger pointer in the
  table)` or the end of the bank for the largest pointer. Extents tile the string
  region, so every byte after the table belongs to exactly one row (modulo shared
  pointers); nothing is orphaned or unreferenced.
- A valid section whose first word is `0x00000008` declares **two banks**: an 8-byte header
  (`u32 8`, `u32 block1_start`) precedes the first bank, and the second runs from that
  word to the end. US battle/system text uses load argument `0x8001A000`;
  packing preserves the header verbatim. The first word alone is insufficient
  to identify text: bank boundaries and pointer tables must also validate.

## Command bytecodes

Operand counts are the bytes following the control byte.

| Byte | Token | Operands | Meaning |
| --- | --- | --- | --- |
| `0x00` | `{end}` | 0 | string terminator |
| `0x01` | `{nl}` | 0 | line break |
| `0x02` | `{clear}` | 0 | clear the box |
| `0x03` | `{player}` | 0 | current player name |
| `0x04` | `{name(0xNN)}` | 1 | speaker name (`0`-`6` = Ryu, Nina, Garr, Teepo, Rei, Momo, Peco) |
| `0x05` | `{color(0xNN)}` | 1 | text colour (`1`-`7`) |
| `0x06` | `{/color}` | 0 | colour reset |
| `0x07` | `{item(0xNN)}` | 1 | item name placeholder |
| `0x08` | `{byte(0x08)}` | — | opaque: control byte in other builds, **zero US occurrences** |
| `0x09` | `{byte(0x09)}` | — | opaque: **zero US occurrences** |
| `0x0A` | `{sound(0xNN)}` | 1 | sound cue |
| `0x0B` | `{pause}` | 0 | pause |
| `0x0C` | `{pos(0xNN)}` | 1 | box position/style |
| `0x0D` | `{anim}` | 0 | text animation |
| `0x0E` | `{effect(0xNN)}` | 1, **or 2** when the operand is `0x0F` | screen effect |
| `0x0F` | `{rumble}` | 2 | rumble |
| `0x10` | `{fast}` | 0 | speed up text |
| `0x11` | `{/fast}` | 0 | end fast text |
| `0x14` | `{choice(0xNN,0xNN,0xNN)}` | 3 | choice block: option count/kind and separators |
| `0x16` | `{time(0xNN)}` | 1 | hold/delay |
| `0x20` | `{byte(0x20)}` | — | opaque: **zero US occurrences** |

Any byte with no literal or command mapping is preserved as `{byte(0xNN)}`.

Row boundaries and command widths affect decoding:

- `0x16` (`{time}`) can end a row without NUL. Seven areas (`AREA009`, `AREA020`,
  `AREA066`, `AREA134`, `AREA148`, `AREA149`, `AREA172`) contain **no `0x00` at
  all** and end their rows with `{time(0xNN)}` — a NUL-only reader cannot open them.
- A command whose operands are cut off by the extent is emitted as `{byte(0xNN)}`.
  `{byte(0xNN)}` accepts any byte value, preserving incomplete commands unchanged.

### Option lists

The choice options are **not** referenced by the pointer table. They are stored in
the same row extent, after the row's `{end}`:

```
This is B3  {nl}Do you want to ride{nl}the lift?  {choice(0x00, 0x0c, 0x13)}Go to B1{end}Go to B2{end}Quit{end}
```

The bytes after a row's first `NUL` can therefore belong to its option list.
They remain part of that row's allocation.

## Character table

Literal characters: `0x41`-`0x5A` = `A`-`Z`, `0x61`-`0x7A` = `a`-`z`,
`0x30`-`0x39` = `0`-`9`, `0xFF` = space.

Punctuation: `0x3A`=`(`, `0x3B`=`)`, `0x3C`=`,`, `0x3D`=`-`, `0x3E`=`.`,
`0x3F`=`/`, `0x40`=`=`, `0x5C`=`?`, `0x5D`=`!`, `0x8E`=`'`, `0x8F`=`:`,
`0x90`=`"`, `0x91`=`;`, `0x93`=`%`.

Symbols: `0x5B`=`…`, `0x5E`=`♥`, `0x5F`=`♪`, `0x60`=`Ƶ`, `0x7B`-`0x89` arrows and
shapes (`↑↓←→〜○△×□★►↖↘↗↙`), `0x8A`=`©`, `0x8D`=`&`, `0x92`=`•`.

Every other byte is preserved verbatim; mapping unknown bytes to `' '` is rejected
as lossy.

### Languages and script coverage

The table above is the US (`SLUS_004.22`) table. EU and JP encodings and consumers
remain unverified. External accented-Latin tables are investigation leads;
the US codec does not establish their regional mapping. A kanji or two-byte table
requires evidence from the Japanese build's own bytes, font table and consumers.

## Editable codec

The document is JSON only (`bof3.dialogue-text/v3`). It carries the **source** it
came from, a fingerprint, and one row array per bank:

```json
{
  "schema": "bof3.dialogue-text/v3",
  "version": 3,
  "source": {
    "archive": "out/extracted/BIN/BATTLE/BATTLE.EMI",
    "entry": 11,
    "address": 2147590144,
    "class": "battle"
  },
  "fingerprint": "…",
  "source_sha256": "…",
  "banks": [[{ "number": 1, "text": "You scattered the enemy!{end}" }]]
}
```

The `source` object identifies the archive, original TOC entry, load address and
class. **Escaping is JSON's own** (`\\`, `\"`, `\n`,
`\uXXXX`), so any literal is representable and no bespoke grammar is needed; the game's
control codes keep their `{token}` spelling inside string values. A `{`/`}`/`\` that is
not a token is rejected, and a byte with no modelled token fails closed as
`{byte(0xNN)}`.

Both classes are supported: `dialogue` (`0x80010000`, 200 US subfiles) and `battle`
(`0x8001A000`, 44 US two-bank subfiles). `--entry` selects one index, a comma list,
or `all`, and `extract`/`pack` accept the same selection.

## Text classes and raw-text instances

Every emitted source carries a `text_class`. `dialogue` and `battle` are located
structurally, by load argument. A third class, `heuristic`, marks a raw-text
instance found by scanning rather than by structure. `--class
dialogue|battle|heuristic` filters a selection; an absent class is refused with the
classes that are present.

Banked rows are bounded by their slot table. A raw-text instance has no slot table,
so its extent is carried explicitly:

```json
"latch": { "offset": 38407, "length": 656 }
```

`extract --latch ENTRY:OFFSET+LENGTH` writes one, `query --class heuristic` searches
the decoded instances, and `pack` patches in place. **A raw-text edit that changes the
byte length is refused**, because no allocation is proven for raw text; relocation and
expansion are not inferred. `latch` is absent for the banked classes.

**A latch is a lead, not verified text, and it is never drawn from a classified window.** A
range an existing owner already accounts for — a known-class subfile's whole payload, its pointer
tables and row extents, or a reviewed `textbin` range — is a barrier the scanner never latches
inside. Generate the registry with `bin/harness text windows`; every latching mode
reports its applied barriers and removed or split latches.

**Identified non-text payloads are not searched, and a lead has to carry a word.** The tool's filter
layer (`filters.rs`, contracts in `models.rs`) refuses byte ranges that a parse or a record already
accounts for, and refuses whole payloads whose family an in-repo record identifies and whose structural
check passes (`pBAV` audio, `pQES` music); nothing is excluded on a signature alone, and families no
record identifies stay searchable. Filtering also has a reporting side: a candidate must contain a word
of at least three letters after its command tokens are stripped, so token soup is not reported. Both the
pre-filter and the post-filter are the same code for every mode.

**A hit is anchored on its containing string, not on its match.** `scan` reports each candidate's offset,
length, decode ratio, letter count and distinct-byte count, and labels the class
`heuristic`. `dialogue` and `battle` are the supported bank classes; the tool also
supports verified literal layouts described in its
[format reference](../../../tools/rust/bof3-text/docs/formats.md).
Use `probe --round-trip` to test bank structure. Neither a successful structural
probe nor a readable latch establishes a new consumer-verified class. A failed
probe does not rule out another text layout. Discovery must retain unresolved
sources until their storage and consumers are established.


## Allocation and packing

- A subfile is a **fixed allocation**; the archive is sector-aligned, so the
  payload size never changes and no relocation is needed.
- Packing re-emits the offset tables verbatim and rewrites only row extents, so an
  unedited round trip is byte-identical.
- An edited row must fit its own extent — including any option list it carries;
  overflow is refused. A row with no allocated extent cannot receive text, and
  slots sharing one extent must be edited identically.
- Outputs are **create-new**: an existing destination is refused, and the source
  archive cannot be reached through an alternate spelling, symlink or hard link.

## Coverage

`bin/harness text verify` checks unchanged round trips for recognized dialogue
and battle banks. `extract-corpus` also exports supported executable, inline and
record layouts, checking unchanged reproduction before publishing each document.
Use `coverage --strict` on the complete extracted tree for file membership,
source ranges and unresolved consumers. Bank verification and search-index
integrity do not establish full US text coverage.

### Uncertainty

- The precise meaning of the three `{choice}` operands (beyond "option count/kind
  and separators") and of individual operand values such as `{time}` and
  `{effect}` remain unreconstructed; the operands are round-tripped, not interpreted.
- Unchanged round trips prove byte preservation for supported inputs. Edited
  rendering, choices and dynamic substitutions require separate bounded runtime
  observations, reconciled with the original source and consumer.

## Evidence

The maintained [command reference](../../../tools/rust/bof3-text/docs/commands.md)
describes `analyze`, `probe`, `verify`, `extract-corpus` and `coverage`.
`analyze` exposes original allocation bytes, commands, operand offsets and shared
references. Use these source locations to reproduce byte checks and qualify
consumer observations. Keep run-specific reports outside the format contract;
missing source or runtime evidence remains an acceptance gap.
