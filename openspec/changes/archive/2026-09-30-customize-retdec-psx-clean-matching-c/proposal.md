# Proposal

## Why

The isolated RetDec evaluation produces several byte-exact BOF3 functions, but information loss still prevents clean source recovery and some historical matching gains are not semantically justified. Its repairs are split between plugin changes, dependency patches, and untracked source files, so the customized tool cannot yet be reproduced from one complete patch.

## What Changes

- Produce ordinary, readable C89 for PSX MIPS I and the repository's existing compiler profiles. Preserve machine-operation widths, signedness, memory effects, and source provenance through optimization and emission.
- Recover source forms from proven instruction and data-flow patterns, starting with the retained counter's increment/comparison and qualified scratchpad access. Do not use function-specific rewrites, register pins, artificial clobbers, assembly substitutes, or edits to generated C.
- Analyze the two compiler-generated division guards in case 053. Remove exceptional paths only when their unreachability is proven for the declared input contract. Reachable or unproven paths remain explicitly unsupported by ordinary-C execution.
- Preserve every validated exact match and semantic result under unchanged inputs, metadata, and compiler profiles. Compare per function, not aggregate scores. Admit matching improvements only after semantic checks, retaining a validated baseline when a proposed transformation loses evidence or match quality.
- Deliver one cumulative `psx-toolchain.patch` covering all required rz-retdec integration and RetDec source changes, including necessary new files. Accompany it with pinned revisions, application/build instructions, checksums, and an acceptance report. Rebuild and rerun from clean pinned sources using that patch alone.
- Reuse existing evaluation cases and runtime checks. Distinguish instruction similarity, complete byte identity, bounded runtime agreement, and unsupported behavior in the results.

## Capabilities

### New Capabilities

- `retdec-psx-clean-c`: Ordinary PSX C output, evidence-backed recovery, conservative exception handling, and per-function preservation of validated matches.
- `retdec-psx-patch-delivery`: A single complete patch that reproduces the customized plugin and its validated outputs from identified source revisions.

### Modified Capabilities

None. The existing OpenSpec capabilities do not define decompiler output or patch delivery. This change adds those observable contracts without changing production lift acceptance.

## Impact

The implementation affects rz-retdec metadata transfer and pass configuration, RetDec MIPS translation, source recovery, LLVM-to-BIR conversion, and the C writer. The durable deliverables belong under `tools/patches/rz-retdec/`. No new dependency, installed-tool change, production source promotion, or harness behavior change is proposed.

Current evidence is read-only input under `out/retdec-evaluation/`, especially `remainders-02/report.md`, `unaligned-matching-report.md`, and the retained corpus results. The existing `rz-retdec/` checkout and `build/_deps/retdec-src/` contain prior work that must be preserved, not edited in place. Future implementation uses separate source checkouts under `tmp/retdec-psx-source/`; only commands generate fresh build and evaluation artifacts under `out/retdec-psx/`. Installed `toolchains/`, generated PsyQ bindings, target manifests, Splat files, and game C remain unchanged.

`AGENTS.md`, `docs/agents/project-context.md`, `docs/agents/matching.md`, and `docs/agents/harness.md` own scope and acceptance. Existing original-profile native comparisons, pinned PCSX-Redux differential runs, retained synthetic/ABI/arithmetic checks, clean patch application/reversal, and a fresh CMake build provide implementation evidence. OpenSpec validation checks the plan, not decompiler correctness.

Full guest exception execution/resumption, new device emulation, new adapters or admission registries, compiler-profile changes, and a universal 100% matching claim are out of scope. This workflow creates planning artifacts only; implementation requires a later apply request.
