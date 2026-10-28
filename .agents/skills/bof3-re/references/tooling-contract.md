# Tooling contract

Read before changing Python harness, CLI or skill scripts. Source lifting follows
[target contract](target-contract.md); test ownership follows
[unit map](../../bof3-test/references/unit-map.md).

## Names and ownership

- Keep one owning definition and migrate importers. Functions use
  verb-led `snake_case`; predicates `is_`/`has_`/`can_`. Constructors/protocols keep
  conventional names. Classes `PascalCase`, constants `UPPER_SNAKE`. Private
  functions may use one leading underscore; filenames do not advertise privacy.
- Module ceiling 4,000 lines, enforced by
  `test_harness_dry.py::test_harness_modules_stay_decomposed`. Reuse compatible common
  helpers and bundle related operations by domain; avoid size-driven slices and
  maintained `.before` or `.after` modules. Keep current inputs and requested
  deliverables; remove obsolete audit evidence when authorized.
  Noun modules/subpackages: `ranking.py`, `transactions.py`,
  `review.py`, `editing.py`, `cli.py`. Package supplies domain; no repeated prefix
  or opaque synonyms. `__init__.py`/`__main__.py` remain protocol exceptions.
  Count normally Ruff-formatted lines; do not compress lines to fit.
- `harness.naming` owns naming facts/checks; `types` owns representation facts/checks;
  `macros` owns macro facts/checks; `combiner` owns source membership/checks. Shared function
  identity/metadata stays in `domain`, compiler selection in `build`, Markdown
  operations in `docs`, not facts or approvals described there.
- `opportunities.py` discovers/describes leads; `cli.py` adapts arguments. Discovery
  is not ranking, evidence closure or edit authority. Cross-domain queries dispatch
  to owners. One concern skill owns its modes; route references separate authority.
- Dependencies flow toward owners. No cycles hidden by facades, compatibility
  re-exports, `x as x` or `# noqa: F401` shims. Package initializers stay inert.
- Replace superseded behavior and migrate its callers. Remove legacy parsers,
  aliases and fallback routes rather than preserving backward compatibility.
- Header order: accurate one-line module docstring, `from __future__ import
  annotations`, stdlib imports, local imports. Run `ruff format` on touched Python.

## Commands and shared mechanisms

Implement `main()` as `return run_main(build_parser, argv)`. Put domain CLI in
`cli.py` and cross-domain commands in `harness.commands`. Dispatch from `bin/` and
skill scripts without duplicating parsing, validation, mutation or recovery policy.

Use `harness.common.cli` helpers `add_root_argument(parser)` and
`add_example_argument(parser, text)`. No raw argv scans or hand-rolled
`--root`/`--example` parsing.

Keep helpers in `harness.common` only when current callers share their semantics
and trust boundaries. Domain adapters supply policy; common must not import them.
Keep specialized helpers local. Remove workflow admission, review and recovery
engines when no retained capability needs them; do not replace them with a generic
runner or skill-side implementation.

Retain rollback images only for explicit changed paths. Observe unrelated Git state
and hash reported dirty paths without copying clean sources/dependency trees. Pin
consumed inputs through owner guards. Exclude root `build/` outputs from recovery
and refuse indexed/committed paths there. Preserve raw index bytes/identity, writer
cleanup, owned-only restoration and native/semantic gates. Do not require whole-
workspace snapshots, recursive source inventories or new proof/grant frameworks
for bounded naming/type/macro/combiner work.

Use OS-default file creation. Only tool setup may set executable bits. Preserve
file-type, symlink, inode and hardlink guards; captures and rollback omit permission
fields and checks.

Stdlib first, then installed dependencies; no new dependency without explicit user
approval. Prefer smallest working diff and deletion over speculative abstraction.
Toolchain wrappers declare class attributes/minimal overrides on `toolchain/base.py`.
Managed toolchain owns installation, executable path, invocation environment and
verification; wrappers dispatch only. Harness never launches model processes.

## Cleanup commands and skill ownership

Naming, macros, types, combining and source metadata expose only `list` and
`check`. Require an explicit target. Listings return ordered facts with source
locations and explicit truncation; stale required inputs fail without refreshing.
Checks derive complete affected consumers, preserve every grouped member and
compare full reviewed original ranges. Missing coverage fails, never silently
passes. Checks may produce disposable native outputs but never repair reviewed
files, publish acceptance or launch AI models.

Skills own evidence-backed selection, direct edits, semantic judgment and required
independent review. Read [editing and recovery](editing-recovery.md) before writes.
Use the existing domain resolver, original-byte decoder, build and match services.
Keep macro/type lexical leads distinct from semantics and preserve target-local
ownership, effective build profiles, ABI, placement, data and bindings. Same
addresses or repeated code do not prove shared state or useful grouping.

Retire old prepare/apply/finalize commands, ranked accounts, semantic admission
records and forwarding wrappers with their live callers. Historical evidence
remains historical without a compatibility parser. Frozen campaign scopes and
unresolved evidence/recovery obligations survive command retirement.

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
