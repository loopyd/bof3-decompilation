---
type: Reverse-engineering method
title: Data discovery
description: Reusable procedures for locating and inferring BOF3 data structures.
tags: [methods, discovery]
---

# Data discovery

## Pointer maps

Pointer-map rows use `HEX_OFFSET@ARCHIVE`. Parse the offset as an archive-file
offset, discard comments and empty sentinels, then check that the complete
record fits inside the named archive.

```text
parse row -> resolve archive -> check offset + record_size -> decode record
```

For BOF3 US v1.1, select `_1.1` maps whenever a versioned map exists.

## String anchors

To locate an inline-name table:

1. Encode two or more known names using the owning data encoding.
2. Search archive bytes for each sequence.
3. Compare the distance between hits with the expected record stride.
4. Test neighboring records for the same layout.
5. Reject a candidate if its full count exceeds the archive or crosses an EMI
   entry boundary unexpectedly.

## Structure inference

Use records that differ in one known property:

1. Align records by their known stride.
2. Diff bytes across controlled examples.
3. Associate changing bytes with the known property.
4. Confirm width, signedness, and bit ordering across additional records.
5. Confirm runtime loads/stores before giving the field a semantic C name.

## Cross-reference

Correlate a candidate table with:

- runtime indexing and load width;
- duplicate copies in other archives;
- names or values from the pinned research inputs;
- callers that consume the resulting IDs;
- object or UI behavior visible from the game.

Agreement between copies is evidence of layout, not proof of which copy owns
runtime state.
