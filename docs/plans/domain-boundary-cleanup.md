# Domain boundary cleanup

## Goal

Remove confirmed duplicate target/manifest resolution from the harness and Pi
status scripts while keeping `harness.domain` limited to stable identity,
manifest, and resolved-target facts.

## Evidence baseline — 2026-07-31

- `ids.py` owns target/function selector parsing; function workflows now consume
  `parse_function_id()` and its shared CLI help constants.
- `registry.py` has `resolve_target()`, but it intentionally requires a present
  binary, so callers that only need a validated manifest cannot use it.
- `analyzer.py`, `rizin_project.py`, `commands/build.py`, and `commands/splat.py`
  repeat normalize → `load_target_manifests()` → lookup → unknown-target error.
- `.pi/skills/psx-rizin/scripts/snapshot-status.py` independently parses every
  `target.toml`; `.pi/skills/bof3-lift-loop/scripts/loop-status.py` independently
  scans them only to list IDs.
- `agent-context.py` has alias-only compatibility behavior (`BATTLE#15@...`).
  It is not a public harness CLI contract yet.

## Phase 1 — Manifest-only domain lookup — complete (2026-07-31)

1. Add a small public domain lookup that normalizes a supplied target selector,
   loads manifests, and returns the matching `TargetManifest` without requiring
   a binary.
2. Keep `resolve_target()` for callers that require all resolved paths and an
   existing binary; do not make the lightweight lookup construct paths.
3. Replace the four confirmed normalize/load/lookup copies:
   - `tools/python/harness/analyzer.py`
   - `tools/python/harness/rizin_project.py`
   - `tools/python/harness/commands/build.py`
   - `tools/python/harness/commands/splat.py`
4. Add focused unit coverage for canonical and shipped target IDs plus an
   unknown target. Preserve each command's existing user-facing errors unless a
   common error is already part of the lookup contract.

Acceptance: focused tests and full `tools/python/tests` pass; `ruff check` and
`git diff --check` pass.

Completed evidence: added `lookup_target_manifest()` and migrated the four
confirmed callers. Focused tests, `just check`, the full Python suite (275
passed), Ruff, and `git diff --check` passed. `resolve_target()` remains the
binary-requiring resolved-path API.

## Phase 2 — Eliminate unvalidated skill manifest scans — complete (2026-07-31)

1. Update `.pi/skills/psx-rizin/scripts/snapshot-status.py` to load typed
   manifests from `harness.domain` rather than parsing TOML itself.
2. Update `.pi/skills/bof3-lift-loop/scripts/loop-status.py` to list IDs via the
   same loader.
3. Preserve the scripts' JSON schemas, ordering, and no-mutation behavior.
4. Extend the existing skill-script smoke test only if it does not already
   execute the changed paths.

Acceptance: skill-script smoke test, full Python suite, focused Ruff check, and
`git diff --check` pass.

Completed evidence: both scripts now load typed manifests via `harness.domain`;
they retain their schemas and sorted output. Direct-execution contract tests,
the existing skill-script smoke test, `just check`, the full Python suite (275
passed), Ruff, and `git diff --check` passed.

## Phase 3 — Remove the agent-context alias shim — complete (2026-07-31)

Removed `BATTLE#15` and other manifest-alias fallback parsing from
`agent-context.py`. Every function-facing harness and Pi script now accepts
only the documented canonical selector `TARGET@0xADDRESS` or shipped EMI
selector `BIN/FAMILY/ARCHIVE.EMI#INDEX@0xADDRESS` through
`parse_function_id()`. The skill smoke test now uses the canonical target.

Acceptance met: the skill-script smoke suite passes; `BATTLE#15@...` is
rejected; Ruff and `git diff --check` pass.

## Phase 4 — Follow-up refactor candidates — complete (2026-07-31)

No source change qualified under the two-equivalent-callers or verified-dead-code
gate.

1. **Task registration:** retained. `doctor_task()` and `setup_task()` have
   incompatible `Path` and mutable `SetupState` runner contracts; no third
   compatible registry exists.
2. **Python submodule toolchains:** retained. Existing toolchain base classes
   cover the proven common behavior, while M2c and Maspsx retain different
   install and working-directory contracts; no third equivalent toolchain exists.
3. **Large modules:** retained. No cohesive, independently usable unit with
   focused-test evidence was demonstrated; module size alone is not evidence.
4. **Path helpers:** retained. `ResolvedTarget` remains the complete path bundle;
   one-off paths remain locally owned and do not justify generic wrappers.

Acceptance met: Phase 4 makes no source changes or speculative abstractions;
this documentation-only result records the gate evidence.

## Deferred non-goals

- Do not create generic wrappers for `root / manifest.binary`, `splat`, or
  target-owned evidence paths. `ResolvedTarget` already serves callers that
  require a full path bundle.
- Do not move catalog/materialization logic, raw data parsing, or toolchain
  installation code into `domain`.
- Do not commit, stage, reset, clean, or change generated/proprietary state.

## Boundaries and risks

The existing working tree contains separate, uncommitted archive/helper and
selector cleanup. This plan must preserve it. Only merge a domain extraction
when at least two callers share identical behavior; avoid turning `domain` into
a generic utilities package.
