# Python coding standards

Applies to `tools/python/` and project skill Python scripts.
Read after `SOUL.md` and `AGENTS.md`; lift-side C rules stay in `AGENTS.md`.

## Naming

- One concept, one name repo-wide; rename the loser, keep the owner
  (precedent: `signature_index_path`, `resolve_function_name`).
- Functions/methods are verb-led `snake_case` (`resolve_function`, not
  `function_name`).
- Functions express an action and its object: `collect_evidence`,
  `validate_candidate`, `prepare_transaction`, `resolve_function`.
  Predicates use `is_`, `has_`, or `can_`; constructors and Python protocols keep
  their conventional names. Private functions may take one leading underscore;
  filenames do not advertise privacy.
- Classes are `PascalCase`. Toolchain wrappers are declarative subclasses of
  the `toolchain/base.py` contract (class attributes, minimal overrides).
- Constants are `UPPER_SNAKE` at module top.

## Module organization

- Hard ceiling: 450 lines per module, enforced by
  `test_harness_dry.py::test_harness_modules_stay_decomposed`. Decompose
  mechanically before growing into noun categories or a cohesive subpackage.
  Dependencies flow toward the owner; do not hide cycles behind facade imports.
- Use a single noun for each module: `ranking.py`, `transactions.py`,
  `review.py`, `editing.py`, `cli.py`. The package supplies the domain, not a
  repeated prefix such as `macro_transactions.py`. Use noun subpackages when a
  category needs decomposition; do not invent opaque synonyms just to fit filenames.
  `__init__.py` and `__main__.py` are Python protocol exceptions, not private files.
- `harness.macros`, `harness.naming`, and `harness.types` own their respective
  opportunity discovery, evidence, transactions, audits, editing and CLI policy.
  Naming identifies symbols; types own C representation and layout decisions.
  `harness.docs` owns Markdown operations, not the domain facts or approvals described.
  An `opportunities.py` owner discovers and describes leads; `cli.py` only adapts
  arguments. Lead enumeration is not ranking, evidence closure or edit authority.
  Cross-domain queries dispatch to these owners instead of re-exporting them.
  Keep one concern skill for discovery, evidence, audit and transaction modes;
  route-specific references preserve authority boundaries, not separate skill copies.
- No compatibility re-export shims (`x as x`, `# noqa: F401` facades).
  Importers reference the owning module directly.
- Header order: one-line module docstring, `from __future__ import
  annotations`, stdlib imports, local imports. Ruff-clean; run `ruff format`
  on every touched file.
- Every module has an accurate one-line docstring.

## CLI commands

- `main()` is one line: `return run_main(build_parser, argv)`.
- Domain commands live in their package's `cli.py`; `harness.commands` owns
  cross-domain commands. `bin/` and skill invocation scripts only dispatch; they
  must not duplicate parsing, validation, mutation or recovery policy.
- Shared flags come from `harness.common.cli`: `add_root_argument(parser)`,
  `add_example_argument(parser, text)`. No per-command parse boilerplate, no
  raw argv scans, no hand-rolled `--root`/`--example`.

## DRY and simplicity

- `harness.common` owns genuinely shared mechanisms in single-noun categories:
  `cli`, `digests`, `files`, `paths`, `process`, `workspace`, `evidence`,
  `execution`, `review`, and `revalidation`. It is not a miscellaneous dumping
  ground or a second owner of macro/naming/type policy.
- Extract repeated behavior only when semantics and trust boundaries agree.
  Domain adapters supply policy explicitly; common mechanisms must not import
  domain adapters to discover policy. Keep specialized helpers with their owner.
  Do not merge hash formats, path validators or evidence checks merely because
  their implementations look similar.
- Stdlib first, then already-installed dependencies; never add a dependency
  for what a few lines do. Smallest working diff wins; deletion over addition.

## Tests

- Establish the existing behavior before refactoring, using current checks or
  disposable characterization probes. Add tests only when the user explicitly
  requests expanded coverage. Update existing import/monkeypatch targets and
  layout expectations with their owners, without weakening behavioral assertions.
- Patch the module that owns the global, never a re-exporting facade.
- Contract tests guard shared infrastructure (front matter, chain shape,
  module ceiling, naming collisions) so regressions fail the suite, not a
  review.
