# Reviewed graphics byte ranges

`bof3-textures` owns graphics decoding, indexing and reviewed byte roles.
The text tool consumes these roles for source coverage and whole-VRAM payload
recognition. Its [text formats](../../bof3-text/docs/formats.md) describe text
codecs, editable documents, instructions and other coverage boundaries.

```sh
bin/harness textures ranges out/extracted/BIN/ETC/GAME.EMI --json
bin/harness textures ranges --stdin --json < out/extracted/SLUS_004.22
bin/harness textures ranges --identity --json
```

The `bof3.texture-ranges/v1` response binds every range to the original source
size and SHA-256, selected entry descriptor, payload hash and owner interpretation.
Entry ranges use offsets relative to the stored payload; add the entry offset
for an archive byte position. Executable ranges use file offsets. Compressed
ranges identify physical storage through the checked origin walk, rather than
using decoded offsets as physical positions. Each range retains its evidence.
Unrecognized inputs return `source: null` and supply no classifications.

`whole_vram_tiles` identifies an entirely qualified upload payload. Partial
prefixes do not qualify for whole-payload filtering. The text scanner checks
current archive identity and does not create permanent VRAM registry windows.
The text harness builds and selects this component. Direct text invocations
require `bof3-textures` on `PATH` or `PSX_BOF3_TEXTURES` set to its executable.
Owner failures, changed interpretations and mismatched descriptors fail before
text filtering. `--no-payloads` keeps its comparison-only empty-filter behavior.

The ordinary texture index reports logical resources and records. Those records
can contain opaque bytes; their geometry alone does not qualify physical fields.
The rules below retain their original source, consumer and acceptance limits.

## Raw sprite-bank coverage

For original US `BPLU349.EMI` entry 0, coverage recognizes the image headers,
packed image words, mode-2 token streams and 32-halfword palette records consumed
by `SLUS_004.22@0x8014E6D0`, `0x8014E7F8`, `0x8014E8FC` and `0x80166C48`.
It requires the original archive identity, raw transfer mode, load address
`0x8003B800` and the exact 331,788-byte payload hash. The fixed ranges use payload
coordinates and become file offsets by adding the entry's 2048-byte offset.

This graphics rule excludes mode-2 four-byte opaque prefixes, allocation gaps
and selection data. The [selected BPLU program and cell rule](#selected-bplu-program-and-cell-fields)
qualifies separate numeric fields. Packed words contain unused bits; classification
does not assign those bits a meaning. It does not establish the absence of rasterized text,
complete CLUTs, runtime selection, upload, residency or display. Other sprite
banks remain unresolved under this rule.

## Standalone sprite coverage

Coverage recognizes the image and palette section words, their directory words,
eight image records and eight 32-halfword palettes in original US `DRG04_00.EMI`
entry 3. Qualification requires the original archive, entry offset 26624, raw
mode, destination `0x8003B80C`, checkword `0x10`, size 313320 and payload SHA-256
`6669621720080f29b7a431d150ade295210ecef49d1dec3085b4e828c962f538`.
Only consumed packed or raw image words qualify. Allocation gaps, animation
programs and selection storage stay unresolved.

The original `MAGIC151.EMI` entry 3 handler at `0x801EF388` selects slot 545
for form code 11 through the tables in `BATTLE.EMI` entry 15. The original LBA
table and complete disc user bytes bind that slot to `DRG04_00.EMI`. After the
loader reports ready, the handler at `0x801EF484` calls GAME `0x801C1254` with
the first identifier of the current hint row. For an original hint in `0..18`
and a retained root-array word of 12 at `0x8003B800`, this installs root ordinal
0 at `0x8003B80C`. Successful loading and stable original code, tables and source
bytes remain conditions.

The installed root supplies the palette reader at `0x80166C48` and the image
selector at `0x8014DC08`, followed by the queued image reader at `0x8014E6D0`.
Palette indices must be in `0..7`. The finite image cases use selectors
`4, 6, 12, 14, 44, 46, 16, 18` for image slots `0..7`, respectively, through
the original request argument `(actor[8] + 4) & 255`. They require a changed
request selector, a clear image-skip bit, queue capacity, valid animation
records, bounded destinations, nonaliased memory and ordinary helper returns.
These cases establish conditional source use. They do not establish natural
selector production, general caller bounds, actual invocation, residency,
hardware transfer, live CLUTs, displayed pixels or safe editing.

The same conditional route qualifies eight more original entry-3 banks. Each
requires mode 0, destination `0x8003B80C`, checkword `0x10`, its exact archive
and payload identities, and the entry geometry below. Selectors are listed in
image-slot order. Each selector reaches a valid image and animation program
through `(actor[8] + 4) & 255`; separate palette indices `0..7` reach the eight
palettes. All conditions and evidence limits above still apply.

| Archive | Form codes | Slot | Entry offset | Payload bytes | Image selectors |
| --- | --- | --- | --- | --- | --- |
| `DRG00_00.EMI` | 1, 5 | 525 | 40960 | 44404 | 6, 4 |
| `DRG01_00.EMI` | 14 | 530 | 40960 | 38844 | 4, 6, 12, 14, 44, 46, 16, 18 |
| `DRG03_00.EMI` | 15, 16 | 540 | 40960 | 76072 | 4, 6, 12, 14, 44, 46, 16, 18 |
| `DRG05_00.EMI` | 17, 18 | 546 | 38912 | 70700 | 4, 6, 12, 14, 44, 46, 16, 18 |
| `DRG06_00.EMI` | 3, 13 | 551 | 34816 | 233124 | 4, 16, 6, 44, 46, 12, 14 |
| `DRG07_00.EMI` | 2, 12 | 552 | 38912 | 42264 | 4, 6, 12, 14, 44, 46, 16, 18 |
| `PAPYD00.EMI` | 0, 6, 9 | 562 | 32768 | 11796 | 4, 16, 28, 6 |
| `PAPYU00.EMI` | 0, 6, 9 | 567 | 32768 | 17400 | 4, 6, 12, 16, 28 |

The original form table selects `PAPYD00` when the request work byte 8 is below
2 and `PAPYU00` otherwise. Both pointer cells select the same slot for each
other case. Mode-2 image storage includes the consumed flag and token bytes;
its four-byte opaque prefix stays unresolved. Destination bounds must include
any final-token overshoot. Programs, cells, selection triples, unconsumed tails
and allocation gaps stay unresolved. Banks loaded at other root addresses
require separate binding evidence.

Four further first-slot cases use the same loader, ordinal-0 initializer and
consumers. Their original BATTLE form-pointer cells are `0x800B4A44..0x800B4A68`.
They retain the same archive, entry, mode, destination, checkword and payload
qualification. The table lists only image slots with a valid finite selection
case. Palette indices are separate from image selectors.

| Archive | Form codes | Slot | Entry offset | Payload bytes | Image selectors | Palette indices |
| --- | --- | --- | --- | --- | --- | --- |
| `DRG02_00.EMI` | 21, 22 | 535 | 20480 | 72420 | 4, 6, 44, 12, 46, 14, 16, 18 | 0..7 |
| `DRG08_00.EMI` | 23, 24 | 557 | 20480 | 84732 | 4, 12, 44, 16, 6, 14, 46, 18 | 0..7 |
| `CRYUD00.EMI` | 25 | 521 | 40960 | 110592 | 16, 40, 0, 4, 8, 36, 20, 12, 32, 24, 56, 6, 44, 60, 48 | 0 |
| `CRYUU00.EMI` | 25 | 523 | 40960 | 110592 | 6, 8, 12, 20, 16, 24, 32, 36, 4, 40, 44, 56, 60, 48 | 0 |

For form 25, request work byte 8 below 2 selects `CRYUD00`; otherwise it selects
`CRYUU00`. CRYUD image slot 2 uses selector 0 with actor byte 8 equal to 252,
through the original low-byte argument. CRYUU selectors cover image slots
`0..7` and `9..14` in that order. Image slot 8 and its directory word stay
unresolved because this route has no valid selection case for them. Only the
listed consumed image storage, selected directory words and palette records
qualify. The conditions, exclusions and evidence limits above still apply.

### MAGIC131 raw replacements

Coverage also recognizes 38 original raw replacement archives in `BIN/BPLCHAR`.
Each profile requires its exact original archive, entry 3, payload offset and
hash, raw mode, checkword `0x10`, payload size and reviewed destination.
The following finite cases bind the original BATTLE entry-15 form table to the
hint-indexed `u16` request in `MAGIC131.EMI#3@0x801EF3D0`. Slots in each range
follow archive suffixes `01..04` in order. Image selectors follow image-slot order.

| Archives | Form | Family | Slots | Image selectors |
| --- | --- | --- | --- | --- |
| `DRG00_01..04.EMI` | 1 | 1 | 526..529 | 6, 4 |
| `DRG01_01..04.EMI` | 14 | 1 | 531..534 | 4, 6, 12, 14, 44, 46, 16, 18 |
| `DRG02_01..04.EMI` | 21 | 1 | 536..539 | 4, 6, 44, 12, 46, 14, 16, 18 |
| `DRG03_01..04.EMI` | 15 | 1 | 541..544 | 4, 6, 12, 14, 44, 46, 16, 18 |
| `DRG05_01..04.EMI` | 17 | 1 | 547..550 | 4, 6, 12, 14, 44, 46, 16, 18 |
| `DRG07_01..04.EMI` | 2 | 1 | 553..556 | 4, 6, 12, 14, 44, 46, 16, 18 |
| `DRG08_01..04.EMI` | 23 | 1 | 558..561 | 4, 12, 44, 16, 6, 14, 46, 18 |
| `PAPYD01..04.EMI` | 0 | 1 | 563..566 | 4, 16, 28, 6 |
| `PAPYU01..04.EMI` | 0 | 2 | 568..571 | 4, 6, 12, 16, 28 |
| `CRYUD01.EMI` | 25 | 1 | 522 | Same as `CRYUD00` above |
| `CRYUU01.EMI` | 25 | 2 | 524 | Same as `CRYUU00` above |

The suffix determines the finite hint, preceding BPL bank and selected root.
`D/U` selects the family-1/family-2 bank. The ordinal is the identifier's position
in the original three-byte hint row at `0x80181B34 + 3*hint`.

| Suffix | Hint | BPL bank | Identifier | Ordinal | Replacement destination |
| --- | --- | --- | --- | --- | --- |
| DRG/PAPY `01` | 7 | `BPLD/U247.EMI` | 7 | 2 | `0x8007A80C` |
| DRG/PAPY `02` | 10 | `BPLD/U278.EMI` | 7 | 1 | `0x8005F80C` |
| DRG/PAPY `03` | 13 | `BPLD/U457.EMI` | 7 | 2 | `0x8007180C` |
| DRG/PAPY `04` | 15 | `BPLD/U478.EMI` | 7 | 1 | `0x8005680C` |
| CRYU `01` | 5 | `BPLD/U034.EMI` | 0 | 0 | `0x8003B80C` |

This is a conditional source-use path. GAME's original phase table at
`0x801C7B54` selects the family-1/2 BPL request at `0x80197BC0` before phase 3
calls `0x801C1400` with argument 1 at `0x80197C9C`. With active count 1 and the
listed identifier at `0x80144F5D`, its call at `0x801C14BC` reaches initializer
`0x801C1254`, which publishes the selected root at actor offset `0x4C` and writes
actor byte `0x24 = 1`. Phase gates, successful unskipped loads, stable original
tables and the root array, ordinary helper returns and noninterference are
required. Root retention through battle setup and the later MAGIC131 states is
an explicit condition; natural retention has not been established.

MAGIC131's preceding handler at `0x801EF1C0` requests the same BPL family and hint
only if no active actor has both byte `+0x79 == 4` and bit 0 of word `+0x128`
set. Family selection uses bit 1 of `0x801462EC`. Its next handler indexes the
form's slot table by the hint. The SLUS LBA table and complete matching physical
disc user bytes bind both requests to their original archives. Independently
decoded BPL roots reproduce the texture tool's decoded hashes and select the
raw replacement destination.

After loader-ready, `0x801EF488` sets actor byte 0 to 1 and calls `0x8014D6B8`
at `0x801EF508` with `(actor[8]+4)&255`. Each listed selector is below 128.
Retaining actor byte `0x24 = 1` avoids root reinitialization in `0x8014D6DC` and
reaches `0x8014DC08`. Valid selected animation records, queue capacity, stable
source and code, and bounded nonaliasing output remain conditions. The queued
image reader at `0x8014E6D0` consumes the same packed, raw or mode-2 image storage
described above, including any final-token overshoot.

Only the image section word, selected image-directory words, image headers and
consumed image words or tokens qualify under this route. CRYUU01 image slot 8
has no qualified selection or directory word. The
[stored-copy rule](#stored-image-copies) separately identifies its encoding.
This route has no bound palette-copy call after the
replacement, so its palette section word, directory and records remain unresolved.
Opaque mode-2 prefixes, tails, programs and selection storage also stay unresolved.
These profiles establish no natural selector production, invocation, runtime
residency, transfer, live CLUT, display, absence of rasterized text or safe editing.

The same MAGIC131 route qualifies image storage in sixteen original
`RYUD/U00..03.EMI` and `RYUD/U10..13.EMI` archives. Each requires entry 3 at
offset 40960, raw mode, checkword `0x10`, its reviewed destination, exact payload
size and hash, and the complete original archive identity. The first group has
110592-byte payloads. The second has 61260 bytes for `D` and 57480 for `U`.

| Archives | Form | Family | Slots | Reachable image slots |
| --- | --- | --- | --- | --- |
| `RYUD00..03.EMI` | 25 | 1 | 580..583 | 0..15 |
| `RYUU00..03.EMI` | 25 | 2 | 588..591 | 0..15 |
| `RYUD10..13.EMI` | 4 | 1 | 584..587 | 0..12, 14, 15 |
| `RYUU10..13.EMI` | 4 | 2 | 592..595 | 0..12, 14, 15 |

In image-slot order, the first group uses selectors
`4,6,8,40,0,12,44,36,16,24,28,32,20,56,60,48`. The second omits selector 60
and image slot 13. Selector 0 requires actor byte 8 equal to 252. The request
and retained-root conditions above still apply, and all palettes remain unresolved.
The stored-copy rule separately identifies image 13 in the form-4 banks without
adding a reachable selector.

| Suffix | Hint | BPL bank | Identifier | Ordinal | Replacement destination |
| --- | --- | --- | --- | --- | --- |
| `00` | 7 | `BPLD/U247.EMI` | 7 | 2 | `0x8007A80C` |
| `10` | 3 | `BPLD/U025.EMI` | 5 | 2 | `0x8007A80C` |
| `01`, `11` | 10 | `BPLD/U278.EMI` | 7 | 1 | `0x8005F80C` |
| `02`, `12` | 13 | `BPLD/U457.EMI` | 7 | 2 | `0x8007180C` |
| `03`, `13` | 15 | `BPLD/U478.EMI` | 7 | 1 | `0x8005680C` |

### REI and RT raw replacements

Eight original `REID/U00..01.EMI` and `RTD/U00..01.EMI` archives have a separate
conditional image and palette path. Each profile requires its complete original
archive, entry 3, raw mode, checkword `0x10`, 110592-byte payload, exact payload
hash and reviewed destination. REI payloads start at offset 38912; RT payloads
start at 26624.

GAME's BPL request and root publisher above provide two finite root cases.
Suffix `00` uses hint 0, `BPLD/U012.EMI`, identifier 0 and ordinal 0 at
`0x8003B80C`. Suffix `01` uses hint 10, `BPLD/U278.EMI`, identifier 7 and
ordinal 1 at `0x8005F80C`. The initializer writes that ordinal to actor halfword
`+0x2C`. The replacement request tests this same halfword for zero or nonzero,
so the request suffix and retained root agree. Successful unskipped loading and
retained actor root and byte `+0x24 = 1` remain conditions.

REI's original BATTLE03 queue producer at `0x801D6818`, with cursor 0, an
available actor and actor byte `+0x79 = 4`, allocates mode 0, selector 11 and
stores actor 0 at queued-slot offset `+0x74`. Dispatcher `0x801E5824` publishes
this pointer at `0x801EB4E0`. The mode-0 table and `0x801E5AF4` reach
`0x801E74B8`, whose states 2 and 3 call request `0x801E75B0` and post-load
reader `0x801E7634`. The reader selects this queued actor and passes
`(actor[8]+4)&255` to `0x8014D6B8` at `0x801E7698`. Its call at `0x801E76E4`
copies palette index 0 through `0x80166C48`.

RT's original BATTLE15 producer at `0x800AE108`, with action 64, reads byte 77
at `0x800B6550`, allocates mode 2 and stores incoming argument 1 at slot
`+0x74`. Argument 1 equal to GAME actor 0 and selection byte `0x80146374 = 0`
are explicit conditions. Code request `0x800AE1F8` uses action row `0x800B6860`
to request slot 336, bound to the complete original `MAGIC064.EMI` disc bytes.
With flag `0x801462E8 & 0x800`, the queued dispatcher and `0x800ADFBC` reach
row handler `0x801EEC7C`. Outer state 1 and inner states 8 and 9 call the
original MAGIC064 request at `0x801EF2A4` and reader at `0x801EF3C8`.
The reader requires saved work byte 9 equal to `0x80` and loader-ready. It passes
the selected actor's byte 8 unchanged to `0x8014D8D4`, which requires a changed
selector, and copies palette index 0 at `0x801EF4E0`.

Both request handlers test actor byte 8 below 2 for `D`, or otherwise select `U`.
The same retained byte must satisfy the request and reader. This restricts REID
selectors to 4 or 5 and RTD selectors to 0 or 1. REIU admits below-128 selectors
whose pre-add actor byte is at least 2, including wrapped selectors 0..3.
RTU admits selectors 2..127. Only the following finite cases qualify; selector
lists follow the listed image-slot order.

| Archives | Slots | Image slots | Selectors |
| --- | --- | --- | --- |
| `REID00..01.EMI` | 572..573 | 1 | 4 |
| `REIU00..01.EMI` | 574..575 | 0..12 | 6, 0, 16, 20, 32, 28, 12, 8, 36, 40, 44, 56, 48 |
| `RTD00..01.EMI` | 576..577 | 3 | 0 or 1 |
| `RTU00..01.EMI` | 578..579 | 0..5 | 4, 6, 16, 48, 56, 58 |

RTD selectors 0 and 1 both select image and program 3. Their control triples
are `03 03 01` and `03 03 00`; the third byte selects the already reviewed
mirror cases. Selector 1 qualifies payload bytes `[46167,46170)` in both
archives. The request and ready handler must retain that same actor and byte 8.
It adds no bytes in images, program bodies, cells or palettes. Paths through
selectors 8 and 9 remain outside these source bindings.

The SLUS LBA table and complete original physical user bytes bind these slots.
Only root words 0 and 4, reachable image-directory words, consumed image storage,
palette-directory word 0 and its 32 halfwords qualify under these routes.
The stored-copy rule separately identifies the image encodings listed below.
Other image bytes and directory words, palette index 1, opaque mode-2 prefixes,
tails, programs and selections remain unresolved. Stable original code and source, bounded dispatch states,
ordinary noninterfering helper returns, clear image-skip bit, valid animation
records, queue capacity and bounded nonaliasing work and output are conditions.
Output bounds include mode-2 final-token overshoot. Natural incoming arguments,
state progression, selector and root retention, invocation, residency, transfer,
display, absence of rasterized text and safe editing remain unverified.

## Stored image copies

Coverage identifies the following original raw image records as exact stored
copies of independently reviewed image encodings. Each destination retains its
original archive identity and requires entry 3, raw mode, checkword `0x10`, its
reviewed offset and load argument, and the complete payload size and SHA-256.
The sprite-bank root and image directory bind each record to its containing
allocation. Complete record bytes, dimensions, mode, consumed length, decoded
output and final-token overshoot match the witness.

All offsets below use entry-payload coordinates. Each destination has root 0.

| Destination archives | Image slots | Record offsets | Witness archive and entry | Witness root and slots | Witness record offsets |
| --- | --- | --- | --- | --- | --- |
| `CRYUU00.EMI` | 8 | 16736 | `BPLU349.EMI` entry 0 | Root 2, slot 8 | 240168 |
| `CRYUU01.EMI` | 8 | 18972 | `BPLU349.EMI` entry 0 | Root 2, slot 8 | 240168 |
| `REID00..01.EMI` | 2 | 17840 | `REIU00.EMI` entry 3 | Root 0, slot 2 | 17840 |
| `REID00..01.EMI` | 12 | 69188 | `BPLU349.EMI` entry 0 | Root 1, slot 11 | 178076 |
| `RTD00..01.EMI` | 0, 1, 4, 5 | 40, 5512, 20816, 29440 | `RTU00.EMI` entry 3 | Root 0, slots 0, 1, 4, 5 | 40, 5512, 20396, 29020 |
| `RYUD10..13.EMI` | 13 | 40896 | `RYUD00.EMI` entry 3 | Root 0, slot 13 | 40896 |
| `RYUU10..13.EMI` | 13 | 38892 | `RYUU00.EMI` entry 3 | Root 0, slot 13 | 38892 |

Only translated witness fields already classified as known nontext qualify.
These are the eight-byte image header and consumed packed words or mode-2
tokens. The four-byte mode-2 prefix at relative `[8,12)` remains unresolved.
The rule excludes directory words, palettes, programs, selection storage and
allocation tails. Physical copies retain separate source identities.

This rule establishes stored image encoding. It does not extend the destination
selector lists or establish destination selection, invocation, transfer,
residency, display, absence of rasterized text or safe editing.

## Compressed image copies

The four original US BRTD entry-7 banks below contain complete image records
identical to reviewed raw `RTD00.EMI` and `RYUD00.EMI` records. They retain the
exact archive, transfer-mode-1, destination, offset, size, checkword and payload
identities listed under [BRT image fields](#compressed-brt-image-and-palette-fields).
All donor images belong to root 0.

| Destination archives | Copies of RTD00 images 0..5 | Copies of RYUD00 images 0..15 |
| --- | --- | --- |
| `BRTD247.EMI` | Root 1, images 0..5 | Root 2, images 0..15 |
| `BRTD457.EMI`, `BRTD467.EMI` | Root 0, images 0..5 | Root 2, images 0..15 |
| `BRTD478.EMI` | Root 0, images 0..5 | Root 1, images 0..15 |

Native root and relative-directory parsing establishes each record's allocation.
Complete record bytes, dimensions, mode, consumed length, decoded image bytes
and final-token overshoot match the donor. Only previously qualified donor
fields transfer. RTD00 retains its existing RTU00 copy provenance.

The complete outer decompression walk tracks every physical literal origin,
including overlapping references. A literal qualifies only when every decoded
use belongs to an existing reviewed numeric field or a copied donor field.
Existing physical records retain their evidence. Newly qualified mixed origins
cite this combined proof. Mode-2 prefixes, unconsumed tails and conflicting
opaque aliases remain excluded. Stored encoding does not establish destination
selection, invocation, display or safe editing.

## Stored sprite-record copies

Coverage also recognizes exact copies of independently reviewed animation
metadata, sprite cells and 32-halfword palettes. Qualification requires the
original US archive, raw entry descriptor, complete payload identity and native
root, directory and allocation relationships. Complete logical records must
match. Only witness fields qualified before this rule transfer to the copy.

| Destination archives | Witness archives |
| --- | --- |
| `BPLU349.EMI`, `CRYUD00..01.EMI`, `CRYUU00..01.EMI` | `BPLU349.EMI` |
| `DRG00`, `DRG01`, `DRG02`, `DRG03`, `DRG05`, `DRG07`, `DRG08`, each `_01..04.EMI` | The corresponding `_00.EMI` |
| `PAPYD00..04.EMI`, `PAPYU01..04.EMI` | `BPLU349.EMI`, `PAPYD00.EMI` |
| `REID00..01.EMI`, `REIU00..01.EMI` | `BPLU349.EMI` |
| `RTD00..01.EMI`, `RTU00..01.EMI` | `REID00.EMI` |
| `RYUD00..03.EMI`, `RYUD10..13.EMI`, `RYUU00..03.EMI`, `RYUU10..13.EMI` | `BPLU349.EMI` |

Animation metadata has `2 + 2*steps + 2*declared_cells` bytes. The ignored
first header byte and unfetched control fields retain their prior status.
Cells have `1 + 5*part_count` bytes within their declared allocation, with an
independently checked program-cell reference. This rule admits no empty
one-byte cells or metadata aliases. Palette records require a palette-directory
reference and a complete 64-byte allocation. Unused packed bits receive no new
meaning. Directory words, selection data and allocation tails retain their
prior status. Physical copies retain separate source identities.

This rule establishes stored numeric fields. It does not extend selector lists
or establish destination selection, invocation, animation timing, live CLUTs,
display, absence of rasterized text or safe editing.

## Compressed sprite-record copies

The original US compressed banks below contain complete animation metadata,
nonempty sprite cells and 32-halfword palettes identical to reviewed raw
`BPLCHAR` records. Each profile requires its original archive, transfer mode 1,
entry index, offset, size, destination `0x8003B800`, checkword and payload hash.

| Destination archives | Entry |
| --- | ---: |
| `PL012`, `PL015`, `PL016`, `PL025`, `PL026`, `PL034`, `PL056`, `PL247`, `PL257`, `PL267`, `PL278`, `PL27A`, `PL349`, `PL457`, `PL467`, `PL478`, `PL567`, `PL578`, `PL678`, each `.EMI` | 3 |
| The corresponding `BPLD` archives except `BPLD349.EMI` | 7 |
| `BPLD349.EMI` | 0 |
| The corresponding `BPLU` archives except raw `BPLU349.EMI` | 7 |
| `BRTD247`, `BRTD457`, `BRTD467`, `BRTD478`, and their `BRTU` counterparts, each `.EMI` | 7 |
| `START.EMI` | 7 |

Native root, relative-directory and allocation checks bind both copies. Program
metadata includes both count bytes, all declared step pairs and cell-relative
halfwords. Cells require a declared program reference and exactly
`1 + 5*part_count` bytes. Palettes require a directory reference and 64 bytes.
Complete record bytes must agree, including excluded bytes. These matches have
no metadata aliases or empty cells.

The program-only copy pass uses qualified metadata from raw `REID00` and `RTD00`
records. It follows the same complete-record and physical-origin rules.

Only donor fields classified before this rule transfer. Program header byte zero,
unfetched fields, opaque packed bits and allocation tails retain their prior
status. The complete outer decode tracks every overlapping-reference origin.
Every decoded use of a physical literal must belong to a copied donor field or
an existing reviewed field. The image-copy pass runs first, and every prior
physical record retains its evidence. The rule establishes stored numeric
representation. Destination selection, invocation, animation timing, live CLUTs,
display, absence of rasterized text and safe editing remain unverified.

## Stored raw sprite-record copies

Coverage also recognizes complete program metadata and nonempty cell records
copied into original US raw sprite banks. Profiles require the exact
original archive, raw entry descriptor and complete payload hash. Native parsing
checks the destination root, program directories, declared cell references and
allocation bounds. Donors use their actual sprite-bank or resident layout.

Complete record bytes must match. Only fields already classified in the donor
transfer, so a later-qualified donor can resolve another copy without widening
the field's role. Program header byte zero, metadata aliases, empty cells,
unfetched fields, opaque bits and allocation tails receive no new role. Earlier
physical ranges retain their evidence. Use `bin/harness text coverage --root
out/extracted --json` to inspect admitted sources and byte extents. These stored
field copies do not establish selection, invocation, display, absence of
rasterized text or safe editing.

## Stored resident-record copies

Coverage recognizes complete program metadata and nonempty cell records copied
between original US resident-animation banks. Profiles require the exact original
archive, raw transfer mode, entry descriptor and complete payload hash.

| Profile family | Entries | Destination |
| --- | --- | --- |
| Reviewed AREA resident banks | 4 or 10 | `0x800D3800` |
| Shared BATTLE, BATTLE2 and BOSS resident bank | 2 or 5 | `0x800F0800` |

Native parsing checks group-relative program directories, declared cell offsets
and containing allocations in both copies. Complete record bytes must agree.
Program metadata has `2 + 2*steps + 2*declared_cells` bytes. Cells have
`1 + 5*part_count` bytes and require a declared program reference.
Only donor fields classified before this rule transfer. Program header byte zero,
metadata aliases, empty cells, unfetched fields, opaque packed bits and tails
receive no new role. Existing physical ranges retain their evidence.

Use `bin/harness text coverage --root out/extracted --json` to inspect admitted
sources and byte extents. Matching stored fields does not establish a destination
selector, resident-base publication, callback binding, invocation, animation
timing, live CLUT, display, absence of rasterized text or safe editing.

## DRG00 selected program and cell fields

The five original `DRG00_00..04.EMI` entry-3 banks also qualify numeric fields
for selectors 6 and 4. Each requires raw mode, entry offset 40960, checkword
`0x10`, size 44404, its reviewed destination and payload SHA-256
`1effd0ac6e77259713f70e51130634e848ed75bc66ddb3a5961d9476697d92d1`.
The standalone and MAGIC131 routes above retain each source and root binding.

| Selector | Selection triple | Program offset | Cell-table offset | Selected cell interval |
| --- | --- | --- | --- | --- |
| 6 | `0,3,0` | 31254 | 31272 | `[31288,34701)` |
| 4 | `1,0,0` | 21080 | 21098 | `[21114,23842)` |

The original selector at `0x8014DC08` stores this actor's selected program at
`+0x50`. The unconditional call at `0x8014D8B0` reaches `0x8014D910`, which
publishes the cell-relative table at `+0x54` in its call delay. Fetch reader
`0x8014DB98` stores the selected relative offset at `+0x5A` in its return delay.
Each program contains eight pairs, `(3,0)` through `(3,7)`. Repeated same-actor
forward calls at `0x8014D978` fetch all eight cells. Coverage includes the two
section words, first two bytes of each selection triple, selected program
directory words, pair counts, fetched pairs, relative cell words and counted
five-byte cell records. Program header byte zero, mirror bytes, other programs
and cells, and allocation tails retain their prior status.

The separate selected-mirror rule below qualifies the third selection bytes.

MAGIC131 and GAME's loop at `0x801BDAB8` use the same actor array at
`0x80145E90`, with stride `0x140` bytes. The reviewed sort and draw path republishes
this actor before `0x8014C4D4`, which reads `actor[+0x54] + u16(actor+0x5A)`.
The publisher-retained mode byte `+0x28 = 2` selects palette leaf `0x8014CF10`
through the original table at `0x80149998`. Its page and U-wrap arithmetic
preserves the same five-byte cell reads.

Successful unskipped loading, retained `+0x24 = 1`, root and program pointers,
stable original bytes and bounded nonaliased memory remain conditions. After
MAGIC131 initialization, require actor halfword `+0x80 & 0x2880 == 0` and
`0x801DB524(index)&255 == 0`; otherwise later requests can replace the program.
Continuing calls must select this same actor. The bounded draw case requires
active state, `+0x48 = 0`, selected `+0x2A = 0`, visible geometry, sort height
below 55, list capacity, display flags 2, sort mode 6, valid prefetch/packet
storage and ordinary noninterfering helper returns. This identifies stored
fields without establishing natural state production, call scheduling,
invocation, cadence, live CLUTs, transfer, display or safe editing.

## Additional DRG program and mirror fields

The same conditional readers qualify original `DRG01`, `DRG02`, `DRG03`,
`DRG05`, `DRG07` and `DRG08` banks with suffixes `_00..04.EMI`, plus
`DRG04_00.EMI` and `DRG06_00.EMI`. Each requires its original archive,
entry 3, raw mode, checkword `0x10`, reviewed offset and destination, and exact
payload length and SHA-256. The source bindings and finite selector lists remain
those in [standalone sprite coverage](#standalone-sprite-coverage) and
[MAGIC131 raw replacements](#magic131-raw-replacements). DRG06 has seven
selectors; DRG04 and DRG06 have no additional suffixes under these routes.

Each walk starts at pair zero. Continuing reads decrement the byte countdown,
wrap at the pair count, or follow one high-bit redirect before a direct fetch.
That destination may itself have a high-bit duration. Coverage follows fetched
pairs until an index repeats and includes redirect operands, selected relative
cell words and bounded counted-cell records. Native checks reconcile complete
programs, declared allocations and every direct fetch. Metadata aliases and
out-of-declared-table fetches do not qualify. Header byte zero, unfetched
high-bit predicates, other programs/cells and allocation tails retain their
prior status. The texture tool's `query --records` output gives each source's
logical program and cell coordinates.

With selected mirror one and actor `+0x48 = 0`, part control bit 7 selects
renderer case 2 or 3. Case 2 uses the inline path at `0x8014C884..0x8014C8C8`;
case 3 calls `0x8014CF9C..0x8014D260`. Both consume the same five-byte parts.
The retained `+0x28 = 2` palette and page case remains valid. Selected mirror
values zero or one replace DRG00's `+0x2A = 0` condition; all other source,
program-retention, continuing-call and bounded-draw conditions above still apply.
This establishes stored fields without natural selector production,
scheduling, invocation, cadence, live CLUTs, transfer, display or safe editing.

A separate record qualifies each selected third byte in these banks and
`DRG00_00..04.EMI`. The original load at `0x8014DD00` and store at
`0x8014DD08` publish that byte as this actor's `+0x2A`. The renderer reads
it at `0x8014C720` and uses it at `0x8014C728..0x8014C730` to select the
reviewed mirror cases. Every qualified stored value is zero or one. The same
source/root/selector and retained same-actor draw conditions apply. This rule
adds no meaning for other selectors or control values and changes no shared
BPL field mask.

## CRYU, PAPY, RYU and RT selected fields

The same finite program and counted-cell readers qualify original
`CRYUD/U00..01`, `PAPYD/U00..04`, `RYUD/U00..03`, `RYUD/U10..13` and
`RTD/U00..01` banks. Each requires its original archive, exact raw entry-3
geometry, destination, checkword `0x10`, complete payload hash and the finite
selectors in [standalone sprite coverage](#standalone-sprite-coverage),
[MAGIC131 raw replacements](#magic131-raw-replacements) or
[RT raw replacements](#rei-and-rt-raw-replacements). The retained root,
initialization, forward/direct-fetch and same-actor draw conditions above apply.
MAGIC131's overwrite masks and helper-result conditions remain required.

RT also requires an active actor, a changed selector through `0x8014D8D4`,
and no effect-table pointer equal to that actor when `0x80196718` scans its
32 slots. Actor halfword `+0x80` bit `0x80` must be clear so `0x801DDAB4`
skips effect allocation. The complete `0x80196B9C..0x80196C8C` palette helper
writes bounded staging storage and `0x80145988` without changing the selected
actor/program or scratchpad pointer. Mode-two table entries and nonaliased
palette storage must remain valid. MAGIC064 clears actor byte-zero bit `0x40`
while preserving its active bit. Retained `+0x48 = 0`, root/program/table,
mirror zero or one and ordinary noninterfering palette/sound helpers remain
conditions for continued same-actor reads and drawing.

Native directory, allocation and finite-fetch checks qualify only consumed
program fields, selected cell words and counted five-byte records. Separate
mirror records qualify each selected third byte through the original
`0x8014DD00/0x8014DD08` load/store and `0x8014C720..0x8014C730` dispatch.
Only bytes outside earlier qualified fields are added, preserving their evidence.
Header byte zero, unfetched predicates, other records and allocation tails retain
their prior status. These rules
establish no natural production, scheduling, invocation, cadence, live CLUTs,
transfer, display or safe editing.

## REI selected program and mirror fields

The same readers also qualify `REID/U00..01.EMI` under the original source,
root and selector bindings in [REI raw replacements](#rei-and-rt-raw-replacements).
Each requires raw entry 3 at offset 38912, checkword `0x10`, size 110592,
its reviewed destination and exact payload SHA-256. REID uses selector 4;
REIU uses `6,0,16,20,32,28,12,8,36,40,44,56,48`. Only consumed numeric
program fields, selected relative cell words, counted five-byte cells and
selected mirror bytes qualify. Earlier field evidence is preserved.

The post-reader at `BATTLE#3@0x801E7634` requires the same active actor,
effect-pointer exclusion, allocation-skip bit and bounded palette staging as RT.
Its store at `0x801E7744` clears actor word `+0x128` bit zero before calling
`0x801E0B64`. This rebuild passes the character record at
`0x80144968 + 0xA4*actor[0x13C]` to `SLUS@0x80164A44` through
`0x801E0B9C/0x801E0BA0`. Require a character index below eight and battle
actor count at most three, with stable original code and tables, separate intact
frames and valid nonaliasing character, actor and enemy-record storage.

The complete SLUS rebuild copies and adjusts character stat fields through
`0x80165760`, `0x801658DC`, `0x80165B3C` and `0x801648A0`. Their four
bounded jump tables target their respective bodies; their callees store only
through supplied stat-field pointers. The last reader requires a bounded list
ending in opcode 14 or its explicit character byte `+0x1B == 255` skip case.
The rebuild's optional cap calls also remain within the same character record.
These records are separate from the actor array at `0x80145E90`, stride `0x140`.

The remaining `BATTLE#15` calls at `0x800AAA74`, `0x800A4458` and
`0x800A9BD8` update stat storage without changing actor root, program, cell
table, mirror, palette mode or draw controls. The bounded actor count restricts
the last call to its player branch through `0x800A9FC4`; its arithmetic leaves
perform no stores. Valid nonaliased enemy-record storage remains required for
its final status operations. The REI reader restores its saved scratchpad
pointer before updating the separate queued work.

Continued reads and drawing require the same retained actor, palette mode two,
mirror zero or one, no overriding request, bounded geometry, lists and packets,
and ordinary noninterfering palette helpers. Opaque headers, unfetched predicates,
other records and allocation tails retain prior status. These fields establish
no natural selector or state production, scheduling, invocation, cadence, live
CLUTs, transfer, display, rasterized-text absence or safe editing.

## Save-icon source coverage

Original `SHOP.EMI` entry 8 and `START.EMI` entry 9 contain the same save-header
producer, palette and three icon frames. Qualification requires the exact original
archive, reviewed entry index and offset, raw mode, load address `0x801EEC00`,
checkword `0x141`, size 5268 and payload SHA-256
`6e7b93d4fd75586413b5ab3a6785d7b2492d8d1b566a3a6d79941bb656131a4d`.
Each physical copy retains its own source identity.

The original SHOP call at `0x801D6A08` supplies mode `0x13` in its delay slot.
This mode copies the 32-byte palette at payload offset `0x1268` and three
128-byte packed 4-bit frames at `0x1294`, `0x1354` and `0x1414` into the `SC`
header at `0x800C1800`, at offsets `0x60`, `0x80`, `0x100` and `0x180`.
Only these image fields qualify here; frame gaps remain unresolved. The
[producer instructions](../../bof3-text/docs/formats.md#save-icon-source-coverage)
retain their separate code classification.

The path requires successful original loading, stable source and code, a bounded
valid title, an ordinary returning helper, an intact frame and nonaliased work.
These source extents do not establish actual invocation, the active supplying
archive, BIOS animation, displayed pixels or safe editing.

## Compressed bank metadata coverage

Coverage recognizes compression metadata in transfer-mode-1 entries whose
complete archive matches the original US inventory. The shared original loader
selects this decoder at `0x80162420..0x80162444`, checks the first payload word
against the TOC checkword at `0x801623A8..0x80162420`, and initializes and resumes
decoding through `0x80164028..0x801644CC`.

The host walk requires at least six payload bytes, a nonzero matching checkword,
bounded token reads, references into initialized output, complete payload
consumption and a decoded byte count equal to the checkword. If a condition
fails, this rule classifies no bytes. These checks bound the classifier; they do
not establish bounds enforced by the original decoder or validate destination RAM.

Only the four-byte checkword, token controls, reference displacement operands,
optional length extensions and final `FF` terminator become known nontext.
Literal data and skipped prefix bytes `[4,6)` remain unresolved except for the
source-bound model fields below. References may
overlap earlier output. Neither those references nor their literal dependencies
establish the roles of decoded bytes, the absence of rasterized text or runtime
behavior. Other transfer modes retain their existing classification.

## Compressed sprite palette-zero coverage

Existing source-qualified compressed image profiles also qualify palette slot 0
for roots whose array word and image-section word already have reviewed image
roles. The complete checked decode must contain three ordered roots. Each new
palette pointer and its 64-byte record must fit the root's palette section.

GAME `0x801C1254` publishes the selected root at `0x801C1308` and calls
`SLUS_004.22@0x80166C48` with palette index 0 at `0x801C13DC`. The reader
follows the root's palette-section word and first directory word, then copies
32 halfwords to the primary and mirrored staging buffers. Coverage qualifies
those two relative words and that record under the existing source profile's
selection and loading conditions.

A physical literal qualifies only when every decoded occurrence belongs to the
union of reviewed image, program and palette fields. Earlier classifications
retain their evidence. Other palette slots, opaque aliases and allocation tails
retain their prior status. This establishes stored representation; completed
loading, active CLUT selection, hardware transfer, display, rasterized-text
absence and safe edits remain separate requirements. Standalone roots without a
qualified installation path remain unresolved.

## Compressed PL025 numeric and image fields

For original US `BIN/PLCHAR/PL025.EMI` entry 3, coverage also recognizes stored
literals whose decoded uses all have reviewed numeric or encoded-image roles.
The rule requires transfer mode 1, file offset 71680, destination `0x8003B800`,
checkword 408916, size 332447 and payload SHA-256
`d58be9091c72afa0aa758f6b9e1a3aa542dd1ffdae693f805fbca1f271ec8744`.
It also requires the original whole archive identity and a complete checked
compression walk.

| Decoded interval | Consumed numeric field |
| --- | --- |
| `[0,12)` | Three root-relative words |
| `[16,20)` | Root-0 palette-directory offset |
| `[63680,63684)` | Palette-slot-0 record offset |
| `[63684,63748)` | Palette record of 32 halfwords |

GAME `0x801C12F0` reads the three root words and `0x801C1308` publishes the
selected root. For root 0 and palette slot 0, its original JAL at `0x801C13DC`
calls `SLUS_004.22@0x80166C48`, which reads the palette-directory and record
offsets, then copies 32 halfwords to A0 and its `+0x4000` mirror.

The original helper `SLUS_004.22@0x8014DC08..0x8014DD3C` reads each root's
image-section word, adds the root base, loads a u32 by image index, adds that
table base and queues the resulting record pointer at `0x80145A04`.
The following finite directory entries point to the reviewed image records.
The indices describe conditional alternatives; actual PL025 selection and queue
capacity remain unqualified.

| Root | Image-section word | Image-directory words | Finite indices |
| --- | --- | --- | --- |
| 0 | `[12,16)` | `[28,144)` | 0 through 28 |
| 1 | `[75736,75740)` | `[75752,75900)` | 0 through 36 |
| 2 | `[274112,274116)` | `[274128,274260)` | 0 through 32 |

The reviewed image fields cover 99 records consumed by
`SLUS_004.22@0x8014E6D0` and its helpers `0x8014E7F8` and `0x8014E8FC`.
Each record has an eight-byte width, height and mode header. The 27 mode-1
records store six five-bit values per four-byte word. Coverage includes every
consumed word, including its unused bits and final unused values, without
assigning those bits a meaning. The 72 mode-2 records have a four-byte opaque
prefix followed by flags, literals and reference tokens. Coverage includes the
consumed tokens and excludes each prefix and allocation tail.

The compression walk records each literal's physical source offset and copies
that origin through overlapping references. A stored literal becomes known
nontext only when every decoded occurrence lies in these reviewed fields.
An opaque occurrence excludes the whole origin, even when another occurrence
has a known role. The 145 decoded field intervals therefore admit 250914 stored
literal bytes. Origins `[24,27)` and `[43904,43908)` still have opaque
uses and remain unresolved.

Selection data, programs, cells, other literal bytes and
skipped outer prefix bytes retain their prior status. These fields establish
stored numeric and encoded-image representation. Runtime selection, residency,
transfer, upload, CLUT, display and safe editing remain unqualified. Images can
contain rasterized text.

## Counted sprite metadata coverage

Coverage recognizes consumed fields in original US raw entries loaded at
`0x800E3800`. The complete archive must match the original disc inventory, and
the payload length must be `4 + 8 * count`, where `count` is byte zero.
Changed sources and other shapes remain unresolved under this rule.

The original consumer at `SLUS_004.22@0x8014DD3C` reads the count as a loop bound
and finds the first matching little-endian record ID at bytes 0..1. It copies
record bytes 2, 4, 5, 6 and 7 into actor fields; byte 7 becomes a halfword.
Coverage therefore classifies the count byte and record ranges `[0,3)` and `[4,8)` as known nontext.
Header bytes `[1,4)` and record byte 3 remain unresolved. Although a halfword load
fetches record byte 3, the following byte store discards it.

These ranges qualify numeric stored fields. They do not establish runtime
selection, unique physical suppliers, complete payload ownership, the absence
of rasterized text or safe editing.

## Staged palette coverage

Coverage recognizes type-0 palette-word allocations in exact original US
archives. The TOC destination and entire allocation must lie within
`0x80033800..0x80037000`. Both destination and nonzero size must be multiples
of 32 bytes, and the first payload word must match the TOC check word.
This includes the 5120-byte AREA024 entry-8 allocation at `0x80033E00`.

Original SLUS readers at `0x80162B08` and `0x80162C14` copy entry bytes to
their declared destination. Staging at `0x8014E284` copies those rows into
`0x80037800`, after clearing the four excluded rows at `0x80037000..0x80037800`.
The pending-upload consumer at `0x8014E22C` describes 256-by-32 image words
and conditionally calls `LoadImage` at `0x8017B690`.
The [texture catalog](../../bof3-textures/docs/source-paths.md#raw-uploads-and-shared-palettes)
exposes the same source-bound allocations as `palette-bank` resources.
This classification establishes stored image words. Completed loading,
intervening writes, active CLUT selection, sampling, display and safe edits
remain separate requirements. Changed archives, other destinations and container
gaps do not qualify.

## PL026 compressed image field coverage

The exact original US `BIN/PLCHAR/PL026.EMI` qualifies reviewed fields in
entry 3, file offsets `[67584,329576)`. The entry uses transfer mode 1,
load address `0x8003B800`, decoded size 339380 and packed payload SHA-256
`48fd888c1d80aa62a812f0753fed508d2c65c7e82778783c3bd87233ec4ffc39`.
The original family-0/index-4 request selects file slot 622. Its ID row
`[0,2,6]` binds the conditional GAME root publication at
`0x801C12F0/0x801C1308` to this source.

The reviewed directory reader and image consumers are the same original routines
used by the PL025 rule. This source has 29, 37 and 24 image-relative words in
three directories. Its 90 images use 27 mode-1 and 63 mode-2 encodings.
Coverage recognizes the reviewed root words, directory words, image headers
and consumed encoded words or tokens. The complete outer walk propagates each
physical literal origin through overlapping references. It includes a stored
literal byte only when every decoded use lies within those reviewed fields.

For identifier 0, root 0 and initialization slot 0, the original call at
`0x801C13DC` passes index 0 to `0x80166C48`. Coverage also recognizes decoded
relative words `[16,20)` and `[63680,63684)`, plus the selected 32-halfword record
at `[63684,63748)`. The reader copies that record to
`[0x80039600,0x80039640)`. Under the concrete original dispatch and dimension
limits preserving rectangle `0/480/256/32`, the image-input path through
`0x8014E22C`, `0x8017B690`, table `0x8017D488` and callback `0x8017CE9C`
consumes the mirror as graphics words. Stable descriptor/source memory and
ordinary returning calls and queue operations remain conditions. The complete
compression walk accepts a literal origin only when every decoded use lies in
the primary image fields or these additional fields. Existing primary records
retain their evidence.

Other palette, program and cell fields, mode-2 prefixes, unconsumed allocation
tails and origins with any opaque alias remain unresolved. This rule qualifies
stored representation without proving actual selection, transfer, residency,
upload, display, color-channel format, CLUT use, the absence of rasterized text
or safe editing. Other archives retain their existing records and gates.

## Additional compressed player image field coverage

Coverage recognizes reviewed fields in entry 3 of the following exact original US
`BIN/PLCHAR/` archives. These entries use
transfer mode 1 and load address `0x8003B800`. Each profile also requires its
file offset, packed size, checkword and payload SHA-256 below. The checkword
is the decoded byte count.

| Archive | File offset | Packed bytes | Checkword | Payload SHA-256 |
| --- | --- | --- | --- | --- |
| `PL247.EMI` | 71680 | 327152 | 408396 | `0ed30aece981f5c9b97ca1cd96ce35f55c51e905b29225feba5e014a41edc895` |
| `PL257.EMI` | 71680 | 328837 | 406900 | `0f2d7c069e65a7c7ad6c3e798a707bfb129c0885b28736e5e7cc304660ee298b` |
| `PL278.EMI` | 73728 | 312757 | 392820 | `263581239eac67a6c611e7c3fc2aa59aae111a1e3bbbed02bd79383d454a61f2` |
| `PL012.EMI` | 73728 | 292491 | 360412 | `934d31899d96e8110cab9a4544c4891a5c1705a4efc9bea247241ded1899ebc8` |
| `PL457.EMI` | 71680 | 284731 | 344824 | `ee268e24edccaf3ada151d42ad718a261dcc0b88d16498bb647aad01f5804a0e` |
| `PL578.EMI` | 75776 | 270419 | 329248 | `6e4ac6e56a9b114d229f63b36051f7a9c0dc769b90a93d12770c6cc4f139ccce` |
| `PL478.EMI` | 73728 | 268792 | 330744 | `23ed7ca1e3ab1711d3c43f508d9de4fb5073055a38803af9b9b763f1dd717d5a` |
| `PL267.EMI` | 67584 | 258331 | 337364 | `27fcef3a95f29736d7db0a280fcbcf0f11647b0e58c2ffb9493e0cc1f1346e80` |
| `PL015.EMI` | 75776 | 250259 | 296840 | `c63e42c1e6c7a31a274b56dec6e78e1ed41677f4115959777e9c9b1e3a81b290` |
| `PL034.EMI` | 65536 | 243732 | 291900 | `d998bc717ebb2d8009fb5a6f4f67e1449bf017b9f816a4de8fa7f1b762780637` |
| `PL27A.EMI` | 73728 | 239772 | 310136 | `396ae98f20eefc3212ad657b9be8787db3445cf97fe80160baab1c5a21e8e5c1` |
| `PL056.EMI` | 69632 | 219544 | 275808 | `447d974c63960bf269ad0a92523cbda6cd66b1e82e1d4bc8e8fc1cf5afa5b159` |
| `PL567.EMI` | 69632 | 215993 | 273792 | `c05f8a142d977e802e41e76a05933aa3d68da7e7644806ac9e5abd8ca638bf94` |
| `PL467.EMI` | 67584 | 214266 | 275288 | `cc36dfd4a9c994f92cec65e5f118d33bcff1d005dcc264afccf5dec74ec4fcc4` |
| `PL349.EMI` | 73728 | 198091 | 236692 | `e85b01d37bb1ae7eb25acf06d4b583fd042c9067ede2abd7cdae38461dd58fef` |
| `PL678.EMI` | 71680 | 199934 | 259712 | `5afc207a11dcf744a83fb24f6310aa8fb78790139726781ef4299def9c428416` |
| `PL016.EMI` | 71680 | 179827 | 227304 | `90788ddcc329308292905eedd01d83a9bf916a1a5a23d3fa31ca99f0099d9816` |

The original family-0 requests bind each archive to the following file slot and
IDs. GAME uses these IDs to select a root through the same conditional
publication path as PL026. The reviewed directory reader and image consumers
also match that rule. Each source has three directories and uses only the
reviewed mode-1 and mode-2 image encodings.

| Archive | Family index | File slot | IDs | Images |
| --- | --- | --- | --- | --- |
| `PL247.EMI` | 7 | 625 | `[2,4,7]` | 88 |
| `PL257.EMI` | 8 | 626 | `[2,5,7]` | 93 |
| `PL278.EMI` | 10 | 628 | `[2,7,8]` | 92 |
| `PL012.EMI` | 0 | 618 | `[0,1,2]` | 102 |
| `PL457.EMI` | 13 | 631 | `[4,5,7]` | 84 |
| `PL578.EMI` | 17 | 635 | `[5,7,8]` | 88 |
| `PL478.EMI` | 15 | 633 | `[4,7,8]` | 83 |
| `PL267.EMI` | 9 | 627 | `[2,6,7]` | 84 |
| `PL015.EMI` | 1 | 619 | `[0,1,5]` | 98 |
| `PL034.EMI` | 5 | 623 | `[0,3,4]` | 85 |
| `PL27A.EMI` | 11 | 629 | `[2,7,10]` | 75 |
| `PL056.EMI` | 6 | 624 | `[0,5,6]` | 86 |
| `PL567.EMI` | 16 | 634 | `[5,6,7]` | 80 |
| `PL467.EMI` | 14 | 632 | `[4,6,7]` | 75 |
| `PL349.EMI` | 12 | 630 | `[3,4,9]` | 64 |
| `PL678.EMI` | 18 | 636 | `[6,7,8]` | 79 |
| `PL016.EMI` | 2 | 620 | `[0,1,6]` | 89 |

The profiles classify root-relative words, image-directory words, image headers
and consumed encoded words or tokens. The complete compression walk includes a
physical literal only when every decoded use lies in those fields. The primary
image profiles leave palette, program and cell fields, opaque aliases,
mode-2 prefixes and allocation tails unresolved. These profiles qualify stored
representation. Actual requests,
transfer, residency, upload, display, the absence of rasterized text and safe
editing remain unqualified.

## Additional PL012 root-0 image-input fields

The exact PL012 entry-3 gate also qualifies global decoded offsets `[16,20)`
and `[63680,63684)` as relative-pointer words. For initialization slot 0, the
original reader at `0x80166C48` copies `[63684,63748)` into the mirror at
`[0x80039600,0x80039640)`. These are global decoded coordinates; root 0 starts
at 12 and is already included.

Original `0x8014E22C` supplies rectangle `0/480/256/32` and source
`0x80037800` through `0x8017B690`. Concrete original table words select
`0x8017D488` and callback `0x8017CE9C`. The callback constructs a GPU image-load
stream and a dimension-derived CPU/DMA source span. Under stable descriptor,
source, dispatch and dimension limits, the entire 64-byte mirror supplies
32 image-input halfwords within that span. Live dispatch, queue lifetime,
hardware transfer, color-channel format, CLUT use and display remain unqualified.

The compression walk intersects roles across every decoded alias. It preserves
the primary image records and their evidence, then emits separate records for
origins newly qualified by the union with these fields. An opaque alias still
excludes its physical literal. This rule does not establish the absence of
rasterized text or safe editing.

## Compressed START root-0 image fields

The exact original US `BIN/ETC/START.EMI` has a reviewed entry-7 profile with
transfer mode 1, load address `0x8003B800`, file offset 174080, packed size
185082 and checkword 245444. Its payload SHA-256 is
`48f91b9d1f086bb2adbb109eab1fd4cf6a6fefd38d7acf7a6b8cdec36fcd3910`.

Original slot `0x268` selects START at physical LBAs `[61926,62179)`.
Successful unskipped loading with `0x80146854 & 0x20` clear and stable bank memory
condition the alternate-loop index-1 and START dispatcher index-0 path.
The initializer at `0x801E3EC4` calls GAME `0x801C1254` to publish root 0,
then calls `0x8014D6B8` at `0x801E3FB8`. The original path through
`0x8014D6DC` reaches `0x8014DC08` with selector 12 and image index 4.
Ready byte 3 alone does not prove that entry 7 was loaded.

The profile qualifies decoded root word `[0,4)`, root-0 relative word
`[12,16)`, its 21 image-directory words at `[28,112)`, and the headers and
consumed encodings of seven mode-1 and fourteen mode-2 images. Each image has
an independently checked serialized extent under the shared original readers.
Only image 4 has the selected conditional call path; the profile does not
establish selection of every directory entry. Every alias of an admitted packed
literal must lie in the reviewed fields.

Roots 1 and 2, palette, program, cell and selection fields, mode-2 prefixes,
unconsumed allocation tails and opaque aliases remain unresolved. Ordinary
returns and bank/work noninterference are conditions. Actual invocation,
transfer, residency, upload, display, rasterized-text absence and safe editing
remain unqualified.

## Compressed BPLD578 root-0 image fields

Coverage recognizes original US `BIN/BPLCHAR/BPLD578.EMI` entry 7 with
transfer mode 1, load address `0x8003B800`, file offset 96256, packed size
211642 and checkword 331788. Its payload SHA-256 is
`4b321cc1ff09b75bad5cd7a34cc1ef806fffcc7c26adb618279d04c07e0857cc`.

Original family 1/index 17 requests slot 492 at LBA 51797. Successful
unskipped loading with flag `0x20` clear and stable hint 17, bank and work
memory condition the GAME phase-1/2/3 route. With identifier 5 at work index 0,
count 1, route 0, clear record masks `0x0080/0x4000` and ordinary helper
returns, GAME `0x801C1400` calls `0x801C1254` to publish root 0. Its call at
`0x801BF56C` then reaches `0x8014DC08` through `0x8014D6B8/0x8014D6DC`
with selector 0 and image 0. These are conditional inputs, not observed
requests or roster contents.

The profile qualifies decoded words `[0,4)` and `[12,16)`, the 14 image
directory words at `[28,84)`, and the headers and consumed encodings of two
mode-1 and twelve mode-2 images. Each record has a separately checked
serialized extent; only image 0 has the selected conditional call path.
The compression walk admits a physical literal only when every decoded use
has a reviewed role.

An additional profile qualifies roots 1 and 2 under the same loading, stability
and ordinary-return conditions. With count 1 and work index 0, conditional
identifiers 7 and 8 match the original row `05 07 08` at ordinals 1 and 2.
Their identifier-map bytes are 0 and 1; the record flags at `0x80144974` and
`0x80144A18` must have masks `0x0080/0x4000` clear. `0x801C1254` publishes
roots `0x8005680C` and `0x8007180C`. `0x801BF448` reselects the same work,
and `0x801BF56C` reaches `0x8014DC08` with selector 0 and images 4 and 9,
respectively, through the existing route-0 path.

The additional fields are the root-array words `[4,12)`, root-relative words
`[110604,110608)` and `[221196,221200)`, and the 16 and 14 image-directory
words at `[110620,110684)` and `[221212,221268)`. Root 1 includes headers
and consumed encodings for three mode-1 and thirteen mode-2 images; root 2
includes three mode-1 and eleven mode-2 images. Each serialized extent is
checked separately; the selected conditional paths bind only images 4 and 9.
A physical literal qualifies only when every decoded alias belongs to either
the unchanged primary root-0 fields or these additional fields. Primary
records retain their evidence.

A separate program profile qualifies 1,337 decoded bytes for root 0 and
selector 0 under the same source identity and publisher conditions. Original
`0x8014D910`, `0x8014DB98` and `0x8014D978` bind these finite fields.
All offsets below refer to the decoded bank at `0x8003B800`.

| Decoded interval | Consumed field |
| --- | --- |
| `[20,28)` | Program-directory and selection relative words |
| `[83988,83990)` | Selected program-relative halfword |
| `[84023,84068)` | Pair-count byte and 22 countdown/index pairs |
| `[84069,84114)` | Jump operand and 22 referenced cell-relative halfwords |
| `[98064,98067)` | Selected image index, program index and nonzero mode predicate |

The declared count is 23 pairs. The last pair redirects the cursor to pair 21;
its first byte never becomes a countdown on this selected path. The count is
an equality-wrap threshold, not a general index bound. Repeated calls require
unchanged program and work fields; no frame cadence is established.

The 22 referenced cells contain 303 parts. For each cell at decoded offset `C`,
the byte at `C` is its part count. With `j` from zero to one less than that
count, the four bytes at `[C+2+5*j,C+6+5*j)` are signed X and Y offsets and
unsigned U and V components. Every selected part has bit 7 clear. The reviewed
consumer requires the same selected work, `work[0]=1`, `work+0x24=1`,
`work+0x2A=1`, `work+0x48=0` and `work+0x28=0`, ordinary helper returns,
stable code and buffers, and valid writable nonaliased packets, queue heads
and frames. These are conditional inputs, not observed resources or execution.

The complete compression walk intersects every decoded use of each physical
literal with the image and program field unions. Existing image records keep
their boundaries and evidence; newly qualified origins use separate program
evidence. Any opaque alias excludes its physical origin.

Program header byte 0 at `[84022,84023)`, the jump-control byte at
`[84068,84069)` and each whole part-flag byte remain unresolved. Other
root-header words, palette fields, programs, selectors and cells, opaque
aliases, mode-2 prefixes and allocation tails also remain unresolved. Actual
invocation, transfer, residency, timing, allocation success, upload, display,
rasterized-text absence and safe editing remain unqualified.

## Compressed BPLD257 image fields

Coverage recognizes original US `BIN/BPLCHAR/BPLD257.EMI` entry 7 with
transfer mode 1, load address `0x8003B800`, file offset 98304, packed size
213807 and checkword 368652. Its payload SHA-256 is
`b627b7492e8462589d8ed191e365b6c53c3595c86d8efa4209cf59128b963a45`.

Original family 1/index 8 requests slot 483 at LBA 50489. Successful
unskipped loading without flag `0x20` early completion and stable hint 8,
bank and work memory condition the GAME phase-1/2/3 route. With count 1,
work index 0 and conditional identifiers 2, 5 or 7, row `02 05 07` selects
root ordinals 0, 1 or 2. The identifier-map bytes are 2, 5 and 0; record
flags at `0x80144ABC`, `0x80144CA8` and `0x80144974` must have masks
`0x0080/0x4000` clear. Under route 0 and ordinary helper returns,
`0x801C1400` calls `0x801C1254` to publish roots `0x8003B80C`,
`0x8005F80C` and `0x8007A80C`. `0x801BF448` reselects the same work,
and `0x801BF56C` reaches `0x8014DC08` through
`0x8014D6B8/0x8014D6DC` with selector 0 and images 7, 0 and 4,
respectively. These are conditional inputs, not observed requests or roster
contents.

The profile qualifies root-array words `[0,12)`, root-relative words
`[12,16)`, `[147468,147472)` and `[258060,258064)`, and the 16, 14 and
16 image-directory words at `[28,92)`, `[147484,147540)` and
`[258076,258140)`. It also qualifies the headers and consumed encodings of
thirteen mode-1 and thirty-three mode-2 images. Each serialized extent is
checked separately; the selected conditional paths bind only images 7, 0
and 4 in their respective roots. The complete compression walk admits a
physical literal only when every decoded alias belongs to this three-root
field union.

Other root-header words, palette, program, cell and selection fields, opaque
aliases, mode-2 prefixes, allocation tails, raw entry 3 and container gaps
remain unresolved. Actual invocation, transfer, residency, argument and
resource safety, upload, display, rasterized-text absence and safe editing
remain unqualified.

## Compressed BPLD457 image fields

Coverage recognizes original US `BIN/BPLCHAR/BPLD457.EMI` entry 7 with
transfer mode 1, load address `0x8003B800`, file offset 94208, packed size
200599 and checkword 331788. Its payload SHA-256 is
`b682d37a2ec37adf08dde1a514f98b85ae4aa0ac68e616cdbb697e140ba5b858`.

Original family 1/index 13 requests slot 488 at LBA 51178. Successful
unskipped loading without flag `0x20` early completion and stable hint 13,
bank and work memory condition the GAME phase-1/2/3 route. With count 1,
work index 0 and conditional identifiers 4, 5 or 7, row `04 05 07` selects
root ordinals 0, 1 or 2. The identifier-map bytes are 4, 5 and 0; record
flags at `0x80144C04`, `0x80144CA8` and `0x80144974` must have masks
`0x0080/0x4000` clear. Under route 0 and ordinary helper returns,
`0x801C1400` calls `0x801C1254` to publish roots `0x8003B80C`,
`0x8005680C` and `0x8007180C`. `0x801BF448` reselects the same work,
and `0x801BF56C` reaches `0x8014DC08` through
`0x8014D6B8/0x8014D6DC` with selector 0 and images 0, 0 and 4,
respectively. These are conditional inputs, not observed requests or roster
contents.

The profile qualifies root-array words `[0,12)`, root-relative words
`[12,16)`, `[110604,110608)` and `[221196,221200)`, and the 13, 14 and
16 image-directory words at `[28,80)`, `[110620,110676)` and
`[221212,221276)`. It also qualifies the headers and consumed encodings of
eight mode-1 and thirty-five mode-2 images. Each serialized extent is checked
separately; the selected conditional paths bind only images 0, 0 and 4 in
their respective roots. The complete compression walk admits a physical
literal only when every decoded alias belongs to this three-root field union.

Other root-header words, palette, program, cell and selection fields, opaque
aliases, mode-2 prefixes, allocation tails, raw entry 3 and container gaps
remain unresolved. Actual invocation, transfer, residency, argument and
resource safety, upload, display, rasterized-text absence and safe editing
remain unqualified.

## Other compressed BPLD image fields

Coverage recognizes the original US `BIN/BPLCHAR/` entries below under the
whole-archive original identity gate, transfer mode 1 and load address
`0x8003B800`. Each entry also requires its exact offset, packed size,
checkword and payload SHA-256. `BPLD349` has one entry and uses entry 0;
the others use entry 7. The original generic loader accepts that one-entry
layout without an entry-7 restriction.

| Archive | Entry | File offset | Packed bytes | Checkword | Payload SHA-256 |
| --- | ---: | ---: | ---: | ---: | --- |
| `BPLD012` | 7 | 98304 | 166846 | 368652 | `053c5e252a85d53b7ddedd9043fe09427c21a6869d7df79e6e27f049a318058b` |
| `BPLD015` | 7 | 98304 | 168143 | 331788 | `58818212b2924ea74f946de57fcb2ca5217df55df59c13c7efc64b712c1e8441` |
| `BPLD016` | 7 | 98304 | 126764 | 331788 | `5788a3234f6ce4d8ee0075d9345b4b4a9958f2adf7d319b95dacefc2b6385268` |
| `BPLD025` | 7 | 98304 | 197048 | 368652 | `b266a31ac21b11a086c4fe548fb5bdc170b15fd5f9e2d4601dc90930bc9b0e7b` |
| `BPLD026` | 7 | 98304 | 155687 | 368652 | `3ae2a261a6fac405171fc8051dc88247c5290806c9d06c247099cdd3631ad7b7` |
| `BPLD034` | 7 | 98304 | 129525 | 331788 | `e4dcdf27c2d790c1731de7c1d3a4186a2427ba64243bcc2e40d9270a3bbf65bf` |
| `BPLD056` | 7 | 96256 | 157020 | 331788 | `a08ce37780b173281c607a3cdff5c4291458fbce794d9a571198aae46092515a` |
| `BPLD247` | 7 | 96256 | 199085 | 368652 | `21e3a59a7b0771d43f42babd465142899935f87e50188b7c6829fba8959c4ee1` |
| `BPLD267` | 7 | 100352 | 172578 | 368652 | `d96a0b1fc1f3e9cb0d19c349459d60e336a114a9d402261d6d6b83cd52c5b018` |
| `BPLD278` | 7 | 98304 | 210377 | 368652 | `145a87d311cce9f157630b5c77b4bd97f54afc5d10fcf5f5ed3ce5d3e53afa8a` |
| `BPLD27A` | 7 | 98304 | 138325 | 269856 | `b9dd8cb2698064159c49a3bb1abb550c7163ac7622d23a3fe07d3033f73d5dd0` |
| `BPLD349` | 0 | 2048 | 129413 | 331788 | `b9229f2ef691a502bf1c9d5a94626f2ed97b9922d92ff1e6db96e6a03662f8e4` |
| `BPLD467` | 7 | 96256 | 159254 | 331788 | `0a2c91b60f2eaf4ec493d6014c0eabacc06979c481d07fab1367710eaeace1f8` |
| `BPLD478` | 7 | 94208 | 196922 | 331788 | `a48d4b434e176f043f2482050906d13788c979bdebe252807952b1e148922802` |
| `BPLD567` | 7 | 98304 | 173974 | 331788 | `03cf547e8cfedee08e9581d4202e1f41425d83e6daebb6c4495098f464e7d3af` |
| `BPLD678` | 7 | 98304 | 170285 | 331788 | `69136a4e291a626a5285c1168642a726bedfc670a593589b3f5fec9c3998cb02` |

Original family 1 requests bind these slots and half-open physical LBA ranges.
The three identifiers in each row are separate conditional inputs for root
ordinals 0, 1 and 2; they are not observed requests or roster contents.

| Archive | Request index / slot | Physical LBAs | IDs for roots 0 / 1 / 2 |
| --- | --- | --- | --- |
| `BPLD012` | 0 / 475 | [49312,49462) | 0 / 1 / 2 |
| `BPLD015` | 1 / 476 | [49462,49612) | 0 / 1 / 5 |
| `BPLD016` | 2 / 477 | [49612,49742) | 0 / 1 / 6 |
| `BPLD025` | 3 / 478 | [49742,49906) | 0 / 2 / 5 |
| `BPLD026` | 4 / 479 | [49906,50051) | 0 / 2 / 6 |
| `BPLD034` | 5 / 480 | [50051,50181) | 0 / 3 / 4 |
| `BPLD056` | 6 / 481 | [50181,50325) | 0 / 5 / 6 |
| `BPLD247` | 7 / 482 | [50325,50489) | 2 / 4 / 7 |
| `BPLD267` | 9 / 484 | [50661,50814) | 2 / 6 / 7 |
| `BPLD278` | 10 / 485 | [50814,50982) | 2 / 7 / 8 |
| `BPLD27A` | 11 / 486 | [50982,51113) | 2 / 7 / 10 |
| `BPLD349` | 12 / 487 | [51113,51178) | 3 / 4 / 9 |
| `BPLD467` | 14 / 489 | [51341,51485) | 4 / 6 / 7 |
| `BPLD478` | 15 / 490 | [51485,51645) | 4 / 7 / 8 |
| `BPLD567` | 16 / 491 | [51645,51797) | 5 / 6 / 7 |
| `BPLD678` | 18 / 493 | [51965,52114) | 6 / 7 / 8 |

The GAME phase-1/2/3 route requires successful unskipped model loading without
flag `0x20` early completion, stable request hint, code, bank and count,
nonaliased work and scratch memory, and ordinary helper returns. With count 1,
work index 0, route 0 and record masks `0x0080/0x4000` clear,
`0x801C1400` calls `0x801C1254` to select the identifier's immutable
row position and publish its root. `0x801BF448` reselects the same work;
`0x801BF56C` reaches the image reader `0x8014DC08` through
`0x8014D6B8/0x8014D6DC` with selector 0.

The profiles qualify the three root-array words, each root's image-directory
relative word, the finite directory words, image headers and consumed encodings
listed below. Root offsets are decoded offsets from `0x8003B800`.
Directory counts and selected images follow root order. Each serialized image
extent is checked separately; the conditional paths select only the listed
images, while the finite image encodings have their own representation proof.

| Archive | Decoded roots | Directory words | Mode-1 / mode-2 images | Selected images |
| --- | --- | --- | --- | --- |
| `BPLD012` | 12 / 110604 / 221196 | 15 / 12 / 16 | 16 / 27 | 2 / 2 / 7 |
| `BPLD015` | 12 / 110604 / 221196 | 15 / 12 / 14 | 10 / 31 | 2 / 2 / 0 |
| `BPLD016` | 12 / 110604 / 221196 | 15 / 12 / 15 | 13 / 29 | 2 / 2 / 5 |
| `BPLD025` | 12 / 110604 / 258060 | 15 / 16 / 14 | 14 / 31 | 2 / 7 / 0 |
| `BPLD026` | 12 / 110604 / 258060 | 15 / 16 / 15 | 17 / 29 | 2 / 7 / 5 |
| `BPLD034` | 12 / 110604 / 221196 | 15 / 13 / 12 | 10 / 30 | 2 / 1 / 0 |
| `BPLD056` | 12 / 110604 / 221196 | 15 / 14 / 15 | 11 / 33 | 2 / 0 / 5 |
| `BPLD247` | 12 / 147468 / 258060 | 16 / 13 / 16 | 14 / 31 | 7 / 0 / 4 |
| `BPLD267` | 12 / 147468 / 258060 | 16 / 15 / 16 | 16 / 31 | 7 / 5 / 4 |
| `BPLD278` | 12 / 147468 / 258060 | 16 / 16 / 14 | 14 / 32 | 7 / 4 / 9 |
| `BPLD27A` | 12 / 147468 / 258060 | 16 / 16 / 4 | 14 / 22 | 7 / 4 / 0 |
| `BPLD349` | 12 / 110604 / 221196 | 13 / 12 / 15 | 10 / 30 | 1 / 0 / 2 |
| `BPLD467` | 12 / 110604 / 221196 | 13 / 15 / 16 | 11 / 33 | 0 / 5 / 4 |
| `BPLD478` | 12 / 110604 / 221196 | 13 / 16 / 14 | 9 / 34 | 0 / 4 / 9 |
| `BPLD567` | 12 / 110604 / 221196 | 14 / 15 / 16 | 10 / 35 | 0 / 5 / 4 |
| `BPLD678` | 12 / 110604 / 221196 | 15 / 16 / 14 | 11 / 34 | 5 / 4 / 9 |

The complete compression walk propagates ultimate literal origins through
initialized overlapping references. It admits a physical literal only when
every decoded alias belongs to that archive's reviewed three-root field union.
Mode-1 CPU output overshoot is recorded without an argument or resource safety
claim.

Other root-header words, palette, program, cell and selection fields, opaque
aliases, mode-2 prefixes, allocation tails, raw entry 3 where present and
container gaps remain unresolved. Actual invocation, transfer, residency,
argument and resource safety, upload, display, rasterized-text absence and
safe editing remain unqualified.

## Compressed BRT image and palette fields

Coverage recognizes the following original US `BIN/BPLCHAR/` entry-7
profiles under the whole-archive identity gate. Each requires transfer mode 1,
destination `0x8003B800`, and its exact file offset, packed size, decoded size
and checkword, and payload SHA-256.

| Archive | Hint / slot | Physical LBAs | File offset | Packed bytes | Decoded bytes / checkword | Payload SHA-256 |
| --- | --- | --- | ---: | ---: | ---: | --- |
| `BRTD247` | 7 / 513 | [54987,55131) | 83968 | 171442 | 368652 | `3ccf09c57d2675a5938c5a5350cc936d238388039dc1b040c126db70ab7880c8` |
| `BRTD457` | 13 / 514 | [55131,55275) | 81920 | 172879 | 331788 | `457cf5138ff71487486d0f48fa423d587b11a49d7ebaa70fa05ac1c02d3d62b0` |
| `BRTD467` | 14 / 515 | [55275,55400) | 83968 | 131534 | 331788 | `7478852c3c488a95220d88e748296f04e44f0001cd741b19ba7405592f56e22b` |
| `BRTD478` | 15 / 516 | [55400,55540) | 81920 | 169202 | 331788 | `e483530c86f92c5773f595aa0efd2048f9550bab209652826fdf645c8919cde2` |
| `BRTU247` | 7 / 517 | [55540,55680) | 83968 | 163748 | 368652 | `baa3b5279e41b88d1c2a6ff0ce8a0186201517232e4d5e25164f179e1ed72d08` |
| `BRTU457` | 13 / 518 | [55680,55823) | 81920 | 170862 | 331788 | `45f75644733e12bde6badf26a49398e68f4699466e5ff41a0cea6b2662e952b6` |
| `BRTU467` | 14 / 519 | [55823,55944) | 83968 | 123828 | 331788 | `a99542bf45778bf7f6ccd47334ef7e0808d4cd7b99e46a1133b59dd201a4868d` |
| `BRTU478` | 15 / 520 | [55944,56083) | 81920 | 166017 | 331788 | `e42ef5798590f2dd63a854eab7f7875be0daa3fe900a75f96315f74bcff9200e` |

The source binding starts with the conditional GAME family-1/2 publisher path
and each hint row's three identifiers. At work index 0 with count 1,
`0x801C1254` publishes the chosen root and sets actor byte `+0x24` to 1.
The original BATTLE entry-15 call at `0x800A69D4` enters `0x800A7C14`
when the resolved form is not `0xFF`. Its first loop copies the complete
320-byte actor record from `0x80145E90` to `0x801EBF20`, preserving the
root at `+0x4C`, byte `+0x24` and the active flag. Returning helpers, stable
code and tables, valid nonaliased memory, and retention of these saved fields
remain conditions.

Original BATTLE entry-3 `0x801E12DC` requires no scanned actor to have
flag `0x2000`. With actor byte `+0x79` different from 4 and a free slot,
it allocates mode 0, selector 12 and publishes the actor pointer at queued
slot `+0x74`. The standard dispatchers `0x801E5824` and `0x801E5AF4`
reach `0x801E7818`, whose byte-1 table selects the request and post-load
handlers. The request at `0x801E7948` requires flag `0x8000`, exact plain
hint 7, 13, 14 or 15, and a true result from `0x801E7B34`. One finite
case has saved record 0 active and its word `+0x128` bit 0 set. Request
actor byte 8 below 2 selects slots 513..516; other values select 517..520.

The complete physical archive bytes match the original SLUS LBA-table
supplier. On the successful normal unskipped path without flag `0x20` early
completion, raw entry 3 finishes before compressed entry 7 replaces the bank
at the same destination. Each preceding BPL root address equals the
corresponding decoded BRT root address. BRT247 roots are 12, 147468 and
258060; the other roots are 12, 110604 and 221196. Identifiers follow the
immutable hint row, not an inference from the filename.

After loader readiness, `0x801E7BA0` copies the saved actor record back,
restoring its root and byte `+0x24`. It captures request actor byte 8 before
that copy and restores it afterward. The finite path requires counts at
`0x801462F0` and `0x801463CF` equal to 1, actor index 0, valid nonzero
stat divisors, bounded writes and queues, and helpers that return without
changing the root, selector or bank. Call `0x801E8414` supplies
`(actor[8]+4)&255` to `0x8014D6B8`; call `0x801E8438` supplies palette
index 0 to `0x80166C48`. The image path uses the shared readers
`0x8014DC08`, `0x8014E6D0`, `0x8014E7F8` and `0x8014E8FC`.

The initial D request admits selectors 4 and 5. Selector 4 reaches one image
in each root.
U variants admit below-128 selectors whose pre-add actor byte is at least 2,
including wrapped selectors 0..3. The following finite selectors have valid
image and program records. Each list follows admitted image-slot order;
excluded slots remain unresolved.

| Archive | Root | Admitted image slots | Selectors |
| --- | ---: | --- | --- |
| `BRTD247` | 0 | 6 | 4 |
| `BRTD247` | 1 | 0 | 4 |
| `BRTD247` | 2 | 0 | 4 |
| `BRTD457` | 0 | 0 | 4 |
| `BRTD457` | 1 | 2 | 4 |
| `BRTD457` | 2 | 0 | 4 |
| `BRTD467` | 0 | 0 | 4 |
| `BRTD467` | 1 | 0 | 4 |
| `BRTD467` | 2 | 0 | 4 |
| `BRTD478` | 0 | 0 | 4 |
| `BRTD478` | 1 | 0 | 4 |
| `BRTD478` | 2 | 1 | 4 |
| `BRTU247` | 0 | 0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13 | 6, 0, 8, 12, 16, 24, 28, 32, 40, 44, 36, 48, 50 |
| `BRTU247` | 1 | 0, 1, 2, 3, 4, 5 | 8, 6, 16, 48, 56, 58 |
| `BRTU247` | 2 | 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 | 65, 6, 8, 40, 0, 12, 44, 36, 16, 24, 28, 32, 20, 56, 60, 48 |
| `BRTU457` | 0 | 0, 1, 2, 3, 4, 5 | 8, 6, 16, 48, 56, 58 |
| `BRTU457` | 1 | 0, 1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 | 57, 6, 24, 16, 36, 20, 28, 40, 44, 32, 48, 12, 8 |
| `BRTU457` | 2 | 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 | 65, 6, 8, 40, 0, 12, 44, 36, 16, 24, 28, 32, 20, 56, 60, 48 |
| `BRTU467` | 0 | 0, 1, 2, 3, 4, 5 | 8, 6, 16, 48, 56, 58 |
| `BRTU467` | 1 | 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 | 57, 6, 16, 24, 28, 0, 20, 32, 8, 12, 40, 44, 36, 48, 50 |
| `BRTU467` | 2 | 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 | 65, 6, 8, 40, 0, 12, 44, 36, 16, 24, 28, 32, 20, 56, 60, 48 |
| `BRTU478` | 0 | 0, 1, 2, 3, 4, 5 | 8, 6, 16, 48, 56, 58 |
| `BRTU478` | 1 | 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 | 65, 6, 8, 40, 0, 12, 44, 36, 16, 24, 28, 32, 20, 56, 60, 48 |
| `BRTU478` | 2 | 0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12 | 57, 6, 8, 40, 16, 24, 36, 44, 12, 20, 32, 48 |

Each profile qualifies the root-relative image and palette words, selected
image-directory words, image headers and consumed encodings, palette-directory
word 0, and its 32-halfword record. A physical literal qualifies only when
every decoded alias belongs to that union. Complete independent decoding
reproduces the native texture index's decoded hash and every queried record.
Mode-2 prefixes remain opaque. Root-array words are excluded because this
path restores a saved root rather than reading the BRT array. These image and
palette profiles alone do not qualify programs, selection triples or cells;
the sections below describe their separate readers. Allocation tails and aliases
with unresolved uses remain unresolved.

These are conditional source uses. Natural state and selector production,
actual requests, invocation, residency, transfer, upload, display,
rasterized-text absence and safe editing remain unverified.

## BRT program and cell fields

The exact BRT profiles above also qualify finite program and counted-cell
fields. The original post-load handler preserves request byte 8 and selects
`(byte8 + 4) & 255`. D requests require byte 8 below 2; U requests require
it at least 2. Only selectors below 128 with an in-root selection triple,
valid image and program indices, mirror byte 0 or 1, complete cell allocations
and a bounded fetch walk qualify. Other cases remain unresolved.

The post-load actor initially has state bytes `6,4,4,1`, horizontal scale zero
and work byte `+0x48 = 2`. Queued handler `0x801E8558` adds `0x2000` to
scale `+0x40` until it reaches `0x10000`. Its next call clears `+0x48`
and record `+0x124` bit `0x2000`. Root and program pointers remain intact.
Actor dispatcher `0x801DECE0` publishes that actor as the scratch work and
global local work. Original table cells at `0x801EB138`, `0x801EB1CC`,
`0x801EB204` and `0x801EB214` carry the waiting state to `0x801E13B8`,
which then sets state `2,0,0,0`.

The finite continuing case requires record flags `+0x80 = 0`, byte
`+0x119 = 0`, record `+0x124` bits `2/0x1000` clear, globals
`0x801462E0`, `0x801462EF` and `0x801463CE` zero, and global halfword
`0x801462E8` bit `0x10` clear. State 2 calls `0x801DEFE4`, whose request
matches the retained selector. Original `0x8014D8D4` therefore leaves the
program unchanged. State 3's byte-2 handler does nothing with the selection
gate clear. The state remains stable and calls `0x801DEDE4`, which reaches
`0x8014D978` on the same actor. Its finite walk uses the shared program readers.

After the completion handler clears scaling mode, counted parts use the
[reviewed BPL draw route](#selected-bpld-program-and-cell-fields), with active
work, retained `+0x24 = 1`, palette mode `+0x28 = 2` and mirror byte 0 or 1.
Other actors must be inactive. Code, bank, roots and work must remain stable;
helpers must return without changing them. Visible geometry, readable list
prefetch slots and bounded nonaliased frames, queues and packets remain required.

Every compressed literal alias must belong to the earlier image/palette fields
or the finite selected-field union. Earlier evidence stays unchanged. The BRT
root-array words remain unresolved because this route restores saved pointers.
Program header byte zero, unfetched predicates, allocation tails and opaque
part bits `0x70` also remain unqualified. These conditional source uses establish
no natural state or selector production, actual invocation, loading, cadence,
residency, display, absence of rasterized text or safe editing.

## BRTD post-completion selections

The same exact BRTD profiles also qualify four later presentation offsets.
Original BATTLE entry 3 function `0x801DEFE4..0x801DF34C` passes request
byte 8 plus an offset to `0x8014D8D4`. With byte 8 equal to 0 or 1, the
following finite cases select additional images and programs.

| Offset | Record flags `+0x80` | Global `0x801462E8` bit `0x10` | Record `+0x124` bit 2 | Selectors |
| ---: | ---: | --- | --- | --- |
| `0x08` | 0 | Set | Clear | 8, 9 |
| `0x14` | 0 | Clear | Set | 20, 21 |
| `0x18` | `0x40` | Set | Clear | 24, 25 |
| `0x30` | `0x800` | Clear | Clear | 48, 49 |

These cases require current kind byte `+0x79` equal to 0 or 7 before the
post-load handler, record byte `+0x119 = 0`, record `+0x124` bit
`0x1000` clear, and globals `0x801462E0`, `0x801462EF` and
`0x801463CE` zero. The post-load handler saves the current flags before
restoring the actor, retains the selected flag bits, and clears only bit
`0x8000` of the global halfword. Completion and the waiting-state handler
preserve the listed selector predicates. The positional-effect helper returns
without a request because record flag `0x80` is clear. Palette updates and
pending-bit clearing do not change these predicates.

State 2 reselects through `0x8014D8D4`, `0x8014D6B8` and `0x8014DC08`.
The stable state-3 loop then advances that program on the same actor. Only
valid selection triples, complete image and cell allocations, and finite
program walks qualify. The original image readers consume the selected image
headers, directory words and raw or compressed encodings. Independent decoding
reproduces their native texture records, including final-token overshoot.
Mode-2 opaque prefixes remain unresolved.

The earlier saved-root, draw, memory, helper and visibility conditions still
apply. Every physical literal alias must belong to the earlier reviewed fields
or these selected image, program and cell fields. Earlier classifications and
evidence remain unchanged. Other presentation offsets, root-array words,
program header byte zero, unfetched predicates, opaque part bits and allocation
tails remain unresolved. This conditional source use establishes no natural
predicate production, invocation, loading, residency, transfer, display,
absence of rasterized text or safe editing.

## Compressed BPLU27A image fields

Coverage recognizes image fields in original US `BIN/BPLCHAR/BPLU27A.EMI`
entry 7, with transfer mode 1, file offset 98304, destination `0x8003B800`,
packed size 133381, decoded size and checkword 275460, and payload SHA-256
`d7dfb508ba4366c13a87fba1d91af0166170839f651f4952dd6d54fa44c815d7`.
The original family-2/index-11 request selects slot 505 and physical LBAs
`[53763,53892)`, which reproduce the complete archive.

The conditional GAME phase-1/2/3 path requires successful unskipped loading
without flag `0x20` early completion, stable hint 11 and flag byte 2, original
code and bank, count 1, work index 0, clear record masks `0x0080/0x4000`,
nonaliased work and scratch, and ordinary helper returns. Identifiers 2, 7
and 10 select decoded roots 12, 147468 and 258060. The image path preserves
flag byte 2 as selector 2, which selects images 1, 4 and 1.

The profile covers the root-array words, each root's image-directory relative
word, 14/16/5 directory words, and the headers and consumed encodings of 35
images. Fourteen images use mode 1 and twenty-one use mode 2. A physical literal
qualifies only when every decoded alias belongs to these fields.
Other root-header words, palettes, programs, cells, selection fields, mode-2
prefixes, allocation tails, raw entry 3 and gaps remain unresolved.
Actual requests, invocation, transfer, residency, every-image selection, argument
and resource safety, upload, display, rasterized-text absence and safe editing
remain unverified.

## Other compressed BPLU image fields

Coverage recognizes the following original US `BIN/BPLCHAR/` entry-7
profiles under the whole-archive identity gate, transfer mode 1 and destination
`0x8003B800`. Each profile also requires the file offset, packed size,
decoded size and checkword, and payload SHA-256 below.

| Archive | Request index / slot | Physical LBAs | File offset | Packed bytes | Decoded bytes / checkword | Payload SHA-256 |
| --- | --- | --- | ---: | ---: | ---: | --- |
| `BPLU012` | 0 / 494 | [52114,52263) | 98304 | 164994 | 368652 | `e2a34bb38b0a2f4640b12edcf99abfbdfbfe94899e16dbedcf4f19654bb46dc2` |
| `BPLU015` | 1 / 495 | [52263,52414) | 98304 | 171761 | 331788 | `6b07b7680c06ab63614b5d824df4482b293d7f19ed7af15dfa9506c25c151202` |
| `BPLU016` | 2 / 496 | [52414,52543) | 98304 | 124778 | 331788 | `09a3ac9e9463bbd28d007c0fa1e12d6349b3ad21f81c40726887b1e1df6a918f` |
| `BPLU025` | 3 / 497 | [52543,52705) | 98304 | 192570 | 368652 | `8a3d4406db576778f6c28da82cc74da24d043a0a9814eba7573f09bb3572b716` |
| `BPLU026` | 4 / 498 | [52705,52845) | 98304 | 145584 | 368652 | `3a3756665db495864c14e4d3ed169a56e599f0da6562da57a439ad28a5ea30ec` |
| `BPLU034` | 5 / 499 | [52845,52977) | 98304 | 134569 | 331788 | `2b9702dd8b5a617ac209ac4463a7c2f352fc190487ce826f970a36f0012d3f66` |
| `BPLU056` | 6 / 500 | [52977,53119) | 96256 | 152690 | 331788 | `dca408e86ea40dbb6cb9b411fd82d4ebf59ae7dff4438e69f111d6de17145d4c` |
| `BPLU247` | 7 / 501 | [53119,53282) | 96256 | 196697 | 368652 | `e346011032cbdeb18efbe5519dfed41ee48379d9f122aa021b757bdefd45edd6` |
| `BPLU257` | 8 / 502 | [53282,53451) | 98304 | 208309 | 368652 | `04c55b4f45a62a7cbb50e400e643553f8d3e76cef8929bec6853877ca1758d65` |
| `BPLU267` | 9 / 503 | [53451,53598) | 100352 | 161332 | 368652 | `49b75da89fb00ed33e435221f0a44ed40a065e405d3b267f1c5068432bc97af2` |
| `BPLU278` | 10 / 504 | [53598,53763) | 98304 | 203533 | 368652 | `e89f0b3b8ed58b12352cba5516071aaa14dcd825a604a234b7adf2ed8b824fd6` |
| `BPLU457` | 13 / 507 | [54056,54221) | 94208 | 203819 | 331788 | `53db0e87ff007d5f285f369e8fe831c69ade60fb52fae4f324ac0bf30256ad3c` |
| `BPLU467` | 14 / 508 | [54221,54364) | 96256 | 156785 | 331788 | `3dc9d693b853e79393b206dddd5215c8bb94ff88cfe9805d1afa0543cfdf6ddd` |
| `BPLU478` | 15 / 509 | [54364,54525) | 94208 | 198974 | 331788 | `dd9c4770ba3eab3f49cabedeeb8b8939876b9e0cdf403edf0ccd59ca0d3d3428` |
| `BPLU567` | 16 / 510 | [54525,54675) | 98304 | 168385 | 331788 | `6b299fd8cea814cf90575f7b0f803a9b300f8cc6cf0b1d2c10b10555cb20f063` |
| `BPLU578` | 17 / 511 | [54675,54842) | 96256 | 210574 | 331788 | `0eaa04778217afe8fe7793e4666a48bbd8ecf5f92cf9324d25c6aca5c3edcbfc` |
| `BPLU678` | 18 / 512 | [54842,54987) | 98304 | 163590 | 331788 | `5d47579d411fd999b25bf7627e0a246e84653f8450e955b57ab75c7bee013a75` |

These profiles use the conditional GAME phase-1/2/3 path described for
BPLU27A, with each archive's own plain request hint and identifier triplet.
The exact flag byte remains 2 through request and image selection, choosing
selector 2. Successful unskipped complete loading, count 1, work index 0,
clear record masks `0x0080/0x4000`, stable original code and bank, nonaliased
work and scratch, and ordinary helper returns remain required.
The three identifier bytes in each archive name give the separate conditional
identifiers for root ordinals 0, 1 and 2.

Root offsets, directory counts and selected images below follow ordinal order.
Each profile qualifies its root-array words, each root's image-directory
relative word, the finite directory words, image headers and consumed image
encodings. Physical literals qualify only when every decoded alias belongs
to that profile's reviewed field union.

| Archive | Decoded roots | Directory words | Mode-1 / mode-2 images | Selected images |
| --- | --- | --- | --- | --- |
| `BPLU012` | 12 / 110604 / 221196 | 15 / 14 / 14 | 15 / 28 | 0 / 0 / 1 |
| `BPLU015` | 12 / 110604 / 221196 | 15 / 14 / 14 | 11 / 32 | 0 / 0 / 0 |
| `BPLU016` | 12 / 110604 / 221196 | 15 / 14 / 15 | 14 / 30 | 0 / 0 / 5 |
| `BPLU025` | 12 / 110604 / 258060 | 15 / 14 / 14 | 13 / 30 | 0 / 1 / 0 |
| `BPLU026` | 12 / 110604 / 258060 | 15 / 14 / 15 | 16 / 28 | 0 / 1 / 5 |
| `BPLU034` | 12 / 110604 / 221196 | 15 / 12 / 12 | 11 / 28 | 0 / 0 / 1 |
| `BPLU056` | 12 / 110604 / 221196 | 15 / 14 / 15 | 12 / 32 | 0 / 0 / 5 |
| `BPLU247` | 12 / 147468 / 258060 | 14 / 13 / 16 | 12 / 31 | 1 / 1 / 4 |
| `BPLU257` | 12 / 147468 / 258060 | 14 / 14 / 16 | 11 / 33 | 1 / 0 / 4 |
| `BPLU267` | 12 / 147468 / 258060 | 14 / 15 / 16 | 14 / 31 | 1 / 5 / 4 |
| `BPLU278` | 12 / 147468 / 258060 | 14 / 16 / 13 | 16 / 27 | 1 / 4 / 0 |
| `BPLU457` | 12 / 110604 / 221196 | 13 / 14 / 16 | 8 / 35 | 1 / 0 / 4 |
| `BPLU467` | 12 / 110604 / 221196 | 13 / 15 / 16 | 11 / 33 | 1 / 5 / 4 |
| `BPLU478` | 12 / 110604 / 221196 | 13 / 16 / 13 | 13 / 29 | 1 / 4 / 0 |
| `BPLU567` | 12 / 110604 / 221196 | 14 / 15 / 16 | 10 / 35 | 0 / 5 / 4 |
| `BPLU578` | 12 / 110604 / 221196 | 14 / 16 / 13 | 12 / 31 | 0 / 4 / 0 |
| `BPLU678` | 12 / 110604 / 221196 | 15 / 16 / 13 | 15 / 29 | 5 / 4 / 0 |

These image profiles leave other root-header words, palettes, programs, cells,
selection fields, mode-2 prefixes, allocation tails, raw entry 3 and gaps
unresolved. Actual
requests, invocation, transfer, residency, every-image selection, argument and
resource safety, upload, display, rasterized-text absence and safe editing
remain unverified.

## Compressed BPLU selected-program fields

A separate profile covers selector 2 in each of the three roots of the
seventeen archives above. It uses the same exact source identities and
conditional family-2 loader, publisher and work-selection path. Original
`0x8014DC08` selects the program. `0x8014D6DC` calls `0x8014D910` with
pair-byte offset zero; `0x8014D910`, `0x8014DB98` and `0x8014D978` read the
selected numeric fields.

The fields are the root-relative program and selection words, selected
image and program indices, program-relative halfword, pair-count byte, fetched
duration/index pairs, jump operands and referenced following-table halfwords.
The finite paths require repeated calls with unchanged code, bank, program
and work fields, valid nonaliased memory and ordinary returns. These are
conditional source paths, not observed invocations or frame cadence.

All offsets below are decoded-bank byte offsets. Each row lists roots 0, 1
and 2 in order. The fetched-pair count includes each distinct direct fetch
once, including a control pair if a jump makes `0x8014DB98` fetch it directly.

| Archive | Program offsets | Declared pair counts | Distinct fetched pairs |
| --- | --- | --- | --- |
| `BPLU012` | 37934 / 169302 / 310326 | 13 / 30 / 17 | 13 / 29 / 16 |
| `BPLU015` | 37934 / 169302 / 305010 | 13 / 30 / 23 | 13 / 29 / 22 |
| `BPLU016` | 37934 / 169302 / 266124 | 13 / 30 / 10 | 13 / 29 / 9 |
| `BPLU025` | 37934 / 199734 / 341874 | 13 / 17 / 23 | 13 / 16 / 22 |
| `BPLU026` | 37934 / 199734 / 302988 | 13 / 17 / 10 | 13 / 16 / 9 |
| `BPLU034` | 40170 / 146496 / 289420 | 13 / 10 / 19 | 13 / 10 / 19 |
| `BPLU056` | 37934 / 194418 / 266124 | 13 / 23 / 10 | 13 / 22 / 9 |
| `BPLU247` | 89142 / 220980 / 308282 | 17 / 19 / 27 | 16 / 19 / 27 |
| `BPLU257` | 89142 / 231282 / 308282 | 17 / 23 / 27 | 16 / 22 / 27 |
| `BPLU267` | 89142 / 192396 / 308282 | 17 / 10 / 27 | 16 / 9 / 27 |
| `BPLU278` | 89142 / 197690 / 346908 | 17 / 27 / 23 | 16 / 27 / 23 |
| `BPLU457` | 73524 / 194418 / 271418 | 19 / 23 / 27 | 19 / 22 / 27 |
| `BPLU467` | 73524 / 155532 / 271418 | 19 / 10 / 27 | 19 / 9 / 27 |
| `BPLU478` | 73524 / 160826 / 310044 | 19 / 27 / 23 | 19 / 27 / 23 |
| `BPLU567` | 83826 / 155532 / 271418 | 23 / 10 / 27 | 22 / 9 / 27 |
| `BPLU578` | 83826 / 160826 / 310044 | 23 / 27 / 23 | 22 / 27 / 23 |
| `BPLU678` | 44940 / 160826 / 310044 | 10 / 27 / 23 | 9 / 27 / 23 |

The count is an equality-wrap threshold, not a general index bound.
`BPLU034` root 1 has a final `80 09` pair that jumps to itself. The direct
fetch then uses both bytes as a countdown and index. Other high-bit
control bytes remain unresolved unless a direct fetch reads them.

A physical literal qualifies only when every decoded alias has an image or
selected-program role. Existing image records retain their boundaries and
evidence. This program-only rule excludes selection mode bytes, program header
byte zero, unselected programs and selectors, cell counts and part fields,
palettes, tails and opaque aliases. The selected-cell rule below extends the
reviewed field union without changing these earlier records. Actual invocation,
cadence, transfer, residency, resource safety, upload, display, rasterized-text
absence and safe editing remain unverified.

## Selected BPLU program and cell fields

Coverage also recognizes finite selector-2 program and cell fields in all
nineteen original US `BPLU` banks for family-2 hints 0 through 18. Compressed
banks require their original entry-7 geometry and payload identities above.
Raw `BPLU349` requires its entry-0 identity. Every archive must match the
original disc inventory. For hint 12, the original LBA table binds slot 506
to the complete `BPLU349.EMI` archive, and the identifier triplet is `3,4,9`.
The same loader, publisher and reselection conditions apply to all three roots.
Each root is a separate count-1, work-index-0 case with its own identifier.

The walk starts at pair 0. On countdown expiry, `0x8014D978` wraps an index
equal to the declared pair count. A following high-bit duration redirects
once through its operand; `0x8014DB98` then fetches that destination directly,
even if its duration also has the high bit set. The walk stops when a fetched
index repeats. A zero duration expires after 256 decrements. Repeated calls
and stable program/work fields are conditions, not observed frame timing.
Only fetched duration/index pairs, redirect operands, selected relative
halfwords and their cell records qualify. The header's first byte bounds the
host parser but remains unresolved because these original readers do not use it.
Every declared cell allocation is checked before selected cells are admitted.

The original GAME loop at `0x801BDAB8..0x801BDB7C` selects the same actor array
used by the publisher. With actor byte 0 equal to 1 and byte `+0x24` equal to 1,
its call through `0x8014D290` inserts the actor into the secondary draw list
when its computed sort height is below 55 and that list has space.
`0x8014BA54` calls `0x8014BC70..0x8014C47C`. With display flag byte
`0x8014832E = 2`, sort mode byte `0x8014933E = 6`, one actor and no competing
stream items, the latter copies the same work pointer into scratchpad and
calls `0x8014C4D4` at `0x8014C35C`. Valid list prefetch slots, sort tables,
queue heads, helper returns and bounded nonaliased work, frames and packets
remain conditions. This source path does not establish natural state production.

The complete renderer `0x8014C4D4..0x8014CE48` reads the cell at
`work[0x54] + u16(work+0x5A)`. For visible coordinates, work bytes
`+0x28` and `+0x48` equal to zero, the selected `+0x2A` value 0 or 1, and the
retained `+0x24 = 1`, it reads the part count and advances five bytes per part. Each part contains
one control byte, signed X and Y, and unsigned U and V. Control bit `0x80`
selects horizontal flipping, and the low four bits select width and height
through the original table at `0x8017F53C`. Bits `0x70` remain opaque.
Both the inline quad path and complete helper `0x8014CF9C..0x8014D260`
consume these fields. The palette setup leaf `0x8014CE78..0x8014CF9C` uses
the original jump table; work byte `+0x28 = 0` selects `0x8014CEC8`.

Cell counts below list roots 0, 1 and 2 in order. Their offsets come from the
selected program's cell-relative table. Use the texture tool's `query --records`
output to inspect their individual source coordinates and hashes.

| Archive | Selected cells |
| --- | --- |
| `BPLU012` | 13 / 29 / 16 |
| `BPLU015` | 13 / 29 / 22 |
| `BPLU016` | 13 / 29 / 6 |
| `BPLU025` | 13 / 16 / 22 |
| `BPLU026` | 13 / 16 / 6 |
| `BPLU034` | 13 / 10 / 19 |
| `BPLU056` | 13 / 22 / 6 |
| `BPLU247` | 16 / 19 / 25 |
| `BPLU257` | 16 / 22 / 25 |
| `BPLU267` | 16 / 6 / 25 |
| `BPLU278` | 16 / 25 / 23 |
| `BPLU27A` | 16 / 25 / 4 |
| `BPLU349` | 10 / 19 / 13 |
| `BPLU457` | 19 / 22 / 25 |
| `BPLU467` | 19 / 6 / 25 |
| `BPLU478` | 19 / 25 / 23 |
| `BPLU567` | 22 / 6 / 25 |
| `BPLU578` | 22 / 25 / 23 |
| `BPLU678` | 6 / 25 / 23 |

`BPLU27A` selects programs at decoded offsets `89142`, `197690` and `274014`.
`BPLU349` selects programs at payload offsets `35904`, `178828` and `261354`,
with pair counts `10`, `19` and `13`. The other program offsets remain as listed
above. Root-array words and consumed program/selection fields qualify separately
from image storage. Selection mode bytes, unfetched high-bit predicates,
unselected programs/cells and allocation tails stay unresolved.

For compressed storage, every decoded alias of a literal must belong to the
reviewed image, program, palette-zero or selected-cell union. Earlier image,
program and palette records retain their exact evidence and classification.
Neither the raw fields nor compressed literals establish actual requests,
execution, cadence, allocation success, residency, upload, display, absence of
rasterized text or safe editing.

## Selected BPLD program and cell fields

Coverage recognizes selector-0 program and cell fields in all nineteen original
US `BPLD` banks. Each requires the original archive identity and exact entry
index, offset, packed size, payload hash, transfer mode 1, destination
`0x8003B800`, and decoded-size checkword. `BPLD349` uses entry 0; the other
banks use entry 7. The family-1 request at GAME `0x80197BC0` uses
plain hints 0 through 18 with global flag byte `0x801462EC = 0`. Original
slots `475 + hint` and their physical LBA extents bind the complete archives.
The identifier triplet at `0x80181B34 + 3 * hint` selects each root separately.

Successful unskipped loading, stable hints and roots, count 1, work index 0,
clear record masks `0x0080/0x4000`, valid nonaliased memory and ordinary helper
returns remain conditions. GAME `0x801C1254` publishes the root and initializes
work bytes `+0x24 = 1` and `+0x28 = 2`. The wrapper at `0x801C14D0` clears
`+0x48`. The reselection call at `0x801BF56C` preserves the zero flag byte
as selector 0. `0x8014D6DC` starts its selected program at pair 0.

The [finite program walk and same-actor draw path](#selected-bplu-program-and-cell-fields)
apply here with palette mode 2 retained. The selected mode byte at `+0x2A` is
0 or 1. Original palette setup `0x8014CE78` selects jump-table entry
`0x8014CF10` for mode 2. The renderer and complete `0x8014CF9C` helper adjust
texture-page and UV fields for this mode. Their part readers still consume
one control byte, signed X and Y, and unsigned U and V. The low four control
bits select dimensions; bits `0x70` remain opaque. Visible geometry, display
flag byte 2, sort mode 6, a single queued actor, no competing stream items,
readable prefetch slots and bounded nonaliased frames and packets remain
explicit conditions.

The following decoded program offsets and selected cell counts follow roots
0, 1 and 2 in order. The texture tool's `query --records` output supplies
individual source coordinates and hashes.

| Archive | Program offsets | Selected cells |
| --- | --- | --- |
| `BPLD012` | 39374 / 165242 / 327374 | 15 / 29 / 15 |
| `BPLD015` | 39374 / 165242 / 305206 | 15 / 29 / 22 |
| `BPLD016` | 39374 / 165242 / 269356 | 15 / 29 / 7 |
| `BPLD025` | 39374 / 216782 / 342070 | 15 / 15 / 22 |
| `BPLD026` | 39374 / 216782 / 306220 | 15 / 15 / 7 |
| `BPLD034` | 40662 / 145640 / 286900 | 15 / 9 / 16 |
| `BPLD056` | 39374 / 194614 / 269356 | 15 / 22 / 7 |
| `BPLD247` | 106190 / 217340 / 311980 | 15 / 17 / 25 |
| `BPLD257` | 106190 / 231478 / 311980 | 15 / 22 / 25 |
| `BPLD267` | 106190 / 195628 / 311980 | 15 / 7 / 25 |
| `BPLD278` | 106190 / 201388 / 344856 | 15 / 25 / 26 |
| `BPLD27A` | 106190 / 201388 / 267514 | 15 / 25 / 4 |
| `BPLD349` | 35048 / 176308 / 261846 | 9 / 16 / 15 |
| `BPLD457` | 69884 / 194614 / 275116 | 17 / 22 / 25 |
| `BPLD467` | 69884 / 158764 / 275116 | 17 / 7 / 25 |
| `BPLD478` | 69884 / 164524 / 307992 | 17 / 25 / 26 |
| `BPLD567` | 84022 / 158764 / 275116 | 22 / 7 / 25 |
| `BPLD578` | 84022 / 164524 / 307992 | 22 / 25 / 26 |
| `BPLD678` | 48172 / 164524 / 307992 | 7 / 25 / 26 |

Only fetched pairs, redirect operands, consumed root/selection/directory fields,
selected cell-relative halfwords, part counts and counted parts qualify.
Program header byte zero, selection mode bytes, unfetched high-bit predicates,
unselected programs and cells, and allocation tails remain unresolved.
Every declared cell allocation must fit before any selected cell qualifies.
For a compressed literal, every decoded alias must belong to the reviewed
image, program, palette-zero or selected-cell union. Previous classifications
and evidence remain unchanged.

These conditional source paths do not establish natural state progression,
actual requests or invocation, frame cadence, residency, transfer, upload,
display, absence of rasterized text or safe editing.

## BPL family-flag program and cell fields

Coverage also qualifies finite below-128 flag cases in the original US BPLD
and BPLU banks under their exact archive and payload gates above. At GAME
`0x80197B98`, only flag byte `0x801462EC` bit 1 chooses family 1 or 2.
Byte `0x80146325 = 1` skips the unrelated coordinate-table lookup and keeps
the request enabled. Successful unskipped loading and the reviewed phase-2/3
path retain the flag, hint and root. With record masks `0x0080/0x4000` clear,
`0x801BF568` passes the whole flag byte to `0x8014D6B8`.

Each root is a separate count-1, work-index-0 case. A selector below 128 avoids
the alternate-root branch at `0x8014D6F4`. Its bit 1 must be clear for BPLD
or set for BPLU. The three-byte selection must fit inside that root, select an
image within its directory, and have mirror value 0 or 1. The program index,
complete declared cell allocations and finite fetch walk must also fit. An
invalid case qualifies no fields; other independently valid cases remain
eligible. These checks do not declare the whole selection tail a valid array.

`0x8014DC08` reads all three selection bytes. The third byte reaches work
`+0x2A` through `0x8014DD00/0x8014DD08` and controls the reviewed renderer's
mirror paths. This profile therefore qualifies that numeric byte as well as
the image/program indices, selected program fields, fetched pairs, redirect
operands, cell-relative halfwords, part counts and counted parts. Program
header byte zero, unfetched high-bit predicates and allocation tails remain
unresolved. Part control bits `0x70` remain opaque.

The initialized palette mode 2 and the same-actor draw path have the conditions
[described for BPLD](#selected-bpld-program-and-cell-fields). Flags, roots,
code and work must remain stable. Visible geometry, queue and packet capacity,
readable prefetch slots, valid nonaliased memory and ordinary noninterfering
helper returns remain required. A compressed literal qualifies only when every
decoded alias belongs to the reviewed image, program, palette, selected-cell
or family-flag union. Every earlier classification retains its evidence.

These finite source cases establish neither natural flag production nor actual
requests, execution, cadence, loading, residency, transfer, display, absence of
rasterized text or safe editing. Selectors with bit 7 set use a different root
path and remain outside this profile.

## Area surface field coverage

Coverage recognizes numeric fields in original US raw entries loaded at
`0x80104000`. The whole archive must match the original disc inventory, and
payload word zero must equal its TOC checkword. Dimensions must be nonzero.
The declared cell grid, selected packed words, coordinate reads and byte-grid
extent must fit the payload. A failed check leaves this profile unresolved.

The original consumer at `0x80154294..0x80154438` selects a primary u32 word
for each nonzero u16 cell ID. It derives the word-table base from the dimensions
and grid offset. Coverage includes those header fields, the declared cell grid
and its selected primary words. The coordinate consumer at
`0x80155560..0x801556FC` qualifies each selected four- or eight-byte coordinate
read and the relative header offset used to find it. Unused header fields remain
unresolved. These stored extents do not establish natural visitation of every cell
or a runtime bound on the caller's coordinates.

The producer at `0x80154094..0x801540F8` multiplies header bytes `[20,22)`
by four, adds the payload base and publishes the byte-grid pointer at
`0x8014931C`. The complete getter at `0x80166CB0..0x80166CEC` returns the
unchanged byte at `base + y * width + x`. Its original GAME caller uses numeric
masks and comparisons. For each declared grid byte, finite coordinates
`x = index % width` and `y = index / width` select that byte and fit signed 16
bits. Coverage therefore includes the bounded `width * height` u8 grid under
the same raw-entry gates. Previously classified fields retain their own records.
This qualifies numeric storage without assigning terrain meaning or proving
actual visitation and global caller-coordinate bounds.

The complete original sampler at `0x80154ABC..0x80154ED0` constructs the main
plane at payload offset 48. For integral coordinates, its `LW` at `0x80154B98`
reads a whole numeric word at `base + 4 * (y * width + x)`. Finite coordinates
cover each declared cell. Coverage includes `[48, 48 + 4 * width * height)` only
when that complete extent fits the payload. It adds the complement of previously
classified fields and preserves their records. A missing complete extent leaves
the earlier profile intact and admits no prefix.

The sampler also reads a separate signed-byte plane. Its relative offset is the
u16 field at payload bytes `[42,44)`, multiplied by four. For integral coordinates,
the `LB` at `0x80154B84` reads `base + y * width + x` and the sampler multiplies
that signed byte numerically. The complete original setter at
`0x80155038..0x8015507C` uses the same address for byte writes. Coverage includes
the two-byte offset field when present and the `width * height` stored plane only
when its complete extent fits the payload. Finite declared-grid coordinates select
each byte; earlier field records remain intact. Offsets at or beyond the payload
end qualify no storage. These rules establish neither initialization nor ownership
of external storage, global caller bounds, neighbor safety for fractional
coordinates, terrain meaning or actual visitation.

The original initializer at `0x80157F98..0x801580A4` reads the unsigned-u16
relative offset at payload bytes `[32,34)`, multiplies it by four and adds
`0x80104000`. Coverage includes that two-byte field when present. If the first
four-byte list word fits inside the payload and is zero, coverage also includes
that terminator. The `LW` at `0x80157FD4` and zero-test at `0x80157FDC` skip
the callback loop for this value. Nonzero records, subsequent list words and
callback targets remain unresolved. Both additions preserve prior field records
and establish neither actual initialization nor auxiliary storage ownership.

Conditional reads of subsequent packed words depend on entity fields whose
producers remain unqualified. Those words, unselected records, gaps and tails
stay unresolved. The rule does not establish complete tables, physical texture
or palette sources, rendering, safe editing or the absence of rasterized text.

## Resident animation field coverage

Coverage recognizes a bounded variant of original US AREA resident animation
fields. Admission requires the original archive identity and a reviewed tuple
of entry index, archive offset, raw mode, load address `0x800D3800`, checkword,
payload length and SHA-256. Supported tuples use entry 4 or entry 10; their archive
offsets vary. Use `bin/harness text coverage --root DIR --strict --json` to inspect the
qualified fields and remaining gaps for a source tree.

AREA030, AREA089 and AREA129 instead hold resident data in entry 10. Their normal
token 14 in program 530 has duration 2 and operand 14, beyond its declared
14-cell directory. The exact lookup at payload `[590,592)` contains relative
offset 1, which selects zero-part cell byte 563. The original lookup and count
readers at `0x8014DBE4..0x8014DC04` and `0x8014C6E8..0x8014C708` qualify
this one alias, conditional on stable source/work/pointers, ordinary helper
returns and noninterference. Both lookup bytes must already be typed ordinary
cell fields, and the zero count must already be typed metadata. The alias adds
no physical bytes or records. Every other metadata alias remains unsupported.

The metadata covers root-relative u32 group offsets, group-relative u16 program
offsets, program count bytes, duration/operand pairs and referenced relative cell
offsets. Original consumers `0x8014D878..0x8014D8AC`, `0x8014D928..0x8014D964`
and `0x8014DBB4..0x8014DC04` qualify these fields. First offsets delimit recognized
directories; they do not store separate counts or establish runtime selector
bounds. Duplicate program slots retain their shared target. Program header byte
0 constrains profile recognition but remains unclassified. Initialization reads
the first token's operand directly. High-duration tokens select one destination
token whose operand is then read directly; the field walk preserves this single
fetch without recursively following controls.

Every directory, program and declared cell must fit its containing extent.
Distinct next pointers bound those extents. Only referenced cell lookup words,
counts and five-byte parts qualify. Unqualified aliases and overlaps between
metadata and cell fields refuse the profile. Metadata forms merged contiguous
intervals; cell counts and each part remain separate records.

Original pointer construction at `0x8014C6D8..0x8014C6F4` binds the relative cell
offsets to the count and five-byte part consumer at `0x8014C6F8..0x8014CD90`.
Each part starts with a control byte, followed by signed X/Y and unsigned U/V.
The control reader at `0x8014C710` extracts bit 7 for drawing-path selection;
`0x8014C7F0..0x8014C818` uses the low four bits as a size-table index. The loop
delay slot at `0x8014CD8C` advances the part pointer by five bytes. Bits 4 through
6, entity state and the selected path remain unqualified. The rectangle helper
at `0x8014CF9C` remains outside the reviewed source. This classification identifies
stored fields; it does not establish rectangle behavior.

Allocation tails, unreferenced offset words and unreferenced cells remain
unresolved. These include AREA024's word `[6836,6838)` and cells
`[5495,5504)` and `[7180,7221)`, and AREA000's word `[6518,6520)` and cell
`[6541,6572)`. AREA001 excludes words `[5252,5254)` and `[11818,11820)`
and cells `[5371,5380)` and `[11841,11872)`. AREA002 excludes words
`[7328,7330)` and `[10906,10908)` and cells `[7447,7456)` and
`[10929,10960)`. Other payload bytes retain their prior status. The rule does not
establish complete program consumption, runtime execution, rendering or a unique
supplier.

The original startup callback at `0x8014EA80` publishes the literal base
`0x800D3800` to `0x801459F0` at `0x8014EBE4`, after FIRST readiness.
FIRST has no entry at that destination; this publication is independent of
its payload pointers. For world 24, the original GAME reload call at
`0x8019FBA8` requests file slot `0x2C3`, which resolves to original AREA024.
Entry 4 is loader slot 5 and declares raw transfer to `0x800D3800`.

That transfer requires bit `0x20` of `0x80146854` to be clear. The staging
function at `0x80162B08` otherwise rejects this mode and ends progression
early. Readiness alone therefore does not prove entry 4 was copied. If the
permitted exact transfer completes, the published pointer stays unchanged and
no replacement intervenes, the payload supplies the root read at `0x8014D880`.
The original file-slot cells for world indices 0 through 199 also bind their
exact AREA archives to the same conditional raw-loader route. Each payload
still requires its own reviewed descriptor tuple and checked field layout.
These bindings establish neither actual selection and consumption nor universal
pointer stability through scene rebuilds.

## Battle resident animation field coverage

Coverage also recognizes selected fields in the original US resident bank loaded
at `0x800F0800`. The raw payload has size 3692, checkword 12 and SHA-256
`2cd03f3e11d86c2de1fdcc3b3c996eece1c02c45957f71a94362ac95e8959934`.
Its reviewed entry-index/archive-offset pairs are `2/153600`, `5/417792`,
`5/419840`, `5/425984`, `5/430080` and `5/442368`. The containing archive must
match the original disc manifest. Each archive also contains the exact common
battle payload described under battle overlay instruction coverage.

The original dispatcher at `0x801E6C84` copies five handler words from
`0x801D0F44..0x801D0F58`, then tests global byte `0x801462E0`. When that byte
is not 5, it writes `0x800F0800` to `0x801459F0` and calls the handler selected
by work byte `+1`. After an ordinary return, it writes `0x800D3800` to that
shared pointer. This replaces the base with a constant rather than restoring
its previous value.

For initialized state 1, handler `0x801E6E54` clears work byte `+0xB` before
calling selector `0x801E8FA8`. The selector masks its input with `0x58`, so
the possible results are 0, 3, 4 and 6. Request table `0x801EB494` maps these
to `0xFF`, 5, 4 and 3. Result 0 takes the no-request guard. The other results
set work byte `+0x24` to `0x80`, group halfword `+0x2C` to 1 and byte `+0x48`
to zero before requesting the program through `0x8014D6B8`. The handler then
advances state 1 to state 3. The continuing handler at `0x801E6FB0` preserves
the selected group and uses the same finite request alternatives while its
completion and countdown guards permit progression.

Original consumers `0x8014D6DC`, `0x8014D910`, `0x8014DB98` and `0x8014D978`
read the selected group, program offsets, step counts, duration/operand pairs
and relative cell words. Group 1 starts at payload offset 1792. Its selected
programs are:

| Program index | Payload offset | Steps | Referenced cells |
| ---: | ---: | ---: | ---: |
| 3 | 2210 | 17 | 16 |
| 4 | 2400 | 12 | 12 |
| 5 | 2572 | 4 | 4 |

Program 3 ends with duration/operand `0x80/1`, which redirects progression to
step 1. The finite forward walks qualify payload intervals `[4,8)`,
`[1798,1804)`, `[2211,2399)`, `[2401,2572)` and `[2573,2619)`. These intervals
contain the fetched fields and counted five-byte cell parts. Program header
byte 0, other roots and programs, allocation gaps and tails retain their prior
status. Part control bits `0x70` still have no qualified meaning.

State 3 calls the progress reader at `0x801E7174` and queue helper at
`0x801E717C`. The latter stores the work pointer in the array at `0x80148214`
only while count byte `0x8014832C` is below 30. Drawing happens later.
With byte `0x8014832E` bit 0 set, `0x8014BB20` restores each queued pointer
to scratch cell `0x1F800044` and calls `0x8014C4D4`. The cell reader adds the
retained work `+0x54` table pointer to the `+0x5A` relative offset. It does
not need the temporary shared base to remain selected. Work byte 0 bit `0x40`
must be clear, and the X/Y coordinate guards must pass before the cell read.

These roles require the exact raw transfers to complete, ordinary helper
returns, valid nonaliasing memory and frames, stable code and tables, and the
same bank and work fields to survive until the queued draw. They establish
conditional stored-field use. They do not establish actual invocation, loading,
residency, image or CLUT bindings, uploads, display, absence of rasterized text
or safe editing.

## Executable geometry table coverage

Coverage recognizes the 32-byte numeric table at `0x8017F53C..0x8017F55C`
(file offsets `[955708,955740)`) in the exact original US `SLUS_004.22`.
The original consumer at `0x8014C7F0..0x8014C818` masks its index to 0–15,
doubles it and reads an unsigned width/height pair. The sixteen pairs combine
widths and heights of 8, 16, 24 and 32 pixels.

This qualifies the stored numeric representation. Runtime use, callee effects,
rectangle behavior, mutable tables and other geometry tables remain unverified.


## Whole-payload recognition

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

Coverage also qualifies the complete in-bounds prefix when later rows are clipped.
It shares mode, size, check-word and geometry checks with the payload filter.
The filter requires every source row to fit before excluding a whole payload.
Original producer `0x80162698..0x80162790` constructs each chunk's coordinates; consumer
`0x80163378..0x80163418` calls `LoadImage` only inside the 1024-by-512-word VRAM
bounds. Coverage stops at the last complete row inside those bounds and leaves
the clipped tail unresolved. Uploads that wrap the consumer's six-bit Y coordinate
remain unresolved. The ranges establish conditional word submission. Completed
loading, hardware transfer, CLUT selection, display and safe edits need separate
qualification.
