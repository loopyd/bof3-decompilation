# Flat reverse snapshots

## Goal and baseline

Store one target-qualified JSON file per target under `out/reverse/snapshots/`
instead of `out/reverse/<target>/snapshot.json`. The implementation now stores
all 23 manifest snapshots flat with no nested `snapshot.json` files. `out/` and
the SQLite reverse index are disposable.

Use collision-safe readable filenames: percent-encode the canonical target ID
path components and join them with `--`, e.g.
`emi--world00--area027--13.json`. Do not use `/`→`_`: existing target IDs such
as `exe/slus_004_22` make that encoding non-reversible and future collisions
possible.

## Phase 1 — canonical path and tests (complete)

1. Change only `snapshot_path()` in `tools/python/harness/snapshot.py`; keep all
   analyzer/index/status consumers routed through it.
2. Add direct tests for deterministic encoding, underscore preservation,
   traversal rejection, and distinct IDs producing distinct filenames.
3. Update hardcoded fixtures/assertions in:
   - `tools/python/tests/test_target_manifest_consumers.py`
   - `tools/python/tests/test_reverse_index.py`
   - `tools/python/tests/test_psyq.py`
4. Acceptance: focused snapshot, analyzer, reverse-index, Rizin-project,
   decomp-status, PsyQ, analysis-sequence, lift-loop-status, and skill-status
   tests pass.

## Phase 2 — documentation and disposable regeneration (complete)

1. Update exact path references in `docs/usage.md`,
   `docs/agents/lessons.md`, `.pi/skills/psx-rizin/SKILL.md`, and its workflow /
   Rizin references. Keep broad `out/reverse/` references when still accurate.
2. Run agent-skill compaction checks after `.pi` Markdown edits.
3. Remove the old generated `out/reverse/{emi,exe}/` tree, regenerate all 23
   snapshots serially, then rebuild `out/index/reverse.sqlite` once.
4. Acceptance: exactly 23 JSON files exist directly under
   `out/reverse/snapshots/`; no nested `snapshot.json` remains; every
   `bin/rz-project status TARGET --json` is fresh; `bin/rev-query --json status`
   passes.

## Validation evidence

- Focused harness/skill suite: 43 tests passed.
- Compaction audit and `git diff --check`: passed.
- Generated state: 23 direct JSON files, zero nested `snapshot.json` files.
- Every target reports `fresh=true`; `bin/rev-query --json status` lists all 23.
- `test-skill-scripts.py` remains blocked by an unrelated generated context
  exceeding its 100 KB ceiling; the modified psx-rizin Markdown passed the
  compaction audit.

## Boundaries and non-goals

- No compatibility reader or migration script for disposable old snapshots.
- Do not change snapshot schema/content, target IDs, analyzer semantics, maps,
  Splat, lifts, or tracked generated artifacts.
- The SQLite index must be rebuilt, not patched in place.
- Stop if the filename encoder is not injective over all manifest target IDs or
  if any consumer bypasses `snapshot_path()`.
