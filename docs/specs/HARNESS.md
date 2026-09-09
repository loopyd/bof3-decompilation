# Harness ownership

Read [the documentation index](../INDEX.md) first. This is the Python ownership
map; [coding standards](../agents/CODING_STANDARDS.md) own naming and decomposition.

## Domain packages

All paths are below `tools/python/harness/`.

| Package | Owns | Entry points |
| --- | --- | --- |
| `macros/` | lexical macro facts, assembly/C opportunities, ranking, consumer scope, reviewed macro transactions | `bin/macro-audit` → `harness.macros.cli` |
| `naming/` | symbol identity inventory, evidence collection, naming audits, proposals, application and acceptance | `bin/naming-audit` → `harness.naming.cli`; `bin/naming-evidence-run` → `harness.naming.runner` |
| `types/` | C declarations, representation inference, type-use/candidate indexing, reviewed type transactions | `bin/type-audit` → `harness.types.cli` |
| `common/` | reusable CLI, digests, confined files, process lifecycle, workspace, native receipts and acceptance mechanisms | direct imports from the mechanism owner |
| `analysis/` | cross-domain reverse index, graph, mission and query coordination | `bin/index`, `bin/rev-query` |
| `domain/` | manifests, target identity, original binary/layout, source claims and includes | shared repository facts, not candidate acceptance |

Each domain uses noun files such as `index.py`, `queries.py`, `review.py`,
`transactions.py`, `application.py`, and `cli.py`. Naming's finer categories include
`collection.py`, `journal.py`, `namespace.py`, `equivalence.py`, and `terminal.py`.
Package initializers are inert: import the actual owner, not a compatibility facade.
The cross-domain index remains one derived database; package separation does not
create independent databases or relax freshness checks.

Each domain's `cli.py` also registers its `rev-query` subcommands and owns their
argument-to-query adapters. `commands/rev_query.py` composes those registrations,
checks index freshness and target identity, and prints their results. Naming
inventory remains manifest-backed without requiring an index; transaction scope
retains its indexed target check and labeled output. Macro and type table schemas
live in `macros/schema.py` and `types/schema.py`, composed by `analysis/schema.py`.

## Policy versus mechanism

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
and retain at most 2 MiB of native output per command. On Linux an owner-death
pipe supervises the child process group; timeout, overflow and interrupted
capture clean up that owned tree. Macro/type timeout and overflow receipts fail
with exit codes 124 and 125. Injected test runners do not prove this production
path. Process cleanup is not transaction recovery: abrupt owner death requires
owned-PRE reconciliation before continuation.

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
the resulting exact mode. Exception rollback checks POST/PRE identities, and
workspace rollback does not rewrite already-restored byte-identical files.

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

Guarded owner recovery remains unfinished. Do not retry a transaction against its
still-modified POST or automatically restore from a discovered record. Confirm
writer termination/exclusion, validate independent bindings and reconcile any
published application before explicit parent recovery. Durable workspace/index
images provide backing, not permission or an implemented restoration protocol.
Exception rollback and process cleanup still do not establish complete crash-safe recovery.

Moving implementation files changes tooling-bound execution closure. Do not edit
retained receipts, frozen requests or historical evidence to make old proofs appear
current. Revalidation and original-byte/ABI/source-range preservation remain gates.

## Validation

Run the existing focused domain suites, shared application/history/revalidation
checks, reverse-index/CLI checks, wrapper bootstrap checks and `test_harness_dry.py`.
Existing assertions must follow moved owners without losing behavioral coverage.
No source acceptance follows from a passing Python suite; native checks and
independent review still govern retained BOF3 changes.
