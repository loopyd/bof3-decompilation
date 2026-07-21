# Data discovery

## Pointer maps

Many BOF3 data tables are reached through pointer arrays rather than fixed
offsets. A pointer map is a contiguous array of 32-bit VRAM addresses, each
pointing to a variable-length record elsewhere in the same payload.

Discovery procedure:

1. Identify a stable accessor function (usually a `GET_*` or table-read
   routine) from the lifted C or m2c output.
2. Extract the base address and count from the accessor.
3. Read the pointer array at the base address (count × 4 bytes).
4. Dereference each pointer to locate the record start.
5. Infer record size from the gap between consecutive record starts (or from
   the trailing sentinel pointer, if present).

## Cross-archive duplication

Identical bytes in multiple archives do not imply shared ownership. Confirm:

- Same payload hash and load address → same target, same source.
- Same bytes, different load address → independent copies; lift separately.
- Subset bytes → one may be a truncated or padded variant; compare field-by-field.

## Inference from consumers

When a table has no explicit header, infer layout from consumers:

1. Collect all xrefs to the table base in the owning target.
2. Record every offset and access width (byte, half, word).
3. Build a field map from the union of observed accesses.
4. Validate against at least two independent consumers before promoting.

# Data verification

## Acceptance sequence

A data table or structure is accepted when:

1. **Offset stability**: every field offset is confirmed by at least one
   consumer instruction or pointer dereference.
2. **Width correctness**: access widths match the declared field types
   (byte fields use `lb`/`lbu`/`sb`, half uses `lh`/`lhu`/`sh`, etc.).
3. **Count validation**: array counts match the number of records reachable
   through the pointer map or iteration bounds.
4. **Cross-target consistency**: if the same table appears in multiple
   targets, field offsets and sizes agree.
5. **No orphan bytes**: every byte in the record is accounted for by a
   declared field or explicit padding.

## Duplicate data checks

When the same logical table appears in multiple archives:

- Byte-identical copies: confirm with SHA-256; document as duplicate.
- Structurally identical but different values: confirm layout matches;
  document as variant.
- Partial overlap: identify the shared prefix and divergent tail.

## Regression protection

Once a table is accepted:

- Record the verified offsets in the target-local `internal.h` as
  `ASSERT_OFFSET` or `ASSERT_SIZE` where practical.
- Add the table base to the target symbol map if not already present.
- Note the verification evidence (consumer addresses, archive hash) in a
  comment or `docs/specs/data/` entry.
