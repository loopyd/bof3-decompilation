# Lift-tool iteration performance

**Status:** in progress — phases 1.1, 1.2, 2.1, and 3 implemented; phase 5 measurement pending final full validation.

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
