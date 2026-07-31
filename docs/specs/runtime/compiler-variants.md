---
type: Research result
title: Historical GCC compiler variant research
description: Negative evidence for historical GCC variants as BOF3 compiler candidates.
tags: [compiler, research, gcc, mips, negative-evidence]
---

# Historical GCC compiler variant research

Research into historical GCC compilers that may have produced BOF3 objects.

## Status: Empty catalog

No historical GCC variant has been validated as a BOF3 compiler candidate.
The framework (`config/compiler/variants.json`, `bin/compiler-variants`,
`tools/python/harness/toolchain/gcc_variants.py`) is in place so that when
a candidate appears, the infrastructure exists to validate it safely.

## Research summary

### Tested compilers

| Compiler | Source | Status |
| --- | --- | --- |
| `gcc-2.7.2-psx` (decompals/old-gcc 0.13) | GitHub release | Canonical toolchain — verified |
| GCC 2.5.7 | old-gcc submodule | Diverges earlier than BOF3 objects |
| GCC 2.6.0 | old-gcc submodule | Diverges earlier than BOF3 objects |
| GCC 2.6.3 | old-gcc submodule | Diverges earlier than BOF3 objects |
| GCC 2.7.0 | old-gcc submodule | Retains same residuals as 2.7.2 |
| GCC 2.7.1 | old-gcc submodule | Retains same residuals as 2.7.2 |
| GCC 2.7.2.1–3 | old-gcc submodule | Retains same residuals as 2.7.2 |
| GCC 2.8+ | old-gcc submodule | Diverges earlier than BOF3 objects |
| maspsx (any version) | third_party/maspsx | Verified canonical match |
| ASPSX 2.56 | bin/cc driver | Verified canonical match |
| Stock PsyQ CC1PSX.EXE (4.0, 4.1, 4.3, 4.6) | Disposable test | Fills jump delay slots |

### Key findings

1. **GCC 2.7.x family** (2.6.0, 2.6.3, 2.7.0–2.7.2.1–3): All retain the same
   known residuals documented in `docs/specs/runtime/compiler-provenance.md`
   (notably `exe/slus_004_22@0x80162B08`). No member of this family produces
   exact byte-match for tested functions.

2. **GCC 2.5.7 and 2.8+**: Diverge earlier in the instruction stream, making
   them unsuitable matches for any BOF3 function.

3. **maspsx / ASPSX**: When used with `bin/cc` driver, produce byte-identical
   output to the canonical GCC 2.7.2-psx toolchain for tested functions.

4. **PsyQ CC1PSX.EXE**: Stock versions from 4.0, 4.1, 4.3, and 4.6 were tested
   in disposable workspaces. Their optimized output fills the equivalent jump
   delay slots — consistent with GCC behavior, not evidence of a unique compiler.

### Flag catalog testing

The existing flag catalog (`config/compiler/flag-catalog.json`) contains 55
candidate flag combinations. All were tested via `bin/flag-search` against
tested functions. No combination produced exact byte-match.

## Negative evidence

The following bounded research was performed and documented:

- Per-function flag-catalog search across all 55 candidates
- Bounded permuter search (best score improved but did not yield credible C)
- maspsx ASPSX emulation versions (2.56+)
- Isolated old-GCC binaries: 2.5.7, 2.6.0, 2.6.3, 2.7.0, 2.7.1, 2.7.2,
  2.7.2.1–3, 2.8+
- Disposable stock PsyQ `CC1PSX.EXE` from 4.0, 4.1, 4.3, and 4.6
- Declaration/volatile forms for loader globals
- Branch inversion, early-return, `goto`, local-result, return-expression shapes
- Sanctioned `barrier()`/`CLOBBER_*` placement attempts
- Reviewed flag-catalog candidates plus scheduling, peephole, CSE, ABI, and
  MIPS-mode deltas

None produced exact `bin/byte-match` equality for tested functions.

## Framework

The empty-catalog framework is now in place:

- `config/compiler/variants.json` — schema: `harness.compiler-variants/v1`
- `bin/compiler-variants` — CLI for list/resolve/verify/env/sha256
- `tools/python/harness/toolchain/gcc_variants.py` — schema validator,
  `CompilerVariant` abstract class, `EmptyCatalog` sentinel
- `tools/python/harness/compiler_config.py` — variant resolution,
  environment management
- `tools/python/harness/commands/compiler_variants.py` — command handlers

When a candidate appears, it should be added to `variants.json` with verified
SHA-256 checksum. The doctor command will verify installed state.

## Verification

```sh
# Check catalog state
bin/compiler-variants list
bin/compiler-variants resolve
bin/compiler-variants sha256

# Verify baseline build unchanged
just check
bin/symbols --check
```

Expected: catalog reports empty, resolve returns `none`, build produces identical
output to pre-change state.
