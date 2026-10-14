# Tooling contract

Read before changing Python harness, CLI or skill scripts. Source lifting follows
[target contract](target-contract.md); test ownership follows
[unit map](../../bof3-test/references/unit-map.md).

## Names and ownership

- One concept, one name; keep owning definition, migrate importers. Functions use
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

Shared transactions retain rollback images only for explicit changed paths.
Observe unrelated Git state and hash reported dirty paths; do not copy clean
sources or recursively capture dependency contents. Pin explicitly consumed inputs
through their existing owner guards. Recovery excludes root `build/` outputs and
refuses indexed or committed paths there. Preserve raw index bytes and identity,
writer cleanup, owned-only restoration, and all native and semantic gates.
Whole-workspace snapshots, recursive source inventories, and new proof or grant
frameworks are not prerequisites for bounded naming, type, macro, or combiner work.

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

Each method enumerates every unordered pair. `--minimum-members` is fixed at two.
For `helper-call-cluster`, `--minimum-shared-callees` sets the positive threshold
for distinct target-qualified callees. For `--method macro-family`,
`--minimum-shared-macros` sets the threshold for shared target-private function-like
definitions. For `--method shared-state-type`, `--minimum-shared-relations` sets
the threshold for distinct shared storage addresses and private declaration
spellings. For `--method functional-subcategory`,
`--minimum-shared-subcategories` sets the threshold for distinct shared authored
category IDs. Nondefault thresholds for another method cause refusal.

Eligible pairs rank by descending count of their method's shared relations, then
canonical member IDs. Below-threshold pairs retain their reasons. The `retain-all`
overlap policy records every pair sharing a member, including excluded pairs.
`--top-n` selects a view from the full ranking without truncating the pool.

The macro owner queries only selected function identities and referenced definition
IDs. Discovery binds each use to its lexical line, column, arguments, conditions
and selected implementation, and each definition to current source bytes and
parsed body, parameters and conditions. It captures literal transitive include
inputs. Shared, generated, unsupported, restricted, matching-helper, object-like,
ambiguous and out-of-closure uses remain visible with exclusion reasons. Only
ordinary `function_like` definitions without owner restrictions qualify. Multiple
definitions of one use do not supply a qualified shared macro. The method refuses
more than 4,096 queried uses and preserves the existing reviewed C-source and
original-range gates.

Macro use records describe lexical grouping leads. Conditional records and a
shared definition do not prove active preprocessing, original semantics, native
equivalence or useful source ownership. Complete human-value review and the
source-consolidation acceptance gates remain required before grouping.

The shared-state/type method queries selected function identities and type names.
It checks lexical occurrence positions and source hashes through the type owner,
reconciles current claimed header declarations through the declaration owner, and
captures literal include inputs. Tag and ordinary namespaces stay separate.
Shared, ambiguous, unsupported and out-of-closure declarations remain visible
with reasons. A qualified private declaration spelling retains unresolved lexical
binding; it does not establish a semantic type use or active preprocessing.

Storage relations use exact target-qualified load/store addresses. Indexed rows
must match the domain MIPS decoder over each member's original range and be
present in the type owner's storage-candidate evidence. Address materialization
and accesses without a supported storage candidate retain exclusion reasons.
Candidate blockers and representation status remain unchanged. The decoder covers
direct `lui`/low-half uses within its fixed instruction window; absence does not
prove a missing access. Equal addresses do not prove aliases, pointer producers,
object extent, lifetime, semantic state ownership or safe grouping. The result
separates shared declaration IDs from storage-candidate IDs and caps queried uses,
declarations, references and storage evidence at 4,096 facts per category.

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

Bindings must match the current claimed source, its complete attached metadata,
its ordinary C implementation and the complete reviewed original range. Use
`bin/harness combiner inspect-source SOURCE` for source, metadata and implementation
hashes and Unicode ranges; a discovery result supplies canonical member IDs and
complete original-range hashes. Discovery captures literal transitive includes
and the metadata providers. The full metadata block remains visible, including
behavior continuation lines. Category descriptions and rationales remain authored
classification leads. A current binding or shared category does not establish
original semantics, a return contract, type facts, safe grouping or edit authority.
Every member and pair still needs independent human-value assessment.

The result pins membership, thresholds, method version, captured input bytes and
modes, exact queried index facts, overlaps and ordering. Retain its `fingerprint` externally and pass
it as `--expected-fingerprint` when rechecking the same pool. A changed pin, stale
index, duplicate or cross-target member, invalid threshold, unsupported method or
overlap policy, ownership drift, original JAL disagreement, moving input, more
than 4,096 indexed call edges, stale classification bindings, classification SHA
disagreement, storage-decoder disagreement or an expired
`--work-deadline` causes refusal.
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
