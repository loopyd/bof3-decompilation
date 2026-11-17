# Target-local lane protocol

## Launch

Use [workflow file](../../../../.pi/workflows/bof3-lift-lanes.js), never a session-built
lane loop. Each invocation runs one wave of at most eight stages. Lift, review,
repair, naming and harvest continue across waves with per-lane failure isolation.

```js
subagent({
  workflow: ".pi/workflows/bof3-lift-lanes.js",
  async: true,
  args: { lanes: [
    { key: "l1", target: "<TARGET>", refs: ["<DISC_ID>#<INDEX>@0xADDRESS"] },
    { key: "l2", target: "<TARGET>", kind: "partial",
      refs: ["<DISC_ID>#<INDEX>@0xADDRESS"] }
  ] },
  options: { globalConcurrencyLimit: 8 }
})
```

After each completion notice, repeat the request with identical `args` and
`options.missionId` set to the first receipt's `details.missionId`. Continue until
`pending:false`, then perform parent acceptance. Never overlap waves or reuse a
mission for a different batch. Keep concurrency at eight. Mission state retains
settled reports; completed stages are replayed for routing, not relaunched. A
failed stage stays failed. A stage left `running` after interruption requires
parent reconciliation before further launches. `mission:false` is unsupported.

Each wave launches its children before waiting for their results. Successor stages
run only in the next invocation, avoiding goal post-stop refusal of background
`subagent_launch` calls. Do not retry a refusal in the stopped turn or bypass its
permission checks. Keep the parent turn active until the wave's planned children
have started; the initial async receipt alone does not prove launch approval.

Raise the open-tool notice threshold on every batch: pass
`options: { control: { activeNoticeAfterMs: 3600000 } }`. The default is four
minutes, and a target gate routinely runs longer, so each lane fires
`needs_attention` control events while it is working normally. Every such event
starts a parent turn, and under a goal an undecided turn pauses the campaign.
With the threshold above the longest gate, only a real failure or a supervisor
request wakes the parent.

End every parent turn with a goal disposition, never prose alone. A saved `wait`
does not survive into a new turn: replying without `update_goal` leaves no
disposition and the goal pauses on the next checkpoint. After re-gating or
committing, save the next `wait` (or `ready`) in the same turn.

Pass `ledger` on a lane to hand it the closed shapes from earlier runs: an array of
`{shape, measured}` entries the parent assembles from the target's earlier reports.
Rows whose measurement is the same emitted class - same instruction and byte counts, same
first mismatch - are one closed class, not two experiments; the workflow collapses them
and reports the attempt count behind each class. Four rows of `42/46; 184->176` describe
one dead end, so assemble distinct measurements instead. The
brief then lists them as closed history the lane must not re-author or re-measure, and
requires it to say which shape it is deliberately not repeating. Assemble it before
launch; the workflow cannot read files.

Three lane kinds, one workflow. `kind: "naming"` is naming debt rather than lifting: the
function already matches, only its identity is open, so the lane runs one namer on the
lowest rung and no model rung is spent. Select those rows with
`bin/harness naming list TARGET --json`; a naming lane reports `naming-applied`,
`naming-no-change`, `naming-deferred` or `naming-blocked`, and an applied rename must keep
the owning gate at 100% or be restored.

Scope a naming lane to functions authored in its own target. A function whose body lives in
another target is also named in a shared prototype or macro, so a single-target rename forks
the name into two spellings; those rows are deferred with a fan-out inventory instead of
renamed. A selector must also be measured exact: renaming a partial proves nothing, because
the gate fails on a body nobody edited, so the lane reports it blocked for a lift instead.
Verify once per lane - one naming check and, only if something was edited, one gate
covering every selector - because a gate on a large target builds the whole target.

The default ladder contains `{provider:"ninerouter",model:"ds-combo",thinking:"xhigh"}`
then `{provider:"ninerouter",model:"gpt-combo",thinking:"high"}`. `args.models` replaces
that list; `args.model` selects one entry. A lane's `models` replaces its ladder.
Entries accept objects with `provider`, `model` and `thinking`, or fully qualified
strings such as `ninerouter/ds-combo:xhigh`. Lists may mix both forms. Strings
normalize to objects and require an explicit thinking suffix. Thinking is the union
`"off" | "minimal" | "low" | "medium" | "high" | "xhigh" | "max"`. Empty override
lists and malformed entries fail before launch. Dispatch uses Pi Subagents'
`provider/model:thinking` suffix, not a separate thinking argument.

A fresh lane starts on the first rung. A lane marked `kind: "partial"` starts on the
strongest rung because the cheap rung already produced its residual. Each later rung
receives the previous rung's report and attacks the named residual. A 100% gate PASS ends the lane and goes to the
naming transaction; the reviewer fires only when the whole ladder ended non-exact, and
repair then runs on the strongest rung. Each rung returns a compact `ledger` of the
shapes it measured, and the next rung, review and repair all inherit it. The ledger is
knowledge, not a spent budget: every shape on it is closed, and the receiving rung has
its own attempt allowance for shapes that are not. That is what stops an elevated
partial from re-running experiments the cheap rung already burned, while still letting
the stronger model try the one thing the earlier ledger left open. After every lane closes, one harvest child
improves the `bof3-re` references from that batch's exact lanes only - any exact, whether
a first seed, a partial closed on a later rung, a repaired exact, or an exact lift whose
naming stage was blocked. A partial lane that closes on the strong rung is the most
useful lesson: it records which lever closed which residual class.

One disjoint target per lane. Lane writes target-owned `src/bof3/**`, owning header
and target `splat.yaml`/`symbols.txt`/`target.toml` only. Parent owns shared headers,
`config/splat.yaml`, index, caches, naming baseline and Git. Report shared changes;
do not apply them. No lane, reviewer, namer or harvest child runs `git add`,
`git commit` or `git push`; the parent commits the batch after re-gating every
selector. Preserve unrelated dirty work. Worst-first targets
`emi/etc/game/00`, `emi/battle/battle/15`, `emi/battle/battle/03` get fewer selectors.

Harness resolves registry refs. Freeze supplied membership; report data/no-op/blocked
instead of substituting selectors. Parent chooses follow-up with `lift next TARGET`.
A gate that answers `no authored source` while the target's config changed means the lane
turned a row over without finishing it. Revert that target rather than keeping a
promotion nobody proved, and leave the boundary for a lane that can close it.

## Routing and evidence

- Explicit JSON `results`: one row per supplied selector, lane/target identity,
  final `gate.verdict`. Nested attempt statuses or narrative never trigger review.
  Missing/malformed/contradictory rows block. Same parser for initial and repaired
  results. Reviewer top-level `workflow: accepted|repair|blocked`; unknown blocks.
  Repair requires at least one authored partial row and each such row's `source_path`/`source_sha256` plus reviewer
  `repair:{lever,proposal_sha256,evidence_pins:[{path,sha256}]}`. Use stable eligible
  kebab-case lever, retained proposed source/shape hash and nonempty distinct-path
  evidence pins. Hashes are 64 lowercase hex. Missing/invalid identity blocks.
  Routing compares content pins; parent/native gates authenticate artifacts,
  eligibility, native fidelity and semantics. Preserve exact/data/no-op/blocked
  rows in membership/history; repair only authored partial candidates.
- Runtime acceptance stays in separate fence; child `output:false`. Bound output
  files append notice that breaks strict JSON routing. Aggregate retains actual
  reports, run IDs and artifact references.
- Format failure: report-only retained-child probe, with no new lift. Saved-file validity/runtime attestation alone never proves routable output.
- Reuse one unpiped reverse prefill and original asm/bytes. `m2c`/`m2ctx` only for
  concrete gaps. Search target declarations, then bounded siblings; exclude
  generated/session trees.

## Lift and naming

1. Before first edit, classify [eligible levers](../../bof3-re/SKILL.md#eligible-condition-levers).
   Data-shaped bytes: `analysis data-candidates`. Fragment/head in another segment
   or bare `bin`: `analysis carve`, probe only; parent owns layout. Byte-identical
   exact sibling: `lift clone`. Clean-C shape alternatives: bounded `lift sweep`.
   Revisit eligibility only on new evidence.
2. Prepare evidenced name/module plus filename/Splat/map/header/manifest agreement
   before first gate. Unknown roles stay address-anchored with missing evidence.
3. Batch edited selectors in one `bin/harness lift gate --target TARGET SELECTOR [...]`.
   One native measurement per selector yields instruction and byte comparisons;
   symbols/Splat once per target, internal race retries. No repeated checks on
   unchanged candidate. Keep existing-C pre-edit diagnosis and required companion/
   owned-diff checks.
4. Exact PASS skips lane reviewer. Non-exact goes to distinct reviewer for proposed
   solution and evidence-backed repair until acceptance, a genuine blocker or
   reviewed ladder exhaustion.
5. Naming mode after PASS confirms ownership agreement without rewriting correct
   source. Any later edit invalidates gate and requires another. Request missing
   naming evidence; never invent semantic roles.

## Cumulative history

Record every writer command and outcome, including queries and failures. Pass the
full ordered attempt/lever ledger to review/repair, not report tails. Preserve
missing-history notes; gaps do not impose an execution allowance. Repeated
experiments require changed evidence explaining the expected effect. Routing keys
sorted selector/source hashes, lever, proposal hash and deduplicated evidence
hashes. Paths remain required in reports but do not key repetition. Identical
content blocks another repair; wording, copied paths, counters, rounds and ledgers
cannot bypass this.

Use `lift sweep` for eligible shape alternatives, without call or variant quotas.
No per-variant diff loop or local compile/compare harness. Keep the owning compiler
and flags fixed; flag catalogs and compiler-variant batches are not lane rungs.
At a stall, follow [compiler and runtime evidence](../../bof3-re/references/matching-knowledge.md#compiler-evidence).
`lift permute` still requires parent authorization. No lane `just check`,
`just check-all` or `just check-unit`. Retain best coherent partial and owned facts
for review; revert regressions. Parent owns final restoration.

## Serial batch close

After all lanes terminate, parent regenerates Splat for changed targets, refreshes
index and builds only changed target objects with `bin/harness build TARGET`.
Never full `bin/harness build all` or `just build`. Verify every resolved/named
function compiles scoped and meets source-organization standard. Record `lift status`
coverage delta; reconcile naming-baseline rows for newly introduced raw spellings.
Parent acceptance still requires domain evidence; batch counts never prove whole-game
closure.
