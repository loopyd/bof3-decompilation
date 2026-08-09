---
type: Research result
title: Historical GCC compiler variant research
description: Negative evidence for historical GCC variants as BOF3 compiler
  candidates, plus live pipeline verification results (Phase 6).
tags: [compiler, research, gcc, mips, negative-evidence, pipeline]
---

# Historical GCC compiler variant research

Research into historical GCC compilers that may have produced BOF3 objects.

## Status: four verified negative candidates

`gcc-2.6.3-psx`, `gcc-2.8.0-psx`, `gcc-2.8.1-psx`, and `gcc-2.95.2-psx` are
provenance-pinned, opt-in candidates in `config/compiler/variants.json`; none
is selected by any object and none produced an exact match in a bounded probe.
The framework
(`bin/compiler-variants`, `tools/python/harness/toolchain/gcc_variants.py`)
verifies archive digest, host, extraction containment, and executable identity
before the compiler may run.

## Research summary

### Tested compilers

| Compiler | Source | Status |
| --- | --- | --- |
| `gcc-2.7.2-psx` (decompals/old-gcc 0.13) | GitHub release | Canonical toolchain — verified |
| GCC 2.5.7 | old-gcc submodule | Diverges from earlier BOF3 objects |
| GCC 2.6.0 | old-gcc submodule | Diverges from earlier BOF3 objects |
| GCC 2.6.3 PSX (old-gcc 0.13) | GitHub release, SHA-256-pinned | Bounded pilot tested; no exact result |
| GCC 2.7.0 | old-gcc submodule | Retains same residuals as 2.7.2 |
| GCC 2.7.1 | old-gcc submodule | Retains same residuals as 2.7.2 |
| GCC 2.7.2.1–3 | old-gcc submodule | Retains same residuals as 2.7.2 |
| GCC 2.8.0 PSX (old-gcc 0.17) | SHA-256-pinned GitHub release probe | `exe/slus_004_22@0x80162B08`: all 52 flag profiles differ; best 86.96%, below canonical 98.51%; no override; later `battle/15@0x800AF66C`: 23.81%, no override |
| GCC 2.8.1 PSX (old-gcc 0.17) | SHA-256-pinned GitHub release probe | `battle/15@0x800AF66C`: all 52 flag profiles differ; best 23.81%; no override |
| GCC 2.95.2 PSX (old-gcc 0.17) | SHA-256-pinned GitHub release probe | `battle/15@0x800AF66C`: all 52 flag profiles differ; best 23.81%; no override |
| maspsx (any version) | third_party/maspsx | Verified canonical match |
| ASPSX 2.56 | bin/cc driver | Verified canonical match |
| Stock PsyQ CC1PSX.EXE (4.0, 4.1, 4.3, 4.6) | Retail | Tested — no unique value |

**GCC 2.7.0–2.7.2.3**: see `docs/specs/runtime/compiler-provenance.md`
(`exe/slus_004_22@0x80162B08`). No byte-match improvement.

**GCC 2.8.0 PSX (old-gcc 0.17)**: a fresh, provenance-pinned probe on
`exe/slus_004_22@0x80162B08` confirmed divergence: all 52 flag profiles
were non-exact, with best 86.96% (`-O2 -mno-split-addresses`) versus the
canonical compiler's 98.51%. Its release archive was
`gcc-2.8.0-psx.tar.gz` from old-gcc `0.17`, SHA-256
`1a3c956fe8aea5ebdb251749d95de2c84f023530584d7bd663744b5ec24050b7`,
identity `2.8.0`; its catalog record is retained only as negative provenance,
and no `BOF3_OBJCOMPILER_` override was retained.

### `battle/15@0x800AF66C` historical-version matrix

The user-authorized clean-C revival has reviewed boundary `0x18E6C..0x18EB8`
(76 original bytes) and source
`src/bof3/battle/func_800AF66C.c`. Its canonical residual is
5/20 instructions (25.00%), 76→80 bytes, first at `+0x0000` (`move t0,a1`
absent). Each row ran all 52 flag-catalog profiles through
`bin/flag-search`; no `BOF3_OBJCOMPILER_` or flag override was retained.

| GCC | Compiled profiles | Compile errors | Best profile | Best match | Exact |
| --- | ---: | ---: | --- | ---: | --- |
| canonical 2.7.2 PSX | 47 | 5 | `-O2 -G0` | 25.00% | no |
| 2.6.3 PSX (old-gcc 0.13) | 47 | 5 | `-O2 -G0` | 25.00% | no |
| 2.8.0 PSX (old-gcc 0.17) | 50 | 2 | `-O2 -mno-split-addresses -fno-schedule-insns -fno-delayed-branch` | 23.81% | no |
| 2.8.1 PSX (old-gcc 0.17) | 50 | 2 | `-O2 -mno-split-addresses -fno-schedule-insns -fno-delayed-branch` | 23.81% | no |
| 2.95.2 PSX (old-gcc 0.17) | 52 | 0 | `-O2 -mno-split-addresses -fno-schedule-insns -fno-delayed-branch` | 23.81% | no |

The first clean-C candidate is retained as a target-local partial lift for
future source-shape work, not as proof of retail compiler identity. The matrix
closes these versions for this target; do not repeat it without new source,
ABI, or compiler-provenance evidence.

**ASPSX**: Used with `bin/cc` driver, produces byte-identical output to
canonical GCC 2.7.2-psx toolchain for all tested functions.

**PsyQ CC1PSX.EXE**: Stock versions 4.0, 4.1, 4.3, 4.6 tested in disposable
workspaces. Optimized output fills equivalent jump delay slots — consistent
with GCC behavior, not evidence of a unique compiler.

### Flag catalog testing

The existing flag catalog (`config/compiler/flag-catalog.json`) contains 52
candidate flag combinations. Results are target-specific: reviewed exact lifts
use entries such as `-O1` and `-Wa,--expand-div`; the non-exact residual results
are recorded only with their named target/function evidence.

## Negative evidence

Bounded research has been performed and documented:

- Target-qualified flag-catalog searches across all 52 candidates for the
  documented non-exact residuals
- Bounded permuter search (best score improved but did not yield credible C)
- maspsx ASPSX emulation versions (2.56+)
- Isolated old-GCC binaries: 2.5.7, 2.6.0, 2.6.3, 2.7.0, 2.7.1, 2.7.2,
  2.7.2.1–3, 2.8.0, 2.8.1, and 2.95.2
- Disposable stock PsyQ `CC1PSX.EXE` 4.0, 4.1, 4.3, 4.6
- Declaration/volatile forms for loader globals
- Branch inversion, early-return, `goto`, local-result, return-expression shapes
- Sanctioned `barrier()`/`CLOBBER_*` placement attempts
- Reviewed flag-catalog candidates plus scheduling, peephole, CSE, ABI, MIPS-mode deltas

These negative results do not generalize to other functions; retained exact
flag profiles are documented beside their owning object configuration.

## Phase-4 pilot: `gcc-2.6.3-psx` / `battle/15@0x800AF66C`

The catalog entry records old-gcc release `0.13` (commit
`b9793e7e84f42d442e8d89a2c5c9e568e79e3bb7`), archive
`gcc-2.6.3-psx.tar.gz`, digest
`sha256:db98510a8cece2f9e37665cc16b4f1f7ad17f282f900d2791b62ed74f50e40b2`,
GPL-2.0-or-later licensing, `linux-x86_64`, flat `gcc` executable path, and
observed `gcc --version` output `2.6.3`. `bin/compiler-variants install`,
`verify`, and `path` passed locally.

The initial disposable clean-C pilot was a 76-byte entry-register residual;
its source was removed after that closeout. A later user-authorized revival is
retained as a target-local partial lift and has its separate all-version matrix
above. GCC 2.6.3 initially rejected the
pre-existing declaration spelling
`void __attribute__((noinline)) func_8009B20C(void);`. Its equivalent
post-declarator spelling was used only for the experiment then restored. Under
that temporary compatibility spelling, 47 of 52 flag profiles compiled and all
were different; the best was 19.05% for `-O1 -fno-delayed-branch`. Five
`-mno-split-addresses`/`-Os` profiles are unsupported by GCC 2.6.3 and failed
to compile. There were no exact matches, no `BOF3_OBJCOMPILER_` entry, and no
retained header/source/flag change. The canonical live control remains 76→76
bytes with first difference `+0x0000`: original `move t0,a1; move v0,zero`,
current `move a2,a1; srl a3,a2,1` (2/19 instructions).

This closes the first probe. Do not generalize its score or repeat the flag
matrix; a follow-up needs new source, ABI, or compiler provenance.

## Framework

The candidate framework is now in place:

- `config/compiler/variants.json` — schema: `harness.compiler-variants/v1`
- `bin/compiler-variants` — CLI list/install/verify/path
- `tools/python/harness/toolchain/gcc_variants.py` — `CompilerVariant` / `EmptyCatalog`
- `tools/python/harness/compiler_config.py` — per-object flag/compiler parsing for CMake parity
- `tools/python/harness/commands/compiler_variants.py` — CLI commands

The `variants.json` catalog is reviewed, tracked metadata — the single source
of truth for compiler IDs, archive digests, and executable paths. GCC archives
are cached, SHA-256-verified, under
`inputs/external/private-assets/toolchains/gcc/`; installed variants live in
ignored local state under `toolchains/gcc-variants/` and the canonical
compiler under `toolchains/gcc-2.7.2-psx/` (unrelated PSn00b/Rizin downloads
stay in `toolchains/downloads/`). Canonical GCC and every catalog variant share
one archive lifecycle: cache-symlink/non-regular rejection, cache-local
temporary download, digest validation before atomic cache publication, fresh
sibling staging extraction, staged `gcc --version` identity verification, and
an atomic install swap that preserves a prior verified install on any failed
network, digest, extraction, or identity check. `bin/compiler-variants path
<id>` and `compile_commands.json` resolve a selected compiler through the same
ensure-installed operation; a missing install self-heals from the verified
cache, while an unsupported host, unknown ID, corrupt install, or failed
install fails closed (never canonical/host GCC). `just setup` primes the
canonical compiler plus every host-compatible entry in the
`config/compiler/variants.json` catalog; host-incompatible candidates are
skipped with their ID and host reported, an invalid catalog fails setup
closed, and a catalog with no candidates installs nothing. Setup never sets
`PSX_GCC`, adds an object override, or changes the default compilation
selection. Doctor remains verification-only. No
compiler ID is accepted by the build unless it appears in the catalog.

## Verification

```sh
# Check catalog state
bin/compiler-variants list
bin/compiler-variants verify <id>

# Verify baseline build unchanged
just check
bin/symbols check
```

Expected: `list` reports both candidates, `verify <id>` validates an ignored
local installation, and the default build remains canonical because no object
selects a compiler variant.

## Live pipeline control results (Phase 6)

Tool versions as of 2026-07-30:

| Tool | Version |
|------|---------|
| GCC | 2.7.2 (decompals/old-gcc 0.13) |
| maspsx | third_party/maspsx (HEAD) |
| ASPSX emulation | 2.56 (bin/cc default) |
| GNU assembler | 2.40 (PSn00b) |
| GNU ld | 2.40 (PSn00b) |
| GNU objcopy | 2.40 (PSn00b) |

### Reviewed nondefault optimization control

| Target | asm-diff | byte-match | Flags |
|--------|----------|------------|-------|
| `emi/etc/game/01@0x801D0D5C` | MATCH (100%) | MATCH | `-O1` (BOF3_OBJFLAGS) |

### `--expand-div` override flag path (`-Wa,--expand-div`)

Both `--expand-div` targets pass the full GCC→maspsx→ASPSX→GNU as→GNU ld
pipeline exactly:

| Target | asm-diff | byte-match | Flags |
|--------|----------|------------|-------|
| `emi/battle/battle/15@0x800AB760` | MATCH 53/53 (100%) | MATCH | `-O2 -Wa,--expand-div` |
| `emi/battle/battle/03@0x801E29B4` | MATCH | MATCH | `-O2 -Wa,--expand-div` |

Pipeline verified: all three targets compile through `bin/cc`, translate
through maspsx with `--aspsx-version=2.56`, assemble with GNU as 2.40, link at
function address with GNU ld 2.40, extract bytes with objcopy, and compare
against original overlay bytes — all exact MATCH.

### Focused test coverage (new in Phase 6)

| Test file | Tests | Status |
|-----------|-------|--------|
| `test_bin_cc_pipeline.py` | 3 hermetic stub tests for GCC→maspsx→assembler arg flow | PASS |
| `test_asm_link.py` | 4 fixture-local tests for relocation-aware linking + byte extraction | PASS |
