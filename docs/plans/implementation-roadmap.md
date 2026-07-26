# Implementation Roadmap

**Status:** completed (SLUS local-image mismatch remains separately documented)

## Goal

Complete the remaining repository-tooling, disc/audio-contract, and target-owned
health work without treating generated state, proprietary inputs, or analyzer
hypotheses as reviewed source truth.

## Evidence baseline — 2026-07-26

- `just doctor` passes all 5 checks: toolchain, PsyQ 4.7, disc media, 23 target
  images, and 14 wrapper commands.
- The focused Python suite for `rev-query`, `analysis-sequence`, and wrapper
  bootstrap passes (16 tests); each converted wrapper and `bin/analysis-sequence`
  accepts `--help`.
- `cargo test --locked --manifest-path tools/rust/bof3-disk/Cargo.toml` passes;
  `bof3-disk rebuild` supports only its documented deterministic synthetic ISO9660 contract.
- `just --justfile tools/c/psx-audio/justfile test` and `lint` pass. The lint
  command emits existing compiler warnings, so it is not evidence that every
  audio claim is covered.
- `bin/symbols check` currently reports 78 source/map drifts — 12 in
  `emi/battle/battle/03`, 28 in `emi/etc/shop/00`, 34 in
  `emi/battle/battle/15`, and 4 in `exe/slus_004_22` — plus the EXE
  `D_80143C30` binding/map drift. These are target-owned work, not grounds to
  weaken validation.

## Ordered execution

### 1. Analysis workflow contract — completed

- Kept the canonical-code exclusion diagnostic in `rev_query.py` and its
  focused coverage; no exclusion table was added to `reverse.sqlite`.
- Added target-scoped `bin/analysis-sequence`: it checks snapshot freshness,
  rebuilds only after that check, then executes the requested ranking without
  changing reviewed maps or layouts.
- Added stale/fresh sequence fixtures and documented the public command in
  `docs/usage.md`.

**Evidence:** focused `rev-query` and `analysis-sequence` tests pass (16 tests
with the wrapper suite); `bin/analysis-sequence --help` passes.

**Deferred:** do not add a reviewed-boundary conflict command until a fresh live
snapshot captures a concrete analyzer-versus-reviewed-Splat mismatch.

### 2. Equivalent Python wrapper bootstrap — completed

- Added `bin/python-env` for root resolution, Python selection, `PYTHONPATH`,
  and the pre-existing missing-environment exit.
- Converted only `bin/emi-target`, `bin/flag-search`, `bin/index`,
  `bin/psyq-import`, and `bin/str-media`; the Rizin, harness, symbols,
  permuter, compiler, C, and Rust wrappers remain intentionally separate.
- Added forwarding/missing-Python contract coverage.

**Evidence:** the focused wrapper suite passes; every converted wrapper accepts
`--help`; `just doctor` reports all 5 checks passing.

### 3. Close the supported disc and audio contracts

#### 3a. Deterministic `bof3-disk rebuild` — completed

`bof3-disk rebuild -i DIR -o IMAGE.iso` now produces a deterministic cooked
ISO9660 image from non-empty, top-level, uppercase-ASCII regular files, sorted
by name. It writes no CUE; raw BIN/CUE, XA/CDDA, nested directories,
mkpsxiso-compatible XML, retail-disc parity, and claims based on local media
remain unsupported.

**Evidence:** `tests/rebuild.rs` fixes the two-file synthetic image at
`107da5ec42bab15f6e58bcc754223e690ba323dd87005e5ea04faadc6bb61779`, checks
stable bytes, parsed LBA/size metadata, and extraction. The CLI integration
also invokes `rebuild`. `cargo fmt --check`, `cargo test --locked --manifest-path
tools/rust/bof3-disk/Cargo.toml`, and warning-free `cargo clippy --all-targets
-- -D warnings` pass. `README.md` documents only this synthetic contract.

#### 3b. Extraction fixtures and audio claims — completed

- Added a synthetic ISO fixture test for an out-of-image extent and verified it
  leaves no target file behind; existing fixtures cover both-endian fields,
  nested output, raw/XA payload extraction, and CUE audio output.
- Made the native audio `test` task run CTest rather than only CLI usage. Added
  `xa_test` for synthetic XA decode plus WAV-header output; existing tests cover
  PSF load/CRC/overlay, bounded CPU/SPU transfer/DMA writes, and SPU voice,
  key-off, and pitch-cap behavior.
- Reconciled the public audio document with the actual CLI: direct SEP/VAB
  rendering is approximate; linked-runtime rendering is not exposed. The new
  evidence/limits table qualifies decode, render, export, optional codecs, PSF,
  and all unsupported SPU timing/mixing effects.

**Evidence:** Rust fixture tests, `just --justfile tools/c/psx-audio/justfile
test`, and `lint` pass locally. No fixture uses `inputs/` media.

### 4. Repair global health in bounded target-owned batches — completed

Do not bulk-generate map entries from filenames. For every candidate address,
verify target image range, reviewed Splat boundary, original bytes, and
canonical target-local ownership before changing a map.

**Batch A — `emi/battle/battle/03` completed:** verified the 12 planned source
boundaries and eight additional tracked no-op sources revealed by the status
audit, restored their target-local map/Splat ownership, and added required
source/behavior metadata. The duplicate `D_8014598C` local map entry was
removed because the shared map owns the primitive cursor. `bin/symbols check
emi/battle/battle/03`, `bin/splat emi/battle/battle/03`, and
`bin/decomp-status emi/battle/battle/03 --detail normal` pass
(`exact=31`, `partial=104`, `invalid=0`).

**Batch B — `emi/etc/shop/00` completed:** verified the 28 planned source
boundaries, restored their target-local map/Splat ownership, and repaired the
two scalar declaration errors exposed by compilation (`D_80148650`/
`D_80148651` and halfword `D_801490A4`). Added required source/behavior
metadata to 14 already-mapped lifts revealed by the audit and promoted their
two reviewed asm boundaries. `bin/symbols check emi/etc/shop/00`,
`bin/splat emi/etc/shop/00`, and `bin/decomp-status emi/etc/shop/00 --detail
normal` pass (`exact=42`, `partial=8`, `invalid=0`).

**Batch C — `emi/battle/battle/15` completed:** verified the 34 planned
source boundaries, restored their target-local map/Splat ownership, and added
required source/behavior metadata to the 60 tracked lifts exposed by the status
audit. Ten independently reviewed asm boundaries were promoted to C.
`bin/symbols check emi/battle/battle/15`, `bin/splat emi/battle/battle/15`, and
`bin/decomp-status emi/battle/battle/15 --detail normal` pass
(`exact=46`, `partial=43`, `invalid=0`).

**Batch D — `exe/slus_004_22` completed:** `func_8014B17C` was restored as
target-owned code (rather than the false shared data alias); `D_80143C30` was
added to the target map to satisfy its tracked weak binding. `func_8017212C`
and `func_801729D0` remain SDK-owned, so their authored lifts were removed and
their reviewed Splat boundaries restored to asm. `InitCARD` at `0x8017E028`
was likewise restored to its official PsyQ name and the local body removed.
The target-local `symbols/variables.c` is authored, not generated; only
`symbols/psyq.c` is generated. The required `func_8014AA04` provenance was
added. `bin/symbols psyq-bindings --write` regenerated all SDK binding files;
`bin/symbols check exe/slus_004_22` and
`bin/decomp-status exe/slus_004_22 --detail normal` pass (`exact=32`,
`partial=18`, `invalid=0`). Its Splat step remains blocked by a pre-existing
binary SHA-1 mismatch (`expected 5c709174…`, observed `0ac47750…`).

1. For each batch, update only its `config/targets/<target>/symbols.txt`,
   reviewed Splat layout, or exact source evidence as warranted. Any behavior
   lift change must use `/skill:bof3-re` and the target-qualified matching loop.
6. After each batch run `bin/symbols check TARGET` and
   `bin/decomp-status TARGET --detail normal`. Stop and revise the plan if a
   batch reveals invalid boundaries, invalid lifts, or a scope broader than its
   evidence supports.

**Acceptance:** each accepted batch has clean target-local map/status evidence
or separately named unrelated failures. Run `just check` only after the bounded
batches; report every remaining global gate independently.

### 5. Refresh and close the roadmap — completed

The final audit exposed 34 metadata-only invalid lifts in ten target-owned
batches, including `emi/etc/sisyou/00` (five), which was absent from the
initial report. Each retains its reviewed target-local map and Splat boundary;
its source now records the address and either the observed behavior or an
explicit `UNKNOWN` recovery marker for an intentionally empty body. Per-target
checks now report zero invalid lifts. `bin/symbols check` and the final
`just check` pass: Python (`104 passed`), maps, and the global lift audit
(`exact=287`, `partial=230`, `invalid=0`).

`bin/splat exe/slus_004_22` remains separately blocked by the extracted local
binary SHA-1 mismatch: tracked layout expects `5c709174b26f74629bf19e12360b918edac785c9`,
while `out/binaries/exe/slus_004_22.bin` currently hashes to
`0ac477508838b16d07a34610b72e0b6389c722d4`. Do not rewrite the reviewed hash
or layout without re-acquiring and verifying the executable image. It does not
block the repository gate, so no active roadmap work remains.

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
