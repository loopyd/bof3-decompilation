# Shared transaction lifecycle

Applies to type/macro owners. Read domain application reference too. Branches:
[execution](#execution), [deadlines](#deadlines), [publication](#publication),
[recovery](#recovery), [resume](#resume). Mechanisms never grant domain approval.

## Execution

Pin original request, membership, PRE, external digests and explicitly adopted
dirty baseline. Re-derive live manifest before edits. Changes map permitted
repo-relative paths to complete replacement text; generated files remain untouched.
Only explicit replacements may change authoritative inputs; preserve permissions.
Configured tools, explicit-source companion-free owners and Ninja required. No
installation, toolchain override, silently missing compiler or index recovery.

Context-bearing runs require actual `--implementation-run-id`. Capture owner-derived
source/header/config/binary, resolver catalog including absence, tooling/modes,
adopted baseline, raw index, environment, native tools and Ninja transitions before
edits, around gates and at publication. Legacy omitted-ID route cannot supply
context-bearing final acceptance. Context is not reviewer attestation.

Native receipts need correct JSON schemas, positive exact/byte-match semantics,
selector/source/address, target-owned binary and size agreement. Exit 0 warnings or
malformed payloads fail. Retain stdout JSON separately from stderr; replay validates
both evidence and closure. Splat/build use native plain-output exit contracts.
Partial baselines retain exact-output pins. `verify` is integrity/native evidence,
not final semantic acceptance.

Shared process owner caps gates at 120s and 2 MiB combined output; timeout 124 and
overflow 125 are failed receipts, never positive evidence. Linux guardian subreaper
kills/reaps descendants, including redirected-stdio/`setsid` escapees. Completion
requires private cleanup ACK, not direct-child exit. Missing ACK or uncertain
cleanup raises `ProcessCleanupError`; retain POST/backing, stop for parent. Never
kill guardian and pretend writers stopped. Process cleanup is not source rollback.

One nonblocking cooperative writer lease spans manifest rederivation, edits,
gates, publication and ordinary-failure rollback. Contention blocks before edits.
Persistent `out/reviews/evidence/transaction.lock` is owned single-link regular
file, not stale-PID debris to delete. Identity loss stops guarded restoration.
Free/reacquired lease proves neither termination nor exclusion of manual writers.
Read-only verification/inspection acquires no lease.

## Deadlines

Type/macro `run` and `revalidate --deadline SECONDS` use original absolute monotonic
work cutoff, not duration/UTC. Invalid/nonfinite/boolean and expired values reject;
nested cutoffs only tighten. Preserve separate cleanup hard-stop and consumed
budgets. Native runner uses earlier of cutoff and 120s. Threads/subprocess entry
points need explicit propagation; injected runners are not made interruptible by
return checks.

Forward checks surround reads, transitions, gates and publication. Expiry stops
forward work, not owned rollback with safeguards under original cleanup tail.
Filesystem/Git work can stall; supervise through cleanup hard-stop. Uncertain
native cleanup still prohibits restoration. Late publication can survive failed
invocation or rollback; retain failure status and inspect current owner state,
never infer success from file existence or delete evidence to manufacture it.
Check-only revalidation never gains source-restoration authority. Late mechanically
valid evidence does not prove deadline compliance or authorize advancement.

Naming differs: `evidence --deadline` is relative per-operation cap;
`--work-deadline` is original absolute cutoff, clipped by 600s shard wall. Lifecycle
commands also accept `--work-deadline`. Receipt/journal/finalization writes stop on
expiry; earlier/in-flight evidence may remain. Cleanup attempts every client and
prioritizes `ProcessCleanupError`. Standalone limits are not campaign budgets.

## Publication

Before edits, persist `bof3.transaction-recovery/v3` under
`out/reviews/evidence/{macro,type}-recovery-<nonce>.json`. Bind root, owner,
manifest/run/publication, PRE bytes/hash/mode/inode, prepared POST hash/mode/inode,
fresh staging and reserved PRE/POST quarantines. Same filesystem and single-link
images required. Capture failure blocks edits but may leave images.

Native `RENAME_NOREPLACE` first. Unsupported filesystem permits cooperative
exclusive-empty-reservation, source/reservation identity checks, rename, destination
identity/source-absence checks and directory fsync. Not atomic CAS against manual
writers: check/rename and cleanup/unlink races, visible reservations, crash leftovers
remain. Detected foreign names stay untouched. Failure after move/sync may have
effect; inspect locations. Empty size alone never authorizes cleanup; reservation
must have captured regular, empty, single-link identity unchanged. No hard-link
fallback, migration, permission weakening or power-loss guarantee.

Git-backed v3 captures untouched tracked/unignored workspace PRE bytes/metadata and
exact index bytes/state, excluding shared generated-artifact roots. Missing guards
are unavailable, not clean. Pending index lock blocks capture. Unrelated symlinks
are literal no-follow evidence, never referent reads or restoration targets.
Dependency snapshots preserve content/identity but grant no mutation/restoration
or complete arbitrary-Git-layout confinement. Observations are non-atomic; root
Git metadata trusted. Evidence uses filesystem-native permissions, not enforced
POSIX mode. Keep source-bearing backing nonpublic/uncommitted.

V4 supports separately authorized deletion-aware common owners: present PRE,
`post:null`, exact declared deletion set and reserved PRE quarantine. It does not
add type/macro deletion authority, combiner acceptance or automatic recovery.
Historical v1 is content-only POST; v2 lacks workspace/index guards. Never upgrade
historical records to claim newer safeguards.

## Recovery

Ordinary failure restores captured owned source images only after confirmed native
cleanup. Restore owned images before checking unrelated state; never overwrite
unexpected workspace/index edits or new files. Compare allowed PRE, adopted
workspace, raw index and all-PRE metadata/link safeguards. Drift or failed
inspection means incomplete rollback, not retry permission.

Identity replay requires complete backing, exactly one original POST at source,
staging or reserved POST quarantine, exact bytes/mode/inode/link/location. PRE
already present is no-op; missing destination needs original PRE quarantine;
original absence stays absence. Equal PRE/POST bytes do not waive identity. V4
needs absent source and original single-link quarantined PRE; foreign replacements
or lost/duplicate backing block. Replay does not prove interrupted fsync completed.

```sh
bin/harness types inspect-recovery RECORD --expected-recovery-digest PIN
bin/harness types recover RECORD --expected-recovery-digest PIN --authorization AUTHORIZATION --expected-authorization-digest AUTH_PIN
```

Use `macros` instead of `types` for macro owner. Inspection checks independent pin,
structural root/owner/manifest/backing/quarantine bindings, reports locations, scoped
drift/index locks/publication without source bytes. It proves neither atomic state,
live rederivation, authentic publication, termination nor recovery readiness.

Parent must verify actual native handle terminal and full writer tree stopped,
then obtain independent recovery review. PID, stopped launcher or free lock is
insufficient. CLI validates retained attestations, not actor authenticity.

Closed `bof3.source-recovery-authorization/v1`, single-link regular JSON under
`out/reviews/evidence/`:

- `schema`, `approved:true`, distinct nonempty `parent_run_id`, `reviewer_run_id`,
  both distinct from implementation.
- `review_artifact:{path,sha256}`, retained nonempty single-link evidence.
- `binding` exactly `action:"restore-owned-pre"`, `owner`, captured
  `root:{path,device,inode}`, `implementation_run_id`, `recovery:{path,digest}`,
  `manifest_digest`, sorted exact changed `paths`, captured `publication`, and
  `completion:out/reviews/evidence/{owner}-restoration-{record_nonce}.json`.
- `termination` exactly nonempty `tool`, actual string `handle`, `state:"terminal"`,
  `writer_tree_stopped:true`, `artifact:{path,sha256}`.
- `digest`, `v1:` SHA-256 of compact sorted-key JSON without `digest`.

Artifact paths canonical repo-relative evidence paths; hashes cover bytes.
Authorization, record, completion, publication and both artifacts distinct.
Retain authorization/record pins externally before use.

Recovery leases before validation; requires v3/v4, implementation identity,
workspace/Git guards matching, known absent publication and consistent images.
Recheck every image before each transition, completed PRE, authority/guards around
completion. Stop on singleton failure. Never overwrite unrelated/index state.
Exclusive `bof3.source-restoration/v1` receipt binds pins, authority, termination and
restored identities, no source text. Replay needs valid receipt bytes/mode/links
and fresh guards/PRE/authority. Partial failure retains backing for reviewed retry.
No acceptance, automatic retry, published-application reconciliation, workspace/Git
restoration or automatic worker recovery. Never rewrite proofs after tooling moves.

## Resume

Published application uses owner `resume`, original manifest/application pins and
captured implementation ID. Current owner verification yields `needs-review` without
accepted envelope; exact complete application including attestation plus externally
pinned current final acceptance yields `skip-accepted`. Envelope and pin paired.
Invalid/stale/mismatched acceptance rejects, never downgrades or reapplies. PRE is
not rederived from POST. No native rerun, persisted skip token or budget renewal.

Unpublished POST after cleanup uncertainty uses recovery inspection instead.
Parent retains original queue/budget/high-water pins and withholds mutation until
independently reviewed termination and separately authorized recovery. Reproduce
captured environment; never edit proofs to make continuation pass.
