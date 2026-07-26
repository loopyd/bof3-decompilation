# Implementation Roadmap

**Status:** active

## Goal

Complete the remaining repository-tooling, disc/audio-contract, and target-owned
health work without treating generated state, proprietary inputs, or analyzer
hypotheses as reviewed source truth.

## Evidence baseline

Reviewed locally:

- `just doctor` passes all 5 checks: toolchain, PsyQ, disc media, 23 target
  images, and 14 wrapper commands.
- Every current Rizin snapshot is fresh.
- `bin/rev-query <ranking> --exclusions` now exposes canonical-code rejection
  reasons; its focused test covers SDK, pointer-table, printable/data, and code
  rows while ordinary rankings remain exclusion-filtered.
- `bin/symbols check` reports 78 source/map drifts and one EXE binding/map drift.
  They are target-owned work, not grounds to weaken validation.
- `bof3-disk rebuild` remains unsupported. The supported future scope is a
  deterministic synthetic-fixture ISO/CUE contract, not retail-disc or
  mkpsxiso parity.

## Ordered execution

### 1. Finish the analysis workflow contract

1. Keep the completed candidate-exclusion diagnostic in
   `tools/python/harness/commands/rev_query.py`, its focused test, and
   `docs/usage.md`; do not add an exclusion table to `reverse.sqlite`.
2. Add one target-scoped analysis-sequence command in the owning Python command
   surface (or a narrow new command) that:
   - checks `bin/rz-project status TARGET` first;
   - fails with the target and the `snapshot` stage when stale;
   - rebuilds the index only after freshness succeeds;
   - runs an explicitly requested `rev-query` ranking without refreshing other
     targets or changing reviewed maps/layouts.
3. Add a fixture integration test proving stale snapshots stop before indexing
   and a fresh target completes the sequence. Document the command in
   `docs/usage.md`.
4. Do **not** add a generic reviewed-boundary conflict command yet. Add one only
   after a fresh live snapshot produces a captured `TARGET@0xADDRESS` analyzer
   versus reviewed-Splat mismatch. That later change requires one reproducing
   fixture and must attach to the existing command that owns the evidence.

**Likely files:** `tools/python/harness/commands/`, `bin/` only if a new thin
wrapper is necessary, `tools/python/tests/`, `docs/usage.md`.

**Acceptance:** focused tests pass; stale output is target/stage-qualified;
fresh invocation performs only the documented sequence; no new report exists
without a real mismatch.

### 2. Centralize the equivalent Python wrapper bootstrap

1. Add one tracked `bin/python-env` helper. It owns only repository-root
   resolution, `PSX_PYTHON`/`.venv` selection, `PYTHONPATH`, and existing
   missing-environment exits.
2. Convert only equivalent shell wrappers first:
   `bin/emi-target`, `bin/flag-search`, `bin/index`, `bin/psyq-import`, and
   `bin/str-media`.
3. Preserve command-specific messages when converting the setup-dependent and
   venv-only wrapper groups. Do not convert these without their own regression
   proof:
   - `bin/rz-project` (pinned Rizin `PATH` contract);
   - `bin/harness` (compatibility argument validation);
   - `bin/symbols` and `bin/permute` (Python re-exec entry points);
   - compiler adapters plus C/Rust launchers.
4. Add a small wrapper contract test for module/argument forwarding and
   missing-Python exit behavior. Keep `setup` and `doctor` reporting each
   failing tool independently; do not add or edit anything under `toolchains/`.

**Acceptance:** every converted wrapper retains its public arguments, output,
and exit code; each passes `--help` or `--example`; focused wrapper/doctor tests
and `harness.commands.doctor --root .` pass.

### 3. Close the supported disc and audio contracts

#### 3a. Deterministic `bof3-disk rebuild`

Implement only the approved synthetic contract:

1. Define the supported input set, output form (ISO and/or BIN/CUE), deterministic
   ordering, metadata, and CUE policy in Rust tests before implementation.
2. Add checked-in synthetic inputs and expected byte hashes plus parsed metadata
   assertions.
3. Implement exactly that contract in `tools/rust/bof3-disk/src/` and expose it
   through the existing CLI.
4. Keep these explicit non-goals: retail-disc parity, XA/CDDA whole-disc parity,
   mkpsxiso XML compatibility, and claims based solely on authorized local media.

**Acceptance:** `cargo test --locked --manifest-path tools/rust/bof3-disk/Cargo.toml`
passes with exact synthetic ISO/CUE bytes and metadata. The README describes
only this supported contract.

#### 3b. Extraction fixtures and audio claims

1. Audit existing Rust extraction fixtures for malformed extents/records,
   inventory boundaries, and target-safe output paths. Add only demonstrably
   missing synthetic coverage; never add `inputs/` media.
2. Inventory each decode, render, and export claim in
   `docs/specs/formats/audio.md` against a named native test, binary evidence,
   or an explicit limitation.
3. Upgrade the native audio test path beyond `--help`/`--examples` only where a
   documented claim lacks coverage. Qualify or remove unsupported claims; retain
   documented limits for reverb, noise, modulation, CD/XA mixing, DMA/IRQ timing,
   and incomplete PSF execution.

**Acceptance:** Rust fixture tests, audio native `test` and `lint` pass where
the local environment supports them; every retained user-visible audio claim
has named evidence or a clear limitation.

### 4. Repair global health in bounded target-owned batches

Do not bulk-generate map entries from filenames. For every candidate address,
verify target image range, reviewed Splat boundary, original bytes, and
canonical target-local ownership before changing a map.

1. **Batch A — `emi/battle/battle/03` (12):**
   `801D67EC`, `801DD26C`, `801DD7AC`, `801DD7D8`, `801DDED8`, `801DDF00`,
   `801E1C58`, `801E3BAC`, `801E6990`, `801E6FA0`, `801E7528`, `801E925C`.
2. **Batch B — `emi/etc/shop/00` (28):** repair only after Batch A validates.
3. **Batch C — `emi/battle/battle/15` (34):** repair only after Batch B validates.
4. **Batch D — `exe/slus_004_22` (4):** `8014B17C`, `8017212C`, `801729D0`,
   `8017E028`; separately establish whether
   `src/exe/slus_004_22/symbols/variables.c` is generated before addressing
   `D_80143C30`. Regenerate it with the owning `bin/symbols` command if so;
   never hand-edit a generated binding.
5. For each batch, update only its `config/targets/<target>/symbols.txt`,
   reviewed Splat layout, or exact source evidence as warranted. Any behavior
   lift change must use `/skill:bof3-re` and the target-qualified matching loop.
6. After each batch run `bin/symbols check TARGET` and
   `bin/decomp-status TARGET --detail normal`. Stop and revise the plan if a
   batch reveals invalid boundaries, invalid lifts, or a scope broader than its
   evidence supports.

**Acceptance:** each accepted batch has clean target-local map/status evidence
or separately named unrelated failures. Run `just check` only after the bounded
batches; report every remaining global gate independently.

### 5. Refresh and close the roadmap

After each accepted ordered section, remove its completed bullet from this plan
and replace it with the validation evidence. Do not retain historical checklists
or claim completion from a successful command alone.

## Validation order

1. Focused Python tests for changed command/wrapper code, then wrapper
   `--help`/`--example` checks and `harness.commands.doctor --root .`.
2. `cargo test --locked --manifest-path tools/rust/bof3-disk/Cargo.toml`.
3. `just --justfile tools/c/psx-audio/justfile test` and `lint`.
4. Per-target `bin/symbols check TARGET` and
   `bin/decomp-status TARGET --detail normal` after each health batch.
5. `just check` and `git diff --check` before handoff when practical.

## Boundaries and non-goals

- Do not hand-edit `build/` or `toolchains/`; do not commit `inputs/` or `out/`.
- Do not make analyzer output, a source filename, or matching percentage alone
  into ownership evidence.
- Do not reopen companion-overlay work without new evidence.
- Do not claim whole-disc rebuild parity or unsupported audio fidelity.
- Do not stage, commit, or push without explicit user approval.
