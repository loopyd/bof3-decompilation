# Formats

The tool reads the text subfiles of a BOF3 EMI archive and writes a JSON
text-object document. The repository-wide authority is
[`docs/specs/formats/dialogue-text.md`](../../../../docs/specs/formats/dialogue-text.md);
this page describes how the tool represents it.

## Locating the subfile

The dialogue subfile is the EMI entry whose TOC `load_argument` is `0x80010000`.
Each US archive contains exactly one; `index` reports its payload offset and whether
it parses.

## Executable ASCII

`extract`, `analyze`, `validate` and `pack` also support the US executable's
exception-screen ASCII strings. `extract-corpus` exports them when the complete
source root includes `SLUS_004.22`. This support covers the strings referenced by
the pointer words at `0x8017F470..0x8017F53C`, with an exclusive upper bound.
The consumer at `0x8014B3C4` passes these pointers to renderer `0x80150098`.
The renderer reads bytes until NUL, advances on ASCII space and handles newline
`0x0A`. These bytes use ASCII rather than the dialogue character table.

The JSON schema is `bof3.executable-text/v1`. It contains `source`, mandatory
`source_sha256`, `build`, `load_address`, `consumer`, `renderer` and `strings`.
Each string retains its `address`, whole-file `file_offset`, original `capacity`
including NUL, pointer `references` and editable `text`. Shared pointers appear
on one allocation. Retain every allocation and edit only `text`.

Packing accepts printable ASCII and newline, reserves one byte for NUL and
refuses growth beyond the original string, excluding alignment padding. A
shortened string is NUL-padded inside its allocation. Unchanged strings preserve
every byte. Pointer edits and relocation are refused. The numeric printf format
and unproven exception-table indices are outside this support.

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
unchanged. Character names and other record families require
their own consumer evidence. Runtime acceptance remains a separate gate.

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
