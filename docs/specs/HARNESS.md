# Harness ownership

Read [the documentation index](../INDEX.md) first. This is the Python ownership
map; [coding standards](../agents/CODING_STANDARDS.md) own naming and decomposition.

## Domain packages

All paths are below `tools/python/harness/`.

| Package | Owns | Entry points |
| --- | --- | --- |
| `macros/` | lexical macro facts, assembly/C opportunities, ranking, consumer scope, reviewed macro transactions | `bin/macro-audit` → `harness.macros.cli` |
| `naming/` | symbol naming opportunities, identity inventory, evidence collection, audits, proposals, application and acceptance | `bin/naming-audit` → `harness.naming.cli`; `bin/naming-evidence-run` → `harness.naming.runner` |
| `types/` | C declarations, representation inference, type-use/candidate indexing, reviewed type transactions | `bin/type-audit` → `harness.types.cli` |
| `docs/` | scoped Markdown snapshots, search, context, aggregation and edit/repair/compaction preparation | `bin/docs` → `harness.docs.cli` |
| `common/` | reusable CLI, digests, confined files, process lifecycle, workspace, native receipts and acceptance mechanisms | direct imports from the mechanism owner |
| `analysis/` | cross-domain reverse index, graph, mission and query coordination | `bin/index`, `bin/rev-query` |
| `domain/` | manifests, target identity, original binary/layout, source claims and includes | shared repository facts, not candidate acceptance |

Each domain uses noun files such as `index.py`, `queries.py`, `review.py`,
`transactions.py`, `application.py`, and `cli.py`. Naming's finer categories include
`collection.py`, `journal.py`, `namespace.py`, `equivalence.py`, and `terminal.py`.
Package initializers are inert: import the actual owner, not a compatibility facade.
The cross-domain index remains one derived database; package separation does not
create independent databases or relax freshness checks.

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
authority. See [naming opportunities](../usage.md#symbol-naming-opportunities).

## Policy versus mechanism

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

Macro extraction policy belongs in [MACROS.md](MACROS.md). Naming evidence and
identity-application contracts are linked from the documentation index. Type
candidate/application schemas remain in
[tool usage](../usage.md#type-parent-review-and-final-verification).

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

A private completion pipe, not inherited by gates, acknowledges only finished
cleanup. `poll`, `wait`, `communicate` and terminal `returncode` require that ACK.
Timeout/overflow produces failed gate evidence (124/125) only after confirmed
cleanup. Missing ACK or cleanup wait expiry raises `ProcessCleanupError`: callers
do not kill the still-cleaning guardian and pretend its descendants stopped. Type
and macro owners retain POST and recovery backing instead of restoring source,
workspace or index while writers may remain. Naming evidence propagates uncertainty
without ordinary failure receipts or final journal/telemetry/report publication;
both owned clients still receive cleanup attempts.

The guardian can remain active when termination cannot be confirmed; callers stop
for parent inspection, not automatic retry. Lease release/reacquisition alone is
not quiescence. This mechanism is not a sandbox, cross-platform guarantee or proof
against privileged interference. Injected runners do not prove native lifecycle
behavior. Native cancellation can bypass Python rollback: process closure is not
file restoration. Parent-owned native handle/tree verification, source/workspace/
index inspection and reviewed owned-PRE recovery remain separate obligations.

`common/lease.py` excludes concurrent macro/type application and revalidation
writers in one repository. A nonblocking `flock` spans canonical manifest
re-derivation, source application, native gates, publication and exception
rollback. The persistent `out/reviews/evidence/transaction.lock` is an empty,
single-link regular file with mode `0600`; root, path and inode checks reject
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

Native tool cancellation can bypass Python exception handlers entirely. Macro/type
transactions now use `common/recovery.py` to persist
`bof3.transaction-recovery/v3` evidence before the first source mutation.
`common/quarantine.py` reserves exact displaced-file destinations in advance.
The record binds root identity, owner, manifest, implementation run ID (nullable
for legacy applications), publication path, PRE bytes/hash/mode/inode, intended
POST hash/exact mode/inode, staging path and reserved PRE/POST quarantine mapping.
`common/images.py` prepares POST images under fresh `0700` directories before
record capture, then installs the bound inode with verified no-replace moves.
Images must share the destination filesystem and require native no-replace
support. Failed capture may retain unreferenced images, but cannot mutate sources.
`common/directory.py` owns confined descriptor traversal. Files and directory links are
synced before mutation; capture failure aborts before moving sources.

`common/safeguards.py` reuses the owner's existing workspace/index snapshots,
retaining untouched PRE bytes, hash, mode, identity, ownership and link count,
plus exact Git-index bytes/path/state. Inventory follows `workspace_backup`:
tracked and unignored files, excluding `out/`, `sessions/subagent-artifacts/` and
`.pi/subagents/`. Changed owned paths remain covered by their source images;
unchanged owned paths remain in the workspace inventory. A pending index lock
rejects capture. Non-Git or low-level runs without snapshots explicitly lack these
guards; missing data is not treated as a clean workspace. Capture avoids a second
full-workspace read, but private records grow with retained PRE content.
An exact index snapshot is not a complete Git metadata backup.

Records live at `out/reviews/evidence/{macro,type}-recovery-<nonce>.json`, mode
`0600`. They contain source text: retain them as private evidence. Their digest
detects drift against a separately retained pin; neither that digest, file mode
nor writer PID authenticates restoration authority or proves writer termination.
For new files, creation mode remains `0644` masked by the process umask; v2/v3 bind
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
observations with inspection; actual moves retain descriptor/no-replace checks.
This low-level mechanism does not validate external record authority, acquire an
absent lease, reconcile publication or restore workspace/Git state. Legacy
byte-only rollback has no identity-replay guarantee. A no-op replay does not
complete interrupted directory syncing or establish power-loss durability.

`common/inspection.py` supplies read-only `inspect-recovery RECORD
--expected-recovery-digest PIN` to both owner CLIs. It checks the independent
pin, private record, root/owner/manifest bindings, PRE encoding and reserved
quarantine identities. It reports original PRE, missing, exact v2/v3 POST or
drifted files, staged/displaced POST, unchanged-path hashes and publication presence without printing
source images. Manifest validation is structural, not live re-derivation.
V3 inspection also reports scoped workspace additions/removals/drift, index
agreement and index locks without printing archived bytes. `matches` means only
that these scoped observations agree, not complete workspace/Git verification.
Historical v1 records retain content-only POST evidence; v2 retains exact POST
identity but no workspace/index guards. Neither is upgraded. Observations are
non-atomic; termination, exclusion and publication authenticity remain unverified.
Exit zero means inspection succeeded, not that restoration or continuation is safe.

## Guarded source recovery

`type-audit recover` and `macro-audit recover` restore owned PRE only under an
externally pinned parent authorization and a separately pinned v3 record.
`common/authorization.py` validates authority bindings; `common/restoration.py`
owns the fail-stop operation; `common/inspection.py::load_recovery` validates
backing for both inspection and restoration. No discovered record grants authority.

The supervising parent must actually verify the named native tool handle is
terminal and its writer tree has stopped, then obtain independent recovery review.
The CLI checks retained artifacts and pins; it does **not** authenticate actor IDs
or interpret tool output as proof of termination. PID reuse, a free lock or a
stopped launcher alone is insufficient. These are explicit trusted-parent
attestations, not unattended recovery or authorization for another transaction.

Authorization is a private, single-link `0600` JSON file under
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
