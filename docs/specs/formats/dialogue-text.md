---
type: Format
title: Dialogue text block
description: BOF3 area dialogue subfile layout, pointer banks, command bytecodes, option lists and the editable JSON codec.
tags: [dialogue, text, format]
---

# Dialogue text block

Area dialogue lives in an EMI subfile, not in a standalone file and not in a
target-local `.text` blob. The layout, character table and command bytecodes were
established from three independent sources that agree:

1. **Original US bytes** (`SLUS-00422`, `BOF3_1.1`, md5
   `9dd9a7c934b8b59d0ce76b0f25d18176`) — every claim below is reproducible from the disc.
2. **The US renderer** in `SLUS_004.22`, which reads the pointer table and
   dispatches on the control bytes
   ([evidence](../../../out/text-format/consumer-verification.md)).
3. **The reference translation tool** (`brisma/bof3-translation-tool`,
   `bof3tool.py` `decode_text`/`encode_text`), whose decode/encode pair names the
   codes and fixes their operand widths, and the older
   `glitch-in-the-herring/bof3-text-extractor` plus the BOF3JS EU knowledgebase
   ([chapter 5](../bof3-eu/05-features-meshes.md), `SLES_013.04`, leads only).

Tooling: `bin/harness text index|scan|extract|validate|query|pack|probe|map|build-index|search|verify`
(implementation `tools/rust/bof3-text`, owner docs
`tools/rust/bof3-text/docs/README.md`).

## Locating the subfile

The dialogue subfile is the EMI entry whose TOC `load_argument` is `0x80010000`.
The [EMI container](emi.md) rules apply: `0x10`-byte header, `16`-byte TOC
entries from file offset `0x10`, payloads from `0x800`, each payload aligned by
`next = current + ((size + 0x7FF) & ~0x7FF)`.

`WORLD00/AREA000.EMI` entry `11` sits at payload offset `0xB8000`, size `9170`.
Each of the 200 US area archives carries exactly one dialogue subfile.

## Bank layout

```
u16 pointer_bytes                 ; byte size of the offset array, including itself
u16 offsets[pointer_bytes / 2]    ; bank-relative pointers
strings[]                         ; begins at byte pointer_bytes
```

- `pointer_bytes` is even, `>= 4`, and `offsets[0] == pointer_bytes`, so the first
  row begins right after the table.
- Pointers are **independent**: they need not increase, and a later slot may point
  back at an earlier row. Equal pointers are the same row; a pointer equal to the
  bank size is an empty slot.
- A row's bytes are its **extent**: `[offsets[i], next larger pointer in the
  table)` or the end of the bank for the largest pointer. Extents tile the string
  region, so every byte after the table belongs to exactly one row (modulo shared
  pointers); nothing is orphaned or unreferenced.
- A subfile whose first word is `0x00000008` declares **two banks**: an 8-byte header
  (`u32 8`, `u32 block1_start`) precedes the first bank, and the second runs from that
  word to the end. This is **live US data**: 44 battle/system text subfiles (load
  argument `0x8001A000`, sizes `14428` and `16872`) use it, and all 44 round-trip
  byte-identically; packing preserves the header verbatim. Seven further
  68-byte subfiles of a different class (`0x800E3800`) begin with the same four bytes by
  coincidence and are refused by the header validation, not silently handled.

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

Notes established from the corpus and the two consumers:

- `0x16` (`{time}`) **only ever appears as the last command of a row** (0 of
  19,341 extents place content after it). Seven areas (`AREA009`, `AREA020`,
  `AREA066`, `AREA134`, `AREA148`, `AREA149`, `AREA172`) contain **no `0x00` at
  all** and end their rows with `{time(0xNN)}` — a NUL-only reader cannot open them.
- A command whose operands are cut off by the extent is emitted as `{byte(0xNN)}`.
  `{byte(0xNN)}` is accepted for any value, so such text repacks unchanged; measured
  across the corpus, 0 of the 6,516 `{byte}` tokens stand for a byte that has a token.

### Option lists

The choice options are **not** referenced by the pointer table. They are stored in
the same row extent, after the row's `{end}`:

```
This is B3  {nl}Do you want to ride{nl}the lift?  {choice(0x00, 0x0c, 0x13)}Go to B1{end}Go to B2{end}Quit{end}
```

2,034 US rows carry such a list. This is why the bytes after a row's first `NUL`
have no pointer of their own: they belong to the row that asked the question.

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

The table above is the US (`SLUS_004.22`) table. For other regional builds the
evidenced surface is a **single-byte accented-Latin range around `0x97`-`0xAA`**,
which the reference tool supplies out-of-band (`--extra-table 9A=à 9B=ò …`, PAL
example `À=97 Ò=98 Ù=99 ì=a5 è=a0 é=a1 É=a4 ù=a8 °=aa`). That is the multilingual
(EU) extension, and it is still Latin script — not a new script.

**No kanji or two-byte table is evidenced.** The pinned reference decoder has no
double-byte path and no CJK codepoints; the JP build (`SLPS-00990`) is unexamined.
Treat "the game also supports a kanji table" as **unproven** until the Japanese
build's own bytes and font table establish it. Regional limits are recorded in
`out/text-format/decoder-provenance.md`.

## Editable codec

The document is JSON only (`bof3.dialogue-text/v2`). It carries the **source** it
came from, a fingerprint, and one row array per bank:

```json
{
  "schema": "bof3.dialogue-text/v2",
  "version": 2,
  "source": {
    "archive": "out/extracted/BIN/BATTLE/BATTLE.EMI",
    "entry": 11,
    "address": 2147590144,
    "class": "battle"
  },
  "fingerprint": "…",
  "banks": [[{ "number": 1, "text": "You scattered the enemy!{end}" }]]
}
```

`source` spells `PATH/ARCHIVE.EMI[#ENTRY]@0xADDRESS`; `entry` is omitted when the
archive holds a single text subfile. **Escaping is JSON's own** (`\\`, `\"`, `\n`,
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
inside, so the 311 latches that used to sit inside verified text banks are now zero. The registry of
those windows is generated by `bin/harness text windows` and every latching mode reports what the
check removed.

**Identified non-text payloads are not searched, and a lead has to carry a word.** The tool's filter
layer (`filters.rs`, contracts in `models.rs`) refuses byte ranges that a parse or a record already
accounts for, and refuses whole payloads whose family an in-repo record identifies and whose structural
check passes (`pBAV` audio, `pQES` music); nothing is excluded on a signature alone, and families no
record identifies stay searchable. Filtering also has a reporting side: a candidate must contain a word
of at least three letters after its command tokens are stripped, so token soup is not reported. Both the
pre-filter and the post-filter are the same code for every mode.

**A hit is anchored on its containing string, not on its match.** `scan` reports each candidate's offset,
length, decode ratio, letter count and distinct-byte count, and labels the class
`heuristic`; `dialogue` and `battle` remain the only classes with evidence. Latch counts
outside those two banks are a list to investigate, not discovered classes (see
`out/text-format/lexagraph-scan.md`). Structural probing then **refuted every candidate**:
`probe --round-trip` parses 0 of 6,100 subfiles outside the verified classes as a text-bank
section, while its positive control parses and reproduces 200/200 `dialogue` and 44/44 `battle`
subfiles — and the candidates are documented elsewhere as VRAM pages, palette, encounter and
audio containers (see `out/text-format/class-promotion.md`). No further class is promotable on
present evidence.


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

All **200** US dialogue subfiles parse, extract and repack byte-identically when
unedited — as do all 44 two-bank battle/system text subfiles (`0x8001A000`), so the tool covers **244** text subfiles in two classes. Earlier 190/10 and 174/26 splits
came from three wrong assumptions, all corrected here: that `0x16` was inert, that
offsets had to increase, and that bytes after the first `NUL` were unreferenced.

### Uncertainty

- The precise meaning of the three `{choice}` operands (beyond "option count/kind
  and separators") and of individual operand values such as `{time}` and
  `{effect}` remain unreconstructed; the operands are round-tripped, not interpreted.
- The two-bank layout is implemented from the reference tool and **is** exercised
  by 44 US battle/system subfiles, all of which round-trip byte-identically.
- No whole-image or runtime oracle exists here: this is a byte-level format, not
  proof of in-game rendering.

## Evidence

- External decoder provenance, pinned width table, language and region limits:
  `out/text-format/decoder-provenance.md` (retained sources under
  `out/text-format/provenance/`).
- Heuristic scan of all 880 US archives, with false-positive and false-negative
  behaviour stated: `out/text-format/lexagraph-scan.md`.
- Raw-text instance round trip, controlled edit and refusal evidence:
  `out/text-format/raw-text-instances.md`.
- Guide/provenance record and corrections: `out/text-format/evidence-scope.md`.
- US consumer verification: `out/text-format/consumer-verification.md`, raw excerpt
  `out/text-format/us-text-dispatch.md`.
- Independent per-command derivation: `out/text-format/command-derivation.md`.
- Round-trip, safety and rejection evidence:
  `out/text-format/roundtrip-validation.md`.
