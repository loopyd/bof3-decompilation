# Sprite selection

## Sprite roots

The `sprite_bank::select` library function follows the original US helper at
`SLUS_004.22@0x8014dc08`. Supply decoded bank bytes, a root ordinal, the final
selector byte at entity `0x4b` and the image-skip condition from the same
consumer observation.
The function decodes a supported three-root bank or standalone root before
resolving the selected slot. A standalone payload has root ordinal zero and
keeps every offset in its original payload coordinates. It does not include a
synthetic root table. Its RAM destination does not prove which active entity
selects it.

The main selector dispatches to this helper when entity byte `0x24` has bit
zero set. Its other branch uses a resident table and is outside this function.

The helper reads three bytes at `selection_offset + 3 * selector`. Byte zero
selects an image. Byte one selects an animation program. Byte two is copied to
entity byte `0x2a` and controls mirroring. It is not a palette selector.

Pass `skip_image=true` when entity byte zero has bit one set. That path returns
`image=None` and retains the raw `image_index` without dereferencing it. Program
selection still occurs. Without the skip condition, an image index outside its
offset table causes an error. A missing root, an out-of-bounds triple or a
program index outside its table also causes an error.

The result preserves the selected program header address and bounded program
metadata. The original initializer later advances the entity's program pointer
past its two-byte header. This lookup does not initialize or advance playback.

The caller must qualify the valid selector for the root. Bytes that fit the
root extent can still be padding or another unrecognized tail record. This
function does not infer a table count, enumerate valid selectors, make tail
bytes editable or bind a physical source to an active runtime entity. It does
not choose a palette, submit image uploads, establish residency or render a
frame. Conditional image selection cannot be treated as an unconditional bank
or image dependency.

### Entry requests

The request passed to `SLUS_004.22@0x8014d6b8` can differ from the final
selector byte. That wrapper masks the request to eight bits. When request bit
`0x80` is set, the main path at `0x8014d6dc` uses the low seven bits to select
a four-byte record through the pointer table at `0x80181c7c`. Entity word
`0x64` selects the table pointer. Record byte two becomes entity byte `0x4b`,
and byte three becomes entity byte `0x2a`. The first two record bytes form an
ID passed to the metadata lookup at `0x8014dd3c`.

Use the resulting entity byte when calling `sprite_bank::select`. The library
does not evaluate the entry request or its mutable tables. Those tables and
entity state do not establish a valid request domain, direction names or a
physical bank binding. Preserve their context when qualifying a selector.

## Resident banks

The `resident::select(source, group_index, program_index)` library function
follows the resident branch at `SLUS_004.22@0x8014d878`. Supply the resident
payload referenced by `0x801459f0`, the same entity's unsigned 16-bit value at
`0x2c`, and its program byte at `0x4b`. The function validates the recognized
resident layout before selecting a program.

The group offset is a 32-bit value relative to the resident base. The program
offset is a 16-bit value relative to that group. The result contains the program
header offset, bounded end and original step pairs in payload coordinates.
Repeated offsets retain the original alias. A missing group or program returns
an error. A selected unresolved program also returns its original refusal reason;
other decoded programs remain selectable. Partial documents use
`bof3.resident-animation/v3`, with `status: decoded` or `status: unresolved`
for each program. Unresolved programs and their complete source spans are read-only. Selection returns the two-byte program header; it does not initialize
or advance playback.

The caller must establish the active resident base and entity together. Table
membership does not assign an action, game direction, playback mode, image or
palette. This lookup does not prove runtime reachability, upload residency or
rendered appearance.
