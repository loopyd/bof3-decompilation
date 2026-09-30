# Naming terminal acceptance

Read-only current no-op/exhaustion for existing capability-backed exhausted rows.
Structural `validate`/capabilities are not semantic acceptance. Proposals and
applications keep separate owners; never rewrite report/capability/receipt/identity.

```sh
bin/harness naming terminal-verify TARGET REPORT --transaction data:NAME --parent-attestation PARENT_JSON --expected-parent-digest PARENT_PIN --evidence-root CANONICAL_ROOT
```

Omit root only for default-root evidence. Preparation uses read-only
`harness.naming.terminal.terminal_binding(root,target,report,transaction)` under
same evidence context. Binding covers canonical target/selector/transaction/report
bytes/mode, row/capability digests, retained namespace evidence digest, missing fact
and owner-derived current repository/scope/tooling digest. Inventory/index/manifest/
row/receipt/capability validators must pass. Derived open/unavailable evidence
rejects. Unrelated row failures remain separate `full_report.blocker`.

Closed parent `bof3.naming-terminal-parent-review/v1` exactly:
`schema`, `accepted:true`, `parent_run_id`, `evidence_preparation_run_id`,
`reviewer_run_id`, `binding`, `preparation_artifact`, `review_artifact`,
`ladder_exhausted:true`, `unresolved_leads:[]`, `ceiling_rationale`.
Three actual nonempty distinct run IDs. Parent pin is SHA-256 compact sorted-key
JSON, retained externally, never inferred from supplied file.

Artifact references exactly canonical absolute `path` and byte `sha256`; retained,
nonempty, symlink-free. Preparation JSON exactly
`schema:bof3.naming-terminal-preparation/v1`, `evidence_preparation_run_id`, `binding`.
Semantic review JSON exactly `schema:bof3.naming-terminal-semantic-review/v1`,
`reviewer_run_id`, `binding`, `ladder_exhausted`, `unresolved_leads`,
`ceiling_rationale`. Shared fields match parent. Rationale exactly `missing_fact`
from binding, nonempty `explanation` of static ceiling despite gap and nonempty
retained path/hash `evidence`.

False ladder, executable open leads, contradictory review, missing evidence,
self-review/stale state reject. Parent/reviewer inspect referenced evidence;
missing second consumer alone is not static ceiling. Validator cannot infer
semantic truth or discover omitted leads. Local parent attribution, not historical
actor authentication. Preparation ID is actual run assembling/inspecting submission;
never invent historical collection identity. Tooling supplies no production attestation.

Success `selected_row_accepted:true`, separate `full_report`,
`production_complete:false`. Unfiltered successful `validate` with `complete:true`
still mandatory. Row acceptance neither accepts others nor grants identity writes,
retry/recovery/scheduler authority or full-target completion.
