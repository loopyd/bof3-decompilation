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

## Graphics owner

Graphics parsers, source profiles, byte-role proofs and their limits belong to
[`bof3-textures`](../../bof3-textures/docs/byte-ranges.md). Text coverage consumes
its checked ranges. Graphics data does not become a text codec or editable text
allocation through this classification.

## Save-icon source coverage

Original `SHOP.EMI` entry 8 and `START.EMI` entry 9 contain the same save-header
producer, palette and three icon frames. Qualification requires the exact original
archive, reviewed entry index and offset, raw mode, load address `0x801EEC00`,
checkword `0x141`, size 5268 and payload SHA-256
`6e7b93d4fd75586413b5ab3a6785d7b2492d8d1b566a3a6d79941bb656131a4d`.
Each physical copy retains its own source identity.

Coverage includes the complete producer body at `0x801EF27C..0x801EF96C`,
including its return delay slot. The original SHOP call at `0x801D6A08` supplies
mode `0x13` in its delay slot. The
[palette and frame fields](../../bof3-textures/docs/byte-ranges.md#save-icon-source-coverage)
belong to the texture owner; surrounding code and gaps remain unresolved.

The path requires successful original loading, stable source and code, a bounded
valid title, an ordinary returning helper, an intact frame and nonaliased work.
These source extents do not establish actual invocation, the active supplying
archive, BIOS animation, displayed pixels or safe editing.

## Area surface reader instruction coverage

The [surface fields](../../bof3-textures/docs/byte-ranges.md#area-surface-field-coverage)
belong to the texture owner.

The [exact original SLUS identity](#slus-instruction-coverage) qualifies
the complete 144-byte leaf at `0x8014E3F4..0x8014E484`, file offsets
`[754676,754820)`. The original GAME call at `0x8019FB00` conditionally enters
after `clearAllRecords` returns. Review covers all 36 words and four branch or
return delay slots. The body resets fixed slot bytes at `0x80145BD4` and
`0x80145D54`; it does not initialize the auxiliary plane. Valid nonaliased
stores, stable code and ordinary incoming return control remain conditions.
Referenced slot data, actual invocation, auxiliary ownership, rendering and
safe editing remain unqualified.

## Battle resident dispatch fields

The [resident animation fields](../../bof3-textures/docs/byte-ranges.md#battle-resident-animation-field-coverage)
belong to the texture owner.

Coverage separately recognizes the five copied handler words and request bytes
at table indices 0, 3, 4 and 6 in the exact common battle payload. Other request
bytes receive no classification from this review.

These roles require the exact raw transfers to complete, ordinary helper
returns, valid nonaliasing memory and frames, stable code and tables, and the
same bank and work fields to survive until the queued draw. They establish
conditional stored-field use. They do not establish actual invocation, loading,
residency, image or CLUT bindings, uploads, display, absence of rasterized text
or safe editing.

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

## Native overlay instruction coverage

Coverage recognizes complete instruction bodies in the following original US
entries. Each uses raw mode 0. Entry 0 starts at archive offset 2048;
COMMU01 entry 7 starts at offset 143360. AREA121 entry 14 starts at offset
815104, and AREA077 entry 14 starts at offset 819200. The complete archive
must match its original US disc identity.

| Archive | Entry | Load address | Checkword | Payload bytes | Payload SHA-256 |
| --- | ---: | --- | --- | ---: | --- |
| `SCENA00.EMI` | 0 | `0x801F6C00` | `0x105` | 28528 | `c5bda6ff8260b59a361a49f80720bb6942d9eed8d6497d3cc9d1d2b483c3089e` |
| `COMMU01.EMI` | 7 | `0x801D0C00` | `0x19` | 16948 | `61e0ed2d90247be2bad9904015ac042ac3a83e638721497a90e298815354e165` |
| `SCENA13.EMI` | 0 | `0x801F6C00` | `0x112` | 21488 | `31859c1ab5aa4a2c8250f0011569ac3b66a442f023717fb677883417ddeab469` |
| `SISYOU.EMI` | 0 | `0x801D0C00` | `0x1F` | 13963 | `42accff83b45c5ef99947f70a6c968fdf2a2a4afd0abdc3fca8874db20ecc8af` |
| `SCENA04.EMI` | 0 | `0x801F6C00` | `0x109` | 14176 | `cb0ff0c46ea33d5c9eef07493b7b2c2673278dbb97373c3eb8f77b8adfdbcb9f` |
| `PLP056.EMI` | 0 | `0x801CE400` | `0x27` | 9428 | `98b83aa1d6951af3844ea34058e34fbcc18acb5d5e8f69eb4aa289107564bfa8` |
| `PLP567.EMI` | 0 | `0x801CE400` | `0x31` | 9224 | `3eee6be151e6a3ffff13ca98efad8521719a29bcd7ffd465f37f2ec5188389e3` |
| `SCENA03.EMI` | 0 | `0x801F6C00` | `0x108` | 25672 | `3570700503e37f2a43a3ad9877521773532fd8b2278ceb70ca1f7cb4e37ef7e8` |
| `AREA121.EMI` | 14 | `0x801F2C00` | `0xAD` | 15547 | `42da9ce8b9e2225fc93ed1bcb0296fd7a42a341f5a81187fcb0025ac5365a275` |
| `AREA077.EMI` | 14 | `0x801F2C00` | `0x81` | 16248 | `0987764aeff328845033df60674008b12b1af555fe888c68445459964c292e9e` |
| `SCE15EF1.EMI` | 0 | `0x801D0C00` | `0x11B` | 13496 | `0af89b01778db26d220b3fe5975f766edf0b798c96dc6df1c82031fb9cbea361` |
| `GAME.EMI` | 0 | `0x80195800` | `0x1E` | 229720 | `1663ff2e78134fe15f3cae6fb4074acf14b68b79cfa02be0c7651eaf6a1728ff` |

Offsets below are relative to the selected entry. Runtime addresses are the
load address plus these offsets. Adjacent complete functions are coalesced
only for coverage accounting.

The GAME pass preserves earlier physical ranges and their evidence. Use
`bin/harness text coverage --root out/extracted --json` for its admitted extents.

| Archive | Payload intervals | Complete functions | Bytes |
| --- | --- | ---: | ---: |
| SCENA00 | `[448,19748)`, `[21128,22432)`, `[23504,23936)` | 61 | 21036 |
| COMMU01 | `[104,8600)`, `[10644,16312)` | 33 | 14164 |
| SCENA13 | `[676,1016)`, `[1244,1696)`, `[2116,2180)`, `[2408,2620)`, `[3416,4540)`, `[6260,13080)`, `[17592,18532)`, `[20020,21232)` | 44 | 11164 |
| SISYOU | `[24,2116)`, `[2940,4692)`, `[6048,11812)`, `[12912,13144)` | 43 | 9840 |
| SCENA04 | `[256,676)`, `[812,2492)`, `[2984,4336)`, `[4800,5172)`, `[5412,6784)`, `[9112,12788)`, `[13264,13696)` | 38 | 9304 |
| PLP056 | `[4,9304)` | 38 | 9300 |
| PLP567 | `[4,9100)` | 38 | 9096 |
| SCENA03 | `[624,692)`, `[1632,2380)`, `[3604,7420)`, `[8196,8704)`, `[9972,10040)`, `[11256,11392)`, `[13476,14156)`, `[22452,23724)`, `[23876,25112)`, `[25308,25380)` | 66 | 8604 |
| AREA121 | `[60,304)`, `[448,540)`, `[1144,1284)`, `[1596,1720)`, `[2072,3864)`, `[5340,5584)`, `[5900,6460)`, `[6624,6692)`, `[6996,7036)`, `[7936,8012)`, `[9080,10020)`, `[10112,11160)`, `[11544,13004)`, `[13304,14552)` | 48 | 8076 |
| AREA077 | `[4,72)`, `[2936,3224)`, `[4212,4388)`, `[4612,4680)`, `[4872,5556)`, `[5664,6088)`, `[6480,7784)`, `[8412,8480)`, `[8900,10828)`, `[10948,13904)` | 55 | 7964 |
| SCE15EF1 | `[4,72)`, `[192,3856)`, `[4308,6048)`, `[6844,7476)`, `[8004,9176)`, `[10908,11544)` | 31 | 7912 |
| GAME | Reported by `text coverage --json` | 236 | 27632 |

Fresh native compilation and relocation reproduce all complete sized ELF
function symbols. Compiler assembly separates instructions from initialized
data. Headers, initialized sections, retained functions and gaps remain outside
these ranges. SISYOU's literal allocations retain their text classification.
These profiles record stored instruction roles. Check current source matches with
`bin/harness lift status TARGET`. Execution, callee semantics, resource safety,
rendering and safe editing need separate evidence.

## Scenario instruction and dispatch-table coverage

Coverage recognizes complete instruction bodies in entry 0 of original US
`BIN/SCENARIO/SCENA02.EMI`, `SCENA06.EMI`, `SCENA09.EMI` and `SCENA11.EMI`.
Each entry uses archive offset 2048, raw mode 0 and load address `0x801F6C00`.
The complete archive must match its original US disc identity.

| Archive | Checkword | Payload bytes | Payload SHA-256 | Complete functions |
| --- | --- | ---: | --- | ---: |
| SCENA02 | `0x107` | 30588 | `e58e751dc92ba3cc9bbc788bb7d93ae1f4b036968c4bc95e4bf29945dd97a8aa` | 69 |
| SCENA06 | `0x10B` | 30964 | `a3f118a0a9a17aefb50b4cd588c9a152a943591462fc9424ddfe9f5c9e9e705b` | 63 |
| SCENA09 | `0x10E` | 31064 | `3292290e08fdfeee3c46383fc4bfa1ae0666dc5d5724335c1395fb1ab3a5614f` | 87 |
| SCENA11 | `0x110` | 17304 | `c4463f21512ac74adf7002b7a5654450c2049513bef7c817473a9d5108532239` | 51 |

The following payload intervals contain instructions. Adjacent complete functions
are coalesced only for coverage accounting.

| Archive | Payload intervals | Bytes |
| --- | --- | ---: |
| SCENA02 | `[1436,1532)`, `[3680,3740)`, `[4144,4152)`, `[5224,5876)`, `[6456,6976)`, `[8312,12964)`, `[16468,18284)`, `[19140,19560)`, `[20588,22452)`, `[22808,23176)`, `[23348,24540)`, `[24852,26556)`, `[26636,27520)`, `[30072,30316)` | 14480 |
| SCENA06 | `[1688,8016)`, `[8984,12788)`, `[14288,20616)`, `[20872,24888)`, `[25040,26556)`, `[27464,29052)`, `[29228,30652)` | 25004 |
| SCENA09 | `[1884,30596)` | 28712 |
| SCENA11 | `[1204,1536)`, `[1672,17088)` | 15748 |

Fresh native compilation and relocation reproduce every complete sized ELF
function symbol. Compiler assembly separates instructions from initialized data.
Retained functions and gaps between these intervals receive no instruction claim.

Three separate readonly jump tables qualify as numeric storage. Their original
dispatches have index-load-delay NOPs, unsigned bounds, settled table loads and
targets within the complete owning function.

| Archive | Payload interval | Entries | Owning function |
| --- | --- | ---: | --- |
| SCENA06 | `[4,32)` | 7 | `[0x801F7300,0x801F7A00)` |
| SCENA06 | `[36,56)` | 5 | `[0x801F7300,0x801F7A00)` |
| SCENA11 | `[52,96)` | 11 | `[0x801F72E4,0x801F7748)` |

For the first SCENA06 table, `LBU` at `0x801F7594` is followed by a NOP
at `0x801F7598`, an index mask at `0x801F759C` and the unsigned bound
at `0x801F75A0`. The other two leading NOPs are at `0x801F780C` and
`0x801F7628`. Every table load has its own NOP before the indirect jump.
The four-byte gap `[32,36)` in SCENA06 is outside both tables.

These ranges establish stored instruction or numeric representation. Other
initialized sections and tables, headers, padding, referenced storage and retained
bytes keep their prior classifications. Grouped-comparison capability, semantic
acceptance, general caller bounds, observed execution, resource safety, rendering
and safe editing remain separate checks.

## AREA030 instruction coverage

Coverage recognizes 229 complete instruction bodies in entry 4 of original US
`BIN/WORLD00/AREA030.EMI`, `BIN/WORLD02/AREA089.EMI` and
`BIN/WORLD03/AREA129.EMI`. Each entry has archive offset 180224, raw mode 0,
load address `0x801D0C00`, checkword `0x16`, size 75285 and payload SHA-256
`d869b1540648c3fb98fe64cb9ec62a95b4f8082a111cfd15331a102b0d7a7dd2`.
The complete archive must also match its original US disc identity.

| Payload interval | Runtime interval | Bytes |
| --- | --- | ---: |
| `[152,220)` | `[0x801D0C98,0x801D0CDC)` | 68 |
| `[1472,69844)` | `[0x801D11C0,0x801E1CD4)` | 68372 |

Adjacent complete functions are coalesced only for coverage accounting. Fresh
native compilation and relocation produce complete sized ELF function symbols
whose bytes match the original payload. Compiler assembly separates instructions
from initialized data. The five-word jump table at `0x801D0C1C` is a separate
nonexecutable section and remains outside this instruction classification.

These ranges establish stored instruction representation. Headers, tables,
padding outside symbols, referenced storage and all retained payload bytes keep
their prior classifications. Execution, callee semantics, resource safety,
rendering and safe editing remain separate checks.

## GAME actor and record dispatch instruction coverage

Coverage recognizes these complete instruction bodies under the existing exact
US `GAME.EMI#0` archive and payload identities. Offsets are relative to entry 0.
The entry must use raw mode 0, archive offset 2048, load address `0x80195800`,
checkword `0x1E`, size 229720 and payload SHA-256
`1663ff2e78134fe15f3cae6fb4074acf14b68b79cfa02be0c7651eaf6a1728ff`.

| Payload interval | Runtime interval | Bytes |
| --- | --- | ---: |
| `[15424,15704)` | `[0x80199440,0x80199558)` | 280 |
| `[18660,18836)` | `[0x8019A0E4,0x8019A194)` | 176 |
| `[108792,108908)` | `[0x801B00F8,0x801B016C)` | 116 |
| `[164804,165112)` | `[0x801BDBC4,0x801BDCF8)` | 308 |
| `[176744,176820)` | `[0x801C0A68,0x801C0AB4)` | 76 |
| `[186708,186884)` | `[0x801C3154,0x801C3204)` | 176 |

The original AREA030 entry-4 frame calls `0x80199440` at `0x801E0BD8`.
That body first calls `0x801C0A68`. Its first actor branch publishes
`0x80145E90` to `0x80146250` and `0x1F800044`, then calls `0x801B00F8`
at `0x8019948C`. The reader calls `0x801C3154` and `0x801BDBC4` before
loading its actor-selected function pointer and making an indirect call.
All six bodies match the original archive and retained RAM. Complete original
instruction and control-flow review includes every delay slot and return.

AREA030 entry-4 call `0x801E0C00` conditionally enters `0x8019A0E4`.
This updater visits 20 records at `0x80143FC8` with stride `0x74`. For each
active record, it publishes the pointer to `0x1F800044` and dispatches byte 5
through `0x801C7C70`. It does not set `A0` or publish `0x80146884`. The
record fields and table cells retain their prior classifications.

Classification covers these instruction bytes. Ordinary prior and callee returns,
preserved saved values, valid memory and return control, intact nonaliasing frames
and stable code and pointers remain conditions. Selected handlers, tables,
actors, palettes, state and neighboring bodies retain their prior classifications.
This evidence does not establish observed entry, general selector bounds, callee
effects, resource safety, rendering, safe editing or an ELF match.

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
| `[8548,9236)` | `[0x801D2D64,0x801D3014)` | 688 |
| `[47868,49756)` | `[0x801DC6FC,0x801DCE5C)` | 1888 |
| `[72636,72704)` | `[0x801E27BC,0x801E2800)` | 68 |
| `[76576,76660)` | `[0x801E3720,0x801E3774)` | 84 |

Two complete bodies qualify under the existing exact GAME and SLUS identities.
GAME offsets below are relative to entry 0's payload. SLUS offsets include its
2048-byte executable header.

| Source | Source interval | Runtime interval | Bytes |
| --- | --- | --- | ---: |
| `GAME.EMI#0` | `[15256,15344)` | `[0x80199398,0x801993F0)` | 88 |
| `SLUS_004.22` | `[797824,798136)` | `[0x80158C80,0x80158DB8)` | 312 |

The retained GAME object's complete 88-byte executable section and sized function
match the original body after all eight `R_MIPS_26` relocations. This comparison
does not establish a fresh build or linked-executable match.

GAME call `0x801993D0` enters the SLUS body. For an active slot with byte 1 equal
to 7, the SLUS dispatcher selects SHOP `0x801E27BC` through the nine-pointer
table copied from `0x80149B0C`. Panel byte 2 equal to 9 then selects wrapper
`0x801E3720` through cell `0x801E5D8C`. After its byte-3-selected indirect call,
the wrapper reloads `0x80148648` and calls `0x801DC6FC` at `0x801E375C`.
Original bytes and retained RAM agree for all five bodies. Copied pointers and
saved returns corroborate this conditional path without proving earlier execution.
The supporting table cells retain their prior classifications.

The three selected little-endian function-address words at `0x801E514C`,
`0x801E5174` and `0x801E518C` also qualify as known nontext. For selector
bytes 2, 1 and 0, they select `0x801D10E4`, `0x801D12C0` and
`0x801D12FC`, respectively.

For phase 3, the additional cell at `0x801E517C` selects `0x801D27EC`.
That dispatcher selects `0x801D28D0` through cell `0x801E51B4` for
substep 1 and `0x801D2D64` through cell `0x801E51B8` for substep 2.
All three four-byte little-endian function-address words also qualify
as known nontext.

Complete original-word and structural control-flow review covers every
instruction and delay slot. All admitted bodies and table cells match retained
RAM. Ordinary prior and callee returns, valid indirect-call targets, preserved
frames, valid nonaliased memory and stable selectors, code and tables remain
conditions. Other table cells, neighboring bodies and referenced data retain their
prior classifications. General selector bounds, selector producers, callee behavior,
whole-body execution, resource safety, safe editing and runtime text acceptance
remain unqualified.

The exact original executable identified under
[SLUS instruction coverage](#slus-instruction-coverage) also qualifies
`0x8014E640..0x8014E6D0`, file interval `[755264,755408)`.
Original SHOP call `0x801D137C` conditionally enters this call-free body.
Review covers all 36 original words and every branch, jump and return delay
slot. Referenced input and repeat-state fields remain outside the classification.
Physical button mapping and input cadence remain separate requirements.

## Instruction coverage from original calls

The following complete original US bodies qualify as known nontext. Every body
has an original direct JAL entry in a previously qualified instruction record.
Review covers every raw instruction word, structural branch and control-transfer
delay word. SLUS uses the exact executable identity in
[SLUS instruction coverage](#slus-instruction-coverage). EMI ranges require an
original whole archive, raw mode, load address, payload size and payload SHA-256:

| Payload | Load address | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| `GAME.EMI#0` | `0x80195800` | 229720 | `1663ff2e78134fe15f3cae6fb4074acf14b68b79cfa02be0c7651eaf6a1728ff` |
| Shared battle consumer | `0x801D0C00` | 118224 | `f25ef137618bc024a6aec98c2c4d9a571c6f23884762fb4567406e6552b383c1` |
| Shared battle consumer | `0x80096800` | 133316 | `77d963c56e3ba6b1619323e3007c25d70300e855ac9f4b088ddce2604f92e140` |

GAME additionally requires entry 0 at archive offset 2048 and check word `0x1E`.
Shared battle entry indices and archive offsets may differ. The entry labels below
refer to `BIN/BATTLE/BATTLE.EMI`; the same exact payloads qualify in other original
archives. Runtime ends are exclusive. Each row lists one qualifying original call.

| Body source | Runtime interval | Original call source | JAL address |
| --- | --- | --- | --- |
| SLUS | `[0x8014D4E0,0x8014D54C)` | SLUS | `0x8014D2B4` |
| SLUS | `[0x8014ECAC,0x8014ECEC)` | BATTLE#3 | `0x801E57F4` |
| SLUS | `[0x80150098,0x801501E4)` | BATTLE#3 | `0x801D911C` |
| SLUS | `[0x80161454,0x80161808)` | SLUS | `0x8014D4C8` |
| SLUS | `[0x8016728C,0x80167344)` | GAME#0 | `0x80197BC0` |
| SLUS | `[0x8017212C,0x801721FC)` | SLUS | `0x8016DFB0` |
| SLUS | `[0x8017A5E4,0x8017A620)` | SLUS | `0x8014AE4C` |
| SLUS | `[0x8017A620,0x8017A6F0)` | BATTLE#3 | `0x801D754C` |
| SLUS | `[0x8017A8C8,0x8017A8EC)` | SLUS | `0x8014C030` |
| SLUS | `[0x8017A904,0x8017A92C)` | SLUS | `0x8014CD5C` |
| SLUS | `[0x8017AA6C,0x8017AA80)` | BATTLE#3 | `0x801D9950` |
| SLUS | `[0x8017C2D8,0x8017C330)` | SLUS | `0x8014BDDC` |
| SLUS | `[0x8017EE0C,0x8017EE1C)` | SLUS | `0x8014AD78` |
| SLUS | `[0x8017EE1C,0x8017EE2C)` | SLUS | `0x8014ADA4` |
| SLUS | `[0x8017F3D0,0x8017F3E4)` | SLUS | `0x8017F1D4` |
| GAME#0 | `[0x8019651C,0x80196670)` | GAME#0 | `0x801C15C4` |
| GAME#0 | `[0x80196670,0x80196718)` | GAME#0 | `0x80196750` |
| GAME#0 | `[0x801992B8,0x80199308)` | GAME#0 | `0x80197ABC` |
| GAME#0 | `[0x801A0514,0x801A061C)` | GAME#0 | `0x801993B8` |
| GAME#0 | `[0x801A06D8,0x801A0AE4)` | GAME#0 | `0x80197AB4` |
| GAME#0 | `[0x801AF390,0x801AF494)` | BATTLE#15 | `0x800B0C54` |
| GAME#0 | `[0x801AF494,0x801AF5B0)` | BATTLE#15 | `0x800B26D4` |
| GAME#0 | `[0x801AF5B0,0x801AF800)` | BATTLE#15 | `0x800B2D68` |
| GAME#0 | `[0x801AF800,0x801AF930)` | BATTLE#15 | `0x800B277C` |
| GAME#0 | `[0x801AF930,0x801AFA1C)` | BATTLE#15 | `0x800B2E3C` |
| GAME#0 | `[0x801AFA1C,0x801AFD10)` | BATTLE#15 | `0x800B1074` |
| GAME#0 | `[0x801AFD10,0x801AFE18)` | BATTLE#15 | `0x800B2630` |
| GAME#0 | `[0x801BEE5C,0x801BF11C)` | GAME#0 | `0x80197AAC` |
| GAME#0 | `[0x801BF11C,0x801BF448)` | GAME#0 | `0x80197C24` |
| GAME#0 | `[0x801C3204,0x801C329C)` | GAME#0 | `0x801C15D4` |
| BATTLE#3 | `[0x801DCAD8,0x801DCCB0)` | BATTLE#3 | `0x801DC234` |
| BATTLE#3 | `[0x801E2E30,0x801E30B8)` | BATTLE#3 | `0x801E2990` |
| BATTLE#15 | `[0x800A78B8,0x800A7B40)` | BATTLE#15 | `0x800A761C` |
| BATTLE#15 | `[0x800A9FC4,0x800AA368)` | BATTLE#15 | `0x800A9C0C` |
| BATTLE#15 | `[0x800B06F4,0x800B09CC)` | BATTLE#15 | `0x800B04C4` |

Info-page exports narrow the search; original bytes determine body boundaries.
The SLUS bodies ending at `0x801721FC` and `0x8017A620` include the return
delay words at `0x801721F8` and `0x8017A61C`. GAME bodies beginning at
`0x801AF494`, `0x801AF930`, `0x801AFA1C` and `0x801AFD10` have independent
original call entries even when an exported span combines them with a neighbor.
The following body at `0x801AFE18..0x801AFF04` remains outside this rule because
its entry has not been qualified.

GAME body `0x801AF494..0x801AF5B0` also qualifies its five-word numeric jump
table at `0x80195F7C..0x80195F90`, payload bytes `[1916,1936)`. It masks the
selector with `0xFF`, tests unsigned `< 5` at `0x801AF4B0` and branches to the
default at `0x801AF598` when outside that bound. The scaled load at `0x801AF4C4`
and register jump at `0x801AF4CC` select only `0x801AF598`, `0x801AF4D4`,
`0x801AF544`, `0x801AF558` or `0x801AF578`. All targets stay inside the complete
consumer. Guard, load, jump and delay words reconcile with the original table.

The two SLUS bodies at `0x8017EE0C` and `0x8017EE1C` contain SYSCALL at
`0x8017EE10` and `0x8017EE20`. Their structural continuations assume exception
return to the following instruction; they do not establish the handler or its
effects. The five-word body at `0x8017F3D0` loads a pointer from `0x8018B2F8`
and ends with `JR t1` at `0x8017F3DC` and its delay word. Only those instruction
words qualify. Pointer storage, target identity and outgoing effects remain
outside this rule.

Call continuations assume ordinary returns, valid nonaliasing memory, intact
frames and stable code and tables. Other referenced data and external callees
retain their prior status. These ranges establish stored instructions and the
bounded numeric table. They do not establish matching ELF output, observed
invocation, feasible paths, transitive effects, resource safety, display or safe
editing.

## Further direct-call instruction coverage

The following bodies use the exact GAME and shared battle identities under
[instruction coverage from original calls](#instruction-coverage-from-original-calls).
Each has an original JAL entry in previously qualified instructions. Review
covers every raw instruction, structural branch and control-transfer delay word.
Runtime ends are exclusive; the battle entry label refers to `BATTLE.EMI#15`.

| Body source | Runtime interval | Original call source | JAL address |
| --- | --- | --- | --- |
| BATTLE#15 | `[0x800A7B40,0x800A7C14)` | BATTLE#15 | `0x800A7954` |
| GAME#0 | `[0x8019625C,0x8019651C)` | GAME#0 | `0x801992E8` |
| GAME#0 | `[0x80196784,0x801968BC)` | GAME#0 | `0x801965C8` |
| GAME#0 | `[0x801A18C8,0x801A1A24)` | GAME#0 | `0x801A0898` |
| GAME#0 | `[0x801C2438,0x801C24D0)` | GAME#0 | `0x801BF088` |
| GAME#0 | `[0x801C2894,0x801C29A0)` | GAME#0 | `0x801BF2E8` |
| GAME#0 | `[0x801C29A0,0x801C2AAC)` | GAME#0 | `0x801BF204` |

GAME body `0x80196784..0x801968BC` also qualifies the five-word numeric table
at `0x80195804..0x80195818`, payload bytes `[4,24)`. It masks the selector to
eight bits, tests unsigned `< 5` at `0x801967C8` and branches to `0x801968A0`
outside that bound. The scaled load at `0x801967EC` and jump at `0x801967F4`
select `0x80196888`, `0x801967FC`, `0x80196870`, `0x80196854` or `0x80196830`
in table order. Every target stays inside the complete consumer; guard, load,
jump and delay words reconcile with the original table.

Call continuations assume ordinary returns, valid nonaliasing memory, intact
frames and stable code and tables. Referenced storage, external callees and
neighboring bytes retain their prior status. These ranges establish stored
instructions and numeric targets, without matching ELF, observed invocation,
feasible-path, transitive-effect, resource-safety, display or safe-edit acceptance.

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

## AREA007 selected event coverage

Original US `AREA007.EMI#13` qualifies numeric selector fields, six complete event
scripts and six pointer cells as nontext. The entry must use raw
mode at archive offset 733184, load at `0x801F2C00`, have checkword `0x3B` and
contain 2548 bytes. The whole archive must match the original disc manifest.
The payload SHA-256 must be
`b8369f325ddcc7250258e7709dc319ec7d9b7c4582ab6d3f23fb88a1954be356`.

| Runtime range | Payload offsets | Archive offsets | Bytes |
| --- | --- | --- | ---: |
| `[0x801F33F4,0x801F340B)` | `[2036,2059)` | `[735220,735243)` | 23 |
| `[0x801F340C,0x801F3423)` | `[2060,2083)` | `[735244,735267)` | 23 |
| `[0x801F3424,0x801F343B)` | `[2084,2107)` | `[735268,735291)` | 23 |
| `[0x801F343C,0x801F3453)` | `[2108,2131)` | `[735292,735315)` | 23 |
| `[0x801F3454,0x801F346B)` | `[2132,2155)` | `[735316,735339)` | 23 |
| `[0x801F346C,0x801F3483)` | `[2156,2179)` | `[735340,735363)` | 23 |
| `[0x801F3490,0x801F34A8)` | `[2192,2216)` | `[735376,735400)` | 24 |

Each script contains two three-byte `F0` headers, three four-byte `D` records,
`FD` and `FE` conditional delimiters, and an explicit `FF` end. Original GAME
readers consume these operands as numeric control fields. Every `D` record has
second byte zero, so setter `0x801A72E8` uses its halfword arm. For example,
`D0 00 00 0C` writes `0x000C` to work offset `0x7C` and leaves callback byte
`0x7A` unchanged. It does not select callback `0x0C`.

The root at `0x801F30F4..0x801F3227` supplies two copies of each selector
`0x8000` through `0x8005`. Only these twelve two-byte fields are classified.
Their payload offsets are 1285, 1301, 1317, 1332, 1350, 1368, 1471, 1487,
1503, 1518, 1536 and 1554. Type-1 records store the high and low bytes at
offsets `0x0E` and `0x0F`. Type-5 records use `0x0D` and `0x0E`.
Original constructors `0x801A50C0` and `0x801A6F58` publish these values to
actor halfword `+0x7C` at `0x801A52FC` and `0x801A71B4`.

The pool caller at `0x801A0790` passes actor `+0x74` to reader `0x801A17A0`.
Its field `+8` therefore refers to the same selector. Under world selector 7,
the reader uses the descriptor's directory pointer to select these six cells.
The cells point to the six scripts in order. These finite cases do not
establish the directory count.

Classification assumes a valid nonnegative actor slot below 30, ordinary
returns, eligible actor lifetime, nonaliasing and stable code and resources.
Other root fields and directory cells, predicate meanings, actual invocation,
menu rendering and safe editing retain their prior status.

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

## Sprite consumer instruction coverage

Coverage also recognizes the following complete original bodies. Each span
includes its return delay word. Original words and structural control flow
establish the instruction role, including call continuations under ordinary
callee returns. The existing whole-executable or archive and raw-payload
identity gates apply. Shared battle copies retain their own archive locations.

| Source and entry | Runtime span | Conditional original entry |
| --- | --- | --- |
| GAME 0 | `[0x80196718,0x80196784)` | JAL `0x801C12C8` |
| GAME 0 | `[0x80196B9C,0x80196C8C)` | BATTLE 3 JAL `0x801E8474` |
| GAME 0 | `[0x80197AA4,0x80197C1C)` | Phase 1, cell `0x801C7B58` |
| GAME 0 | `[0x80197C1C,0x80197C94)` | Phase 2, cell `0x801C7B5C` |
| GAME 0 | `[0x80197C94,0x80197E54)` | Phase 3, cell `0x801C7B60` |
| GAME 0 | `[0x801B0320,0x801B036C)` | Work byte 1 equal to 1, cell `0x801CD0B0` |
| GAME 0 | `[0x801BDAB8,0x801BDB7C)` | JAL `0x801993B0` |
| GAME 0 | `[0x801BF448,0x801BF5A8)` | JAL `0x80197CA4` |
| GAME 0 | `[0x801C1400,0x801C1630)` | JAL `0x80197C9C` |
| BATTLE 3 | `[0x801DFBDC,0x801DFC20)` | State 6, cell `0x801EB138` |
| BATTLE 3 | `[0x801E7BA0,0x801E8558)` | Handler 3, cell `0x801D0F78` |
| BATTLE 15 | `[0x800A7C14,0x800A805C)` | JAL `0x800A69D4` |
| SLUS | `[0x8014BA54,0x8014BAB8)` | GAME 0 JAL `0x801993C8` |
| SLUS | `[0x8014BB20,0x8014BC70)` | JAL `0x8014BA7C` |
| SLUS | `[0x8014BC70,0x8014C47C)` | JAL `0x8014BA84` |
| SLUS | `[0x8014C4D4,0x8014CE48)` | JAL `0x8014BC2C` |
| SLUS | `[0x8014CE78,0x8014CF9C)` | JAL `0x8014C6FC` |
| SLUS | `[0x8014CF9C,0x8014D260)` | JAL `0x8014C760` |
| SLUS | `[0x8014D290,0x8014D4E0)` | GAME 0 JAL `0x801BDB44` |
| SLUS | `[0x8014D8D4,0x8014D910)` | BATTLE 3 JAL `0x801DF334` |
| SLUS | `[0x8014DC08,0x8014DD3C)` | JAL `0x8014D868` |

GAME dispatcher `0x80197A24` reads the phase table at `0x801C7B54` using
halfword `0x80143B92`. Dispatcher `0x801B00F8` reads the table at
`0x801CD0AC` using work byte 1. BATTLE dispatcher `0x801DEEB4` reads
`0x801EB120` using work byte 1. Dispatcher `0x801E7818` reads
`0x801D0F6C` using work byte 2. These finite entry cases do not establish
the complete selector domains or qualify the supporting table storage.

The SLUS palette helper at `0x8014CE78` has one computed jump. Its unsigned
guard accepts selectors below 5 before the scaled load at `0x8014CEB8`
and jump at `0x8014CEC0`. Five original words at
`0x80149998..0x801499AC` target `0x8014CEC8`, `0x8014CEF0`,
`0x8014CF10`, `0x8014CF3C` and `0x8014CF68`. All targets lie inside the
body and close its structural flow. Coverage qualifies these 20 table bytes
separately as numeric data. Other indirect calls retain opaque targets and
assume ordinary returns.

Stable code and applicable tables, valid nonaliasing memory and intact frames
remain conditions. Referenced fields, buffers, external callees and neighboring
bytes retain their prior status. This classification does not establish an ELF
match, observed invocation, all-path feasibility, general selector bounds,
transitive effects, resource safety, display or safe editing.

## Shared battle direct-call coverage

Coverage recognizes these complete original BATTLE entry-15 bodies, loaded at
`0x80096800`. Each has an original direct-call entry from an already qualified
instruction body. Every raw word and structural path reconciles, including
all branch, call, computed-jump and return delay words. The existing exact
archive and raw-payload identity gates apply to each original archive location.

| Runtime span | Conditional original entry |
| --- | --- |
| `[0x80097EC0,0x80097FE8)` | BATTLE 15 JAL `0x80096FD0` |
| `[0x8009AF84,0x8009B03C)` | BATTLE 15 JAL `0x8009841C` |
| `[0x8009DCC8,0x8009DD20)` | BATTLE 15 JAL `0x8009F298` |
| `[0x8009E500,0x8009E7C4)` | BATTLE 15 JAL `0x800A2580` |
| `[0x800A30E8,0x800A31E0)` | BATTLE 15 JAL `0x8009DE30` |
| `[0x800A31E0,0x800A3638)` | BATTLE 3 JAL `0x801DBF80` |
| `[0x800A3B6C,0x800A3C9C)` | BATTLE 15 JAL `0x800A252C` |
| `[0x800A3C9C,0x800A3DF8)` | BATTLE 15 JAL `0x800A1B50` |
| `[0x800A3DF8,0x800A3F28)` | BATTLE 3 JAL `0x801DBF68` |
| `[0x800A3F28,0x800A403C)` | BATTLE 15 JAL `0x8009DA9C` |
| `[0x800A403C,0x800A40D0)` | BATTLE 15 JAL `0x8009E1E8` |
| `[0x800A4294,0x800A4458)` | BATTLE 15 JAL `0x8009DDA0` |
| `[0x800A4458,0x800A4688)` | BATTLE 3 JAL `0x801E0DB4` |
| `[0x800A6E14,0x800A7648)` | BATTLE 15 JAL `0x800A6AC4` |
| `[0x800A805C,0x800A82A0)` | BATTLE 15 JAL `0x800A6DD4` |
| `[0x800A8F58,0x800A9304)` | BATTLE 3 JAL `0x801DFA98` |
| `[0x800A9304,0x800A94A8)` | BATTLE 3 JAL `0x801E2824` |
| `[0x800A94A8,0x800A955C)` | BATTLE 3 JAL `0x801D399C` |
| `[0x800A955C,0x800A9610)` | BATTLE 3 JAL `0x801D3974` |
| `[0x800A9820,0x800A99AC)` | BATTLE 3 JAL `0x801DFBAC` |
| `[0x800A9BD8,0x800A9FC4)` | BATTLE 3 JAL `0x801E0A24` |
| `[0x800AAC88,0x800AAEBC)` | BATTLE 3 JAL `0x801E0634` |
| `[0x800AAEBC,0x800AB0B8)` | BATTLE 3 JAL `0x801E1594` |
| `[0x800ABCE0,0x800ABF5C)` | BATTLE 15 JAL `0x800ABC9C` |
| `[0x800AC218,0x800AC704)` | BATTLE 15 JAL `0x800AC1AC` |
| `[0x800B0498,0x800B06F4)` | BATTLE 15 JAL `0x800B03A0` |
| `[0x800B0B0C,0x800B0CE4)` | BATTLE 15 JAL `0x800B0A98` |
| `[0x800B1728,0x800B1A74)` | BATTLE 15 JAL `0x800B14C4` |
| `[0x800B25C0,0x800B2C90)` | BATTLE 15 JAL `0x800B2140` |
| `[0x800B2C90,0x800B326C)` | BATTLE 15 JAL `0x800B2294` |
| `[0x800B3324,0x800B3AE8)` | BATTLE 15 JAL `0x800B23A0` |
| `[0x800B3AE8,0x800B4220)` | BATTLE 15 JAL `0x800B24B4` |
| `[0x800B4220,0x800B43B8)` | BATTLE 15 JAL `0x800B25A8` |

Two computed jumps use separate numeric tables. Body `0x800A805C` subtracts
one from a stack byte, then accepts an unsigned index below eight at
`0x800A80F8`. Its scaled load at `0x800A810C` and jump at `0x800A8114`
read eight words at `0x80096974..0x80096994`. In order, their targets are
`0x800A811C`, `0x800A8160`, `0x800A825C`, `0x800A8180`,
`0x800A81CC`, `0x800A8218`, `0x800A825C` and `0x800A811C`.

Body `0x800B3AE8` bounds argument byte 9 below six at `0x800B3B18`.
Its scaled load at `0x800B3B2C` and jump at `0x800B3B34` read six words
at `0x80096A5C..0x80096A74`. Their targets are `0x800B3B3C`,
`0x800B3B44`, `0x800B3B44`, `0x800B3B44`, `0x800B3B4C` and
`0x800B3B4C`. Both loads have a NOP load delay, and both jumps have a NOP
branch delay. Every table target lies inside its consumer body.

Coverage records the 23,204 instruction bytes and 56 numeric table bytes
separately. Ordinary callee returns, stable code and tables, intact frames and
valid nonaliasing memory remain conditions. Referenced fields, buffers,
external callees and neighboring bytes retain their prior status. These
records establish source-byte roles. They do not establish ELF matches,
observed invocation, all-path feasibility, transitive effects, resource safety,
display or safe editing.

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

The XA initializer at `0x801636A0` clears EMI progress byte `0x80146494`
before calling the cue selector. The EMI READY callback at `0x80162230`
returns before reading a sector unless that byte equals one; XA completion
sets it to three. The initializer replaces the SYNC callback and preserves the
separate READY callback storage. A retained EMI callback therefore does not
establish consumption while its progress gate stays disabled.

This guard does not close other transfer paths. The SDK read callback at
`0x80177D08` can read sectors using destination and count storage at
`0x80185AA0` and `0x80185AA8`. Its lifetime during XA, other READY callbacks
and intervening progress writers remain unresolved. Submode bits and a single
callback observation do not prove that nonzero Form 1 payloads are unused.

The original SDK poll at `0x80178218` can take its timeout branch at
`0x80178258` and return without restoring READY. With no intervening READY
writer, its final `CdReady` call can return without dispatching an event or
changing the pointer.
This establishes a retained-callback path in the SDK read machinery. It does
not establish that this path occurs before XA or that a subsequent sector
transfer occurs.

Coverage separately identifies the complete SDK read callback instruction body
at `[0x80177D08,0x80177F1C)`, executable file offsets `[0xE1D08,0xE1F1C)`.
The exact original executable identity remains required. The qualified installer
at `0x80177F1C` loads that address at `0x80178068` and `0x8017806C`, then calls
setter `0x80175698` at `0x80178078`. The setter stores it at `0x80185784`.
The qualified dispatcher at `0x80176468` loads the slot at `0x801765EC` and
conditionally calls it at `0x80176608`. Registration, setter, dispatch and all
133 body words, including every control-transfer delay word, reconcile.

This instruction classification assumes pointer continuity, ordinary helper
returns, valid nonaliasing memory, intact frames and stable code. The completion
callback loaded from `0x80185A94`, referenced data and neighboring bytes retain
their prior status. Matching ELF output, actual invocation, callback lifetime,
feasible paths, transfer safety, XA payload roles, display and safe editing remain
unverified.

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

The SHOP Sell weapon-row call at `0x801DC8E0` uses GAME `0x801AF800`, which passes
the resolved record pointer directly to renderer `0x8014F800`. The name counter
at GAME `0x801AF20C` stops at NUL or twelve encoded bytes. An ordinary Sell
observation verifies an edited full-field weapon name with a twelve-byte renderer
limit. Runtime acceptance covers the observed weapon row and call path.

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

Extract the selected entry as `bof3.inline-text/v47`. Its first eighteen strings include
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

Six configuration captions use `game-literal` with these capacities, including
NUL: `Msg Speed` at `0x801D0DE0`, 10 bytes; `Window Clr` at `0x801D0DEC`, 11;
`Background` at `0x801D0DF8`, 11; `Sound` at `0x801D0E04`, 6; `Autorun` at
`0x801D0E0C`, 8; and `Controller` at `0x801D0E14`, 11. Spaces encode as `0xFF`.
The reader at `0x801E2898` copies each field to its own stack buffer and draws it
through `0x8014F800` or `0x8014FC90` with count `0xFF`. Its caller at
`0x801E26A8` supplies exactly six row selectors. References remain empty because
the reader forms these addresses in instructions. Preserve alignment and the
following tables. These captions have source proof only; the settings-field
observations above do not establish their edited rendering or visual fit.

Nine sort captions use the same literal codec. Capacities include NUL. Each
retains its original reference at `0x801ED534 + 4*index`.

| Index | Address | Caption | Capacity |
| --- | --- | --- | --- |
| 0 | `0x801ED4C8` | SORT | 5 |
| 1 | `0x801ED4D0` | ManualSort | 11 |
| 2 | `0x801ED4DC` | NormalItem | 11 |
| 3 | `0x801ED4E8` | CombatItem | 11 |
| 4 | `0x801ED4F4` | Power | 6 |
| 5 | `0x801ED4FC` | Defence | 8 |
| 6 | `0x801ED504` | Kind | 5 |
| 7 | `0x801ED50C` | High AP | 8 |
| 8 | `0x801ED514` | Low AP | 7 |

Reader `0x801EA1F0` draws the heading and selected rows through `0x8014F800` with
count `0xFF`. Publisher `0x801D8390` selects groups 5 or 6 for ManualSort, High AP
and Low AP. The five item-sort captions have a separate fresh menu-to-Sort route
through initializer `0x801D177C` and publisher `0x801D3884`. Its group 0..3 bound
requires the task-pool reset when byte `0x80148660` differs from 1. That bound
does not cover the reset-bypass route or other menu modes. Preserve pointer and
group tables, alignment, task priority and overlay residency. NormalAbil and
CombatAbil remain unsupported. Natural activation, edited rendering and visual
fit remain unverified.

Retain all 148 allocations and edit only `text`. Re-extract older inline
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
Three further `game-literal` fields at `0x801D18F4`, `0x801D18F8` and
`0x801D18FC` contain `...`, `△` and `×` in the game character table. Their
capacities are four, two and two bytes including NUL. Calls at `0x801D1490`
and `0x801D14D8` pass the first field to `0x8014FC90` with a three-glyph limit.
Calls at `0x801D14B8` and `0x801D1500` pass the other fields with one-glyph
limits. The renderer stops at NUL or its count. These fields have direct address
references and retain empty pointer-reference arrays. The two alignment bytes
after each single-glyph field, pointer words and other labels remain protected.
Controlled runtime observations verify the three edited ASCII labels. The display
selects `STEREO` when byte `0x80144951` is zero and `MONO` when it is nonzero.
Edited game-glyph rendering, natural menu and configuration producers, audio
output and the other labels' consumers remain unverified.

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

SHISU also exposes fifteen command labels selected by the five original group
records at `0x801D9CE8`. Their `game-literal` allocations include NUL and permit
three or four encoded body bytes. Retain the group index range 0..4; the local
reader does not enforce it. Consumer `0x801D5C7C` selects pointer words from
`0x801D9C8C`, queries width through GAME `0x801AF20C`, and draws through
`0x8014F800` with count sixteen. The width reader stops at NUL, and these
fields produce counts 0..4. The other eight table labels remain unsupported.

SHOP also exposes `SORT`, `LOOK`, `High AP` and `Low AP` with capacities of
five, five, eight and seven bytes including NUL. Game spaces encode as `0xFF`.
Wrapper `0x801E3F34` obtains the panel task and calls consumer `0x801E3F5C`.
The consumer selects the header and three rows through pointer table
`0x801E5EFC` and count/index bytes `03 01 02 03` at `0x801E5F0C`.
Each path calls `0x8014F800` with count `0xFF`; NUL bounds each field.

SHOP also exposes twenty-two command labels selected by nine five-byte records
at `0x801E59C0`. Each field permits three or four body bytes plus NUL.
Code 13, `Bttn`, remains unsupported. Caller `0x801E29C8` loads group and
selection from panel-task bytes +0xA and +0xB, then calls `0x801DA77C`.
The reader selects pointer words from `0x801E5964`, queries the same GAME
width reader, and draws through `0x8014F800` with count sixteen. Retain group
0..8, a valid panel root and a returning valid state handler. The caller does
not establish the natural group bound.

Three SHOP stat captions, `Pwr`, `Def` and `Agl`, occupy six-byte records at
`0x801E5A10`. Only each record's first four bytes are editable, including NUL;
its final two padding bytes stay protected. Fields have empty pointer-reference
lists because the reader computes their addresses. Dispatcher `0x801E2C18`
calls `0x801DB9F0`, whose loop draws exactly three rows through `0x8014F800`
with count `0xFF`. Preserve valid task, record and returning-helper conditions.

These fields use `game-literal`. Shorter or empty edits retain NUL and
zero-fill the field. Packing preserves alignment bytes, pointers and selection
records. No relocation is supported. Natural entry, producer bounds, visual
fit, rendering and resource safety remain unverified.

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
remains empty. The filename format remains unsupported.

SHOP's ten Shift-JIS digit strings at `0x801E540C + 4*i`, for `i` from 0 to 9,
use `shift-jis-digit`. Each three-byte allocation holds one full-width digit
`０` through `９`, encoded as `82 4F` through `82 58`, then NUL. The fourth
alignment byte stays protected. Pointer words at `0x801E5434 + 4*i` retain their
original addresses. The save-title formatter at `0x801D7080` selects seven
strings and calls `sprintf` at `0x801D71F0`. It substitutes the slot, hour,
minute and level digits into the Shift-JIS title. Replacements must retain one
full-width digit and cannot change the formatted byte count. Keep `renderers: []`.
The title format at `0x801D0C98` uses `shift-jis-title-format` in a 65-byte
allocation including NUL. It retains seven plain `%s` conversions and exactly
64 encoded bytes. The literals use full-width Latin letters, digits, ideographic
space, parentheses and full stop. Each conversion receives a two-byte digit,
so the formatted title also fills 64 bytes. Pad shorter wording with ideographic
spaces. Other conversions, ASCII literals and different lengths are refused.
Packing preserves the following nonzero bytes and all argument selectors.

Static original instructions establish the title path. Its caller passes
`0x80145AD4` to the save-header producer at `0x801EF27C` from call site
`0x801D6A08`. The producer's BIOS `strcpy`
call at `0x801EF2E8` copies it to `0x800C1804`. The title occupies 64 bytes;
its NUL replaces the first reserved byte, already zeroed by the producer.
Retaining the full title width preserves the original write span and prevents
stale trailing title bytes. As in the original, all seven selectors must
name digit strings: slot is at most 9, and the sampled hour, minute-tens and
level-tens bytes are at most 99. Edits do not change these conditions.

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

`bof3.inline-text/v47` retains `source`, `source_sha256`, `layout`, `entry`,
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

The `US COMMU00 names and numeric format` layout covers entry 0 of `BIN/ETC/COMMU00.EMI`
and entry 14 of `BIN/WORLD04/AREA175.EMI` through `AREA185.EMI`. Each payload
loads at `0x801EEC00`. Its sixty nine-byte records start at `0x801F2700`.
The first five bytes of each record use `game-fixed-literal`; the remaining
four attribute bytes stay unchanged. All five name bytes are available for
literal game characters. Shorter edits zero-fill unused bytes.

The name consumers belong to `BIN/ETC/COMMU02.EMI` entry 8, also present as
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

The same layout appends `%d` at `0x801EEC44`, with capacity three including
NUL and codec `ascii-i32-decimal-format`. Its source allocation fits only the
original format. Preserve the alignment byte at `0x801EEC47`. Original consumers
`0x801F1318` and `0x801F13B8` issue seven formatter calls using two unsigned
byte arguments and five full-word arguments. All calls use the shared format.
Within each consumer, successive calls write `0x20`-spaced slots from
`0x801490D8`. Signed-word arguments
require at most eleven decimal bytes plus NUL. The first consumer dispatches
action `0x55`; the second dispatches `0x56`. Their final display consumer remains
unverified, so this row retains `renderers: []` and an empty pointer-reference
list. The previous sixty name rows retain their identities and order.

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

The `US COMMU01 list and footer text` layout covers entry 7 of original US
`BIN/ETC/COMMU01.EMI`, loaded at `0x801D0C00`. It exposes three footer fields:

| Address | Original text | Capacity including NUL | Codec | Maximum formatted bytes before NUL |
| --- | --- | --- | --- | --- |
| `0x801D0C10` | `%2d` | 4 | `ascii-i8-decimal-format` | 4 |
| `0x801D0C14` | `%d` | 3 | `ascii-u8-decimal-format` | 3 |
| `0x801D4C30` | `Culture` | 8 | `game-literal` | Not formatted |

Original consumer `0x801D3038` loads a signed byte from `0x801455C2` and an
unsigned byte from `0x801455C4`. Formatter calls `0x801D3500` and `0x801D354C`
write into `0x80145AD4`, with the destination set in each call's delay slot.
The signed domain `[-128, 127]` needs at most four decimal bytes; the unsigned
domain `[0, 255]` needs at most three. The signed codec retains one decimal
conversion with optional width 1..4 and zero padding when both encoded and
formatted limits fit. The three-byte unsigned allocation fits only `%d` plus
NUL. Its format cannot change. Calls `0x801D3514` and `0x801D3560` draw the
scratch output through `0x8014FF0C`, with the source set in their delay slots.

Call `0x801D3534` draws `Culture` through `0x8014F800`, with a seven-character
limit and the source pointer stored in its delay slot. Shorter or empty literal
edits retain NUL and zero-fill the remaining field. Preserve the alignment byte
at `0x801D0C17` and the icon data at `[0x801D4C38,0x801D4D34)`.

Twenty separate eight-byte list fields occupy `[0x801D4D34,0x801D4DD4)`.
They use `game-fixed-literal` and reserve no terminator byte. `Merchant`,
`Explorer`, `Antiques` and `Handyman` occupy all eight bytes without NUL.
Shorter or empty edits zero-fill their remaining bytes. Consumer `0x801D4178`
selects the first ten rows by index. Consumer `0x801D44E0` selects the lower
ten through four first/count pairs at `[0x801D4DD4,0x801D4DDC)`. Both pass a
count of eight to `0x8014F800`. That reader looks ahead one following byte
before exiting without processing it. The kind table stays protected.

The list count must be at most ten and the kind index must be 0..3. These
callers do not enforce those producer bounds. All twenty-three fields retain
empty pointer-reference lists. Recognition pins the payload outside the fields,
and packing binds the original archive identity. Natural selection, visual fit,
producer bounds and edited rendering remain unverified.

The `US SCENA17 credits text` layout covers entry 0 of original US
`BIN/SCENARIO/SCENA17.EMI`, loaded at `0x801F6C00`. It exposes 167 disjoint
literal runs through `ascii-credits-literal`, totaling 2,383 editable bytes.
Each run requires exactly its original byte count and permits only ASCII A-Z,
a-z and spaces. Pad shorter wording with spaces. Runs reserve no terminator
byte. NUL, punctuation, digits and command introducers are refused.

The pointer reader `0x801F938C` calls parser `0x801F91F0`, whose original
instructions map uppercase letters to atlas indices 0..25 and lowercase to
26..51. ASCII space advances X by twelve. Commands `!n`, `#n` and `@` set
style, select special glyphs or submit a special image. All 91 command bytes,
145 parent-string NULs, padding and offsets remain protected. The table's 172
pointers name 145 parent strings. Only 162 pointers directly target a first
literal run and appear in its references. Ten pointers target nine strings
with a protected command prefix. Those words remain pinned and must not be
attributed to later literal-run addresses. No relocation is supported.

The reader's complete callers are `0x801F8110` and `0x801F8170`. Its stream
index has no local upper bound, and its sixteen-word terminal probe can read
one word beyond the recorded table at `0x801F981C..0x801F9DBC`. These edits
preserve that original behavior. Exact byte widths do not establish equal
pixel widths or glyph draw counts. Natural selection, whole-roll traversal,
GPU resource safety and edited rendering remain unverified.

The `US SCENA10 numeric text` layout covers entry 0 of original US
`BIN/SCENARIO/SCENA10.EMI`, loaded at `0x801F6C00`. Both allocations include NUL:

| Address | Original format | Capacity | Codec | Maximum formatted bytes before NUL | Renderer |
| --- | --- | --- | --- | --- | --- |
| `0x801F6C08` | `%02d` | 5 | `ascii-u16-decimal-format` | 5 | `0x8014F800` |
| `0x801F6C10` | `*%2d` | 5 | `ascii-u8-decimal-field-format` | 4 | `0x80150098` |

Consumer `0x801F719C` obtains a value from `0x8016628C`, masks it to sixteen
bits in formatter call `0x801F7330`'s delay slot and writes to `0x80145AD4`.
Draw call `0x801F7364` retains a two-character limit. That count does not
establish a two-digit argument bound. Consumer `0x801F7AD4` masks its value to
eight bits at `0x801F7B84`, formats it at `0x801F7BCC` and passes the same
buffer to `0x80150098` at `0x801F7BE8`.

Each format retains one decimal argument with optional width 1..4 and zero
padding. Printable ASCII literals must fit both the five-byte allocation and
the formatted bound. Preserve the unsupported `%s` field at `0x801F6C04`,
three alignment bytes at `0x801F6C0D..0x801F6C10` and the nonzero bytes after
`0x801F6C15`. Code addressing retains empty pointer-reference lists. Natural
selection and edited rendering remain unverified.

The `US SCENA12 counter text` layout covers entry 0 of original US
`BIN/SCENARIO/SCENA12.EMI`, loaded at `0x801F6C00`. Its `%03d` field at
`0x801F6C04` has capacity five including NUL and uses
`ascii-u8-decimal-format`. Original consumer `0x801F75EC` reads unsigned byte
`0x801FD288`, formats into `0x80145AD4` at call `0x801F762C`, then draws through
`0x8014FF0C` at call `0x801F7640`. Both calls assign their buffer argument in
the delay slot. Retain one decimal argument, at most four encoded bytes before
NUL and three formatted bytes before NUL. Bytes from `0x801F6C09` stay protected.

The `US BOSS025 turn-count text` layout covers entry 13 of original US
`BIN/BOSS/BOSS025.EMI` and its identical payload in `BOSS027.EMI` entry 16,
both below `BIN/BOSS` and loaded at `0x800C1800`:

| Address | Original text | Capacity including NUL | Codec | Maximum formatted bytes before NUL |
| --- | --- | --- | --- | --- |
| `0x800C184C` | `%2d` | 4 | `ascii-i32-decimal-format` | 11 |
| `0x800C32A8` | `turns left` | 11 | `game-literal` | Not formatted |

Original consumer `0x800C3088` loads a full word from `0x801463D0` and computes
wrapping `21 - word` in the formatter call's delay slot at `0x800C30C4`.
There is no proved 0..21 bound. Preserve the signed-word maximum of eleven
decimal bytes plus NUL. Call `0x800C30D4` draws the result from `0x801490F8`
through `0x8014FF0C`. Call `0x800C30F4` draws the label through `0x8014F800`
with limit ten, passing its pointer in the delay slot. Shorter or empty label
edits zero-fill the eleven-byte field. Preserve all nine trailing zeros from
`0x800C32B3` through `0x800C32BB`.

Both layouts retain empty pointer-reference lists and pin all payload bytes
outside their allocations. Packing binds each original archive. Original
instructions establish conditional source and output bounds; natural selection
and edited rendering remain unverified.

The `US COMMU05 faery labels and counts` layout covers entry 7 of original US
`BIN/ETC/COMMU05.EMI`, loaded at `0x801D0C00`. It exposes six fields:

| Address | Original text | Capacity including NUL | Codec | Maximum formatted bytes before NUL |
| --- | --- | --- | --- | --- |
| `0x801D0C04` | `%d/%d` | 6 | `ascii-i8-plus-one-u8-decimal-pair-format` | 8 |
| `0x801D0C0C` | `%2d` | 4 | `ascii-u8-decimal-format` | 3 |
| `0x801D195C` | `faeries` | 8 | `game-literal` | Not formatted |
| `0x801D1964` | `faery` | 6 | `game-literal` | Not formatted |
| `0x801D196C` | `faeries` | 8 | `game-literal` | Not formatted |
| `0x801D1974` | `faery` | 6 | `game-literal` | Not formatted |

The four labels retain their original spelling and separate allocations.
Readers `0x801D1048` and `0x801D129C` choose singular text when their respective
unsigned count byte equals one, otherwise plural text. Calls `0x801D1180` and
`0x801D13CC` pass the selected label to `0x8014F800`; both delay slots set a
seven-character limit. Shorter or empty literal edits retain NUL and zero-fill
the remaining allocation. Preserve the alignment bytes at `0x801D196A..0x801D196B`
and `0x801D197A..0x801D197B`.

Formatter call `0x801D0EC0` uses `%d/%d`. Its first argument is a signed byte
plus one, bounded by `[-127, 128]`. Its second is the sum of three unsigned count
bytes masked to eight bits, bounded by `[0, 255]`. The pair codec retains two
ordered decimal conversions with digit budgets of four and three, an eight-byte
output limit, and optional widths from 1 through 4 when they fit. It does not
reuse the larger signed-pair bound. Call `0x801D0ED4` draws the scratch output
at `0x80145AD4` through `0x80150098`.

Calls `0x801D1114` and `0x801D1364` pass unsigned bytes to the `%2d` format and
use the same scratch destination. It retains one decimal conversion and at most
three encoded bytes plus NUL, with a three-byte formatted bound. Calls
`0x801D1130` and `0x801D1380` draw through `0x8014FF0C`. Preserve both formats'
NULs and the alignment bytes at `0x801D0C0A..0x801D0C0B`. All six fields use
direct code addresses and retain empty pointer-reference lists. Recognition pins
the complete payload outside these allocations; packing binds the original
archive identity and preserves other entries. Original instructions establish
these source and output bounds. Natural selection and edited rendering remain
unverified.

The `US AREA030 panel text` layout covers entry 4 of the original US
`WORLD00/AREA030.EMI`, `WORLD02/AREA089.EMI` and `WORLD03/AREA129.EMI` archives
below `BIN`. Each payload loads at `0x801D0C00`. Each archive requires its own
source-bound document and edited output.

This is a fishing UI layout. It includes seven packed menu labels. Consumer
`0x801E1004` also resolves fish names through GAME item records; these dynamic
names are outside this layout's editable allocations.

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
| `0x801E21C0` | `Gear` | 4 | `game-fixed-literal` |
| `0x801E21C4` | `Data` | 4 | `game-fixed-literal` |
| `0x801E21C8` | `Rule` | 4 | `game-fixed-literal` |
| `0x801E21CC` | `ROD` | 3 | `game-fixed-literal` |
| `0x801E21D0` | `LURE` | 4 | `game-fixed-literal` |
| `0x801E21D4` | `EQUIP` | 5 | `game-fixed-literal` |
| `0x801E21D9` | `GUIDE` | 5 | `game-fixed-literal` |

The same payload includes twelve fishing help-text fields through
`game-exact-literal`. Each edit must retain exactly the recorded number of
encoded literal bytes. Pad shorter wording with spaces, which encode as `0xFF`.
Empty text, NUL, dialogue commands and shorter or longer fields are refused.

| Address | Original text | Editable bytes |
| --- | --- | --- |
| `0x801E1DAC` | `Casting...` | 10 |
| `0x801E1DB8` | `Set rod & lure...` | 17 |
| `0x801E1DCC` | `Quit fishing...` | 15 |
| `0x801E1DDC` | `Wind reel...` | 12 |
| `0x801E1DE8` | `Move Rod...Directional button` | 29 |
| `0x801E1E08` | `Reset lure...` | 13 |
| `0x801E1E18` | `……` | 2 |
| `0x801E1E1C` | `……` | 2 |
| `0x801E1E20` | `Line got caught on rocks!` | 25 |
| `0x801E1E3C` | `Line broke; lure lost!` | 22 |
| `0x801E1E54` | `Lure fell off; catch got away!` | 30 |
| `0x801E1E74` | `,` | 1 |

The thirteen twelve-byte records at `0x801E1E78` retain their byte counts and
pointer words at record offset four. Records 1 and 12 share `0x801E1DB8`; its
single editable field retains both references. All thirteen pointers remain
immutable. Readers at `0x801D3824` and `0x801D3898` pass the selected pointer and
count to `0x8014F800`. Selection setup at `0x801D36F8` chooses the work route.
Initializer `0x801D39B8` stores the selected pointer at work offset `0x50` and
clears the revealed count at `0x49`; the declared count remains in the table.
The reveal reader at `0x801D3A54` advances one source byte per token, bounded
by that unchanged table count, and calls glyph-quad renderer `0x801D9740`.
The advance and unreveal paths at `0x801D3C74` and `0x801D3F40` reuse the retained
source and table count. The reveal loop ignores NUL, so zero padding would alter
its tokens. Exact-width edits preserve source advancement and the table count.

`Wind reel...` has no NUL before the next field. The line-broke record retains its
original count of 23, including its following NUL; only its 22 literal bytes are
editable. Preserve every original terminator, alignment byte, record count,
attribute and timer. These bindings apply to the thirteen reviewed table rows
and their initialized retained work. Natural selection of every row and edited
rendering remain unverified; the direct readers do not check the table index.

The seven menu labels use every byte of their fixed fields without reserving
NUL. Shorter edits zero-fill the field. The status-cell reader at `0x801D7C10`
passes `Gear`, `Data` and `Rule` to renderer `0x8014F800` at calls `0x801D7CD0`,
`0x801D7D04` and `0x801D7D38`, each with a four-glyph limit. The rod and lure
readers call that renderer at `0x801D820C` and `0x801D88A4` with limits of three
and four. The equipment reader at `0x801D88E0` draws `EQUIP` and `GUIDE` at
`0x801D894C` and `0x801D8CF0`, each with a five-glyph limit. Preserve the zero
bytes at `0x801E21CF` and `0x801E21DE..0x801E21DF`. Code supplies the field
addresses directly; pointer-reference arrays stay empty. Original instruction
evidence establishes these source and draw-call bindings. Edited rendering
remains unverified.

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

The [texture owner](../../bof3-textures/docs/byte-ranges.md#original-us-mode-3-vram-tiles)
supplies source-bound whole-VRAM recognition. Text filtering requires current
original archive qualification and a whole-payload result. It does not persist
these exclusions as unconditional registry windows. The retained index fingerprint
includes the texture owner's interpretation, so changed owner data requires an
index rebuild.
