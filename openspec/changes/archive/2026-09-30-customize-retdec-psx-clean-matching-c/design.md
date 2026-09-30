# Design

## Context

See `proposal.md` for scope and motivation. Read-only inspection found two source roots with independent histories:

| Component | Inspected base revision | Existing location under `out/retdec-evaluation/` |
| --- | --- | --- |
| rz-retdec | `4ac6b293553d7f5f00574e4dca4c21b799db63e1` | `rz-retdec/` |
| RetDec | `8272d0355794f8b8a63d8611b1dea50b4f2d87c3` | `build/_deps/retdec-src/` |

The plugin transfers metadata in `src/rz-plugin/data.cpp` and `utils.cpp`. Its `decompiler-config.h` repeats LLVM optimization passes before RetDec source recovery. RetDec's `src/llvmir2hll/llvm/source_recovery.cpp` contains the retained guard, membership-return, and table-initializer recovery. MIPS translation, delayed-load handling, the LLVM-to-BIR converter, and `c_hll_writer.cpp` determine whether source recovery still has the necessary machine semantics.

The current cumulative patch covers 19 tracked RetDec files. The plugin has a separate two-file patch. Six untracked RetDec files include source-recovery declarations/implementation, exception declarations, and the explicit-state frontend. A tracked diff alone is therefore not a complete deliverable. Inventory dependencies before deciding which prior changes belong in the ordinary-C package.

The latest retained report is `out/retdec-evaluation/remainders-02/report.md`, not the older open-items report. It records 125 selected cases, 118 admitted references, 117 native comparisons, and ten byte-exact results. The separate metadata-assisted six-function sample retains five byte-exact results. The counter remains 15/24 instructions. Cases 038, 044, and 055 already have unaligned translation and link successfully, but lack full function-specific runtime validation. Case 053 remains the admitted native-link blocker. These are historical observations to reproduce, not fresh acceptance evidence.

The repository has extensive pre-existing dirty work. This change must not stage, revert, or publish it.

## Goals / Non-Goals

The acceptance unit is one target-qualified function with frozen original bytes, full extent, metadata, compiler profile, and evidence level. Source recovery must operate on reusable patterns rather than named corpus cases.

This design does not change production harness behavior or production lift acceptance. It does not promise an exact counter before its semantics and scheduling are understood. A remaining counter mismatch or exception blocker must have a specific disposition, not be hidden in a successful package build. No new tests or dependencies are planned; use existing checks and retained probes.

## Decisions

### 1. Preserve evidence before changing the pipeline

Freeze the latest baseline and recover historical variants from retained artifacts. Record source and plugin hashes, source revisions, target identities, original bytes' hashes, compiler commands, metadata, and per-case outcomes. Separate signature-only, metadata-assisted, raw-register, and native-runtime evidence.

Protect all validated exact matches. Protect validated non-exact instruction scores under identical inputs. Preserve the latest retained outcomes as the operational baseline, while classifying older score-only gains separately. Known-wrong unsigned-division output cannot become a candidate merely because it scores better. Missing historical semantic evidence stays an explicit unresolved comparison; it is not silently declared invalid or validated.

Reject aggregate score thresholds and a universal best-ever-score promise. The retained shift experiments show why those are unsafe: some apparent gains depend on incorrect unsigned emission.

### 2. Keep machine semantics separate from source hints

Preserve width, signedness, operation origin, and relevant memory effects before destructive canonicalization. Carry only facts needed by concrete recoveries, using existing IR/configuration mechanisms where possible. Metadata that cannot survive a transformation must be invalidated, not guessed from the final expression.

Maintain full-width machine entry semantics even when supplied C metadata describes narrower values. Represent volatile qualification on the actual memory access through BIR and C emission, rather than only marking the load's destination external. Do not infer volatility for every scratchpad access or promise object bounds from an address alone.

The user approved an opt-in plugin-owned declaration input after the diagnostic
proved Rizin drops `volatile`. Reuse the existing configuration object/type format
through `DEC_OBJECT_DECLARATIONS`, limited to explicit global declarations. Validate
type, address, identity, and supported qualification before merging; reject conflicting
or unsupported declarations. Include the supplied metadata in cache identity.
Without this input, preserve the frozen invocation behavior. Keep diagnostic results
separate from both frozen corpora.

Carry object qualification through type rewrites and mark the corresponding LLVM
memory instructions volatile before destructive optimization. Apply it only to proven
accesses to the declared object, never to pointee accesses merely because a pointer
cell is volatile. Preserve the supplied base type and emit declaration-only `extern`
objects through BIR and C. Do not modify installed Rizin, infer qualification from
addresses, or add a separate access registry.

Implement the counter as the first end-to-end recovery. Track the increment, truncation, stored byte, and comparison use so that testing the incremented byte against 16 preserves wraparound and evaluation order. Recover the typed `g_battle_work` access only with its supplied declaration evidence.

Reject blanket optimizer disabling, reverse-pattern guesses on an arbitrary equality, and extra casts as a substitute for preserving the underlying operation. Keep width-sensitive shifts and unsigned division correct regardless of source similarity.

### 3. Prove division guards or retain the block

Trace both case-053 guard predicates from the original instructions and pre-optimization IR. Derive divisor ranges and signed-overflow conditions from reaching definitions and declared input facts, including memory aliases and intervening writes. A sampled runtime run or a compiler's undefined-behavior assumption is not a proof.

Apply guard elimination only where the proof covers all admitted inputs. Record failed proof obligations as well as successful ones. If either trap can occur or remains unknown, preserve explicit rejection of ordinary-C execution. The existing explicit-state frontend is separate and cannot supply missing state through an ordinary returning callback.

Before simplifying guards across a call, inspect the helper's original MIPS I instructions through the current file image. Use a bounded, nonrecursive leaf analysis: visit at most 256 instruction addresses within 1 KiB of entry, follow both conditional successors and direct jumps, include delay-slot effects, and require every path to return through the unchanged return-address register. Reject cycles, missing words, nested/indirect calls, unsupported instructions, memory writes, and control transfers in delay slots. Record a conservative GPR write mask from every reachable instruction, including delayed loads. Never infer preservation from a synthetic selected-only body or from an assumed calling convention.

Keep the summary local to the current module and original input bytes. Guard analysis may consume only proven preservation of its specific live registers; an unknown summary leaves the guard unchanged. This analysis is not a caller registry, a new runtime, or permission to decode arbitrary code without bounds. Existing retained cases and disposable inspection of their original bytes validate the implementation; no new regression tests are authorized.

Reject no-op handlers, linker-only stubs, blanket BREAK removal, and implicit caller preconditions. Full guest exception resumption remains a separate capability.

### 4. Use bounded source alternatives only after semantic checks

Start with one provenance-preserving path and the retained baseline. If a general recovery offers multiple legitimate source forms, bound the choice to the baseline and at most two pattern-derived alternatives per function. Each alternative must satisfy the same semantic and clean-C obligations before original-profile matching can select it. Retain the baseline on ties or insufficient evidence.

Keep the compiler-guided comparison in the isolated evaluation workflow. Use its results to accept or reject general plugin recovery rules; do not add compiler invocation, a case-name lookup table, or an output registry to the installed plugin. If one deterministic rule set cannot preserve the protected corpus, delivery remains blocked rather than handing out manually chosen generated files.

Reject unbounded permutation search, textual repairs of generated C, and function-specific special cases. The final clean build must emit the accepted output without a postprocessor.

### 5. Deliver one patch across a documented nested layout

Use the plugin's existing local dependency path, `deps/retdec/retdec/`, for the pinned RetDec source. `deps/retdec/CMakeLists.txt` already selects this local tree instead of fetching it. The cumulative patch is relative to the rz-retdec root: plugin paths remain `src/rz-plugin/...`, and RetDec paths are prefixed with `deps/retdec/retdec/`.

Deliver these files under `tools/patches/rz-retdec/`:

- `psx-toolchain.patch`, the only customization patch.
- `README.md`, with exact source layout, prerequisite identities, application, build, validation, and reversal commands.
- `manifest.json`, with base revisions, required file identities, patch checksum, and accepted build/input identities.
- `acceptance.md`, with per-corpus results, repaired cases, exclusions, historical score dispositions, and limitations without private game bytes.

The manifest records evidence and packaging inputs; it is not an admission registry. Include all required new source files and build declarations. Keep earlier validated ordinary-C fixes. Include previously applied build-compatibility edits needed by the local dependency path, which bypasses the fetch-time patch command. Audit stateful code dependencies explicitly: retain required shared code, but do not advertise or extend resumption. Unrelated dependency repairs require separate approval.

Reject a stack of incremental patches, a binary-only delivery, and replay from a dirty build cache. A clean source assembly plus this single patch must build and reproduce the accepted generated outputs. No installation or dependency download is authorized by this design.

## Risks / Trade-offs

- LLVM transforms can destroy provenance. Check retained intermediate IR at the relevant pass boundaries and invalidate unsupported facts.
- Correct volatile accesses can change code generation. Preserve the evidenced qualification and reject source transformations that lose protected matches.
- Semantic checks cover bounded inputs, not every PSX program. Report the domain and retain unsupported delay-slot, MMIO, and alias cases.
- Two historical shift-score regressions remain unresolved. Do not sacrifice the unsigned-division correction to restore them. Their evidence classification must be explicit before any non-regression claim.
- Existing evaluation scripts may contain fixed paths. Invoke existing configurable entry points or parameterize disposable command invocations; do not redesign the production harness. If a required script cannot run without a code change, report that narrow blocker before expanding scope.
- A single patch spans nested source histories. Verify both base revisions and file hashes, forward application, complete additions, clean build, reverse application, and baseline restoration.
- Generated evidence is disposable. Preserve durable reproduction instructions and non-private conclusions in the delivery, not only links to `out/`.

## Migration plan

1. Preserve the existing evaluation tree and record its dirty state. Assemble source copies under a fresh `tmp/retdec-psx-source/` directory without altering installed tools or the original checkouts.
2. Reconstruct the ordinary-C baseline from reviewed existing changes, including required untracked files. Use command-generated build/evaluation outputs under a fresh `out/retdec-psx/` directory. Do not hand-edit generated output or `build/` trees.
3. Apply and validate bounded semantic/provenance and source-recovery changes in the separate source copies. Keep recoverable snapshots per change and retain rejected results separately.
4. Assemble the single durable patch and replay it on another clean nested source layout. Build with existing prerequisites and Cutter disabled. Rerun generation, compilation, native comparisons, and the existing runtime checks from that rebuilt plugin.
5. Demonstrate reverse application on a disposable copy and unchanged production pins. A failed check leaves the package unaccepted and the prior validated tool/output available.

Existing evaluation drivers and original-profile comparison commands remain the evidence mechanisms. The retained 15 synthetic fixtures, six-function Redux runs, raw-register checks, unaligned probes, shift checks, and division checks must be replayed where affected. Missing function-specific runtime coverage remains a limitation, not authorization to add tests. Run `git diff --check` on authored changes and applicable existing repository gates if their owning files change. No advisory manifest, OpenSpec status, or successful build replaces these checks.
