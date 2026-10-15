# Harness ownership

Read [the documentation index](../INDEX.md) first. This map owns harness modules
and lifecycles; [coding standards](coding-standards.md) own naming and decomposition.

## Domain packages

All paths are below `tools/python/harness/`.

| Package | Owns | Entry points |
| --- | --- | --- |
| `runtime/` | emulator staging, launch, completion, receipts and origin validation | `bin/harness runtime` → `harness.runtime.cli` |
| `toolchain/` | pinned tool setup, native builds and installation verification | `bin/harness setup`, `bin/harness doctor` |
| `reconstruction/` | prepared original-only inputs, native generation and independent comparison | `bin/harness reconstruction` → `harness.reconstruction.cli` |
| `macros/` | lexical facts, assembly/C opportunities, ranking, consumers, reviewed transactions and unchanged existing-abstraction dispositions | `bin/harness macros` → `harness.macros.cli` |
| `combiner/` | inspection, compiler preservation, joint images, grouped comparison and rollback-only rehearsal/recovery; ranking/permanent application remain queued | `bin/harness combiner` → `harness.combiner.cli`; [rollout contract](combiner.md) |
| `naming/` | symbol naming opportunities, identity inventory, evidence collection, audits, proposals, application and acceptance | `bin/harness naming` → `harness.naming.cli`; `bin/harness naming evidence` → `harness.naming.runner` |
| `types/` | C declarations, representation inference, type-use/candidate indexing, reviewed type transactions | `bin/harness types` → `harness.types.cli` |
| `decomp/` | lift inventory/mission pins, source scope, deterministic native diagnosis and candidate audit | `bin/harness decomp diagnose` / `audit` → `harness.decomp.cli` |
| `common/` | reusable CLI, digests, confined files, process lifecycle, workspace, native receipts and acceptance mechanisms | direct imports from the mechanism owner |
| `analysis/` | cross-domain reverse index, graph, mission and query coordination | `bin/harness analysis index`, `bin/harness analysis query` |
| `domain/` | manifests, target identity, original binary/layout, source claims and includes | shared repository facts, not candidate acceptance |

`bin/harness setup --component psyqo` replaces installer-created hardlinks in the
installed PSYQo toolchain with independent files.
`bin/harness doctor --component psyqo` verifies that each installed regular file
has one link.

`toolchain/permuter.py` owns managed decomp-permuter setup, workspace preparation
and execution. Use `bin/harness setup --component decomp-permuter` to set it up
and `bin/harness doctor --component decomp-permuter` to verify it.
`bin/harness lift permute TARGET@0xADDRESS --prepare-only` creates the function
workspace using the configured project Python and installed native tools.

Each domain uses noun files such as `index.py`, `queries.py`, `review.py`,
`transactions.py`, `application.py`, and `cli.py`. Naming's finer categories include
`collection.py`, `journal.py`, `namespace.py`, `equivalence.py`, and `terminal.py`.
Package initializers are inert: import the actual owner, not a compatibility facade.
Parent [lift diagnosis](codex.md#parent-lift-diagnosis) and [audit](codex.md#retained-lift-audit)
use `decomp.diagnosis`, `audit`, `missions`, `inventory`, `gates`, `execution` and
`evidence`; `decomp.scope` owns directory/source policy and `common.journal` owns
exclusive records and fsynced streams. Neither gate command writes source or grants
acceptance.
The [lift-loop skill](../../.agents/skills/bof3-lift-loop/SKILL.md) instructs the
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
`naming.evidence` derives `selected_call` observations from byte-validated instruction
captures using `naming.calls`. It supports canonical straight-line wrappers and
8..32-instruction acyclic BEQ/BNE callers with direct calls and forward local jumps.
The branching decoder checks every path and delay slot, settled constant-address
halfword reads, argument expressions and saved-RA stack restoration. Calls remain
opaque. A later call leaves outgoing result registers unresolved; conditional return
records state their normal-return and stack-preservation assumptions. Unsupported
instructions, load timing, loops, indirect flow and post-call dependencies remain
explicit capability gaps. `owner_body` retains its separate decoded shapes.
The selected function's complete validated capture produces `selected_range`, with
its half-open boundary, byte count, original hash and live image/layout binding.
Generated caller work controls call decoding; capturing the selected body alone
does not make it a caller. A two-way zero/nonzero halfword guard with different
first callees and an identical later call suffix produces `one_level_beyond` as
`caller_state_dispatch`. It records the state read, alternative calls and common
continuation, without assigning meaning to the state or the callees' effects.
Typed receipts retain the command, complete output, observations and refusal reasons;
journal replay re-derives them from current original bytes and generated caller work.
One receipt cannot close two rungs. Function conclusion
admission remains disabled; imported rows without caller work still need discovery.

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
aliases and traversal failures reject instead of truncating the set.
The cache, manifest parser, target-ID and confined traversal owners are also pinned.
Missing fields, wrong table/value shapes and numeric overflow during one captured
model's parsing become path-qualified `ValueError` failures. This boundary does
not catch manifest/claim reader exceptions; accepted model values,
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
lexical owners, plus shared I/O and confined file/path helpers.
Preservation requires every observed input, including optional absence, in the
retained PRE-plus-POST map: initial omissions and later unbound reads reject.
The initial retained-input read loop also uses a fresh `InputBatch`, closed before
semantic validation. Content-state and optional-absence comparisons remain;
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
cross-sample validation-success cache is implied.

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

`common.inputs.read_input` returns a `{sha256}` state, or `null` for absence, with
the exact bytes hashed under one stable metadata sample. `file_state` retains its
state-only schema. Profile capture
uses those bytes without a duplicate read, but verification always reopens and
rehashes inputs. All file mutation fields, ancestor symlink checks and profile-only
single-link checks remain enforced. Only `ENOENT`/`ENOTDIR` denote absence;
permission, I/O and malformed-path failures reject.
Absence is rechecked after the ancestor walk; newly present entries reject.

`build.receipts` owns grouped producer receipts: `build-adapter producer` always
compiles under the writer lease, while `build-adapter receipt` requires an external
receipt SHA and freshly checks configured provenance. Receipts cannot authorize
reuse or native equality. Translation binds every supplied
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
The driver returns evidence in-process after cleanup; producer v8 and comparison v6
bind it separately from live invocation identity. Profile compatibility remains
independent of source/output paths; older restoration records remain versioned history.
Producer v8 persists forced-input roles; older receipts are not upgraded or admitted
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
options remain explicit unresolved cases, never default substitutions. Retained
graphs require `bof3.program-inputs/v3`. Live owner pins and stage fingerprints
require fresh evidence after policy edits.
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
cannot pass verification and require fresh capture. This does not
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

`combiner.images` owns joint text preparation; `combiner.history` validates
transaction v5 with its exact owner set, preservation v8, profile v6 and program
graph v3. Transaction v5 pins `build.runtime` and `build.observations` with the
other owners. The history owner rejects older formats. Obtain fresh preparation
without rewriting old records or their pins. Recovery keeps manifest v1 and
validates retained bindings without live PRE. These checks grant neither
restoration nor source authority.

The `bin/harness lift status` batch path loads a fresh catalog after each successful build and
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
but leaves the bound report in memory. It checks the original report bytes before
returning `checked:true`, `prepared:false`; the CLI
exposes this as `prepare-transaction --check`. The sibling lock may be created;
source, report and receipt publication remain disabled. This catches candidate
schema/scope defects before independent review without granting acceptance or
rebinding existing proposals. See the [invocation contract](tool-usage.md#frozen-naming-postapply-lifecycle).

`naming.snapshot.create_snapshot` owns physical PRE capture for a pinned prepared
FUNCTION/DATA transaction. It composes `inputs.frozen`/`transaction_paths`,
`common.inventory.capture_file`, actual-index capture and
`history.read_snapshot_states`: exact source scope, bytes/absence and frozen
report/reviewed/index pins. It checks retained copies before manifest-last
publication, then the manifest and live inputs/index/report before return.
Copies cover selected transaction paths only; `inputs.state` conservatively hashes
the native build closure, including unrelated source paths. It does not read
`.pi/settings.json`. Source and report bytes remain unchanged.
This evidence is not macro/type recovery or unattended restoration authority.

`naming.review.prepare_attestation` packages an explicitly pinned parent decision
through the existing attestation validator. It derives bundle bindings/digests,
requires actual distinct run IDs and retained review/snapshot proofs. After
exclusive publication it revalidates bundle/source/execution pins, the attestation,
parent-decision/report/gates pins. Both producers finally compare
published byte SHA-256 with the originally encoded payload, then recheck writer
lease. Valid replacement JSON cannot substitute another reviewer identity
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

`common.git.read_git` caps workspace/index snapshot query output at 2 MiB. It
retains filename bytes and propagates cleanup uncertainty; failed or truncated
output is never a baseline.
Gitlink discovery groups HEAD and index-path queries, but still compares fresh HEAD
and physical index snapshots before/after reading staged and committed entries.
No persistent cache or subprocess-supervision bypass is used.
Type/macro exception handlers retain owned rollback and
`common.safeguards.verify_restored_state`; neither grants new attempts, forward
gates, retry or restoration authority. Check-only revalidation grants no
source-restoration authority. Unexpected gate writes require parent inspection.
Failed invocation or source rollback may leave published artifacts. Retain the
failure status and inspect current owner state; never delete or rewrite failed
publication to manufacture acceptance. Mechanical revalidation does not authorize
advancement.

The read-only cleanup context router selects one existing domain skill:

| Canonical request | Selected skill |
| --- | --- |
| `macro-opportunity TARGET ID` | `bof3-macros` |
| `type-opportunity TARGET ID` | `bof3-types` |
| `naming-opportunity TARGET ID` | `bof3-naming` |

Pass one form to `bin/harness agent context cleanup`. The router checks a known target
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
The `symbol`, `type` and `repair` identity routes now select `bof3-naming`, with only the mode's direct references loaded.
Its audit, transaction, retained-lift and relocation contracts live in that one
skill tree; the old naming-evidence and identity-maintenance skills are retired.
One owner does not collapse authority: audit may write authorized disposable
evidence, not identities; only approved transaction modes may edit repository truth.
Explicit-only invocation policy is preserved. Macro/type skills keep their own
domain lifecycles; type spelling remains separate from type representation.

Cleanup context and saved-proposal history must remain importable under the
`bin/harness agent context` stdlib-only `-S` bootstrap. `common.inputs` loads domain claims/includes/
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
capture. Naming postapply and macro/type native gates retain at most 2 MiB of
native output per command. Supported native ownership
requires Linux; other platforms reject before spawning rather than claim process
groups provide complete descendant closure. `common/children.py` establishes a
fresh single-threaded subreaper before launch. Owner-pipe EOF or direct-child exit
triggers killing and reaping adopted descendants, including redirected-stdio and
`setsid` escapees. The sole reaper retains child identities until wait, preserving
direct-child exit status.

After `_main` finishes cleanup and closes the completion pipe, the supervisor
flushes its available Python output streams and exits directly with the retained status.
This avoids interpreter-finalization overhead for short commands; it does not
skip child reaping, the cleanup ACK or output capture. Exceptions
before this point retain Python's failing-exit path. No supervisor state is pooled.

The bounded supervisor observes its direct child's exit through libc
`pidfd_open` readiness where available; unsupported or denied capability retains
the existing 50 ms polling fallback. Readiness neither reaps descendants nor
replaces the private cleanup ACK; descriptor errors retain guarded cleanup.

A private completion pipe, not inherited by gates, acknowledges only finished
cleanup. `poll`, `wait`, `communicate` and terminal `returncode` require that ACK.
Output overflow produces failed gate evidence (125) only after confirmed
cleanup. Missing ACK raises `ProcessCleanupError`: callers
do not kill the still-cleaning guardian and pretend its descendants stopped. Type
and macro owners retain POST and recovery backing instead of restoring source,
workspace or index while writers may remain. Naming evidence propagates uncertainty
without ordinary failure receipts or final journal/telemetry/report publication;
both owned clients still receive cleanup attempts.

Ordinary naming collection failures retain the error and full last-operation
command, selector, exit, killed state, raw output and stderr in a failed receipt.
That forensic record never commits a successful row or closes semantic work.

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

Repository files use OS-default creation. Captures, preservation and rollback
omit permission fields and checks. Only tool setup may set executable bits.
File-type, symlink, inode, hardlink, root-confinement and owned-rollback guards
remain. Keep source-bearing records out of public logs and commits.

### File publication

`common.rename.publish_file` uses native `renameat2(RENAME_NOREPLACE)` on
the ext4 project filesystem. A collision fails without replacing either
path. Unsupported platforms or filesystems raise
`UnsupportedRenameNoReplaceError` without moving either path. The publisher
does not create empty reservations or use hard links.

Native no-replace protects destination occupancy; surrounding identity checks and
multi-file transitions remain cooperative and non-atomic. Manual writers can race
observations. Detected foreign names stay untouched. Failure after movement or
directory sync may have effect: retain evidence and inspect actual locations before
reviewed recovery. Prepared images and displaced files can survive crashes.
Zero length alone never authorizes deleting crash residue. No atomic multi-file
commit, race-free recovery or power-loss guarantee follows.

`common.files` creation, quarantine/restoration and `common.git` index moves use
this publisher. Retained-hard-link publication and its alias return field remain
removed; same-leaf preflight alone does not prove distinct-leaf capability. Source/images
stay single-link, prepared images stay on the destination filesystem, and source
bytes/identity, root confinement, leases, submodule exclusion and guarded
rollback remain required. Shared `common.rename.describe_locations` supplies
files/Git publication and restoration errors with observed names and uncertainty,
never inferred retained source after post-move failure. `safe_unlink` classifies failed movement by device/inode,
file type, link count, size and mtime, excluding ctime. This changes publication support,
not restoration authority or the scope of workspace/Git guards.

### Recovery capture

Native tool cancellation can bypass Python exception handlers entirely. Macro/type
transactions now use `common/recovery.py` to persist
`bof3.transaction-recovery/v5` evidence before the first source mutation.
`common/quarantine.py` reserves exact displaced-file destinations in advance.
The record binds root identity, owner, manifest, implementation run ID (nullable
for legacy applications), publication path, PRE bytes/hash/device/inode, intended
POST hash/device/inode, staging path and reserved PRE/POST quarantine mapping.
`common/images.py` prepares POST images under fresh staging directories before
record capture, then installs the bound inode through [file publication](#file-publication).
Images must share the destination filesystem and retain one link. Failed capture
may retain unreferenced images, but cannot mutate sources.
Shared `common.runtime.apply_changes` additionally accepts an exact `deletions`
path set and `null` content for those paths only. The default remains text-only;
extra, undeclared, unowned or already-absent deletions reject before source mutation.
Deleted files move to their reserved PRE quarantine, never to an empty placeholder.
Mixed edits capture `bof3.transaction-recovery/v6`: each deleted path has present
PRE and `post: null`; replacements retain prepared POST images. Text-only capture
remains v5. The common owner vocabulary includes `combiner`, but this mechanism
does not supply its domain transaction, audit, native gates or recovery CLI.
`common.execution.applied` can bind absent POST only with an exact, canonical,
manifest-owned `deletions` set and captured present PRE. Other calls remain
text-only. A retained file, absent PRE, mismatched deletion set or unrelated input/
build drift rejects without advancing the context. This is execution bookkeeping,
not deletion authority or combiner acceptance; [joint preparation](combiner.md#joint-transaction-preparation)
binds proposed images separately and remains read-only.
`common/directory.py` owns confined descriptor traversal. Files and directory links are
synced before mutation; capture failure aborts before moving sources.

`common/safeguards.py` captures `bof3.recovery-safeguards/v5` with
unrelated dirty-path observations, current HEAD and exact raw Git-index
bytes/path/state. Changed owned paths are covered by their PRE/POST images.
A pending index lock rejects capture. Runs without Git/index observations
lack these guards; missing observations never establish verified recovery.
The index snapshot is not a complete Git metadata backup.

### Submodule isolation candidate

`common/workspace.py` reads bounded Git status and hashes reported dirty
files and literal symlink bytes without following link targets. It excludes
`build/`, `out/`, `sessions/subagent-artifacts/` and `.pi/subagents/`; tracked
root build paths reject. Clean sources and dependency worktrees are not
copied. Submodule checkout changes remain in Git state; submodule worktree
contents are checked through the inputs actually consumed by the transaction.

`common/submodules.py` protects both HEAD and indexed gitlink boundaries,
including staged removals. Mutations overlapping a dependency boundary,
descendant or ancestor reject before source images or evidence are created,
including low-level calls without optional workspace guards. Artifact
guards also run before writer-lease acquisition. Transactions never restore,
reset, initialize, deinitialize or repair dependencies. Bound-image owned
rollback uses captured identities without depending on a concurrently corrupted
index; unbound rollback separately enforces dependency exclusion.

Scoped consumed-input freshness, owned PRE/POST identities, raw index checks,
writer exclusion and cleanup checks remain required. Naming's physical PRE
snapshot retains only its explicit transaction paths.

Observations remain non-atomic and do not exclude manual edits. Root Git
metadata remains trusted. Arbitrary Git-layout compatibility and index-extension
confinement remain unassessed.

### Recovery evidence and rollback

Records live at `out/reviews/evidence/{macro,type,combiner}-recovery-<nonce>.json`.
They contain source text: retain them as nonpublic evidence. Their digest
detects drift against a separately retained pin; neither that digest nor writer
PID authenticates restoration authority or proves writer termination. New files
use OS-default creation. Exception rollback checks POST/PRE identities.

Ordinary type/macro failure restores only the transaction's captured owned source
images. Unexpected workspace or Git-index changes are not proof of ownership:
never overwrite them or quarantine newly observed user files. After owned rollback,
compare allowed PRE bytes, the adopted dirty-path state, current HEAD and the
exact raw index against captured guards. Drift or inspection failure preserves
external state and requires parent review; it grants neither complete rollback
nor retry authority. Restore owned images before inspecting external state, so
an unreadable unrelated path cannot suppress otherwise safe owned restoration.
`ProcessCleanupError` still forbids rollback while native descendant termination
is uncertain.

These observations are non-atomic. The non-Git fallback lacks workspace/Git
guards and cannot claim verified recovery. Nested safeguards v5 excludes changed
paths and archives no unrelated source contents. Owner rollback never restores
unrelated files or Git state.

Identity-bound source rollback is replayable: it requires complete backup/image
coverage, recognizes original PRE as a no-op, and resumes a missing destination
only with its original PRE quarantine. Originally absent paths remain absent.
`common/images.py` requires exactly one original POST image at the source,
staging path or reserved POST quarantine; unexpected bytes, inode, links or
location combinations reject before that path is changed. PRE/POST byte equality
does not bypass identity checks. `common/observation.py` shares non-atomic file
observations with inspection; actual moves use the checked [publication backend](#file-publication)
with native no-replace semantics and non-atomic surrounding observations.
This low-level mechanism does not validate external record authority, acquire an
absent lease, reconcile publication or restore workspace/Git state. Unbound
byte-only rollback has no identity-replay guarantee. A no-op replay does not
complete interrupted directory syncing or establish power-loss durability.

V6 deletion rollback requires an absent source and its original single-link PRE
inode in the reserved quarantine; restored PRE with an absent quarantine is a
replayable no-op. Missing backing, duplicate locations or foreign replacements
reject without overwriting them. Deletions have no POST inode to authenticate.
Read-only inspection reports `deleted` only with that PRE backing, and distinguishes
lost backing as `missing` or `drifted`. Neither observed absence nor a v6 record
grants restoration authority or establishes an atomic multi-file commit.

`common/inspection.py` supplies read-only `inspect-recovery RECORD
--expected-recovery-digest PIN` to both owner CLIs. It checks the independent
pin, recovery record, root/owner/manifest bindings, PRE encoding and reserved
quarantine identities. It reports original PRE, missing, exact identity-bound POST
or drifted files, staged/displaced POST, unchanged-path hashes and publication
presence without printing source images. Manifest validation is structural, not
live re-derivation.
Inspection accepts transaction-recovery v5/v6 and, when present, safeguards v5.
A `null` safeguards value reports unavailable guards and blocks guarded recovery.
Inspection rejects all other recovery and safeguard formats. Obtain fresh
independently pinned backing without rewriting old records or their pins.
Safeguards v5 report dirty-path additions/removals/drift, HEAD and raw-index
agreement, and index locks. `matches` means these observations agree, not complete
workspace/Git verification. Observations are non-atomic; termination, exclusion
and publication authenticity remain unverified.
Exit zero means inspection succeeded, not that restoration or continuation is safe.

## Guarded source recovery

`bin/harness types recover` and `bin/harness macros recover` restore owned PRE
only under an externally pinned parent authorization and a separately pinned
v5 or v6 record with nested safeguards v5.
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
It omits source bytes. Identical replay requires valid receipt bytes/link
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

`bin/harness types resume` and `bin/harness macros resume` inspect a published
application for the original pinned manifest and implementation run.
`common/continuation.py`
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
identity and cumulative attempt history. This read-only, non-atomic disposition
grants no mutation or retry authority. Reproduce the captured launch
environment; never edit proofs or relax execution-context checks to make resume
pass. Historical integrity alone is not current acceptance. The broader bounded
execution loop remains unfinished.

Unpublished POST after `ProcessCleanupError` uses `inspect-recovery`, not the
published-application `resume` route. Retain cumulative consumption pins and failure
evidence across fresh invocations. An available lease does not clear the parent's
cleanup-unconfirmed stop. Inspection does not authenticate termination. Retain
native process handles in their owning
PID namespace rather than interpreting saved PIDs in another tool invocation.
Neither API persists a cleanup hold or automatically prevents another dispatch.
The supervising parent must withhold mutation until independently reviewed native
termination permits separately authorized recovery; no implicit retry follows.

## Bounded continuation

The parent prepares review scopes under the
[standing autonomous authorization](codex.md#autonomous-execution), without
another user prompt for external review or safe-checkpoint index refresh.
Execution requires no allowance, grant or attempt, member, repair or call quota.
Preserve cumulative attempt history and explicit missing-history notes.

The active-session [skill operator](../../.agents/skills/bof3-lift-loop/SKILL.md)
owns orchestration and retains actual process handles. Owner freshness, writer
quiescence and independent parent acceptance remain separate gates.

`run_bounded` optionally accepts prompt `input_data` bytes, `on_spawn` and
`on_output(stream, chunk)` callbacks. Selector-driven stdin writes proceed alongside
stdout/stderr reads without a blocking initial pipe write. Output callbacks receive
only retained bytes within the combined cap. These trusted owner callbacks do not
confer child/semantic authority; callback failure still requires original-owner
cleanup, and cleanup uncertainty takes priority over the callback error.

`common.process.run_bounded` and `owned_popen` have no default work time limit.
Ordinary transport operations may supply a relative timeout. The Linux supervisor
reaps owned descendants before completion ACK; uncertain cleanup raises
`ProcessCleanupError`. `naming.session` owns client acquisition and cleanup.
Cleanup attempts all acquired clients and preserves `ProcessCleanupError` over
ordinary failures.

`toolchain/rizin.py` owns the managed Rizin executable. The PS1 decompiler plugin
uses `toolchain/rz_psxdec.py` for its sibling-fork build and `nativebuild.py` for
receipt validation. See the [build and usage guide](../../tools/retdec-psx/README.md).

Naming collection cannot approve identities or satisfy an unfiltered full-target
naming audit by itself. Emitted artifacts never replace a successful terminal
status and current owner verification.

## Validation

Run the existing focused domain suites, shared application/history/revalidation
checks, reverse-index/CLI checks, entry/adapter bootstrap checks and `test_harness_dry.py`.
Existing assertions must follow moved owners without losing behavioral coverage.
No source acceptance follows from a passing Python suite; native checks and
independent review still govern retained BOF3 changes.

## Validation gates


- `bin/harness lift asm-diff` proves instruction equivalence; `bin/harness lift byte-match` proves bytes.
- Run `bin/harness source symbols check` after map edits; normalize with
  `bin/harness source symbols normalize [TARGET] --write` when needed.
- Run `bin/harness source symbols baseline --write` after an accepted transaction that retains
  raw `func_`/`D_` spellings; it records the reviewed debt so `bin/harness source symbols check`
  flags only later unreviewed names (`bin/harness source symbols baseline` alone is a dry run).
- `bin/harness lift status [TARGET...]` is the live lift audit.
- Run focused tests for changed behavior with `just check-unit <unit>` and the
  shared `just check` before handoff when practical. The shared gate includes
  lint, documentation drift, text tests and corpus checks, symbols and source
  validation; `just check-all` adds every Python unit. Report skipped checks and
  residual risks.
- Tests assert behavior or parsed structure, never literal wording from agent,
  skill, prompt, or workflow Markdown.
