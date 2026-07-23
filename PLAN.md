# Implementation Plan

## Problem

The repository has a functioning target-qualified BOF3 reverse-engineering
harness, but its durable source inventory and planning workflow are not yet
coherent enough to make repository-wide progress predictable. The current live
audit records 498 authored lifts: 170 exact, 192 partial, and 136 invalid.
132 invalid lifts are missing required source metadata, while `bin/symbols
check` reports 81 source/map drifts across seven target areas. Candidate ranking
also recently surfaced analyzer/data/SDK false positives, showing that planning
must pair function-selection metrics with canonical-boundary evidence.

## Goal

Provide a maintained plan workflow and execute the remediation sequence that
makes the repository auditable: every authored lift has valid metadata and map
ownership, ranked work is limited to validated canonical code, exact lifts stay
protected, and function-lifting can scale by target without obscuring toolchain,
asset, audio, or documentation ownership.

## Phase 1 — Establish planning and audit contracts

| File | Phase | Action |
| --- | --- | --- |
| `PLAN.md` | 1 | Add the repository-wide implementation plan. |
| `AGENTS.md` | 1 | Reference this plan for planning and plan-management requests. |
| `tools/python/harness/decomp_status.py` | 1 | Expose invalid-lift reasons in a stable target summary projection. |
| `tools/python/tests/test_decomp_status.py` | 1 | Cover the summary projection and invalid-reason aggregation. |

| ID | Task |
| --- | --- |
| 1.1 | Add `PLAN.md` as the single repository implementation plan with current audit baselines and phase gates. |
| 1.2 | Add a concise `AGENTS.md` planning reference that directs plan requests to `PLAN.md`. |
| 1.3 | Add a machine-readable invalid-lift summary to the decomp-status report. |
| 1.4 | Add regression coverage for the decomp-status invalid summary. |

**Design decisions**

- Keep plans at the repository root because this plan coordinates source,
  configuration, tooling, and documentation rather than one evidence spec.
- Treat `bin/decomp-status --json` and `bin/symbols check` as the numerical
  baseline; never infer progress from source-file counts alone.

## Phase 2 — Restore source, map, and Splat integrity

| File | Phase | Action |
| --- | --- | --- |
| `config/targets/emi/battle/battle/03/symbols.txt` | 2 | Reconcile map ownership for the 12 drifted battle/03 lifts. |
| `config/targets/emi/battle/battle/15/symbols.txt` | 2 | Reconcile map ownership for the 37 drifted battle/15 lifts. |
| `config/targets/emi/etc/shop/00/symbols.txt` | 2 | Reconcile map ownership for the 28 drifted shop/00 lifts. |
| `config/targets/exe/slus_004_22/symbols.txt` | 2 | Reconcile map ownership for the four drifted executable lifts. |
| `src/**/func_*.c` | 2 | Add required source and behavior metadata to invalid lifts. |
| `config/targets/**/splat.yaml` | 2 | Correct boundaries only after canonical payload evidence confirms them. |
| `tools/python/harness/commands/symbols.py` | 2 | Add a target-scoped check mode for incremental map remediation. |
| `tools/python/tests/test_canonical_symbols.py` | 2 | Cover target-scoped map validation and canonical ordering. |

| ID | Task |
| --- | --- |
| 2.1 | Add target-scoped symbol validation so one target can be repaired without unrelated drift masking its result. |
| 2.2 | Repair battle/15 map ownership and metadata before adding more lifts in that target. |
| 2.3 | Repair shop/00 map ownership and metadata before resuming its duplicate-group work. |
| 2.4 | Repair battle/03 map ownership and metadata before promoting its partial backlog. |
| 2.5 | Repair the four SLUS map drifts and validate their owning source metadata. |
| 2.6 | Resolve the remaining target-local invalid metadata in priority order from `bin/decomp-status`. |
| 2.7 | Verify every disputed Splat function boundary against canonical bytes before labeling it C or ASM code. |

**Design decisions**

- Do not bulk-add map entries from filenames without checking ownership,
  canonical bytes, and Splat placement.
- Preserve raw address names until behavior and ABI evidence justify a semantic
  promotion.
- Stop any target-local repair that changes an existing exact byte match until
  the dependent match set is revalidated.

## Phase 3 — Harden analysis inventory and candidate selection

| File | Phase | Action |
| --- | --- | --- |
| `tools/python/harness/reverse_index.py` | 3 | Persist canonical eligibility evidence for analyzed functions. |
| `tools/python/harness/commands/rev_query.py` | 3 | Expose eligibility and exclusion reasons in ranking output. |
| `tools/python/harness/layout.py` | 3 | Provide canonical code-boundary predicates for index consumers. |
| `tools/python/harness/rizin_project.py` | 3 | Keep snapshot replay identity tied to reviewed layouts and symbols. |
| `tools/python/tests/test_reverse_index.py` | 3 | Cover eligibility persistence across rebuilds. |
| `tools/python/tests/test_rev_query.py` | 3 | Cover code/data/SDK exclusion and eligible-candidate ranking. |

| ID | Task |
| --- | --- |
| 3.1 | Store canonical boundary and candidate-eligibility evidence during reverse-index rebuilds. |
| 3.2 | Report exclusion reasons instead of silently dropping filtered analyzer candidates. |
| 3.3 | Exclude shared SDK bodies, printable data, pointer tables, and non-function Splat ranges from lift rankings. |
| 3.4 | Add a verification command that reports analyzer roots conflicting with canonical payload or reviewed Splat evidence. |
| 3.5 | Require a fresh snapshot and index rebuild after any committed source, map, or Splat change. |

**Design decisions**

- Analyzer output remains a hypothesis; original bytes, manifests, and reviewed
  boundaries are the candidate-eligibility authority.
- Keep exclusion evidence in the index/report layer rather than teaching each
  agent to rediscover strings and pointer tables manually.

## Phase 4 — Reduce the unmatched function backlog

| File | Phase | Action |
| --- | --- | --- |
| `config/compiler/object-flags.cmake` | 4 | Record only flag-search profiles proven by exact matches. |
| `src/emi/battle/battle/15/` | 4 | Resolve high-impact partial lifts after Phase 2 repairs. |
| `src/emi/etc/shop/00/` | 4 | Resolve duplicate-backed shop lifts after ownership repairs. |
| `src/emi/battle/battle/03/` | 4 | Resolve leaf and duplicate candidates after ownership repairs. |
| `src/emi/world00/area*/` | 4 | Continue compact validated world-area lift groups. |
| `src/exe/slus_004_22/` | 4 | Continue runtime-service lifts with SDK boundaries preserved. |
| `docs/specs/runtime/memory-layouts.md` | 4 | Record stable field-layout evidence recovered by matched lifts. |
| `LESSONS.md` | 4 | Record reusable matching and boundary gotchas after independent verification. |

| ID | Task |
| --- | --- |
| 4.1 | Re-rank candidates from the eligibility-filtered index after every target repair batch. |
| 4.2 | Lift one target-qualified function at a time through asm-diff, byte-match, and independent review gates. |
| 4.3 | Use flag-search before matching aids when a clean-C function has a systematic compiler-profile residual. |
| 4.4 | Use one bounded permuter run only after types, control flow, and ABI are evidenced. |
| 4.5 | Escalate register pins and `INCLUDE_ASM` only after the documented clean-C ladder is exhausted and explicit approval is obtained. |
| 4.6 | Promote duplicate bodies only after two independently loaded target members byte-match. |
| 4.7 | Update specs and lessons only with evidence that survives the exact-match and review gates. |

**Design decisions**

- Prioritize map-clean targets and validated code candidates over raw caller
  count alone.
- Keep partial lifts target-local and non-reusable until one representative
  byte-matches.
- Never lift PsyQ/BIOS bodies; use the selected SDK space and generated weak
  bindings.

## Phase 5 — Keep acquisition, media, and audio tooling reliable

| File | Phase | Action |
| --- | --- | --- |
| `tools/python/harness/commands/setup.py` | 5 | Keep private-media setup orchestration limited to ignored inputs and toolchains. |
| `tools/python/harness/commands/doctor.py` | 5 | Keep prerequisite checks aligned with setup lifecycle verification. |
| `tools/python/harness/toolchain/*.py` | 5 | Maintain one lifecycle owner per external toolchain. |
| `tools/rust/bof3-disk/src/main.rs` | 5 | Replace the unimplemented rebuild parity path or remove it after confirming no supported workflow depends on it. |
| `tools/c/psx-audio/` | 5 | Maintain audio decode, rendering, and export tests independently from decomp matching work. |
| `docs/specs/formats/audio.md` | 5 | Keep verified audio-format and renderer evidence current. |

| ID | Task |
| --- | --- |
| 5.1 | Keep setup and doctor outputs concise while preserving independent verification of every managed toolchain. |
| 5.2 | Add or complete the bof3-disk rebuild parity implementation only when its command contract is required by a supported workflow. |
| 5.3 | Add regression coverage around media inventory and extraction boundaries without committing proprietary inputs. |
| 5.4 | Keep PSX audio format changes behind focused C and wrapper-level validation. |

**Design decisions**

- `inputs/external/`, toolchains, build products, and `out/` remain private or
  disposable and are never planning deliverables.
- Do not add a second toolchain installer or duplicate lifecycle abstraction.

## Phase 6 — Enforce repository health and documentation closure

| File | Phase | Action |
| --- | --- | --- |
| `justfile` | 6 | Keep the aggregate check aligned with intentional repository health gates. |
| `tools/python/harness/commands/validate_sources.py` | 6 | Validate authored source conventions without hiding ownership failures. |
| `tools/python/tests/` | 6 | Extend tests for every changed harness contract. |
| `docs/index.md` | 6 | Link newly stabilized specifications and remove stale references. |
| `AGENTS.md` | 6 | Keep plan and verification references concise and current. |

| ID | Task |
| --- | --- |
| 6.1 | Make `just check` pass only after symbol-map and source metadata drift are repaired rather than skipped. |
| 6.2 | Keep Ruff, pytest, source validation, map validation, and exact-lift checks independently diagnosable. |
| 6.3 | Add documentation links only for proven, stable contracts and remove superseded workflow text. |
| 6.4 | Review this plan after each completed phase and replace completed backlog counts with current audit evidence. |

**Design decisions**

- Do not weaken checks to accommodate drift; repair the owning source, map, or
  layout instead.
- Keep transient process instructions in skills and AGENTS.md, while durable
  format/runtime facts belong in `docs/specs/`.

## Files Summary

| File | Action |
| --- | --- |
| `PLAN.md` | New |
| `AGENTS.md` | Modified |
| `tools/python/harness/decomp_status.py` | Modified |
| `tools/python/harness/commands/symbols.py` | Modified |
| `tools/python/harness/reverse_index.py` | Modified |
| `tools/python/harness/commands/rev_query.py` | Modified |
| `tools/python/harness/layout.py` | Modified |
| `tools/python/harness/rizin_project.py` | Modified |
| `tools/python/harness/commands/validate_sources.py` | Modified |
| `tools/python/tests/test_decomp_status.py` | Modified |
| `tools/python/tests/test_canonical_symbols.py` | Modified |
| `tools/python/tests/test_reverse_index.py` | Modified |
| `tools/python/tests/test_rev_query.py` | Modified |
| `config/targets/**/symbols.txt` | Modified |
| `config/targets/**/splat.yaml` | Modified |
| `config/compiler/object-flags.cmake` | Modified |
| `src/**/func_*.c` | Modified or new |
| `src/**/internal.h` | Modified |
| `docs/specs/runtime/memory-layouts.md` | Modified |
| `docs/specs/formats/audio.md` | Modified |
| `docs/index.md` | Modified |
| `LESSONS.md` | Modified |
| `justfile` | Modified if check contract changes |
| `tools/rust/bof3-disk/src/main.rs` | Modified or deleted after command-contract review |

## Acceptance Criteria

- [ ] `PLAN.md` stays below 300 lines and uses numbered phases and tasks.
- [ ] `AGENTS.md` directs plan creation and management to `PLAN.md`.
- [ ] `bin/decomp-status --json` reports no invalid lifts caused by missing required metadata.
- [ ] `bin/symbols check` reports no source/map or binding/map drift.
- [ ] Reverse rankings exclude canonical data, pointer tables, non-function boundaries, and shared SDK bodies while retaining valid reviewed code candidates.
- [ ] Each new or modified lift passes target-qualified `bin/asm-diff` and `bin/byte-match` before promotion.
- [ ] Every exact lift receives an independent read-only review before commit.
- [ ] Exact duplicate sharing occurs only after two independently loaded members byte-match.
- [ ] `just check` passes without skipped source, map, or validation gates.
- [ ] New Python harness behavior includes focused pytest coverage and Ruff passes.
- [ ] No proprietary media, generated `out/`, build products, or installed toolchains are committed.
