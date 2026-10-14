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

Use fresh canonical repo-relative outputs under `out/reviews/evidence`; prepare
may overwrite existing files. Retain digests externally, never from supplied files.

Bind `bof3.macro-transaction-request/v1` to `target`, `concern`,
`candidate_artifact`, nonempty canonical `affected_functions` and adopted dirty
baseline. Use `constant|expression|local_template|shared_template`; local concerns
require independently exact wrappers. For blocks, use template concerns and pin
positive `block_min_instructions` and `ranking:{path,expected_ranking_digest}`.
Select candidate/fingerprint under the same floor and global-or-matching target
focus. Revalidate ranking at prepare/pre-write; retain its file state in execution
history without reranking intended POST. Membership waives no coverage, private
proof or parent review.

Use closed `bof3.reviewed-macro-opportunity/v1`: `schema`, `candidate_id`,
`candidate_fingerprint`, `concern`, `owners`, `owner_fingerprints`,
`semantic_guards`, `observations`, `review`, `digest`. Named independent reviewer
must accept the discovery guards/observations. V1 private paths must already belong
to one manifest. Shared owners require sanctioned templates/public headers and
wrapper dependency/private-proof evidence.

Cap opportunity capture at 4 MiB and each proof envelope at 64 MiB. Parse/hash one
confined metadata-checked sample; reject symlinks, oversize or observed mutation.
Capture proves no later freshness and supplies no aggregate memory bound.

Reject omitted consumers across whole writable owner/affected files. Follow indexed
transitive uses and map grouped-source hashes/metadata ranges to exactly one indexed
function per use. Contextual source/header-through-includer uses cover all source
members; direct uses exclude unreferenced siblings. Block ambiguous identities;
covered siblings cannot substitute. Repeat before writes.

Traverse literal quoted/angle includes through source/include/PsyQ roots with
containment and cycle checks. Quotes first search the including directory; angles
do not. Support compact directives, comments and continuations; exclude commented
includes. Conditional/computed selection remains unproven. Require actual
preprocessor binding and independent `all_use_sites` beyond lexical coverage.

For grouped C, require one explicit lift owner, complete Splat/claim/index
membership, current indexed macro-source hash and every attached ordinary function
in `affected_functions`. Cap capture at 4 MiB. Preserve address order, metadata,
spellings, kinds, progress and text outside implementations. Refuse template
replacement, C-owner macro-definition changes and new includes. Keep legacy
single-function behavior. Require per-member native checks, which currently refuse
ordinary grouped sources. Preparation proves no native preservation or acceptance.

Map CHANGES from allowed paths to complete text. Let the owner rederive scope,
confine writes, run gates and publish immutable receipts. Context/final acceptance
requires the actual implementation ID. Follow lifecycle rules for lease, 2 MiB
native output, publication, v3 backing, rollback and uncertain cleanup.
`verify` checks integrity/native evidence; parent review supplies final acceptance.

## Private-header creation

V2 permits one new target-private `local_template` header. Require fresh caller
authority/bounds and retain consumed failures/entries. Do not create shared
headers/types, dummy scaffolding or manual extractions.

Use v1 fields in `bof3.reviewed-macro-opportunity/v2` plus exactly
`creation:{target,header,header_text,manifest_before,manifest_after}`. Digest pins
complete header and both manifest texts. Append only to target `headers`; preserve
identity/all other facts. Set `owners` to the absent header only and that header's
`owner_fingerprints` entry to null. Require a canonical `include/` path ending
`_internal.h`, an existing parent and no file/manifest claim. Ordinary loading
still requires live files.

Bind exact header/manifest POST and affected sources. Validate include aliases,
literal closure, support-source ownership, new-header consumers equal to the
affected source set, no cross-target reach and no new identifier already in
C/header/templates. Preserve target/null PRE/config PRE hashes and require
semantic/all-use/human-value/exact-wrapper review.

Capture absent PRE and preserve foreign changes through no-overwrite installation
and identity-safe restoration. Missing history membership proves neither absence
nor shared promotion authority. History checks original null PRE; generic
revalidation adopts POST plus `structural_manifest` as PRE. That form cannot admit
fresh creation or ordinary run. Require native gates, independent application
review and parent acceptance.

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

Final verification rechecks original proof/receipts/current closure/reviewer and
unrelated adopted state without writes/gates/rebinding. Allow same-state replay;
reject changed state/owner/application. Historical tooling success is not current
acceptance; local attestation is no remote signature or malicious-writer defense.

Published resume uses original pins:

```sh
bin/harness macros resume ORIGINAL_MANIFEST APPLICATION --expected-manifest-digest MANIFEST_PIN --expected-application-digest APPLICATION_PIN --implementation-run-id ORIGINAL_RUN
```

Pair optional `--reviewed-envelope REVIEWED --expected-envelope-digest ENVELOPE_PIN`
to check current acceptance. Follow lifecycle `needs-review|skip-accepted` rules;
resume grants no extraction, durable skip token, termination proof or budget renewal.

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
