# Parent diagnosis and retained audit

Read [native execution](native-execution.md) before either command.

## Diagnose existing C

```sh
bin/harness decomp diagnose REQUEST --expected-request-digest PIN --output out/reviews/lift-diagnosis/NAME
```

`scripts/mission.py diagnose` forwards unchanged. Diagnose existing claimed lifts;
use first-source protocol for absent C. Diagnosis grants no source edits, model,
restoration, acceptance, budget debit/renewal or campaign advancement.

Closed `bof3.lift-request/v1`: `schema`, canonical `selector`, `source`, sorted
unique `paths` of at most 12 entries, nonempty `task` at most 8 KiB,
current `adopted_baseline`, `capabilities:"mission-scoped-source-edit"`.
Potential write scope is not diagnosis write authority. Existing ownership must
match; shared source/header edits reject. Externally retain request digest.

Validate mission/index freshness, original span/PS-X header, source/config ownership,
native inputs/tools/environment, workspace metadata and exact raw index. Block stale
analysis without refresh/repinning. Cap each native command's combined output at
1 MiB. The lease excludes cooperating writers only, not manual editors/prior workers.

Configure CMake/Ninja, remove the selected object and verify absence, then run full
JSON asm-diff/byte-match. Validate schemas/identity/span/instruction-byte agreement
and unchanged PRE/native inputs. Exact/valid partial results remain diagnostics.

Use a fresh direct-child output directory; reject existing output unchanged.
Retain mission/policy, started/spawn/terminal records, fsynced stdout/stderr and
`bof3.lift-diagnosis/v1`, hashing all 23 input/native artifacts. Print only paths/
pins/status; keep PRE/source evidence nonpublic/uncommitted. Preserve failure
evidence without claiming success. PID/free lease proves no cleanup or termination.

## Writer report

Two JSON fences, `json` mission then `acceptance-report`. Acceptance object has
exactly `pre_mission` with mission/baseline digests, `commands`, `attempts`, `risks`,
`retained_candidate` source path, `remaining_candidates`,
`snapshot_index_refresh_required`, `staged_index_changed`, `parent_restore_required`,
`matching_aid_approvals`. Commands/risks/candidates/approvals are lists; three flags
are booleans. Each ordered attempt has `order`, `diagnosis`, `change`, `command`,
`result`, `retained`, `reason`. Do not infer JSON from legacy prose. Parent compares
claims to measurement; parsing or byte equality never supplies semantic review.

## Audit retained candidate

```sh
bin/harness decomp audit DIAGNOSIS PROPOSAL --expected-diagnosis-digest PIN --expected-proposal-sha256 SHA256 --output out/reviews/lift-audit/AFTER
```

After authorized edits and confirmed writer-tree termination, run
`scripts/mission.py audit` with unchanged arguments and original mission/policy/pins.
Reject missing/failed/altered/legacy diagnosis without artifact coverage. Preserve
historical retired writer/reviewer slots without upgrading proofs. Prepare fresh
missions only at authorized checkpoints.

Manifest/native inputs, index, unrelated workspace and source-directory guards
remain. No index refresh between diagnosis/audit. Report refresh for later parent
checkpoint. Default `--proposal-format mission` validates measured report above.
For unavailable compiler, explicit `--proposal-format unmeasured` requires writer
`status:"unverified"`, `match_percent:null`, truthful attempts/paths/risks/flags.
Parent measurement stays separate; never rewrite writer claim into exactness.

Read-only audit runs cold gates, checks evidence/proposal before and after, emits
`bof3.lift-audit/v3` with pins/artifacts/comparison. Lower instruction score or lost
byte exactness requests parent restoration review, never restores. Comparison is
against this diagnosis, not best-of-history or semantics. Exact/partial remains
`needs-independent-review` unless restoration review required. Exit 0 is not
acceptance; affected consumers, aids, semantics and domain gates remain owed.

Use fresh direct-child audit output. Retain failed candidates/evidence without
retry/restore/acceptance permission. Parent accounts every invocation against the
original budget; neither command runs a model, repair scheduler or recovery loop.
