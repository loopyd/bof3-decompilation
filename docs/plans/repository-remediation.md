# Repository Remediation Plan

**Status:** active

## Scope and evidence baseline

This is the active repository-wide remediation plan. It coordinates source/map
integrity, reverse-index evidence, lifting gates, toolchain ownership, and
repository-health checks. It does not authorize committing private media,
installed toolchains, generated `out/` state, or build products.

Baseline reviewed on 2026-07-23:

- `harness.commands.doctor --root .` passes all 5 checks, including every
  currently registered managed toolchain and wrapper check.
- Focused tests for decomp status, canonical symbols, reverse index, and
  rev-query pass (19 tests); the current Python toolchain-focused suite passes
  (4 tests).
- `bin/symbols check` fails on source/map drift in battle/03, battle/15,
  shop/00, and SLUS plus one SLUS binding/map drift. Game/01 was normalized
  locally in Phase 2.6; the remaining failures must be repaired at their owning
  targets.
- `bin/decomp-status --json` exceeded a 45-second audit timeout. Treat its
  aggregate counts as stale until its runtime is profiled or target-scoped
  reports provide a repeatable baseline.
- The harness already has invalid-lift reporting, canonical candidate exclusion,
  snapshot identity validation, and regression coverage. The work is to verify
  those contracts against live target data, fill missing diagnostics, and avoid
  duplicating existing behavior.

## Operating rules

- Execute incomplete tasks in a phase only after its listed evidence baseline is
  current. Phases 2 and 5 may proceed independently after Phase 1 because map
  repair and toolchain ownership do not share mutable source artifacts.
- Before modifying a lift, load `/skill:bof3-re`; retain `TARGET@0xADDRESS`
  qualification and the clean-C matching ladder.
- A failed source/map or toolchain check is a repair task, not permission to
  weaken a gate. Record durable format/runtime evidence in `docs/specs/` and
  reusable process findings in `LESSONS.md`.
- Mark a task complete only with its validation evidence and update this plan's
  baseline and remaining work at that time.

## Phase 1 — Establish auditable baselines **[complete]**

**Evidence:** focused Python tests pass; doctor passes; current map failures and
the decomp-status timeout are recorded above.

| ID | Status | Work | Validation |
| --- | --- | --- | --- |
| 1.1 | complete | Keep `decomp-status` invalid records and target ordering machine-readable. | `test_decomp_status.py` |
| 1.2 | complete | Keep canonical symbol-map normalization and source/binding drift checks covered. | `test_canonical_symbols.py`; `bin/symbols check` |
| 1.3 | complete | Record the live health baseline rather than carrying the old lift-count snapshot forward. | Commands in Scope and evidence baseline |
| 1.4 | complete | Profiled the full audit: `bin/decomp-status --json --detail minimal` exceeded a 300-second bound (exit 124, no JSON). No new mode was needed: existing positional target selection already provides the bounded aggregate (`bin/decomp-status TARGET --json --detail minimal`) without changing report semantics; battle/15 completed in 20.87 seconds (exit 2 only for known invalid lifts) and emitted valid aggregate JSON. | Full timeout evidence plus target-scoped valid JSON baseline |

**Exit gate:** the current baseline, including known failures, is reproducible
and does not rely on obsolete numerical totals.

## Phase 2 — Repair source, map, and Splat ownership **[active]**

**Evidence needed before each target:** target manifest, original image,
reviewed Splat boundaries, target-local map, and `bin/symbols check` output.

| ID | Status | Work | Validation |
| --- | --- | --- | --- |
| 2.1 | complete | Add a target selector to `symbols check` only if the existing command cannot diagnose one target without unrelated output. Reuse canonical validation; do not create a parallel checker. | Test+lint passed: pytest 13/13, ruff clean. Scoped `bin/symbols check emi/battle/battle/15` isolates battle/15 only (exit 2). Full `bin/symbols check` remains exit 2 with all-target baseline (battle/03, battle/15, shop/00, SLUS; game/01 was normalized in Phase 2.6). Unknown-target `exe/no_such_target` uses standard error path (exit 2). `git diff --check` clean. |
| 2.2 | active | Reconcile battle/15 source-to-map ownership one reviewed address at a time. Promoted only `func_8009E1E0`, `func_8009E7C4`, and `func_8009DE50` after manifest load `0x80096800`, reviewed offsets `0x79E0`, `0x7FC4`, and `0x7650` (= address minus load), original exact-match evidence, and required `@behavior` metadata; coupled each raw local-map entry with only its unchanged-boundary Splat `asm`→`c` classification. DE50's original five instructions/20 bytes require a signed `s16` store of `-20` at byte offset 4 through the existing `D_801463A0` pointer value. | Scoped check now reports the remaining 34 drifts only (expected exit 2); `bin/splat emi/battle/battle/15` passed; DE50 full asm diff and byte match passed (5/5 instructions, 20/20 bytes); target status reports DE50 exact (`exact=11`, `invalid=60`); `git diff --check` clean |
| 2.3 | pending | Reconcile shop/00 source-to-map ownership one reviewed address at a time. | Same as 2.2 |
| 2.4 | pending | Reconcile battle/03 source-to-map ownership one reviewed address at a time. | Same as 2.2 |
| 2.5 | pending | Repair the four reported SLUS source/map drifts and the `D_80143C30` binding/map disagreement at the owning declaration/map. | `bin/symbols check`; target build/match checks where applicable |
| 2.6 | complete | Normalize the reported game/01 map only through `bin/symbols normalize emi/etc/game/01 --write`; inspect the resulting diff (pure address-sort reorder: GAME_FRONT_POPUP_WORD moved before GAME_FRONT_FADE_PHASE). | `bin/symbols check emi/etc/game/01` → passed; canonical-symbols tests 5/5; SDK tests 7/7; ruff clean; `git diff --check` clean; diff is 1 insertion/1 deletion, semantic-free |
| 2.7 | pending | Resolve invalid lift metadata in priority order from a fresh target-scoped decomp-status report. | `bin/decomp-status <TARGET>`; source validation |
| 2.8 | pending | Correct a disputed Splat boundary only after original bytes and `t_addr` prove the reviewed layout is wrong. | Splat review plus target-local symbols and matching validation |

**Non-goals:** bulk-generating map entries from filenames; semantic renaming;
or changing an exact lift without revalidating it.

**Exit gate:** `bin/symbols check` has no source/map, normalization, or
binding/map drift; invalid lifts are only explicitly reviewed exceptions, if
any.

## Phase 3 — Verify analysis inventory and candidate selection **[partially implemented]**

The existing code already validates Rizin snapshot identity, rejects non-code,
data, pointer-table, and SDK candidates in `rev-query`, and has focused tests.
Do not reimplement those filters.

| ID | Status | Work | Validation |
| --- | --- | --- | --- |
| 3.1 | complete | Preserve target-qualified snapshot identity and atomic index rebuild behavior. | `test_reverse_index.py` |
| 3.2 | complete | Preserve candidate exclusion for noncanonical boundaries, printable data, pointer tables, and shared SDK symbols. | `test_rev_query.py` |
| 3.3 | pending | Make exclusion reasons inspectable in the supported ranking output or a diagnostic command; do not silently add an alternate index. | Focused CLI/output test using fixture rows |
| 3.4 | pending | Add a reviewed-boundary conflict report only if live snapshots demonstrate an unresolved analyzer/root mismatch. | Fixture test plus one live target evidence sample |
| 3.5 | pending | Document and enforce the rebuild sequence after committed map/Splat changes: snapshot refresh, index rebuild, then ranking. | Command integration test or documented checked workflow |

**Exit gate:** ranking offers only target-qualified, reviewed code candidates and
can explain why analyzer-only roots were excluded.

## Phase 4 — Reduce the unmatched lift backlog **[blocked by Phase 2]**

No batch lifting occurs while ownership drift remains. Each candidate is an
independent `TARGET@0xADDRESS` task, not a shared implementation.

| ID | Status | Work | Validation |
| --- | --- | --- | --- |
| 4.1 | pending | Rebuild snapshots/index and re-rank only after a clean target ownership batch. | `just index`; target-qualified `bin/rev-query` |
| 4.2 | pending | Lift one candidate through reviewed C, `bin/asm-diff`, `bin/byte-match`, and independent review. | Exact byte match and review evidence |
| 4.3 | pending | Use compiler-flag search before matching aids for clean-C residuals. | Flag evidence and unchanged behavior |
| 4.4 | pending | Use one bounded permuter run only after control flow, ABI, and types are evidenced. | Recorded workspace/result; exact-match validation |
| 4.5 | pending | Promote duplicate bodies only after two independently loaded members byte-match. | Two target-local byte matches; shared-body review |
| 4.6 | pending | Add stable field-layout or matching lessons only after independent exact-match evidence. | `docs/specs/` or `LESSONS.md` review |

**Exit gate:** lift progress is measured by live exact/partial/invalid status,
not filename counts; no unreviewed cross-target sharing exists.

## Phase 5 — Keep managed tooling, media, and audio reliable **[active; independent of Phase 2]**

### 5A — Toolchain lifecycle ownership

Current evidence: doctor passes all registered tools. Splat, spimdisasm,
asm-differ, and decomp-permuter recently received local-source lifecycle work.
The tables below classify every externally callable `bin/` wrapper into one of
four categories, with the retention rationale for each.

### Toolchain-owned executables (install/verify/invoke owners)

| Toolchain | Install source | Owned executable/invocation | Wrapper decision |
| --- | --- | --- | --- |
| GCC / PSn00b | pinned archives | compiler and bin directories | retain compiler/linker adapters (`cc`, `as`, `ld`, etc.) |
| PsyQ / disc | authorized archive or private input | libraries/media, not a CLI | no launcher wrapper |
| Rizin | pinned archive | `toolchains/rizin/bin/rizin` | owner-dispatched wrapper implemented |
| maspsx | `third_party/maspsx` | script through project Python | owner-dispatched wrapper implemented; `cc` stays a build adapter |
| m2c / asm-differ | pinned submodules | m2c script / installed console entry point | `bin/m2c` and `bin/asm-diff` remain target-qualified lifting workflows |
| decomp-permuter | pinned submodule + `toml` | script through project Python, source cwd with `-u` unbuffered flag | coordinator extracted into `harness.commands.permute`; final invocation delegates to `DecompPermuterToolchain.execute()` |
| splat | pinned submodule installed into `.venv` | `.venv/bin/splat` | command already resolves the owned executable; wrapper only bootstraps harness |
| spimdisasm | pinned submodule installed into `.venv` | `.venv/bin/spimdisasm` | owner-dispatched wrapper implemented |
| PsyQ signatures | submodule plus `bin/symbols` | repository command, not independent external CLI | retain repository command path |

### Full bin/ wrapper classification (35 wrappers)

| Wrapper | Classification | Owner / rationale |
| --- | --- | --- |
| `rizin` | raw dispatcher | `harness.commands.tool` — thin shell bootstrap for RizinToolchain |
| `maspsx` | raw dispatcher | `harness.commands.tool` — thin shell bootstrap for MaspsxToolchain |
| `spimdisasm` | raw dispatcher | `harness.commands.tool` — thin shell bootstrap for SpimdisasmToolchain |
| `splat` | target-qualified workflow | delegates to `harness.commands.splat`; uses `SplatToolchain.execute()`; resolves target and manifest |
| `m2c` | target-qualified workflow | delegates to `harness.commands.lift.run_m2c`; maintains target context, flags, assembly resolution |
| `asm-diff` | target-qualified workflow | delegates to `harness.commands.lift.run_asm_diff`; target-qualified diff with outputs bundling |
| `byte-match` | target-qualified workflow | delegates to `harness.commands.lift.run_byte_match`; target-qualified bytes-only comparison |
| `promote` | target-qualified workflow | delegates to `harness.commands.lift.run_promote`; validates clang-format, runs match, reports acceptance |
| `m2ctx` | target-qualified workflow | delegates to `harness.commands.lift.run_m2ctx`; generates target-local context from symbols |
| `rz-project` | target-qualified workflow | delegates to `harness.commands.rizin_project`; target-qualified Rizin project lifecycle |
| `cc` | build adapter | compiler environment and maspsx integration; retains gcc setup, assembler selection, maspsx pipeline |
| `as` | build adapter | thin `$PSX_AS` redirect to PSn00b toolchain bin |
| `ld` | build adapter | thin `$PSX_LD` redirect to PSn00b toolchain bin |
| `ar` | build adapter | thin `$PSX_AR` redirect to PSn00b toolchain bin |
| `nm` | build adapter | thin `$PSX_NM` redirect to PSn00b toolchain bin |
| `objcopy` | build adapter | thin `$PSX_OBJCOPY` redirect to PSn00b toolchain bin |
| `objdump` | build adapter | thin `$PSX_OBJDUMP` redirect to PSn00b toolchain bin |
| `ranlib` | build adapter | thin `$PSX_RANLIB` redirect to PSn00b toolchain bin |
| `strip` | build adapter | thin `$PSX_STRIP` redirect to PSn00b toolchain bin |
| `decomp-status` | repository command | `harness.commands.decomp_status`; live matching report; not an external CLI launcher |
| `rev-query` | repository command | `harness.commands.rev_query`; reverse-index query and ranking; not an external CLI launcher |
| `symbols` | repository command | `harness.commands.symbols`; map normalization, drift check, binding generation |
| `flag-search` | repository command | `harness.commands.flag_search`; compiler-flag search utility |
| `index` | repository command | `harness.commands.rebuild_index`; snapshot rebuild and reverse-index refresh |
| `harness` | repository command | narrow entry for `harness.commands.psyq` (PsyQ signature scan/calls/proposal) |
| `str-media` | repository command | `harness.commands.str_media`; string/media extraction utility |
| `permute` | raw dispatcher | thin shell bootstrap for `harness.commands.permute`; coordinator extracted into harness command |
| `psx-audio` | repository command | thin exec to `tools/c/psx-audio/psx-audio` compiled binary; not a managed toolchain |
| `psx-audio-bin` | repository command | compiled ELF binary tracked in repository; direct exec |
| `package-psx-audio` | repository command | shell packaging script for psx-audio artifact |
| `psyq-import` | repository command | bootstrap for `harness.commands.psyq_import` |
| `emi-ex` | repository command | bootstrap for the EMI extraction binary; retained as is |
| `emi-target` | repository command | bootstrap for `harness.commands.emi_target` |
| `bof3-disk` | repository command | bootstrap for `harness.commands.disc`; media extraction/checksum workflow |
| `build` | repository command | bootstrap for CMake configure and build; build-system orchestration |

| ID | Status | Work | Validation |
| --- | --- | --- | --- |
| 5.1 | complete | Keep setup and doctor independently exercising registered toolchains. | `harness.commands.doctor --root .` |
| 5.2 | complete | Inventory every `tools/python/harness/toolchain/*.py`, its setup/doctor registration, executable, install source, verification command, required cwd, and environment. The reviewed inventory distinguishes true toolchain launchers from build adapters and workflow dispatchers. | Inventory above; `harness.commands.doctor --root .` |
| 5.3 | complete | Define the minimum common API for managed execution: executable path, working directory, environment overlay, quiet verification, and the `execute` contract. `ExecutableToolchain` supplies owned `invocation`/`execute` hooks with `quiet` mode for silent verification. M2c, asm-differ, permuter, and maspsx all use this contract. | Focused lifecycle and contract tests; doctor is quiet (`0` leaked help lines) |
| 5.4 | active | Move duplicated project-Python selection, `PYTHONPATH`, third-party paths, and executable construction from eligible `bin/` wrappers into their owning toolchains. `rizin`, `maspsx`, `spimdisasm` now dispatch through `harness.commands.tool`; `commands/splat` now delegates to `SplatToolchain.execute()`. Retain workflow wrappers until their command-layer ownership is extracted. | Wrapper tests; direct toolchain invocation; doctor |
| 5.5 | active | Convert eligible wrappers incrementally, beginning with `splat`, `spimdisasm`, `asm-diff`, `m2c`, and `permute`; preserve public CLI behavior and error messages. `spimdisasm`, `splat`, and `permute` commands are complete. `run_m2c` delegates through `M2cToolchain.execute()`. `asm-diff` remains a target-qualified workflow wrapper. | Per-wrapper `--help`/example checks plus focused tests |
| 5.6 | complete | Audit non-Python wrappers (`as`, `cc`, `ld`, `maspsx`, Rizin, PSn00b, PsyQ, media tools) and centralize only behavior already owned by a corresponding toolchain. Every wrapper is classified as raw dispatcher, target-qualified workflow, build adapter, or repository command in the Phase 5 inventory. | Evidence-backed inventory; `git diff --check` |
| 5.7 | complete | Make setup and doctor consume the same owned verification contract where practical, without hiding individual tool failures. Added a minimal `_managed_toolchains()` factory shared by setup and doctor that preserves per-tool failure names and independent task rendering. | Factory-order assertion; doctor failure isolation test; 46-suite focused test pass; doctor `5/5` |

### 5B — Disc and audio contracts

| ID | Status | Work | Validation |
| --- | --- | --- | --- |
| 5.8 | pending | Determine whether any supported workflow requires `bof3-disk rebuild`; it currently returns “rebuild parity is not implemented yet.” Implement parity only if that contract is required, otherwise remove it from advertised supported behavior. | Command-contract test and README/usage update |
| 5.9 | pending | Keep media inventory/extraction tests free of proprietary inputs and verify target/image boundaries. | Focused fixture tests; no `inputs/` changes |
| 5.10 | pending | Audit PSX audio decode/render/export coverage and update the format specification only for verified behavior. | Native tests and spec review |

**Exit gate:** every managed executable is installed, resolved, invoked, and
verified by its owner; wrappers contain no duplicate environment/path policy;
doctor remains independently diagnosable.

## Phase 6 — Repository health and documentation closure **[pending]**

| ID | Status | Work | Validation |
| --- | --- | --- | --- |
| 6.1 | pending | Run `just check` after Phase 2 repairs and report every failing gate separately. Do not skip source/map validation. | `just check` |
| 6.2 | pending | Keep Python lint/tests, source validation, map validation, and exact-match checks independently actionable. | Each command exits independently with focused diagnostics |
| 6.3 | pending | Link active plans from `docs/index.md` only if that index is the repository’s established planning index; otherwise avoid a redundant navigation layer. | Documentation review |
| 6.4 | pending | On each completed phase, replace stale baselines, mark tasks, and remove superseded tasks. | Plan review with command evidence |

## Acceptance criteria

- [ ] Baselines are current, reproducible, and do not retain obsolete lift counts.
- [ ] `bin/symbols check` is clean without suppressions.
- [ ] Candidate ranking excludes non-code and shared SDK bodies with inspectable reasons.
- [ ] Every promoted lift has target-qualified asm-diff, byte-match, and review evidence.
- [ ] Each managed toolchain owns its executable, environment, invocation, and verification; eligible wrappers are thin dispatchers.
- [ ] `harness.commands.doctor --root .` remains fully passing after toolchain refactors.
- [ ] `just check` passes once ownership drift is repaired.
- [ ] No private media, installed dependencies, `out/`, or build products are committed.
