# Lift-tool iteration performance

**Status:** in progress — phases 1.1, 1.2, 2.1, 2.3, and 3 implemented; phase 5 measurement is pending final full validation.

## Goal

Make the review-gated lift loop fast enough for serial batch lifting without
weakening exact-byte, target-local ownership, companion, or snapshot-freshness
gates. Optimize the measured hot paths first; preserve a live match command for
every accepted lift.

## Evidence baseline (2026-07-26)

Measured on `emi/battle/battle/03@0x801D6DE4` with warm local toolchains:

| Command | Wall time | Finding |
| --- | ---: | --- |
| `bin/asm-diff ... --detail minimal` | 0.881 s | Builds, links, extracts bytes, disassembles both sides, and writes artifacts. |
| `bin/byte-match ...` | 0.982 s | Calls the same full match path despite needing only compiled bytes. |
| `bin/splat emi/battle/battle/03` | 0.874 s | Required reviewed-layout generation/validation; not a primary target. |
| `bin/rz-project analyze emi/battle/battle/03` | 0.842 s | Target-local analyzer refresh is already bounded. |
| `bin/index` | 0.898 s | Rebuilds every target's derived reverse index. |
| `bin/companion-check ...` | 6.145 s | Re-hashes/scans every extracted EMI through `load_catalog()`. |
| `bin/decomp-status emi/battle/battle/03 --detail minimal` | 120.489 s | Re-runs a complete match for all 140 authored lifts. |
| `function-brief.py` for an existing lift | 122.244 s | Runs the target-wide status audit, then `asm-diff`, then `byte-match`. |

`decomp-status` is the dominant regression: its `minimal` display projection
changes output volume, not computation. `collect_lifts()` calls
`run_asm_diff_one()` for every source. The resulting per-function cache must
be content-keyed; timestamps are insufficient proof for byte-match results.

## Non-goals

- Do not weaken `bin/byte-match` as the acceptance authority.
- Do not cache a result across a change to source, compile/link inputs, binary,
  target layout/map, or toolchain identity.
- Do not make `rev-query` use stale snapshots, make `loop-status` silently
  repair authored state, or relax companion ABI/ownership verification.
- Do not add a daemon, database service, dependency, parallel writes to a
  target, or hand-maintained build dependency list.
- Do not optimize intentionally bounded searches (`bin/permute`,
  `bin/flag-search`) before the deterministic loop path.

## Phase 1 — remove duplicated live work

### 1.1 Give `byte-match` a byte-only comparator

**Files:**
- `tools/python/harness/commands/lift.py`
- `tools/python/harness/match/asm_diff.py`
- `tools/python/harness/match/_asm_link.py` (only if a small shared helper is
  needed)
- `tools/python/tests/test_lift.py`

Split the current `run_asm_diff_one()` internals into a common compile/link/
original-byte comparison and an optional diagnostic-artifact layer. The
`byte-match` path must still:

1. build the owned object;
2. link undefined symbols through the same target-qualified canonical bindings;
3. compare the original function bytes; and
4. validate declared section placements.

It must skip only work not used for its contract: compiler/original/linked
objdump calls, instruction normalization, `SequenceMatcher`, diff/bundle
materialization, and output-directory recreation. Keep `asm-diff` fully live
and artifact-producing.

**Acceptance:** existing exact/partial exit semantics and JSON fields remain
compatible; a focused test proves `byte-match` never calls disassembly/bundle
writers while still rejecting mismatched bytes and placement bytes.

### 1.2 Make an existing-lift brief perform one live comparison

**Files:**
- `.pi/skills/bof3-re/scripts/function-brief.py`
- `.pi/skills/bof3-re/scripts/test-skill-scripts.py`
- `docs/usage.md`

For a source that exists, remove the broad `bin/decomp-status TARGET --json`
subprocess and do one `bin/asm-diff TARGET@0xADDRESS --json`. Derive the
brief's `byte_match` projection from that same payload rather than launching a
second build/link/compare. Keep metadata validation in the brief itself or
return the same invalid result shape before matching.

Do not substitute cached status for this live per-function evidence: the brief
is used immediately before an edit/review decision. The retained output must
clearly identify that both fields came from one live comparison.

**Acceptance:** a mocked script test asserts no `decomp-status` or second
`byte-match` child is spawned for an existing lift; exact and non-exact brief
records retain their current decisive fields.

### 1.3 Eliminate cheap same-process duplication only

**Files:**
- `tools/python/harness/commands/lift.py`
- `tools/python/harness/match/asm_diff.py`
- `tools/python/harness/match/_asm_link.py`
- `tools/python/harness/build.py`
- focused existing tests

Thread the already parsed manifest and canonical symbol-address mapping through
the match request instead of reloading them in `resolve_function()`,
`_run_match()`, `run_asm_diff_one()`, and `_target_map_bindings()`. Write
`out/bindings/<target>/symbols.c` only when its generated content differs.
Preserve CMake's own configure/dependency detection; do not add an mtime-based
configure skip that can miss a newly globbed source.

**Acceptance:** match bytes are unchanged; a unit test covers the supplied
canonical binding map; unchanged generated bindings preserve their mtime.

**Completed evidence (2026-07-26):** `byte-match` uses the byte-only path and retains live build/link/placement validation; existing-lift `function-brief` performs one live `asm-diff` and derives its byte projection; `companion-check` uses target-scoped immutable entry verification. Focused tests cover these paths.

## Phase 2 — cache target audits, not acceptance checks

### 2.1 Add a content-addressed decomp-status cache

**Files:**
- new `tools/python/harness/match/status_cache.py`
- `tools/python/harness/decomp_status.py`
- `tools/python/harness/commands/decomp_status.py`
- new focused cache/status tests
- `docs/usage.md`

Store disposable per-function match summaries in
`out/matching/status-cache.sqlite`; do not extend `out/index/reverse.sqlite`,
which is solely Rizin-derived evidence. Enable this cache by default for
`bin/decomp-status`; provide `--no-cache` for diagnosis and an explicit
`--clear-cache` maintenance action only if the implementation needs one.

A cache row is reusable only when all of these content fingerprints match:

- target ID and source-relative path;
- source C content;
- sibling target headers compiled by CMake (`src/<target>/*.h`);
- repository shared compile inputs (`CMakeLists.txt`, compiler object-flag
  config, `include/**/*.h|*.inc`, and `src/shared/**/*.h|*.inc`);
- target link inputs: target manifest section placements, target map, shared
  map, selected PsyQ map, and the generated-binding schema/version;
- original binary content; and
- compiler/linker/objdump identity used by the matcher.

Hash shared input groups once per report and target-specific groups once per
target; hash each lift source separately. Store only the report fields needed
by `decomp-status` (`byte_match`, instruction counts, sizes, and the input
fingerprint), not paths to transient diff artifacts. Metadata-invalid files are
reported before cache lookup. Cache writes use a transaction and tolerate a
missing/corrupt cache as a miss; no cache hit may be treated as a live
acceptance check.

**Implemented shape:** `tools/python/harness/match/status_cache.py` stores only report summaries in `out/matching/status-cache.sqlite`; `bin/decomp-status` uses it by default and `--no-cache` forces live recomputation. The fingerprint includes target compiler inputs, target maps/layout/manifest, binary content, source content, and the cache schema. It is never consulted by `asm-diff` or `byte-match`.

**Acceptance:**

- cold cached and `--no-cache` reports are structurally identical;
- a second unchanged target audit reuses all rows;
- changing one source recomputes only that source;
- changing a target header/map/manifest recomputes that target;
- changing a shared header/map/compiler input recomputes affected rows;
- changing an original binary recomputes that target; and
- the warm `emi/battle/battle/03` audit is at least 90% faster than the
  120.489-second baseline while still reporting the same exact/partial/invalid
  counts.

### 2.2 Use audit cache only where it is safe

**Files:**
- `.pi/skills/bof3-lift-loop/SKILL.md`
- `.pi/skills/bof3-re/SKILL.md`
- `docs/usage.md`

Document the distinction:

- use cached `decomp-status` for target/project audits and progress counts;
- use live `asm-diff`, live `byte-match`, companion check, Splat, and symbols
  check immediately before accepting a lift;
- never accept a lift solely because its status-cache row says `exact`.

**Acceptance:** the lift-loop review/commit gate remains explicitly live and
unchanged in the skill instructions.

### 2.3 Batch cache-miss object builds without weakening per-function matching

**Evidence baseline (current worktree):**

- The project has 643 `func_*.c` lifts. A warm full `validate-sources` audit
  completed in **1.10 s** after safe CMake build-tree reuse; its component
  report was `exact=395`, `partial=222`, `invalid=26`.
- `pytest`, Ruff, and `symbols check` took approximately **1.33 s**, **0.03 s**,
  and **0.29 s**, respectively. The audit remains the only material `just
  check` cost after the warm cache is populated.
- `collect_lifts()` still invokes `run_asm_diff_one()` independently for every
  cache miss. Although the configure pass is now reused, each miss launches a
  separate `cmake --build --target lift_<hash>` process.
- `CMakeLists.txt` already creates both a stable per-source `lift_<hash>` target
  and a directory-wide `target_<hash>` target. The per-source targets are the
  safe batch unit: they exclude metadata-invalid or otherwise unrelated source
  files, while allowing one native build invocation to request every valid
  cache-miss object for an owning image.

**Goal:** reduce a cold or partially invalidated audit to one CMake build
invocation per owning target, then retain the existing target-qualified
link/byte/placement comparison and per-function report format.

**Boundaries and non-goals:**

- This is an audit optimization only. `bin/asm-diff` and `bin/byte-match` stay
  live, one-function acceptance authorities and must not consume status-cache
  rows or a previously batched object without rebuilding it.
- Do not batch the directory-wide `target_<hash>` target: one unrelated broken
  or metadata-invalid source must not hide the status of valid selected lifts.
- Do not add a daemon, a new dependency, a parallel Python worker pool, a
  persistent build scheduler, or user-facing batch flags. One CMake invocation
  lets the configured native generator select its own safe parallelism.
- Do not commit `out/`, `build/`, or toolchain artifacts. Existing 26 invalid
  lifts are report data outside this performance scope; they must remain
  visible without being compiled merely to accelerate valid rows.

#### Phase 2.3.1 — separate audit discovery from comparison

**Files:**
- `tools/python/harness/decomp_status.py`
- `tools/python/tests/test_decomp_status.py`
- only a small adjacent matcher test if required

Make the current sequential `collect_lifts()` loop first construct its report
worklist in sorted target/source order:

1. retain the existing filename and metadata-invalid records immediately;
2. compute the existing content fingerprint and reuse matching status-cache
   records unchanged;
3. group only valid cache misses by their owning manifest; and
4. preserve the existing source path, address, binary, load address, and
   reviewed section-placement evidence for each miss.

The preflight must not alter record ordering, totals, cache schema, status
labels, `--no-cache` semantics, or the rule that a missing/corrupt cache is a
miss. It may use small local tuples/dicts; do not introduce an executor class
or a second reporting model.

**Acceptance:** a focused test with cached, uncached, and metadata-invalid
sources proves that only valid misses enter the build worklist, report output is
identical to the current sequential shape, and invalid rows cause no build
request.

#### Phase 2.3.2 — build selected source objects once per target

**Files:**
- `tools/python/harness/build.py`
- `tools/python/harness/match/asm_diff.py`
- `tools/python/harness/decomp_status.py`
- `tools/python/tests/test_build.py`
- `tools/python/tests/test_decomp_status.py`
- `tools/python/tests/test_asm_diff.py` only if the existing test location
  covers the extracted helper better

Add the smallest build helper that accepts the selected source paths and runs
one `cmake --build <tree> --target lift_<hash> ...` command for that target.
Reuse `cmake_target_for_source()` and the already-safe configured-tree check;
do not duplicate CMake target hashing in the validator. A zero-miss target must
not invoke CMake.

Refactor only enough of `run_asm_diff_one()` for the audit path to run its
existing object freshness check, link, original-byte extraction, placement
validation, size lookup, disassembly, and diagnostic bundle generation **after
that batch succeeds**, without issuing another build. Normal one-function
`asm-diff` and `byte-match` retain their current build path.

If a multi-target CMake request fails, retry the selected misses through the
current one-source build path solely to produce the current per-source invalid
records. That slow fallback is an error path, not a success-path optimization;
it prevents a shared build failure from obscuring which lift failed.

**Acceptance:** tests verify one build invocation for multiple valid misses in
the same target, no batch call for all-cache-hit targets, an object freshness
check before comparison, and individual error attribution after an injected
batch failure. Existing byte-match, reviewed placement, and diagnostic-artifact
tests must continue to pass unchanged.

#### Phase 2.3.3 — prove invalidation and measure both paths

**Files:**
- `tools/python/tests/test_decomp_status.py`
- `docs/usage.md`
- this plan

Expand cache tests so a source change recomputes one row, a target compile/link
input recomputes that target's valid rows, and a shared compile input invalidates
the affected targets according to the existing fingerprint contract. Confirm
that each invalidation produces one selected-target build rather than N source
builds.

Measure with Python/stdlib timing after one warm-up, recording the command,
source revision, lift count, counts, and median of three runs for:

| Path | Command shape | Required result |
| --- | --- | --- |
| warm audit | `bin/decomp-status --detail minimal` | report parity and no builds for hits |
| target cache miss | remove/rotate only the disposable target cache rows, then `bin/decomp-status TARGET --detail minimal` | one CMake build invocation for that target's valid misses |
| forced live target audit | `bin/decomp-status --no-cache TARGET --detail minimal` | identical status totals; one selected-target build |
| live acceptance | `bin/asm-diff TARGET@ADDRESS`, `bin/byte-match TARGET@ADDRESS` | unchanged live build/link/placement behavior |

Do not compare a cache-hit audit to an acceptance command. Restore the
throwaway cache naturally by rerunning the audit; do not hand-edit reviewed
inputs to create a benchmark case.

**Acceptance:** cache-miss audit totals match the sequential baseline, the
batch command count is one per affected target on success, and the measured
uncached target audit improves materially over the pre-batch per-source build
path without any accepted lift relying on a cache row.

**Dependencies and blockers:** Phase 2.3.1 precedes 2.3.2 so the batch never
compiles unreportable inputs. Phase 2.3.2 precedes measurements. Stop if CMake
cannot accept multiple `lift_<hash>` targets in one configured generator, if an
object lacks a trustworthy post-batch freshness signal, or if batch failure
cannot retain per-source diagnostics. The direct-JAL snapshot schema upgrade
and safe CMake-tree validation currently in the worktree must be validated and
land before benchmarking this redesign, because stale graph evidence or a
partial build tree invalidates the baseline.

## Phase 3 — make companion verification target-scoped

**Files:**
- `tools/python/harness/emi/catalog.py`
- `tools/python/harness/commands/companion_check.py`
- `tools/python/tests/test_emi_catalog.py`
- new companion-check command tests

Keep `build_catalog()`'s complete catalog contract and its
`companion_relations` result; existing callers/tests rely on it. Add a narrow
helper for `companion-check` that verifies only the caller manifest's declared
companions. It must read only the required caller/companion EMI manifest and
payloads, then perform the same checks as the full catalog path:

1. target identity, load address, payload size, and SHA-256;
2. RAM payload classification; and
3. immutable caller `jal` bytes at each declared callsite.

Pass the manifest dictionary already loaded by `build_report()` into
`_companion_report()` instead of loading it again. The narrow helper's report
must be byte-for-byte equivalent in readiness-relevant fields to the full
catalog result for a given caller.

**Acceptance:** tests cover the existing missing ABI/boundary/map/declaration
failures plus changed caller bytes and changed companion payload identity.
A timed valid no-companion check and valid companion check demonstrate a
substantial reduction from the 6.145-second baseline without accepting an
unverified relation.

**Completed measurement (2026-07-26):** the battle target check fell from
6.145 s to **0.088 s** after replacing global EMI catalog construction with the
caller-scoped verifier. The target-scoped verifier has parity coverage against
the catalog relation result.

## Phase 4 — batch analysis refreshes without stale evidence

**Files:**
- `.pi/skills/bof3-lift-loop/SKILL.md`
- `.pi/skills/bof3-lift-loop/scripts/loop-status.py`
- possibly `docs/usage.md` and loop-script tests

A Splat/map promotion changes the Rizin replay recipe, so the target snapshot
rightly becomes stale. Do not repair every stale target between serial lifts.
Instead:

1. obtain a bounded candidate queue from one fresh snapshot/index before the
   batch;
2. retain each candidate's mission/companion evidence with the journal;
3. complete source/map/Splat/live-match/review gates serially;
4. refresh only the edited targets and rebuild the global index once at the
   batch checkpoint, before querying for a new queue or reporting index-backed
   selection;
5. stop rather than query a stale index if the checkpoint refresh fails.

Make `loop-status.py` inspection-only by default: report staleness and require
an explicit `--recover` to analyze/reindex generated artifacts. This avoids an
unexpected repository-wide analyzer pass from a dashboard invocation. The
recovery command may still refresh all stale targets when explicitly requested;
do not add a weakly validated index mode.

**Acceptance:** a script test proves default status does not run analysis or
index rebuild, explicit recovery does, and the loop never requests a new
candidate from a stale index. Batch validation continues to use target-local
fresh snapshots at the checkpoint.

## Phase 5 — measure, document, and adopt

**Interim measurements (2026-07-26):** warm cached
`bin/decomp-status emi/battle/battle/03 --detail minimal` is **0.132 s** versus
**117.610 s** with `--no-cache` (baseline: 120.489 s), while preserving
`36 exact / 104 partial / 0 invalid`. Existing-lift `function-brief` is
**1.641 s** versus the 122.244-second baseline.

**Files:**
- `docs/usage.md`
- this plan
- only the smallest test additions from phases 1–4

Run a repeatable shell/Python-stdlib timing sequence after a warm-up for:
`byte-match`, `asm-diff`, `companion-check`, cached and uncached target
`decomp-status`, existing-lift `function-brief`, `rz-project analyze`, `index`,
and `loop-status` with and without explicit recovery. Record medians and source
revision in this plan before marking it complete.

Adopt this operational loop:

```sh
# One live iteration (never cache-only acceptance)
bin/asm-diff TARGET@0xADDRESS --detail normal
bin/byte-match TARGET@0xADDRESS
bin/companion-check TARGET@0xADDRESS
bin/splat TARGET
bin/symbols check TARGET

# Cheap warm audit/progress count
bin/decomp-status TARGET --detail minimal
```

Run `just check`, all focused cache/companion/script tests, `git diff --check`,
and a clean decomp-status audit before handoff.

**Completed evidence (2026-07-26, worktree at `1e11c90` plus pending diff):**

`collect_lifts()` now uses a preflight+batch strategy:

1. **`_build_preflight()`** (Phase 2.3.1) — separates invalid/cached records
   from valid cache misses, ordered by target then source.  Invalid rows and
   cache hits are returned immediately as ``ready``; misses are grouped into a
   per-target worklist.

2. **`_run_batch_misses()`** (Phase 2.3.2) — for each target with misses,
   runs one ``cmake --build --target lift_<hash> <hash2> <hash3> ...``
   invocation (via the new ``build.batch_build()`` helper), then compares each
   miss individually with ``asm_diff._asm_diff_compare()`` without rebuilding.
   On batch failure, falls back to per-source ``diff_runner()`` calls.

3. **`build.batch_build(root, targets)`** — new public helper that passes all
   requested per-source ``lift_<hash>`` targets through CMake's ``--target``
   interface in one process. Zero-miss targets never invoke CMake. Reuse checks
   both generator files and ``CMAKE_HOME_DIRECTORY``; a copied, foreign, or
   incomplete tree is removed before configuring the requested root, because
   CMake cannot replace a foreign or mismatched-generator cache in place.

4. **`asm_diff._asm_diff_resolve()` and ``asm_diff._asm_diff_compare()``** —
   extracted from the former monolithic ``run_asm_diff_one()``.  The resolve
   step determines source path, address, size, object/output paths without
   building.  The compare step runs link, byte match, placement validation,
   size lookup, disassembly, and diagnostics.  ``run_asm_diff_one()`` calls
   both; the batch audit calls resolve once per miss to know what to build,
   builds in batch, then calls compare zero-build per miss.

**Test additions (Phase 2.3.3):**

- ``test_build_preflight_separates_cache_misses_from_ready`` — invalid metadata,
  cache hits, and valid misses land in the correct buckets.
- ``test_build_preflight_reuses_cache_hits`` — cache hits appear in ready,
  never in the worklist.
- ``test_batch_builds_fresh_misses_once_per_target`` — two misses in one
  target produce one successful mocked ``batch_build`` invocation, create
  fresh objects, compare under the supplied root, and avoid fallback.
- ``test_batch_failure_falls_back_per_source_with_error_attribution`` — a
  nonzero batch result runs the existing per-source comparator once per source
  and preserves exact/partial attribution without duplicate records.
- ``test_batch_stale_object_falls_back_once_without_duplicate_record`` and
  ``test_batch_resolve_failure_falls_back_once`` — a successful batch that
  leaves a stale object or cannot resolve one comparison takes the per-source
  path once, never compares stale output, and emits one attributed record.
- ``test_source_change_invalidates_cache_and_recomputes`` and
  ``test_compile_inputs_invalidate_only_affected_target_then_all_targets`` —
  source, target-map, and shared-header invalidation rebuild only the affected
  target(s), once per target.
- ``test_no_batch_when_all_sources_are_cached`` — an all-cache-hit target
  issues zero build commands.
- ``test_batch_build_passes_multiple_targets_as_native_args`` — verifies the
  all targets appear after ``--target`` without a ``--`` separator.
- ``test_batch_build_raises_on_empty_targets`` — validates the guard.

Existing ``byte-match``, ``asm-diff``, placement validation, and cache tests
pass unchanged.

**Measured medians (three runs after warm-up, `emi/battle/battle/03`, 184 lifts,
`exact=71 partial=103 invalid=10`):**

| Path | Command | Median | Runs |
| --- | --- | ---: | --- |
| cache miss | delete only this target's disposable status-cache rows, then `bin/decomp-status emi/battle/battle/03 --detail minimal` | 41.530 s | 41.163, 41.859, 41.530 s |
| warm audit | `bin/decomp-status emi/battle/battle/03 --detail minimal` | 0.797 s | 0.731, 0.805, 0.797 s |
| no-cache diagnostic | `bin/decomp-status --no-cache emi/battle/battle/03 --detail minimal` | 39.582 s | 39.098, 40.484, 39.582 s |

All three retained identical totals. The live acceptance check
`bin/asm-diff emi/battle/battle/03@0x801D6EAC --detail minimal` and
`bin/byte-match emi/battle/battle/03@0x801D6EAC` both passed after the audit.
`--no-cache` intentionally bypasses summaries but still uses the selected
per-target batch build; it is a diagnostic audit, never lift acceptance.

## Dependencies and blockers

- Phase 1.1 precedes 1.2 because the brief's single live comparison should use
  the fast byte projection rather than duplicate it.
- Phase 2 depends on the stable result payload exposed by phase 1.1.
- Phase 3 is independent of phase 2 and can proceed after its parity tests are
  established.
- Phase 4 depends on the current lift-loop journal/selection contract and must
  not be enabled while an existing run has uncheckpointed stale evidence.
- The currently interrupted battle lift must be restored to a live
  `decomp-status` exact state before resuming the lifting loop; performance work
  must not hide invalid retained source.
