# Streamlined BOF3 Tooling Handoff

This file is a handoff for the next LLM. It describes what was changed, what
has been validated, what still needs validation, and the safest next commands.

## Goal

Align the live `rebof3-simple` command surface with the intended small-agent
BOF3 workflow:

```text
bin/bootstrap --plan
bin/harness refresh
bin/harness status
bin/harness candidates --module <module>
bin/harness claim <target-or-module>
bin/harness lift <target>
bin/harness verify function <source-or-target>
bin/harness verify module <module> --allow-different
bin/harness report summary|module|function ...
bin/harness finish <target> --status done|blocked --message ...
bin/harness ghidra import-project --no-analysis
bin/harness ghidra analyze
bin/harness ghidra export
bin/harness ghidra coverage
```

Stale public top-level harness verbs such as `setup`, `catalog`, `analyze`,
`split`, `dashboard`, `diff`, and `binary` should not be advertised as the
current workflow.

## Implementation Completed

- Added `bin/bootstrap`.
- Added `tools/python/rebof3/commands/bootstrap.py`.
- Added the bootstrap pipeline:
  1. `bin/doctor`
  2. `bin/setup`
  3. `bin/disk-extract`
  4. `bin/emi-unpack`
  5. `bin/inventory-build`
  6. `bin/ghidra-bootstrap`
  7. `bin/harness ghidra import-project --no-analysis`
  8. `bin/harness ghidra analyze`
  9. `bin/harness ghidra export`
  10. `bin/inventory-import-ghidra-symbols`
  11. `bin/harness refresh`
- Reworked `bin/harness --help` public verbs to:
  - `refresh`
  - `status`
  - `candidates`
  - `claim`
  - `release`
  - `lift`
  - `verify`
  - `report`
  - `ghidra`
  - `finish`
- Implemented `bin/harness refresh`.
  - Seeds EMI targets, artifact targets, migration targets, and function targets.
  - Writes `output/harness/harness.sqlite3`.
  - Writes `output/harness/catalog.json`.
  - Writes `output/harness/report.json`.
  - Writes `output/harness/report.md`.
  - Writes `output/harness/dashboard/index.html`.
- Implemented `bin/harness candidates`.
- Implemented `bin/harness lift`.
- Implemented `bin/harness verify function`.
- Implemented `bin/harness verify module`.
- Implemented `bin/harness report summary`.
- Implemented `bin/harness report module`.
- Implemented `bin/harness report function`.
- Implemented locked Ghidra wrappers:
  - `bin/harness ghidra import-project --no-analysis`
  - `bin/harness ghidra analyze`
  - `bin/harness ghidra export`
  - `bin/harness ghidra coverage`
- Kept `export-symbols` as a compatibility alias for `bin/harness ghidra export`.
- Updated `bin/pipeline ghidra-ready` and `bin/pipeline decomp-full-ready` to use
  the locked harness Ghidra lane.
- Changed `harness-ready` and `lift-ready` to call `bin/harness refresh`.
- Removed the stale public `binary-parity` pipeline/Make target.
- Updated docs to use `output/` instead of stale `out/` where touched.
- Updated harness docs away from the old `setup/catalog/analyze/split/dashboard`
  flow.
- Updated tests for the new surface and pipeline shape.

## Important Files Changed

- `bin/bootstrap`
- `tools/python/rebof3/commands/bootstrap.py`
- `tools/python/rebof3/harness/cli.py`
- `tools/python/rebof3/harness/commands.py`
- `tools/python/rebof3/harness/lift.py`
- `tools/python/rebof3/harness/report.py`
- `tools/python/rebof3/harness/binary.py`
- `tools/python/rebof3/pipelines/reverse.py`
- `tools/python/rebof3/pipelines/harness.py`
- `tools/python/rebof3/pipelines/registry.py`
- `tools/python/rebof3/pipelines/__init__.py`
- `harness.toml`
- `Makefile`
- `.agents/harness.md`
- `README.md`
- `bin/README.md`
- `docs/DECOMP_WORKFLOW.md`
- `docs/SETUP.md`
- `docs/README.md`
- `docs/plan.md`
- `docs/specs/formats/artifacts.md`
- tests under `tools/python/tests/`

## Validation Completed

These commands passed:

```bash
bin/bootstrap --plan
bin/harness --help
bin/harness ghidra --help
bin/harness refresh
bin/harness status --module emi:ETC/GAME#0
bin/harness candidates --module emi:ETC/GAME#0 --source existing --limit 3
bin/harness --json refresh
bin/harness --json report summary
bin/harness candidates --module emi:ETC/GAME#0 --source existing --limit 3
bin/pipeline --list
bin/pipeline ghidra-ready --plan
bin/pipeline decomp-full-ready --plan
bin/build
git diff --check
PYTHONDONTWRITEBYTECODE=1 .venv/bin/python -m pytest -q -p no:cacheprovider tools/python/tests
```

Latest full test result:

```text
176 passed, 9 warnings
```

Focused Ruff on touched files passed.

## Validation Findings

- `bin/bootstrap --plan` exits 0 and shows the expected 11-step flow.
- `bin/harness --help` exits 0 and shows only the streamlined public top-level
  commands.
- `bin/harness ghidra --help` exits 0 and shows:
  - `import-project`
  - `analyze`
  - `export`
  - `export-symbols` as an alias
  - `coverage`
- `bin/harness refresh` exits 0 and currently reports:
  - `catalog entries: 6344`
  - `targets upserted: 6714`
  - `report: output/harness/report.json`
  - `dashboard: output/harness/dashboard/index.html`
- `bin/harness status --module emi:ETC/GAME#0` exits 0 and reports:
  - `total targets: 6714`
  - `cataloged: 3656`
  - `queued: 3040`
  - `ready: 18`
  - module progress for `output/extracted/BIN/ETC/GAME.EMI#0`:
    `done=0/18 (0%) queued=18`
- `bin/harness candidates --module emi:ETC/GAME#0 --source existing --limit 3`
  exits 0 and lists three source-backed candidates:
  - `func-src:src/modules/game/00/func_801c71ac.c`
  - `func-src:src/modules/game/00/func_801c7188.c`
  - `func-src:src/modules/game/00/func_801c5798.c`

## Known Failures / Gaps

`bin/doctor --strict` currently fails because generated Ghidra inventory indexes
are missing:

```text
MISS ghidra-symbols-index missing output/inventory/ghidra_symbols_index.json
MISS ghidra-function-index missing output/inventory/ghidra_function_index.json
```

The doctor hint is:

```bash
bin/harness ghidra export
bin/inventory-import-ghidra-symbols
```

This was not fixed because it requires the Ghidra export path to be run against
the local Ghidra project.

Full repo-wide Ruff still reports pre-existing unrelated issues outside the
touched implementation:

```text
tools/python/rebof3/analysis_db/context.py: unused import re
tools/python/rebof3/commands/gen_context.py: unused ParserBuilder import
tools/python/rebof3/match/asm_diff.py: unused source_pattern
tools/python/rebof3/match/asm_diff.py: repeated key "src/modules/logo"
```

Do not treat these as regressions from the harness/bootstrap work unless the
next task explicitly includes repo-wide Ruff cleanup.

## Validation Was Interrupted Here

Small subagent validation completed through this checkpoint:

```bash
bin/harness candidates --module emi:ETC/GAME#0 --source existing --limit 3
```

The user then asked for this handoff file. Continue from the next checkpoint
below.

## Next Validation Steps

Run these one command at a time. Prefer small subagents or manual checkpoints.

1. Report function path:

```bash
bin/harness report function bof3/src/modules/game/00/func_801c71ac.c
```

Expected: exit 0, prints target/function/source/binary/asm-diff/m2c/next fields.

2. Report module path:

```bash
bin/harness report module emi:ETC/GAME#0
```

Expected: exit 0, prints module progress and a function table.

3. Claim by module without changing source:

```bash
bin/harness claim --module emi:ETC/GAME#0 --owner validation-agent
```

Expected: exit 0, claims a function target. Record the target id.

4. Release that target:

```bash
bin/harness release <target-id> --owner validation-agent
```

Expected: exit 0.

5. Validate lift on a known source-backed target only if you are comfortable
writing generated workspace artifacts:

```bash
bin/harness lift func-src:src/modules/game/00/func_801c71ac.c
```

Expected: creates or updates `output/harness/workspaces/...` and context/m2c
outputs. This may fail if function size or raw binary inputs are insufficient.
If it fails, capture the exact error; do not guess.

6. Validate function asm-diff only if the build/object prerequisites are ready:

```bash
bin/harness verify function bof3/src/modules/game/00/func_801c71ac.c --allow-different
```

Expected: returns 0 with `--allow-different` if it can build and diff. It may
fail if size inference is missing because Ghidra indexes are currently absent.
If it fails, capture exact output.

7. Validate module scan only after function verify is understood:

```bash
bin/harness verify module emi:ETC/GAME#0 --allow-different
```

Expected: compact match-percent table. It may fail for the same reason as
function verify if Ghidra indexes are absent.

8. Validate Ghidra coverage as a read-only check:

```bash
bin/harness ghidra coverage --allow-partial
```

Expected: exit 0 with coverage counts, even if partial.

9. Validate doctor after refreshing Ghidra symbol indexes:

```bash
bin/harness ghidra export
bin/inventory-import-ghidra-symbols
bin/doctor --strict
```

Run these only when it is acceptable to touch the shared Ghidra project. Ghidra
commands use the harness `ghidra` SQLite lock.

## Safe Continuation Rules

- Use the nested repo:

```bash
cd /home/rcorreia/projects/rebof3-mono/rebof3-simple
```

- Use `bin/` commands, not ad hoc Python entrypoints.
- Treat Ghidra as single-writer. Use only `bin/harness ghidra ...` for shared
  Ghidra project writes.
- Do not reintroduce stale top-level harness commands.
- Do not use `binary-parity`; it was intentionally removed from the public
  pipeline/Make surface during this pass.
- Generated output under `output/` is not source.
- If validation fails, capture exact command, exit code, stdout, stderr, and the
  smallest likely missing prerequisite.

## Final Completion Criteria

The task is complete when:

- `bin/bootstrap --plan` shows the full intended flow.
- `bin/harness --help` shows only the streamlined public top-level commands.
- `bin/harness refresh` succeeds.
- `bin/harness status` and `candidates` succeed for at least one real module.
- `bin/harness report summary|module|function` succeed.
- Claim/release succeeds on a function target.
- Function verify behavior is understood and documented, including any missing
  Ghidra index prerequisite.
- `bin/build` succeeds.
- Full Python tests pass.
- `bin/doctor --strict` either passes or has a documented, concrete missing
  prerequisite with the exact command to repair it.
