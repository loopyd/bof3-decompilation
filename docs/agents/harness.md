# Harness ownership

Read [the documentation index](../INDEX.md) first. This map owns harness modules
and lifecycles; [coding standards](coding-standards.md) own naming and decomposition.

## Domain packages

All paths are below `tools/python/harness/`.

| Package | Owns | Entry points |
| --- | --- | --- |
| `macros/` | lexical facts, assembly/C opportunities, ranking, consumers, reviewed transactions and unchanged existing-abstraction dispositions | `bin/macro-audit` → `harness.macros.cli` |
| `combiner/` | inspection, compiler preservation, joint images, grouped comparison and rollback-only rehearsal/recovery; ranking/permanent application remain queued | `sh bin/combiner` → `harness.combiner.cli`; [rollout contract](combiner.md) |
| `naming/` | symbol naming opportunities, identity inventory, evidence collection, audits, proposals, application and acceptance | `bin/naming-audit` → `harness.naming.cli`; `bin/naming-evidence-run` → `harness.naming.runner` |
| `types/` | C declarations, representation inference, type-use/candidate indexing, reviewed type transactions | `bin/type-audit` → `harness.types.cli` |
| `docs/` | scoped Markdown references, snapshots, search, context, aggregation and edit/repair/compaction preparation | `bin/docs` → `harness.docs.cli` |
| `decomp/` | lift inventory/mission pins, source scope, deterministic native diagnosis and candidate audit | `bin/agent-run diagnose` / `audit` → `harness.decomp.cli` |
| `common/` | reusable CLI, digests, confined files, process lifecycle, workspace, native receipts and acceptance mechanisms | direct imports from the mechanism owner |
| `analysis/` | cross-domain reverse index, graph, mission and query coordination | `bin/index`, `bin/rev-query` |
| `domain/` | manifests, target identity, original binary/layout, source claims and includes | shared repository facts, not candidate acceptance |

Each domain uses noun files such as `index.py`, `queries.py`, `review.py`,
`transactions.py`, `application.py`, and `cli.py`. Naming's finer categories include
`collection.py`, `journal.py`, `namespace.py`, `equivalence.py`, and `terminal.py`.
Package initializers are inert: import the actual owner, not a compatibility facade.
Parent [lift diagnosis](codex.md#parent-lift-diagnosis) and [audit](codex.md#retained-lift-audit)
use `decomp.diagnosis`, `audit`, `missions`, `inventory`, `gates`, `execution` and
`evidence`; `decomp.scope` owns directory/source policy and `common.journal` owns
exclusive records and fsynced streams. Shared boot/cutoff checks live in
`common.deadlines`. Neither gate command writes source or grants acceptance.
The [lift-loop skill](../../.codex/skills/bof3-lift-loop/SKILL.md) instructs the
active session to delegate domain missions and independently review outcomes.
The harness never discovers, configures or launches Codex/model processes. Retired
transport records remain historical; their writer/reviewer slots cannot be reused
as skill missions. Local process supervision remains for deterministic native tools.
The cross-domain index remains one derived database; package separation does not
create independent databases or relax freshness checks.

`domain/declarations.py` owns namespace-qualified declaration entities and their
source occurrences; `domain/c_context.py` parses statements and closes context
dependencies through that model. Tags and same-spelling typedefs remain distinct.
Aggregate completion retains forward provenance and definition-owned fields;
pointer/array aliases do not inherit pointee layout. `types/representation.py`
resolves explicit identity aliases for field queries and assertions. Constraints
bind ordinary type identities, retain unresolved evidence and expose contradictions
instead of overwriting layout facts. Type derivation changes require rebuilding
the versioned index, not relabeling old rows. Parsed declarations and assertions
remain representation evidence, not native byte-match or transaction acceptance.

`naming.instructions.resolve_instructions` includes the target-qualified selector
when refusing capture without a closed reviewed function boundary. The diagnostic
does not admit raw/unreviewed caller ranges or relax the capture gate.

FUNCTION identity native checks cover the selected function and every changed C
caller derived from frozen `source_locations`. `application.collect_function_checks`
requires one canonically valid exact record per caller and matching resolver
source/symbol; producer and validator share `plan(root, target, row)`. Public
snapshot preparation runs these checks before physical-PRE publication and reports
invalid caller metadata with its path. Native exactness does not excuse malformed
progress tags; repair them separately before a future identity transaction.
Caller evidence binds selector, name, source, positive aligned original size and
exact current bytes. Target compilation alone is insufficient; metadata does not
promote raw ranges to reviewed boundaries.

`domain.functions` owns attached per-function metadata, address-selected read access
and lexical implementation ranges, including template invocations. `domain.claims`
enumerates all members and rejects duplicate owners; the index scopes lifecycle and
direct macro occurrences. Shared comment/literal traversal belongs to `common.lexicon`.
The [combiner rollout](combiner.md) still must migrate layout, native, naming/type/
macro coverage and mutation owners before production consolidation is enabled.

`domain/cache.py` owns complete `config/targets/**/*.toml` discovery and process-local
manifest reuse. Default lookup hashes live manifest and claim contents and checks
canonical paths before reusing parsed data. An explicit `read_manifest` callback
receives repository-relative names and supplies immutable bytes for parsing/cache
keys; it does not authorize trusting stale physical inputs. `ProfileContext` binds
that reader to its guarded captured bytes, keeps the original manifest inventory,
and rechecks discovery plus file metadata/content before publication. PRE/POST
inventory checks use the same domain owner; alternate TOML names are not hidden.
Build inventory v4 retains this complete TOML set and raw hashes, including edits
with restored timestamps, plus each source's effective function compiler settings.
It also binds object flags and the variant catalog as
bounded raw hashes or verified absence; missing and empty files differ. The
frontend and CMake discover alternate TOML names; inventory capture precedes the
flags include and target reads, with a final endpoint check. Older snapshots
reject. Defaults remain unchanged; producers share strict literal configuration
parsing. These checks do not
establish configure-time continuity, atomic helper reads or full compiler closure.
Each load must consume exactly that inventory once, including cache hits; prior
context inputs cannot stand in for this load's missing or unexpected reads.
Descriptor-based discovery never suppresses a failed subtree scan or follows
directory symlinks. Nonregular TOML inputs reject; only an absent initial target
tree is empty. Limits are 16,384 visited directories and 65,536 total entries;
aliases, traversal failures and deadlines reject instead of truncating the set.
The cache, manifest parser, target-ID and confined traversal owners are also pinned.
Missing fields, wrong table/value shapes and numeric overflow during one captured
model's parsing become path-qualified `ValueError` failures. This boundary does
not catch manifest/claim reader exceptions or deadlines; accepted model values,
cache isolation and per-load claim validation are unchanged.
`load_manifest_generation` returns each load's isolated models and immutable claim
sample together; there is no global latest-claims lookup. Every load validates
binary/source/header claims, including parsed-model cache hits. Default reads keep
pass-local parent resolution and stable regular-file samples; only absent paths
are optional, while escapes, nonregular inputs and I/O failures reject.
Only the shared reader's verified missing-path result becomes absence; errors
after a present-input observation, during reading or at final checks propagate.
Dangling aliases cannot bypass containment; contained aliases retain ordinary
lookup semantics, and captured parent/alias resolutions are rechecked on return.
An explicit claim reader is a trusted sample provider: bytes alone do not prove
physical containment, regularity or freshness. It supplies bytes or `None` for
absence. Profiles enforce those physical guards through their input reader and
capture every consulted claim once with its exact per-load set; placement
bounds use sampled lengths, never a later binary stat. Final profile verification
still rechecks physical inputs. Each manifest capture and verification pass uses
a fresh descriptor-confined `InputBatch`. It caches directory handles, not byte
samples, absence or validation success; context-owned immutable samples remain
separate. Batch canonical checks precede path observations without repeating
`Path.resolve`; parent links are rechecked before bytes, leaf metadata brackets
each read, and directory linkage/metadata are verified before descriptors close.
Directory allocation size follows the existing profile policy; leaf sizes remain
bound. Eviction never resets original directory observations. Canonical,
single-link, ancestor-membership and manifest-inventory guards remain mandatory;
no batch or original profile context is carried across output mutations.
Profile inspection projects Splat boundaries from its captured text; preservation
validation reuses one sample for strict decoding and projection. `parse_splat_text`
performs no I/O and hashes the supplied UTF-8 text. Its `origin` labels diagnostics,
not freshness. The file parser retains its existing newline normalization;
raw CRLF samples therefore retain their distinct digest. Final input checks stay.
Profiles also pin the layout, PS-X, source-claim, function/tag, symbol-map and C
lexical owners, plus shared I/O, confined file/path and deadline helpers.
Preservation requires every observed input, including optional absence, in the
retained PRE-plus-POST map: initial omissions and later unbound reads reject.
The initial retained-input read loop also uses a fresh `InputBatch`, closed before
semantic validation. State/hash/mode and optional-absence comparisons remain;
each verification still rereads inputs and runs its complete guards. No batch,
content or validation-success cache crosses passes or output mutations.
Newly required owners require fresh capture/review, never current-state backfill.
These explicit repository owners do not establish imported/runtime, PyYAML,
stdlib, compiler-backend or include closure, or authenticate already-loaded code.
Object flags and compiler IDs are parsed from captured override text. Variant
selection lazily parses the captured catalog once per profile context; unused
catalog schema is not validated early. Missing catalogs differ from empty files,
unknown configured IDs reject, and defaults/ordered arguments stay unchanged.
Ordinary compiler file loaders retain their behavior through pure parsing owners;
neither parsed variants nor their context may survive output mutations.
Source-ownership scans consume captured C text, and compiled names reuse the
canonical parser's captured target-local map once per profile/verification pass.
Explicit text/map providers are trusted data, not freshness evidence; ordinary
domain callers retain file reads. Metadata, collision, map/boundary agreement and
final physical/input-set checks remain mandatory. No shared/SDK map substitutes
for target-local names, and supplied empty maps never trigger file fallback.
Repository layout consumes the returned claim
sample, but its later file/layout reads are not an atomic repository snapshot.
Expanded claim capture makes repeated preservation proofs more expensive; no
cross-sample validation-success cache or relaxed producer deadline is implied.

`common.observation.observe_directory` binds bounded, nofollow membership to an
opened directory and checks path linkage. Profiles retain raw directory metadata;
only size may differ with the same other fields and membership. File checks and
latched mutation watches stay exact; membership endpoints do not prove history.
`PathWatch(paths, directories=...)` additionally latches every immediate child
event in explicitly named existing directories, including create/delete/rename
and mutation-restore sequences. Default selected-path filtering is unchanged.
Directory inputs share the bounded input/ancestor budget and nofollow linkage
checks; missing directories, overflow and interrupted event drains reject.
This is not recursive discovery or content capture: callers must enumerate each
dependency directory, establish watches before sampling, and verify afterward.
No existing producer gains complete include/toolchain closure from this primitive.

`DirectoryBatch` reuses membership hashes only within one verification pass under
a live `PathWatch(..., namespace_only=True)` fence. This opt-in ignores unrelated
child content/attribute events, not namespace, selected-path, ancestor or watch-loss
events. Every observation still samples raw directory metadata; values remain
provisional until successful batch exit validates events and linkage. Failure
latches, exit closes descriptors, and closed batches cannot revive. Unwatched
directories, including the filesystem root, retain full uncached observations.
`ProfileContext.verify()` uses this batch without skipping file reads/hashes,
single-link checks, ancestor comparisons or either manifest-inventory check.
No membership sample or validation result survives the pass.

`common.inputs.read_input` returns state and the exact bytes hashed under one
stable metadata sample; `file_state` retains its state-only schema. Profile capture
uses those bytes without a duplicate read, but verification always reopens and
rehashes inputs. All file mutation fields, ancestor symlink checks and profile-only
single-link checks remain enforced. Only `ENOENT`/`ENOTDIR` denote absence;
permission, I/O and malformed-path failures reject.
Absence is rechecked after the ancestor walk; newly present entries reject.

`build.receipts` owns grouped producer receipts: `build-adapter producer` always
compiles under the writer lease, while `build-adapter receipt` requires an external
receipt SHA and freshly checks configured provenance. Receipts cannot authorize
reuse, native equality or deadline compliance. Translation binds every supplied
dispatch to the requested root/source and validates it before reading metadata,
including ordinary-source passthrough. It reuses the bounded compiler source
reader rather than reopening text without an admission limit; later dispatch
checks and grouped ownership/partition guards remain mandatory. Source names are
compared resolved without relaxing original dispatch-path validation; newline
normalization is unchanged. Entry/return checkpoints cover ordinary passthrough.
`build.arguments` separates source operands from ordered explicit `-include` and
`-imacros` inputs. Forced files remain captured and collision-protected but never
select a translation unit or its metadata; missing files/operands reject. Original
arguments stay unchanged. Recognized spellings do not prove compiler support,
preprocessor ordering or transitive dependency closure.
Stage mode/argument rewriting respects forced operand spans; dispatch inputs also
participate in the existing preallocation temporary-family collision guard.
`build.invocation` supplies the immutable recipe consumed by dispatch
and driver. `build.execution` captures successful process, partition and publication
edges and verifies them against that recipe without recreating deleted staging.
The driver returns evidence in-process after cleanup; producer v7 and comparison v6
bind it separately from live invocation identity. Profile compatibility remains
independent of source/output paths; older restoration records remain versioned history.
Producer v7 persists forced-input roles; older receipts are not upgraded or admitted
as fresh provenance. The invocation recipe shape is unchanged.
Receipt production lends its live dispatch to the synchronous compiler call. The
driver checks the exact root/arguments before staging and retains every validation
checkpoint; only the receipt owner closes the borrowed dispatch. Standalone calls
own their dispatch. This removes duplicate preparation, not fresh verification,
and grants no concurrent sharing, persistent reuse or retry after uncertain cleanup.
`build.programs` derives executable candidates, declared scripts and the process
supervisor from those same stages. `common.lookups` walks raw spellings through
held nofollow directory descriptors, preserving alias and absence evidence.
Typed repository/external observations grant no external writes. Dispatch watches
and recaptures this bounded generation; its fingerprint transitively binds execution.
Profiles retain the same configured graph and verify it afresh, not a persistent
validation-success cache. Verification can seed watches from a copied, validated
current-schema graph bound to the same root, then require a full fresh capture to
equal it, with watch checks before and after. Ordinary discovery still captures
before establishing watches. This removes duplicate discovery, not content checks;
endpoint descriptions grant neither continuous inter-call coverage nor authority
to trust an unauthenticated baseline. `build.runtime` derives versioned source-pin-driven
candidate edges from declared Stage policies; `build.observations` validates
retained direct/runtime graphs and causal replay without live reads. Provider
settings participate in profile cache keys; recapture rederives the expansion.
The canonical GCC policy pins the captured driver binary and binds its effective
GCC_EXEC_PREFIX, COMPILER_PATH and PATH. It records initial specs and cpp/cc1
candidates, including machine/version prefixes, compiled default roots and PATH
fallbacks; repeated roots retain their order. Alternate binaries and search-control
options remain explicit unresolved cases, never default substitutions. Existing
policy replay and historical graph shapes remain unchanged; live owner pins and
stage fingerprints require fresh evidence after policy edits.
These observations do not prove effective import, shell or GCC selection.
Executable suffixes, specs expansion, post-specs prefixes, other language backends
and preprocessor inputs remain unresolved; native and consolidation gates stay closed.
See [program observations](combiner.md#program-observations)
for limits and unresolved runtime/backend dependencies.
`build.inventory` owns configured
classification and its always-run graph gate; `config/compiler/graph.cmake` owns
unique lift nodes, with grouped producers forced and ordinary commands retained.
`build.routing` selects externally pinned, source-bound preservation records for
mixed ordinary/grouped builds without changing the caller environment. Explicit
single-record and routing-table inputs conflict; neither silently overrides the
other. The selected record still passes the existing preservation checks.
Preprocessing operands share explicit spans across source classification, profile
flag checks, recipe rewriting and runtime-control inspection: joined/split
`-D/-U/-A/-I`, plus exact `-isystem`, `-idirafter`, `-iprefix`, `-iwithprefix`
and `-iwithprefixbefore`. Values are not source files or switches; `-I-` stays
distinct from directory operand `-I -`. Generated output slots retain position.
Only a stage's own output Artifact permits two generated slots/characters beyond
the unchanged user-argument bounds; other opaque values or roles reject.
Stored flag fragments keep historical validation, including trailing options;
only complete invocation parsing imposes operand completeness and CLI limits.
This is not a complete GCC grammar, include-search model or native support claim.

`build.dependencies` owns digest-bound current/retained/prospective source images
for profile resolution and conservative literal snapshots. Virtual seed bytes and
physical include reads stay distinct; comparison records both kinds of evidence.
Configured profiles capture explicit repository-local `-include`/`-imacros` files
against the invocation's CWD, including when replaying retained source images.
They join the unchanged PRE input set: missing or changed files reject, and no
header edits are added to the POST allowlist. Old records lacking these inputs
cannot pass fresh verification; historical decoding is unchanged. This does not
resolve transitive headers, forwarded options, default paths or compiler support.
Grouped comparison seeds its literal snapshot from dispatch's physical forced
files too. Their descendants and negative lookups join mutation/output guards;
captured non-toolchain bytes pass matching-policy checks before production.
Comparison's protected inputs also exclude compiler staging, publication-temp
and quarantine families through shared driver/program guards before production.
Cold staging selection uses the first accessible POSIX temp candidate without
write probes or cache changes; a cached tempfile directory keeps precedence.
Allocation failure propagates rather than trying another directory. These checks
do not reserve names or freeze directory/alias changes before allocation.
This bounded union is not effective preprocessing or transitive PRE preservation.
Full input closure and native/transaction integration remain open.
See the [combiner contract](combiner.md) for supported entrypoints, invocation,
invalid markers and retained recovery handling.

`combiner.images` owns joint text/mode preparation; `combiner.history` owns strict
transaction v1–v4 history and their exact owner sets. Current preparation emits v4
with runtime/observation owners pinned and preservation v7/profile v5. Historical
v1/v2 retain preservation v5/profile v3; v3 retains preservation v6/profile v4.
Retained profile versions use
`build.preservation.validate_profile_history`, independent of current admission
schema constants; new captures still use `validate_transition`'s current-version
gate. Recovery keeps manifest v1 and validates retained bindings without live PRE.
Live transaction verification/rehearsal rejects historical v1–v3; recapture v4 rather
than rewriting old pins. No schema support grants restoration or source authority.

The decomp-status batch path loads a fresh catalog after each successful build and
reuses it only during read-only source resolution. Ownership winner/tie rules stay
unchanged. Per-source fallback builds and native comparisons reload ownership;
the phase catalog is not passed across those subprocess boundaries or stored in
resolved results. This avoids repeated full claim validation without weakening
post-build comparison checks.

Link bindings use a supplied resolved manifest's target ID and PsyQ space even
when a custom weak-binding source lives elsewhere. Its filename does not change
canonical map ownership. Without a manifest, legacy directory inference remains;
explicit canonical bindings, including an empty mapping, still suppress map reload.
Address-encoded names and per-symbol weak-file fallback keep their precedence.
This lookup rule is not source authority or native match evidence.

Macro, naming and type `cli.py` modules register their `rev-query` subcommands and own their
argument-to-query adapters. `commands/rev_query.py` composes those registrations,
checks index freshness and target identity, and prints their results. Naming
inventory remains manifest-backed without requiring an index; transaction scope
retains its indexed target check and labeled output. Macro and type table schemas
live in `macros/schema.py` and `types/schema.py`, composed by `analysis/schema.py`.

`naming/opportunities.py` owns legacy inventory projection and read-only raw-symbol
leads. `naming/debt.py` owns raw-spelling classification; the opportunity collector
parses one target-map snapshot through the canonical domain parser. Its CLI lives
in `naming/cli.py`, not an analysis adapter or type module. Discovery fingerprints
bind map bytes and row identity, not semantic evidence, layout or application
authority. See [naming opportunities](tool-usage.md#symbol-naming-opportunities).

### Naming preparation

Naming scope gathers all other-target seeds before one transitive local-include
traversal. It still scans the complete resulting set for cross-target spellings;
each scope call rederives it. No persistent include cache or freshness exemption
is introduced.

Live-proven metadata repair preserves semicolon-delimited progress annotations as
adjacent comment text while canonicalizing the fields. Notes are not progress
authority; an annotation containing another progress tag is rejected, not erased.

`naming.audit.prepare_transaction(check_only=True)` uses the same validation and
report lock as publication, including prospective full-report provenance checks,
but leaves the bound report in memory. It checks the original report bytes and
inherited deadline before returning `checked:true`, `prepared:false`; the CLI
exposes this as `prepare-transaction --check`. The sibling lock may be created;
source, report and receipt publication remain disabled. This catches candidate
schema/scope defects before independent review without granting acceptance or
rebinding existing proposals. See the [invocation contract](tool-usage.md#frozen-naming-postapply-lifecycle).

`naming.snapshot.create_snapshot` owns physical PRE capture for a pinned prepared
FUNCTION/DATA transaction. It composes `inputs.frozen`/`transaction_paths`,
`common.inventory.capture_file`, actual-index capture and
`history.read_snapshot_states`: exact source scope, bytes/modes/absence and frozen
report/reviewed/index pins. It checks retained copies before manifest-last
publication, then the manifest and live inputs/index/report/deadline before return.
Copies cover selected transaction paths only; `inputs.state` conservatively hashes
the native build closure, including unrelated source paths. It does not read
`.pi/settings.json`. Source and report bytes remain unchanged.
This evidence is not macro/type recovery or unattended restoration authority.

`naming.review.prepare_attestation` packages an explicitly pinned parent decision
through the existing attestation validator. It derives bundle bindings/digests,
requires actual distinct run IDs and retained review/snapshot proofs. After
exclusive publication it revalidates bundle/source/execution pins, the attestation,
parent-decision/report/gates pins and deadline. Both producers finally compare
published byte SHA-256 with the originally encoded payload, then recheck writer
lease/deadline. Valid replacement JSON cannot substitute another reviewer identity
or snapshot manifest. Failures retain artifacts, including manifests, for
inspection without implying validity or restoration authority.
Neither producer infers acceptance or preservation from prose.
`naming.cli` only adapts `snapshot` and `prepare-review` arguments
and prints artifact paths; existing ingestion/public verification remain mandatory.
Both operations hold `common.lease` writer exclusion and the nonblocking naming
report lock. These cooperative checks neither exclude manual editors nor authorize
source mutation, index recovery or automatic restoration. Schemas and invocation
belong to the [naming lifecycle](tool-usage.md#frozen-naming-postapply-lifecycle).

## Policy versus mechanism

`commands.binaries` owns executable materialization, shared by setup and
`just binaries`; the latter restores existing extracted media without setup or
downloads. It validates original PS-X headers through `domain.psx` and writes only
the payload to `out/binaries`, matching reviewed Splat coordinates and hashes.
Correct payloads stay untouched; exact legacy full-image copies are normalized.
Unexplained differences require explicit `--force` on the module CLI. Replacements
use expected-content publication and report retained quarantine paths; extracted
originals stay unchanged. EMI restoration stays with `emi.catalog_bootstrap`.

`common.git.read_git` bounds workspace/index snapshot queries by 30 seconds,
2 MiB and the inherited absolute work cutoff. It retains filename bytes and
propagates cleanup uncertainty; timeout or truncated output is never a baseline.
Gitlink discovery groups HEAD and index-path queries, but still compares fresh HEAD
and physical index snapshots before/after reading staged and committed entries.
No persistent cache or subprocess-supervision bypass is used.
Type/macro exception handlers suspend only the forward cutoff around owned rollback
and `common.safeguards.verify_restored_state`. They still require the parent's cleanup
hard-stop; this grants no new budget, forward gates, retry or restoration authority.

The read-only cleanup context router selects one existing domain skill:

| Canonical request | Selected skill |
| --- | --- |
| `macro-opportunity TARGET ID` | `bof3-macros` |
| `type-opportunity TARGET ID` | `bof3-types` |
| `naming-opportunity TARGET ID` | `bof3-naming` |

Pass one form to `bin/agent-context cleanup`. The router checks a known target
and one nonempty printable ID token, retains that opaque ID unchanged, and loads
only the selected body. It performs no opportunity query, index rebuild, ranking,
transaction or approval. Candidate existence, target membership, freshness and
caller-retained fingerprints remain owner gates before preparation/application.
Canonical re-derivation rejects inconsistent fields or skill selection, not a
different internally valid request; it does not authenticate the caller's original
target/ID. The parent retains those bindings independently. Existing
`type TARGET OLD -> NEW` remains an identity route, not type representation work.
Likewise naming-opportunity is read-only assessment, not whole-target
`audit-target TARGET` or the approved `symbol TARGET OLD -> NEW` identity route.
All three now select `bof3-naming`, with only the mode's direct references loaded.
Its audit, transaction, retained-lift and relocation contracts live in that one
skill tree; the old naming-evidence and identity-maintenance skills are retired.
One owner does not collapse authority: audit may write authorized disposable
evidence, not identities; only approved transaction modes may edit repository truth.
Explicit-only invocation policy is preserved. Macro/type skills keep their own
domain lifecycles; type spelling remains separate from type representation.

Cleanup context and saved-proposal history must remain importable under the
wrapper's stdlib-only `-S` bootstrap. `common.inputs` loads domain claims/includes/
manifests only inside `input_state`; `naming.proposal` imports `TargetContext` only
under `TYPE_CHECKING` and loads context helpers inside live `validate_proposal`.
Deferring imports preserves live ownership/closure validation and dependency
failures when those operations run; it does not bypass guards or alter the wrapper.

Macro extraction policy belongs in [macros.md](macros.md). Naming evidence and
identity-application contracts are linked from the documentation index. Type
candidate/application schemas remain in
[tool usage](tool-usage.md#type-parent-review-and-final-verification).

Macro `assessment`, `disposition` and `inspection` own the separate unchanged
audit lifecycle. It reuses native receipts and execution capture but cannot supply
mutation/revalidation/private-promotion proofs. Existing dispositions count apart
from applications; `common.evidence.write_new_evidence_output` publishes exclusively.

Common mechanisms preserve existing wire schemas and trust boundaries, including
historical `bof3.type-*` defaults where callers already depend on them. They do not
approve symbols, infer layouts, choose macro abstractions, install tools, refresh
an index or recover failed transactions automatically. Domain adapters supply the
owner and validation policy. Similar-looking path or digest checks are not
interchangeable unless their semantics agree.

`common/process.py` owns application/revalidation command lifetimes and bounded
capture. Naming postapply and macro/type native gates use 120-second deadlines
and retain at most 2 MiB of native output per command. Supported native ownership
requires Linux; other platforms reject before spawning rather than claim process
groups provide complete descendant closure. `common/children.py` establishes a
fresh single-threaded subreaper before launch. Owner-pipe EOF or direct-child exit
triggers killing and reaping adopted descendants, including redirected-stdio and
`setsid` escapees. The sole reaper retains child identities until wait, preserving
direct-child exit status.

After `_main` finishes cleanup and closes the completion pipe, the supervisor
flushes its available Python output streams and exits directly with the retained status.
This avoids interpreter-finalization overhead for short commands; it does not
skip child reaping, the cleanup ACK, output capture or deadline checks. Exceptions
before this point retain Python's failing-exit path. No supervisor state is pooled.

The bounded supervisor observes its direct child's exit through libc
`pidfd_open` readiness where available; unsupported or denied capability retains
the existing 50 ms polling fallback. Readiness neither reaps descendants nor
replaces the private cleanup ACK; descriptor errors retain guarded cleanup.

A private completion pipe, not inherited by gates, acknowledges only finished
cleanup. `poll`, `wait`, `communicate` and terminal `returncode` require that ACK.
Timeout/overflow produces failed gate evidence (124/125) only after confirmed
cleanup. Missing ACK or cleanup wait expiry raises `ProcessCleanupError`: callers
do not kill the still-cleaning guardian and pretend its descendants stopped. Type
and macro owners retain POST and recovery backing instead of restoring source,
workspace or index while writers may remain. Naming evidence propagates uncertainty
without ordinary failure receipts or final journal/telemetry/report publication;
both owned clients still receive cleanup attempts.

Ordinary naming collection failures retain the error and full last-operation
command, selector, exit, killed state, raw output and stderr in a failed receipt.
That forensic record never commits a successful row or closes semantic work.

Naming lifecycle `validate`, `prepare-transaction`, `snapshot`, `postapply-gates`,
`prepare-review`, `postapply-review`, `verify`, `finalize-transaction` and
`terminal-verify` accept one `--work-deadline`: the original absolute monotonic
cutoff. Shared CLI dispatch binds the stricter inherited/supplied cutoff, restores
the caller's context on exit and rejects late success. These are cooperative guards
and owned-subprocess budgets, not asynchronous Python cancellation or automatic
rollback; always inspect exit status, not just emitted JSON. Cleanup retains its
separately frozen tail, never charged by extending work time.

Before autonomous identity application, `snapshot --reserve-seconds SECONDS`
requires a positive reserve under a bound work cutoff. Freeze a realistic remaining
native/index/independent-POST/finalization budget, with margin, in the mission; do
not compress review to fit a nearly spent window. Admission checks precede capture
and output creation and recur before publication/return. Expiry may retain
unaccepted PRE artifacts but grants no source permission. A snapshot is not durable
timing admission: recheck the same remaining-time reserve immediately before editing.
Insufficient time defers before application; never reset consumed attempts or
rebind a failed transaction. Reserve estimates cannot guarantee completion.

The guardian can remain active when termination cannot be confirmed; callers stop
for parent inspection, not automatic retry. Lease release/reacquisition alone is
not quiescence. This mechanism is not a sandbox, cross-platform guarantee or proof
against privileged interference. Injected runners do not prove native lifecycle
behavior. Native cancellation can bypass Python rollback: process closure is not
file restoration. Parent-owned native handle/tree verification, source/workspace/
index inspection and reviewed owned-PRE recovery remain separate obligations.

`common/lease.py` excludes cooperating writers, including macro/type application
and revalidation and naming preparation, in one repository. For macro/type, a
nonblocking `flock` spans canonical manifest
re-derivation, source application, native gates, publication and exception
rollback. The persistent `out/reviews/evidence/transaction.lock` is an empty,
owned single-link regular file; root, path and inode checks reject
substitution. Nested writers reject. Release closes the descriptor without
unlinking the lock, and exec'd native gates do not inherit it. Invalid requests
may create this lock artifact before domain validation, but cannot thereby edit
sources. Detected lease loss stops subsequent guarded source/Git restoration;
retained artifacts require explicit recovery.

This is cooperative exclusion, not a repository-wide security boundary. Naming
report-set locks, manual editors and other nonparticipating commands remain
separate. Direct low-level helpers do not acquire a lease. Neither a free lock
nor its file establishes prior native-worker termination or restoration authority.
Read-only proof verification and recovery inspection acquire no writer lease.

Repository locks, staging and evidence use filesystem-native permissions. Creation
requests restrictive modes where supported, but no fixed POSIX mode is enforced
on NTFS. This does not guarantee confidentiality: keep source-bearing records out
of public logs and commits. Source/PRE/POST mode capture and drift checks remain
part of identity-preserving rollback, not an owner-only permission policy.
Recovery records, completion receipts and plan review artifacts use creation hints
without requiring `chmod`. Explicit captured-mode restoration remains strict.

### File publication

The [in-place NTFS checkpoint](../plans/autonomous-bof3-decompilation.md#in-place-ntfs-publication--2026-09-13-reviewed-tooling-checkpoint)
records parent acceptance of the independently reviewed repaired implementation. The user
refuses migration and authorizes only this cooperative fallback, not home, mount,
settings or sandbox changes. `common.rename.publish_file` first attempts strict
native `RENAME_NOREPLACE`; collisions retain native no-replace behavior. Only an
unsupported platform/filesystem result selects cooperative publication: open the
regular single-link source without following links, exclusively create an empty
destination reservation, compare source/reservation descriptor and path identities,
rename over the checked reservation, then verify source absence and published
identity and fsync both directories. Rename-sensitive ctime is excluded from POST
comparison; a displaced reservation's descriptor link count is not a FUSE invariant.

This fallback is not atomic CAS against noncooperating writers. An external writer
can race source/reservation checks before rename or the reservation-cleanup check
before unlink; undetected substitution can affect foreign names. Detected foreign
names are preserved. Readers can observe the empty reservation, and crashes can
leave reservations, prepared images or displaced files. Failure after movement or
sync does not imply no effect: retain evidence and inspect actual locations before
reviewed recovery. Cleanup requires an initially regular, empty, single-link
reservation whose captured descriptor/path identity remains unchanged, subject to
that check/unlink race. Nonempty or multiply linked captures are ineligible;
zero length alone never authorizes deleting crash residue. No race-free or
power-loss guarantee follows.

`common.files` creation, quarantine/restoration and `common.git` index moves use
this publisher. Retained-hard-link publication and its alias return field are
removed; same-leaf preflight no longer claims native capability. Source/images
stay single-link, prepared images stay on the destination filesystem, and source
bytes/mode/identity, root confinement, leases, submodule exclusion and guarded
rollback remain required. Shared `common.rename.describe_locations` supplies
files/Git publication and restoration errors with observed names and uncertainty,
never inferred retained source after post-move failure. `safe_unlink` classifies failed movement by device/inode,
mode, link count, size and mtime, excluding ctime. This changes publication support,
not restoration authority or the scope of workspace/Git guards.

### Recovery capture

Native tool cancellation can bypass Python exception handlers entirely. Macro/type
transactions now use `common/recovery.py` to persist
`bof3.transaction-recovery/v3` evidence before the first source mutation.
`common/quarantine.py` reserves exact displaced-file destinations in advance.
The record binds root identity, owner, manifest, implementation run ID (nullable
for legacy applications), publication path, PRE bytes/hash/mode/inode, intended
POST hash/exact mode/inode, staging path and reserved PRE/POST quarantine mapping.
`common/images.py` prepares POST images under fresh staging directories before
record capture, then installs the bound inode through [file publication](#file-publication).
Images must share the destination filesystem and retain one link. Failed capture
may retain unreferenced images, but cannot mutate sources.
Shared `common.runtime.apply_changes` additionally accepts an exact `deletions`
path set and `null` content for those paths only. The default remains text-only;
extra, undeclared, unowned or already-absent deletions reject before source mutation.
Deleted files move to their reserved PRE quarantine, never to an empty placeholder.
Mixed edits capture `bof3.transaction-recovery/v4`: each deleted path has present
PRE and `post: null`; replacements retain prepared POST images. Text-only capture
remains v3. The common owner vocabulary includes `combiner`, but this mechanism
does not supply its domain transaction, audit, native gates or recovery CLI.
`common.execution.applied` can bind absent POST only with an exact, canonical,
manifest-owned `deletions` set and captured present PRE. Other calls remain
text-only. A retained file, absent PRE, mismatched deletion set or unrelated input/
build drift rejects without advancing the context. This is execution bookkeeping,
not deletion authority or combiner acceptance; [joint preparation](combiner.md#joint-transaction-preparation)
binds proposed images separately and remains read-only.
`common/directory.py` owns confined descriptor traversal. Files and directory links are
synced before mutation; capture failure aborts before moving sources.

`common/safeguards.py` reuses the owner's existing workspace/index snapshots,
retaining untouched PRE bytes, hash, mode, identity, ownership and link count,
plus exact Git-index bytes/path/state. Inventory follows `workspace_backup`:
tracked and unignored files, excluding `out/`, `sessions/subagent-artifacts/` and
`.pi/subagents/`. Changed owned paths remain covered by their source images;
unchanged owned paths remain in the workspace inventory. A pending index lock
rejects capture. Non-Git or low-level runs without snapshots explicitly lack these
guards; missing data is not treated as a clean workspace. Regular-file archival
reuses captured bytes; dependency evidence is recaptured for comparison. Recovery
records grow with retained PRE content.
An exact index snapshot is not a complete Git metadata backup.

`common/links.py` captures unrelated workspace symlinks as literal bytes plus
identity through confined no-follow parent traversal, never reading referents.
Link evidence binds bytes, mode, device/inode, ownership, link count and timestamps.
Link target/type/identity drift
rejects, including same-target replacement. Macro/type publication rechecks captured
links; snapshots grant no restoration authority, unrelated links are never restored,
and symlink-owned mutation remains refused.

### Submodule isolation candidate

The [successor checkpoint](../plans/autonomous-bof3-decompilation.md#submodule-isolation-successor--2026-09-13-acceptance-pending)
records parent acceptance of the exact frozen candidate after independent Pasteur
review: PASS with no confirmed open findings, within the support ceiling below.
Both repairs are consumed; the original six findings and three repair-one residuals
are closed. This accepts only the submodule prerequisite. The whole-game goal is
active and incomplete; the separate publication prerequisite is accepted, while
C/macros retain their own readiness/review gates. The prior rejected implementation
remains historical.

| Common owner | Responsibility |
| --- | --- |
| `submodules.py` | HEAD/index gitlink boundaries, recursive dependency snapshots, marker/absence binding and unconditional mutation exclusion |
| `repositories.py` | independent root/module metadata copies, sanitized configuration/environment and bounded scratch Git queries |
| `inventory.py` | confined file/namespace observations, identity rechecks and shared capture budgets |
| `trees.py` | scratch worktree inputs, literal links, supported status semantics and live-input rechecks |
| `workspace.py`, `safeguards.py` | composed baseline, dependency verification and durable observational guards |
| `recovery.py`, `runtime.py`, `images.py`, `evidence.py` | dependency exclusion before source/image/evidence writes, even without optional workspace/index snapshots |

`inventory.describe_identity` omits directory allocation size: NTFS/FUSE can report
different sizes for an unchanged directory during observation. Directory membership,
device/inode, mode, link count, ownership and modification/change times remain bound;
non-directory size and file-content hashes remain exact. This removes an unstable
storage detail, not namespace, content, confinement or mutation checks.

New dependency captures use `bof3.submodule-snapshot/v3`. Historical v2 records remain
structurally readable, but live observation verification requires v3, including
absent/uninitialized dependencies. Never rewrite historical records or their external
pins to claim compatibility; recapture and independently rebind a new transaction.

Root status and workspace backup use an independent metadata/worktree mirror;
submodule-suppression flags alone did not prevent live dependency reads. Separate HEAD/index
enumeration protects both current and staged-removed gitlinks; baseline composition
retains staged changes, module HEAD-versus-gitlink differences, tracked changes and
recursive untracked state. Clean modules remain absent from the dirty map but
present in preservation evidence. Absent, empty uninitialized and initialized
modules remain distinct; marker selection and final namespace/identity observations
are rechecked. Final raw observations recursively recheck the captured aggregate
after nested queries, without interpreting live Git data. Nonempty uninitialized
modules reject. Root mirrors retain the generated-artifact exclusions even for
tracked paths.

Initialized modules use independent scratch files, never hardlinks or redirects
back to live metadata. Confined capture supplies refs, index/object dependencies
and configuration before module-state queries. Scratch config parsing disables
includes; queries disable hooks, fsmonitor, recursive status, optional writes,
implicit fetching and host attribute/exclude files. Supported configuration values
are reconstructed explicitly. Metadata and host configuration are separately
rechecked against their live origins after queries; scratch consistency alone is
insufficient. Host configuration is parsed initially, then freshly recaptured with
environment, control-path, absence, identity, mode and content/hash checks before
unchanged inputs can avoid reparsing. This is not a cross-transaction cache:
fresh Git/workspace boundaries remain mandatory. See the
[reviewed performance checkpoint](../plans/autonomous-bof3-decompilation.md#transaction-validation-performance--2026-09-13-reviewed-tooling-checkpoint)
for measurements and remaining validation debt. Captured module `*.lock` files
are observed, excluded from query
copies and never deleted. The superproject index-lock capture refusal still applies.
Optional multi-pack-index and commit-graph files retain raw observations but are
not copied or interpreted; scratch queries disable both accelerators. Their
presence alone does not reject capture: the real root contains a commit graph.

Worktree mirrors capture regular `.gitignore`/`.gitattributes` controls and
tracked/unignored content; literal symlink bytes are compared without referent
reads. Linked controls reject even though ordinary literal links are supported.
Ignored undeclared embedded `.git` directories stay opaque and are never queried;
their enclosing namespace is observed, not a complete ignored-content backup.
Unignored undeclared repositories reject, including empty roots independently of
descendant selection. Metadata hooks/logs/modules/worktrees
directories are excluded from that metadata scan; declared nested modules receive
their own bounded capture.

Supported settings include file-mode, autocrlf/eol normalization, ignore-case,
symlink and Unicode handling, repository format and SHA-1/SHA-256 object format.
Unknown status-affecting settings reject, including configured filters, config
includes, external attribute/exclude controls, submodule-ignore settings and
nonempty global ignore/attribute files. Unsupported layouts include external or
linked Git controls, bare/shared-commondir worktrees, worktree-specific config,
alternates, grafts/loose or packed replacement refs, promisor metadata, unmerged indexes and
sparse/assume-unchanged entries. This is an explicit support ceiling, not arbitrary
Git-layout compatibility; nonrecursive boundary discovery still reads root HEAD/index.

One recursive capture budget permits 200,000 inventory charges, 2 GiB of observed
bytes and 512 queries, including rechecks; charges are not unique-file counts.
Regular inputs cap at 128 MiB each, metadata at 256 MiB, gitfiles at 4 KiB,
namespace depth at 64 and module ancestry at 32. Each isolated query has 30 seconds,
2 MiB output, 1 GiB address space, 30 CPU seconds and zero file-output allowance,
bounded further by the inherited work cutoff. Failure/overflow is never a baseline.
The root mirror has a separate budget from recursive module capture.

`bof3.recovery-safeguards/v3` adds `gitlink` entries containing canonical
`bof3.submodule-snapshot/v3` evidence; historical submodule v2 and safeguard v1
file/v2 file-link records remain readable without rewriting their bytes or pins.
Enclosing transaction-recovery v3/v4 formats retain their existing scopes.
Macro/type publication rechecks dependencies. Boundary/descendant/ancestor
mutation overlap rejects before images,
including low-level calls without optional guards. Image/evidence destinations are
checked before directory creation; application, `common.revalidation` and
`macros.disposition` artifact guards run before writer-lease acquisition. Bound-image
owned rollback uses captured identities without depending on a concurrently
corrupted index; unbound rollback separately enforces dependency exclusion.
Direct receipt/attestation destinations are also guarded; subsequent external-state
checks still require parent review. Snapshots grant no dependency
restoration, reset, initialization, deinitialization or repair authority. Drift
preserves dependency state for parent review; observations remain non-atomic.

Acceptance does not exclude manual concurrent edits; root Git metadata remains
trusted. Arbitrary Git-layout compatibility and arbitrary index-extension confinement
are unassessed: no extension escape was demonstrated and no projection hardening
is claimed. The inspected real root and ten root-module indexes contain checksum-valid
v2 TREE extensions only; that inventory proves nothing about other formats.

### Recovery evidence and rollback

Records live at `out/reviews/evidence/{macro,type,combiner}-recovery-<nonce>.json`.
They contain source text: retain them as nonpublic evidence. Their digest
detects drift against a separately retained pin; neither that digest, file mode
nor writer PID authenticates restoration authority or proves writer termination.
For new files, creation mode remains `0644` masked by the process umask; v2/v3/v4 bind
the resulting exact mode. Exception rollback checks POST/PRE identities.

Ordinary type/macro failure restores only the transaction's captured owned source
images. Unexpected workspace or Git-index changes are not proof of ownership:
never overwrite them or quarantine newly observed user files. After owned rollback,
compare allowed PRE bytes, the adopted workspace, exact raw index snapshot and a
separate all-PRE safeguard snapshot (including untouched modes/identities/links).
Drift or inspection failure preserves external state and requires parent review;
it is not complete rollback or retry authority. Restore owned images before
inspecting external state, so an unreadable or foreign untouched path cannot
suppress otherwise safe owned restoration. `ProcessCleanupError` still forbids
rollback while native descendant termination is uncertain.

These snapshots are non-atomic observations, not exclusion of manual editors.
The non-Git fallback lacks workspace metadata safeguards; do not present it as
metadata-verified recovery. Durable v3 safeguards retain their separate inventory
excluding changed paths. Generic workspace/index restoration primitives remain
available only to separately authorized callers, never as an owner fallback.

Identity-bound source rollback is replayable: it requires complete backup/image
coverage, recognizes original PRE as a no-op, and resumes a missing destination
only with its original PRE quarantine. Originally absent paths remain absent.
`common/images.py` requires exactly one original POST image at the source,
staging path or reserved POST quarantine; unexpected bytes, mode, inode, links or
location combinations reject before that path is changed. PRE/POST byte equality
does not bypass identity checks. `common/observation.py` shares non-atomic file
observations with inspection; actual moves use the checked [publication backend](#file-publication)
with its explicit cooperative race limitations.
This low-level mechanism does not validate external record authority, acquire an
absent lease, reconcile publication or restore workspace/Git state. Legacy
byte-only rollback has no identity-replay guarantee. A no-op replay does not
complete interrupted directory syncing or establish power-loss durability.

V4 deletion rollback requires an absent source and its original single-link PRE
inode in the reserved quarantine; restored PRE with an absent quarantine is a
replayable no-op. Missing backing, duplicate locations or foreign replacements
reject without overwriting them. Deletions have no POST inode to authenticate.
Read-only inspection reports `deleted` only with that PRE backing, and distinguishes
lost backing as `missing` or `drifted`. Neither observed absence nor a v4 record
grants restoration authority or establishes an atomic multi-file commit.

`common/inspection.py` supplies read-only `inspect-recovery RECORD
--expected-recovery-digest PIN` to both owner CLIs. It checks the independent
pin, recovery record, root/owner/manifest bindings, PRE encoding and reserved
quarantine identities. It reports original PRE, missing, exact v2/v3/v4 POST or
drifted files, staged/displaced POST, unchanged-path hashes and publication presence without printing
source images. Manifest validation is structural, not live re-derivation.
V3/v4 inspection also reports scoped workspace additions/removals/drift, index
agreement and index locks without printing archived bytes. `matches` means only
that these scoped observations agree, not complete workspace/Git verification.
Historical v1 records retain content-only POST evidence; v2 retains exact POST
identity but no workspace/index guards. Neither is upgraded. Observations are
non-atomic; termination, exclusion and publication authenticity remain unverified.
Exit zero means inspection succeeded, not that restoration or continuation is safe.

## Guarded source recovery

`type-audit recover` and `macro-audit recover` restore owned PRE only under an
externally pinned parent authorization and a separately pinned v3 or v4 record.
`common/authorization.py` validates authority bindings; `common/restoration.py`
owns the fail-stop operation; `common/inspection.py::load_recovery` validates
backing for both inspection and restoration. No discovered record grants authority.

The supervising parent must actually verify the named native tool handle is
terminal and its writer tree has stopped, then obtain independent recovery review.
The CLI checks retained artifacts and pins; it does **not** authenticate actor IDs
or interpret tool output as proof of termination. PID reuse, a free lock or a
stopped launcher alone is insufficient. These are explicit trusted-parent
attestations, not unattended recovery or authorization for another transaction.

Authorization is a single-link regular JSON file under
`out/reviews/evidence/`, with closed schema `bof3.source-recovery-authorization/v1`:

- `schema`, `approved: true`, distinct nonempty `parent_run_id` and
  `reviewer_run_id` (also distinct from the captured implementation run ID);
- `review_artifact: {path, sha256}`: retained, nonempty, single-link review;
- `binding`: exactly `action: "restore-owned-pre"`, `owner`, captured `root`
  (`path`, `device`, `inode`), `implementation_run_id`,
  `recovery: {path, digest}`, `manifest_digest`, sorted exact changed `paths`,
  captured `publication`, and deterministic `completion` path
  `out/reviews/evidence/{owner}-restoration-{record_nonce}.json`;
- `termination`: exactly nonempty `tool`, specific nonempty string `handle`,
  `state: "terminal"`, `writer_tree_stopped: true`, and `artifact: {path, sha256}`;
- `digest`: `v1:` SHA-256 of compact sorted-key JSON without `digest`.

Artifact paths are canonical repository-relative evidence paths with byte SHA-256;
authorization, record, completion, publication and both artifacts must be distinct.
Keep the authorization digest outside the assessor's mutable artifact before use.

Recovery acquires the writer lease before live validation. It refuses historical
schemas, missing implementation identity or workspace/Git guards, guard drift,
unknown or present application publication, and inconsistent image locations.
Every owned image is checked before each transition; completed paths must remain
PRE. Each singleton rollback stops on failure rather than modifying later paths.
Authority, guards and complete owned PRE are checked again around completion.
Unexpected unrelated or Git-index changes are never overwritten.

Completion writes an exclusive private `bof3.source-restoration/v1` receipt bound
to both pins, authority binding, terminal attestation and restored PRE identities.
It omits source bytes. Identical replay requires valid receipt bytes/mode/link
count and fresh authority, guards and PRE checks; a receipt alone never proves
current success. Partial restoration or publication failure stops and retains
original evidence for reviewed retry; the original recovery record is not rewritten.
The result grants neither source acceptance nor automatic retry. Power-loss
durability, published-application reconciliation, workspace/Git restoration and
automatic worker/session recovery remain unfinished.

Moving implementation files changes tooling-bound execution closure. Do not edit
retained receipts, frozen requests or historical evidence to make old proofs appear
current. Revalidation and original-byte/ABI/source-range preservation remain gates.

## Request-bound resume

`type-audit resume` and `macro-audit resume` inspect a published application for
the original pinned manifest and implementation run. `common/continuation.py`
returns a current disposition, not a scheduler or another persisted receipt:

- `needs-review`: the owning live verifier confirms the application and captured
  context, but no accepted envelope was supplied;
- `skip-accepted`: the supplied independently pinned envelope contains this exact
  complete application, including its attestation, and owner final verification
  confirms current acceptance.

Both routes require the complete original manifest/request binding and a nonempty
matching captured run ID. Manifest checking is structural; PRE is never regenerated
from current POST. Envelope and pin must be supplied together. Invalid supplied
acceptance, stale state or mismatched pins fail rather than downgrade to review.
The result includes owner, manifest/request/run/application/proof bindings and the
envelope pin, without applying changes, rerunning gates or writing a skip token.

The parent still establishes writer termination/exclusion and preserves queue
identity and attempt budgets. This read-only, non-atomic disposition grants no
mutation or retry authority and resets no budget. Reproduce the captured launch
environment; never edit proofs or relax execution-context checks to make resume
pass. Historical integrity alone is not current acceptance. The broader bounded
execution loop remains unfinished.

Unpublished POST after `ProcessCleanupError` uses `inspect-recovery`, not the
published-application `resume` route. Retain the original budget, latest consumption
pins and failure evidence across fresh invocation. Remaining time/launches and an
available lease do not clear the parent's cleanup-unconfirmed stop. Inspection
does not authenticate termination; retain native process handles in their owning
PID namespace rather than interpreting saved PIDs in another tool invocation.
Neither API persists a cleanup hold or automatically prevents another dispatch.
The supervising parent must withhold mutation until independently reviewed native
termination permits separately authorized recovery; no implicit retry follows.

## Bounded continuation

The parent prepares review scopes and budgets under the
[standing autonomous authorization](../INDEX.md#autonomous-execution), without
another user prompt for external review or safe-checkpoint index refresh.
Validator fields do not confer authority by themselves or replace that authorization.

`common.continuation.check_budget` is a read-only validator, not a scheduler or
launch permission. Its `bof3.execution-budget/v1` contract pins the canonical
root path/device/inode, ordered unique queue IDs and fingerprints, same-boot
monotonic start/deadline, UTC start, global launch ceiling and per-entry repair
ceilings. All integer counters reject booleans. There are no implicit defaults.

Supply the externally retained budget digest, latest checkpoint digest and
sequence, plus the complete `bof3.execution-consumption/v1` chain from zero.
Each checkpoint pins the contract and previous checkpoint; each transition
consumes exactly one launch or one entry repair, never refunds or resets.
Queue membership, counters, ceilings, root and boot must remain bound.
`bof3.execution-budget-status/v1` reports remaining time/counts and exhaustion
with `launch_authorized:false`. Latestness depends on the parent's external
high-water pin: a mutually consistent stale prefix and stale pin cannot be
detected internally. Queue fingerprints are bound, not freshly re-queried.

The controller must durably pre-debit before dispatch and preserve the original
deadline across fresh invocations. Repair work that launches a process owes
both debits. No checkpoint writer, reservation service, CLI, automatic recovery
or accepted-source authority is implemented by this validator. Owner freshness,
writer quiescence and independent parent acceptance remain separate gates.

The active-session [skill operator](../../.codex/skills/bof3-lift-loop/SKILL.md)
accounts original mission bounds and actual handles. No harness model-dispatch
debit, detached controller or second campaign database owns orchestration.

`run_bounded` optionally accepts prompt `input_data` bytes, `on_spawn` and
`on_output(stream, chunk)` callbacks. Selector-driven stdin writes proceed alongside
stdout/stderr reads without a blocking initial pipe write. Output callbacks receive
only retained bytes within the combined cap. These trusted owner callbacks do not
confer child/semantic authority; callback failure still requires original-owner
cleanup, and cleanup uncertainty takes priority over the callback error.

`common.process.run_bounded(..., deadline=...)` accepts an optional finite absolute
`time.monotonic()` deadline; the earlier of that deadline and the per-call timeout
applies, including startup. Already-expired work does not spawn. The Linux
supervisor also enforces the deadline without parent polling, then reaps owned
descendants before completion ACK. Capture, wait and confirmed completion all
respect expiry. Cleanup may finish after the work deadline but grants no further
work; uncertain cleanup raises `ProcessCleanupError`, not a successful timeout
receipt. `owned_popen` accepts the same optional deadline and rejects prelaunch
expiry with `subprocess.TimeoutExpired`. Convert a validated nanosecond deadline
to monotonic seconds without replacing it with a fresh relative allowance.

### Owner work deadlines

Public `common.files.read_file` checks the inherited deadline before traversal,
before reading an acquired leaf, and after descriptor cleanup; missing-parent
success also checks expiry. Byte limits and confined path rules are unchanged.
These are cooperative checkpoints, not kernel-I/O interruption or a hard time
bound. Low-level reads used within publication/quarantine stay deadline-neutral.
`apply_changes` suspends failed forward work for its no-explicit-tail rollback;
an explicit cleanup cutoff remains bound. Direct rollback/restoration callers
still own their cleanup context; reads never silently discard that cutoff.

Type/macro `run` and `revalidate` commands accept `--deadline SECONDS`, an optional
**absolute monotonic work cutoff**, not a duration or UTC timestamp. Their
`run_transaction(..., deadline=...)` and `revalidate_application(..., deadline=...)`
APIs expose the same control. The caller must validate original budget/consumption pins and
freeze a separate cleanup hard-stop; a numeric cutoff grants no authority and is
not an execution-budget validator. Do not replace either endpoint on a retry.

`common.deadlines` binds this limit to the synchronous owner invocation before
writer-lease acquisition. Invalid/nonfinite/boolean values reject; already-expired
work raises `DeadlineExpired` without entering the writer. Nested bindings can
only tighten the current limit and reset in `finally`. The native default runner
inherits the limit, including hidden partial-function baseline gates during
manifest rederivation. It retains the earlier of the work cutoff and its normal
120-second command ceiling. Injected runner call signatures remain unchanged;
checks around their return do not make a custom runner interruptible. New threads
and subprocess entry points require explicit deadline propagation.

Forward checks surround source transitions, gate execution and proof publication.
After expiry, owned-image rollback and its safeguards run without that expired
work limit; the parent must supervise the owner until the original cleanup
hard-stop. Filesystem/Git inspection or restoration can still stall, so this is
bounded cooperation, not guaranteed automatic recovery. `ProcessCleanupError`
still prevents rollback while descendant termination is unconfirmed.

Expiry after a gate may leave no ordinary command receipt. If publication itself
crosses the cutoff, an output/attestation may remain as retained failure evidence
after source rollback. A file's existence is not success: retain native terminal
status, verify current owner state and use the existing inspection/recovery gates.
Never delete or rewrite failed publication to manufacture acceptance.

Check-only revalidation acquires no source-restoration authority. Expiry or an
unexpected gate write leaves current source for parent inspection, never automatic
rollback. A late revalidation publication can still verify against unchanged
source; mechanical verification does not prove deadline compliance or authorize
advancement. Retain the failed invocation and the parent's budget decision.

Naming collection uses `bin/naming-evidence-run ... --work-deadline SECONDS` and
`run_evidence(..., work_deadline=...)` for the same absolute cutoff. Its existing
`--deadline` remains a relative per-operation/index-request cap, not a campaign
budget. The effective collection limit is the earlier of the inherited cutoff and
the existing 600-second shard wall. `naming.session` owns client acquisition and
cleanup; index, native-byte and Rizin clients pass the cutoff to their guardians
and clip foreground waits. Cleanup attempts all acquired clients and preserves
`ProcessCleanupError` over ordinary failures; shutdown/drain is not work.

Naming checks before receipt/journal writes and finalization stop further forward
work on expiry. They do not make filesystem operations atomic or interruptible:
in-flight writes and earlier completed checkpoints may remain. Expiry skips later
manifest/telemetry/report finalization, not retained evidence; never delete it to
manufacture success. Collection still cannot approve identities or satisfy an
unfiltered full-target naming audit by itself.

Without an absolute cutoff, existing standalone limits remain. These owner controls
are not an automatic scheduler or the complete bounded Codex sequence.

## Validation

Run the existing focused domain suites, shared application/history/revalidation
checks, reverse-index/CLI checks, wrapper bootstrap checks and `test_harness_dry.py`.
Existing assertions must follow moved owners without losing behavioral coverage.
No source acceptance follows from a passing Python suite; native checks and
independent review still govern retained BOF3 changes.
