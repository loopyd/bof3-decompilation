# Project symbol naming cleanup

## Goal and evidence baseline

Replace address-canonical `func_XXXXXXXX` and `D_XXXXXXXX` names only where target-local evidence satisfies the cleanup two-corroborator gate. Preserve every unresolved address name rather than guessing.

Baseline (2026-08-08): clean worktree; 24 target maps contain 504 raw function names and 832 raw data names. The largest scopes are `exe/slus_004_22`, `emi/battle/battle/{15,03}`, `emi/etc/game/00`, and `emi/world00/area030/04`. This is an evidence campaign, not a mechanical rename.

## Phase 0 — Metadata-authoritative flexible naming framework ✅ implemented 2026-08-08

Before renaming symbols, finish and validate the harness framework so source identity never depends on an address-encoded filename:

1. Keep `tools/python/harness/domain/tags.py` as the sole parser authority for `@source`, `@behavior`, and declaration provenance.
2. Centralize target source discovery and compiled-symbol resolution in the domain registry. Require every lift source to carry parsable `@source` and `@behavior`; reject missing metadata and duplicate address claims deterministically.
3. Migrate all harness consumers, including matching, decomp status, analyzer/index, lift/m2c, rev-query missions/priorities, decomp.me, permute, and layout checks. A semantic source filename must work everywhere a legacy `func_XXXXXXXX.c` file worked.
4. Remove the `func_XXXXXXXX.c` filename fallback only after a repository-wide metadata migration proves every lift has valid tags and characterization tests cover every consumer.
5. Update `AGENTS.md`, `.pi/agents/`, `.pi/skills/bof3-re/`, and `docs/agents/` so metadata is the identity contract and address-based filenames are neither required nor preferred. Raw compiled symbols may remain `func_XXXXXXXX` until an evidence-gated symbol rename; source filename and compiled symbol identity are separate.

Validation: focused source-registry and consumer tests and repository metadata audit pass. The pre-existing duplicate/misplaced `dispatchWorkTable69a0` Splat ownership was reconciled to its map, source metadata, and exact bytes at `0x800AD69C`. Run `just check` before handoff. No target bytes, addresses, ABI, map ownership, or Splat boundaries may change during framework migration.

## Phase 1 — Target-qualified audit

Process targets independently in descending raw-symbol count. For each target:

1. Confirm manifest/load identity and fresh Rizin snapshot with `snapshot-status.py` and `bin/rz-project status`.
2. Have `bof3-cleanup` inventory raw names and rank only candidates with at least two corroborators: consistent local callers/accesses, reviewed Rizin annotation, or proven table/layout plus consumers.
3. Record unresolved names and the missing evidence. Never infer a semantic name from an address, decompiler label, duplicate hash, string, or one xref alone.

Artifacts under `out/` are disposable evidence; durable runtime findings go in `docs/specs/` only when established.

## Phase 2 — Serial naming transactions

Apply one target-local symbol transaction at a time:

- unchanged address, ABI, width, signedness, volatility, pointer depth, storage, layout, and code shape;
- update map, `internal.h`, `symbols.c`, direct same-target references, and Splat label when a proven function rename requires it;
- retain address-based lift filenames unless the function already has required `@behavior` and `@source` metadata and the complete rung-4 transaction is justified;
- add `/* @source 0xXXXXXXXX @kind ... */` to every semantic data declaration.

No SDK-map edits, cross-target address reuse, generated PsyQ binding edits, source moves, shared-header promotion, or behavior refactors.

## Phase 3 — Per-transaction verification and review

For each renamed function or data symbol:

- repository metadata preflight resolves the source by `@source` with no filename fallback
- `bin/symbols normalize TARGET --write`
- `bin/symbols check TARGET`
- `bin/splat TARGET`
- `bin/build TARGET`
- fresh `bin/asm-diff TARGET@0xADDRESS --detail normal` and `bin/byte-match TARGET@0xADDRESS` for every touched lift
- `git diff --check`
- independent `bof3-review`

Revert a transaction that changes bytes; never fix it forward.

## Phase 4 — Target checkpoint

After all evidence-gated candidates in one target:

1. refresh target analysis/index if reviewed annotations or maps changed;
2. report renamed symbols, unresolved raw names, evidence, checks, and residual risks;
3. commit only with explicit authorization, then continue to the next target.

## Acceptance criteria

- Every lift source has parsable `@source` and `@behavior`, with no duplicate address claim in a target and no filename-derived identity fallback.
- Every semantic rename has exact target-local address/layout and two independent corroborators.
- No guessed names and no target ownership leakage.
- All renamed lifts remain exact byte matches.
- Maps, bindings, declarations, references, and metadata agree.
- Remaining `func_*`/`D_*` names have explicit evidence gaps rather than cosmetic aliases.

## Blockers and non-goals

- “Name all” means audit every raw symbol; it does not authorize unsupported semantic guesses.
- Stale/missing snapshots, mixed code/data boundaries, absent callers, runtime-populated tables, or unknown ABI block individual renames, not the campaign.
- No lifting, matching experimentation, file relocation, compiler/toolchain changes, SDK-body lifting, or private/generated artifact commits.
