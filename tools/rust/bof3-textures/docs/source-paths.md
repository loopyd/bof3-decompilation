# Graphics source paths

The catalog distinguishes stored bytes, decoded records, RAM destinations and
draw-time sources. A declared load address identifies a destination. It does not
prove that an entry was loaded, remained resident or supplied a later draw.
Equal bytes at separate original locations retain separate resource identities.

## Coordinates and dependencies

| Source layer | Coordinates | Required relationship |
| --- | --- | --- |
| Original file or EMI entry | Physical file and entry offsets | Qualified original path, size and hash |
| Outer compressed bank | Decoded-bank offsets | Qualified compressed entry and bounded decoder |
| Nested image, palette or program | Containing payload or decoded-bank offsets | Validated section and record references |
| Active RAM data | Runtime address | Observed or conditional load, current base and entity selection |
| Texture and palette in VRAM | Word coordinates and draw state | Upload, later writes, page, CLUT and consumer selection |

[Catalog records](catalog.md) defines the coordinate spaces and dependency
statuses exposed by queries. Decoded offsets cannot be added to an archive
offset to locate compressed source bytes. A compressed token can contribute to
several decoded records through back-references. A complete entry hash does not
partition its physical bytes into image, palette, metadata or nontext spans.

## Raw uploads and shared palettes

Qualified type-3 entries supply raw VRAM chunks. The original recorder at
`SLUS_004.22@0x80162698` derives their destinations from the entry descriptor;
the consumer at `0x80163378` submits only chunks whose complete rectangles fit.
Skipped chunks remain preserved source bytes. The upload descriptor does not
choose a texture depth, CLUT or game role.

Qualified type-0 palette entries declare RAM spans inside
`0x80033800..0x80037000`. The original loader and sector-copy path at
`SLUS_004.22@0x80162B08`, `0x801625E4` and `0x80162C14` connect entry bytes to
their declared destinations. The staging routine at `0x8014E284` and pending
upload consumer at `0x8014E22C` provide the conditional path to VRAM.
Completed loading, intervening writes and active palette selection are separate
dependencies. [Catalog records](catalog.md) describes the qualified FIRST,
title and area associations without extending them to other uses.

## Compressed and raw sprite banks

The [outer bank decoder](../src/bank.rs) handles qualified type-1 streams
declaring destination `0x8003B800`. Its original token consumer is
`SLUS_004.22@0x80164064`. The decoded bank can contain three sprite roots.
Qualified raw banks bypass this outer decoder, while standalone roots retain
their payload coordinates and unresolved root-installation dependency.

Each recognized root supplies image, palette, program and selection sections.
The original selector at `SLUS_004.22@0x8014DC08` chooses image and program
records independently and can skip the image upload. Nested image records use
the codec dispatch at `0x8014E6D0`: mode 1 expands packed indices, mode 2 uses
the bounded dictionary stream, and other modes submit raw words. Codec modes
do not identify GPU texture depth. [Sprite selection](selection.md) defines
the caller-supplied root, selector and image-skip conditions.

`stream-request`, `root-selection`, `image-stage`, `palette-stage` and
`resident-base` edges retain the conditional request and loading relationships.
PL, BPLD and BPLU family identifiers do not assign character names or field and
battle roles. Shared destinations and same-archive ordering do not establish
exclusive producers, completed decompression or draw-time residency.

## Resident programs and draw-state tables

The original initializer at `SLUS_004.22@0x8014EBB8` sets the resident base to
`0x800D3800` and the definition base to `0x800E3804`. The resident selector at
`0x8014D878` reads a 32-bit relative group offset, then a 16-bit group-relative
program offset. Qualified definition records can conditionally reference these
groups. Their page, V base and CLUT selector fields are draw metadata, not
physical texture or palette identities.

The catalog also recognizes the qualified shared BATTLE/BOSS metadata at
`0x800F0800` when its archive contains the matching common battle code. The
original path at `emi/battle/battle/03@0x801E6C84` conditionally sets this base
around a selected callback, then writes constant `0x800D3800` after return.
Callback selection and reachability of the resident renderer remain unresolved.
This path does not restore an observed previous base.

Resident sources share cell and animation layouts, not an established image or
palette. Active bases, entity flags, selectors and later uploads must agree
before a stored reference can identify a draw-time resource. Unresolved programs
remain visible and read-only. [Sprite selection](selection.md) describes the
resident lookup; [composition](composition.md) requires explicit draw context.

## Embedded data and runtime producers

Qualified `SHOP.EMI` entry 8 and `START.EMI` entry 9 contain icon data alongside
producer code. Their separate source identities retain the palette and three
frame spans described by the [save-icon owner](../src/save_icon.rs). The original
producer at each payload's `0x801EF27C` copies these bytes into an `SC` save
header. Source layout and editable reconstruction are supported. A qualified
original save-header capture, source loading, BIOS animation timing and edited
display remain unresolved. Save operations belong to the emulator owner.

`SLUS_004.22` and other original non-EMI files remain catalog inputs. Identifying
an executable as a consumer does not classify its remaining bytes as graphics
or prove the absence of embedded graphics. Unresolved embedded, generated and
media-owned dispositions remain coverage blockers.

The original helper at `SLUS_004.22@0x8014D54C` copies bytes through the selected
program pointer. That copy does not identify generated texels. Runtime-generated
texture producers require their own source, destination and consumer chain;
unknown producers stay unresolved. A RAM or VRAM capture alone establishes
captured contents, not the producer, physical source, upload history or complete
inverse edit. Constructing a draw packet also does not generate its sampled
texture bytes.

Full support remains incomplete while required source paths, dependencies,
packing operations or runtime observations are unresolved. Preserving unknown
bytes and successfully reconstructing an archive do not close those gaps.
