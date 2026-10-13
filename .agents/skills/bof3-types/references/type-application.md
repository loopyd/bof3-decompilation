# Type application

Branches: [discovery](#discovery), [prepare/run](#prepare-and-run),
[parent review](#parent-review), [revalidation](#revalidation),
[shared promotion](#shared-promotion). Before writes/native work read
[transaction lifecycle](transaction-lifecycle.md) and
[native execution](../../bof3-re/references/native-execution.md).

## Discovery

Check `bin/harness analysis rz-project status TARGET --json` and
`bin/harness analysis query --json status`. Stale evidence blocks preparation;
parent refreshes at safe checkpoint only. `analysis index --recover` refreshes
all stale manifest snapshots then rebuilds disposable index; require both statuses
fresh. Never refresh during pinned transaction. Inspect `bin/harness analysis query
--help` for `types`, `type-uses`, `type-candidates` and owner queries. Target-qualified
original accesses/callers/consumers outrank analyzer representations.

Reuse target/shared/PsyQ types before new alias/aggregate/prototype. Prove width,
signedness, pointer depth, alignment, offsets, qualifiers and ABI. Unaligned `u32`
can silently shift later fields; represent unaligned words with `u8[N]`, verify
actual layout, not comments. Pad from next free byte, not last field start. Use
SDK `MATRIX` when evidence fits; adjacent stack blobs can be separate locals,
not one duplicated aggregate. Recover stable target-local fields before permuting.
`volatile` needs asynchronous/hardware evidence; pointer-cell volatility differs
from pointee volatility. Hex for addresses/masks/encoded values, decimal for human
quantities. Matching improvements remain target evidence, not universal rules.

## Prepare and run

```sh
bin/harness types account out/reviews/type-account.json
bin/harness types validate-account out/reviews/type-account.json
bin/harness types baseline
bin/harness types prepare REQUEST MANIFEST
bin/harness types run MANIFEST CHANGES APPLICATION --implementation-run-id RUN
bin/harness types verify APPLICATION --expected-application-digest APPLICATION_PIN
```

Accounted inventory is not accepted cleanup. Candidate requires separate independent
review, current fingerprint, resolved representation/semantics and two independent
observations. Dirty request explicitly adopts exact current preflight/baseline
digest; no later drift exemption. Canonical repo-relative opportunity capture is
confined, metadata-checked, capped 4 MiB; parsing/hash share same sample. Symlinks,
oversize or observed mutation reject. Shared private-proof capture cap 64 MiB each,
not aggregate bound. Admission capture never replaces live rederivation.

Changes JSON maps only manifest-owned paths to complete replacement text. Owner
runs checks/immutable receipts and ordinary-failure rollback under lifecycle rules.
Externally retain application digest before verification; never derive expected pin
from supplied artifact. Shared preparation requires two independently accepted
private envelopes, distinct targets/paths/digests, identical representation and
semantic contracts, no target addresses, private/exact-wrapper and shared dependency
proof. Integrity pins or legacy `expected_application_digest` never substitute.

## Parent review

```sh
bin/harness types review APPLICATION REVIEWED --parent-attestation PARENT_JSON --expected-application-digest APPLICATION_PIN
bin/harness types final-verify REVIEWED --expected-envelope-digest ENVELOPE_PIN
```

After distinct reviewer inspects actual application/receipts, parent supplies closed
`bof3.type-parent-review/v1`: `schema`, `accepted:true`, distinct nonempty
`parent_run_id`, `implementation_run_id`, `reviewer_run_id`,
`review_artifact:{path,sha256}`, `binding`, `preservation`.
Artifact canonical absolute retained nonempty reviewer bytes; implementation ID
matches captured run. Preservation exactly `scope`, `body`, `abi`, `range`, `index`,
`adopted_baseline`, all `true`, attesting actual PRE comparison, never inferred prose.

Binding exactly `application_digest`, `application_proof_digest` including original
attestation, `manifest_digest`, `request_digest`, `review_context_digest`,
`post_state_digest`, `native_receipts_digest` over full array,
`adopted_baseline_digest` over full context baseline. Structured pins follow owner
canonical compact sorted-key JSON digest; artifact SHA-256 covers bytes. Missing
context/legacy omitted-ID cannot promote.

Closed `bof3.type-reviewed-application/v1`: `schema`, original `application`,
`parent_review`, `digest` over other fields. Externally pin before `final-verify`.
Replay validates original proofs/receipts, current closure, reviewer bytes and
unrelated adopted state without gates/edits/proof rewriting. Same-state replay
valid; different owner/application/context/state rejects. Trust local parent/runtime,
not malicious-writer authentication. Shared input pins exactly `path`, `target`,
`expected_envelope_digest`; canonical paths under `out/reviews`.

Published resume:

```sh
bin/harness types resume ORIGINAL_MANIFEST APPLICATION --expected-manifest-digest MANIFEST_PIN --expected-application-digest APPLICATION_PIN --implementation-run-id ORIGINAL_RUN
```

Add paired `--reviewed-envelope REVIEWED --expected-envelope-digest ENVELOPE_PIN`
for current acceptance check. Follow lifecycle resume constraints.

## Revalidation

```sh
bin/harness types revalidate PRIVATE_ENVELOPE CHECK --expected-envelope-digest PRIVATE_PIN --execution-run-id FRESH_RUN --adopted-baseline CURRENT_BASELINE --intervening ORDERED_PINS_JSON
bin/harness types verify-revalidation CHECK --expected-revalidation-digest CHECK_PIN
bin/harness types review-revalidation CHECK FRESH --expected-revalidation-digest CHECK_PIN --parent-attestation FRESH_PARENT_JSON
bin/harness types final-verify-revalidation FRESH --expected-envelope-digest FRESH_PIN
```

CHECK/FRESH must be new canonical paths under `out/reviews/evidence`. Omit
`--intervening` only for still-current proofs. Otherwise ordered closed
`{envelope,expected_envelope_digest}` objects retain full accepted private envelopes
and original external pins. Original POST/intervening PRE, consecutive transitions
and final/current state must agree across native build/tools/environment/index and
unrelated adopted workspace. Only reviewed changed paths explain drift; original
owned POST bytes/modes intact. Build drift valid only inside recorded owner build
transitions, never gaps. Manifest evidence membership is not mutation authority.
No source reapplication; new gates must leave current state unchanged. Unexpected
writes require explicit recovery. Adoption alone never excuses drift.

Owner APIs `revalidate_application`, `verify_revalidation`, `review_revalidation`,
`verify_reviewed_revalidation` transport same contract. Fresh execution produces
`bof3.type-application-revalidation/v1`; externally pin. `checked:true` is not
acceptance. Fresh parent object `bof3.type-revalidation-parent-review/v1` uses same
identity/artifact/preservation fields; implementation binds new execution. Reviewer
differs from all original review identities and retains fresh artifact; parent may
remain. Binding exactly `revalidation_digest`, `prerequisite_envelope_digest`,
`manifest_digest` over full revalidation manifest, `review_context_digest`,
`pre_state_digest`, `native_receipts_digest`, `adopted_baseline_digest`.
Closed `bof3.type-reviewed-revalidation/v1`: `schema`, `revalidation`,
`parent_review`, `digest`. Replay checks history and current closure without writes.

Before distinct-target private executions declare same sorted
`--participating-targets TARGET TARGET`, one or two canonical targets including all
mutation targets; requires implementation ID. This captures binary/Splat/maps/
reviewed/include evidence, adds no writable paths or participant gates. Omission
captures selected target only. Revalidation inherits scope, never expands it;
missing/mismatched historical scope rejects. Each private result still needs fresh
native gates and independent review.

## Shared promotion

Original private envelopes support preparation only. Shared parent acceptance
requires exactly two fresh accepted revalidations, distinct targets/executions,
identical explicit two-target capture and common native inputs/environment/index/
build/workspace. Retain complete nested envelopes/pins plus `shared_pre`
state/build/baseline. Mixed original/fresh proofs reject. Shared `run` requires new
implementation ID, same participation, live rederivation and captured PRE equality
before edits/gates; omissions cannot downgrade to legacy.

Shared `review`/`final-verify` require new implementation/reviewer not reused from
private prerequisites or revalidation, new reviewer artifact and external envelope
pin. Replay validates frozen private histories/common PRE separately from authorized
shared delta, new gates, current POST/context and unrelated adopted state. Never
compare private closure directly to intended shared POST or reconstruct missing
history. Address-free contracts, ownership/exact wrappers and all consumers remain
mandatory. Fresh private evidence is not shared POST acceptance or live promotion.
