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

Implement `main()` as `return run_main(build_parser, argv)`. Put domain CLI in
`cli.py` and cross-domain commands in `harness.commands`. Dispatch from `bin/` and
skill scripts without duplicating parsing, validation, mutation or recovery policy.

Use `harness.common.cli` helpers `add_root_argument(parser)` and
`add_example_argument(parser, text)`. No raw argv scans or hand-rolled
`--root`/`--example` parsing.

Keep shared `cli`, `digests`, `files`, `paths`, `process`, `workspace`, `evidence`,
`execution`, `review`, `revalidation` mechanisms in `harness.common`. Extract only
matching semantics/trust boundaries. Domain adapters supply policy; common must
not import them. Keep specialized helpers local; similar formats/checks need not
share semantics.

Retain rollback images only for explicit changed paths. Observe unrelated Git state
and hash reported dirty paths without copying clean sources/dependency trees. Pin
consumed inputs through owner guards. Exclude root `build/` outputs from recovery
and refuse indexed/committed paths there. Preserve raw index bytes/identity, writer
cleanup, owned-only restoration and native/semantic gates. Do not require whole-
workspace snapshots, recursive source inventories or new proof/grant frameworks
for bounded naming/type/macro/combiner work.

Stdlib first, then installed dependencies; no new dependency without explicit user
approval. Prefer smallest working diff and deletion over speculative abstraction.
Toolchain wrappers declare class attributes/minimal overrides on `toolchain/base.py`.
Managed toolchain owns installation, executable path, invocation environment and
verification; wrappers dispatch only. Harness never launches model processes.

## Combiner discovery

`bin/harness combiner discover TARGET TARGET@0xADDRESS TARGET@0xADDRESS`
runs `helper-call-cluster`, `macro-family`, `shared-state-type` or
`functional-subcategory` over two to 32 distinct, explicitly owned C functions of
one target. Each member needs attached source metadata, map/layout agreement and
a finite aligned original range. The command validates the complete
reverse index and corroborates indexed direct calls through the domain MIPS
decoder. It retains unresolved and unsupported indexed edges.

Enumerate every unordered pair; `--minimum-members` is fixed at two. Select the
method's threshold below and reject nondefault thresholds for other methods.

| Method | Threshold |
| --- | --- |
| `helper-call-cluster` | positive `--minimum-shared-callees` for distinct target-qualified callees |
| `--method macro-family` | `--minimum-shared-macros` for shared target-private function-like definitions |
| `--method shared-state-type` | `--minimum-shared-relations` for distinct shared storage addresses/private declaration spellings |
| `--method functional-subcategory` | `--minimum-shared-subcategories` for distinct shared authored category IDs |

Eligible pairs rank by descending count of their method's shared relations, then
canonical member IDs. Below-threshold pairs retain their reasons. The `retain-all`
overlap policy records every pair sharing a member, including excluded pairs.
`--top-n` selects a view from the full ranking without truncating the pool.

Query selected function identities/referenced definition IDs through the macro
owner. Bind uses to lexical line/column/arguments/conditions/selected implementation
and definitions to current source bytes/body/parameters/conditions. Capture literal
transitive includes. Retain shared/generated/unsupported/restricted/matching-helper/
object-like/ambiguous/out-of-closure uses with exclusion reasons. Qualify only
ordinary `function_like` definitions without owner restrictions; multiple
definitions for one use cannot qualify. Refuse more than 4,096 queried uses and
preserve reviewed C-source/original-range gates.

Treat macro records as lexical grouping leads; conditional/shared definitions prove
no active preprocessing, original semantics, native equivalence or useful ownership.
Require human-value review and source-consolidation acceptance before grouping.

For shared-state/type, query selected function identities/type names, check lexical
positions/source hashes through the type owner, reconcile claimed headers through
the declaration owner and capture literal includes. Keep tag/ordinary namespaces
separate. Retain shared/ambiguous/unsupported/out-of-closure declarations with
reasons. Qualified private spellings still have unresolved lexical bindings;
they prove neither semantic type uses nor active preprocessing.

Use exact target-qualified load/store addresses. Require indexed rows to match
the domain MIPS decoder over each original range and the type owner's storage
evidence. Retain exclusion reasons for address materialization/unsupported storage
accesses, without changing candidate blockers/representation status. The decoder
covers direct `lui`/low-half uses in a fixed instruction window; absence proves no
missing access. Equal addresses prove no aliases, pointer producers, extent,
lifetime, state ownership or safe grouping. Separate declaration/storage-candidate
IDs and cap uses/declarations/references/storage evidence at 4,096 facts per category.

The functional method requires `--subcategories FILE`, a canonical
repository-relative JSON input, and `--expected-subcategories-sha256 SHA`.
The document has exactly `schema`, `target`, `subcategories` and `members`;
`schema` is `bof3.combiner-subcategories/v1` and `target` is the canonical target
ID. Each category has exactly `id` and `description`. IDs use lowercase kebab-case
and at most 80 characters; descriptions and rationales are nonempty trimmed text
of at most 4,096 characters. Each member has exactly `id`, `source`,
`source_sha256`, `original_sha256`, `metadata_sha256`, `implementation_sha256`
and `subcategories`. Member IDs match the complete canonical pool exactly.
Each member's `subcategories` entry has exactly `id` and `rationale` and refers
to one declared category. Duplicate JSON keys, categories, members or assignments
cause refusal. Categories and total assignments are each capped at 4,096.

Match bindings to current claimed source, complete attached metadata, ordinary C
implementation and complete reviewed original range. Use
`bin/harness combiner inspect-source SOURCE` for source, metadata and implementation
hashes and Unicode ranges; discovery supplies member IDs/original-range hashes.
Capture literal transitive includes/metadata providers and retain full metadata,
including behavior continuations. Treat category descriptions/rationales as authored
leads; current bindings/shared categories prove no semantics, return contract, type
facts, safe grouping or edit authority. Independently assess every member/pair's human value.

The result pins membership, thresholds, method version, captured input bytes and
modes, exact queried index facts, overlaps and ordering. Retain its `fingerprint` externally and pass
it as `--expected-fingerprint` when rechecking the same pool. A changed pin, stale
index, duplicate or cross-target member, invalid threshold, unsupported method or
overlap policy, ownership drift, original JAL disagreement, moving input, more
than 4,096 indexed call edges, stale classification bindings, classification SHA
disagreement or storage-decoder disagreement causes refusal.
Individual files are capped at 16 MiB and the original image at 64 MiB.
Discovery records neither human-value approval nor native or mutation acceptance.

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
