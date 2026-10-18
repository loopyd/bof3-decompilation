# Sprite selection

The `sprite_bank::select` library function follows the original US helper at
`SLUS_004.22@0x8014dc08`. Supply decoded bank bytes, a root ordinal, the original
byte selector and the image-skip condition from the same consumer observation.
The function decodes the supported three-root bank layout before resolving the
selected slot.

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
