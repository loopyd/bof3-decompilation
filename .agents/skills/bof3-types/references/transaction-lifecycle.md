# Shared transaction lifecycle

Before type/macro execution, read the domain application reference too. Select
[execution](#execution), [publication](#publication),
[recovery](#recovery), [resume](#resume). Mechanisms never grant domain approval.

## Execution

Pin original request, membership, PRE, external digests and adopted dirty baseline.
Rederive the live manifest before edits. Map permitted repo-relative paths to
complete replacements; preserve generated files/permissions. Require configured
tools, explicit-source companion-free owners and Ninja. Do not install, override
toolchains, ignore missing compilers or recover the index.

Require actual `--implementation-run-id` for context-bearing runs. Capture
owner-derived source/header/config/binary, resolver catalog including absence,
tooling/modes, adopted baseline, raw index, environment, native tools and Ninja
transitions before edits, around gates and at publication. Legacy omitted-ID runs
cannot gain context-bearing final acceptance; context is not reviewer attestation.

Require native receipt schemas, positive exact/byte-match semantics and matching
selector/source/address/target-owned binary/size. Reject exit 0 warnings and malformed
payloads. Separate stdout JSON/stderr; replay checks evidence/closure. Use native
plain-output exit contracts for Splat/build. Retain partial exact-output pins.
`verify` supplies integrity/native evidence, not semantic acceptance; check-only
revalidation grants no source-restoration authority.

Cap combined gate output at 2 MiB; overflow 125 is failed evidence. The Linux
guardian subreaper kills/reaps descendants, including redirected-stdio/`setsid`
escapees. Require private cleanup ACK, not direct-child exit. Missing ACK/uncertain
cleanup raises `ProcessCleanupError`; retain POST/backing and stop for parent.
Do not kill the guardian as termination proof or treat cleanup as source rollback.
Naming cleanup attempts every client and prioritizes `ProcessCleanupError`.

Hold one nonblocking cooperative writer lease through rederivation, edits, gates,
publication and ordinary-failure rollback. Block contention before edits. Preserve
the owned single-link regular `out/reviews/evidence/transaction.lock`; it is not
stale-PID debris. Stop guarded restoration on identity loss. A free/reacquired lease
proves no termination or manual-writer exclusion. Read-only checks acquire no lease.

## Publication

Before edits, persist `bof3.transaction-recovery/v3` under
`out/reviews/evidence/{macro,type}-recovery-<nonce>.json`. Bind root, owner,
manifest/run/publication, PRE bytes/hash/mode/inode, prepared POST hash/mode/inode,
fresh staging and reserved PRE/POST quarantines. Same filesystem and single-link
images required. Capture failure blocks edits but may leave images.

Use native `RENAME_NOREPLACE` on current ext4. Unsupported platforms/filesystem
errors fail without empty reservations or hard-link/alternate publishers. Identity
checks/multi-file transitions remain cooperative; manual writers are not excluded.
Preserve foreign names. After move/sync failure, inspect locations and retain
prepared/displaced images; empty size grants no cleanup. Do not weaken permissions
or claim atomic multi-file/power-loss guarantees. The
[shared publication contract](../../../../docs/agents/harness.md#file-publication)
retains historical NTFS authorization separately from current behavior.

Retain recovery PRE/POST images only for changed paths. Git-backed
`bof3.recovery-safeguards/v4` pins HEAD, exact index bytes/identity, and unrelated
Git status with dirty-path digests, excluding shared artifact roots. Capture dirty
symlink bytes without following links and submodule checkout identities/protected
boundaries without recursive contents. Pin consumed dependencies through owner
guards; unchanged files supply no rollback images. Missing guards are unavailable,
not clean; index locks block capture. Observations are non-atomic and trust root
Git metadata. Ext4 creation respects umask; preserve exact modes/drift checks and
keep source-bearing backing nonpublic/uncommitted.

V4 supports separately authorized deletion-aware common owners: present PRE,
`post:null`, exact declared deletion set and reserved PRE quarantine. It does not
add type/macro deletion authority, combiner acceptance or automatic recovery.
Historical v1 is content-only POST; v2 lacks workspace/index guards. Nested
safeguards v1-v3 remain inspectable, but their full-workspace guards are not
revalidated or silently converted and cannot authorize guarded recovery. New
transactions require scoped backing. Never upgrade historical records to claim
newer safeguards.

## Recovery

After ordinary failure and confirmed native cleanup, restore captured owned images
before checking unrelated state. Preserve unexpected workspace/index edits/new
files. Compare allowed PRE, adopted workspace, raw index, HEAD and scoped
safeguards. Drift/failed inspection means incomplete rollback, not retry permission.

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

Parent verifies the actual native handle terminal/full writer tree stopped, then
obtains independent recovery review. PID, stopped launcher or free lock is
insufficient; CLI checks attestations, not actor authenticity.

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
