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

`func_80162B08` is a documented compiler residual, not an asm fallback:

| Evidence | Result |
| --- | --- |
| Reviewed function range | `0x80162B08–0x80162C0C`, 67 instructions, 268 bytes |
| Original at `0x80162BA4` | `j 0x80162C0C; nop` |
| Canonical GCC output | `j 0x80162C0C; li v0, 1` |
| Current clean-C parity | 66/67 instructions, 268/268 bytes |

The zero-slot arm already loads `v0 = 1` to store `D_8014646C`. GCC's delayed
branch reorganization additionally moves the common successful-return value
into that arm's unconditional-jump delay slot. The original leaves that slot a
nop. The canonical raw GCC assembly contains the `li`; therefore neither
maspsx nor ASPSX object conversion is its cause.

### Negative evidence

The following clean-C or profile classes were tested in disposable workspaces
and restored because none produced `bin/byte-match` equality:

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

Do not use inline assembly, register pinning, `INCLUDE_ASM`, Splat asm, or a
per-object profile override to hide this residual. A future attempt requires
independent evidence for a BOF3-specific compiler patch or build option and
must preserve the target's clean-C byte-match gate.

## Verification

```sh
bin/asm-diff exe/slus_004_22@0x80162B08 --detail full
bin/byte-match exe/slus_004_22@0x80162B08
```

The expected current state is one difference at `+0xA0`: original `nop`,
current `li v0, 1`.
