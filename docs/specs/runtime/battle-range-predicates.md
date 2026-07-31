---
type: Runtime evidence
title: Battle range-predicate leaves
description: Register-level evidence for two local battle/15 range predicates.
tags: [battle, matching, evidence]
---

# Battle range-predicate leaves

This note records only evidence from the reviewed `battle/15` function ranges.
Neither function has recovered callers, so parameter names describe observed
roles rather than a proven gameplay contract.

## `emi/battle/battle/15@0x800AF66C`

**Identity:** load `0x80096800`, payload offset `0x18E6C`, range `0x4C` bytes;
Rizin identifies two arguments, no stack frame, three basic blocks, and no
recovered callers/callees.

| Original use | Observed role |
| --- | --- |
| `a0+0x34`, `a0+0x38` | pointer to a two-axis range record |
| `a1`, copied to `t0`, then shifted right once | unsigned extent/value |
| scratchpad cell `0x1F800044`, fields `+0x34`, `+0x38` | second two-axis range record |

The returned value is zero unless `value` is at least the first unsigned
axis-difference; it then returns whether it is at least the second:

```text
half = value >> 1
first = work.axis_34 + half - range.axis_34
if value < first:
    return 0
second = work.axis_38 + half - range.axis_38
return value >= second
```

This is a reconstruction of the arithmetic and branch condition, not a claim
that the records are rectangles or collision bounds. The original deliberately
keeps `value` in `t0`, reuses `a0`/`a1` for the input/work fields, and returns
through `v0`; C parameter order must therefore remain `(range, value)`.

## Relationship to `emi/battle/battle/15@0x800AF720`

`0x800AF720` has the same scratchpad field pair and comparison topology, but
its first two argument words feed the subtractions directly and its third
argument (`a2`) is the value/extent. Its observed prototype shape is therefore
`(start, end, value)`, while `0x800AF66C` is `(range, value)`. The adjacent
`0x800AF6B8` uses the same work fields and an extent copied from `a3`, which
supports the local convention but does not establish a shared public type.

## Matching consequence

For `0x800AF66C`, a candidate that copies `a0` to preserve the range pointer is
not evidence for the intended ABI: original instructions load `a0`'s two fields
into `a1` then `a0` and reserve `t0` for `a1`. The unresolved mismatch is the
compiler's allocation/scheduling of this proven `(range, value)` shape, not an
unknown parameter order.

The retained clean-C partial lift first loads `g_battle_work`, then shifts
`a1` directly and keeps range/work values in `v0`/`v1`; it is 76→80 bytes and
first differs at entry, where the original is `move t0,a1; move v0,zero`.
A 52-profile matrix found canonical GCC 2.7.2 and GCC 2.6.3 tie at 25.00%,
while GCC 2.8.0, 2.8.1, and 2.95.2 are worse (23.81%), all non-exact. This
rules out those tested compiler profiles for this C shape; it does not prove
retail GCC version or justify an object override. Continue with clean-C
lifetime/expression ordering that makes `value` live before work materialization
and permits the original `a0`/`a1` reuse. Do not add a pin or a generic macro
on this evidence alone; use a fresh `bin/asm-diff` for each bounded source-shape
attempt.
