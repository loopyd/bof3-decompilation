# Existing abstractions

Read [semantic guards](discovery-ranking.md#semantic-review) and
[application acceptance](application.md#parent-review) before unchanged assessment.
Do not unshare/re-extract useful macros to manufacture mutation receipts. This
lifecycle accepts current no-op, never reconstructs private history/shared promotion.

```sh
bin/harness macros prepare-existing REQUEST > MANIFEST
bin/harness macros check-existing MANIFEST --expected-manifest-digest MANIFEST_PIN --implementation-run-id RUN --deadline ORIGINAL_CUTOFF --output CHECK
bin/harness macros review-existing CHECK PARENT_REVIEW --expected-inspection-digest CHECK_PIN --output DISPOSITION
bin/harness macros verify-existing DISPOSITION --expected-envelope-digest DISPOSITION_PIN
bin/harness macros account-existing REFERENCES
```

Closed `bof3.macro-existing-request/v1`: `schema`, `target`, sorted distinct
`targets`, `candidate_artifact`, `candidate_id`, `candidate_fingerprint`, sorted
distinct `definition_ids`/`affected_functions`, `adopted_baseline`, nonempty
`rationale` proving worthwhile abstraction/no edit. Blocks additionally require
`block_min_instructions`/`ranking`; non-block supplied fields reject. Templates need
four uses; constant/expression-only leads excluded.

Reviewed-opportunity artifact uses `shared_template` for multiple targets,
`local_template` for one. All eight guards, mapping/all-use observations required.
Freeze candidate members separately from larger consumer/check scope. Every covered
function already exact; each target gets gates. Cover known direct/transitive
consumers, including other parameter values and outside-candidate uses. Ambiguous
mapping blocks even when sibling covered. Whole owner files frozen, not edited;
mutation scope still owes whole writable-file consumer coverage. Lexical/include
leads never prove actual definition selection or argument safety.

Warm native build before freezing. Check under writer exclusion, no source changes;
PRE/POST source/index/tooling/environment/full adopted workspace/build closure
unchanged around every gate and publication. Outputs exclusive new canonical paths
under `out/reviews/evidence`; reuse rejects before native work. Preserve observed
modes/types, no permission change. Original absolute cutoff, no fresh duration.
Gate failure/unexpected writes/cleanup uncertainty grants no rollback authority;
retain state for separately scoped recovery.

`checked:true` not acceptance. Parent supplies `bof3.macro-existing-parent-review/v1`
with complete application-style binding/preservation, three distinct actual run IDs
and retained nonempty absolute review artifact. Opportunity reviewer independent of
execution and parent. Externally pin manifest/inspection/disposition each handoff.
Final verification rederives scope/checks/current closure/receipts without native
runs or evidence edits.

Only verified disposition returns `accepted:true`, `existing_abstraction_count:1`,
`safe_application_count:0`. Accounting consumes explicit
`{path,expected_envelope_digest}` list, rejects duplicate candidate IDs, counts
existing work separately without rewriting inventory. Frozen-five macro-only
`existing` claim uses `{disposition:Path,expected_envelope_digest}`; original
baseline/pilot freshness still binds. No repinning stale inputs. These schemas
reject at mutation/revalidation/private-proof/promotion/resume entry points.
