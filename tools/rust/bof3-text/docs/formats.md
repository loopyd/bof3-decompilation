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

## Inline pointer coverage

Supported inline layouts declare four-byte pointer words bound to the validated
payload layout. Coverage checks that each word lies inside the payload and stores
the text allocation's little-endian address, then classifies those four bytes as
known nontext. Pointer words and text allocations must not overlap. Other payload
bytes retain their existing classification; this rule does not classify padding
or code or establish runtime rendering.

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

Coverage recognizes the complete 5120-byte palette allocation in original US
`AREA024.EMI` entry 8. It requires the original archive identity, entry offset
737280, raw transfer mode, load address `0x80033E00` and exact payload hash.
The allocation contains 2560 little-endian image words, without a header.

The staging routine at `0x8014E284` copies this span into the image buffer
described by `0x8014E22C`. The original SDK profile at `0x8017CE9C` interprets
the descriptor as image dimensions and consumes words through CPU and DMA paths.
This qualifies the stored representation. Mutable limits, dispatch bindings,
debug callbacks and queue paths remain separate execution questions. The rule
does not establish a full palette upload, sampling, display, unique ownership or
the absence of rasterized text. Other entries and archive gaps stay unchanged.

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
evidence. Selection mode bytes, program header byte zero, unselected programs and selectors,
cell counts and part fields, palettes, tails and opaque aliases remain
unresolved. Actual invocation, cadence, transfer, residency, resource safety,
upload, display, rasterized-text absence and safe editing remain unverified.

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
u16 field at payload bytes `[42,44)`, multiplied by four. The complete original
setter at `0x80155038..0x8015507C` uses the same offset for numeric byte writes.
Coverage includes that two-byte field when present. Initialization and ownership
of the separate plane remain unresolved, and the rule excludes its storage.
Integral alternatives qualify the stored main words without proving neighbor
safety for fractional coordinates, terrain meaning or actual visitation.

Conditional reads of subsequent packed words depend on entity fields whose
producers remain unqualified. Those words, unselected records, gaps and tails
stay unresolved. The rule does not establish complete tables, physical texture
or palette sources, rendering, safe editing or the absence of rasterized text.

## Resident animation field coverage

Coverage recognizes fixed fields in entry 4 of four original US archives. Each
requires its original archive identity, entry offset 198656, raw transfer mode,
load address `0x800D3800` and exact payload hash.

| Archive | Payload bytes | Metadata bytes | Referenced cells | Cell field bytes |
| --- | ---: | ---: | ---: | ---: |
| `AREA024.EMI` | 11368 | 1917 | 354 | 9214 |
| `AREA000.EMI` | 10592 | 1951 | 349 | 8404 |
| `AREA001.EMI` | 11968 | 1808 | 326 | 9906 |
| `AREA002.EMI` | 11104 | 1654 | 300 | 9215 |

The metadata covers root-relative u32 group offsets, group-relative u16 program
offsets, program count bytes, duration/operand pairs and referenced relative cell
offsets. Original consumers `0x8014D878..0x8014D8AC`, `0x8014D928..0x8014D964`
and `0x8014DBB4..0x8014DC04` qualify these fields. First offsets delimit recognized
directories; they do not store separate counts or establish runtime selector
bounds. Duplicate program slots retain their shared target. Program header byte
0 constrains profile recognition but remains unclassified.

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
supplier. The writer of base pointer `0x801459F0` and the loader handoff remain
unqualified; entries at the same load address do not inherit any profile.

## START initializer instruction coverage

Coverage recognizes the complete instruction body at `0x801D15C0..0x801D1740`
in original US `START.EMI#8`. The raw entry must have offset 360448, load address
`0x801D0C00`, checkword 18, size 117986 and SHA-256
`a47d32865283a01ce87b2f2f8cb19d3305525659b95a8feb4f307eddf63a8cb1`.
The containing archive must match the original disc manifest. The classified
payload interval is `[2496,2880)`, or archive bytes `[362944,363328)`.

Original GAME01 registration stores `0x80196F78` in callback slot 0. The SLUS
scheduler passes that field to the typed `OpenTh` entry-function argument. The
alternate-loop index 1, START index 2 and name-entry index 0 conditionally reach
the original `JAL` at `0x801E4678`. The complete initializer has 96 words in 12
structural blocks and seven actual branch or return delay slots. Original and
retained RAM bytes agree. Its two compiled `BREAK` division guards terminate at
unqualified exception edges and have no delay slots.

This classification requires ordinary helper returns, valid thread arguments,
successful unskipped START loading, stable code and selectors, and noninterfering
callback slots. Exiting the current thread can clear the new registration if the
current slot is slot 0. The rule does not establish actual scheduling, initializer
execution, historical initialization, selector bounds, exception-handler behavior
or argument and resource safety. Neighboring code and character records retain
their prior classifications. No matching initializer ELF is claimed.

## START and STATUS category reader instruction coverage

The START entry identified above and original US `STATUS.EMI#0` share the same
117,986-byte payload, load address, checkword and payload hash. STATUS entry 0
must be in raw mode at archive offset 2048. Each containing archive must
independently match the original disc manifest. Coverage classifies these
complete instruction bodies in both entries:

| Reader | Payload interval | Runtime interval | Bytes |
| --- | --- | --- | ---: |
| Items | `[61428,63316)` | `[0x801DFBF4,0x801E0354)` | 1888 |
| Ability | `[63316,64584)` | `[0x801E0354,0x801E0848)` | 1268 |
| Equipment | `[65136,66984)` | `[0x801E0A70,0x801E11A8)` | 1848 |

All 1,251 original words have complete structural control flow, including their
138 control-transfer delay words. Equipment's unsigned selector guard limits
the computed jump at `0x801E0ABC` to six table entries at `0x801D0D94`.
All targets stay within the reviewed body. Separate pointer records cover those
six table cells. Native internal PCs `0x801DFF24`, `0x801E05A0` and `0x801E0E2C`
bind executable words without establishing first-instruction or all-path execution.

The rule requires stable code and table bytes, valid incoming return control and
memory, an aligned intact stack, ordinary returning callees that preserve saved
values, and stores that do not alias code, table or frame slots. It does not
qualify strings, referenced data, callees, selector bounds outside the reviewed
Equipment guard, resource safety or a unique archive supplier. The 552-byte gap
between Ability and Equipment retains its prior status. The separate initializer
classification remains specific to START entry 8.

## Stat-label instruction coverage

The same exact START and STATUS pins qualify two complete instruction bodies:

| Body | Payload interval | Runtime interval | Bytes |
| --- | --- | --- | ---: |
| Four-label reader | `[47948,48876)` | `[0x801DC74C,0x801DCAEC)` | 928 |
| Wrapper | `[57904,58088)` | `[0x801DEE30,0x801DEEE8)` | 184 |

All 278 original words, 29 control-transfer delays, 35 loads, complete structural
flow and saved frames reconcile with retained RAM. The wrapper's original call
at `0x801DEEC8` conditionally enters the reader. Its four renderer calls consume
the labels at `0x801EC99C`, `0x801EC9A2`, `0x801EC9A8` and `0x801EC9AE`.
The wrapper's incoming record and flag producer remain unidentified. This
classification establishes stored instructions; invocation and an ordinary menu
route remain unverified.

The category readers' memory, frame, nonaliasing, ordinary-return and stable-code
conditions also apply. Referenced data, external callees and the neighboring
three-label entry at `0x801DEEE8` retain their prior classifications. No matching
C/ELF, all-path feasibility, general argument bounds, transitive callee effects,
rendering, safe editing or unique archive supplier is established.

## Config instruction coverage

The same exact START and STATUS entry pins admit these complete instruction
bodies:

| Body | Payload interval | Runtime interval | Bytes |
| --- | --- | --- | ---: |
| Phase dispatcher | `[68224,68284)` | `[0x801E1680,0x801E16BC)` | 60 |
| Phase 1 | `[68816,69664)` | `[0x801E18D0,0x801E1C20)` | 848 |
| Phase 2 | `[69664,70764)` | `[0x801E1C20,0x801E206C)` | 1100 |
| Direct callee at `0x801E2668` | `[72296,72360)` | `[0x801E2668,0x801E26A8)` | 64 |
| Settings caller | `[72360,72856)` | `[0x801E26A8,0x801E2898)` | 496 |
| Direct callee at `0x801E2898` | `[72856,73728)` | `[0x801E2898,0x801E2C00)` | 872 |
| Settings reader | `[73728,74116)` | `[0x801E2C00,0x801E2D84)` | 388 |

All words have complete structural control flow, including branch, call and
return delays. Phase-entry observations have return address `0x801E16AC`,
matching the dispatcher's indirect call at `0x801E16A4`. The selected callee must
return normally; general phase bounds remain unverified. Phase 2's unsigned
focus-minus-two guard bounds the computed jump at `0x801E1CD8` to six original
table entries at `0x801D0DC8`. Every target is inside the reviewed body.
Separate pointer records cover those six cells.

Original phase-body calls and observed internal PC `0x801E2854` bind the settings
caller. Reader entries have return address `0x801E285C`, matching that call and
its `A1 = T0` delay word. The category readers' memory, frame, nonaliasing,
ordinary-return and stable-code/table conditions also apply.

The original phase calls at `0x801E1960`, `0x801E1AD0` and `0x801E1CA8`
conditionally enter the 64-byte callee. The settings caller's `0x801E280C` call
and `A0 = S0` delay word bind the 872-byte callee. Both complete bodies restore
their frames and return through their own `JR RA` delays. All 234 words and
fourteen control-transfer delays have complete structural flow; the larger body
has eleven `LWL`/`LWR` merge-forwarding pairs. Original and retained RAM bytes
agree. Invocation of these two callees remains unobserved.

Each instruction record covers its own complete body. Referenced data, external
callees and neighbors remain outside that record. No all-path feasibility,
general selector/count bounds, transitive callee effects, additional rendering,
safe editing, C/ELF match or unique archive supplier is established.

## Menu table and record coverage

The same exact START and STATUS pins qualify these numeric stored fields:

| Field | Payload interval | Bytes |
| --- | --- | ---: |
| Equipment jump targets | `[404,428)` | 24 |
| Config phase-2 jump targets | `[456,480)` | 24 |
| Config phase-1/phase-2 handler pointers | `[114920,114928)` | 8 |
| Settings record-start indices | `[115116,115122)` | 6 |
| Settings record counts | `[115124,115130)` | 6 |
| Glyph limit and signed horizontal offset | `[114980+8*i,114982+8*i)`, `i=0..14` | 30 |

The two reviewed unsigned guards bound their six-cell jump tables. Only the
observed phase-1 and phase-2 entry links qualify the two handler cells; other
phase cells remain unresolved. Settings groups 0 through 5 read start indices
`0,3,7,11,13,15` and counts `3,4,4,2,2,0`. The sixth group skips record loads.
The reader uses each selected record's first byte as the glyph limit and
sign-extends its second byte before horizontal centering. Retained original
draw arguments agree with all fifteen count/position pairs.

Stable code, maps and records, valid memory and frames, nonaliasing writes and
ordinary returns remain conditions. These records classify numeric storage.
The unconsumed pairs `[115122,115124)` and `[115130,115132)` remain unresolved;
their role is unproven. Record text and tails, Manual/Auto headers, neighboring
cells, general bounds, retired loads, glyph reads, additional rendering and
safe editing remain outside this proof.

## Menu description instruction coverage

The same exact START and STATUS entry pins admit the complete description reader
at `0x801E9BEC..0x801E9D18`, payload interval `[102380,102680)`. All 75 original
words and ten control-transfer delay words reconcile with native entry, lookup
and renderer observations. Its five direct calls and one indirect linking call
require ordinary returns. The index into the indirect-call table at `0x801ED420`
has no in-body guard. Table bytes, referenced data and callees remain outside
this classification. The category readers' memory, frame, nonaliasing and
stable-code conditions also apply.

The exact original executable identified in
[SLUS instruction coverage](#slus-instruction-coverage) also admits the complete
52-byte lookup leaf at `0x801502D0..0x80150304`, file interval `[762576,762628)`.
Its 13 words contain two loads and one return with its delay slot, without calls,
stores or stack changes. Original words match native entry and return captures.
Classification requires stable code and descriptor/table bytes, valid readable
memory and incoming return control. Neither body establishes arbitrary-ID bounds,
general pointer safety, edited rendering or a unique supplying archive.

Ordinary SQUARE entry from a loaded inn scene reaches GAME call `0x80197628`,
whose delay slot supplies loader slot `0x269`. Original executable table
`0x80182444[617]` resolves that slot to LBA `62179`, the start of `STATUS.EMI`.
Linked native observations match the slot, resolved LBA and complete 2048-byte
STATUS header. Entry 0 selects 117,986 raw bytes for destination `0x801D0C00`.
Its mode halfword at archive header offset `0x1C` is zero; the separate checkword
at `0x18` is `0x12`.

Between the first raw-payload call at `0x80162C4C` and menu-ready continuation
`0x80197650`, native RAM history records CD DMA commits of 20 and 280 bytes
within the reader. Together they overwrite all 300 reader bytes and match the
original body. The initial bytes already match, so committed writes establish
the overwrite. Subsequent ordinary Ability selection displays the edited Heal
description, and the reader's lookup return at `0x801E9C6C` binds record ID
`0x40D0` to `0x8001C9F7` on this same lineage. These observations support the
selected STATUS reader transfer for this transition. They do not cover the
complete entry or per-sector CD lifecycle, identify suppliers for earlier
independent captures, or identify the description-bank supplier.

## SHOP selector instruction coverage

Coverage recognizes these complete bodies in original US `SHOP.EMI#0`.
The containing archive must match the original disc manifest. Entry 0 must
use raw mode 0, archive offset 2048, load address `0x801D0C00`, checkword
`0x13`, size 87660 and payload SHA-256
`b9b26c0d387ebbbaa211ed949806f76f5a3cc6ae891df39793ef12335c158219`.

| Payload interval | Runtime interval | Bytes |
| --- | --- | ---: |
| `[672,732)` | `[0x801D0EA0,0x801D0EDC)` | 60 |
| `[1252,1312)` | `[0x801D10E4,0x801D1120)` | 60 |
| `[1728,1788)` | `[0x801D12C0,0x801D12FC)` | 60 |
| `[1788,2224)` | `[0x801D12FC,0x801D14B0)` | 436 |
| `[7148,7208)` | `[0x801D27EC,0x801D2828)` | 60 |
| `[7376,8548)` | `[0x801D28D0,0x801D2D64)` | 1172 |

The three selected little-endian function-address words at `0x801E514C`,
`0x801E5174` and `0x801E518C` also qualify as known nontext. For selector
bytes 2, 1 and 0, they select `0x801D10E4`, `0x801D12C0` and
`0x801D12FC`, respectively.

For phase 3, the additional cell at `0x801E517C` selects `0x801D27EC`.
That dispatcher selects `0x801D28D0` through cell `0x801E51B4` for
substep 1. Both four-byte little-endian function-address words also qualify
as known nontext.

Complete original-word and structural control-flow review covers every
instruction and delay slot. All admitted bodies and table cells match retained
RAM. Ordinary callee returns, preserved frames, valid nonaliased memory and
stable code and tables remain conditions. Other table cells, neighboring bodies
and referenced data retain their prior classifications. General selector bounds,
callee behavior, actual execution, safe editing and runtime text acceptance
remain unqualified.

The exact original executable identified under
[SLUS instruction coverage](#slus-instruction-coverage) also qualifies
`0x8014E640..0x8014E6D0`, file interval `[755264,755408)`.
Original SHOP call `0x801D137C` conditionally enters this call-free body.
Review covers all 36 original words and every branch, jump and return delay
slot. Referenced input and repeat-state fields remain outside the classification.
Physical button mapping and input cadence remain separate requirements.

## Shared battle instruction coverage

Coverage recognizes the following complete instruction bodies in original US raw
payloads loaded at `0x80096800`, with size 133316 and SHA-256
`77d963c56e3ba6b1619323e3007c25d70300e855ac9f4b088ddce2604f92e140`.
The containing archive must also match the original disc manifest. Entry indices
and physical offsets may differ between archives.

| Payload interval | Runtime interval | Bytes |
| --- | --- | ---: |
| `[0x2B0,0x2E8)` | `[0x80096AB0,0x80096AE8)` | 56 |
| `[0x2E8,0x324)` | `[0x80096AE8,0x80096B24)` | 60 |
| `[0x324,0x37C)` | `[0x80096B24,0x80096B7C)` | 88 |
| `[0x614,0x690)` | `[0x80096E14,0x80096E90)` | 124 |
| `[0x690,0x700)` | `[0x80096E90,0x80096F00)` | 112 |
| `[0x700,0x73C)` | `[0x80096F00,0x80096F3C)` | 60 |
| `[0x73C,0x778)` | `[0x80096F3C,0x80096F78)` | 60 |
| `[0x778,0x7BC)` | `[0x80096F78,0x80096FBC)` | 68 |
| `[0x7BC,0x84C)` | `[0x80096FBC,0x8009704C)` | 144 |
| `[0xDD4,0xE1C)` | `[0x800975D4,0x8009761C)` | 72 |
| `[0xF3C,0xF78)` | `[0x8009773C,0x80097778)` | 60 |
| `[0x14BC,0x151C)` | `[0x80097CBC,0x80097D1C)` | 96 |
| `[0x151C,0x1558)` | `[0x80097D1C,0x80097D58)` | 60 |
| `[0x1558,0x15C0)` | `[0x80097D58,0x80097DC0)` | 104 |
| `[0x16B8,0x16C0)` | `[0x80097EB8,0x80097EC0)` | 8 |
| `[0x18A8,0x18E4)` | `[0x800980A8,0x800980E4)` | 60 |
| `[0x18E4,0x1944)` | `[0x800980E4,0x80098144)` | 96 |
| `[0x1A54,0x1AE4)` | `[0x80098254,0x800982E4)` | 144 |
| `[0x1AE4,0x1B4C)` | `[0x800982E4,0x8009834C)` | 104 |
| `[0x1B4C,0x1B88)` | `[0x8009834C,0x80098388)` | 60 |
| `[0x1B88,0x1BC4)` | `[0x80098388,0x800983C4)` | 60 |
| `[0x1BC4,0x1C08)` | `[0x800983C4,0x80098408)` | 68 |
| `[0x1C08,0x1C50)` | `[0x80098408,0x80098450)` | 72 |
| `[0x21B4,0x21FC)` | `[0x800989B4,0x800989FC)` | 72 |
| `[0x2308,0x2344)` | `[0x80098B08,0x80098B44)` | 60 |
| `[0x2914,0x2978)` | `[0x80099114,0x80099178)` | 100 |
| `[0x2978,0x29B4)` | `[0x80099178,0x800991B4)` | 60 |
| `[0x2B28,0x2B64)` | `[0x80099328,0x80099364)` | 60 |
| `[0x3790,0x37CC)` | `[0x80099F90,0x80099FCC)` | 60 |
| `[0x3D90,0x3DCC)` | `[0x8009A590,0x8009A5CC)` | 60 |
| `[0x477C,0x4784)` | `[0x8009AF7C,0x8009AF84)` | 8 |
| `[0x4A0C,0x4A74)` | `[0x8009B20C,0x8009B274)` | 104 |
| `[0x4A74,0x4AB0)` | `[0x8009B274,0x8009B2B0)` | 60 |
| `[0x4C4C,0x4C88)` | `[0x8009B44C,0x8009B488)` | 60 |
| `[0x4F00,0x4F94)` | `[0x8009B700,0x8009B794)` | 148 |
| `[0x4F94,0x5028)` | `[0x8009B794,0x8009B828)` | 148 |
| `[0x5164,0x51A0)` | `[0x8009B964,0x8009B9A0)` | 60 |
| `[0x527C,0x52CC)` | `[0x8009BA7C,0x8009BACC)` | 80 |
| `[0x5F94,0x6068)` | `[0x8009C794,0x8009C868)` | 212 |
| `[0x6068,0x607C)` | `[0x8009C868,0x8009C87C)` | 20 |
| `[0x607C,0x60AC)` | `[0x8009C87C,0x8009C8AC)` | 48 |
| `[0x67EC,0x6998)` | `[0x8009CFEC,0x8009D198)` | 428 |
| `[0x6998,0x7294)` | `[0x8009D198,0x8009DA94)` | 2300 |
| `[0x7294,0x72B4)` | `[0x8009DA94,0x8009DAB4)` | 32 |
| `[0x746C,0x74C8)` | `[0x8009DC6C,0x8009DCC8)` | 92 |
| `[0x7520,0x7550)` | `[0x8009DD20,0x8009DD50)` | 48 |
| `[0x7550,0x7618)` | `[0x8009DD50,0x8009DE18)` | 200 |
| `[0x7618,0x7650)` | `[0x8009DE18,0x8009DE50)` | 56 |
| `[0x7650,0x7664)` | `[0x8009DE50,0x8009DE64)` | 20 |
| `[0x7664,0x7678)` | `[0x8009DE64,0x8009DE78)` | 20 |
| `[0x7678,0x768C)` | `[0x8009DE78,0x8009DE8C)` | 20 |
| `[0x768C,0x7770)` | `[0x8009DE8C,0x8009DF70)` | 228 |
| `[0x7770,0x77BC)` | `[0x8009DF70,0x8009DFBC)` | 76 |
| `[0x77BC,0x77EC)` | `[0x8009DFBC,0x8009DFEC)` | 48 |
| `[0x77EC,0x781C)` | `[0x8009DFEC,0x8009E01C)` | 48 |
| `[0x781C,0x784C)` | `[0x8009E01C,0x8009E04C)` | 48 |
| `[0x784C,0x787C)` | `[0x8009E04C,0x8009E07C)` | 48 |
| `[0x787C,0x7984)` | `[0x8009E07C,0x8009E184)` | 264 |
| `[0x79E0,0x7A00)` | `[0x8009E1E0,0x8009E200)` | 32 |
| `[0x7A00,0x7A24)` | `[0x8009E200,0x8009E224)` | 36 |
| `[0x7A24,0x7A48)` | `[0x8009E224,0x8009E248)` | 36 |
| `[0x7FC4,0x7FE4)` | `[0x8009E7C4,0x8009E7E4)` | 32 |
| `[0x7FE4,0x8004)` | `[0x8009E7E4,0x8009E804)` | 32 |
| `[0x8004,0x8024)` | `[0x8009E804,0x8009E824)` | 32 |
| `[0x8024,0x8044)` | `[0x8009E824,0x8009E844)` | 32 |
| `[0x818C,0x8230)` | `[0x8009E98C,0x8009EA30)` | 164 |
| `[0x828C,0x82CC)` | `[0x8009EA8C,0x8009EACC)` | 64 |
| `[0x82CC,0x8328)` | `[0x8009EACC,0x8009EB28)` | 92 |
| `[0x86CC,0x8708)` | `[0x8009EECC,0x8009EF08)` | 60 |
| `[0x8708,0x8728)` | `[0x8009EF08,0x8009EF28)` | 32 |
| `[0x87B8,0x8810)` | `[0x8009EFB8,0x8009F010)` | 88 |
| `[0x8A58,0x8A78)` | `[0x8009F258,0x8009F278)` | 32 |
| `[0x8A78,0x8AB0)` | `[0x8009F278,0x8009F2B0)` | 56 |
| `[0x8AB0,0x8AE8)` | `[0x8009F2B0,0x8009F2E8)` | 56 |
| `[0x8AE8,0x8B20)` | `[0x8009F2E8,0x8009F320)` | 56 |
| `[0x8B20,0x8BDC)` | `[0x8009F320,0x8009F3DC)` | 188 |
| `[0x8BDC,0x8C14)` | `[0x8009F3DC,0x8009F414)` | 56 |
| `[0x8C14,0x8C4C)` | `[0x8009F414,0x8009F44C)` | 56 |
| `[0x8C4C,0x8C84)` | `[0x8009F44C,0x8009F484)` | 56 |
| `[0x8C84,0x8CBC)` | `[0x8009F484,0x8009F4BC)` | 56 |
| `[0x8CBC,0x8CF4)` | `[0x8009F4BC,0x8009F4F4)` | 56 |
| `[0x8CF4,0x8D2C)` | `[0x8009F4F4,0x8009F52C)` | 56 |
| `[0x8D2C,0x8D64)` | `[0x8009F52C,0x8009F564)` | 56 |
| `[0x8D64,0x8D9C)` | `[0x8009F564,0x8009F59C)` | 56 |
| `[0x8D9C,0x8DD4)` | `[0x8009F59C,0x8009F5D4)` | 56 |
| `[0x8DD4,0x8E0C)` | `[0x8009F5D4,0x8009F60C)` | 56 |
| `[0x8E0C,0x8E44)` | `[0x8009F60C,0x8009F644)` | 56 |
| `[0x8E44,0x8E7C)` | `[0x8009F644,0x8009F67C)` | 56 |
| `[0x8E7C,0x8EB4)` | `[0x8009F67C,0x8009F6B4)` | 56 |
| `[0x8EB4,0x8EEC)` | `[0x8009F6B4,0x8009F6EC)` | 56 |
| `[0x8EEC,0x8F58)` | `[0x8009F6EC,0x8009F758)` | 108 |
| `[0x8FEC,0x9014)` | `[0x8009F7EC,0x8009F814)` | 40 |
| `[0x9014,0x903C)` | `[0x8009F814,0x8009F83C)` | 40 |
| `[0x903C,0x9064)` | `[0x8009F83C,0x8009F864)` | 40 |
| `[0x9064,0x9084)` | `[0x8009F864,0x8009F884)` | 32 |
| `[0x9178,0x9198)` | `[0x8009F978,0x8009F998)` | 32 |
| `[0x9198,0x91B8)` | `[0x8009F998,0x8009F9B8)` | 32 |
| `[0x91B8,0x91E0)` | `[0x8009F9B8,0x8009F9E0)` | 40 |
| `[0x91E0,0x9220)` | `[0x8009F9E0,0x8009FA20)` | 64 |
| `[0x92E4,0x9338)` | `[0x8009FAE4,0x8009FB38)` | 84 |
| `[0x9610,0x9670)` | `[0x8009FE10,0x8009FE70)` | 96 |
| `[0x9670,0x9684)` | `[0x8009FE70,0x8009FE84)` | 20 |
| `[0x9684,0x9698)` | `[0x8009FE84,0x8009FE98)` | 20 |
| `[0x9698,0x96AC)` | `[0x8009FE98,0x8009FEAC)` | 20 |
| `[0x96AC,0x96D0)` | `[0x8009FEAC,0x8009FED0)` | 36 |
| `[0x96D0,0x96F4)` | `[0x8009FED0,0x8009FEF4)` | 36 |
| `[0x96F4,0x977C)` | `[0x8009FEF4,0x8009FF7C)` | 136 |
| `[0x977C,0x97B4)` | `[0x8009FF7C,0x8009FFB4)` | 56 |
| `[0x97B4,0x97E4)` | `[0x8009FFB4,0x8009FFE4)` | 48 |
| `[0x97E4,0x97F8)` | `[0x8009FFE4,0x8009FFF8)` | 20 |
| `[0x97F8,0x9800)` | `[0x8009FFF8,0x800A0000)` | 8 |
| `[0x9800,0x9988)` | `[0x800A0000,0x800A0188)` | 392 |
| `[0x9A44,0x9AE8)` | `[0x800A0244,0x800A02E8)` | 164 |
| `[0x9AE8,0x9B78)` | `[0x800A02E8,0x800A0378)` | 144 |
| `[0x9B78,0x9C2C)` | `[0x800A0378,0x800A042C)` | 180 |
| `[0x9FD0,0xA098)` | `[0x800A07D0,0x800A0898)` | 200 |
| `[0xA204,0xA240)` | `[0x800A0A04,0x800A0A40)` | 60 |
| `[0xA240,0xA32C)` | `[0x800A0A40,0x800A0B2C)` | 236 |
| `[0xA32C,0xA3F4)` | `[0x800A0B2C,0x800A0BF4)` | 200 |
| `[0xA3F4,0xA444)` | `[0x800A0BF4,0x800A0C44)` | 80 |
| `[0xA444,0xA468)` | `[0x800A0C44,0x800A0C68)` | 36 |
| `[0xA668,0xA6C4)` | `[0x800A0E68,0x800A0EC4)` | 92 |
| `[0xAC14,0xACB4)` | `[0x800A1414,0x800A14B4)` | 160 |
| `[0xACB4,0xADDC)` | `[0x800A14B4,0x800A15DC)` | 296 |
| `[0xADDC,0xAF04)` | `[0x800A15DC,0x800A1704)` | 296 |
| `[0xAF04,0xAFA4)` | `[0x800A1704,0x800A17A4)` | 160 |
| `[0xAFA4,0xB178)` | `[0x800A17A4,0x800A1978)` | 468 |
| `[0xB230,0xB2D0)` | `[0x800A1A30,0x800A1AD0)` | 160 |
| `[0xB338,0xB3E4)` | `[0x800A1B38,0x800A1BE4)` | 172 |
| `[0xB3E4,0xB4B4)` | `[0x800A1BE4,0x800A1CB4)` | 208 |
| `[0xB4B4,0xB584)` | `[0x800A1CB4,0x800A1D84)` | 208 |
| `[0xB920,0xB9D0)` | `[0x800A2120,0x800A21D0)` | 176 |
| `[0xB9D0,0xBACC)` | `[0x800A21D0,0x800A22CC)` | 252 |
| `[0xBBCC,0xBC04)` | `[0x800A23CC,0x800A2404)` | 56 |
| `[0xBC04,0xBC3C)` | `[0x800A2404,0x800A243C)` | 56 |
| `[0xBC3C,0xBD0C)` | `[0x800A243C,0x800A250C)` | 208 |
| `[0xBD0C,0xBD60)` | `[0x800A250C,0x800A2560)` | 84 |
| `[0xBD60,0xBD98)` | `[0x800A2560,0x800A2598)` | 56 |
| `[0xC080,0xC2E0)` | `[0x800A2880,0x800A2AE0)` | 608 |
| `[0xC2E0,0xC570)` | `[0x800A2AE0,0x800A2D70)` | 656 |
| `[0xC570,0xC6F0)` | `[0x800A2D70,0x800A2EF0)` | 384 |
| `[0xC6F0,0xC860)` | `[0x800A2EF0,0x800A3060)` | 368 |
| `[0xC860,0xC8E8)` | `[0x800A3060,0x800A30E8)` | 136 |
| `[0xCE38,0xCE88)` | `[0x800A3638,0x800A3688)` | 80 |
| `[0xCE88,0xCEBC)` | `[0x800A3688,0x800A36BC)` | 52 |
| `[0xCEBC,0xCEF0)` | `[0x800A36BC,0x800A36F0)` | 52 |
| `[0xCEF0,0xD1B4)` | `[0x800A36F0,0x800A39B4)` | 708 |
| `[0xD1B4,0xD210)` | `[0x800A39B4,0x800A3A10)` | 92 |
| `[0xD210,0xD36C)` | `[0x800A3A10,0x800A3B6C)` | 348 |
| `[0xD960,0xD9D8)` | `[0x800A4160,0x800A41D8)` | 120 |
| `[0xD9D8,0xDA38)` | `[0x800A41D8,0x800A4238)` | 96 |
| `[0xDA38,0xDA94)` | `[0x800A4238,0x800A4294)` | 92 |
| `[0xDE88,0xDEC4)` | `[0x800A4688,0x800A46C4)` | 60 |
| `[0xDEC4,0xDF00)` | `[0x800A46C4,0x800A4700)` | 60 |
| `[0xDF00,0xDF44)` | `[0x800A4700,0x800A4744)` | 68 |
| `[0xE100,0xE13C)` | `[0x800A4900,0x800A493C)` | 60 |
| `[0xE2D0,0xE308)` | `[0x800A4AD0,0x800A4B08)` | 56 |
| `[0xE308,0xE32C)` | `[0x800A4B08,0x800A4B2C)` | 36 |
| `[0xE3B4,0xE408)` | `[0x800A4BB4,0x800A4C08)` | 84 |
| `[0xE408,0xE444)` | `[0x800A4C08,0x800A4C44)` | 60 |
| `[0xE444,0xE470)` | `[0x800A4C44,0x800A4C70)` | 44 |
| `[0xE7D8,0xE8B8)` | `[0x800A4FD8,0x800A50B8)` | 224 |
| `[0xE8B8,0xE920)` | `[0x800A50B8,0x800A5120)` | 104 |
| `[0xE920,0xE964)` | `[0x800A5120,0x800A5164)` | 68 |
| `[0xE964,0xEA44)` | `[0x800A5164,0x800A5244)` | 224 |
| `[0xEAC4,0xEB00)` | `[0x800A52C4,0x800A5300)` | 60 |
| `[0xEB00,0xEB30)` | `[0x800A5300,0x800A5330)` | 48 |
| `[0xF01C,0xF074)` | `[0x800A581C,0x800A5874)` | 88 |
| `[0xF1AC,0xF1E8)` | `[0x800A59AC,0x800A59E8)` | 60 |
| `[0xF1E8,0xF218)` | `[0x800A59E8,0x800A5A18)` | 48 |
| `[0xF500,0xF5E0)` | `[0x800A5D00,0x800A5DE0)` | 224 |
| `[0xF5E0,0xF654)` | `[0x800A5DE0,0x800A5E54)` | 116 |
| `[0xF654,0xF698)` | `[0x800A5E54,0x800A5E98)` | 68 |
| `[0xF7F0,0xF82C)` | `[0x800A5FF0,0x800A602C)` | 60 |
| `[0xF82C,0xF8A8)` | `[0x800A602C,0x800A60A8)` | 124 |
| `[0xF8A8,0xF8DC)` | `[0x800A60A8,0x800A60DC)` | 52 |
| `[0xFC88,0xFDBC)` | `[0x800A6488,0x800A65BC)` | 308 |
| `[0x10070,0x100A4)` | `[0x800A6870,0x800A68A4)` | 52 |
| `[0x100A4,0x1042C)` | `[0x800A68A4,0x800A6C2C)` | 904 |
| `[0x1042C,0x104F8)` | `[0x800A6C2C,0x800A6CF8)` | 204 |
| `[0x104F8,0x10584)` | `[0x800A6CF8,0x800A6D84)` | 140 |
| `[0x10584,0x10614)` | `[0x800A6D84,0x800A6E14)` | 144 |
| `[0x10E48,0x10E94)` | `[0x800A7648,0x800A7694)` | 76 |
| `[0x10E94,0x110B8)` | `[0x800A7694,0x800A78B8)` | 548 |
| `[0x11AA0,0x11AF8)` | `[0x800A82A0,0x800A82F8)` | 88 |
| `[0x11AF8,0x11B60)` | `[0x800A82F8,0x800A8360)` | 104 |
| `[0x11BF8,0x11C50)` | `[0x800A83F8,0x800A8450)` | 88 |
| `[0x11C50,0x11CFC)` | `[0x800A8450,0x800A84FC)` | 172 |
| `[0x11CFC,0x11D40)` | `[0x800A84FC,0x800A8540)` | 68 |
| `[0x1253C,0x12580)` | `[0x800A8D3C,0x800A8D80)` | 68 |
| `[0x131AC,0x13284)` | `[0x800A99AC,0x800A9A84)` | 216 |
| `[0x14074,0x140A4)` | `[0x800AA874,0x800AA8A4)` | 48 |
| `[0x140A4,0x140FC)` | `[0x800AA8A4,0x800AA8FC)` | 88 |
| `[0x14274,0x14488)` | `[0x800AAA74,0x800AAC88)` | 532 |
| `[0x14C70,0x14CBC)` | `[0x800AB470,0x800AB4BC)` | 76 |
| `[0x14CBC,0x14CF8)` | `[0x800AB4BC,0x800AB4F8)` | 60 |
| `[0x14F60,0x15034)` | `[0x800AB760,0x800AB834)` | 212 |
| `[0x153A4,0x153E0)` | `[0x800ABBA4,0x800ABBE0)` | 60 |
| `[0x153E0,0x15428)` | `[0x800ABBE0,0x800ABC28)` | 72 |
| `[0x15428,0x15430)` | `[0x800ABC28,0x800ABC30)` | 8 |
| `[0x15430,0x154E0)` | `[0x800ABC30,0x800ABCE0)` | 176 |
| `[0x1575C,0x1581C)` | `[0x800ABF5C,0x800AC01C)` | 192 |
| `[0x1581C,0x15A18)` | `[0x800AC01C,0x800AC218)` | 508 |
| `[0x15F04,0x15F54)` | `[0x800AC704,0x800AC754)` | 80 |
| `[0x15F54,0x15FD0)` | `[0x800AC754,0x800AC7D0)` | 124 |
| `[0x15FD0,0x16044)` | `[0x800AC7D0,0x800AC844)` | 116 |
| `[0x16044,0x16070)` | `[0x800AC844,0x800AC870)` | 44 |
| `[0x16070,0x16134)` | `[0x800AC870,0x800AC934)` | 196 |
| `[0x16134,0x16164)` | `[0x800AC934,0x800AC964)` | 48 |
| `[0x16164,0x16780)` | `[0x800AC964,0x800ACF80)` | 1564 |
| `[0x16780,0x167F4)` | `[0x800ACF80,0x800ACFF4)` | 116 |
| `[0x167F4,0x16844)` | `[0x800ACFF4,0x800AD044)` | 80 |
| `[0x16844,0x16874)` | `[0x800AD044,0x800AD074)` | 48 |
| `[0x16874,0x168A8)` | `[0x800AD074,0x800AD0A8)` | 52 |
| `[0x168A8,0x16A6C)` | `[0x800AD0A8,0x800AD26C)` | 452 |
| `[0x16A6C,0x16AEC)` | `[0x800AD26C,0x800AD2EC)` | 128 |
| `[0x16AEC,0x16C34)` | `[0x800AD2EC,0x800AD434)` | 328 |
| `[0x16C34,0x16CD4)` | `[0x800AD434,0x800AD4D4)` | 160 |
| `[0x16CD4,0x16DC8)` | `[0x800AD4D4,0x800AD5C8)` | 244 |
| `[0x16DC8,0x16E7C)` | `[0x800AD5C8,0x800AD67C)` | 180 |
| `[0x16E7C,0x16E9C)` | `[0x800AD67C,0x800AD69C)` | 32 |
| `[0x16E9C,0x16F1C)` | `[0x800AD69C,0x800AD71C)` | 128 |
| `[0x16F1C,0x17008)` | `[0x800AD71C,0x800AD808)` | 236 |
| `[0x17008,0x170D8)` | `[0x800AD808,0x800AD8D8)` | 208 |
| `[0x170D8,0x17130)` | `[0x800AD8D8,0x800AD930)` | 88 |
| `[0x17130,0x171AC)` | `[0x800AD930,0x800AD9AC)` | 124 |
| `[0x171AC,0x171CC)` | `[0x800AD9AC,0x800AD9CC)` | 32 |
| `[0x171CC,0x1724C)` | `[0x800AD9CC,0x800ADA4C)` | 128 |
| `[0x1724C,0x17374)` | `[0x800ADA4C,0x800ADB74)` | 296 |
| `[0x17374,0x174A4)` | `[0x800ADB74,0x800ADCA4)` | 304 |
| `[0x174A4,0x174C4)` | `[0x800ADCA4,0x800ADCC4)` | 32 |
| `[0x174C4,0x17544)` | `[0x800ADCC4,0x800ADD44)` | 128 |
| `[0x17544,0x1766C)` | `[0x800ADD44,0x800ADE6C)` | 296 |
| `[0x1766C,0x1779C)` | `[0x800ADE6C,0x800ADF9C)` | 304 |
| `[0x1779C,0x177BC)` | `[0x800ADF9C,0x800ADFBC)` | 32 |
| `[0x177BC,0x17814)` | `[0x800ADFBC,0x800AE014)` | 88 |
| `[0x17814,0x1786C)` | `[0x800AE014,0x800AE06C)` | 88 |
| `[0x1786C,0x1789C)` | `[0x800AE06C,0x800AE09C)` | 48 |
| `[0x1789C,0x17908)` | `[0x800AE09C,0x800AE108)` | 108 |
| `[0x17908,0x179F8)` | `[0x800AE108,0x800AE1F8)` | 240 |
| `[0x179F8,0x17A60)` | `[0x800AE1F8,0x800AE260)` | 104 |
| `[0x17B4C,0x17B90)` | `[0x800AE34C,0x800AE390)` | 68 |
| `[0x17B90,0x17BC0)` | `[0x800AE390,0x800AE3C0)` | 48 |
| `[0x17C38,0x17C74)` | `[0x800AE438,0x800AE474)` | 60 |
| `[0x17CD0,0x17D74)` | `[0x800AE4D0,0x800AE574)` | 164 |
| `[0x17EC8,0x17F28)` | `[0x800AE6C8,0x800AE728)` | 96 |
| `[0x17F88,0x1802C)` | `[0x800AE788,0x800AE82C)` | 164 |
| `[0x181C4,0x1855C)` | `[0x800AE9C4,0x800AED5C)` | 920 |
| `[0x18E6C,0x18EB8)` | `[0x800AF66C,0x800AF6B8)` | 76 |
| `[0x18F20,0x18F64)` | `[0x800AF720,0x800AF764)` | 68 |
| `[0x19454,0x19498)` | `[0x800AFC54,0x800AFC98)` | 68 |
| `[0x19798,0x19858)` | `[0x800AFF98,0x800B0058)` | 192 |
| `[0x19858,0x19918)` | `[0x800B0058,0x800B0118)` | 192 |
| `[0x19918,0x19980)` | `[0x800B0118,0x800B0180)` | 104 |
| `[0x19980,0x199F0)` | `[0x800B0180,0x800B01F0)` | 112 |
| `[0x199F0,0x19A50)` | `[0x800B01F0,0x800B0250)` | 96 |
| `[0x19B98,0x19C10)` | `[0x800B0398,0x800B0410)` | 120 |
| `[0x1A1CC,0x1A22C)` | `[0x800B09CC,0x800B0A2C)` | 96 |
| `[0x1A22C,0x1A254)` | `[0x800B0A2C,0x800B0A54)` | 40 |
| `[0x1A254,0x1A2B0)` | `[0x800B0A54,0x800B0AB0)` | 92 |
| `[0x1A2B0,0x1A30C)` | `[0x800B0AB0,0x800B0B0C)` | 92 |
| `[0x1A4E4,0x1A554)` | `[0x800B0CE4,0x800B0D54)` | 112 |
| `[0x1A624,0x1A66C)` | `[0x800B0E24,0x800B0E6C)` | 72 |
| `[0x1A6C4,0x1A730)` | `[0x800B0EC4,0x800B0F30)` | 108 |
| `[0x1A730,0x1A79C)` | `[0x800B0F30,0x800B0F9C)` | 108 |
| `[0x1A79C,0x1AB8C)` | `[0x800B0F9C,0x800B138C)` | 1008 |
| `[0x1AB8C,0x1ABEC)` | `[0x800B138C,0x800B13EC)` | 96 |
| `[0x1ACBC,0x1AD04)` | `[0x800B14BC,0x800B1504)` | 72 |
| `[0x1AD5C,0x1ADBC)` | `[0x800B155C,0x800B15BC)` | 96 |
| `[0x1AE64,0x1AED0)` | `[0x800B1664,0x800B16D0)` | 108 |
| `[0x1B274,0x1B5AC)` | `[0x800B1A74,0x800B1DAC)` | 824 |
| `[0x1B5AC,0x1B704)` | `[0x800B1DAC,0x800B1F04)` | 344 |
| `[0x1B704,0x1B888)` | `[0x800B1F04,0x800B2088)` | 388 |
| `[0x1B888,0x1B8CC)` | `[0x800B2088,0x800B20CC)` | 68 |
| `[0x1B8CC,0x1B8D4)` | `[0x800B20CC,0x800B20D4)` | 8 |
| `[0x1B904,0x1B958)` | `[0x800B2104,0x800B2158)` | 84 |
| `[0x1B958,0x1B998)` | `[0x800B2158,0x800B2198)` | 64 |
| `[0x1B998,0x1B9D8)` | `[0x800B2198,0x800B21D8)` | 64 |
| `[0x1B9D8,0x1BA18)` | `[0x800B21D8,0x800B2218)` | 64 |
| `[0x1BA58,0x1BAAC)` | `[0x800B2258,0x800B22AC)` | 84 |
| `[0x1BAEC,0x1BB2C)` | `[0x800B22EC,0x800B232C)` | 64 |
| `[0x1BB2C,0x1BBB8)` | `[0x800B232C,0x800B23B8)` | 140 |
| `[0x1BC38,0x1BC78)` | `[0x800B2438,0x800B2478)` | 64 |
| `[0x1BC78,0x1BCCC)` | `[0x800B2478,0x800B24CC)` | 84 |
| `[0x1BCCC,0x1BD0C)` | `[0x800B24CC,0x800B250C)` | 64 |
| `[0x1BD8C,0x1BDC0)` | `[0x800B258C,0x800B25C0)` | 52 |
| `[0x1CA6C,0x1CB24)` | `[0x800B326C,0x800B3324)` | 184 |

The bodies at `0x8009CFEC..0x8009D198`, `0x8009DA94..0x8009DAB4`
and `0x8009DC6C..0x8009DCC8` have complete original control-flow reviews.
Their sized executable ELF sections match original bytes and captured resident
RAM, with all text relocations reconciled. Call continuations assume ordinary
returns. This proves stored instruction roles; work records, selectors, arguments,
callees and resource safety remain unqualified. The neighboring body beginning
at `0x8009DCC8` remains excluded because its linked bytes differ from the original.

The 32 listed bodies within `0x8009DD20..0x8009F414` contain 575 instruction
words in 92 structural blocks. Their complete sized executable ELF function
sections and all 197 object text relocations reconcile with original bytes and
captured resident RAM. Structural review covers every word, branch and delay
slot. Direct-call continuations assume ordinary returns and intact caller frames.
Intervening gaps and referenced data remain excluded. Actual execution,
work-record, selector and argument validity, callee behavior and resource safety
remain unqualified.

The 23 listed bodies within `0x80096800..0x8009CFEC` contain 584 instruction
words in 72 structural blocks. Their complete sized executable ELF function
sections and all 222 object text relocations reconcile with original bytes and
captured resident RAM. Structural review covers every word and all 63 actual
control-transfer delay slots. Direct and indirect-call continuations assume
ordinary returns and intact caller frames. The three `JALR v0` wrappers leave
target validity, table bounds and callee behavior unqualified. Intervening gaps,
refused bodies and referenced data remain excluded. This classification does not
establish execution, feasible paths, selector or argument validity, valid work
records or resource safety.

The 32 listed bodies within `0x8009F414..0x8009FFE4` contain 427 instruction
words in 60 structural blocks. Their complete sized executable ELF function
sections and all 156 object text relocations reconcile with original bytes and
captured resident RAM. Every word and delay slot has a structural review.
Direct-call continuations assume ordinary returns and intact caller frames,
including calls to the excluded body at `0x8009DCC8`. Four intervening gaps and
referenced data remain excluded. Callee behavior and actual execution remain
unqualified.

The 22 listed bodies within `0x8009FFE4..0x800AC754` contain 2228 instruction
words in 298 structural blocks. Their complete sized executable ELF function
sections and all 628 object text relocations reconcile with original bytes and
captured resident RAM. Structural review covers every word and each actual
control-transfer delay slot. Direct-call continuations assume ordinary returns
and intact caller frames. The body at `0x800AB760..0x800AB834` includes compiled
`BREAK` instructions at `0x800AB7E4` and `0x800AB7FC`. These have no delay slots;
their exceptional paths terminate at unqualified CPU-exception edges. Exception
handler behavior and continuation remain unqualified. Intervening gaps, bodies
with unmatched complete extents or bytes, and referenced data remain excluded.
This classification does not establish execution, feasible paths, work-record,
selector or argument validity, callee behavior or resource safety.

The 28 smaller bodies within `0x800A0244..0x800AC01C` contain 1088
instruction words in 172 structural blocks. Their complete sized executable ELF
function sections and all 329 object text relocations reconcile with original
bytes and captured resident RAM. Structural review covers every word and all
132 actual control-transfer delay slots. Direct calls assume ordinary returns,
preserved saved values, intact caller frames and nonaliasing dynamic stores.
Four refused bodies, prior refusals, intervening gaps and referenced data remain
excluded. This classification does not establish execution, feasible paths,
valid work records, selector or argument validity, callee behavior, resource
safety or safe editing.

The 32 shorter bodies within `0x8009773C..0x800AC754` contain 564
instruction words in 105 structural blocks. Their complete sized executable ELF
function sections and all 156 object text relocations reconcile with original
bytes and captured resident RAM. Structural review covers every word and all
90 actual control-transfer delay slots. Calls assume ordinary returns, preserved
saved values, intact caller frames and nonaliasing dynamic stores. Targets,
selectors, table contents and bounds at the fifteen indirect-call sites remain
unqualified. Prior refused bodies, gaps and referenced data remain excluded.
This classification does not establish execution, feasible paths, valid work
records, argument validity, callee behavior, resource safety or safe editing.

The 31 additional bodies within `0x80096AB0..0x800ABC30` contain 352
instruction words in 61 structural blocks. Their complete sized executable ELF
function sections and all 110 object text relocations reconcile with original
bytes and captured resident RAM. Structural review covers every word and all
56 actual control-transfer delay slots. Calls assume ordinary returns, preserved
saved values, intact caller frames and nonaliasing dynamic stores. Targets,
selectors, table contents and bounds at the seven indirect-call sites remain
unqualified. Prior refused bodies, gaps and referenced data remain excluded.
This classification does not establish execution, feasible paths, valid work
records, argument validity, callee behavior, resource safety or safe editing.

The 24 listed bodies within `0x800AC964..0x800B70C4` contain 700 instruction
words in 90 structural blocks. Their complete sized executable ELF function
sections and all 130 object text relocations reconcile with original bytes and
captured resident RAM. Structural review covers every word and all 77 actual
control-transfer delay slots. Direct and indirect-call continuations assume
ordinary returns, intact caller frames and nonaliasing dynamic stores. Targets,
selectors and table bounds at the twelve indirect-call sites remain unqualified.
Eight refused bodies, intervening gaps and referenced data remain excluded.
This classification does not establish execution, feasible paths, valid work
records, argument validity, callee behavior or resource safety.

The seventeen smaller bodies within `0x800AE34C..0x800B25C0` contain 252
instruction words in 44 structural blocks. Their complete sized executable ELF
function sections and all seventeen object text relocations reconcile with
original bytes and captured resident RAM. Structural review covers every word
and all 33 actual control-transfer delay slots. Calls assume ordinary returns,
preserved saved values, intact caller frames and nonaliasing dynamic stores.
Targets, selectors, table contents and bounds at the two indirect-call sites
remain unqualified. Prior refused bodies, missing artifacts, gaps and referenced
data retain their classifications. No execution, feasible-path, argument,
work-record, resource-safety or editable-allocation claim follows.

The body at `0x800AE9C4..0x800AED5C` contains 230 instruction words.
Complete original ASM, word and control-flow reviews reconcile with captured
resident bytes, including all fifteen control-transfer delay slots. Qualified
original callers conditionally enter it at JAL sites `0x800AE55C` and `0x800AE814`.
Call continuations assume ordinary returns, preserved saved values, intact caller
frames and nonaliasing dynamic stores. Its indexed table, pointer validity and
callee behavior remain unqualified. Referenced data and neighboring bodies remain
excluded. No ELF match, observed execution, valid selector or safe-editing claim
follows.

The body at `0x800B0F9C..0x800B138C` contains 252 instruction words.
Complete original ASM, word and control-flow reviews reconcile with captured
resident bytes, including all twenty control-transfer delay slots. Qualified
original callers conditionally enter it at JAL sites `0x800B0ECC` and `0x800B0F38`.
Call continuations assume ordinary returns, preserved saved values, intact caller
frames and nonaliasing dynamic stores. Pointer validity, indexed tables, format
contents, buffer capacity and callee behavior remain unqualified. Referenced data
and neighboring bodies remain excluded. No ELF match, observed execution, feasible
path, valid selector or safe-editing claim follows.

The three complete bodies at `0x800B1A74..0x800B2088` contain 389 instruction
words, with separate boundaries at `0x800B1DAC` and `0x800B1F04`. Original ASM,
word and control-flow reviews reconcile with captured resident bytes, including
all 39 control-transfer delay slots. Qualified original JAL sites `0x800B0FDC`,
`0x800B11DC` and `0x800B131C` conditionally enter the respective bodies.
Call continuations require ordinary returns, preserved saved values, intact
frames and live local bytes, and nonaliasing dynamic stores. Indexed tables,
flags, pointers, pointed storage and callee behavior remain unqualified.
Each MFHI follows its MULTU without an intervening call or HI/LO write.
Hardware timing remains unverified. Referenced data and callees remain excluded.
No ELF match, observed execution, feasible path,
valid-argument, resource-safety or safe-editing claim follows.

The body at `0x8009D198..0x8009DA94` has 575 instruction words in 64 blocks.
Its complete sized executable ELF section, original bytes and captured resident
RAM match, with all 190 text relocations reconciled. Structural review covers
every word and delay slot. The indirect-call continuation assumes an ordinary
return and an intact caller frame. The target of `JALR v0` at `0x8009D41C`
remains unqualified. No native entry or full-function execution is claimed.

The body at `0x800AC964..0x800ACF80` has a captured native entry at `0x800AC964`
with return address `0x801D1A80`. Complete original control-flow review accounts
for all 391 words, including the return delay slot.
Both direct calls target `0x80158DB8`, whose body remains outside this review.
The bodies at `0x800AD2EC..0x800AD434` and `0x800AD434..0x800AD4D4` have complete
original control-flow reviews and conditional entries through the original table
consumed by `0x800AD26C`. Call continuations assume that callees return; these
two bodies have no captured entry or ELF-match claim.
The seven bodies in `0x800AD71C..0x800AD930`, `0x800ADA4C..0x800ADCA4`
and `0x800ADD44..0x800ADF9C` also have complete original control-flow reviews
and conditional entries through reviewed callers that load handler addresses
from tables or construct them on the stack.
They have no captured entry or ELF-match claim. Their call continuations assume
that callees return.
The body at `0x800AE09C..0x800AE108` has a complete original control-flow review
and a conditional selector-1 entry through `0x800AE014`. Its two calls target
`0x800A9820` and `0x801E5988`, whose behavior remains outside this review.
Its call continuations assume returns; no captured entry or ELF match is claimed.
The body at `0x800AE108..0x800AE1F8` has a captured native entry with return
address `0x801D39F0` from the original call at `0x801D39E8`. Complete original
control-flow review accounts for all sixty words, including returns and delay
slots. Call continuations assume returns; no ELF match or full-function
execution is claimed.
The body at `0x800AE1F8..0x800AE260` has a captured native entry with return
address `0x801D381C` from the original call at `0x801D3814`. Complete original
control-flow review accounts for all 26 words, including the return delay slot.
Its call to `0x80161FDC` assumes a return. No ELF match or full-function execution
is claimed.
The other listed bodies match complete executable ELF sections of sized function
symbols, with text relocations reconciled. All listed bodies include their
returns and delay slots. Their bytes match original `BATTLE.EMI` entry 15 and
captured resident RAM. They contain no embedded tables or literal pools.

The same source gate qualifies twelve bytes at payload `[0x194,0x1A0)`, runtime
`0x80096994..0x800969A0`, as three numeric function-address words. The complete
dispatcher at `0x800AD26C` loads all three words, copies them to its stack and
calls the indexed address. Selectors 0, 1 and 2 conditionally select the three
handlers at `0x800AD2EC`, `0x800AD434` and `0x800AD67C`. The words match original
bytes and resident RAM. Actual invocation, current-work validity, selector bounds
and pointer lifetimes remain unverified. The handler at `0x800AD434` constructs
the addresses `0x800AD4D4` and `0x800AD5C8` on its stack and conditionally calls
them for current-work byte `+2` values 0 and 1. Both complete leaf bodies match
their sized executable ELF sections, original bytes and resident RAM, with all
text relocations reconciled. Valid work records, selector bounds and list
integrity remain unverified. Surrounding header bytes receive no classification
from this rule.

The same source gate qualifies three more twelve-byte tables at payload
`[0x1A0,0x1AC)`, `[0x1AC,0x1B8)` and `[0x1B8,0x1C4)`. Their complete dispatchers
at `0x800AD69C`, `0x800AD9CC` and `0x800ADCC4` load each table's three words,
copy them to the stack and conditionally call the selected pointer for work-byte
`+1` values 0, 1 and 2. The target addresses are:

| Runtime table | Conditional targets |
| --- | --- |
| `0x800969A0` | `0x800AD71C`, `0x800AD808`, `0x800AD9AC` |
| `0x800969AC` | `0x800ADA4C`, `0x800ADB74`, `0x800ADCA4` |
| `0x800969B8` | `0x800ADD44`, `0x800ADE6C`, `0x800ADF9C` |

All words and complete target bodies match original bytes and resident RAM.
The handler at `0x800AD808` constructs `0x800AD8D8` and `0x800AD930` for
work-byte `+2` values 0 and 1. The dispatcher at `0x800AE014` constructs
`0x800AE06C` and `0x800AE09C` for work-byte `+1` values 0 and 1. The body
at `0x800ADFBC` tests `u16[0x801462E8] & 0x800` before an indirect call through
a table indexed by work-byte `+5`. That table's extent and targets remain
unqualified. These are conditional source facts, with selector bounds and
actual invocation still unverified. Surrounding header and referenced data bytes
receive no classification from these instruction reviews.

Neighboring functions and tables retain their prior status. This classification
establishes the listed instruction and address bytes. It does not establish
full-function execution, valid arguments, selector bounds, indirect-call safety,
resource safety, a unique supplier or duplicate-source ownership.

## Battle overlay instruction coverage

Coverage recognizes these complete bodies in original US raw payloads loaded at `0x801D0C00`
with size 118224 and SHA-256
`f25ef137618bc024a6aec98c2c4d9a571c6f23884762fb4567406e6552b383c1`.
The containing archive must also match the original disc manifest.

| Payload interval | Runtime interval | Bytes | Text relocations |
| --- | --- | ---: | ---: |
| `[0x2C44,0x2DBC)` | `[0x801D3844,0x801D39BC)` | 376 | 26 |
| `[0x3C50,0x3EC0)` | `[0x801D4850,0x801D4AC0)` | 624 | 54 |
| `[0x4144,0x4408)` | `[0x801D4D44,0x801D5008)` | 708 | 34 |
| `[0x48F8,0x4A58)` | `[0x801D54F8,0x801D5658)` | 352 | 24 |
| `[0x4A58,0x4BAC)` | `[0x801D5658,0x801D57AC)` | 340 | 19 |
| `[0x4BAC,0x4D0C)` | `[0x801D57AC,0x801D590C)` | 352 | 19 |
| `[0x4D0C,0x4E60)` | `[0x801D590C,0x801D5A60)` | 340 | 19 |
| `[0x4E60,0x4FC0)` | `[0x801D5A60,0x801D5BC0)` | 352 | 19 |
| `[0x4FC0,0x51CC)` | `[0x801D5BC0,0x801D5DCC)` | 524 | 41 |
| `[0x58C4,0x596C)` | `[0x801D64C4,0x801D656C)` | 168 | 10 |
| `[0x5B74,0x5BB0)` | `[0x801D6774,0x801D67B0)` | 60 | 4 |
| `[0x5BB0,0x5BEC)` | `[0x801D67B0,0x801D67EC)` | 60 | 4 |
| `[0x5BEC,0x5C18)` | `[0x801D67EC,0x801D6818)` | 44 | 8 |
| `[0x5E14,0x6084)` | `[0x801D6A14,0x801D6C84)` | 624 | 62 |
| `[0x6084,0x60C0)` | `[0x801D6C84,0x801D6CC0)` | 60 | 4 |
| `[0x60C0,0x6190)` | `[0x801D6CC0,0x801D6D90)` | 208 | 22 |
| `[0x6190,0x61E4)` | `[0x801D6D90,0x801D6DE4)` | 84 | 5 |
| `[0x61E4,0x6220)` | `[0x801D6DE4,0x801D6E20)` | 60 | 4 |
| `[0x6220,0x62AC)` | `[0x801D6E20,0x801D6EAC)` | 140 | 11 |
| `[0x62AC,0x62EC)` | `[0x801D6EAC,0x801D6EEC)` | 64 | 3 |
| `[0x62EC,0x6328)` | `[0x801D6EEC,0x801D6F28)` | 60 | 4 |
| `[0x6508,0x656C)` | `[0x801D7108,0x801D716C)` | 100 | 10 |
| `[0x656C,0x65A4)` | `[0x801D716C,0x801D71A4)` | 56 | 5 |
| `[0x65A4,0x65E0)` | `[0x801D71A4,0x801D71E0)` | 60 | 4 |
| `[0x65E0,0x6634)` | `[0x801D71E0,0x801D7234)` | 84 | 7 |
| `[0x68D4,0x690C)` | `[0x801D74D4,0x801D750C)` | 56 | 6 |
| `[0x7110,0x72B0)` | `[0x801D7D10,0x801D7EB0)` | 416 | 12 |
| `[0x72B0,0x7490)` | `[0x801D7EB0,0x801D8090)` | 480 | 14 |
| `[0x7490,0x7670)` | `[0x801D8090,0x801D8270)` | 480 | 14 |
| `[0x7670,0x7850)` | `[0x801D8270,0x801D8450)` | 480 | 14 |
| `[0x7850,0x7A90)` | `[0x801D8450,0x801D8690)` | 576 | 21 |
| `[0x7EE4,0x81F8)` | `[0x801D8AE4,0x801D8DF8)` | 788 | 26 |
| `[0x8704,0x8788)` | `[0x801D9304,0x801D9388)` | 132 | 5 |
| `[0x8788,0x87E4)` | `[0x801D9388,0x801D93E4)` | 92 | 1 |
| `[0x87E4,0x8828)` | `[0x801D93E4,0x801D9428)` | 68 | 1 |
| `[0x8828,0x8884)` | `[0x801D9428,0x801D9484)` | 92 | 1 |
| `[0x8884,0x88D4)` | `[0x801D9484,0x801D94D4)` | 80 | 1 |
| `[0x88D4,0x8A84)` | `[0x801D94D4,0x801D9684)` | 432 | 29 |
| `[0x8A84,0x8C04)` | `[0x801D9684,0x801D9804)` | 384 | 23 |
| `[0x8C04,0x8D00)` | `[0x801D9804,0x801D9900)` | 252 | 10 |
| `[0x8D00,0x8DAC)` | `[0x801D9900,0x801D99AC)` | 172 | 10 |
| `[0x8DAC,0x8EB4)` | `[0x801D99AC,0x801D9AB4)` | 264 | 14 |
| `[0x8EB4,0x9080)` | `[0x801D9AB4,0x801D9C80)` | 460 | 23 |
| `[0x9478,0x954C)` | `[0x801DA078,0x801DA14C)` | 212 | 9 |
| `[0x954C,0x9728)` | `[0x801DA14C,0x801DA328)` | 476 | 33 |
| `[0x9728,0x9808)` | `[0x801DA328,0x801DA408)` | 224 | 9 |
| `[0x9808,0x98B4)` | `[0x801DA408,0x801DA4B4)` | 172 | 2 |
| `[0x98B4,0x99A8)` | `[0x801DA4B4,0x801DA5A8)` | 244 | 10 |
| `[0x99A8,0x9A9C)` | `[0x801DA5A8,0x801DA69C)` | 244 | 10 |
| `[0x9A9C,0x9BD4)` | `[0x801DA69C,0x801DA7D4)` | 312 | 11 |
| `[0xA458,0xA6F8)` | `[0x801DB058,0x801DB2F8)` | 672 | 18 |
| `[0xA6F8,0xA7A0)` | `[0x801DB2F8,0x801DB3A0)` | 168 | 9 |
| `[0xA7A0,0xA7E4)` | `[0x801DB3A0,0x801DB3E4)` | 68 | 2 |
| `[0xA7E4,0xA834)` | `[0x801DB3E4,0x801DB434)` | 80 | 2 |
| `[0xA834,0xA894)` | `[0x801DB434,0x801DB494)` | 96 | 2 |
| `[0xA894,0xA924)` | `[0x801DB494,0x801DB524)` | 144 | 8 |
| `[0xA924,0xA9CC)` | `[0x801DB524,0x801DB5CC)` | 168 | 9 |
| `[0xA9CC,0xAAF4)` | `[0x801DB5CC,0x801DB6F4)` | 296 | 6 |
| `[0xAAF4,0xAC44)` | `[0x801DB6F4,0x801DB844)` | 336 | 7 |
| `[0xAC44,0xADE4)` | `[0x801DB844,0x801DB9E4)` | 416 | 25 |
| `[0xADE4,0xAF20)` | `[0x801DB9E4,0x801DBB20)` | 316 | 21 |
| `[0xAF20,0xAF78)` | `[0x801DBB20,0x801DBB78)` | 88 | 4 |
| `[0xAF78,0xB444)` | `[0x801DBB78,0x801DC044)` | 1228 | 91 |
| `[0xBC94,0xBED8)` | `[0x801DC894,0x801DCAD8)` | 580 | 28 |
| `[0xC0B0,0xC150)` | `[0x801DCCB0,0x801DCD50)` | 160 | 8 |
| `[0xC150,0xC2F8)` | `[0x801DCD50,0x801DCEF8)` | 424 | 13 |
| `[0xC2F8,0xC3D0)` | `[0x801DCEF8,0x801DCFD0)` | 216 | 13 |
| `[0xC3D0,0xC48C)` | `[0x801DCFD0,0x801DD08C)` | 188 | 10 |
| `[0xC48C,0xC54C)` | `[0x801DD08C,0x801DD14C)` | 192 | 8 |
| `[0xC54C,0xC66C)` | `[0x801DD14C,0x801DD26C)` | 288 | 23 |
| `[0xC66C,0xC69C)` | `[0x801DD26C,0x801DD29C)` | 48 | 4 |
| `[0xC69C,0xC750)` | `[0x801DD29C,0x801DD350)` | 180 | 8 |
| `[0xC750,0xC7CC)` | `[0x801DD350,0x801DD3CC)` | 124 | 3 |
| `[0xC7CC,0xC848)` | `[0x801DD3CC,0x801DD448)` | 124 | 3 |
| `[0xC848,0xC99C)` | `[0x801DD448,0x801DD59C)` | 340 | 12 |
| `[0xC99C,0xCB04)` | `[0x801DD59C,0x801DD704)` | 360 | 23 |
| `[0xCB04,0xCB60)` | `[0x801DD704,0x801DD760)` | 92 | 3 |
| `[0xCB60,0xCBAC)` | `[0x801DD760,0x801DD7AC)` | 76 | 5 |
| `[0xCBAC,0xCBD8)` | `[0x801DD7AC,0x801DD7D8)` | 44 | 2 |
| `[0xCBD8,0xCC00)` | `[0x801DD7D8,0x801DD800)` | 40 | 1 |
| `[0xCC58,0xCCAC)` | `[0x801DD858,0x801DD8AC)` | 84 | 3 |
| `[0xCCAC,0xCD78)` | `[0x801DD8AC,0x801DD978)` | 204 | 22 |
| `[0xCD78,0xCEB4)` | `[0x801DD978,0x801DDAB4)` | 316 | 10 |
| `[0xCEB4,0xCEF0)` | `[0x801DDAB4,0x801DDAF0)` | 60 | 1 |
| `[0xCEF0,0xCF7C)` | `[0x801DDAF0,0x801DDB7C)` | 140 | 15 |
| `[0xCF7C,0xD0B4)` | `[0x801DDB7C,0x801DDCB4)` | 312 | 14 |
| `[0xD27C,0xD2D8)` | `[0x801DDE7C,0x801DDED8)` | 92 | 5 |
| `[0xD2D8,0xD300)` | `[0x801DDED8,0x801DDF00)` | 40 | 6 |
| `[0xD300,0xD328)` | `[0x801DDF00,0x801DDF28)` | 40 | 6 |
| `[0xD328,0xD350)` | `[0x801DDF28,0x801DDF50)` | 40 | 0 |
| `[0xD350,0xD3EC)` | `[0x801DDF50,0x801DDFEC)` | 156 | 4 |
| `[0xD3EC,0xD4AC)` | `[0x801DDFEC,0x801DE0AC)` | 192 | 15 |
| `[0xD4AC,0xD4F8)` | `[0x801DE0AC,0x801DE0F8)` | 76 | 0 |
| `[0xD4F8,0xD544)` | `[0x801DE0F8,0x801DE144)` | 76 | 0 |
| `[0xD544,0xD590)` | `[0x801DE144,0x801DE190)` | 76 | 0 |
| `[0xD590,0xD5B0)` | `[0x801DE190,0x801DE1B0)` | 32 | 2 |
| `[0xD5B0,0xD5D4)` | `[0x801DE1B0,0x801DE1D4)` | 36 | 2 |
| `[0xD83C,0xD960)` | `[0x801DE43C,0x801DE560)` | 292 | 10 |
| `[0xD960,0xDA0C)` | `[0x801DE560,0x801DE60C)` | 172 | 17 |
| `[0xDA0C,0xDA90)` | `[0x801DE60C,0x801DE690)` | 132 | 16 |
| `[0xDA90,0xDA98)` | `[0x801DE690,0x801DE698)` | 8 | 0 |
| `[0xDA98,0xDB54)` | `[0x801DE698,0x801DE754)` | 188 | 10 |
| `[0xDB54,0xDBFC)` | `[0x801DE754,0x801DE7FC)` | 168 | 8 |
| `[0xDBFC,0xDC04)` | `[0x801DE7FC,0x801DE804)` | 8 | 0 |
| `[0xDC04,0xDC58)` | `[0x801DE804,0x801DE858)` | 84 | 6 |
| `[0xDC58,0xDCC0)` | `[0x801DE858,0x801DE8C0)` | 104 | 4 |
| `[0xDCC0,0xDD2C)` | `[0x801DE8C0,0x801DE92C)` | 108 | 14 |
| `[0xDD2C,0xDD4C)` | `[0x801DE92C,0x801DE94C)` | 32 | 0 |
| `[0xDD4C,0xDDA8)` | `[0x801DE94C,0x801DE9A8)` | 92 | 7 |
| `[0xDDA8,0xDE18)` | `[0x801DE9A8,0x801DEA18)` | 112 | 9 |
| `[0xDE18,0xDE64)` | `[0x801DEA18,0x801DEA64)` | 76 | 5 |
| `[0xDE64,0xDEE0)` | `[0x801DEA64,0x801DEAE0)` | 124 | 9 |
| `[0xDFC4,0xE028)` | `[0x801DEBC4,0x801DEC28)` | 100 | 4 |
| `[0xE0E0,0xE154)` | `[0x801DECE0,0x801DED54)` | 116 | 17 |
| `[0xE154,0xE1E4)` | `[0x801DED54,0x801DEDE4)` | 144 | 3 |
| `[0xE1E4,0xE24C)` | `[0x801DEDE4,0x801DEE4C)` | 104 | 1 |
| `[0xE24C,0xE2B4)` | `[0x801DEE4C,0x801DEEB4)` | 104 | 1 |
| `[0xE2B4,0xE30C)` | `[0x801DEEB4,0x801DEF0C)` | 88 | 2 |
| `[0xE3DC,0xE3E4)` | `[0x801DEFDC,0x801DEFE4)` | 8 | 0 |
| `[0xE3E4,0xE74C)` | `[0x801DEFE4,0x801DF34C)` | 872 | 70 |
| `[0xE74C,0xE7B8)` | `[0x801DF34C,0x801DF3B8)` | 108 | 2 |
| `[0xE7B8,0xE7F8)` | `[0x801DF3B8,0x801DF3F8)` | 64 | 2 |
| `[0xE7F8,0xEB68)` | `[0x801DF3F8,0x801DF768)` | 880 | 64 |
| `[0xEB68,0xEBEC)` | `[0x801DF768,0x801DF7EC)` | 132 | 11 |
| `[0xEBEC,0xECAC)` | `[0x801DF7EC,0x801DF8AC)` | 192 | 19 |
| `[0xECAC,0xED14)` | `[0x801DF8AC,0x801DF914)` | 104 | 5 |
| `[0xED14,0xEE1C)` | `[0x801DF914,0x801DFA1C)` | 264 | 29 |
| `[0xEE60,0xEF64)` | `[0x801DFA60,0x801DFB64)` | 260 | 14 |
| `[0xEF64,0xEFDC)` | `[0x801DFB64,0x801DFBDC)` | 120 | 8 |
| `[0xF020,0xF070)` | `[0x801DFC20,0x801DFC70)` | 80 | 8 |
| `[0xF070,0xF0B4)` | `[0x801DFC70,0x801DFCB4)` | 68 | 2 |
| `[0xF59C,0xF60C)` | `[0x801E019C,0x801E020C)` | 112 | 2 |
| `[0xF60C,0xF86C)` | `[0x801E020C,0x801E046C)` | 608 | 49 |
| `[0xF86C,0xF8B0)` | `[0x801E046C,0x801E04B0)` | 68 | 2 |
| `[0xF8B0,0xF954)` | `[0x801E04B0,0x801E0554)` | 164 | 16 |
| `[0xF954,0xF9EC)` | `[0x801E0554,0x801E05EC)` | 152 | 9 |
| `[0xF9EC,0xFAEC)` | `[0x801E05EC,0x801E06EC)` | 256 | 19 |
| `[0xFAEC,0xFB44)` | `[0x801E06EC,0x801E0744)` | 88 | 3 |
| `[0xFB44,0xFB88)` | `[0x801E0744,0x801E0788)` | 68 | 4 |
| `[0xFB88,0xFF64)` | `[0x801E0788,0x801E0B64)` | 988 | 88 |
| `[0xFF64,0x1020C)` | `[0x801E0B64,0x801E0E0C)` | 680 | 22 |
| `[0x1020C,0x1030C)` | `[0x801E0E0C,0x801E0F0C)` | 256 | 14 |
| `[0x1030C,0x10450)` | `[0x801E0F0C,0x801E1050)` | 324 | 29 |
| `[0x10450,0x104C8)` | `[0x801E1050,0x801E10C8)` | 120 | 11 |
| `[0x104C8,0x10698)` | `[0x801E10C8,0x801E1298)` | 464 | 40 |
| `[0x10698,0x106DC)` | `[0x801E1298,0x801E12DC)` | 68 | 4 |
| `[0x106DC,0x107B8)` | `[0x801E12DC,0x801E13B8)` | 220 | 12 |
| `[0x107B8,0x10850)` | `[0x801E13B8,0x801E1450)` | 152 | 13 |
| `[0x10850,0x10894)` | `[0x801E1450,0x801E1494)` | 68 | 2 |
| `[0x10894,0x109CC)` | `[0x801E1494,0x801E15CC)` | 312 | 22 |
| `[0x109CC,0x10A70)` | `[0x801E15CC,0x801E1670)` | 164 | 17 |
| `[0x10AB4,0x10C88)` | `[0x801E16B4,0x801E1888)` | 468 | 42 |
| `[0x10C88,0x10D38)` | `[0x801E1888,0x801E1938)` | 176 | 12 |
| `[0x10D38,0x10D90)` | `[0x801E1938,0x801E1990)` | 88 | 5 |
| `[0x10DDC,0x10E14)` | `[0x801E19DC,0x801E1A14)` | 56 | 1 |
| `[0x10E14,0x10EDC)` | `[0x801E1A14,0x801E1ADC)` | 200 | 18 |
| `[0x10EDC,0x10EE4)` | `[0x801E1ADC,0x801E1AE4)` | 8 | 0 |
| `[0x10EE4,0x10F04)` | `[0x801E1AE4,0x801E1B04)` | 32 | 1 |
| `[0x10F04,0x10F2C)` | `[0x801E1B04,0x801E1B2C)` | 40 | 3 |
| `[0x10F2C,0x10F5C)` | `[0x801E1B2C,0x801E1B5C)` | 48 | 3 |
| `[0x10F5C,0x10F64)` | `[0x801E1B5C,0x801E1B64)` | 8 | 0 |
| `[0x10FA8,0x11058)` | `[0x801E1BA8,0x801E1C58)` | 176 | 11 |
| `[0x11058,0x11078)` | `[0x801E1C58,0x801E1C78)` | 32 | 4 |
| `[0x11078,0x110D8)` | `[0x801E1C78,0x801E1CD8)` | 96 | 4 |
| `[0x1111C,0x11188)` | `[0x801E1D1C,0x801E1D88)` | 108 | 7 |
| `[0x11188,0x111D4)` | `[0x801E1D88,0x801E1DD4)` | 76 | 5 |
| `[0x111D4,0x1127C)` | `[0x801E1DD4,0x801E1E7C)` | 168 | 13 |
| `[0x112C0,0x11424)` | `[0x801E1EC0,0x801E2024)` | 356 | 31 |
| `[0x11424,0x11520)` | `[0x801E2024,0x801E2120)` | 252 | 13 |
| `[0x11520,0x11570)` | `[0x801E2120,0x801E2170)` | 80 | 3 |
| `[0x11570,0x11684)` | `[0x801E2170,0x801E2284)` | 276 | 15 |
| `[0x11684,0x11714)` | `[0x801E2284,0x801E2314)` | 144 | 7 |
| `[0x11714,0x118F8)` | `[0x801E2314,0x801E24F8)` | 484 | 32 |
| `[0x118F8,0x1197C)` | `[0x801E24F8,0x801E257C)` | 132 | 15 |
| `[0x1197C,0x119E0)` | `[0x801E257C,0x801E25E0)` | 100 | 9 |
| `[0x119E0,0x11D48)` | `[0x801E25E0,0x801E2948)` | 872 | 63 |
| `[0x11D48,0x11DB4)` | `[0x801E2948,0x801E29B4)` | 108 | 6 |
| `[0x11DB4,0x11E88)` | `[0x801E29B4,0x801E2A88)` | 212 | 2 |
| `[0x120A4,0x1214C)` | `[0x801E2CA4,0x801E2D4C)` | 168 | 3 |
| `[0x1214C,0x12190)` | `[0x801E2D4C,0x801E2D90)` | 68 | 3 |
| `[0x12190,0x12230)` | `[0x801E2D90,0x801E2E30)` | 160 | 3 |
| `[0x124B8,0x124F8)` | `[0x801E30B8,0x801E30F8)` | 64 | 4 |
| `[0x124F8,0x12560)` | `[0x801E30F8,0x801E3160)` | 104 | 3 |
| `[0x12560,0x125C8)` | `[0x801E3160,0x801E31C8)` | 104 | 3 |
| `[0x1260C,0x126AC)` | `[0x801E320C,0x801E32AC)` | 160 | 20 |
| `[0x126F0,0x12734)` | `[0x801E32F0,0x801E3334)` | 68 | 2 |
| `[0x12734,0x12798)` | `[0x801E3334,0x801E3398)` | 100 | 7 |
| `[0x12798,0x12838)` | `[0x801E3398,0x801E3438)` | 160 | 10 |
| `[0x12838,0x1287C)` | `[0x801E3438,0x801E347C)` | 68 | 2 |
| `[0x1287C,0x1292C)` | `[0x801E347C,0x801E352C)` | 176 | 11 |
| `[0x1292C,0x129D8)` | `[0x801E352C,0x801E35D8)` | 172 | 10 |
| `[0x129D8,0x12A38)` | `[0x801E35D8,0x801E3638)` | 96 | 2 |
| `[0x12A38,0x12C3C)` | `[0x801E3638,0x801E383C)` | 516 | 35 |
| `[0x12C3C,0x12CC0)` | `[0x801E383C,0x801E38C0)` | 132 | 11 |
| `[0x12CC0,0x12D80)` | `[0x801E38C0,0x801E3980)` | 192 | 19 |
| `[0x12D80,0x12E00)` | `[0x801E3980,0x801E3A00)` | 128 | 9 |
| `[0x12E44,0x12F04)` | `[0x801E3A44,0x801E3B04)` | 192 | 14 |
| `[0x12F04,0x12F68)` | `[0x801E3B04,0x801E3B68)` | 100 | 10 |
| `[0x12FAC,0x12FD0)` | `[0x801E3BAC,0x801E3BD0)` | 36 | 0 |
| `[0x12FD0,0x13014)` | `[0x801E3BD0,0x801E3C14)` | 68 | 4 |
| `[0x13504,0x1354C)` | `[0x801E4104,0x801E414C)` | 72 | 1 |
| `[0x1354C,0x13768)` | `[0x801E414C,0x801E4368)` | 540 | 47 |
| `[0x13768,0x13890)` | `[0x801E4368,0x801E4490)` | 296 | 9 |
| `[0x13890,0x138D4)` | `[0x801E4490,0x801E44D4)` | 68 | 2 |
| `[0x138D4,0x13944)` | `[0x801E44D4,0x801E4544)` | 112 | 11 |
| `[0x13944,0x139DC)` | `[0x801E4544,0x801E45DC)` | 152 | 9 |
| `[0x139DC,0x13AEC)` | `[0x801E45DC,0x801E46EC)` | 272 | 22 |
| `[0x13AEC,0x13B60)` | `[0x801E46EC,0x801E4760)` | 116 | 4 |
| `[0x13B60,0x13BA4)` | `[0x801E4760,0x801E47A4)` | 68 | 2 |
| `[0x13BA4,0x13BE0)` | `[0x801E47A4,0x801E47E0)` | 60 | 0 |
| `[0x13BE0,0x13C3C)` | `[0x801E47E0,0x801E483C)` | 92 | 3 |
| `[0x13C3C,0x13CD4)` | `[0x801E483C,0x801E48D4)` | 152 | 14 |
| `[0x13CD4,0x13D28)` | `[0x801E48D4,0x801E4928)` | 84 | 1 |
| `[0x13D28,0x13D6C)` | `[0x801E4928,0x801E496C)` | 68 | 2 |
| `[0x13D6C,0x13E50)` | `[0x801E496C,0x801E4A50)` | 228 | 14 |
| `[0x13E50,0x13EE8)` | `[0x801E4A50,0x801E4AE8)` | 152 | 16 |
| `[0x13F2C,0x1407C)` | `[0x801E4B2C,0x801E4C7C)` | 336 | 28 |
| `[0x1407C,0x14134)` | `[0x801E4C7C,0x801E4D34)` | 184 | 15 |
| `[0x14134,0x1418C)` | `[0x801E4D34,0x801E4D8C)` | 88 | 5 |
| `[0x141D8,0x14210)` | `[0x801E4DD8,0x801E4E10)` | 56 | 3 |
| `[0x14210,0x142CC)` | `[0x801E4E10,0x801E4ECC)` | 188 | 15 |
| `[0x142CC,0x14334)` | `[0x801E4ECC,0x801E4F34)` | 104 | 7 |
| `[0x14334,0x14364)` | `[0x801E4F34,0x801E4F64)` | 48 | 3 |
| `[0x143A8,0x144C0)` | `[0x801E4FA8,0x801E50C0)` | 280 | 27 |
| `[0x144C0,0x145DC)` | `[0x801E50C0,0x801E51DC)` | 284 | 15 |
| `[0x145DC,0x14688)` | `[0x801E51DC,0x801E5288)` | 172 | 19 |
| `[0x14688,0x146E8)` | `[0x801E5288,0x801E52E8)` | 96 | 4 |
| `[0x146E8,0x146F0)` | `[0x801E52E8,0x801E52F0)` | 8 | 0 |
| `[0x146F0,0x1471C)` | `[0x801E52F0,0x801E531C)` | 44 | 1 |
| `[0x1471C,0x148EC)` | `[0x801E531C,0x801E54EC)` | 464 | 24 |
| `[0x14A7C,0x14B04)` | `[0x801E567C,0x801E5704)` | 136 | 3 |
| `[0x14B04,0x14B68)` | `[0x801E5704,0x801E5768)` | 100 | 4 |
| `[0x14B68,0x14BC8)` | `[0x801E5768,0x801E57C8)` | 96 | 10 |
| `[0x14BC8,0x14C24)` | `[0x801E57C8,0x801E5824)` | 92 | 9 |
| `[0x14C24,0x14D0C)` | `[0x801E5824,0x801E590C)` | 232 | 14 |
| `[0x14D0C,0x14D88)` | `[0x801E590C,0x801E5988)` | 124 | 9 |
| `[0x14D88,0x14E38)` | `[0x801E5988,0x801E5A38)` | 176 | 22 |
| `[0x14E38,0x14EEC)` | `[0x801E5A38,0x801E5AEC)` | 180 | 22 |
| `[0x14EEC,0x14EF4)` | `[0x801E5AEC,0x801E5AF4)` | 8 | 0 |
| `[0x14EF4,0x14F8C)` | `[0x801E5AF4,0x801E5B8C)` | 152 | 4 |
| `[0x14F8C,0x1501C)` | `[0x801E5B8C,0x801E5C1C)` | 144 | 4 |
| `[0x1501C,0x150A4)` | `[0x801E5C1C,0x801E5CA4)` | 136 | 4 |
| `[0x150A4,0x1522C)` | `[0x801E5CA4,0x801E5E2C)` | 392 | 25 |
| `[0x15488,0x154DC)` | `[0x801E6088,0x801E60DC)` | 84 | 6 |
| `[0x154DC,0x15554)` | `[0x801E60DC,0x801E6154)` | 120 | 6 |
| `[0x15554,0x155F0)` | `[0x801E6154,0x801E61F0)` | 156 | 7 |
| `[0x155F0,0x156BC)` | `[0x801E61F0,0x801E62BC)` | 204 | 14 |
| `[0x156BC,0x157C0)` | `[0x801E62BC,0x801E63C0)` | 260 | 5 |
| `[0x157C0,0x15820)` | `[0x801E63C0,0x801E6420)` | 96 | 4 |
| `[0x15820,0x158B4)` | `[0x801E6420,0x801E64B4)` | 148 | 14 |
| `[0x15B24,0x15B9C)` | `[0x801E6724,0x801E679C)` | 120 | 8 |
| `[0x15B9C,0x15CEC)` | `[0x801E679C,0x801E68EC)` | 336 | 28 |
| `[0x15CEC,0x15D30)` | `[0x801E68EC,0x801E6930)` | 68 | 4 |
| `[0x15D30,0x15D90)` | `[0x801E6930,0x801E6990)` | 96 | 4 |
| `[0x15D90,0x15DC0)` | `[0x801E6990,0x801E69C0)` | 48 | 4 |
| `[0x15DC0,0x15E54)` | `[0x801E69C0,0x801E6A54)` | 148 | 7 |
| `[0x15E54,0x15EA0)` | `[0x801E6A54,0x801E6AA0)` | 76 | 1 |
| `[0x15EA0,0x15F28)` | `[0x801E6AA0,0x801E6B28)` | 136 | 7 |
| `[0x15F28,0x15FC8)` | `[0x801E6B28,0x801E6BC8)` | 160 | 10 |
| `[0x15FC8,0x16020)` | `[0x801E6BC8,0x801E6C20)` | 88 | 1 |
| `[0x16020,0x16084)` | `[0x801E6C20,0x801E6C84)` | 100 | 6 |
| `[0x16084,0x16128)` | `[0x801E6C84,0x801E6D28)` | 164 | 8 |
| `[0x16128,0x16254)` | `[0x801E6D28,0x801E6E54)` | 300 | 19 |
| `[0x16254,0x163A0)` | `[0x801E6E54,0x801E6FA0)` | 332 | 31 |
| `[0x163A0,0x163B0)` | `[0x801E6FA0,0x801E6FB0)` | 16 | 0 |
| `[0x163B0,0x165EC)` | `[0x801E6FB0,0x801E71EC)` | 572 | 49 |
| `[0x165EC,0x166A8)` | `[0x801E71EC,0x801E72A8)` | 188 | 10 |
| `[0x166A8,0x166F4)` | `[0x801E72A8,0x801E72F4)` | 76 | 4 |
| `[0x166F4,0x168B8)` | `[0x801E72F4,0x801E74B8)` | 452 | 43 |
| `[0x168B8,0x16928)` | `[0x801E74B8,0x801E7528)` | 112 | 2 |
| `[0x16928,0x16958)` | `[0x801E7528,0x801E7558)` | 48 | 2 |
| `[0x16958,0x169B0)` | `[0x801E7558,0x801E75B0)` | 88 | 3 |
| `[0x169B0,0x16A34)` | `[0x801E75B0,0x801E7634)` | 132 | 4 |
| `[0x16A34,0x16B78)` | `[0x801E7634,0x801E7778)` | 324 | 29 |
| `[0x16B78,0x16C18)` | `[0x801E7778,0x801E7818)` | 160 | 12 |
| `[0x16C18,0x16C88)` | `[0x801E7818,0x801E7888)` | 112 | 4 |
| `[0x16C88,0x16CF0)` | `[0x801E7888,0x801E78F0)` | 104 | 8 |
| `[0x16CF0,0x16D48)` | `[0x801E78F0,0x801E7948)` | 88 | 3 |
| `[0x16D48,0x16F34)` | `[0x801E7948,0x801E7B34)` | 492 | 43 |
| `[0x16F34,0x16FA0)` | `[0x801E7B34,0x801E7BA0)` | 108 | 4 |
| `[0x17958,0x17A2C)` | `[0x801E8558,0x801E862C)` | 212 | 16 |
| `[0x17A2C,0x17A84)` | `[0x801E862C,0x801E8684)` | 88 | 4 |
| `[0x18104,0x181D8)` | `[0x801E8D04,0x801E8DD8)` | 212 | 16 |
| `[0x183A8,0x18474)` | `[0x801E8FA8,0x801E9074)` | 204 | 8 |
| `[0x18474,0x184FC)` | `[0x801E9074,0x801E90FC)` | 136 | 4 |
| `[0x184FC,0x1855C)` | `[0x801E90FC,0x801E915C)` | 96 | 4 |
| `[0x1855C,0x185CC)` | `[0x801E915C,0x801E91CC)` | 112 | 6 |
| `[0x185CC,0x185FC)` | `[0x801E91CC,0x801E91FC)` | 48 | 3 |
| `[0x185FC,0x1865C)` | `[0x801E91FC,0x801E925C)` | 96 | 4 |
| `[0x1865C,0x1867C)` | `[0x801E925C,0x801E927C)` | 32 | 2 |
| `[0x18790,0x1883C)` | `[0x801E9390,0x801E943C)` | 172 | 12 |
| `[0x1883C,0x1889C)` | `[0x801E943C,0x801E949C)` | 96 | 4 |
| `[0x1889C,0x188D0)` | `[0x801E949C,0x801E94D0)` | 52 | 5 |
| `[0x188D0,0x18930)` | `[0x801E94D0,0x801E9530)` | 96 | 4 |
| `[0x18930,0x18938)` | `[0x801E9530,0x801E9538)` | 8 | 0 |
| `[0x18938,0x18A0C)` | `[0x801E9538,0x801E960C)` | 212 | 15 |
| `[0x18A0C,0x18BA8)` | `[0x801E960C,0x801E97A8)` | 412 | 27 |
| `[0x18D10,0x18DAC)` | `[0x801E9910,0x801E99AC)` | 156 | 13 |
| `[0x18DAC,0x18F18)` | `[0x801E99AC,0x801E9B18)` | 364 | 28 |
| `[0x18F18,0x190A8)` | `[0x801E9B18,0x801E9CA8)` | 400 | 25 |
| `[0x19318,0x193DC)` | `[0x801E9F18,0x801E9FDC)` | 196 | 15 |
| `[0x193DC,0x19574)` | `[0x801E9FDC,0x801EA174)` | 408 | 32 |
| `[0x19574,0x195A4)` | `[0x801EA174,0x801EA1A4)` | 48 | 0 |
| `[0x195A4,0x195E0)` | `[0x801EA1A4,0x801EA1E0)` | 60 | 0 |
| `[0x195E0,0x19654)` | `[0x801EA1E0,0x801EA254)` | 116 | 2 |
| `[0x19930,0x1999C)` | `[0x801EA530,0x801EA59C)` | 108 | 1 |
| `[0x19A50,0x19B28)` | `[0x801EA650,0x801EA728)` | 216 | 14 |
| `[0x19B28,0x19B6C)` | `[0x801EA728,0x801EA76C)` | 68 | 3 |
| `[0x19B6C,0x19B74)` | `[0x801EA76C,0x801EA774)` | 8 | 0 |
| `[0x19B74,0x19BDC)` | `[0x801EA774,0x801EA7DC)` | 104 | 8 |
| `[0x19BDC,0x19C48)` | `[0x801EA7DC,0x801EA848)` | 108 | 4 |
| `[0x19C48,0x19C94)` | `[0x801EA848,0x801EA894)` | 76 | 8 |
| `[0x19CF4,0x19DFC)` | `[0x801EA8F4,0x801EA9FC)` | 264 | 21 |
| `[0x19DFC,0x19EB8)` | `[0x801EA9FC,0x801EAAB8)` | 188 | 15 |
| `[0x19EB8,0x19F38)` | `[0x801EAAB8,0x801EAB38)` | 128 | 10 |
| `[0x19F38,0x19F64)` | `[0x801EAB38,0x801EAB64)` | 44 | 0 |
| `[0x19F64,0x19F6C)` | `[0x801EAB64,0x801EAB6C)` | 8 | 0 |
| `[0x19F6C,0x19FB8)` | `[0x801EAB6C,0x801EABB8)` | 76 | 0 |

Every listed body matches its complete sized executable ELF section, original
bytes and resident RAM, with its text relocations reconciled. Structural
control-flow review covers every instruction and delay slot, assuming call returns.
The bodies at `0x801DEFE4` and `0x801E5CA4` have six and five bounded internal
switch targets. Their separate address tables retain their prior status. The bodies at `0x801DB058`, `0x801E29B4` and `0x801DCCB0` include four, two and
two `BREAK` exception paths. Indirect call targets and selectors remain unqualified. These checks do not establish feasible paths, exception handling or
callee behavior.

A separate complete original body at `[0x801DC044,0x801DC73C)`, payload
`[0xB444,0xBB3C)`, qualifies through a conditional call. The original `JAL` at
`0x801DBBFC` in the qualified caller at `0x801DBB78` targets this body and sets
`a2=0xFFFF` in its delay slot. All 1,784 original body bytes match resident RAM;
structural review covers all 446 instruction words and delay slots, assuming
return from direct calls. Its compiled callee remains a partial mismatch and
provides no admission basis. This establishes stored instructions, without
observed entry, valid records, callee behavior or gameplay acceptance.

These complete original bodies also qualify through original direct calls from
already qualified executable callers. Every original instruction and delay slot
matches ASM and resident RAM. Structural call continuations assume returns;
matching callee ELF sections and observed entry are not admission claims.

| Payload interval | Runtime interval | Bytes | Original JAL | Qualified caller |
| --- | --- | ---: | --- | --- |
| `[0x690C,0x6E40)` | `[0x801D750C,0x801D7A40)` | 1332 | `0x801E91E4` | `0x801E91CC` |
| `[0x6E40,0x7110)` | `[0x801D7A40,0x801D7D10)` | 720 | `0x801E9424` | `0x801E9390` |
| `[0x7A90,0x7EE4)` | `[0x801D8690,0x801D8AE4)` | 1108 | `0x801E9A98` | `0x801E99AC` |
| `[0x81F8,0x8704)` | `[0x801D8DF8,0x801D9304)` | 1292 | `0x801EA15C` | `0x801E9FDC` |
| `[0x9080,0x91BC)` | `[0x801D9C80,0x801D9DBC)` | 316 | `0x801D75F4` | `0x801D750C` |
| `[0x91BC,0x929C)` | `[0x801D9DBC,0x801D9E9C)` | 224 | `0x801D768C` | `0x801D750C` |
| `[0x929C,0x9478)` | `[0x801D9E9C,0x801DA078)` | 476 | `0x801D8C70` | `0x801D8AE4` |
| `[0xBB3C,0xBC94)` | `[0x801DC73C,0x801DC894)` | 344 | `0x801DC52C` | `0x801DC044` |
| `[0xD0B4,0xD27C)` | `[0x801DDCB4,0x801DDE7C)` | 456 | `0x801D56BC` | `0x801D5658` |
| `[0xDEE0,0xDFC4)` | `[0x801DEAE0,0x801DEBC4)` | 228 | `0x801D6D60` | `0x801D6CC0` |
| `[0x11E88,0x120A4)` | `[0x801E2A88,0x801E2CA4)` | 540 | `0x801E2860` | `0x801E25E0` |
| `[0x148EC,0x14A7C)` | `[0x801E54EC,0x801E567C)` | 400 | `0x801E4910` | `0x801E48D4` |
| `[0x181D8,0x183A8)` | `[0x801E8DD8,0x801E8FA8)` | 464 | `0x801E6F54` | `0x801E6E54` |

Two original numeric function-address tables at payload `[0x320,0x32C)`
and `[0x3D0,0x3E0)` contain three and four words. The qualified dispatchers
at `0x801E63C0` and `0x801E960C` copy all their words to the stack, index
by unsigned work bytes `+1` and `+3`, and call the selected addresses at
`0x801E6408` and `0x801E9660`. Finite selector alternatives identify these
additional complete original bodies. Every table word, consumer instruction
and complete body matches original bytes and resident RAM.

| Payload interval | Runtime interval | Bytes | Table | Selector | Original JALR |
| --- | --- | ---: | --- | ---: | --- |
| `[0x158B4,0x159F8)` | `[0x801E64B4,0x801E65F8)` | 324 | `0x801D0F20` | 1 | `0x801E6408` |
| `[0x159F8,0x15B24)` | `[0x801E65F8,0x801E6724)` | 300 | `0x801D0F20` | 2 | `0x801E6408` |
| `[0x18BA8,0x18D10)` | `[0x801E97A8,0x801E9910)` | 360 | `0x801D0FD0` | 0 | `0x801E9660` |

Overall selector bounds, current-work validity and actual invocation remain
unverified. Call continuations assume returns. The last body contains two
terminal division-trap instructions; divisor validity is unverified. Matching
callee ELF sections, gameplay behavior and resource safety are not established.
Surrounding header and referenced data retain their prior status.

The body at `0x801D4850` contains 156 instruction words. Its complete sized
executable ELF section, all 54 text relocations, original bytes and resident RAM
reconcile. Complete control-flow review covers ten conditional branches, two
internal jumps, the return and every delay slot. It contains no calls, embedded
tables or indirect switches.
The bodies at `0x801DBB78`, `0x801DF3F8` and `0x801E0788` contain 307, 220 and
247 instruction words. Their complete sized executable ELF sections, original
bytes and resident RAM match, with all 91, 64 and 88 text relocations reconciled.
Complete control-flow review covers every word and delay slot. Call continuations
assume returns. The body at `0x801DF3F8` contains one indirect call; its handler
table, selector validity and callee effects remain unqualified.
The body at `0x801E590C` has 31 instruction words. They match its complete sized
executable ELF function section and captured resident RAM, with all nine text
relocations reconciled.
Complete original control-flow review includes every branch, jump, return and
delay slot. The body contains no external calls, embedded data or literal pool.
The body at `0x801E5988` has 44 words. They match its complete sized executable
ELF section, original bytes and resident RAM, with all 22 text relocations reconciled.
Its straight-line body ends with `JR RA` and a store in the return delay slot.
The qualified body at `0x800AE09C` conditionally calls it at `0x800AE0E4`.
It contains no calls, embedded data or literal pool.
Neighboring bytes retain their prior status. This establishes instruction representation;
execution, valid arguments, allocator behavior, resource safety and duplicate-source
ownership remain unverified.

## SLUS startup initialization coverage

Coverage recognizes two additional ranges in the exact original US executable
identified under [SLUS instruction coverage](#slus-instruction-coverage).
File intervals include its 2048-byte PS-X header.

| File interval | Runtime interval | Role |
| --- | --- | --- |
| `[739852,739888)` | `[0x8014AA0C,0x8014AA30)` | Nine startup-loop instructions |
| `[1004216,1027168)` | `[0x8018B2B8,0x80190C60)` | 22952 bytes of startup initialization storage |

The header's entry PC is `0x8014AA0C`. The loop constructs the destination and
exclusive end addresses, stores zero, advances four bytes and branches while
the unsigned address is below the end. Its sole branch has a `NOP` delay slot.
Under normal loader entry, valid mapped RAM, stable code and no interference,
5738 aligned stores clear exactly the destination interval before any call.
The stored destination bytes are all zero, but the loop establishes their role
and extent. The header's `b_addr` and `b_size` are both zero.

These ranges establish instruction and initialization-storage representation.
They do not classify the rest of startup, its trailing data, neighboring storage
or the heap from `0x80190C60` onward. They do not establish observed execution
or the storage's later contents.

## SLUS instruction coverage

Coverage recognizes the following complete bodies in the exact original US
`SLUS_004.22`, with size 1445888 and SHA-256
`0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a`.
File offsets include the 2048-byte PS-X header. The header loads the following
1443840 bytes at `0x80096800`.

| File interval | Runtime interval | Bytes | Text relocations |
| --- | --- | ---: | ---: |
| `[0xB4A04,0xB4A0C)` | `[0x8014AA04,0x8014AA0C)` | 8 | 0 |
| `[0xB4AC8,0xB4CA0)` | `[0x8014AAC8,0x8014ACA0)` | 472 | 66 |
| `[0xB4CA0,0xB4D28)` | `[0x8014ACA0,0x8014AD28)` | 136 | 15 |
| `[0xB4D28,0xB4E08)` | `[0x8014AD28,0x8014AE08)` | 224 | 24 |
| `[0xB4E08,0xB4E9C)` | `[0x8014AE08,0x8014AE9C)` | 148 | 6 |
| `[0xB4E9C,0xB4EE0)` | `[0x8014AE9C,0x8014AEE0)` | 68 | 1 |
| `[0xB4EE0,0xB4FC0)` | `[0x8014AEE0,0x8014AFC0)` | 224 | 23 |
| `[0xB50F0,0xB517C)` | `[0x8014B0F0,0x8014B17C)` | 140 | 9 |
| `[0xB517C,0xB51A4)` | `[0x8014B17C,0x8014B1A4)` | 40 | 3 |
| `[0xB51A4,0xB533C)` | `[0x8014B1A4,0x8014B33C)` | 408 | 48 |
| `[0xB533C,0xB53C4)` | `[0x8014B33C,0x8014B3C4)` | 136 | 11 |
| `[0xB573C,0xB5854)` | `[0x8014B73C,0x8014B854)` | 280 | 25 |
| `[0xB5854,0xB587C)` | `[0x8014B854,0x8014B87C)` | 40 | 4 |
| `[0xB587C,0xB58B0)` | `[0x8014B87C,0x8014B8B0)` | 52 | 3 |
| `[0xB58B0,0xB5900)` | `[0x8014B8B0,0x8014B900)` | 80 | 8 |
| `[0xB5900,0xB5954)` | `[0x8014B900,0x8014B954)` | 84 | 7 |
| `[0xB7D3C,0xB7E78)` | `[0x8014DD3C,0x8014DE78)` | 316 | 25 |
| `[0xB80FC,0xB822C)` | `[0x8014E0FC,0x8014E22C)` | 304 | 16 |
| `[0xB85A0,0xB8640)` | `[0x8014E5A0,0x8014E640)` | 160 | 11 |
| `[0xC7F18,0xC8994)` | `[0x8015DF18,0x8015E994)` | 2684 | 408 |
| `[0xCBF58,0xCBFDC)` | `[0x80161F58,0x80161FDC)` | 132 | 12 |
| `[0xCBFDC,0xCC160)` | `[0x80161FDC,0x80162160)` | 388 | 56 |
| `[0xCC160,0xCC178)` | `[0x80162160,0x80162178)` | 24 | 2 |
| `[0xCC178,0xCC1E8)` | `[0x80162178,0x801621E8)` | 112 | 15 |
| `[0xCC1E8,0xCC230)` | `[0x801621E8,0x80162230)` | 72 | 4 |
| `[0xCC230,0xCC500)` | `[0x80162230,0x80162500)` | 720 | 70 |
| `[0xCC500,0xCC5E4)` | `[0x80162500,0x801625E4)` | 228 | 17 |
| `[0xCC5E4,0xCC618)` | `[0x801625E4,0x80162618)` | 52 | 3 |
| `[0xCC618,0xCC698)` | `[0x80162618,0x80162698)` | 128 | 14 |
| `[0xCC698,0xCC790)` | `[0x80162698,0x80162790)` | 248 | 32 |
| `[0xCC898,0xCC9F0)` | `[0x80162898,0x801629F0)` | 344 | 42 |
| `[0xCC9F0,0xCCA6C)` | `[0x801629F0,0x80162A6C)` | 124 | 9 |
| `[0xCCA6C,0xCCB08)` | `[0x80162A6C,0x80162B08)` | 156 | 13 |
| `[0xCCB08,0xCCC14)` | `[0x80162B08,0x80162C14)` | 268 | 33 |
| `[0xCCC14,0xCCCD8)` | `[0x80162C14,0x80162CD8)` | 196 | 20 |
| `[0xCCCD8,0xCCD00)` | `[0x80162CD8,0x80162D00)` | 40 | 6 |
| `[0xCCD00,0xCCD18)` | `[0x80162D00,0x80162D18)` | 24 | 0 |
| `[0xCCD18,0xCCD9C)` | `[0x80162D18,0x80162D9C)` | 132 | 4 |
| `[0xCF4F4,0xCF570)` | `[0x801654F4,0x80165570)` | 124 | 2 |
| `[0xCF5F4,0xCF694)` | `[0x801655F4,0x80165694)` | 160 | 3 |
| `[0xCFD48,0xCFE04)` | `[0x80165D48,0x80165E04)` | 188 | 16 |
| `[0xD0CB0,0xD0CEC)` | `[0x80166CB0,0x80166CEC)` | 60 | 4 |
| `[0xDD01C,0xDD0B8)` | `[0x8017301C,0x801730B8)` | 156 | 9 |
| `[0xE927C,0xE92B4)` | `[0x8017F27C,0x8017F2B4)` | 56 | 5 |

Every listed body matches its complete sized executable ELF section, original
bytes and retained resident RAM, with all text relocations reconciled. Review
covers every original instruction and delay slot. Call continuations assume
returns. The boot loop at `0x8014AAC8` includes a four-word compiler epilogue at
`0x8014AC90..0x8014ACA0` that is unreachable from its original entry. Complete
section and relocation identity identifies those stored instructions; no path
to that epilogue is established.

The body at `0x8014E5A0..0x8014E640` also has conditional original entries
through JAL sites `0x800AEA1C`, `0x800AEBB8` and `0x800AED3C` in
`emi/battle/battle/15@0x800AE9C4`. Its three control-transfer delay slots include
the saved return address store on both branch paths. Frame restoration assumes
ordinary callee preservation, valid memory, intact saved slots and nonaliasing
stores. The cursor guard and masked index establish no allocation capacity or
table bounds. Referenced globals, tables, buffers and the callee remain excluded.
No observed invocation, unique runtime supplier or safe-editing claim follows.

The equipment selector body has five bounded internal switch targets. Its
separate 20-byte address table remains unclassified. Four indirect call sites
in three bodies retain unqualified targets and selectors. These checks establish
stored instructions, without observed execution, valid arguments, callee
behavior, loader or playback correctness, or resource safety.

The separate original body at `0x8014B020..0x8014B0F0`, file offsets
`[741408,741616)`, qualifies through original calls at `0x8014ABB0` and
`0x8014AD0C` in the qualified boot loop and initializer. All 208 original bytes
match resident RAM. Structural review covers all 52 words, two loop branches
and the return delay slot. The compiled callee remains partial and provides no
admission basis. Actual entry, valid records and callee behavior remain unverified.

Two startup callbacks also qualify through complete original-word and structural
control-flow review, without matching ELF evidence:

| File interval | Runtime interval | Bytes |
| --- | --- | ---: |
| `[0xB8A80,0xB8C64)` | `[0x8014EA80,0x8014EC64)` | 484 |
| `[0xB8C64,0xB8CAC)` | `[0x8014EC64,0x8014ECAC)` | 72 |

The qualified boot loop installs the first pointer at `0x8014AB08`. That body
installs the second pointer at `0x8014EC44`. Every original word matches the
decoder and retained resident RAM. Review covers all branches, delay slots and
both 24-byte frames. External call continuations assume ordinary returns,
preserved saved return addresses, nonaliasing stores and stable code. A native
loader entry binds the first body's interior JAL at `0x8014EAB4`; it does not
prove callback entry or whole-body execution. The second callback's execution
and the callees' behavior remain unverified. Referenced data remain excluded.

These additional complete original bodies qualify through direct calls from
qualified executable callers. Every original word and delay slot matches ASM
and retained resident RAM. Structural call continuations assume returns. Ten
indirect calls retain unqualified targets and selectors. Matching callee ELF,
observed entry, argument validity and callee behavior remain unverified.

| File interval | Runtime interval | Bytes | Original JAL | Qualified caller |
| --- | --- | ---: | --- | --- |
| `[0xDE6E0,0xDE700)` | `[0x801746E0,0x80174700)` | 32 | `0x8014AF1C` | `0x8014AEE0` |
| `[0xDE700,0xDE848)` | `[0x80174700,0x80174848)` | 328 | `0x8014AB10` | `0x8014AAC8` |
| `[0xDE9D8,0xDEA08)` | `[0x801749D8,0x80174A08)` | 48 | `0x8014AF14` | `0x8014AEE0` |
| `[0xDF3EC,0xDF47C)` | `[0x801753EC,0x8017547C)` | 144 | `0x80161F60` | `0x80161F58` |
| `[0xDFADC,0xDFBE0)` | `[0x80175ADC,0x80175BE0)` | 260 | `0x8014E1C0` | `0x8014E0FC` |
| `[0xE4514,0xE45E4)` | `[0x8017A514,0x8017A5E4)` | 208 | `0x8014AE34` | `0x8014AE08` |
| `[0xE488C,0xE48C8)` | `[0x8017A88C,0x8017A8C8)` | 60 | `0x8014B144` | `0x8014B0F0` |
| `[0xE5330,0xE53CC)` | `[0x8017B330,0x8017B3CC)` | 156 | `0x8014AFA4` | `0x8014AEE0` |
| `[0xE53CC,0xE5438)` | `[0x8017B3CC,0x8017B438)` | 108 | `0x8014AC60` | `0x8014AAC8` |
| `[0xE581C,0xE58D4)` | `[0x8017B81C,0x8017B8D4)` | 184 | `0x8014AF60` | `0x8014AEE0` |
| `[0xE58D4,0xE596C)` | `[0x8017B8D4,0x8017B96C)` | 152 | `0x8014ABA8` | `0x8014AAC8` |
| `[0xE59CC,0xE5A40)` | `[0x8017B9CC,0x8017BA40)` | 116 | `0x8014AB58` | `0x8014AAC8` |
| `[0xE80B4,0xE80E4)` | `[0x8017E0B4,0x8017E0E4)` | 48 | `0x8014AF24` | `0x8014AEE0` |

These complete original SLUS bodies also qualify through original direct
calls and explicit caller dependencies rooted in the qualified profile.
Every original word and terminal delay slot matches ASM and resident RAM.
Call continuations assume returns. Five bodies end with a register jump whose
original in-body constant selects `0xA0`, `0xB0` or `0xC0`. BIOS behavior and
returns remain unverified. Two indirect calls retain unqualified targets and
selectors. Matching callee ELF and observed execution are not admission claims.

| File interval | Runtime interval | Bytes | Original JAL | Qualified caller |
| --- | --- | ---: | --- | --- |
| `[0xB56B4,0xB573C)` | `[0x8014B6B4,0x8014B73C)` | 136 | `0x8014ACD0` | `0x8014ACA0` |
| `[0xE8104,0xE8110)` | `[0x8017E104,0x8017E110)` | 12 | `0x8017E0BC` | `0x8017E0B4` |
| `[0xE8224,0xE8290)` | `[0x8017E224,0x8017E290)` | 108 | `0x8017E0C4` | `0x8017E0B4` |
| `[0xE82B4,0xE831C)` | `[0x8017E2B4,0x8017E31C)` | 104 | `0x8017E0CC` | `0x8017E0B4` |
| `[0xE8CDC,0xE8CE8)` | `[0x8017ECDC,0x8017ECE8)` | 12 | `0x8014AF3C` | `0x8014AEE0` |
| `[0xE8CEC,0xE8CF8)` | `[0x8017ECEC,0x8017ECF8)` | 12 | `0x8017E274` | `0x8017E224` |
| `[0xE91CC,0xE91FC)` | `[0x8017F1CC,0x8017F1FC)` | 48 | `0x801746E8` | `0x801746E0` |
| `[0xE937C,0xE9388)` | `[0x8017F37C,0x8017F388)` | 12 | `0x8017F1DC` | `0x8017F1CC` |
| `[0xE93AC,0xE93B8)` | `[0x8017F3AC,0x8017F3B8)` | 12 | `0x8017F294` | `0x8017F27C` |

These further complete bodies qualify through original direct calls from the
qualified profile. Every raw original word matches the managed decoder and
retained resident RAM. Review closes all branches, internal jumps, return paths
and delay slots. Each listed incoming call is reachable in its qualified caller;
no call from the boot loop's dead epilogue supplies an entry.
Call continuations assume returns with the caller stack preserved. Eleven
indirect calls retain unqualified targets and behavior. Matching callee ELF and
observed execution are not admission claims. The containing Splat bins, referenced
data, neighboring bodies and unreviewed coprocessor or trap paths remain outside
these intervals.

| File interval | Runtime interval | Bytes | Original JAL | Qualified caller |
| --- | --- | ---: | --- | --- |
| `[0xB4FC0,0xB5020)` | `[0x8014AFC0,0x8014B020)` | 96 | `0x8014AB60` | `0x8014AAC8` |
| `[0xB80A8,0xB80FC)` | `[0x8014E0A8,0x8014E0FC)` | 84 | `0x8014E154` | `0x8014E0FC` |
| `[0xB822C,0xB8284)` | `[0x8014E22C,0x8014E284)` | 88 | `0x8014AB40` | `0x8014AAC8` |
| `[0xB8564,0xB85A0)` | `[0x8014E564,0x8014E5A0)` | 60 | `0x8014ACF4` | `0x8014ACA0` |
| `[0xB86D0,0xB87F8)` | `[0x8014E6D0,0x8014E7F8)` | 296 | `0x8014AB48` | `0x8014AAC8` |
| `[0xC6EBC,0xC7044)` | `[0x8015CEBC,0x8015D044)` | 392 | `0x8014AC00` | `0x8014AAC8` |
| `[0xC7044,0xC70B8)` | `[0x8015D044,0x8015D0B8)` | 116 | `0x8014AB68` | `0x8014AAC8` |
| `[0xCD010,0xCD05C)` | `[0x80163010,0x8016305C)` | 76 | `0x8014AC48` | `0x8014AAC8` |
| `[0xD30B8,0xD30EC)` | `[0x801690B8,0x801690EC)` | 52 | `0x801628BC` | `0x80162898` |
| `[0xD98F8,0xD9964)` | `[0x8016F8F8,0x8016F964)` | 108 | `0x8015E22C` | `0x8015DF18` |
| `[0xDD818,0xDD89C)` | `[0x80173818,0x8017389C)` | 132 | `0x801628E8` | `0x80162898` |
| `[0xDDC50,0xDDC80)` | `[0x80173C50,0x80173C80)` | 48 | `0x8016292C` | `0x80162898` |
| `[0xDE668,0xDE6B8)` | `[0x80174668,0x801746B8)` | 80 | `0x8014ADE0` | `0x8014AD28` |
| `[0xDE848,0xDE8E4)` | `[0x80174848,0x801748E4)` | 156 | `0x80174790` | `0x80174700` |
| `[0xDE8E4,0xDE914)` | `[0x801748E4,0x80174914)` | 48 | `0x8014AD30` | `0x8014AD28` |
| `[0xDF3C4,0xDF3DC)` | `[0x801753C4,0x801753DC)` | 24 | `0x8014AD64` | `0x8014AD28` |
| `[0xDF3DC,0xDF3EC)` | `[0x801753DC,0x801753EC)` | 16 | `0x8017A540` | `0x8017A514` |
| `[0xDF534,0xDF5A0)` | `[0x80175534,0x801755A0)` | 108 | `0x801753FC` | `0x801753EC` |
| `[0xDF640,0xDF660)` | `[0x80175640,0x80175660)` | 32 | `0x8016206C` | `0x80161FDC` |
| `[0xDF660,0xDF680)` | `[0x80175660,0x80175680)` | 32 | `0x80162258` | `0x80162230` |
| `[0xDF680,0xDF698)` | `[0x80175680,0x80175698)` | 24 | `0x8016213C` | `0x80161FDC` |
| `[0xDF698,0xDF6B0)` | `[0x80175698,0x801756B0)` | 24 | `0x8016212C` | `0x80161FDC` |
| `[0xDF6B0,0xDF7E8)` | `[0x801756B0,0x801757E8)` | 312 | `0x8014E140` | `0x8014E0FC` |
| `[0xDF914,0xDFA58)` | `[0x80175914,0x80175A58)` | 324 | `0x8014E1D0` | `0x8014E0FC` |
| `[0xDFA78,0xDFA98)` | `[0x80175A78,0x80175A98)` | 32 | `0x80162274` | `0x80162230` |
| `[0xDFBE0,0xDFC60)` | `[0x80175BE0,0x80175C60)` | 128 | `0x8016227C` | `0x80162230` |
| `[0xE1348,0xE1614)` | `[0x80177348,0x80177614)` | 716 | `0x8014E12C` | `0x8014E0FC` |
| `[0xE22E4,0xE22FC)` | `[0x801782E4,0x801782FC)` | 24 | `0x80175438` | `0x801753EC` |
| `[0xE4F0C,0xE5090)` | `[0x8017AF0C,0x8017B090)` | 388 | `0x8014AD5C` | `0x8014AD28` |
| `[0xE52B4,0xE52C4)` | `[0x8017B2B4,0x8017B2C4)` | 16 | `0x8017A594` | `0x8017A514` |
| `[0xE52D4,0xE5330)` | `[0x8017B2D4,0x8017B330)` | 92 | `0x8014AAF8` | `0x8014AAC8` |
| `[0xE5A40,0xE5B44)` | `[0x8017BA40,0x8017BB44)` | 260 | `0x8014AB38` | `0x8014AAC8` |
| `[0xE5C98,0xE6140)` | `[0x8017BC98,0x8017C140)` | 1192 | `0x8014AB28` | `0x8014AAC8` |
| `[0xE7F74,0xE7FA0)` | `[0x8017DF74,0x8017DFA0)` | 44 | `0x8017B384` | `0x8017B330` |

The following bodies use the same raw-original admission basis. Their direct
calls are rooted in the qualified profile, with dependencies of at most two
new-body edges from the earlier matched or conditional bodies. Every instruction,
branch, return and delay slot matches original bytes, the managed decoder and
resident RAM. Call continuations assume returns with the caller stack preserved.
Ten indirect calls retain unqualified targets and behavior. No matching callee
ELF, observed execution or referenced-data classification follows.

| File interval | Runtime interval | Bytes | Original JAL | Qualified caller |
| --- | --- | ---: | --- | --- |
| `[0xB87F8,0xB88FC)` | `[0x8014E7F8,0x8014E8FC)` | 260 | `0x8014E774` | `0x8014E6D0` |
| `[0xB88FC,0xB8A80)` | `[0x8014E8FC,0x8014EA80)` | 388 | `0x8014E79C` | `0x8014E6D0` |
| `[0xCD05C,0xCD230)` | `[0x8016305C,0x80163230)` | 468 | `0x80163028` | `0x80163010` |
| `[0xCD8B0,0xCD9B8)` | `[0x801638B0,0x801639B8)` | 264 | `0x80163044` | `0x80163010` |
| `[0xD22E0,0xD2374)` | `[0x801682E0,0x80168374)` | 148 | `0x8015D070` | `0x8015D044` |
| `[0xD4D2C,0xD4D50)` | `[0x8016AD2C,0x8016AD50)` | 36 | `0x8015CF54` | `0x8015CEBC` |
| `[0xD5100,0xD51C4)` | `[0x8016B100,0x8016B1C4)` | 196 | `0x8015D020` | `0x8015CEBC` |
| `[0xD5A04,0xD5A24)` | `[0x8016BA04,0x8016BA24)` | 32 | `0x8015D028` | `0x8015CEBC` |
| `[0xD7EBC,0xD7FF4)` | `[0x8016DEBC,0x8016DFF4)` | 312 | `0x8015CEC8` | `0x8015CEBC` |
| `[0xD880C,0xD88A4)` | `[0x8016E80C,0x8016E8A4)` | 152 | `0x8015D018` | `0x8015CEBC` |
| `[0xD9514,0xD95B8)` | `[0x8016F514,0x8016F5B8)` | 164 | `0x8015D00C` | `0x8015CEBC` |
| `[0xD9698,0xD96B8)` | `[0x8016F698,0x8016F6B8)` | 32 | `0x8015D004` | `0x8015CEBC` |
| `[0xDD89C,0xDD918)` | `[0x8017389C,0x80173918)` | 124 | `0x8017385C` | `0x80173818` |
| `[0xDDCB0,0xDE08C)` | `[0x80173CB0,0x8017408C)` | 988 | `0x80173C64` | `0x80173C50` |
| `[0xDE6B8,0xDE6E0)` | `[0x801746B8,0x801746E0)` | 40 | `0x8014AFC8` | `0x8014AFC0` |
| `[0xE01E8,0xE0468)` | `[0x801761E8,0x80176468)` | 640 | `0x80175648` | `0x80175640` |
| `[0xE0468,0xE0734)` | `[0x80176468,0x80176734)` | 716 | `0x80175668` | `0x80175660` |
| `[0xE0734,0xE0B50)` | `[0x80176734,0x80176B50)` | 1052 | `0x8017573C` | `0x801756B0` |
| `[0xE0CB8,0xE0DAC)` | `[0x80176CB8,0x80176DAC)` | 244 | `0x80175578` | `0x80175534` |
| `[0xE0DAC,0xE0DFC)` | `[0x80176DAC,0x80176DFC)` | 80 | `0x8017554C` | `0x80175534` |
| `[0xE0DFC,0xE0FE8)` | `[0x80176DFC,0x80176FE8)` | 492 | `0x8017555C` | `0x80175534` |
| `[0xE1154,0xE1254)` | `[0x80177154,0x80177254)` | 256 | `0x80175A80` | `0x80175A78` |
| `[0xE1614,0xE1634)` | `[0x80177614,0x80177634)` | 32 | `0x80177540` | `0x80177348` |
| `[0xE1634,0xE1914)` | `[0x80177634,0x80177914)` | 736 | `0x80177380` | `0x80177348` |
| `[0xE1914,0xE19BC)` | `[0x80177914,0x801779BC)` | 168 | `0x80177418` | `0x80177348` |
| `[0xE19BC,0xE1C68)` | `[0x801779BC,0x80177C68)` | 684 | `0x801774AC` | `0x80177348` |
| `[0xE2138,0xE2218)` | `[0x80178138,0x80178218)` | 224 | `0x8014E0C0` | `0x8014E0A8` |
| `[0xE2218,0xE22E4)` | `[0x80178218,0x801782E4)` | 204 | `0x8014E0CC` | `0x8014E0A8` |
| `[0xE5560,0xE55F4)` | `[0x8017B560,0x8017B5F4)` | 148 | `0x8014E588` | `0x8014E564` |
| `[0xE5690,0xE56F4)` | `[0x8017B690,0x8017B6F4)` | 100 | `0x8014E264` | `0x8014E22C` |
| `[0xE6548,0xE67D8)` | `[0x8017C548,0x8017C7D8)` | 656 | `0x8017BA94` | `0x8017BA40` |
| `[0xE6A90,0xE6B40)` | `[0x8017CA90,0x8017CB40)` | 176 | `0x8017BD00` | `0x8017BC98` |

These additional bodies have acyclic caller dependencies rooted in the preceding
profile. Every original instruction, branch, terminal delay and return path
matches the managed decoder and retained RAM. Call continuations assume ordinary
returns with saved return storage preserved. Seven indirect calls retain
unqualified targets and behavior. Twenty-three bodies end with register jumps
bound by original in-body constants to external BIOS entries. BIOS behavior and
returns remain unverified.

The bodies at `0x8017E16C..0x8017E1F4` and `0x8017F3E4..0x8017F44C` save and
restore incoming return addresses through absolute words `0x8018B2CC` and
`0x8018B2EC`. Their original SP is unchanged. Conditional return identity assumes
that calls and dynamic stores preserve each word, with no aliasing write or
reentrant execution. The words themselves remain unclassified. These are
stored-instruction facts, without matching callee ELF, observed entry, valid
arguments, callee behavior or resource-safety acceptance.

| File interval | Runtime interval | Bytes | Original JAL | Qualified caller |
| --- | --- | ---: | --- | --- |
| `[0xCDE88,0xCDEA0)` | `[0x80163E88,0x80163EA0)` | 24 | `0x80163974` | `0x801638B0` |
| `[0xD2960,0xD2BF0)` | `[0x80168960,0x80168BF0)` | 656 | `0x8016F47C` | `0x8016F378` |
| `[0xD2CE0,0xD2D28)` | `[0x80168CE0,0x80168D28)` | 72 | `0x8016ED38` | `0x8016E8A4` |
| `[0xD2E68,0xD2E94)` | `[0x80168E68,0x80168E94)` | 44 | `0x80168B48` | `0x80168960` |
| `[0xD2E94,0xD2EC0)` | `[0x80168E94,0x80168EC0)` | 44 | `0x80168B38` | `0x80168960` |
| `[0xD3094,0xD30B8)` | `[0x80169094,0x801690B8)` | 36 | `0x8016BA60` | `0x8016BA24` |
| `[0xD4B8C,0xD4D08)` | `[0x8016AB8C,0x8016AD08)` | 380 | `0x8016AD38` | `0x8016AD2C` |
| `[0xD5A24,0xD5AA0)` | `[0x8016BA24,0x8016BAA0)` | 124 | `0x8016BA0C` | `0x8016BA04` |
| `[0xD88A4,0xD8D9C)` | `[0x8016E8A4,0x8016ED9C)` | 1272 | `0x8016E88C` | `0x8016E80C` |
| `[0xD8E1C,0xD8EA8)` | `[0x8016EE1C,0x8016EEA8)` | 140 | `0x8016E944` | `0x8016E8A4` |
| `[0xD8EA8,0xD9378)` | `[0x8016EEA8,0x8016F378)` | 1232 | `0x8016ED00` | `0x8016E8A4` |
| `[0xD9378,0xD9514)` | `[0x8016F378,0x8016F514)` | 412 | `0x8016ED20` | `0x8016E8A4` |
| `[0xD95C8,0xD9698)` | `[0x8016F5C8,0x8016F698)` | 208 | `0x8016F580` | `0x8016F514` |
| `[0xDAFE8,0xDB014)` | `[0x80170FE8,0x80171014)` | 44 | `0x80173CFC` | `0x80173CB0` |
| `[0xDB014,0xDB02C)` | `[0x80171014,0x8017102C)` | 24 | `0x80173CE0` | `0x80173CB0` |
| `[0xDC9D0,0xDCF10)` | `[0x801729D0,0x80172F10)` | 1344 | `0x8016ABAC` | `0x8016AB8C` |
| `[0xDD918,0xDDC20)` | `[0x80173918,0x80173C20)` | 776 | `0x80173900` | `0x8017389C` |
| `[0xDE08C,0xDE354)` | `[0x8017408C,0x80174354)` | 712 | `0x80173F38` | `0x80173CB0` |
| `[0xDE914,0xDE944)` | `[0x80174914,0x80174944)` | 48 | `0x8016B180` | `0x8016B100` |
| `[0xDE944,0xDE974)` | `[0x80174944,0x80174974)` | 48 | `0x801690A0` | `0x80169094` |
| `[0xDE974,0xDE9A8)` | `[0x80174974,0x801749A8)` | 52 | `0x8016B14C` | `0x8016B100` |
| `[0xDEA38,0xDEA48)` | `[0x80174A38,0x80174A48)` | 16 | `0x8017631C` | `0x801761E8` |
| `[0xDEA60,0xDEA7C)` | `[0x80174A60,0x80174A7C)` | 28 | `0x8017DA64` | `0x8017DA54` |
| `[0xDF4F4,0xDF504)` | `[0x801754F4,0x80175504)` | 16 | `0x801781E0` | `0x80178138` |
| `[0xDF504,0xDF514)` | `[0x80175504,0x80175514)` | 16 | `0x8017801C` | `0x80177F1C` |
| `[0xDF524,0xDF534)` | `[0x80175524,0x80175534)` | 16 | `0x80177FD0` | `0x80177F1C` |
| `[0xDF5A0,0xDF5C0)` | `[0x801755A0,0x801755C0)` | 32 | `0x80178000` | `0x80177F1C` |
| `[0xDF7E8,0xDF914)` | `[0x801757E8,0x80175914)` | 300 | `0x80177F7C` | `0x80177F1C` |
| `[0xE0BD8,0xE0CB8)` | `[0x80176BD8,0x80176CB8)` | 224 | `0x80176300` | `0x801761E8` |
| `[0xE1C68,0xE1CD4)` | `[0x80177C68,0x80177CD4)` | 108 | `0x80177664` | `0x80177634` |
| `[0xE1CD4,0xE1D08)` | `[0x80177CD4,0x80177D08)` | 52 | `0x801777F8` | `0x80177634` |
| `[0xE1F1C,0xE20E8)` | `[0x80177F1C,0x801780E8)` | 460 | `0x80178200` | `0x80178138` |
| `[0xE5438,0xE5560)` | `[0x8017B438,0x8017B560)` | 296 | `0x8017B590` | `0x8017B560` |
| `[0xE67D8,0xE6830)` | `[0x8017C7D8,0x8017C830)` | 88 | `0x8017C5CC` | `0x8017C548` |
| `[0xE6830,0xE68FC)` | `[0x8017C830,0x8017C8FC)` | 204 | `0x8017C568` | `0x8017C548` |
| `[0xE68FC,0xE69C8)` | `[0x8017C8FC,0x8017C9C8)` | 204 | `0x8017C5A0` | `0x8017C548` |
| `[0xE69C8,0xE6A0C)` | `[0x8017C9C8,0x8017CA0C)` | 68 | `0x8017C5B4` | `0x8017C548` |
| `[0xE6A0C,0xE6A90)` | `[0x8017CA0C,0x8017CA90)` | 132 | `0x8017C5D8` | `0x8017C548` |
| `[0xE7A54,0xE7BB0)` | `[0x8017DA54,0x8017DBB0)` | 348 | `0x8017AFB4` | `0x8017AF0C` |
| `[0xE7E98,0xE7F74)` | `[0x8017DE98,0x8017DF74)` | 220 | `0x8017DB94` | `0x8017DA54` |
| `[0xE7FA0,0xE7FAC)` | `[0x8017DFA0,0x8017DFAC)` | 12 | `0x8014B31C` | `0x8014B1A4` |
| `[0xE8028,0xE807C)` | `[0x8017E028,0x8017E07C)` | 84 | `0x8014B30C` | `0x8014B1A4` |
| `[0xE807C,0xE80B4)` | `[0x8017E07C,0x8017E0B4)` | 56 | `0x8014B314` | `0x8014B1A4` |
| `[0xE80E4,0xE80F0)` | `[0x8017E0E4,0x8017E0F0)` | 12 | `0x8017E048` | `0x8017E028` |
| `[0xE80F4,0xE8100)` | `[0x8017E0F4,0x8017E100)` | 12 | `0x8017E08C` | `0x8017E07C` |
| `[0xE816C,0xE81F4)` | `[0x8017E16C,0x8017E1F4)` | 136 | `0x8017E050` | `0x8017E028` |
| `[0xE833C,0xE8348)` | `[0x8017E33C,0x8017E348)` | 12 | `0x80174898` | `0x80174848` |
| `[0xE8374,0xE8380)` | `[0x8017E374,0x8017E380)` | 12 | `0x80177968` | `0x80177914` |
| `[0xE8384,0xE8390)` | `[0x8017E384,0x8017E390)` | 12 | `0x8017761C` | `0x80177614` |
| `[0xE83C4,0xE83D0)` | `[0x8017E3C4,0x8017E3D0)` | 12 | `0x8017C114` | `0x8017BC98` |
| `[0xE83D4,0xE83E0)` | `[0x8017E3D4,0x8017E3E0)` | 12 | `0x8014AB18` | `0x8014AAC8` |
| `[0xE83E4,0xE83F0)` | `[0x8017E3E4,0x8017E3F0)` | 12 | `0x8017545C` | `0x801753EC` |
| `[0xE8CFC,0xE8D08)` | `[0x8017ECFC,0x8017ED08)` | 12 | `0x8017AFAC` | `0x8017AF0C` |
| `[0xE8D3C,0xE8D48)` | `[0x8017ED3C,0x8017ED48)` | 12 | `0x8014AD94` | `0x8014AD28` |
| `[0xE8D4C,0xE8D58)` | `[0x8017ED4C,0x8017ED58)` | 12 | `0x8016BA70` | `0x8016BA24` |
| `[0xE8D5C,0xE8D68)` | `[0x8017ED5C,0x8017ED68)` | 12 | `0x8016F4AC` | `0x8016F378` |
| `[0xE8D7C,0xE8D88)` | `[0x8017ED7C,0x8017ED88)` | 12 | `0x8014AD9C` | `0x8014AD28` |
| `[0xE8D8C,0xE8D98)` | `[0x8017ED8C,0x8017ED98)` | 12 | `0x8016BA80` | `0x8016BA24` |
| `[0xE8D9C,0xE8DA8)` | `[0x8017ED9C,0x8017EDA8)` | 12 | `0x8014B7C4` | `0x8014B73C` |
| `[0xE8DAC,0xE8DB8)` | `[0x8017EDAC,0x8017EDB8)` | 12 | `0x8014B8D8` | `0x8014B8B0` |
| `[0xE8DBC,0xE8DC8)` | `[0x8017EDBC,0x8017EDC8)` | 12 | `0x8014B810` | `0x8014B73C` |
| `[0xE8DCC,0xE8DD8)` | `[0x8017EDCC,0x8017EDD8)` | 12 | `0x801746C0` | `0x801746B8` |
| `[0xE8EBC,0xE8EC8)` | `[0x8017EEBC,0x8017EEC8)` | 12 | `0x8014ADF0` | `0x8014AD28` |
| `[0xE8EDC,0xE8EE8)` | `[0x8017EEDC,0x8017EEE8)` | 12 | `0x801748AC` | `0x80174848` |
| `[0xE9084,0xE9110)` | `[0x8017F084,0x8017F110)` | 140 | `0x80174698` | `0x80174668` |
| `[0xE91FC,0xE927C)` | `[0x8017F1FC,0x8017F27C)` | 128 | `0x8017F0C8` | `0x8017F084` |
| `[0xE938C,0xE9398)` | `[0x8017F38C,0x8017F398)` | 12 | `0x8017F0DC` | `0x8017F084` |
| `[0xE939C,0xE93A8)` | `[0x8017F39C,0x8017F3A8)` | 12 | `0x8017F254` | `0x8017F1FC` |
| `[0xE93E4,0xE944C)` | `[0x8017F3E4,0x8017F44C)` | 104 | `0x8017F0B0` | `0x8017F084` |

## Executable image-selector instruction coverage

The exact original US executable qualifies two complete instruction bodies.
Original START call `0x801E3FB8` conditionally enters the wrapper at
`0x8014D6B8`; its call at `0x8014D6C4` enters `0x8014D6DC`.

| File offsets | Runtime addresses | Bytes |
| --- | --- | ---: |
| `[751288,751324)` | `[0x8014D6B8,0x8014D6DC)` | 36 |
| `[751324,751828)` | `[0x8014D6DC,0x8014D8D4)` | 504 |

All original words, control transfers and delay slots reconcile with complete
structural flow and retained resident RAM. Ordinary external returns,
preserved frames, nonaliasing and stable code are conditions. These records
qualify instructions without requiring a matching ELF or establishing
execution, callee semantics, argument or resource safety, transfer or display.

## Executable program instruction coverage

The exact original US executable qualifies three complete instruction bodies.
The original call at `0x8014D8B0` conditionally enters `0x8014D910` with
selector 0 under the reviewed wrapper path. Its call at `0x8014D960`
conditionally enters `0x8014DB98`. The original GAME call at `0x801B0354` with
a NOP delay conditionally enters `0x8014D978` after the reviewed selected
substate handler returns. This caller binds selector cases 0, 1 and 2 only.

| File offsets | Runtime addresses | Bytes |
| --- | --- | ---: |
| `[751888,751992)` | `[0x8014D910,0x8014D978)` | 104 |
| `[751992,752164)` | `[0x8014D978,0x8014DA24)` | 172 |
| `[752536,752648)` | `[0x8014DB98,0x8014DC08)` | 112 |

Every original word and control-transfer delay slot matches retained resident
RAM. The bodies at `0x8014D910` and `0x8014D978` restore ordinary call/return
frames; `0x8014DB98` is a call-free leaf. Returning calls, valid disjoint
frames, nonaliasing and stable incoming work are conditions. These records
qualify instructions.
Matching ELF, actual execution, referenced field and resource semantics,
rendering and safe editing remain unqualified.

## Executable numeric-sampler instruction coverage

The exact original US executable qualifies these complete instruction bodies.
Original GAME `JAL` at `0x801BB774` calls the wrapper at `0x80154F78`.
Retained return observations at `0x801BB77C` bind that call arm. The wrapper's
original calls at `0x80154F94` and `0x80154FA4` reach the other two bodies.

| File offsets | Runtime addresses | Bytes |
| --- | --- | ---: |
| `[780988,782032)` | `[0x80154ABC,0x80154ED0)` | 1044 |
| `[782032,782120)` | `[0x80154ED0,0x80154F28)` | 88 |
| `[782120,782200)` | `[0x80154F28,0x80154F78)` | 80 |
| `[782200,782296)` | `[0x80154F78,0x80154FD8)` | 96 |

All aligned words, control transfers and delay slots reconcile with closed
whole-entry structural flow. Original bytes agree with the retained origin and
endpoint RAM. The rule assumes ordinary calls and returns, writable disjoint
frames, preserved saved registers and return storage, and stable code. It
qualifies instruction representation without establishing a matching ELF,
execution of every word, argument or resource safety, rendering or safe editing.
The same exact original GAME entry-0 gate qualifies three related instruction
bodies. Payload offsets are relative to the raw entry; archive offsets include
its 2048-byte prefix.

| Payload offsets | Archive offsets | Runtime addresses | Bytes |
| --- | --- | --- | ---: |
| `[154296,155376)` | `[156344,157424)` | `[0x801BB2B8,0x801BB6F0)` | 1080 |
| `[155376,155488)` | `[157424,157536)` | `[0x801BB6F0,0x801BB760)` | 112 |
| `[155488,155568)` | `[157536,157616)` | `[0x801BB760,0x801BB7B0)` | 80 |

Original call and return words, retained execution at `0x801BB77C` and
`0x801BB52C`, and the restored 24-byte frame bind execution inside the first
and third bodies. An entry-reachable original call at `0x801BB508` conditionally
enters the second. Complete structural flow covers every original word and
delay slot in each body. These records classify instructions under ordinary
call and return assumptions; they do not establish execution of every word,
incoming-register validity, sampler semantics, resource safety or rendering.

The separate original GAME caller `0x801C2538..0x801C2710` qualifies its
complete 472-byte instruction body at archive offsets `[185656,186128)`.
Its original call at `0x801C24D8` and retained execution inside the body with
return address `0x801C24E0` bind this instruction representation. Coverage
requires the exact original archive and entry-0 raw payload identity, load
address `0x80195800`, checkword `0x1E` and payload size 229720.

After an ordinary return from `0x80154F78`, a nonzero scratch-byte test in this
caller conditionally reaches `JAL 0x80154F28` at `0x801C26A0`. This qualifies
the additional 80-byte wrapper without proving its execution. The review covers
all words, branches, calls, terminal BREAK guards, returns and delay slots in
both bodies. Actual state-0 invocation, actor setup correctness, sampler
semantics and resource safety remain unqualified.

## SLUS dialogue interpreter instruction coverage

The exact original US executable gate also qualifies these complete instruction
bodies. File offsets include the 2048-byte header. Every original instruction
word and delay slot agrees with retained RAM. Structural review accounts for
all branches, local jumps, load schedules and saved frames.

| Runtime range | File offsets | Bytes |
| --- | --- | ---: |
| `[0x801503E0,0x80150470)` | `[762848,762992)` | 144 |
| `[0x801507BC,0x8015083C)` | `[763836,763964)` | 128 |
| `[0x8015083C,0x80150E14)` | `[763964,765460)` | 1496 |
| `[0x80150E48,0x80150E98)` | `[765512,765592)` | 80 |
| `[0x80150E98,0x80150F08)` | `[765592,765704)` | 112 |
| `[0x80150F08,0x80150F3C)` | `[765704,765756)` | 52 |

The original state-2 call at `0x80159568` enters `0x801503E0`, which calls the
outer dispatcher at `0x80150420`. Its finite selector alternatives 0 and 2
bind the parser and mode-2 dispatcher. The latter constructs two constant
targets selected by subselector 0 or 1. Neither dispatcher proves a general
index bound. The parser's five stored instructions at
`0x801509D0..0x801509E4` have an infeasible unsigned-byte-equals-`0x100`
incoming condition. They remain stored instructions without an execution claim.

Classification assumes valid memory and cursors, intact disjoint frames,
nonaliasing, stable code and selectors, and ordinary external returns with ABI
preservation. Frames are restored on return; the parser has a backedge without
a finite-termination proof. Scripts, tables, task fields, external callees and
neighboring bytes retain their prior status. These records do not establish
an action-60 script supplier, selected-task identity, latch release, invocation,
completion, resource safety, rendering, safe editing or an ELF match.

## GAME work and event instruction coverage

The exact original GAME entry-0 gate qualifies these complete instruction bodies.
Each range requires the original archive identity, raw transfer mode, load address
`0x80195800`, checkword `0x1E`, payload size 229720 and payload SHA-256
`1663ff2e78134fe15f3cae6fb4074acf14b68b79cfa02be0c7651eaf6a1728ff`.

| Runtime range | Payload offsets | Archive offsets | Bytes |
| --- | --- | --- | ---: |
| `[0x801A17A0,0x801A18C8)` | `[49056,49352)` | `[51104,51400)` | 296 |
| `[0x801A4914,0x801A4990)` | `[61716,61840)` | `[63764,63888)` | 124 |
| `[0x801A4990,0x801A4AA8)` | `[61840,62120)` | `[63888,64168)` | 280 |
| `[0x801A4AA8,0x801A4B80)` | `[62120,62336)` | `[64168,64384)` | 216 |
| `[0x801A4D54,0x801A50C0)` | `[62804,63680)` | `[64852,65728)` | 876 |
| `[0x801A50C0,0x801A5400)` | `[63680,64512)` | `[65728,66560)` | 832 |
| `[0x801A65A0,0x801A68C0)` | `[69024,69824)` | `[71072,71872)` | 800 |
| `[0x801A6F58,0x801A72B0)` | `[71512,72368)` | `[73560,74416)` | 856 |
| `[0x801A7330,0x801A7704)` | `[72496,73476)` | `[74544,75524)` | 980 |
| `[0x801C1254,0x801C1400)` | `[178772,179200)` | `[180820,181248)` | 428 |

Structural review covers every original word, local branch, call, terminal BREAK
guard, return, delay slot and frame save/restore sequence. All instruction bytes
match retained RAM. Original dispatcher arms for types 0, 1, 5, 6 and 9 bind
the five constructors and their calls to the smaller work helpers. Type 6 calls
`0x801C1254` at `0x801A660C`. Type 9 can deactivate the selected slot and
return without incrementing the counter, depending on its gate and record.
The pool caller at `0x801A0790` passes an actor's subrecord at offset `0x74`
to `0x801A17A0`.
The reader's field at offset 8 therefore refers to the actor's field at `0x7C`.
The caller prefix itself is not classified by this rule.

Classification assumes valid record, work, scratch and table memory, intact
frames, nonaliasing, stable code/data and ordinary callee returns with ABI
preservation. The constructors additionally require a nonnegative slot below
30; their signed upper-bound test alone does not establish that domain.
Referenced data, event scripts, external callees and neighboring bytes retain
their prior status. These instruction records do not establish invocation,
complete field initialization, event triggers, work lifetime, resource safety,
rendering, safe editing or an ELF match.
The terminal BREAK guards do not qualify exception-handler behavior.

## GAME record-setter instruction coverage

The exact original GAME entry-0 gate qualifies the complete 72-byte instruction
body at `0x801A72E8..0x801A7330`, payload offsets `[72424,72496)` and archive
offsets `[74472,74544)`. All eighteen original words, three control-transfer
delays and load schedules agree with retained RAM. This call-free body preserves
SP, RA and saved registers.

The complete original caller at `0x801A3F3C` supplies an ordinary record cursor
to `0x801A4BF8` through its call at `0x801A3F80`. The dispatcher's high-nibble-D
arm calls the setter at `0x801A4C48`, with `A0 = S0` in the delay word. Valid
record and current-work memory, return control, nonaliasing and stable code/data
remain conditions. Input and work fields, tables, caller bodies and neighbors
retain their prior status. An actual D directive selecting callback `0x0C`,
eligible task, callback lifetime, invocation, ordinary menu route, matching ELF,
argument or resource safety, rendering and safe editing remain unverified.

## COMMU00 callback instruction coverage

Original US `COMMU00.EMI#0` qualifies the complete 64-byte instruction body at
`0x801F15F8..0x801F1638`, payload offsets `[10744,10808)` and archive offsets
`[12792,12856)`. The entry must be raw mode at archive offset 2048, load at
`0x801EEC00`, have checkword `0x18` and contain 15696 bytes. The whole archive
must match the original disc manifest, and the payload must have SHA-256
`27e06df900c8809cc941f2086f0126cff1c951039ad6e63d6e3c21c6fd708f7a`.

All sixteen original words, two control-transfer delays, both load delays and
the restored 24-byte frame agree with the complete sized executable ELF
function. All seven object text relocations reproduce the original bytes.
Ordinary helper return and ABI preservation, valid incoming task memory and
return control, an intact frame, nonaliasing and stable code remain conditions.
Referenced data, the external helper and neighboring bodies retain their prior
status. A source-bound directive, eligible task, residency, callback lifetime,
invocation, helper effects, argument or resource safety, rendering and safe
editing remain unverified.

## GAME sprite instruction coverage

The original US GAME entry-0 gate also qualifies the complete instruction body
at `0x801AF2A0..0x801AF390`. Its payload offsets are `[105120,105360)` and
archive offsets are `[107168,107408)`. All 60 original instruction words,
eight control-transfer delays, retained resident bytes and the existing sized
linked ELF section agree. All six executable relocations reproduce those bytes.
The existing objects were not rebuilt for this classification.

Nine original JAL sites in the qualified shared battle body
`0x800B1A74..0x800B1DAC` conditionally enter this body. Ordinary callee returns,
preserved registers, valid memory, intact frames and nonaliasing stores remain
conditions. Referenced data, tables, pointers, buffers and callees remain
excluded. Invocation, feasible execution, callee behavior, resource safety,
safe editing and unique source ownership remain unqualified.

The preceding leaf at `0x801AF270..0x801AF2A0` also qualifies. Its payload
offsets are `[105072,105120)` and archive offsets are `[107120,107168)`.
All 12 original words, three control-transfer delays, retained RAM bytes,
existing sized executable ELF section and five executable relocations agree.
The existing objects were not rebuilt. The qualified sprite body has one
original JAL to this leaf at `0x801AF2CC`.

Both return paths preserve SP, RA and saved registers without memory loads or
stores. Stable executable bytes and valid incoming return control remain
conditions. Linked table symbols establish relocation arithmetic; table extents,
contents, returned-pointer validity and consumer safety remain unqualified.
Invocation, runtime feasibility, resource safety, safe editing and source
semantics also remain unqualified.

## Guarded executable switch coverage

For the exact original US `SLUS_004.22`, coverage recognizes the complete
instruction body `0x80175C60..0x801761E8` and its separate five-word table at
`0x8014A354..0x8014A368`. A qualified original call at `0x80176344` enters the
body. The unsigned selector-minus-one guard accepts indices 0 through 4 before
the scaled table load and register jump. All five original targets remain
inside the complete body, and the load and jump delay slots reconcile.

The 1416 instruction bytes and 20 numeric table bytes have separate records.
Call continuations assume ordinary returns with the caller's stack and saved
return storage preserved. Dynamic stores must not alias that storage. This
classification does not establish actual execution, table immutability,
hardware progress, argument validity or callee and resource semantics.

## Executable geometry table coverage

Coverage recognizes the 32-byte numeric table at `0x8017F53C..0x8017F55C`
(file offsets `[955708,955740)`) in the exact original US `SLUS_004.22`.
The original consumer at `0x8014C7F0..0x8014C818` masks its index to 0–15,
doubles it and reads an unsigned width/height pair. The sixteen pairs combine
widths and heights of 8, 16, 24 and 32 pixels.

This qualifies the stored numeric representation. Runtime use, callee effects,
rectangle behavior, mutable tables and other geometry tables remain unverified.

## Executable instruction coverage

Coverage recognizes file offsets `[3928,5756)` in the exact original US
`LOGO/LOGO.EXE`, with size 122880 and SHA-256
`21b3f791e450ff308a874e3691d0c0c88df428464f9ad08ea2c7a680a6c0dd2c`.
These 1828 bytes correspond to runtime addresses `[0x801CE758,0x801CEE7C)` and
twelve complete instruction-only function bodies. Each body matches its freshly
compiled executable ELF function symbol and reviewed source boundaries.

The 32-byte `CdFlush` wrapper at file offsets `[9700,9732)`, runtime
`[0x801CFDE4,0x801CFE04)`, matches the complete SDK executable section in
`libcd.a` member `s_008.o` after one call relocation to `CD_flush` at
`0x801D141C`. The original caller and call/return boundaries qualify the wrapper;
its callee remains unresolved.

The 52-byte `CdComstr` body at file offsets `[9756,9808)`, runtime
`[0x801CFE1C,0x801CFE50)`, matches `libcd.a` member `s_010.o` after all five
relocations. Both branches join at its return and delay slot. This extent
excludes twelve trailing SDK section-padding bytes; the SDK symbol has no
function type or size. No direct caller is identified in LOGO. The referenced
table, fallback string and preceding `CdSetDebug` gap remain unresolved.

The same exact identity qualifies the 32-byte `CdMix` wrapper at file offsets
`[10908,10940)`, runtime `[0x801D029C,0x801D02BC)`. Its complete SDK executable
section from `libcd.a` member `s_020.o` matches after relocating its call to
`CD_vol` at `0x801D1394`. Original callers bind the typed `CdlATV` argument.
This proof uses the SDK section, relocation and original call/return sequence;
the SDK symbol has no function type or size and is not a compiled ELF function
extraction.

The 136-byte `CD_vol` body at file offsets `[15252,15388)`, runtime
`[0x801D1394,0x801D141C)`, matches `bios_1.o` after all fourteen HI16/LO16
relocations. Its 34 straight-line instructions end with a return and delay slot
immediately before the SDK's `CD_flush` symbol. The original `CdMix` caller
preserves the typed argument. The referenced CD-register pointer words also
match the SDK data, but remain outside this instruction classification.

The same exact identity qualifies the complete 2416-byte original body
`[0x801D263C,0x801D2FAC)`, file offsets `[20028,22444)`. The already
qualified caller at `0x801CECA4` conditionally calls it at `0x801CECBC`
when the u32 at `0x801EB678` is nonzero. All 604 words, including delay
slots, form complete structural control flow. Call continuations assume ordinary
returns and intact frames. The indirect calls at `0x801D2B98` and `0x801D2C44`
use the guarded pointer at `0x801EB674`; its targets and resource safety remain
unqualified. This instruction-body rule requires neither a matching callee ELF
nor observed execution. The header-copy loop leaves CAPCOM `H+12`, at sector
offset 20, opaque.

Under the same gate, the complete 52-byte body `[0x801D2FAC,0x801D2FE0)`,
file offsets `[22444,22496)`, qualifies through the original call at
`0x801D2878` in that caller. The call passes count eight when the u32 at
`0x801ECEA4` is nonzero. All thirteen words, including two conditional branches
and three delay slots, form complete structural control flow. Argument validity,
memory access, return integrity, aliasing and stable code remain assumptions.
The copied bytes and pointed storage remain outside this instruction rule.

Only these instruction bytes become known nontext. Surrounding retained ranges,
tables and other executable bytes do not inherit the classification. Runtime
execution, mutable-pointer stability, rendering and complete text coverage remain
unverified.

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
start. Coverage identifies `[2336*s+8, 2336*s+12)` as packet metadata when
the Form 1 video subheader and exact `60 01 01 80` marker/type bytes match.

Under the same source and packet gates, coverage identifies numeric packet
index/count words and a frame ID at `[2336*s+12, 2336*s+20)`. Original LOGO
reads at `0x801D2AF8`, `0x801D2E04` and `0x801D2E08` consume the two u16
words; `0x801D2E10` compares the index with count minus one. Paired-header
reads at `0x801CEA58` and `0x801CEAF4` consume the complete u32 frame ID.
The other packets in each frame carry exact copies of that ID. Existing
index, count and frame checks remain required. Header bytes `[20,40)` within
each sector stay unresolved; their values do not establish coded length.

Coverage also identifies conservative coded prefixes in CAPCOM frames 1 through 221.
Their original LOGO consumer at `0x801D7578` reads fresh input and returns zero
with its next unread halfword in `a0`. Each reviewed prefix excludes the final
eight fetched bytes to cover unread lookahead. The original packet index, count
and frame ID bind each prefix to ordered 2016-byte bodies at sector offset 40.
Coverage admits 1,607,660 bytes from those observed frontiers.

For frames 222 through 231, coverage identifies 2704 bytes per frame as exact
packet-bound copies of the qualified frame 221 prefix. Their first 2712 assembled
bytes match, including eight lookahead bytes that remain excluded. This is a
stored-format classification; later acquisition and decoder entry or return
remain unverified. The initial twelve bytes contain eight header bytes and four
initially cached coded bytes.

Across both evidence bases, 403,476 video-body bytes remain unresolved, including
excluded lookahead and tails. These bounds establish neither each frame's logical
end nor whole-movie playback.

Static format identification does not establish original sector selection,
arrival, playback, command completion, predictor history or hardware handling
of parameter copies.
Form 1 user payloads outside the reviewed empty payloads, packet marker/type
and numeric fields, and reviewed coded prefixes remain unresolved.
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

The JSON schema is `bof3.executable-text/v18`. Re-extract older documents before
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
reachability through every reader of that word. Command and interrupt table
labels also have direct diagnostic `%s` paths to BIOS printf. Their literal
fields allow percent bytes. Preserve all recorded pointer aliases, including
the twelve references to the shared `?` field, and the original `CdlBackword`
spelling. The stored tables contain 32 and eight pointer words; coverage
classifies those 160 bytes as known nontext. The getters expose only 28 and
seven entries. Raw diagnostic indexes prevent a closed `%s`
argument domain or formatted size bound. The separate `none` getter fallback
remains unsupported because no downstream byte consumer is qualified.

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

Build recognition pins every archive byte outside these name fields and the five
supported category allocations. Extract names with `--layout record-names`.
Packing a name document changes only name fields and also checks the complete current source SHA-256, canonical source path and all
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

Extract the selected entry as `bof3.inline-text/v32`. Its first eighteen strings include
the ten counted labels at `0x801D0C7C`, with stride 20, followed by these eight
names at indices 10 through 17. The label consumer calls `0x8014FC90` at
`0x801DCF50`, using the same count convention as the BATE, SHISU and SHOP tables.
The formation builder at `0x801DAC1C` selects counted rows by active party size.
The duplicate labels belong to separate groups:

| Party members | String indices | Original labels |
| --- | --- | --- |
| 1 | 0 | Normal |
| 2 | 1 through 3 | Normal, Attack, Defense |
| 3 | 4 through 9 | Normal, Attack, Defense, Chain, Magic, Refuge |

Each row reserves its eighth byte for the encoded length and permits seven
literal character bytes. The other twelve bytes of each twenty-byte record stay
protected. Packed-media observations verify all three groups under controlled
party and formation availability. Natural availability and unique START or STATUS
supply remain unverified.

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
Packed START media verifies all four labels edited to `GO`, `Cut`, `More` and
`Prev` during new-game name entry. The first ordinary cell, edited from `A` to
`Z`, renders in both the keyboard and entered name. START confirmation copies
the normalized five-byte name to the main character record at `0x80144968`.
Other cells, pages, STATUS rendering, other contexts and memory-card persistence
remain unverified.

The next fifteen strings expose four stat-label groups through `game-literal`.
The groups start at `0x801EC940`, `0x801EC99C`, `0x801ECB0C` and `0x801ECB2C`.
The first three contain `Pwr`, `Def`, `Int` and `Agl`; the last contains `Pwr`,
`Def` and `Agl`. The first group uses four-byte records and the others use
six-byte records. Each editable allocation is four bytes, including NUL, and
accepts up to three encoded bytes. The two trailing bytes of six-byte records
stay protected. The fixed-address consumers call `0x8014F800` at
`0x801DBC88`, `0x801DBCFC`, `0x801DBD64`, `0x801DBDC0`, `0x801DC90C`,
`0x801DC934`, `0x801DC95C`, `0x801DC978`, `0x801DE6EC` and `0x801DF1AC`.
These stat labels have no pointer-table references. Controlled Status and
Equipment member selection verify edited labels in the groups at `0x801EC940`
and `0x801ECB0C`, respectively. Identical START and STATUS payloads leave the
active supplier indistinguishable. Other groups and natural menu availability
remain unverified.

The numeric format exposes the four-byte `%3d` allocation at `0x801D0C4C` through
`ascii-u16-decimal-format`. Its eighteen formatter sites read one unsigned byte
or halfword, write to the 64-byte scratch buffer at `0x80145AD4`, then draw through
`0x80150098`. Retain one decimal argument, at most three encoded bytes plus NUL,
and at most five formatted characters. Format widths use 1 through 4. The next
allocation at `0x801D0C50` stays protected. Controlled Equipment member
selection verifies decimal width 2 in this stat panel. Other numeric paths and
values remain unverified.

The item-category labels ITEM, WEAPON, ARMOR, OPTION and VITAL occupy
`0x801ECC3C`, `0x801ECC44`, `0x801ECC4C`, `0x801ECC54` and `0x801ECC5C`.
Their capacities are five, seven, six, seven and six bytes including NUL.
The reader at `0x801DFBF4` selects pointers from `0x801ECC64` using the panel
category byte and draws through `0x8014F800` at `0x801DFF24`. Keep the nine
alignment bytes and twenty pointer bytes protected. Controlled packed-media
Items observations verify all five edited category labels. A preinstruction capture
at `0x801DFF24` binds `V0=0x801ECC3C`, slot `S0=0x801ECC64` and record
`S1=0x80148504`, with handler 6, type 12, callback state 2 at `+3`, reader byte
zero at `+8` and category zero at `+A`. This controlled capture stops before the
JAL and delay store. WEAPON, ARMOR, OPTION and VITAL use separate controlled
records with categories 1 through 4. These observations verify category-label
glyphs only.

Ordinary Items navigation after loading a game-created save also displays all
five original labels. The regular list reaches ITEM, WEAPON, ARMOR and OPTION;
the Vtal command opens VITAL. Selected calls at `0x801DFF24` bind categories
0 through 4 to their respective pointers and continue to `0x8014F800` with return
address `0x801DFF2C` and the selected pointer stored at `SP + 0x10`. This verifies
each selected JAL and delay store. Row reads and completed rendering for those
invocations, general selector bounds and the unique START or STATUS supplier
remain unverified.

The skill-category labels HEAL, ASSIST, ATTACK and SKILL occupy
`0x801ECC78`, `0x801ECC80`, `0x801ECC88` and `0x801ECC90`, with capacities
five, seven, seven and six bytes including NUL. The reader at `0x801E0354`
selects from `0x801ECC98` using the panel's byte at offset `0xB` and draws at
`0x801E05A0`. Its length call uses the resident A0/1B BIOS stub. Keep the seven
alignment bytes and sixteen pointer bytes protected. Controlled Ability rendering
verifies first-glyph edits in all four labels at selectors 0 through 3.
Preinstruction captures at `0x801E05A0` bind record `S1=0x80148570` to slot
`0x801ECC98` and pointer `V0=0x801ECC78` for HEAL, and slot `0x801ECCA4` and
pointer `V0=0x801ECC90` for SKILL. Both callback states at `+3` are zero; the reader
byte at `+8` is zero for HEAL and three for SKILL. These controlled captures stop
before the JAL and delay store.

Ordinary Ability navigation after loading a game-created save also displays all
four original labels. Selected calls at `0x801E05A0` bind category values 0 through
3 at record byte `+B` to the four respective pointers. At these calls,
`AT - 0x3368` identifies the pointer slot; `S0` holds a shifted centering
coordinate. Each continues to `0x8014F800` with return address `0x801E05A8` and
its text pointer stored at `SP + 0x10`, verifying the selected JAL and delay
store. Row reads and completed rendering for those invocations, general selector
bounds and the unique START or STATUS supplier remain unverified.

The separate equipment-category table at `0x801ECCD0` exposes WEAPON, ARMOR
and OPTION at `0x801ECCB0`, `0x801ECCB8` and `0x801ECCC0`, with capacities
seven, six and seven bytes including NUL. The reader at `0x801E0A70` assigns
category values 1, 2 or 3 from the panel's byte at offset 9, then draws through
`0x8014F800` at `0x801E0E2C`. These local assignments do not establish general
transitive callee behavior. ITEM and VITAL in this table
stay protected because their selected consumers are unresolved. Keep all nine
alignment bytes and twenty pointer bytes protected. Controlled Equipment
rendering verifies first-glyph edits in WEAPON, ARMOR and OPTION at observed
selectors 0, 1 and 4, with categories 1, 2 and 3 and callback state zero. These
captures follow menu inputs from the controlled availability fixture.

Ordinary Equipment navigation after loading a game-created save also displays
the original WEAPON, ARMOR and OPTION labels at selectors 0, 1 and 5. Selected
calls at `0x801E0E2C` supply pointers `0x801ECCB0`, `0x801ECCB8` and
`0x801ECCC0`, respectively. Each continues to renderer `0x8014F800` with return
address `0x801E0E34` and its text pointer stored at `SP + 0x10`, verifying the
selected JAL and delay store. Row reads and completed rendering for those
invocations, general selector bounds and the unique START or STATUS supplier
remain unverified.

The settings reader at `0x801E2C00` exposes fifteen fixed-width fields from the
first fifteen eight-byte records at `0x801ECD24`. These contain Fast, Med., Slow,
two groups of 1 through 4, Stereo, Mono, Off and On. Each record retains its count
byte and signed horizontal offset. Edit only the field beginning two bytes into
the record, using `game-fixed-literal` and its recorded one-, four- or six-byte
capacity. The two draw paths at `0x801E2CDC` and `0x801E2D18` pass that count to
`0x8014F800` or `0x8014FC90`. A full field needs no NUL; shorter text is zero-filled.
The literal codec maps encoded byte `0x3E` in Med. to a period.

The caller at `0x801E26A8` supplies six local selectors. The first five groups
contain 3, 4, 4, 2 and 2 records; the sixth has count zero. Manual and Auto remain
protected because this path skips their group. Preserve all thirty count/position
bytes, fifty trailing bytes, sixteen Manual/Auto bytes and sixteen group-map
bytes. Controlled Config rendering of all fifteen edited fields is verified with
a menu availability fixture. All fifteen STATUS-only glyph edits also appear in
Config after booting from BIOS, loading an unchanged game-created save and
opening Config through the main menu. Differences from the matching control screen
are confined to the fifteen first-character regions; settings remain unchanged.

Original and edited ordinary Config captures bind calls at `0x801E2854` to
reader `0x801E2C00` for groups 0 through 5. Group 5 draws no fields. For groups 0
through 4, calls at `0x801E2D18` supply all fifteen field pointers at `SP + 0x10`
and their stored glyph limits in `A3`. These calls enter renderer `0x8014FC90`
with return address `0x801E2D20`. The observed reader and draw calls reach their
return continuations.

Ordinary Config navigation also selects the row for message speed without
changing settings. The selected original Med. field reaches `0x801E2CDC` and
enters renderer `0x8014F800` with return address `0x801E2CE4`; its edited form
follows the same path. Its text pointer `0x801ECD2E` is at `SP + 0x10`, and `A3`
holds its stored glyph limit of four.
The observed calls reach their return continuation. Other fields on this draw
path, glyph reads, pixels tied to individual calls, transitive callee effects,
general selector bounds, Manual/Auto consumption and the unique START or STATUS
supplier remain unverified.

Retain all 133 allocations and edit only `text`. Re-extract older inline
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
eight template records. The `name_copy_consumer` at `0x801DEA18` copies up to
eight bytes from the live battle record into a substitution buffer through
`0x801501E4`, which appends NUL. The first battle's Gary label reads the live
name through `0x801D8AE4` and renderer `0x8014FC90`.

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
Different archives remain separate sources. Controlled runtime checks verify
the `Gary` label from `AREA024.EMI` entry 11 edited to `Zary` in the first battle.
Other names, encounter families, regions and battle or substitution contexts
remain unverified.

## Inline overlay text

US `GAME.EMI` entry 0 also exposes five category labels through
`--entry 0 --layout inline`. The entry loads at `0x80195800`. Its table at
`0x801C7C5C` points to `ITEM`, `WEAPON`, `ARMOR`, `OPTION` and `VITAL` at
`0x801C7C34`, `0x801C7C3C`, `0x801C7C44`, `0x801C7C4C` and `0x801C7C54`.
The `0x80199D00` call passes the selected string to `0x8014F800`. These
`game-literal` allocations contain five, seven, six, seven and six bytes,
respectively, including NUL. Nine alignment bytes and the pointer table remain
protected. Recognition permits edits in both the category and name allocations;
each document's packer changes only its own fields. Natural selector bounds and
edited category rendering remain unverified.

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
checks cover the equipment-screen `WEAPON`, `ARMOR` and `OPTION` category labels.
Observed calls bind `WEAPON` to `0x801D8E98`, `ARMOR` to `0x801D8EA0` and
`OPTION` to `0x801D8EA8` through table `0x801D8EB8`.
The `OPTION` check uses a controlled cursor value of 4;
original code copies that value into the reader's input and selects category 3.
The reviewed controller path toggles the cursor with XOR 3. Natural availability
of `OPTION` and rendering of other category slots remain unverified.

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

`bof3.inline-text/v32` retains `source`, `source_sha256`, `layout`, `entry`,
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
| `0x801D0C04` | `%2d` | 4 | `ascii-i16-plus-one-decimal-format` |
| `0x801D0C08` | `%d/6` | 5 | `ascii-u16-decimal-field-format` |
| `0x801D0C10` | `23` | 3 | `ascii-digit-glyph-literal` |
| `0x801D0C14` | `*%2d` | 5 | `ascii-u8-decimal-field-format` |
| `0x801D0C38` | `%3d` | 4 | `ascii-u16-decimal-format` |
| `0x801D0C8C` | `%4d` | 4 | `ascii-u16-decimal-format` |
| `0x801E28C4` | `Manillo` | 12 | `game-fixed-literal` |
| `0x801E28D0` | `????????????` | 12 | `game-fixed-literal` |

The `%2d` field at `0x801D0C04` has four consumers. Their argument domain
includes a signed halfword plus one, covering `[-32767, 32768]`. Its codec retains
one decimal argument, at most three encoded bytes plus NUL, and at most six
formatted bytes. It accepts `%d` with an optional width from 1 to 4. Extra
literal characters exceed its conservative output bound.

The `%d/6` field receives an unsigned halfword plus one, covering `[1, 65536]`.
There is no proven limit of six and no sixteen-bit wrap. The field codec uses a
five-digit argument budget. Its four-byte encoded limit restricts every accepted
edit to at most seven formatted bytes, including literals. The `*%2d` field
receives a guarded unsigned byte in `[2, 255]` and permits at most four formatted
bytes, including literals. Both retain one decimal argument and accept widths
from 1 to 4, with optional zero padding when it fits the allocation.

The `23` field passes directly to `0x8014FF0C`. It requires one or two glyphs
from spaces, digits and the `:` glyph index. Empty input is refused because the
renderer advances before testing the next NUL. The three prefix formats
write into `0x80145AD4`. Three `%2d` sites draw through `0x8014FF0C`, and the
fourth uses `0x8014F800`; `%d/6` and `*%2d` use `0x80150098`.
Header-adjacent bytes at `[0x801D0C0D,0x801D0C10)`, `0x801D0C13` and
`[0x801D0C19,0x801D0C1C)` remain protected. The last span contains nonzero data.

The formats at `0x801D0C38` and `0x801D0C8C` retain one decimal argument bounded
by `[0, 65535]`. They permit at most five formatted bytes before NUL and three
encoded ASCII bytes.
They accept `%d` with an optional width from 1 to 4. Zero padding cannot fit
these allocations. Other conversions, missing arguments and extra literals
that exceed either bound are refused. Consumer `0x801E1004` uses the first
format with both a byte and a masked sixteen-bit result. Consumer `0x801E1320`
uses the second with a masked sixteen-bit result. Both call `0x8014F800`.
The first consumer preserves its three-character draw limit; the second uses
five. The formatter's output bound does not prove that every digit appears
within the first consumer's shorter draw limit.

The two fixed labels use the game's literal encoding and all twelve field bytes.
Consumer `0x801E1004` copies twelve bytes, appends NUL in the destination buffer
and calls `0x8014F800` with a fifteen-character limit. Shorter edits zero-fill
the remaining field bytes. Other labels, dynamic record names and runtime
rendering remain unverified.

Recognition pins the complete selected payload outside these allocations.
Packing additionally binds the complete archive hash, canonical source path,
entry and entry offset. It preserves all other payloads. Each archive replica
remains a separate source. Other embedded strings, different code
layouts and observed rendering remain separate coverage requirements.

The `US battle messages` layout supports ten thirteen-byte literal records
in BATTLE entry 3, loaded at `0x801D0C00`. The table starts at `0x801EB000` with
stride thirteen. Supported table rows are 0, 2 and 4 through 11; each
accepts up to twelve literal bytes followed by NUL. The copy consumer at
`0x801DE94C` uses that stride and copies at most twelve bytes through
`0x801501E4`, which appends NUL. Rows 0, 2, 6, 7, 8, 9, 10 and 11 retain
renderer `0x8014F800`. Event enqueue at `0x801DE560` stores message pointers;
draw consumer `0x801E9538` forwards them to that renderer.

Row 4, `Defend` at `0x801EB034`, and row 5, `Charge` at `0x801EB041`, use
`game-literal` with thirteen-byte allocations. Only their indexed bounded copy
is qualified. Preserve their `renderers: []`. Natural selectors, message
publication and rendering remain unverified for these two rows.

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
table row. Command-bearing row 3 remains outside this layout's editable allocations.

The same BATTLE layout exposes three ASCII allocations for its digit-sprite
renderers. Identical payload copies in other US archives use local entry indices.
Capacities include NUL and exclude alignment bytes:

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
The original `:` selects glyph index ten.
Other ASCII and percent sequences are refused. The numeric formats cannot add
literals within their five-byte output bound. Packing preserves the two alignment
bytes at `0x801D0C72` and the adjacent data at `0x801D0C7C`.

Observed rendering covers fallback digit `9` beneath enemy names, width one at
HP/AP value 15, and width one at queued animation digit 2. Other widths, values
and battle contexts remain unverified. Identical payload copies leave the active
archive indistinguishable.

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
`0x801D0E9D..0x801D0EA0` remain protected. Ordinary shop Buy-list observations
verify the edited price suffix `%6dG` at `0x801D0E88`. Sell quantity-preview
observations verify that price suffix and the edited total suffix `%7dG` at
`0x801D0E90`. The Sell preview also covers the original starred quantity field
for one item. The separate money display retains its original `Z` suffix.
Other quantity paths, larger quantities, purchase effects and sale effects
remain unverified.

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

### Battle-results text

The `US battle-results text` layout covers the 8417-byte entry 0 of original US
`BIN/BATTLE/BATL_END.EMI`, loaded at `0x801EEC00`. It exposes six ASCII formats
and one literal glyph. Capacities include the original NUL; formatted bounds
exclude it. Re-extract older inline documents before editing.

| Address | Original | Capacity | Codec | Maximum formatted bytes | Renderer |
| --- | --- | --- | --- | --- | --- |
| `0x801EEC04` | `%d` | 3 | `ascii-i32-decimal-format` | 11 | `0x8014F800` |
| `0x801EEC40` | `%d` | 3 | `ascii-u16-decimal-format` | 5 | `0x8014F800` |
| `0x801EEC44` | `%6d` | 4 | `ascii-i32-decimal-format` | 11 | `0x8014FF0C` |
| `0x801EEC48` | `%2d` | 4 | `ascii-u8-plus-one-decimal-format` | 3 | `0x8014FF0C` |
| `0x801EEC4C` | `%7d` | 4 | `ascii-i32-decimal-format` | 11 | `0x8014FF0C` |
| `0x801EEC50` | `*%2d` | 5 | `ascii-u8-decimal-field-format` | 4 | `0x80150098` |
| `0x801F0CD0` | `Ƶ`, byte `0x60` | 2 | `game-literal` | Not formatted | `0x8014F800` |

Each format retains one decimal argument and both its encoded and formatted
bounds. The three-byte allocations permit only `%d`. The signed-word fields
retain the full 32-bit domain, including the eleven-byte negative minimum.
The field at `0x801EEC40` receives a value masked to sixteen bits. Both `%d`
formats draw through battle-bank command `0x07`. Its operand selects a 32-byte
buffer at `0x801490D8 + index * 32`; `0x8014F800` draws that buffer through the
recursive `0x8014F84C` consumer. The signed format supplies EXP and money in
buffer 0 and the resulting level in buffer 1. The unsigned format supplies six
stat deltas in buffers 2..7. Their maximum outputs, including NUL, fit these
buffers. These are conditional source paths; they do not establish ordinary
availability or exclusive scratch-buffer ownership.

Call `0x801F08F4` supplies an unsigned byte plus one to the format at
`0x801EEC48`. Its argument domain includes 256 and its output bound is three
bytes. The `*%2d` consumer at `0x801F0A74` formats unsigned byte counts of two or
more; its original maximum output is `*255`. The glyph at `0x801F0CD0` permits
at most one literal game byte plus NUL and reaches the draw call at `0x801F0A58`.

The empty `references` arrays reflect code addressing. Recognition pins those
instructions and every payload byte outside the allocations. Allocations stop
at the original terminators; following padding remains protected, as do byte
`0x1D` at payload offset `0x55` and the palette at `0x20D4`.

Controlled results-fixture observations verify `%6d` to `%5d` for remaining
EXP 10, `%2d` to `%3d` for next level 1, and `%7d` to `%6d` for current money 0.
Each numeric edit moves the digits by one eight-pixel glyph cell. Replacing
`Ƶ` with `X` changes the currency glyph. These observations use zero rewards.

A separate fixture sets the EXP pool to 30 and verifies the original Whelp
preview at level 3 with gains of 5 HP, 1 AP, 2 Power, 1 Defense, 2 Agility and
3 Intelligence. Changing the plural HP label from `H` to `L` changes only that
glyph; all other screen pixels remain identical. The identical BATTLE entry 11
and BOSS001 entry 8 bank copies leave the active supplier ambiguous.

A controlled list containing one item with count 2 verifies the original Green
Apple display and an edit to Brown Apple. Changing `*%2d` to `*%d`
removes the space and moves the quantity digit eight pixels left; the star stays
fixed. These previews stop before stored level and stat updates or inventory
grant. Ordinary reward availability, normal cleanup and other battle-results
text remain unverified.

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

### Card-choice bank aliases

US `BIN/ETC/AFLDKWA.EMI` entry 0 and `BIN/ETC/FIRST.EMI` entry 11 contain
identical 16,872-byte battle-text payloads loaded at `0x8001A000` in raw mode 0.
Their payload offsets are 2048 and 243712. The eight-byte header selects bank 0
at offset 8 and bank 1 at `0x1694`. Bank 0 occupies 5,772 bytes.

During ordinary startup, SLUS call `0x8014EAB4` requests loader slot `0x261`.
Original table cell `0x80182DC8` and a native resolver return select FIRST at
LBA `61359`. The consumed header identifies entry 11's raw bank at `0x8001A000`,
with size 16,872 and checkword 8. Complete native RAM histories observe every bank
byte copied from zeros during that load and no bank writes in the subsequent
startup interval. A linked paused capture shows the title logo with the same
edited bank. This binds FIRST entry 11 to this initial startup bank. It does not
identify the supplier after later game loads or establish use of the AFLDKWA copy.

| Bank 0 JSON rows | Selected runtime ID | Original text | RAM address | Capacity including NUL |
| --- | --- | --- | --- | --- |
| 29 | `0x1C` | `Save` | `0x8001A438` | 5 |
| 126 through 134, shared | `0x85` | `Memory card` | `0x8001AA0D` | 12 |
| 153 | `0x98` | `Save: Which memory card?` | `0x8001AB37` | 25 |

Runtime IDs are zero-based; JSON row numbers are one-based. All nine shared rows
use one allocation and must have identical text. Lookup `0x801502D0` resolves
these IDs. SHOP consumers `0x801D4A3C` and `0x801DDFB0` pass the prompt, title
and two slot labels through `0x8014F800` and `0x8014F84C`.

Use `bof3.dialogue-text/v3` with class `battle` and a separate source-bound
document and packed output for each archive. Packing preserves both bank tables,
the header and all bytes outside the edited row extents. Editing one archive
does not update the other.

Controlled packed-media checks verify the prompt edited to
`Keep: Which memory chip?`, the `Keep` title and both `Memory chip` slot labels.
Those checks use a frontend-state fixture after the first battle.

These edits also display through an ordinary diary interaction after loading a
game-created save from the title menu. Both complete SHOP readers and the edited
bank match loaded RAM. The original-label diary route has created and reloaded
one save, and a native read watch matches its prompt consumer. Selected
value-panel calls and the unique supplying archive remain unbound.

### Ability description lookup

The same two archive entries contain the original `Heal light wounds on one
target` description in bank 1. One-based rows 207, 208 and 209 share its 32-byte
allocation at bank offset `0x1363`, payload offset 10743 and RAM address
`0x8001C9F7`. All three rows must retain identical text when packed.

Ordinary Ability navigation selects runtime ID `0x40D0`. The description reader
at `0x801E9BEC` loads that ID from the current record's halfword at `+0x10`.
Lookup `0x801502D0` selects bank 1, zero-based index 208 and table cell
`0x8001B834`, then returns `0x8001C9F7`. The reader stores that pointer at
`SP + 0x10` at `0x801E9C84`, before calling renderer `0x8014F800` at
`0x801E9C9C`. Its delay word sign-extends the Y coordinate.

Packing all three shared rows as `Mend light wounds on one target{end}` changes
three bytes within the same 32-byte allocation. After a fresh boot of the edited
disc, loading a game-created inn save through Load Game and selecting Heal through
Ability displays `Mend light wounds on one target`. Heal retains its AP cost of 4.

Linked native captures bind ID `0x40D0`, its lookup result and the stack argument
for the edited description. A read watch at `0x8014F8D4` stops before the byte load
with `S1=0x8001C9F7` and the edited byte `0x4D` at that address. Continuation
returns to `0x801E9CA4` with the caller stack restored. These observations bind
row 209 for the selected original and edited descriptions. The other shared-row
lookups, general selector bounds and the unique description-bank supplier remain
unverified.

### Item description display

Bank 1, one-based row 3 in the same two archive entries contains
`Restores 40HP{end}`. Packing both copies as `Restores 41HP{end}`, booting the
edited disc and loading a game-created inn save makes the ordinary shop Buy list
display `Restores 41HP` with Healing Herb selected. The complete edited bank
matches loaded RAM. The unique supplying archive, other item descriptions and
the item's healing effect remain unverified.

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
