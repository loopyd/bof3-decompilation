# Target-local lane protocol

## Launch

Use [workflow file](../../../../.pi/workflows/bof3-lift-lanes.js), never a session-built
lane loop. One invocation runs lift, conditional review, repair and naming
with per-lane failure isolation and bounded concurrency.

```js
subagent({
  workflowScriptPath: ".pi/workflows/bof3-lift-lanes.js",
  args: { lanes: [{ key: "l1", target: "<TARGET>",
                    refs: ["<DISC_ID>#<INDEX>@0xADDRESS"] }] },
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
