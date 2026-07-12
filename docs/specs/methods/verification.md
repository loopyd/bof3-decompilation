---
type: Reverse-engineering method
title: Data verification
description: Acceptance checks for BOF3 tables, structures, and duplicate data.
tags: [methods, verification]
---

# Data verification

## Acceptance sequence

1. Identify the game version and archive path.
2. State the coordinate space for every offset.
3. Check `offset + record_size * count` against the owning byte range.
4. Decode known sentinel and representative records.
5. Check field widths and signedness against runtime instructions.
6. Compare known duplicate ranges by hash and bytes.
7. Record complete generated evidence under `out/index/`.
8. Promote only stable layouts and values into tracked specs.

## Required coordinate names

- archive offset;
- EMI entry slot;
- payload-relative offset;
- runtime virtual address;
- record-relative offset.

Never publish an unlabeled `Offset` when more than one coordinate could apply.

## Duplicate data

Group byte-identical ranges by hash, then retain separate ownership records for
different archive paths or runtime addresses. A duplicate is not automatically
the canonical source.

## Failure conditions

Reject or return a finding to `out/` when:

- the selected version is ambiguous;
- an offset uses an unstated coordinate system;
- a record crosses its owning range;
- names decode only under ad hoc substitutions;
- field semantics disagree with runtime access width or signedness;
- a claimed duplicate differs by bytes;
- a table value is supported only by an external reference.
