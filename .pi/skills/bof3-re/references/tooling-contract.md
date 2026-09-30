# Tooling contract

Read before changing Python harness, CLI or skill scripts. Source lifting follows
[target contract](target-contract.md); test ownership follows
[unit map](../../bof3-test/references/unit-map.md).

## Names and ownership

- One concept, one name; keep owning definition, migrate importers. Functions use
  verb-led `snake_case`; predicates `is_`/`has_`/`can_`. Constructors/protocols keep
  conventional names. Classes `PascalCase`, constants `UPPER_SNAKE`. Private
  functions may use one leading underscore; filenames do not advertise privacy.
- Module ceiling 600 lines, enforced by
  `test_harness_dry.py::test_harness_modules_stay_decomposed`. Decompose mechanically
  before growth. Noun modules/subpackages: `ranking.py`, `transactions.py`,
  `review.py`, `editing.py`, `cli.py`. Package supplies domain; no repeated prefix
  or opaque synonyms. `__init__.py`/`__main__.py` remain protocol exceptions.
- `harness.naming` owns identity/evidence/audits; `types` owns representation/layout;
  `macros` owns extraction; `combiner` owns source consolidation. Shared function
  identity/metadata stays in `domain`, compiler selection in `build`, Markdown
  operations in `docs`, not facts or approvals described there.
- `opportunities.py` discovers/describes leads; `cli.py` adapts arguments. Discovery
  is not ranking, evidence closure or edit authority. Cross-domain queries dispatch
  to owners. One concern skill owns its modes; route references separate authority.
- Dependencies flow toward owners. No cycles hidden by facades, compatibility
  re-exports, `x as x` or `# noqa: F401` shims. Package initializers stay inert.
- Header order: accurate one-line module docstring, `from __future__ import
  annotations`, stdlib imports, local imports. Run `ruff format` on touched Python.

## Commands and shared mechanisms

`main()` is `return run_main(build_parser, argv)`. Domain CLI lives in domain
`cli.py`; `harness.commands` owns cross-domain commands. `bin/` and skill scripts
only dispatch; never duplicate parsing, validation, mutation or recovery policy.

Use `harness.common.cli` helpers `add_root_argument(parser)`,
`add_example_argument(parser, text)`, `add_work_deadline_argument(parser)`.
Deadline opt-in binds original monotonic cutoff, rejects late success, never
renews duration or controls cleanup. No raw argv scans or hand-rolled
`--root`/`--example` parsing.

`harness.common` owns shared `cli`, `digests`, `files`, `paths`, `process`,
`workspace`, `evidence`, `execution`, `review`, `revalidation` mechanisms, not
second copies of domain policy. Extract only where semantics/trust boundaries
agree. Domain adapters supply policy; shared mechanisms never import domain
adapters to discover it. Keep specialized helpers local; similar hash formats,
validators or evidence checks need not be equivalent.

Stdlib first, then installed dependencies; no new dependency without explicit user
approval. Prefer smallest working diff and deletion over speculative abstraction.
Toolchain wrappers declare class attributes/minimal overrides on `toolchain/base.py`.
Managed toolchain owns installation, executable path, invocation environment and
verification; wrappers dispatch only. Harness never launches model processes.

## Validation

Establish behavior with existing checks or disposable characterization before
refactoring. No added tests without explicit coverage request. Existing imports,
monkeypatch targets and layout assertions migrate with owners, never weaken.
Patch module owning global, not facade. Moving tests requires fixing `__file__`
paths and cross-test imports; unit dirs remain on `pythonpath`, `build` collected.

Run relevant existing unit checks, Ruff and scoped `git diff --check`. `justfile`
owns exact gate membership: `just check` runs shared gates without pytest units,
`just check-unit UNIT` adds one unit, `just check-all` adds all units. Report each
stage's real status, skips and unreached gates; suite PASS is not recipe PASS.
Frontmatter, chain shape, module ceiling and naming-collision contract checks
remain binding. Production behavior changes remain outside test-optimization scope.
