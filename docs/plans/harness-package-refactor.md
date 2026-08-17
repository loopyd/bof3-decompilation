# Harness package refactor

## Goal and evidence baseline

Organize `tools/python/harness/` by responsibility, make command modules thin,
clarify names and API ownership, and simplify genuine toolchain polymorphism
without turning stateless transformations into class hierarchies.

Baseline:

- The preceding harness DRY/registry work is present in the current uncommitted
  worktree and must be preserved.
- The C declaration resolver has already moved from
  `harness/c_context.py` to `harness/domain/c_context.py`; this plan retains
  that destination and does not create a separate `context/` package.
- Focused scratchpad tests pass (16 tests), Ruff passes for the moved resolver,
  and `git diff --check` is clean.
- Root modules still mix analysis, build, decompilation, and domain concerns;
  several command-private modules contain reusable application algorithms.
- Existing inheritance is concentrated in `toolchain/`; pure parsing,
  formatting, graph, validation, and discovery functions should remain
  functions.

## Phase 1 — naming and API ownership

- [x] Resolve same-name/different-semantics collisions, especially the local
      `commands/_lift_m2c.resolve_function` versus
      `domain.registry.resolve_function`: the command tuple adapter is renamed
      `resolve_function_selector`; registry keeps `resolve_function`. All five
      command callers and the three test files moved atomically.
- [x] Rename ambiguous public classes and methods where repository-wide callers
      prove an atomic rename is safe: `Symbol` -> `MapSymbol`,
      `LayoutBoundary` -> `SplatBoundary`, `ReviewedLayout` ->
      `ReviewedSplatLayout`, `TargetSnapshot` -> `AnalysisSnapshot`,
      `RizinTarget` -> `RizinProjectSpec`, and `StatusCache` ->
      `MatchStatusCache`; every production and test importer moved atomically
      with no forwarding shims.
- [x] Make verb-led operation names and noun properties consistent:
      `boundary_containing` -> `find_containing_boundary`,
      `boundary_starting_at` -> `find_boundary_at`, and `NamingDebt.rows()` ->
      `to_rows()`; immutable derived accessors already use properties, so no
      further conversion was warranted.
- [x] Add a small import-layer/cycle smoke test and public import smoke coverage
      before package moves: `test_harness_imports_resolve_and_are_acyclic`
      imports every harness module and proves the module-level import graph is
      a DAG; `test_no_semantic_name_collisions` now locks each renamed symbol
      to its owning module.
- [x] Remove the unused `domain.c_context._name` compatibility shim (no
      callers remain); `_names` is the sole live declaration-name resolver.
- [x] Extend the cycle guard to map every `__init__.py` to its package name and
      include those nodes and edges in the DAG (Phase 1 tree: 103 modules, 281
      edges, including all ten package-initializer edges
      (assets->str_media, the five `domain.*` re-exports, emi->operations, the
      two `psyq.*` re-exports, and toolchain->io).
      `test_package_initializer_edges_are_locked` locks those edges and
      asserts no initializer edge is silently dropped. Final closure tree
      totals are recorded in the Phase 5 closure audit below and supersede
      this phase record.
- [x] Ruff-format every touched Python path. Formatter normalization expands
      multiline collection literals, which pushed `reverse_index.py` past the
      450-line decomposition seal; extracted the pure SQL DDL to
      `harness/reverse_schema.py` (`create_schema`) with callers in
      `reverse_index.py` and `test_rev_query.py` updated atomically.

Acceptance:

- One concept has one name across production code.
- No compatibility aliases or forwarding modules are introduced for internal
  imports; all repository callers move atomically.
- Focused domain, match, analysis, toolchain, and CLI tests pass.

Validation evidence (Phase 1): full Python suite 415 passed / 1 pre-existing
`.pi` compactness-policy failure unchanged from HEAD; 14 affected focused
suites 205 passed; Ruff lint clean and `ruff format --check` clean on every
touched path; all 33 command modules import; `bin/symbols`, `bin/m2ctx`,
`bin/rz-project`, `bin/decomp-status`, `bin/rev-query`, and
`bin/companion-check --help` exit 0; `bin/scratchpad preview`
`emi/world00/area008/13@0x801F3D88` exits 0 with registry identity preserved;
corrected import-edge audit (Phase 1 tree) = 103 nodes/281 edges/10
initializer edges, currently acyclic; final closure tree totals supersede
this phase record; `git diff --check` clean; no staged files; `.pi` has zero
diff.

## Phase 2 — root package organization

- [x] Create `analysis/` and move analysis engine, Rizin project, snapshot, and
      reverse-index implementation there with responsibility-specific names:
      `harness/analysis/engine.py` (was `analyzer.py`), `project.py` (was
      `rizin_project.py`), `snapshot.py`, `index.py` (was `reverse_index.py`),
      and `schema.py` (was `reverse_schema.py`).
- [x] Create `build/` and move build operations, compiler configuration, and
      binary normalization there: `harness/build/operations.py` (was
      `build.py`), `compiler.py` (was `compiler_config.py`), and
      `binaries.py` (closure correction: `build/binaries.py` was removed as a
      true orphan in Phase 5 — no importer, caller, `bin/*` entrypoint, or
      focused test at HEAD or in the refactor).
- [x] Create `decomp/` and move decompilation status and preflight/worklist
      implementation there: `harness/decomp/status.py` (was `decomp_status.py`)
      and `preflight.py` (was `decomp_status_preflight.py`).
- [x] Move reviewed layout and canonical symbol-map ownership into `domain/`:
      `harness/domain/layout.py` (was `layout.py`) and
      `harness/domain/symbols.py` (was `canonical.py`), and consolidate the
      small root `symbols.py` (weak `WEAK_SYMBOL_AT` binding parsing) into the
      same symbol domain module because its live API belongs there.
- [x] Keep C parsing at `domain/c_context.py`; do not create a `context/`
      package.
- [x] Reduce root modules to genuinely shared infrastructure: only
      `discovery.py`, `io.py`, and `output.py` remain at the root; every
      application/domain module moved into its owning package.
- [x] Update all production, tests, and `bin/*` import targets atomically and
      remove old internal modules rather than retaining facades: 13 tracked
      harness-root modules deleted (15 harness modules total when the two
      non-root moves `commands/_symbols_psyq.py` and `match/asm_differ.py`
      from the prior phase are included), 3 new packages plus domain moves
      created, all 29 production importers and 16 test files updated with no
      forwarding shims; `bin/*` wrappers dispatch through
      `harness.commands.*` and needed no change.

Acceptance:

- Root files have explicit cross-cutting responsibility.
- No import cycles violate `commands -> application packages -> domain /
  toolchain` layering.
- All documented `bin/*` commands import and preserve help/exit contracts.

Validation evidence (Phase 2): focused suites 193 passed; full Python suite
415 passed / 1 pre-existing `.pi` compactness-policy failure unchanged from
HEAD (`.pi` zero diff); Ruff lint clean and `ruff format` clean on every
Phase 2 touched path (7 files reformatted; pre-existing unformatted
 `compiler_variants.py` left untouched; `sync_lift_metadata.py` was later
 removed with its orphan module in Phase 5 closure); import graph
still acyclic with the three new empty package initializers locked by
`test_package_initializer_edges_are_locked` and renamed-symbol owners moved in
`test_no_semantic_name_collisions`; `bin/symbols`, `bin/scratchpad`,
`bin/rz-project`, `bin/decomp-status`, `bin/rev-query`, and
`bin/companion-check --help` exit 0; `bin/scratchpad preview`
`emi/world00/area008/13@0x801F3D88` exits 0 with registry identity
(`diff_label: func_801F3D88`) and dependency-closed context preserved;
`bin/symbols check` remains baseline-red with byte-identical naming-debt
output; `test-skill-scripts.py` remains baseline-red on the unchanged
`agent-context.py` output ceiling; `git diff --check` clean; no staged files.

## Phase 3 — thin command adapters

- [x] Move reverse-query graph, mission, and priority algorithms from
      `commands/_rev_query_*` into the analysis package:
      `analysis/graph.py` (`function_metrics`, `sccs`, `enrich_graph`,
      `dominates`), `analysis/mission.py` (`mission_brief`), and
      `analysis/priority.py` (`candidate_context`, `candidate_exclusion`,
      `priority_rows`, `RANK_FIELDS`). The three `commands/_rev_query_*`
      modules were deleted; `commands/rev_query.py` absorbs the `mission`
      handler and `_print_mission` presentation, adapts parsed arguments to
      explicit keyword parameters (`priority_rows` takes no `argparse`
      namespace), and drops the `_root` re-export for `resolved_root`.
- [x] Move reusable m2c preparation/context behavior out of
      `commands/_lift_m2c.py` into the owning decomp/toolchain layer:
      `splat_assembly`, `context_path`, `render_context`, and `asm_label`
      moved to `toolchain/m2c.py`; `resolve_function_selector` (shared with
      lift/build/flag-search/permute) moved to `commands/_common.py`;
      `commands/_lift_m2c.py` now contains only the `run_m2ctx`/`run_m2c`
      handlers.
- [x] Move PsyQ binding operations from `commands/symbols_psyq.py` into
      `psyq/bindings.py` (`parse_psyq_find`, `select_import_rows`,
      `apply_psyq_provenance`, `sdk_weak_bindings`, `sdk_references`),
      leaving CLI normalization/rendering in commands; the shared-base dedupe
      algorithm moved to `domain/symbols.dedupe_shared_symbols` (its owner),
      and `commands/symbols_psyq.py` keeps only thin handlers plus
      `_root`/`_targets` command helpers.
- [x] Move source validation and metadata synchronization into domain
      modules: `sync_lift_metadata` moved to `domain/source_sync.py` and the
      empty `commands/sync_lift_metadata.py` deleted (no bin entry or
      production callers); `commands/validate_sources.py` was already thin
      (delegates to `decomp.status.build_report`/`write_report`) and needed
      no change. (Closure correction: `domain/source_sync.py` and its test
      were later removed as a true orphan in Phase 5 — the sync command had
      no bin entry or production callers at HEAD, so the move preserved a
      dead module.)
- [x] Move companion-report, data-scan, and EMI target application logic into
      their owning domain/application packages where reuse and tests confirm
      the seam: `commands/companion_check.py` report builder moved to
      `emi/companions.py` (`build_companion_report` plus its private
      helpers, alongside the existing `catalog_verify` companion evidence
      layer); `commands/data_scan.py` clustering moved to
      `analysis/data.py` (`collect_unlabeled_regions`);
      `commands/emi_target.py` was already thin (delegates to
      `emi.catalog_bootstrap`) and needed no change.
- [x] Leave each command module responsible only for parser construction,
      argument normalization, presentation, and exit-code mapping: the moved
      algorithms no longer import `argparse` or `commands`, and the command
      handlers above now only adapt arguments, print, and map exit codes.

Acceptance:

- Domain/application packages do not import `commands`: verified by the
  module-level import DAG test and by direct inspection of the new modules.
- Command-private modules remain only for command-specific parsing or rendering.
- Algorithms retain focused unit coverage outside CLI-only tests: rev-query
  graph/priority/mission tests now import from `analysis.*`; emi companion
  tests import `emi.companions` (sync tests were removed with the orphaned
  `domain.source_sync` module in Phase 5 closure).

Validation evidence (Phase 3): focused affected suites 139 passed (rev-query,
  lift, domain-sources, permute, harness-dry, sync, emi-catalog, psyq,
  sdk-symbols, analysis-sequence, reverse-index); full Python suite 415
  passed / 1 pre-existing `.pi` compactness-policy failure unchanged from
  HEAD (`.pi` zero diff); Ruff lint clean and `ruff format` clean on every
  touched path (pre-existing unformatted files untouched); harness import
  graph still acyclic; `bin/rev-query status|mission|quick-wins`,
  `bin/symbols psyq-report|psyq-bindings|dedupe`,
  `bin/scratchpad preview emi/world00/area008/13@0x801F3D88` (registry
  identity `func_801F3D88` preserved), and `--help` for companion-check,
  data-scan, emi-target, permute, build, and flag-search all exit 0;
  `git diff --check` clean; no staged files.

## Phase 4 — disciplined class simplification

- [x] Audit concrete toolchain subclasses for repeated constructors,
      executable/Python/cwd/environment resolution, verification, and subprocess
      setup: `m2c.py`, `maspsx.py`, `asm_differ.py`, and `permuter.py` each
      repeated the python-script invocation, submodule-update install, and
      `--help` verify bodies; `SplatToolchain`/`SpimdisasmToolchain` were
      already declarative.
- [x] Move identical lifecycle behavior into the existing shallow toolchain
      bases; make concrete wrappers declarative with class attributes and
      minimal necessary overrides: `base.py` gained `pip_packages`,
      `verify_arguments`, and a new `PythonScriptSubmoduleToolchain` (script
      executable under the submodule, interpreter invocation, `--help`
      verification); `M2cToolchain` shrank to label/submodule/script,
      `MaspsxToolchain` and `DecompPermuterToolchain` to declarative
      attributes plus one `working_directory -> source` override each, and
      `AsmDifferToolchain` to attributes plus its existing
      `working_directory`/`executable` overrides. `install_target` is now
      optional so script submodules skip pip entirely.
- [x] Prefer immutable command/result value objects and composition over deeper
      inheritance where tool lifecycles differ: the shared `Check[T]` frozen
      dataclass record replaces the identical `SetupTask`/`DoctorTask`
      records; each command keeps its own ordered task list, state type, and
      loop policy (setup re-raises, doctor counts) unchanged.
- [x] Consolidate setup/doctor result records and rendering only where behavior
      is identical; do not create an abstract command hierarchy: the identical
      record shape and register decorator moved to
      `commands/_common.py` (`Check`, `register_check`); the identical
      per-command `_render` helpers in `setup.py`/`doctor.py` were
      consolidated into the shared `render_task` in `commands/_common.py`
      (added here) and the duplicates removed; no abstract command base was
      introduced.
- [x] Add a common source-resolution error base only if current callers have a
      demonstrated shared catch boundary: not applicable — every caller
      catches `LiftMetadataError`, `SourceAddressCollision`, and
      `CompiledSymbolError` individually (`claims.py` catches only
      `LiftMetadataError`), so no shared catch boundary exists to serve.
- [x] Use `Protocol` only at an existing polymorphic consumer seam; do not add a
      protocol or base class for one implementation: not applicable — the
      toolchain registry and `managed_toolchains()` already consume concrete
      types duck-typed on `label`/`verify`/`run`, and no polymorphic consumer
      seam exists that needs a `Protocol`.

Acceptance:

- Concrete toolchains contain no identical boilerplate overrides.
- No inheritance exists solely for naming symmetry.
- Existing tool invocation, verification, and error behavior is preserved.

Validation evidence (Phase 4): focused suites (`test_python_cli_toolchains`,
`test_permute`, `test_lift`, `test_setup`, `test_doctor`, `test_tool_command`,
`test_splat`, `test_target_manifest_consumers`, `test_harness_dry`) pass;
full Python suite 415 passed / 1 pre-existing `.pi` compactness-policy failure
unchanged from HEAD (`.pi` zero diff); Ruff lint clean and `ruff format` clean
on every touched path; harness import graph still acyclic; setup and doctor
are Python module entrypoints with no `bin/setup` or `bin/doctor` wrapper, so
`PYTHONPATH=tools/python .venv/bin/python -m harness.commands.setup --help` and
`PYTHONPATH=tools/python .venv/bin/python -m harness.commands.doctor --help`
exit 0; `bin/maspsx --help`, `bin/m2ctx --help`, `bin/permute --help` exit 0;
`bin/scratchpad preview emi/world00/area008/13@0x801F3D88` exits 0 with
registry identity preserved; `git diff --check` clean; no staged files.

## Phase 5 — final package cleanup

- [x] Rename `assets/` to `media/` because it contains executable STR media
      operations rather than passive package assets: `harness/media/` now
      holds `str_media.py` (inspect/validate/convert); the only two consumers
      (`commands/str_media.py` import and the `test_harness_dry` initializer
      edge lock) moved atomically; git sees `D assets/` + `?? media/` with no
      staging performed.
- [x] Split large mixed-responsibility modules only when the extracted unit is
      independently named, tested, or reused; avoid cosmetic one-use modules:
      audit found no justified split. `media/str_media.py` (368 lines) has no
      functional test and no external reuse of its internal sector/probe
      helpers — extracting them would create untested one-use modules;
      `analysis/engine.py` (400) and `domain/manifests.py` (418) are
      unit-tested as whole modules with no independently named/reused seam.
- [x] Narrow every package `__init__.py` to intentional stable exports:
      removed the provably dead re-exports `emi_pack` (zero callers
      repo-wide) from `emi/__init__.py` and `function_fingerprint` /
      `scan_payload` (zero callers repo-wide) from `psyq/__init__.py`;
      retained the live re-exports (`emi_unpack`; `index_headers`,
      `parse_headers`, `relocation_masked_hash`), preserving the locked
      initializer edges `emi -> operations` and `psyq -> {fingerprints,
      headers}`. Every other package initializer already exports only its
      intentional stable API (domain re-exports consumed via package path;
      toolchain registry is the ordered managed-toolchain factory;
      analysis/build/decomp/commands/match are package markers).
- [x] Remove empty, obsolete, and superseded private modules after an import
      and `bin/*` entrypoint audit: liveness is established by a production
      importer, a `bin/*`/`just` dispatch, or a focused behavioral test —
      import-smoke alone (importing every module to prove the DAG) is not
      liveness evidence. Under that rule two true orphans were removed:
      `build/binaries.py` (dead at HEAD and in the refactor: no repository
      importer, caller, `bin/*` entrypoint, or focused test) and
      `domain/source_sync.py` (referenced only by its own test; the former
      `commands/sync_lift_metadata.py` already had no `bin` entry or
      production callers at HEAD, so the module never had a real consumer;
      its test `test_sync_lift_metadata.py` was removed with it). All other
      harness modules are live by relative import, command dispatch, or
      focused test; the empty `__init__.py` files
      (analysis/build/commands/decomp/match) are intentional package markers.
      The three dead functions above remain defined in their public modules
      (module-level API kept; removal is function-level and out of this
      item's module scope).
- [x] Reorganize tests only where module moves or new algorithm seams require
      it: only `test_harness_dry.py`'s locked initializer-edge map was updated
      (`harness.assets` -> `harness.media`); no test files moved, and the
      `test_psyq`/`test_emi_catalog`/`test_psyq_headers` suites needed no
      change because they never import the dead names.
- [x] Record the final annotated harness tree and import-layer audit in this
      plan's closure evidence: see below.

Acceptance:

- The final tree expresses analysis, build, decomp, domain, EMI, match, media,
  PsyQ, commands, and toolchain responsibilities directly: the root now holds
  only `discovery.py`, `io.py`, and `output.py`; all application/domain
  modules live in their owning packages.
- No compatibility shims, speculative abstractions, generated files, or dead
  modules remain: dead re-exports removed; no facades introduced.
- Independent closure review reports every phase complete.

Validation evidence (Phase 5): focused suites (`test_harness_dry`,
  `test_psyq`, `test_emi_catalog`, `test_psyq_headers`) 26 passed; full Python
  suite 413 passed / 1 pre-existing `.pi` compactness-policy failure unchanged
  from HEAD (`.pi` zero diff); Ruff lint clean and `ruff format --check` clean
  on every touched path; import smoke of every touched package passes;
  `bin/str-media --help` exit 0; `bin/scratchpad preview
  emi/world00/area008/13@0x801F3D88` exit 0 with registry identity
  `func_801F3D88` and dependency-closed context preserved; `bin/symbols
  check` baseline-red with byte-identical output (SHA-256
  `81a188db711ea8f078f0d0bd5bc421ea212b432f38b316bfa7d1fc6401cfe895`, 19,560
  bytes) and `config`/`src`/`include` zero diff; `test-skill-scripts.py`
  baseline-red on the unchanged `agent-context.py` output ceiling;
  `git diff --check` clean; no staged files.

### Final annotated harness tree (Phase 5 closure)

```text
tools/python/harness/           # BOF3 Python harness
├── analysis/                   # Rizin engine, project replay, snapshots, index, graph, mission, priority, data
├── build/                      # CMake operations and compiler configuration
├── commands/                   # thin CLI adapters (parser, normalization, rendering, exit codes)
├── decomp/                     # lift status auditing and preflight worklists
├── domain/                     # identity, manifests, claims, registry, sources, symbols, layout, C context, naming, tags
├── emi/                        # EMI catalog, verification, companion reports, pack/unpack operations
├── match/                      # assembly/byte-match engine (asm-diff, disasm, link, resolve, bundle, flag-search)
├── media/                      # executable STR media inspection/validation/conversion (was assets/)
├── psyq/                       # PsyQ signatures, headers, fingerprints, bindings
├── toolchain/                  # managed external toolchain adapters and the ordered toolchain registry
├── discovery.py                # shared file discovery/hashing
├── io.py                       # repository paths and JSON I/O
└── output.py                   # CLI detail-level helpers
```

Import-layer audit (Phase 5 closure, refreshed after orphan removal): 106
modules (incl. all package initializers), 289 edges, acyclic, measured with
the checked-in `test_harness_dry` graph helper on the final tree (108
modules/291 edges before removing the two orphans); initializer edges locked
by `test_package_initializer_edges_are_locked` (now `media -> str_media`;
`emi -> operations`; `psyq -> {fingerprints, headers}`);
`test_harness_imports_resolve_and_are_acyclic` imports every harness module
as an import-resolution check only — it is not dead-code liveness evidence.
Layering holds: `commands -> application packages -> domain / toolchain`;
`domain/` and toolchain adapters never import `commands`.

## Closure corrections (independent review)

Four findings from the final closure review were fixed:

1. **Stale import-audit totals.** The closure audit previously claimed 103
   modules/281 edges. Refreshed from the current tree with the checked-in
    `test_harness_dry` helper: 108 modules/291 edges before orphan removal,
    106 modules/289 edges after. Earlier phase records are labeled as
   phase-tree snapshots and are superseded by this closure record.
2. **Circular dead-module audit.** Liveness is now defined as a production
   importer, a `bin/*`/`just` dispatch, or a focused behavioral test — the
   import-smoke test is explicitly not liveness evidence. Two true orphans
   were removed under that rule: `build/binaries.py` (no importer, caller,
   `bin/*` entrypoint, or focused test at HEAD or in the refactor) and
   `domain/source_sync.py` with its test `test_sync_lift_metadata.py` (the
   former `commands/sync_lift_metadata.py` had no `bin` entry or production
   callers at HEAD, so the module never had a real consumer).
3. **Deleted unrelated plans restored.** `agent-skill-compaction.md` and
   `project-symbol-naming-cleanup.md` have active/incomplete work and were
   restored from HEAD; the completed historical plans
   (`flat-reverse-snapshots.md`, `harness-consistency-cleanup.md`,
   `source-tree-classification-cleanup.md`) were also restored so the package
   refactor does not erase project state. This plan supersedes only the
   harness-package-refactor scope; the other plans remain authoritative for
   their own efforts.
4. **Support-only build targets no-op regression (fixed).**
   `commands/build.py` had switched its target-build gate from the
   claim-aware `manifest_source_paths(root, manifest)` check to
   `manifest.sources`, so `bin/build TARGET` returned success without
   invoking CMake for targets whose build inputs are claimed support units
   rather than authored lifts. Two real targets hit this:
   `config/targets/emi/etc/bate/03/target.toml` and
   `config/targets/emi/battle/batl_re2/01/target.toml` (empty `sources`;
   claimed PsyQ support `src/bof3/support/*_psyq.c` plus headers). Their
   CMake targets exist and own those claimed objects (CMakeLists groups
   claimed translation units under the owner manifest's source_dir), so the
   gate is restored to `manifest_source_paths(...)`: a support-only target
   now resolves `cmake_target_for_directory(manifest.source_dir)` and builds
   its claimed objects. The regression test that locked in the no-op
   (`test_build_no_authored_sources_uses_canonical_id`) was replaced with
   `test_build_support_only_target_still_invokes_cmake`, which asserts the
   CMake build invocation (directory + target) for a support-only target.

## Validation

Run focused checks after every phase and the full practical gate at closure:

```sh
PYTHONPATH=tools/python .venv/bin/pytest -q tools/python/tests
.venv/bin/ruff check tools/python
python3 .pi/skills/bof3-re/scripts/test-skill-scripts.py
bin/scratchpad preview 'emi/world00/area008/13@0x801F3D88'
bin/symbols check
just check
git diff --check
git diff --cached --quiet
```

Compare any baseline-red policy or naming checks against `HEAD` byte-for-byte;
do not classify a changed failure as pre-existing.

## Boundaries and non-goals

- Preserve the current uncommitted harness implementation and resolver work.
- Work only in this repository; do not edit `build/`, `toolchains/`, `inputs/`,
  generated bindings, or disposable `out/` as reviewed truth.
- Do not change target identity, manifests, maps, Splat layout, lifted C,
  matching semantics, CLI names, or output contracts.
- Do not stage, commit, push, publish, or mutate external systems.
- Do not introduce dependencies, managers/services/factories, abstract command
  bases, one-use protocols, or deep inheritance.
- Prefer deletion, direct imports, functions for stateless work, frozen
  dataclasses for values, and shallow inheritance only for shared lifecycle.
- Stop only for an evidence-backed compatibility or ownership blocker that
  cannot be resolved from repository callers and tests.
