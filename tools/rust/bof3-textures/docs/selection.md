# Sprite selection

## Sprite roots

The `sprite_bank::select` library function follows the original US helper at
`SLUS_004.22@0x8014dc08`. Supply decoded bank bytes, a root ordinal, the original
byte selector and the image-skip condition from the same consumer observation.
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
an error. Selection returns the two-byte program header; it does not initialize
or advance playback.

The caller must establish the active resident base and entity together. Table
membership does not assign an action, game direction, playback mode, image or
palette. This lookup does not prove runtime reachability, upload residency or
rendered appearance.
