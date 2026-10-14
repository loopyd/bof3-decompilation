# Existing abstractions

Before unchanged assessment, read [semantic guards](discovery-ranking.md#semantic-review)
and [application acceptance](application.md#parent-review). Preserve useful macros;
do not re-extract for receipts. Accept current no-op only, without reconstructing
private history/shared promotion.

```sh
bin/harness macros prepare-existing REQUEST > MANIFEST
bin/harness macros check-existing MANIFEST --expected-manifest-digest MANIFEST_PIN --implementation-run-id RUN --output CHECK
bin/harness macros review-existing CHECK PARENT_REVIEW --expected-inspection-digest CHECK_PIN --output DISPOSITION
bin/harness macros verify-existing DISPOSITION --expected-envelope-digest DISPOSITION_PIN
bin/harness macros account-existing REFERENCES
```

Use closed `bof3.macro-existing-request/v1`: `schema`, `target`, sorted distinct
`targets`, `candidate_artifact`, `candidate_id`, `candidate_fingerprint`, sorted
distinct `definition_ids`/`affected_functions`, `adopted_baseline`, nonempty
`rationale` proving worthwhile abstraction/no edit. Require four template uses;
exclude constant/expression-only leads. Require `block_min_instructions`/`ranking`
for blocks and reject those fields for non-blocks.

Use `shared_template` for multiple targets and `local_template` for one. Resolve
all eight guards and mapping/all-use observations. Freeze candidate membership
separately from full consumer/check scope. Require exact functions and per-target
gates. Cover direct/transitive consumers, other parameters and outside-candidate
uses. Block ambiguous mapping even with covered siblings. Freeze whole owner files
without edits; mutation still requires whole-file consumer coverage. Lexical/include
leads prove neither definition selection nor argument safety.

Warm the native build before freezing. Check under writer exclusion; preserve
PRE/POST source/index/tooling/environment/full adopted workspace/build closure
around every gate/publication. Use exclusive new canonical `out/reviews/evidence`
outputs; reject reuse before native work. Preserve modes/types/permissions.
Failure, unexpected writes or uncertain cleanup grants no rollback authority;
retain state for scoped recovery.

Keep `checked:true` separate from acceptance. Parent supplies `bof3.macro-existing-parent-review/v1`
with complete application-style binding/preservation, three distinct actual run IDs
and retained nonempty absolute review artifact. Opportunity reviewer independent of
execution and parent. Externally pin manifest/inspection/disposition each handoff.
Final verification rederives scope/checks/current closure/receipts without native
runs or evidence edits.

Only verified disposition returns `accepted:true`, `existing_abstraction_count:1`,
`safe_application_count:0`. Account explicit
`{path,expected_envelope_digest}` list, rejects duplicate candidate IDs, counts
existing work separately without rewriting inventory. A frozen-five macro-only
`existing` claim uses `{disposition:Path,expected_envelope_digest}`; original
baseline/pilot freshness still binds. Do not repin stale inputs or use these schemas
for mutation/revalidation/private-proof/promotion/resume.
