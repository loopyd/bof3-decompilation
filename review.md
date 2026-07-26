# Review — Phase 1–2 roadmap implementation

## Correct

- `tools/python/harness/commands/rev_query.py:338-361` implements the requested diagnostic mode without adding a persistence table: ordinary priority rankings still discard excluded candidates and `--exclusions` emits only rejection rows.
- `tools/python/harness/commands/rev_query.py:276-330` derives exclusions from target manifests, reviewed Splat boundaries, target image bytes, and the selected PsyQ SDK map, consistent with the ownership boundaries in `AGENTS.md`.
- `bin/_python:7-15` centralizes the five wrappers' repository-root/Python/PYTHONPATH bootstrap and retains the prior missing-environment message and exit code. Only the five Phase-2 wrappers were converted; excluded wrappers were not touched.

## Fixed

- None (review-only; no changes made).

## Blocker

1. **The documented analysis command has no executable wrapper.** `docs/usage.md:91-92` tells users to run `bin/analysis-sequence`, but the file does not exist (the current `bin/` contains no such wrapper). The new Python module therefore is not available through the documented command surface or `--example` acceptance path. Minimal correction: add the one thin `bin/analysis-sequence` wrapper using the established bootstrap and `python -m harness.commands.analysis_sequence`.

2. **The sequence re-analyzes the target instead of rebuilding the reverse index, violating the core Phase-1 sequence.** `tools/python/harness/commands/analysis_sequence.py:29-30` calls `analyze_project()`, which invokes Rizin, builds a new snapshot, and writes it (`tools/python/harness/rizin_project.py:102-117`). It never calls the index command/rebuild API. This contradicts the roadmap requirement to check freshness, then rebuild the index, then query; its docstring/usage claim "index rebuilt" is false (`analysis_sequence.py:30-31`). Minimal correction: after `status(...)["fresh"]` succeeds, call the existing index-command implementation with `root`; do not call `analyze_project`.

3. **The integration test cannot construct its fixture and does not exercise the required fresh path.** `tools/python/tests/test_analysis_sequence.py:25-26` calls `sqlite.connect(...)` on a `Path` (no such method), and `:27` references `_schema` without importing it. Even if those are fixed, its snapshot payload at `:62-70` stores `target` as `{"id": ...}`, while `rizin_project.status()` requires the string target ID (`tools/python/harness/rizin_project.py:130`). Thus the supposed fresh snapshot is always stale. Finally, `:120` requests `--ranking status`, but `rev_query` defines `status` as a non-ranking subcommand and the sequence parser only supplies ranking arguments; `parser.parse_args()` fails. Minimal correction: use `sqlite3.connect` and import/call `_schema`, create the exact snapshot schema accepted by `status`, use a supported ranking such as `metrics`, and mock/assert the index rebuild is called while analyzer refresh is not.

4. **All five converted wrappers now invoke non-existent hyphenated Python modules.** `bin/emi-target:6`, `bin/flag-search:6`, `bin/psyq-import:6`, and `bin/str-media:6` changed established module names from `emi_target`, `flag_search`, `psyq_import`, and `str_media` to hyphenated names. The matching command modules are underscore-named, so normal wrapper invocation cannot import them. (`bin/index:6` happens to remain valid.) The tests use a dummy interpreter that exits zero without inspecting `-m` (`tools/python/tests/test_wrapper_bootstrap.py:58-105`), so they miss the regression. Minimal correction: restore the underscore module names and make the forwarding test run a real interpreter (or a fake that records/asserts the module argument).

5. **`rev-query duplicates` now crashes because it has no `exclusions` parser attribute.** `tools/python/harness/commands/rev_query.py:548-552` evaluates `args.exclusions` for every ranked command, including `duplicates`; however `build_parser()` deliberately does not add that option to `duplicates` (`:761-767`). Therefore an ordinary `bin/rev-query duplicates` raises `AttributeError`. Minimal correction: use `getattr(args, "exclusions", False)` at the detail selection (or set a default on every ranked parser).

## Note

- `tools/python/tests/test_wrapper_bootstrap.py:14-31` executes the repository's real `bin/_python`, so its root is derived from that script's location rather than the temporary `cwd`; it cannot establish the claimed missing-venv behavior for the temporary directory. Test the helper through a copied/symlinked repository layout, or test wrappers with `PSX_PYTHON` pointing to a deliberately missing executable.
- Focused tests and doctor were not run in this read-only review. The fixture and wrapper defects above are directly evident from the changed code and should be fixed before validation.
