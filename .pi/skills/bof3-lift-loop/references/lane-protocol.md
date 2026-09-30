# Target-local lane protocol

## Launch

Use [workflow file](../../../workflows/bof3-lift-lanes.js), never a session-built
lane loop. One invocation runs lift, conditional review, bounded repair and naming
with per-lane failure isolation and bounded concurrency.

```js
subagent({
  workflowScriptPath: ".pi/workflows/bof3-lift-lanes.js",
  args: { lanes: [{ key: "l1", target: "<TARGET>",
                    refs: ["<DISC_ID>#<INDEX>@0xADDRESS"] }],
          maxSelectors: 2, repairRounds: 1 },
  globalConcurrencyLimit: 8
})
```

One disjoint target per lane. Lane writes target-owned `src/bof3/**`, owning header
and target `splat.yaml`/`symbols.txt`/`target.toml` only. Parent owns shared headers,
`config/splat.yaml`, index, caches, naming baseline and Git. Report shared changes;
do not apply them. Preserve unrelated dirty work. Worst-first targets
`emi/etc/game/00`, `emi/battle/battle/15`, `emi/battle/battle/03` get fewer selectors.

Harness resolves registry refs. Freeze supplied membership; report data/no-op/blocked
instead of substituting selectors. Parent chooses follow-up with `lift next TARGET`.

## Routing and evidence

- Explicit JSON `results`: one row per supplied selector, lane/target identity,
  final `gate.verdict`. Nested attempt statuses or narrative never trigger review.
  Missing/malformed/contradictory rows block. Same parser for initial and repaired
  results. Reviewer top-level `workflow: accepted|repair|blocked`; unknown blocks.
  Routing validates reports, not native fidelity or semantic acceptance.
- Runtime acceptance stays in separate fence; child `output:false`. Bound output
  files append notice that breaks strict JSON routing. Aggregate retains actual
  reports, run IDs and artifact references.
- Format failure: report-only retained-child probe, not new lift or renewed attempt
  budget. Saved-file validity/runtime attestation alone never proves routable output.
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
   solution and bounded repair within `repairRounds`.
5. Naming mode after PASS confirms ownership agreement without rewriting correct
   source. Any later edit invalidates gate and requires another. Request missing
   naming evidence; never invent semantic roles.

## Cumulative bounds

Every writer `bin/harness` invocation counts toward 12 across lift and all repairs:
help/examples, queries, failures; commands sharing one shell call count separately.
Record each command/count when issued. Reserve diagnosis/final validation before
experiments; no exemptions or resets on repair.

At most one `bin/harness lift sweep`, four variants total. No per-variant diff loop
or local compile/compare harness. `lift flag-search`/`lift permute` require parent
authorization. No lane `just check`, `just check-all` or `just check-unit`.

Pass full ordered attempt/lever ledger and consumed sweep budget to review/repair,
not report tails. Missing evidence blocks experiments. Retain best coherent partial
and owned facts for review; revert regressions. Parent owns final restoration.

## Serial batch close

After all lanes terminate, parent regenerates Splat for changed targets, refreshes
index and builds only changed target objects with `bin/harness build TARGET`.
Never full `bin/harness build all` or `just build`. Verify every resolved/named
function compiles scoped and meets source-organization standard. Record `lift status`
coverage delta; reconcile naming-baseline rows for newly introduced raw spellings.
Parent acceptance still requires domain evidence; batch counts never prove whole-game
closure.
