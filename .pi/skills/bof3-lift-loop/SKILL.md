---
name: bof3-lift-loop
description: Execute or resume BOF3 lift, naming, type, macro and cleanup campaigns through bounded active-session missions, target-local lanes and serialized shared writes. No harness model launcher.
---

# BOF3 lift loop

Use this skill and branch references, not user documentation. Bind caller-supplied mission scope,
evidence pins and original budgets; never bind this skill to a named plan file.
Goal: 100% 1:1 human-readable BOF3 source, with evidenced names, types, useful C89
macros and required independent acceptance. Finite missions bound work, not goal.
This skill instructs the active-session operator; not a script or scheduler.

## Session and ownership

- Use exposed session tools for delegation, observation and closure. Inspect host
  capability first. No `codex`, model SDK/API, Pi runner or detached substitute
  through shell, Python, MCP or wrappers. Workers spawn no orchestration layer.
- Missing delegation permits safe local inspection/tooling, not invented worker IDs,
  parallelism or self-review counted as independent review.
- All lift lanes use [canonical workflow](../../workflows/bof3-lift-lanes.js).
  One disjoint target per lane. Serialize shared writes; preserve unrelated dirty
  work. Never discard/stage it to manufacture worktree isolation.
- Lane owns target sources, owning header and target `splat.yaml`/`symbols.txt`/
  `target.toml`. Parent owns shared headers, `config/splat.yaml`, index, audit/status
  caches, naming baseline and Git.
- Parent refreshes/reviews and feature commits have standing authority. Preserve
  frozen proofs; no push, installs, permission weakening or retry after denial.
- Select `bof3-naming`, `bof3-docs`, `psx-rizin` when relevant without repeated skill
  approval. Parent supplies bounded mode, paths and inputs. Domain, sandbox,
  evidence, independent-review and rollback gates remain binding.

## Mission protocol

Before delegation, read [mission and recovery protocol](references/mission-protocol.md).
Freeze finite queue, original deadlines, attempts/repairs, owner inputs and external
pins. Resume consumption, never replace frozen five-entry pilot or reset budgets.
Active session/selected plan owns dispositions; artifacts supply evidence, not a
new campaign database or model launcher.

## Batch and lane protocol

Before selecting or launching lanes, read [lane protocol](references/lane-protocol.md).
Invoke workflow file with registry refs, `maxSelectors`, `repairRounds` and bounded
concurrency. No session-built lane loop. One combined target gate measures edited
selectors; exact PASS skips lane reviewer, non-exact routes to bounded review/repair.

Every writer harness invocation counts toward cumulative 12, including queries,
help/examples and failures. One sweep, at most four variants across all repairs.
Carry complete ledger; missing consumption evidence blocks experiments.

## Operator stages

Read selected domain skill before its stage. Each stage must return evidence-bound
disposition before successor starts.

1. **Reconcile.** Inspect dirty tree and real handles. Skip accepted work only after
   current owner verification. Uncertain application/cleanup stops guarded writes.
2. **Select/scout.** Freeze fresh target-qualified opportunities and original bytes;
   exclude duplicates, accepted/owned work and unresolved shared effects. Parallel
   independent reads allowed. Bulk template/carve/clean-C-requeue levers measured
   exhausted. Select per-function via `bin/harness lift next`, shortest surviving
   boundaries, then partials missing an instruction/constant. Allocation/ordering
   residuals mark measured clean-C ceiling, not spelling debt.
3. **Lift.** `$bof3-re`, one function, explicit paths, pre-edit diagnosis, clean C89,
   original-byte proof, truthful metadata and live gate. Verify original return ABI;
   decompiler return guesses prove nothing. Domain owner retains rollback gates.
4. **Review.** Distinct reviewer examines changed content, original instructions,
   calls/representation and scoped checks; generated validation artifacts only.
   Lane reviewer handles non-exact selectors only. Parent resolves unavailable
   native checks. Serial batch gate waits for all lanes to close.
5. **Name.** `$bof3-naming`; separate opportunity/evidence/audit/transaction authority.
   Reuse valid target evidence. Preserve storage/ABI/address identity. Exact lane
   confirms semantic name/module and filename/Splat/map/header/manifest agreement;
   edit requires renewed gate. Reviewer supplies missing naming evidence. Selected
   row success never replaces unfiltered `complete:true` or separate approval.
6. **Types.** `$bof3-types`; prove widths, signedness, fields, layout and ownership.
   Reviewed representations only. Shared promotion keeps private-proof/common-state
   gates. Unknown does not justify no-op.
7. **Macros.** `$bof3-macros`; pin instruction floor/top-N, require four uses minimum,
   size-first ranking and human value. Review evaluation count, side effects,
   precedence, conversions, aliasing, volatility and every consumer. Account
   extracted/rejected/deferred outcomes; never manufacture abstractions.
8. **Finalize.** Independent combined-source review, native/domain gates and current
   envelopes. Evidence-backed naming/type/macro no-ops only. Route docs repair to
   `$bof3-docs`. Capture reusable residual class, decisive clean-C shape and measured
   before/after in lifting references, not per-function source annotations.
9. **Account/continue.** Accept only verified/reviewed scope. Account every member,
   preserve pins, refresh affected snapshots/index at safe checkpoints, requery
   consumers, commit owned accepted work, continue within original limits. No push.

## Boundary conflicts blocking the index

On `conflicting function/global symbol: TARGET:NAME`, read
[boundary conflict protocol](references/boundary-conflicts.md) before probing or editing.
Do not skip refresh, rename away conflict or weaken index validation. Original-byte
classification, independent review and successful index refresh all required.

## Tooling, stop and resume

Read [mission and recovery protocol](references/mission-protocol.md#tooling-stop-and-resume)
before native execution, interruption recovery or continuation. Unmeasured results
remain `unverified`/null. Green tooling, one exact lift or accounted blocked queue
never proves whole-game completion.

Before native work read [execution contract](../bof3-re/references/native-execution.md).
Return tool count, wall time, method, actual handles and per-selector disposition.
Parent archives measurements; reusable directives belong in skill references.
