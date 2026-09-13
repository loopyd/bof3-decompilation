# Macro opportunity indexing and resolution

This guide owns macro opportunity ranking, review and resolution tooling for
BOF3 cleanup and source finalization.
Start at [the documentation index](../INDEX.md). Execution history, frozen pilot
membership, and unfinished campaign work stay in
[the active plan](../plans/autonomous-bof3-decompilation.md), not this guide.

Parent-owned index refreshes and external review use the
[standing autonomous authorization](../INDEX.md#autonomous-execution), not a new
user permission request. Preserve pinned transactions and frozen proofs.

## Contents

- [Purpose](#purpose)
- [Required opportunity policy](#required-opportunity-policy)
- [Current implementation and gaps](#current-implementation-and-gaps)
- [Index and discovery](#index-and-discovery)
- [Assembly blocks](#assembly-blocks)
- [Human-value ranking](#human-value-ranking)
- [Candidate and consumer inspection](#candidate-and-consumer-inspection)
- [Semantic review](#semantic-review)
- [Reviewed application](#reviewed-application)
- [Reviewed private-header creation](#reviewed-private-header-creation)
- [Parent acceptance and replay](#parent-acceptance-and-replay)
- [Private revalidation and shared promotion](#private-revalidation-and-shared-promotion)
- [Existing abstractions](#existing-abstractions)
- [Implementation owners and checks](#implementation-owners-and-checks)

## Purpose

The macro opportunist combines machine discovery with human-like judgment. It is
intended to find similar assembly blocks within functions, index repeated shapes,
and give the reverse agent ranked opportunities for well-defined C preprocessor
macros. Assembly similarity supplies leads; it does not establish equivalent C,
shared ownership, or a worthwhile abstraction.

Hand-written C often becomes clearer when genuinely repetitive blocks share one
readable definition. Panel/UI management illustrates the benefit: wrappers can
name an operation and its parameters instead of repeating a whole update body.
BOF3 uses C89, not C++ templates. A good macro can provide useful compile-time
parameterization, but a macro is only sometimes the right cleanup. Readability,
semantic clarity, and original-byte fidelity matter more than macro count.

An already useful macro may suggest grouping its wrappers into a cohesive source
file rather than another extraction. That separate [combiner concern](combiner.md)
owns multi-function metadata and consolidation; discovery does not bypass its
pending native/transaction gates or expand cross-target ownership.

## Required opportunity policy

These are the outer-audit requirements. Discovery and pinned ranking are implemented;
the autonomous attempt scheduler is not:

1. Find repeated or sufficiently similar instruction blocks, retaining each
   target-qualified occurrence, original range, and parameter differences.
2. Apply a minimum instruction-block size before proposing an extraction. The
   numerical floor remains to be chosen/configured; no project default is
   established here. Exclude trivial fragments rather than suggesting everything.
3. Require **at least four uses** of the same proposed code shape. Count distinct
   use sites, not duplicate records or overlapping windows for one occurrence;
   four uses need not mean four distinct functions.
4. Feed the largest eligible blocks first into ranking. Retain machine measures
   such as instruction count, support, and variation alongside the AI's reasoning.
5. Apply a human-like wins filter: **would a human actually make this macro?**
   Prefer a coherent repeated operation, a small meaningful parameter set, clear
   call sites, less repetition, and a definition that is easier to maintain.
   Reject incidental similarity, opaque parameter lists, hidden control flow,
   evaluation hazards, and abstractions that make readers reconstruct the body.
6. Let AI assess and rerank eligible suggestions against those machine parameters
   and readability checks, recording why a lead is worth trying or is deferred.
7. Try a configurable **top N** in the authorized cleanup/finalization scope.
   A bounded ranked march is not permission to edit every discovered target.
   Preserve rejected/deferred reasoning and validate each retained change.

Four-use eligibility is separate from the **independently exact private proof per
declared target, at least two targets,** required for shared promotion. Neither replaces the
other, semantic review, nor validation of all affected consumers. Existing macros
and historical candidates are not retroactively approved or deleted by this policy.

## Current implementation and gaps

The discovery layer includes C lexical patterns, whole-function similarity, and
on-demand repeated assembly sub-block discovery. The block path implements the
explicit size floor, four-use eligibility, and largest-first machine ordering;
AI readability assessments now feed a validated, pinned top-N selection;
bounded autonomous execution remains unfinished.

| Current lead | Admission and current numeric rank |
| --- | --- |
| `constant` | At least three lexical occurrences in the grouped context; rank is occurrence count. |
| `expression_accessor` | At least three grouped member accesses; rank is twice occurrence count. |
| `statement_window` | Two or more matching three-statement C token windows, each at least twelve tokens; rank is token count times occurrences. |
| `exact_group` | At least two members of a nontrivial reviewed byte group; rank is reviewed byte size times members. |
| `parameterized_near_duplicate` | At least two structurally compatible reviewed functions with differing hashes and immediate/address deltas; at least three instructions per function; rank is instruction count times members. |
| `assembly_block` | Explicit positive instruction floor and at least four non-overlapping physical uses; numeric rank is instructions times support, but display orders instruction count first, then support and ID. |

Macro opportunities sort by descending numeric rank, then kind and ID;
near-duplicates sort by descending rank, then ID. Mixed byte/token/occurrence
scores are not the required largest-instruction-block-first policy. The current
three-instruction near-duplicate floor is not the intended configurable block floor.

All generated candidates remain `blocked`. The older lexical/whole-function
queries remain raw lead inventories, not the four-use block proposal policy.
Block artifacts require explicit human-value evidence and membership in a pinned
top-N selection. The CLI validates assessments supplied by an AI/reviewer; it does
not invoke a model or implement an autonomous attempt loop.
`rev-query --limit N` and `macro-audit blocks --limit N` limit **output rows**,
not attempted cleanups; their default is 20 and zero means all rows.

## Index and discovery

```sh
bin/index
bin/rev-query macros [NAME] [--target TARGET] [--classification KIND]
bin/rev-query macro-uses [NAME] [--target TARGET]
bin/rev-query [--limit N] macro-opportunities [--target TARGET] [--kind KIND]
bin/rev-query [--limit N] near-duplicates [--target TARGET]
```

Square brackets denote optional arguments, not literal shell syntax. Use canonical
target IDs; function operations use `TARGET@0xADDRESS`. Original bytes, reviewed
ranges, target manifests, and source metadata outrank analyzer suggestions.

Definitions come from manifest-claimed headers, sources/support sources,
shared helper headers, and `src/shared/**/*.inc`. The index retains definition
bodies, parameters, source hashes, conditions, provenance, restrictions, and lexical
uses. Generated PsyQ bindings are generator-owned/noncandidates. Historical
`matching_helper`/`sanctioned_helper` labels do not authorize the
[banned register/empty-asm aids](../INDEX.md#source-and-duplicate-rules), extraction
of aliases, or no-op shims. Requeue affected consumers for clean-C matching;
preserve address-binding assembly and original evidence.

Use rows are lexical name matches, **not proven preprocessor expansions**. Multiple
same-name definitions and their conditions remain visible. A unique target-owned
source can link to its indexed function; ambiguous and generated owners remain
unlinked. Index v13 requires an explicit rebuild from v12 to correct the former
absolute/relative source-link mismatch. Discovery never refreshes the index itself.

Near-duplicate analysis checks reviewed bytes against the binary and requires
matching instruction count, CFG metrics, call/data-reference shape, and normalized
instruction shape. Only immediate/address operand differences qualify; branch
displacements remain exact. Generated functions, trivial stubs, embedded data, and
analyzer/reviewed boundary disagreements are excluded by that path. Exact-group
leads retain registry blockers and disagreement counterexamples for review.
Stale source/binary/index evidence rejects; an index lead is never source authority.

## Assembly blocks

```sh
bin/macro-audit blocks --min-instructions 8 --target emi/battle/battle/15 --limit 5
bin/macro-audit account out/reviews/macro-block-account.json --min-instructions 8
bin/macro-audit validate-account out/reviews/macro-block-account.json
bin/macro-audit describe assembly_block:f94b25f4546f9b39 --min-instructions 8 --target emi/battle/battle/15
```

Eight is an example experiment floor, **not a project default**. `blocks` requires
an explicit positive integer; negative output limits and invalid targets reject.
`bof3.macro-block-opportunities/v1` reports the floor, fixed minimum of four uses,
ordering, visible candidate count, and zero automatic applications. Its
`attempt_budget` is null: an output limit never starts an autonomous attempt loop.

The scanner loads original payloads once per target, validates PSX headers and
reviewed function ranges/hashes, and excludes SDK-map entries, generated sources,
trivial/data-tagged entries, and analyzer/reviewed/Splat boundary disagreement.
Normalized MIPS words retain opcode/register fields and branch displacement;
immediate/address parameters stay explicit. A generalized suffix index finds
common contiguous sequences **within** reviewed functions; unique separators
prevent cross-function matches. Blocks may contain control transfers or delay
slots: compatible whole-function CFG/call shape is not established by this path.

For each repeated interval, choose its longest prefix admitting four disjoint
physical occurrences, including implicit prefixes needed when longer periodic
matches overlap. Count at most one overlapping range per target, even if indexed
functions overlap. Distinct sites in one function may count. Prefer larger blocks;
contained alternatives with the same selected function/site ordering are suppressed.
This is normalized-sequence discovery, not enumeration of every possible C or
register-renamed/CFG-equivalent abstraction.

Each member retains its owning function, original function hash, source hash when
present, byte offset within the function, start and **exclusive** end address,
block byte hash, and parameter positions/values. A stable `assembly_block:` ID
comes from the normalized pattern. Source-to-block mapping, entry/exit and delay-slot
context, live registers/memory effects, all semantic guards, and human value remain
unproven until reviewed. These are leads for reverse work, not ready-to-insert C.

Target filtering is a **view** of global groups, preserving all members and their
fingerprints; four global uses do not authorize writes to external targets. Block
discovery is computed from the fresh index and original inputs, not stored in a new
SQLite table or silently refreshed. An account includes this additional family
only when `--min-instructions` is supplied, pinning `block_discovery` with
`min_instructions` and `minimum_uses: 4`; validation replays that policy. Accounts
without the option retain their older inventory scope. Block description likewise
requires the explicit floor. A result disappearing under a stricter floor rejects
lookup rather than rebinding its identity.

## Human-value ranking

```sh
bin/macro-audit rank-input --min-instructions 8 --target emi/battle/battle/15 --pool-size 5 --top-n 2 --require-source > out/reviews/macro-ranking-request.json
bin/macro-audit rank out/reviews/macro-ranking-request.json out/reviews/evidence/macro-ranking.json --expected-pool-digest POOL_DIGEST
bin/macro-audit validate-ranking out/reviews/evidence/macro-ranking.json --expected-ranking-digest RANKING_DIGEST
```

Retain `pool_digest` externally before handing the request to the assessor. The
request (`bof3.macro-ranking-request/v1`) binds the exact policy, population and
largest-first candidate pool. `pool_size` and `top_n` are explicit positive integers;
`top_n` cannot exceed `pool_size`. Optional `require_source` admits only groups with
canonical source paths for every global member, not just the focused target.
`population.source_missing` counts exclusions by that filter; it is zero when the
filter is disabled. It does not measure unlifted code outside the visible population.

The assessor changes only `assessor` and `assessments`, inspecting each member's C,
original block and existing templates. Every pool candidate requires its exact ID
and fingerprint, a `try`, `defer` or `reject` decision, rationale, and all five
criteria: `coherent_operation`, `meaningful_parameters`, `clear_call_sites`,
`reduced_repetition`, `maintainability`. Each criterion records nonempty evidence
and a `promising`, `poor` or `unknown` verdict. A `try` needs all five promising and
a positive priority; try priorities are unique and contiguous from one. Other
decisions have null priority; rejection requires at least one poor criterion.

`rank` freshly reconstructs the machine pool and validates the external pin and
complete assessments. Its `bof3.macro-ranking/v1` output retains all decisions and
selects at most `top_n` try rows by human priority. Excess try rows remain in
`budget_deferred`; fewer worthwhile opportunities produce a shorter or empty queue.
The queue preserves global members and marks them `selected_for_review`, never
accepted. Retain the output digest externally for validation and transactions.
Unknown fields, stale candidates, incomplete assessments and altered pins reject.

Selection is not write authority, semantic acceptance or an attempt counter.
`safe_application_count` stays zero and `execution_started` stays false. Repeated
CLI invocations do not consume a durable budget: scheduling, retries and a campaign
attempt ledger remain unfinished. Do not substitute output limits for that ledger.

## Candidate and consumer inspection

`bin/agent-context cleanup macro-opportunity TARGET ID` supplies a bounded prefill
for one caller-selected lead and routes only to `bof3-macros`. It retains the
opaque ID, not a guessed replacement or target-local subset. This is context
transport only: use the owner inspection below to verify existence, target
membership and current fingerprint against the caller's frozen evidence before
proceeding. No ranking parameters or application authority are inferred.

```sh
bin/macro-audit account out/reviews/macro-account.json
bin/macro-audit validate-account out/reviews/macro-account.json
bin/macro-audit describe exact_group:8e1ad03b4ba92303 --target emi/battle/battle/15
bin/macro-audit impact __shared__:src/shared/ui/panel_task.inc:2:PANEL_ADVANCE_X
```

`account` uses `bof3.macro-candidate-account/v1`: every current opportunity in its
declared discovery scope appears exactly once, with status, blocked reason, membership,
candidate fingerprint, and live input fingerprints. `complete` means the inventory
is accounted for, not that cleanup is complete. Generated accounts authorize zero
automatic applications; validation recomputes the account and rejects drift.

`describe ID` returns `bof3.macro-resolution/v1` for one current **global** candidate.
Optional `--target` focuses source files and their lexical macro context without
narrowing the candidate fingerprint; other members remain explicit. Existing
templates are context, not proof that an occurrence expands to them or is accepted.
Unknown IDs/targets and targets without candidate members reject.

`impact DEFINITION_ID` returns `bof3.macro-impact/v1`; exact definition IDs come from
`macros` or `describe`. It follows global indexed uses and transitive definition-body
references, preserving target context through shared definitions and cycles.
`describe` embeds this impact for the definitions observed in its focused files.
Conditions, arguments, restrictions, and unresolved definition owners stay visible.
A candidate group is not the full consumer set: another parameter value or another
macro in the same writable template file may introduce additional consumers.

Impact remains `complete: false` with zero safe applications. Include order,
conditional selection, `#undef`, unindexed inputs, computed names, and token pasting
need separate review. These commands neither establish complete affected-function
coverage nor expand write authority, satisfy `all_use_sites`, or replace reviewed
artifacts. `out/` reports are disposable evidence, never reviewed truth by themselves.

## Semantic review

Before resolving a candidate, inspect the original instructions, proposed C shape,
parameter mapping, all affected use sites, and whether an existing definition
already expresses the useful abstraction. Do not extract another template just
because a byte group exists. Keep target-local wrappers, declarations, maps, and
reviewed boundaries; shared bodies must not leak target addresses.

The reviewed artifact must resolve eight guards with explicit evidence:

| Guard | Review question |
| --- | --- |
| `evaluation_count` | Are arguments evaluated the intended number of times on every path? |
| `side_effects` | Are updates and observable effects preserved without unsafe repeated arguments? |
| `integer_promotions` | Do widths, signedness, conversions, overflow behavior, and comparisons agree? |
| `precedence` | Do parameter and whole-expression parentheses preserve the intended parsing? |
| `lvalue` | Are assignment/address-taking requirements preserved? |
| `volatile` | Are volatile qualifications, access counts, widths, and ordering preserved? |
| `aliasing` | Are pointer/object assumptions and accesses justified? |
| `control_flow` | Are branches, exits, evaluation order, and statement context preserved? |

The observations `parameter_mapping` and `all_use_sites` also require independent
review. For `assembly_block`, a third nonempty observation, `human_value`, must
justify why a human would choose this macro against the readability criteria above;
omitting it rejects preparation. Legacy lead artifacts retain their two observations.
Empty evidence, unresolved guards, or an unaccepted artifact reject.
AI ranking is not independent semantic acceptance; byte equality alone is not a
readability judgment or proof that arbitrary future macro arguments are safe.

## Reviewed application

For bounded cleanup, `run` and `revalidate` accept the original absolute `--deadline` work cutoff;
the parent separately supervises its retained cleanup tail. See
[owner work deadlines](harness.md#owner-work-deadlines) for expiry, rollback and
retained late-publication evidence. Check-only revalidation never restores source;
other macro subcommands do not gain this flag.

```sh
bin/type-audit baseline
bin/macro-audit prepare out/reviews/macro-request.json out/reviews/evidence/macro-manifest.json
bin/macro-audit run out/reviews/evidence/macro-manifest.json out/reviews/macro-changes.json out/reviews/evidence/macro-application.json --implementation-run-id IMPLEMENTATION_RUN
bin/macro-audit verify out/reviews/evidence/macro-application.json --expected-application-digest APPLICATION_DIGEST
```

`type-audit baseline` is the shared workspace-baseline owner. On a dirty worktree,
explicitly adopt its exact current digest; adoption never excuses later drift.

The request (`bof3.macro-transaction-request/v1`) identifies `target`, `concern`,
`candidate_artifact`, nonempty canonical `affected_functions`, and applicable
`adopted_baseline`. Concerns are `constant`, `expression`, `local_template`, and
`shared_template`; local templates require independently exact wrappers.
For an `assembly_block` artifact, include positive integer `block_min_instructions`
in the request so preparation/rederivation reproduces its configured candidate
account. Also supply `ranking: {path, expected_ranking_digest}` referencing the
retained report under `out/reviews/evidence`. The candidate and its fingerprint must
be selected, with the same floor and a global or matching target focus. Preparation
and pre-write rederivation validate fresh ranking; execution context retains the
ranking file's state so later history verification detects deletion/replacement
without reranking historical PRE against intentionally changed POST sources.
The block supports local/shared template concerns, not constant/expression
conversion. It does not weaken owner coverage, exact-wrapper proofs, or parent review.

The artifact (`bof3.reviewed-macro-opportunity/v1`) has exactly `schema`,
`candidate_id`, `candidate_fingerprint`, `concern`, `owners`, `owner_fingerprints`,
`semantic_guards`, `observations`, `review`, and `digest`. Guard statuses are
`resolved` or `not_applicable`, with nonempty evidence; review must be accepted by
a named independent reviewer. V1 remains existing-only: private paths must already
belong to one manifest target.
Shared owners must be sanctioned templates/public headers, with proven wrapper
dependencies and the separate private-proof contract below.

Preparation currently rejects **known omitted consumers** of existing macros in
writable owner/affected-source files. It follows indexed transitive uses and maps
header uses to claimed includers; those sources must be in `affected_functions`.
The guard is conservative for the whole writable file, not just a planned macro
body edit, and run-time rederivation repeats it before writes. It is a necessary
coverage check, not completeness proof for new definitions or unindexed/dynamic
uses. Semantic review and explicit scope remain mandatory.

Macro dependency checks follow literal quoted and angle-bracket includes in one
transitive traversal across the configured build's `src/`, `include/`, and
`toolchains/psyq/4.7/include` search roots. Quotes first search the including file's
directory; angle includes do not. Compact directives, comment separators, and
backslash-newline continuations are recognized; commented-out directives are not
dependencies. Repository containment and cycle detection remain enforced.
This closes the reproduced angle-include and cross-root-chain
omissions; it does not evaluate conditional selection or computed include names.
Do not treat the traversal as complete preprocessor binding or an `all_use_sites`
proof. Other include-walker clients retain their quoted local/root/include defaults.

The changes JSON maps each permitted repo-relative path to its complete replacement
text. `run` rederives the manifest, confines writes, runs pinned checks, retains
immutable receipts, and rolls back ordinary failures after confirmed native process
cleanup. Unconfirmed descendant cleanup retains POST and recovery backing and
stops for parent inspection; see [shared lifecycle rules](harness.md#policy-versus-mechanism).
Generated files are not hand-edited. Retain the application digest externally before `verify`; never derive
an expected pin from the untrusted file being checked. Local append-only
attestation detects replacement, not a remote signature or malicious local writer.
Prepare/run outputs must be canonical repo-relative paths under
`out/reviews/evidence`; general `out/reviews` paths are insufficient. Use fresh
filenames to preserve evidence: preparation can overwrite an existing destination
and does not enforce that freshness recommendation.

Context-bearing native runs bind source/header/config/binary, resolver catalog
(including absence), tooling, modes, adopted baseline, index, environment, native
tools, and Ninja transitions before edits, around gates, and at publication. Only
explicit replacements may alter authoritative inputs; permissions survive.
Configured tools, explicit-source companion-free owners, and Ninja are required.
No installation or automatic index recovery follows. Omitting the implementation
ID retains the legacy route, which cannot supply context-bearing parent acceptance.

Native asm-diff/byte-match receipts must identify the correct target, function,
source, address, original binary, and size, with positive exact semantics—not merely
exit zero. JSON stdout and stderr remain separate. Splat/build use their native exit
contracts; existing partial baselines are pinned to exact output. `verify` checks
integrity/native evidence, **not independent final acceptance**.

Application and revalidation gates use the shared bounded process owner: 120
seconds and 2 MiB of captured native output per command. Timeout or overflow
fails the gate (exit 124 or 125), never supplies positive evidence. Owner-death
process cleanup does not itself prove file rollback; interrupted transactions
require owned-PRE reconciliation before reuse.

Source rollback can resume identity-verified partial restoration; see the
[shared recovery contract](harness.md#guarded-source-recovery). Neither rollback
nor guarded recovery grants source acceptance or an autonomous attempt budget.
Ordinary failure restores only captured owned source images; unexpected workspace
edits, new files and Git staging remain untouched for parent review, not overwritten
as presumed tool side effects. See the [shared recovery contract](harness.md)
for post-rollback verification and metadata limitations.

Macro `run` and `revalidate` share a fail-fast repository writer lease with type
transactions. It spans live manifest validation through gates, publication and
rollback; a competing cooperating writer rejects before source mutation. The
persistent writer lock is not removed on release. Detected identity loss stops
guarded restoration rather than overwriting another writer's work. This does not
exclude manual editors or establish that an interrupted native worker terminated;
see [shared lease contracts](harness.md). Read-only commands acquire no lease.

Macro runs persist a `bof3.transaction-recovery/v3` record before source mutation
at `out/reviews/evidence/macro-recovery-<nonce>.json`. It retains PRE source images,
mode/inode facts, exact prepared POST identity/mode, manifest/run bindings and
reserved PRE/POST quarantine destinations. POST images are staged in fresh
directories on the destination filesystem and installed by verified native
no-replace moves. Capture failure prevents source writes but may retain images.
Records require independently pinned inspection, not automatic restoration. They do
not replace application receipts, parent acceptance or publication reconciliation.

Git-backed runs also retain untouched workspace PRE images/metadata and exact
index bytes/state from the owner's existing snapshots. Regular-file archival
reuses captured bytes; dependency verification recaptures its evidence. Inventory covers tracked/unignored files except
the shared generated-artifact exclusions; missing snapshots mean unavailable
guards, not a clean workspace. Records can include local unignored content and
must remain nonpublic. Repository evidence uses filesystem-native permissions,
not enforced POSIX modes; see [the shared policy](harness.md). A pending index lock
rejects capture.

```sh
bin/macro-audit inspect-recovery out/reviews/evidence/macro-recovery-NONCE.json --expected-recovery-digest INDEPENDENT_PIN
```

Inspection validates record integrity and structural bindings, reports PRE,
exact v2/v3 POST, staging/displaced POST, missing/drifted files, quarantine identity and publication
presence, and does not print source images. It neither re-derives the manifest
against live inputs nor establishes an atomic snapshot,
writer termination/exclusion or publication authenticity. Exit zero does not
authorize restoration or a retry. Historical v1 records retain content-only POST
inspection; v2 has no workspace/index guards. V3 reports scoped workspace drift,
index agreement and locks without printing archived bytes. A matching guard is
not restoration authority or complete atomic verification. Historical records
are not upgraded.

For explicit parent-authorized source-only restoration:

```sh
bin/macro-audit recover out/reviews/evidence/macro-recovery-NONCE.json --expected-recovery-digest RECOVERY_PIN --authorization out/reviews/evidence/recovery-authorization.json --expected-authorization-digest AUTHORIZATION_PIN
```

The [shared authorization schema and recovery gates](harness.md#guarded-source-recovery)
require independent parent-pinned authority, reviewed terminal writer evidence,
matching v3 workspace/Git guards and known absent publication. This command
restores original owned PRE; it never resolves the macro opportunity, accepts
source, consumes a ranked attempt budget or permits automatic retry. Unexpected
publication and workspace/Git drift require separate reconciliation.

### Reviewed private-header creation

Parent confirms `bof3.reviewed-macro-opportunity/v2` admission for one new
target-private `local_template` header is committed in `c9f884b6`, with independent
review and 165 passing checks. The [authoritative plan](../plans/autonomous-bof3-decompilation.md#reviewed-private-header-creation--2026-09-13-acceptance-pending)
owns application scope, pins, consumed budgets, review identities and check receipts.
The user explicitly reopened the same macro after prerequisite commit `452498ad`;
the [reopening checkpoint](../plans/autonomous-bof3-decompilation.md#battle-dispatch-macro-reopening--2026-09-13)
records fresh PRE approval and its consumed owner entry. The original failure and
expired bounds remain historical; no automatic retry or allowance reset follows.

Parent accepts the exact frozen [submodule isolation candidate](harness.md#submodule-isolation-candidate)
after independent Pasteur PASS within its documented support ceiling. Both repairs
are consumed; the original six findings and all three repair-one residuals close.
Immutable root/dependency mirrors, observational safeguard v3 and historical v1/v2
readers remain; unbound rollback and direct receipt/attestation guards now close
their gaps, and empty unignored embedded roots reject independently of descendants.
Bound-image rollback remains independent of a corrupted index.
The [successor checkpoint](../plans/autonomous-bof3-decompilation.md#submodule-isolation-successor--2026-09-13-acceptance-pending)
separates final gates, historical failures and unrelated test debt. Observations remain
non-atomic, manual concurrency is not excluded and root Git metadata is trusted.
Arbitrary Git layouts/index-extension confinement remain unassessed; no extension
escape or projection hardening is established. The root/ten-module checksum-valid
v2 TREE inventory does not prove other formats. The full goal is active/incomplete;
this macro is blocked on the actual `fuseblk` move capability. A same-leaf
`RENAME_NOREPLACE` probe falsely passes while distinct moves fail; the hard-link
fallback leaves two links, correctly rejected before source publication. Correcting
the probe only improves diagnostics. No recovery record, owner command receipts,
application, header or POST acceptance followed; empty rollback maps prove no
restoration. PRE and the adopted baseline remain unchanged. An ext4 working copy
is requested but unauthorized; no migration or portability acceptance is inferred.
Dependency snapshots grant no restoration/repair, retry, acceptance, budget reset
or evidence rebinding. Native/application review and frozen obligations remain.
V1 stays existing-only; shared-header and type creation are not added.

V2 retains all v1 evidence fields and adds exactly one `creation` object with
`target`, `header`, `header_text`, `manifest_before` and `manifest_after`.
The reviewed digest pins complete header contents and both complete manifest
texts. Only appending that header to the target's `headers` list may change
manifest facts; identity and every other field remain unchanged. `owners` contains
only that absent header and its `owner_fingerprints` value is null. The canonical
`include/` path must end in `_internal.h`, have an existing parent directory and
no existing file or manifest claim. This planned transition does not relax the
ordinary live manifest loader's existing-file requirements.

The transaction binds the exact reviewed header and manifest POST texts plus
affected sources. Planned-POST validation must normalize relative include aliases,
follow literal quoted/angle include closure, require all new-header consumers to
equal the affected source set, cover support-source ownership, reject cross-target
reach and reject new macro identifiers already present in existing C/header/template
inputs. Structural checks retain exact target, null header PRE and configuration
PRE-hash bindings.
Literal traversal remains conservative evidence, not preprocessor binding;
independent all-use-site, semantic, human-value and exact-wrapper review survives.

Existing common recovery already records absent PRE;
`preflight_existing_replacements` skips absent paths. Existing-only owner review
was the admission blocker, not replacement preflight. Publication retains staged
no-overwrite installation and identity-safe rollback to proven PRE bytes or
absence, preserving unexpected concurrent changes. Missing PRE membership in
aggregate history is not absence proof and grants no history reconstruction or
shared promotion. Guarded retained-history verification checks original `None`
PRE; generic revalidation uses POST as adopted PRE and `structural_manifest`
without rederivation, not fresh creation admission. Its exact retained POST shape
rejects ordinary run. Parent-reported read-only probes verify these boundaries,
including rejection of missing historical membership; they do not prove fresh
full acceptance or native revalidation. No dummy header scaffolding or manual
extraction substitutes for the reviewed
transaction. Later native gates, independent application review and parent
confirmation remain mandatory before claiming an applied macro.

## Parent acceptance and replay

```sh
bin/macro-audit review APPLICATION out/reviews/evidence/reviewed.json --parent-attestation PARENT_JSON --expected-application-digest APPLICATION_DIGEST
bin/macro-audit final-verify out/reviews/evidence/reviewed.json --expected-envelope-digest ENVELOPE_DIGEST
```

After a distinct reviewer inspects the application and native receipts, the
supervising parent supplies a closed `bof3.macro-parent-review/v1` object:
`schema`, `accepted: true`, distinct nonempty `parent_run_id`,
`implementation_run_id`, `reviewer_run_id`, `review_artifact: {path, sha256}`,
`binding`, and `preservation`. The artifact is canonical absolute retained nonempty
reviewer evidence; the implementation ID matches the captured run. Preservation has
exactly `scope`, `body`, `abi`, `range`, `index`, `adopted_baseline`, all literal true.
No acceptance is inferred from prose or supplied by the descriptor.

Binding has exactly `application_digest`, `application_proof_digest` (including
original attestation), `manifest_digest`, `request_digest`, `review_context_digest`,
`post_state_digest`, `native_receipts_digest`, and `adopted_baseline_digest` (the
context's full adopted baseline). Structured digests are `v1:` plus SHA-256 of
compact sorted-key JSON; retained artifact hashes cover their bytes.

The closed `bof3.macro-reviewed-application/v1` envelope contains `schema`, original
`application`, `parent_review`, and `digest` over the other fields. Final verification
requires the externally retained envelope digest and rechecks owner proofs, receipts,
current closure, reviewer evidence, and unrelated adopted workspace preservation.
Identical read-only replay is valid. Reuse for changed state, another owner, or
another application rejects; verification never rewrites proofs, reruns gates, or
reapplies changes. Historical success is not current acceptance after tooling drift.

To resume inspection of the original published work without applying it again:

```sh
bin/macro-audit resume ORIGINAL_MANIFEST APPLICATION --expected-manifest-digest MANIFEST_PIN --expected-application-digest APPLICATION_PIN --implementation-run-id ORIGINAL_RUN
bin/macro-audit resume ORIGINAL_MANIFEST APPLICATION --expected-manifest-digest MANIFEST_PIN --expected-application-digest APPLICATION_PIN --implementation-run-id ORIGINAL_RUN --reviewed-envelope REVIEWED_ENVELOPE --expected-envelope-digest ENVELOPE_PIN
```

The first form returns `needs-review` only after current owner verification. The
second returns `skip-accepted` only for the same complete application, including
attestation, with valid current parent acceptance. Wrong request/run, changed
state, missing context, invalid acceptance or incomplete pin pairs reject. See
[request-bound resume](harness.md#request-bound-resume) for shared boundaries.
Neither form performs extraction, consumes/resets a ranked budget, establishes
writer termination or supplies a durable skip token. The bounded actor must retain
its original queue and externally pinned proofs; autonomous marching remains open.

## Private revalidation and shared promotion

Shared-template preparation requires two or more distinct declared `shared_targets`,
including the primary target, and one `exact_function_proofs` entry per target.
Every proven exact wrapper must appear in `affected_functions` and receive POST
`asm-diff` and `byte-match` checks, including when lexical indexing finds no uses.
Each pin has exactly `path`, `target`, `selector`, `expected_envelope_digest` and
references an independently reviewed private exact-wrapper envelope under
`out/reviews`. Retain original bytes/hash, native checks, parent acceptance, and
matching address-free semantic/parameter contracts. Integrity-only application
pins and legacy `expected_application_digest` shared pins reject. Representative
proofs never authorize edits affecting uncovered consumers on any target.

For private proofs requiring fresh check-only evidence:

```sh
bin/macro-audit revalidate PRIVATE_ENVELOPE out/reviews/evidence/check.json --expected-envelope-digest PRIVATE_PIN --execution-run-id FRESH_RUN --adopted-baseline CURRENT_BASELINE --intervening ORDERED_PINS_JSON
bin/macro-audit verify-revalidation out/reviews/evidence/check.json --expected-revalidation-digest CHECK_PIN
bin/macro-audit review-revalidation out/reviews/evidence/check.json out/reviews/evidence/fresh.json --expected-revalidation-digest CHECK_PIN --parent-attestation FRESH_PARENT_JSON
bin/macro-audit final-verify-revalidation out/reviews/evidence/fresh.json --expected-envelope-digest FRESH_PIN
```

Omit `--intervening` for still-current private results. Otherwise supply the ordered
closed `{envelope, expected_envelope_digest}` list with full retained private
objects and external pins. Each original POST/intervening PRE and final/current
state must agree, including native build closure, tools, environment, index, and
unrelated adopted state. Only reviewed changed paths explain authoritative drift;
original owned POST bytes/modes stay intact. Build changes are accepted only inside
recorded owner `bin/build` transitions, never gaps. New check-only gates must leave
the resulting state unchanged. Unexpected writes require explicit recovery.

Revalidation uses a fresh distinct execution ID and returns separate
`bof3.macro-application-revalidation/v1` evidence; `checked: true` is not acceptance.
The owner APIs are `revalidate_application`, `verify_revalidation`,
`review_revalidation`, and `verify_reviewed_revalidation`.

Fresh parent review uses `bof3.macro-revalidation-parent-review/v1`, the same
identity/artifact/preservation fields, and binds implementation ID to the new check
execution. The reviewer differs from original review identities and retains a new
artifact; the supervising parent may remain. Binding has `revalidation_digest`,
`prerequisite_envelope_digest`, `manifest_digest` (full revalidation manifest),
`review_context_digest`, `pre_state_digest`, `native_receipts_digest`, and
`adopted_baseline_digest`. The closed `bof3.macro-reviewed-revalidation/v1` envelope
contains `schema`, `revalidation`, `parent_review`, and `digest`. Replay rechecks
original acceptance, intervening history, and current closure without rewriting
proofs, publishing files, or rerunning gates.

Distinct-target private sequences must declare the same sorted
`run --participating-targets TARGET TARGET [TARGET ...]` before every original execution.
New macro manifests bind `participation_limit` to the configured catalog size;
the selected scope must include all mutation targets and requires an implementation
ID. Manifests without this field retain their legacy two-target limit; types retain
their separate policy. This captures evidence only: it adds no writable paths or checks
for another participant. Revalidation inherits participation; missing historical
scope cannot be expanded retroactively. Explicit adoption does not excuse drift.

Shared parent acceptance is supported only through the fresh-revalidation
branch and its owner-validated shared PRE/POST transition. Original private
application envelopes alone remain preparation-only; fresh private envelopes are
shared PRE prerequisites, not shared POST acceptance. Other branches remain guarded.

The fresh shared PRE branch requires one reviewed revalidation per declared target,
with distinct execution IDs, identical explicit full target capture scope, and common
current native inputs, environment, index, build, and workspace state. Mixed fresh
revalidation/original application envelopes reject. Complete nested envelopes and
external pins remain frozen alongside `shared_pre` state/build/baseline. Shared
`run` requires a new implementation ID and the identical `--participating-targets`
binding; it rederives the manifest and compares captured PRE before any edits or
gates. Omitting bindings cannot downgrade to legacy execution. Canonical absolute
parent-review artifacts use their evidence validator, not native-input path rules;
other absolute evidence is not silently excluded.

Shared POST acceptance uses `review`/`final-verify` with a shared implementation and
independent reviewer not reused from any private prerequisite or fresh revalidation,
a new retained reviewer artifact, and an externally pinned shared envelope digest.
Replay verifies all original/intervening histories, original/fresh parent reviews,
native evidence, and the frozen common PRE separately from the authorized shared
delta, new gates, current POST/context, and unrelated adopted workspace. It neither
compares private closure directly to intended shared POST nor substitutes current
facts for missing history. Ownership/exact-wrapper restrictions remain; participation
is evidence scope, never write authority. Local parent/runtime attribution is not
cryptographic authentication or automatic acceptance of a live BOF3 promotion.

Outputs for fresh checks/reviews must be new canonical repo-relative paths under
`out/reviews/evidence`. Commands transport the workflow; they do not schedule it,
infer approval, recover the index, or schedule a bounded attempt campaign.

## Existing abstractions

Do not unshare and re-extract useful existing macros to manufacture application
receipts. The separate unchanged-assessment lifecycle can establish that a frozen
opportunity already has a suitable abstraction. It never reconstructs missing
private applications or authorizes shared promotion.

```sh
bin/macro-audit prepare-existing REQUEST > MANIFEST
bin/macro-audit check-existing MANIFEST --expected-manifest-digest DIGEST \
  --implementation-run-id RUN --deadline CUTOFF --output out/reviews/evidence/CHECK.json
bin/macro-audit review-existing CHECK PARENT_REVIEW --expected-inspection-digest DIGEST \
  --output out/reviews/evidence/DISPOSITION.json
bin/macro-audit verify-existing DISPOSITION --expected-envelope-digest DIGEST
bin/macro-audit account-existing REFERENCES
```

`bof3.macro-existing-request/v1` requires exactly `schema`, `target`, sorted distinct
`targets`, `candidate_artifact`, `candidate_id`, `candidate_fingerprint`, sorted
distinct `definition_ids` and `affected_functions`, `adopted_baseline`, and a nonempty
`rationale` explaining why the existing abstraction is worthwhile and no edit is
needed. Optional `block_min_instructions` and `ranking` retain the assembly-block
floor and pinned top-N gate. Templates require at least four candidate uses;
constant/expression-only leads remain outside this route.
Non-block candidates reject supplied block-floor/ranking fields rather than retaining
unchecked pins.

Use the existing reviewed-opportunity schema, with `shared_template` for multiple
targets or `local_template` for one. All eight semantic guards, parameter mapping
and all-use-site observations remain required. The audit freezes every candidate
member separately from the larger consumer/check scope. Every covered function
must already be exact, and each declared target receives target gates. Known
direct and transitive consumers of every selected definition must be covered,
including consumers outside the candidate and different parameter values. This
unchanged audit freezes whole owner files but does not edit their other macros;
mutation transactions still require whole writable-file consumer coverage.
Every consuming function identity must be covered; ambiguous/unresolved mappings
reject even when another function in the same source is covered.

Literal include paths and lexical use matches provide binding leads, not expansion
proof. Independent review must establish actual definition selection, evaluation
counts, side effects, conversions and all use-site behavior; inspecting an unrelated
macro in the same source file is not evidence that the candidate is already solved.

Warm the required native build before freezing. Checking accepts no source changes
and runs owner-derived native gates under writer exclusion. PRE/POST source, index,
tooling, environment, full adopted workspace and captured build closure must remain
unchanged, including across every gate and publication. Outputs are exclusive new
files under `out/reviews/evidence`; evidence-path reuse rejects before native work.
Adopted entries also retain observed file modes/types; this does not require
restrictive permission bits or authorize changing permissions. `--deadline` is the
original absolute monotonic cutoff, not a fresh duration. Failed gates, unexpected
writes or cleanup uncertainty grant no source rollback authority: preserve state
for separately scoped recovery.

Checking returns `checked:true`, not acceptance. Parent review uses
`bof3.macro-existing-parent-review/v1`, the existing complete application-style
binding and preservation fields, three distinct actual run identities and a retained
nonempty absolute reviewer artifact. The opportunity reviewer must be independent
of the execution and parent. Externally retain manifest, inspection and disposition
pins at each handoff. Final verification rederives scope/checks and validates current
closure and receipts without running native commands or rewriting evidence.

Only final verification returns `accepted:true`, `existing_abstraction_count:1`
and `safe_application_count:0`. `account-existing` consumes an explicit list of
`{path, expected_envelope_digest}` references, rejects duplicate candidate IDs and
counts existing work separately. It does not change generated candidate inventory.
The frozen-five adapter has an explicit macro-only `existing` claim with a
`{disposition: Path, expected_envelope_digest}` proof. Its original baseline/pilot
freshness gates still apply; this route cannot re-pin stale frozen inputs.

These schemas reject at mutation application, revalidation, private exact-proof,
promotion and resume entry points. A current unchanged audit cannot supply the
historical private/shared transaction chain required for future edits.

## Implementation owners and checks

All paths below are under `tools/python/harness/`:

| Responsibility | Owner |
| --- | --- |
| Lexical definitions/uses | `macros/facts.py` |
| Literal include traversal | `domain/includes.py`; macro search policy in `macros/review.py` |
| Macro registry, schema and queries | `macros/index.py`, `macros/schema.py`, `macros/queries.py` |
| C-pattern and reviewed-byte leads | `macros/opportunities.py`, `macros/groups.py`, `macros/similarity.py` |
| Repeated instruction blocks | `macros/blocks.py`, `macros/repeats.py` |
| Accounting, inspection, consumer graph | `macros/accounting.py`, `macros/resolution.py`, `macros/impact.py` |
| Human-value selection | `macros/ranking.py`; CLI wiring in `macros/selection.py` |
| Reviewed artifact, known-consumer check, transaction | `macros/review.py`, `macros/coverage.py`, `macros/transactions.py` |
| Reviewed private-header admission | `domain/headers.py`, `macros/creation.py`; admission/application in the review and transaction owners above |
| Parent acceptance | `macros/application.py`; shared review/revalidation in `common/` |
| Existing-abstraction assessment and disposition | `macros/assessment.py`, `macros/disposition.py`; CLI in `macros/inspection.py` |
| CLI | `macros/cli.py` owns `bin/macro-audit` and macro `rev-query` parsing/adapters; `commands/rev_query.py` composes domain registrations |

The [harness layout](harness.md) and [Python standards](coding-standards.md)
own module structure. Evidence schemas, target identity and native gates do not
change merely because an implementation owner moves. Tooling-bound historical
proofs still require their original closure or authorized fresh revalidation.

Existing checks live in `tools/python/tests/test_macro_facts.py`,
`test_macro_index.py`, `test_macro_opportunities.py`, `test_macro_accounting.py`,
`test_macro_transactions.py`, and the reverse-index/command suites. Native acceptance
still requires live `bin/asm-diff`, `bin/byte-match`, applicable target checks, and
independent review. Green tooling tests do not establish worthwhile macros or
whole-codebase 1:1 source completion.
