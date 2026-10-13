# Macro application

Branches: [prepare/run](#prepare-and-run), [new header](#private-header-creation),
[parent review](#parent-review), [shared promotion](#revalidation-and-shared-promotion).
Before writes read [discovery guards](discovery-ranking.md#semantic-review),
[shared lifecycle](../../bof3-types/references/transaction-lifecycle.md) and
[native execution](../../bof3-re/references/native-execution.md).

## Prepare and run

```sh
bin/harness types baseline
bin/harness macros prepare REQUEST MANIFEST
bin/harness macros run MANIFEST CHANGES APPLICATION --implementation-run-id RUN
bin/harness macros verify APPLICATION --expected-application-digest APPLICATION_PIN
```

Use original absolute `--deadline` for bounded run/revalidation; separate cleanup
tail. Prepare/run outputs canonical repo-relative paths under `out/reviews/evidence`,
not generic `out/reviews`. Choose fresh filenames; prepare may overwrite existing
output. Externally retain every digest, never derive expected pin from supplied file.

`bof3.macro-transaction-request/v1` identifies `target`, `concern`,
`candidate_artifact`, nonempty canonical `affected_functions`, adopted baseline
when dirty. Concerns `constant|expression|local_template|shared_template`; local
requires independently exact wrappers. Block requests additionally pin positive
`block_min_instructions` and `ranking:{path,expected_ranking_digest}`. Candidate/
fingerprint must be selected under same floor/global-or-matching target focus.
Revalidate ranking at prepare/pre-write; execution context retains ranking file
state for history without reranking intended POST. Blocks support template concerns
only. Membership does not waive coverage, private proofs or parent review.

Closed `bof3.reviewed-macro-opportunity/v1`: `schema`, `candidate_id`,
`candidate_fingerprint`, `concern`, `owners`, `owner_fingerprints`,
`semantic_guards`, `observations`, `review`, `digest`. Named independent reviewer
must accept; guards/observations follow discovery reference. V1 private paths must
already belong to one manifest. Shared owners sanctioned templates/public headers
with wrapper dependency/private-proof evidence.

Opportunity capture cap 4 MiB, proof-envelope cap 64 MiB each. One confined
metadata-checked sample supplies parsing/raw hash; reject symlinks, oversize or
observed mutation. Capture is not later freshness or aggregate memory bound.

Preparation rejects known omitted consumers across whole writable owner/affected
files, not merely selected macro. Follow indexed transitive uses; grouped source
hashes/metadata ranges must map each use to exactly one indexed function. Contextual
source/header-through-includer uses conservatively cover all source members; direct
uses do not make unreferenced siblings consumers. Ambiguous identity blocks;
a covered sibling never substitutes. Repeat before writes.

Literal quoted/angle includes traverse source/include/PsyQ roots with containment
and cycle detection. Quotes first search including directory; angles do not.
Compact directives, comments and continuations supported; commented-out includes
excluded. Conditional/computed selection remains unproven. Lexical coverage never
replaces actual preprocessor binding or independent `all_use_sites`.

CHANGES maps allowed paths to complete text. Owner rederives, confines writes,
gates and publishes immutable receipts. Context requires actual implementation ID;
legacy omission cannot gain final acceptance. Ordinary failure restores owned PRE
after confirmed cleanup; uncertain descendants retain POST/backing. See lifecycle
for lease, native 120s/2 MiB bounds, publication races, v3 backing and guarded recovery.
`verify` checks integrity/native evidence, not final acceptance.

## Private-header creation

V2 permits one new target-private `local_template` header, not shared-header/type
creation. No dummy scaffolding/manual extraction. Fresh caller authority/bounds
required; historical failures/consumed entries stay consumed.

`bof3.reviewed-macro-opportunity/v2` keeps v1 fields plus exactly
`creation:{target,header,header_text,manifest_before,manifest_after}`. Digest pins
complete header and both manifest texts. Only append header to target `headers`;
identity/all other facts unchanged. `owners` only absent header;
`owner_fingerprints` value null. Canonical `include/` path ends `_internal.h`, parent
exists, no existing file/manifest claim. Ordinary loader still requires live files.

Bind exact header/manifest POST plus affected sources. Planned-POST validation
normalizes include aliases, literal closure, all new-header consumers equal affected
source set, support-source ownership, no cross-target reach, no new macro identifier
already present in C/header/templates. Preserve target/null PRE/config PRE hashes.
Semantic/all-use/human-value/exact-wrapper review remains mandatory.

Recovery captures absent PRE; no-overwrite install and identity-safe restoration
preserve foreign changes. Missing history membership is not absence proof or
shared promotion authority. Retained history checks original null PRE; generic
revalidation uses POST as adopted PRE plus `structural_manifest`, not fresh creation
admission. That retained POST form rejects ordinary run. Native gates, independent
application review and parent acceptance still required.

## Parent review

```sh
bin/harness macros review APPLICATION REVIEWED --parent-attestation PARENT_JSON --expected-application-digest APPLICATION_PIN
bin/harness macros final-verify REVIEWED --expected-envelope-digest ENVELOPE_PIN
```

Closed `bof3.macro-parent-review/v1`: `schema`, `accepted:true`, distinct nonempty
`parent_run_id`, `implementation_run_id`, `reviewer_run_id`,
`review_artifact:{path,sha256}`, `binding`, `preservation`. Reviewer inspects actual
application/receipts, artifact canonical absolute retained nonempty bytes, execution
ID real/captured. Preservation exactly `scope`, `body`, `abi`, `range`, `index`,
`adopted_baseline`, all true, never inferred from prose.

Binding exactly `application_digest`, `application_proof_digest` including original
attestation, `manifest_digest`, `request_digest`, `review_context_digest`,
`post_state_digest`, `native_receipts_digest`, `adopted_baseline_digest` over full
context baseline. Structured digests `v1:` SHA-256 compact sorted-key JSON; artifact
hashes cover bytes. Closed `bof3.macro-reviewed-application/v1`: `schema`, original
`application`, `parent_review`, `digest` over other fields. Externally pin envelope.

Final verification rechecks original proof/receipts/current closure/reviewer/unrelated
adopted state without writes/gates/rebinding. Same-state replay valid; changed state,
owner/application rejects. Historical success after tooling drift is not current
acceptance. Local attestation is not remote signature or malicious-writer defense.

Published resume uses original pins:

```sh
bin/harness macros resume ORIGINAL_MANIFEST APPLICATION --expected-manifest-digest MANIFEST_PIN --expected-application-digest APPLICATION_PIN --implementation-run-id ORIGINAL_RUN
```

Optional paired `--reviewed-envelope REVIEWED --expected-envelope-digest ENVELOPE_PIN`
checks current acceptance. Lifecycle owns `needs-review|skip-accepted` semantics;
no extraction, durable skip token, termination proof or budget renewal.

## Revalidation and shared promotion

At least two declared `shared_targets`, including primary; one private independently
reviewed exact-wrapper proof per target. Pin exactly `path`, `target`, `selector`,
`expected_envelope_digest` under `out/reviews`. Retain original bytes/hash/native
checks/parent acceptance and identical address-free semantic/parameter contracts.
Every proof wrapper in `affected_functions` gets POST asm-diff/byte-match even with
no lexical uses. Integrity/legacy application pins reject. Representatives never
cover omitted consumers.

```sh
bin/harness macros revalidate PRIVATE_ENVELOPE CHECK --expected-envelope-digest PRIVATE_PIN --execution-run-id FRESH_RUN --adopted-baseline CURRENT_BASELINE --intervening ORDERED_PINS_JSON
bin/harness macros verify-revalidation CHECK --expected-revalidation-digest CHECK_PIN
bin/harness macros review-revalidation CHECK FRESH --expected-revalidation-digest CHECK_PIN --parent-attestation FRESH_PARENT_JSON
bin/harness macros final-verify-revalidation FRESH --expected-envelope-digest FRESH_PIN
```

New canonical CHECK/FRESH under `out/reviews/evidence`. Omit intervening only for
still-current proof; otherwise ordered closed `{envelope,expected_envelope_digest}`
full accepted private records and external pins. Original POST/intervening PRE and
final/current state must agree across build/tools/environment/index/adopted state.
Only reviewed changed paths explain drift; original owned POST bytes/modes intact.
Build transitions only inside recorded owner gates, never gaps. Check-only gates
leave state unchanged; unexpected writes need explicit recovery.

Fresh distinct execution emits `bof3.macro-application-revalidation/v1`;
`checked:true` not acceptance. Owner APIs `revalidate_application`,
`verify_revalidation`, `review_revalidation`, `verify_reviewed_revalidation`.
Parent `bof3.macro-revalidation-parent-review/v1` uses same identities/artifact/
preservation; implementation=new execution, reviewer differs from original review
IDs with fresh artifact; parent may remain. Binding exactly `revalidation_digest`,
`prerequisite_envelope_digest`, full `manifest_digest`, `review_context_digest`,
`pre_state_digest`, `native_receipts_digest`, `adopted_baseline_digest`.
Closed `bof3.macro-reviewed-revalidation/v1`: `schema`, `revalidation`,
`parent_review`, `digest`. Replay checks original/intervening/current evidence,
no writes/gates/history reconstruction.

Before each distinct-target private run declare same sorted `--participating-targets`
including all mutation targets and implementation ID. New macro manifests bind
`participation_limit` to catalog size; legacy absent field retains two-target limit.
Types retain their own limit. Capture adds no write paths or participant gates.
Revalidation inherits scope; missing history cannot expand retroactively.

Shared acceptance only fresh-revalidation branch. One fresh reviewed revalidation
per declared target, distinct execution IDs, identical full capture scope and common
current inputs/environment/index/build/workspace. Mixed original/fresh proofs
reject. Freeze nested pins and `shared_pre` state/build/baseline. Shared run requires
new execution ID, identical participation, rederivation and PRE equality before
edits/gates. Missing binding cannot downgrade.

Shared review execution/reviewer differ from all private/revalidation identities;
new artifact and external envelope pin. Verify original/intervening/fresh reviews
and common PRE separately from authorized delta/new gates/current POST/adopted
workspace. Never compare old private closure directly to intentional shared POST
or infer missing history. Fresh private acceptance is not shared POST acceptance;
participation is evidence scope, never mutation authority.
