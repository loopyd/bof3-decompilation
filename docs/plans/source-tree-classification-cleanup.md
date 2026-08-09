# BOF3 semantic source-tree migration

## Goal — complete

Migration completed: archival binary-container paths no longer hold authored source. Lifts converge into a shallow human-readable tree such as `src/bof3/io/`, `src/bof3/battle/`, `src/bof3/field/`, and `src/bof3/ui/`; stable declarations live under `include/bof3/` or an existing subsystem header.

Binary ownership remains target-qualified through manifests, function-level `@source`/`@behavior`, target maps, and reviewed Splat boundaries. A path must never imply executable/EMI ownership.

## Baseline and correction

The former plan incorrectly preserved `src/exe/...` and `src/emi/...` as target roots and moved twelve SLUS EMI-loader lifts one level deeper into `src/exe/slus_004_22/io/`. Those relocations are byte-exact but directionally wrong. Do not extend that pattern.

Current harness ownership is still based partly on manifest `source_dir` containment. Before moving authored lifts outside those roots, ownership and build selection must become explicit and target-qualified.

## Phase 1 — Explicit source claims in compatibility mode

1. Extend target manifests with explicit authored `sources`, generated/helper `support_sources`, and private `headers`. Keep `source_dir` only as an inventory fallback during migration.
2. Resolve a lift by `(target, @source address)` and exact manifest-claimed path. Reject duplicate path claims, duplicate `(target,address)` claims, missing metadata, and map/Splat/source drift. Equal addresses in different targets remain independent.
3. Migrate build, symbols, Splat, matching, status, PsyQ binding, Rizin, and agent-context consumers away from parent-directory ownership.
4. Build `TARGET` from its explicit claims, not a source-directory glob group.
5. Prevent Splat from creating/deleting authored sources. If upstream path generation requires legacy names, use an ignored `out/splat/<target>/source-view/` projection.
6. Characterize compatibility mode against all current target/source inventories before moving files.

Acceptance:

```text
PYTHONPATH=tools/python .venv/bin/python -m pytest -q tools/python/tests
bin/symbols check
bin/build exe/slus_004_22
bin/splat exe/slus_004_22
```

No production owner lookup may require source-path ancestry before Phase 2.

## Phase 2 — Correct the SLUS I/O pilot as one reviewed batch

Move the twelve mistakenly nested lifts directly from `src/exe/slus_004_22/io/` to `src/bof3/io/`:

- `initEmiLoader.c`
- `emiLoaderSlotLba.c`
- `beginEmiLoaderTransfer.c`
- `emiCdSyncCallback.c`
- `copyEmiType0Payload.c`
- `recordEmiDispatchHandler.c`
- `selectPrimaryEmiDestination.c`
- `selectAlternateEmiDestination.c`
- `copyEmiTransferChunk.c`
- `selectEmiLoaderMode6.c`
- `isEmiLoaderReady.c`
- `dispatchEmiModeCallback.c`

Update the SLUS manifest claims and corresponding Splat source paths atomically. Preserve compiled names, addresses, boundaries, function bodies, metadata, maps, and object flags. Adjust includes only as required by reviewed header ownership.

Create `include/bof3/io/emi_loader_internal.h` only when at least two pilot files share token-identical private declarations. Do not create per-target or per-function directory layers below `src/bof3/io/`.

Batch acceptance requires, for every selector, fresh `bin/asm-diff --detail normal` and `bin/byte-match`, plus one serialized target Splat/symbol/build pass, source-registry checks, diff hygiene, and independent review. One failure rolls back the whole batch.

## Phase 3 — Agent and cleanup batch contract

1. Cleanup accepts `relocate-batch TARGET CLASS SELECTOR...` for one target and one proven subsystem.
2. Preflight the complete batch for dirty overlap, path/name collisions, manifest claims, Splat boundaries, declarations, object flags, and live baseline evidence.
3. Apply path, manifest claim, Splat source, include, and path-keyed flag changes atomically.
4. Validate every selector independently; one failure rolls back the batch.
5. Reverse places new lifts in an already-proven `src/bof3/<subsystem>/`. Unclassified lifts use `src/bof3/unknown/` with explicit debt—never `src/exe/` or `src/emi/`.
6. Reviewer rejects new authored placement under `src/exe/` or `src/emi/`, inferred path ownership, unclaimed paths, and false cross-target header sharing.

Collision rule: prefer `src/bof3/<class>/<semantic-name>.c`; if occupied, append `_XXXXXXXX` using the function address; if still occupied across targets, append a stable short target hash. Suffixes disambiguate files only and never establish ownership.

## Phase 4 — Target/subsystem migration

Migrate remaining lifts in one-target/one-subsystem batches. Reuse established vocabulary before adding folders. Do not combine relocation with compiled-symbol renaming, body edits, boundary promotion, duplicate extraction, or type changes.

For each batch:

```text
bin/symbols check TARGET
bin/splat TARGET
bin/build TARGET
bin/asm-diff TARGET@0xADDRESS --detail normal   # every moved selector
bin/byte-match TARGET@0xADDRESS                 # every baseline-exact selector
git diff --check
git diff --cached --quiet
```

Baseline partials must retain their exact first mismatch, size, and percentage.

## Phase 5 — Headers and legacy-root removal — complete

1. Proven shared contracts live in subsystem headers or `include/bof3/`.
2. Target-private fixed-address facts remain separate even when addresses coincide.
3. Generated PsyQ bindings are explicit target-qualified support sources.
4. Every source, header, and support input is explicitly claimed. Historical `source_dir` values remain compatibility metadata only and do not establish ownership.
5. The `src/exe/` and `src/emi/` filesystem trees are removed.

Fresh lifts go under one semantic `src/bof3/<subsystem>/` root; manifest claims and function metadata establish target ownership. Final acceptance requires no authored C under `src/exe` or `src/emi`, no unclaimed or multiply claimed source path, no duplicate `(target,address)` claim, no source/map/Splat disagreement, and no match/status regression.

## Boundaries

- Never infer target ownership from directory, address, filename, or duplicate bytes.
- Never share a game-specific address merely because targets coincide.
- `src/shared/` remains compile-time implementation-template ownership only.
- Generated `out/`, build products, inputs, SDK bodies, load addresses, map addresses, and reviewed function boundaries are not migration targets.
- No stage, commit, push, reset, clean, or broad restore without explicit approval.
