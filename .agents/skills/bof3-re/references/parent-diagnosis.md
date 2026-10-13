# Parent diagnosis and retained audit

Read [native execution](native-execution.md) before either command.

## Diagnose existing C

```sh
bin/harness decomp diagnose REQUEST --expected-request-digest PIN --output out/reviews/lift-diagnosis/NAME --deadline ORIGINAL_MONOTONIC_CUTOFF
```

`scripts/mission.py diagnose` forwards unchanged. Existing claimed lift only;
absent C requires first-source protocol. No source edit, model, restoration,
acceptance, budget debit/renewal or campaign advancement.

Closed `bof3.lift-request/v1`: `schema`, canonical `selector`, `source`, sorted
unique `paths` of at most 12 entries, nonempty `task` at most 8 KiB,
current `adopted_baseline`, `capabilities:"mission-scoped-source-edit"`.
Potential write scope is not diagnosis write authority. Existing ownership must
match; shared source/header edits reject. Externally retain request digest.

Capture validates mission/index freshness, original span/PS-X header, source/config
ownership, native inputs/tools/environment, workspace metadata and exact raw index.
Stale analysis blocks; never refresh or repin here. Supply original monotonic work
cutoff plus separate cleanup hard-stop. Each native command cap 120s/1 MiB combined
output. Lease excludes cooperating writers, not manual editors or prior workers.

Cold checks configure CMake/Ninja, remove selected object and verify absence, run
full JSON asm-diff plus byte-match, validate schemas/identity/span/instruction-byte
agreement and unchanged PRE/native inputs. Exact and valid partial results are
diagnostics, not acceptance.

Fresh direct-child output directory only. Retain mission/policy, invocation cutoff,
started/spawn/terminal records, fsynced stdout/stderr and `bof3.lift-diagnosis/v1`.
Result `clock` binds boot/original cutoff and hashes all 23 input/native artifacts.
Stdout paths/pins/status only. PRE/source evidence stays nonpublic and uncommitted.
Existing output rejects unchanged. Failure preserves evidence; leftover result is
not success. Saved PID/free lease never proves cleanup or termination.

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

Run only after authorized edit and confirmed writer-tree termination. Helper
`scripts/mission.py audit` forwards unchanged. Reuse original mission/policy,
boot-bound cutoff and external pins. No replacement-clock flag. Missing/failed/
altered/legacy diagnosis without clock/artifact coverage rejects. Retired CLI
writer/reviewer slots remain historical; never strip slots or upgrade proofs.
Prepare fresh mission only at authorized checkpoint.

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

Fresh direct-child audit output only. Failure retains candidate/evidence, grants
no retry/restore/acceptance. Parent accounts every invocation against original
budget; neither command runs a model, repair scheduler or recovery loop.
