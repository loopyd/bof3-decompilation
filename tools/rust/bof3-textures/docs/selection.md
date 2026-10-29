# Sprite selection

## Sprite roots

The `sprite_bank::select` library function follows the original US helper at
`SLUS_004.22@0x8014dc08`. Supply decoded bank bytes, a root ordinal, the helper's
selector byte and the image-skip condition from the same consumer observation
or qualified conditional static path. Static qualification does not establish
execution.
The function decodes a supported three-root bank or standalone root before
resolving the selected slot. A standalone payload has root ordinal zero and
keeps every offset in its original payload coordinates. It does not include a
synthetic root table. Its RAM destination does not prove which active entity
selects it.

The main call at `SLUS_004.22@0x8014d868` uses entity byte `0x4b` when entity
byte `0x24` has bit zero set. The call at `SLUS_004.22@0x8014d564` instead passes
its incoming `a0` low byte. The other main branch uses a resident table and is
outside this function.

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

### Stored mirror edits

The `sprite_bank::encode_selections` library function accepts decoded bank bytes
and explicit `SelectionEdit` values. Each value supplies `root`, `selector`,
`skip_image` and the desired raw `mirror_flag`. The caller must qualify the slot
and image-skip condition just as for `select`. An in-bounds triple does not prove
that its selector is valid for a game consumer.

The encoder validates each selected image and program through the same lookup.
The image-skip branch leaves the image index opaque. Only byte two of a selected
triple can change. The image and program indices, unselected tail bytes and all
other bank bytes remain unchanged. Zero disables horizontal mirroring; nonzero
values enable it in the qualified original consumer.

Changed views of one physical mirror byte must agree. An unchanged view does not
override an edit. The encoder bounds the edit list to 256 times the number of
decoded roots. It rejects missing roots, invalid references and conflicting
edits. It does not infer direction names, discover selectors, establish an
active entity or publish a package. The package's `sprite.json` still keeps
selection tails read-only. Separately qualified slots use `selections.json`.

### Pack field-route mirror slots

The 19 qualified three-root `PL` field banks export `selections.json` with schema
`bof3.sprite-selections/v1`. Its `slots` list contains selectors 0 through 7 in
each root, `skip_image=false` and the stored `mirror_flag`. The package validates
all 24 image and program references before exporting them. This image-enabled
view does not claim observed entity state. Only `mirror_flag` is editable.
Keep every other field and the list length unchanged. Find the member path in
the package manifest's matching export. Other families and tail bytes remain
outside this edit route.

Original `BIN/ETC/GAME.EMI#0` routine `0x801BEE5C` has a route-update path at
`0x801BEFA0..0x801BEFC8` that masks the updated entity byte `0x08` with `7`.
It stores the byte, reloads the scratch pointer at `0x1F800044`, reads that
entity's byte `0x08` and calls `SLUS_004.22@0x8014D664`. The direct wrapper passes
the selector's low byte to `0x8014D54C` and initializes playback with a zero
cursor. Other paths in the routine can pass an unbounded existing byte.

Applying this range to a bank requires the fresh scratch pointer to keep
selecting the route actor and its `0x4C` word to identify the qualified root.
Nested helpers must preserve the relevant saved registers and stack. The
`0xA00`-byte forward copy to `0x800F0800 + 0xA00 * entity[5]` must remain within
valid buffers and must not alias the source, bank, actor, scratch or stack.
These premises qualify conditional edit slots. They do not prove that all eight
selectors execute in every root or assign compass, action or age names. The
entity's image-skip bit still controls whether the image branch executes.

```sh
bin/harness textures unpack out/extracted/BIN/PLCHAR/PL27A.EMI \
  --source BIN/PLCHAR/PL27A.EMI --output out/textures/pl27a-edit
# Edit mirror_flag in the exported selections.json.
bin/harness textures pack out/textures/pl27a-edit \
  --output out/textures/pl27a-packed
bin/harness textures verify out/textures/pl27a-packed \
  --package out/textures/pl27a-edit --json
bin/harness textures unpack out/textures/pl27a-packed \
  --package out/textures/pl27a-edit --output out/textures/pl27a-reopened
```

Use new destinations. Verification exits `1` while broader graphics acceptance
is incomplete; inspect its byte-check results separately. The reopened selection
document reports the packed mirror byte. Image, palette and animation edits can
be combined with this edit before the outer bank is encoded. Other selection
bytes remain protected.

The qualified original incoming context at `SLUS_004.22@0x8014DC14` supplies
selector `5` and entity `0x80145E90`. Its root word points to `0x8007DEE0`,
root offset `272096` in the complete source-bound bank at `0x8003B800`.
The selected triple is at decoded offset `310043`; its mirror byte is at
`310045`. This is a preinstruction root-load checkpoint. It does not establish
retired triple reads or stores, other observed selectors, CLUT choice, uploads or
rendered appearance.

### Entry requests

The request passed to `SLUS_004.22@0x8014d6b8` can differ from the final
selector byte. That wrapper masks the request to eight bits. When request bit
`0x80` is set, the main path at `0x8014d6dc` uses the low seven bits to select
a four-byte record through the pointer table at `0x80181c7c`. Entity word
`0x64` selects the table pointer. Record byte two becomes entity byte `0x4b`,
and byte three becomes entity byte `0x2a`. The first two record bytes form an
ID passed to the metadata lookup at `0x8014dd3c`.

For this main path, use the resulting entity byte with `sprite_bank::select`.
The library does not evaluate the entry request or its mutable tables.
Those tables and entity state do not establish a valid request domain,
direction names or a physical bank binding. Preserve their context when
qualifying a selector.

### Definition lookup

The metadata helper at `SLUS_004.22@0x8014dd3c` masks `a0` to 16 bits and
selects the first matching definition ID. It uses `a1` for the record byte
offset. Each write destination comes from a fresh load of the scratchpad
pointer at `0x1f800044`; incoming `a1` does not identify that destination.
The helper also reloads the mutable definition-table pointer between fields.
Combining the resulting page, V base, CLUT selector, depth and group into one
draw state requires stable pointers and destination state across the lookup.
A successful lookup does not establish later resident-program selection.

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
