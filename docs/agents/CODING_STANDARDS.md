# Python coding standards

Applies to `tools/python/` (harness, commands, tests) and `.pi/**/scripts/`.
Read after `SOUL.md` and `AGENTS.md`; lift-side C rules stay in `AGENTS.md`.

## Naming

- One concept, one name repo-wide; rename the loser, keep the owner
  (precedent: `signature_index_path`, `resolve_function_name`).
- Functions/methods are verb-led `snake_case` (`resolve_function`, not
  `function_name`).
- Private helpers take one leading underscore; decomposed implementation
  lives in `_private.py` siblings, no compat facade.
- Classes are `PascalCase`. Toolchain wrappers are declarative subclasses of
  the `toolchain/base.py` contract (class attributes, minimal overrides).
- Constants are `UPPER_SNAKE` at module top.

## Module organization

- Hard ceiling: 450 lines per module, enforced by
  `test_harness_dry.py::test_harness_modules_stay_decomposed`. Decompose
  mechanically before growing: move verbatim segments to `_private` siblings,
  dependencies flow one direction (original → siblings, never back).
- No compatibility re-export shims (`x as x`, `# noqa: F401` facades).
  Importers reference the owning module directly.
- Header order: one-line module docstring, `from __future__ import
  annotations`, stdlib imports, local imports. Ruff-clean; run `ruff format`
  on every touched file.
- Every module has an accurate one-line docstring.

## CLI commands (`harness/commands/`)

- `main()` is one line: `return run_main(build_parser, argv)`.
- Shared flags come from `_common.py`: `add_root_argument(parser)`,
  `add_example_argument(parser, text)`. No per-command parse boilerplate, no
  raw argv scans, no hand-rolled `--root`/`--example`.

## DRY and simplicity

- No duplicate function bodies; extract to the nearest shared helper
  (`_common.py`, `toolchain/helpers.py`) instead of copying.
- Stdlib first, then already-installed dependencies; never add a dependency
  for what a few lines do. Smallest working diff wins; deletion over addition.

## Tests

- Characterization tests before any refactor: behavior is locked first, only
  monkeypatch module targets may move with the code.
- Patch the module that owns the global, never a re-exporting facade.
- Contract tests guard shared infrastructure (front matter, chain shape,
  module ceiling, naming collisions) so regressions fail the suite, not a
  review.
