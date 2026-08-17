---
type: Runtime spec
title: Compiler provenance and delay-slot evidence
description: Evidence limits for matching BOF3 code against historical PSX compiler output.
tags: [runtime, compiler, matching, evidence]
---

# Compiler provenance and delay-slot evidence

The canonical matching chain is `gcc-2.7.2-psx`, `maspsx`, and ASPSX 2.56
emulation. It is a reproducible comparison toolchain, not proof that every
retail BOF3 object was built by precisely that compiler revision or scheduler
configuration. A source lift is accepted only when `bin/byte-match` is exact.

## `exe/slus_004_22@0x80162B08`

`func_80162B08` (lifted as `stageEmiTransferSlot`) was a documented compiler
residual that is now an exact byte match:

| Evidence | Result |
| --- | --- |
| Reviewed function range | `0x80162B08–0x80162C14` (half-open; last instruction at `0x80162C10`), 67 instructions, 268 bytes |
| Original at `0x80162BA4` | `j 0x80162C0C; nop` |
| Canonical GCC output | `j 0x80162C0C; li v0, 1` |
| Current live result | `bin/byte-match` MATCH, 67/67 instructions, 268 bytes, commit `01e5779d` |

### Historical residual analysis

The dated analysis below explains the delay-slot residual and how it was
resolved; it is historical evidence, not current state.

The zero-slot arm already loads `v0 = 1` to store `D_8014646C`. GCC's delayed
branch reorganization additionally moves the common successful-return value
into that arm's unconditional-jump delay slot. The original leaves that slot a
nop. The canonical raw GCC assembly contains the `li`; therefore neither
maspsx nor ASPSX object conversion is its cause.

The exact C shape that reproduces the original uses a branch-local `return 1`
plus an empty `do/while (0)` `MATCHING_AID` at
`src/bof3/io/stageEmiTransferSlot.c:44-57`. The empty loop and branch-local
return prevent GCC's cross-jumping from duplicating `li v0,1` into the zero
branch's `j epilogue` delay slot (original: nop); the zero branch reaches the
shared `jr ra` with `v0` already holding 1 from the `D_8014646C = 1` store.
This reproduces the original bytes; it does not prove the
compiler-provenance hypothesis (that a particular compiler revision or
scheduler produced the object).

### Negative evidence

The following clean-C or profile classes were tested in disposable workspaces
and restored because none produced `bin/byte-match` equality before the
branch-local-return resolution:

- declaration/volatile forms for loader globals;
- branch inversion, early-return, `goto`, local-result, and return-expression
  shapes;
- sanctioned `barrier()`/`CLOBBER_*` placement attempts;
- reviewed flag-catalog candidates plus scheduling, peephole, CSE, ABI, and
  MIPS-mode deltas;
- bounded permuter search (best score improved but did not yield credible C);
- maspsx ASPSX emulation versions;
- isolated old-GCC binaries: 2.6.0, 2.6.3, 2.7.0, 2.7.1, PSX 2.7.2, and
  2.7.2.1–3 all retain this same 66/67 fill. GCC 2.5.7 and 2.8+ diverge earlier;
- disposable stock PsyQ `CC1PSX.EXE` from 4.0, 4.1, 4.3, and 4.6. Their
  optimized output also fills the equivalent jump delay slot.

The rejected alternatives above are historical: none reproduced the original
bytes before the branch-local-return resolution, and none is a current
residual. Do not replace the exact lift with inline assembly, register
pinning, `INCLUDE_ASM`, Splat asm, or a per-object profile override. The
retained exact lift keeps the `MATCHING_AID` comment that names the
original/current instruction placement and the following exact live
byte-match; remove the aid only if the compiler's tail-merge/delay-slot
behavior for this shape is understood structurally. A future attempt that
seeks a pure clean-C shape without the aid requires independent evidence for a
BOF3-specific compiler patch or build option and must preserve the target's
clean-C byte-match gate.

## Verification

```sh
bin/asm-diff exe/slus_004_22@0x80162B08 --detail full
bin/byte-match exe/slus_004_22@0x80162B08
```

The expected current state is an exact match: 67/67 instructions, 268 bytes,
`stageEmiTransferSlot` (`@status exact`).
