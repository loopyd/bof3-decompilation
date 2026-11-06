// BOF3 autonomous lift/naming lane workflow.
//
// Usage (file-backed; the host reads this file before launching):
//   subagent({
//     workflowScriptPath: ".pi/workflows/bof3-lift-lanes.js",
//     args: {
//       lanes: [                       // one entry per lane; run them in parallel
//         { key: "l1", target: "emi/battle/battle/15",
//           refs: ["BIN/BATTLE/BATTLE.EMI#15@0x800B2104"] },
//         { key: "l2", target: "emi/etc/shop/00",
//           refs: ["BIN/ETC/SHOP.EMI#0@0x801D7208"] }
//       ]
//     },
//     globalConcurrencyLimit: 8
//   })
//
// `refs` are registry references (`DISC_ID#INDEX@0xADDRESS`); every lookup is done
// through harness commands (`bin/harness lift m2c|gate`), which resolve the ref
// themselves. `target` is the manifest target id used by the gate's second
// argument and by the lane's write set.
//
// Stage order per lane:
//   lift (model ladder) -> naming transaction (exact only) -> review (non-exact only)
//     -> repair rounds -> review again
// The ladder defaults to ["ninerouter/ds-combo", "ninerouter/gpt-combo"]; pass
// `args.models` to reorder it and `args.model` for a single-rung run. Each rung gets
// the previous rung's report, so it continues from measured evidence. A 100% gate
// PASS ends the lane and goes straight to naming; the reviewer only sees a ladder
// that ended non-exact.
//
// A lane whose refs are existing partials passes `kind: "partial"` and starts on the
// strongest rung: the cheap rung already produced that residual, so repeating it would
// spend a child to relearn the report. A lane whose refs are naming debt passes
// `kind: "naming"` and skips the ladder entirely - one namer, no lift stage. `models` on a lane overrides the ladder for that
// lane only. Naming always runs on the lowest rung, because it applies an identity from
// evidence that already exists rather than searching for one; review and repair run on
// the lane's strongest rung.
//
// After every lane closes, one harvest child rewrites the bof3-re references from
// this batch's exact results only ($skill-master authoring, $unslop prose, $caveman
// compaction) and runs the skills tests plus the docs drift check. It never commits.
//
// Lessons encoded here (all measured in batches 001-005):
//  - one target per lane, disjoint target-local write sets;
//  - a `lift gate` PASS is exact, so NO review is spent on an exact selector;
//  - a non-exact selector goes to a reviewer, then to evidence-backed repair;
//  - per-lane failure isolation: an error becomes that lane's result
//    and never rejects the batch;
//  - bounded concurrency (the caller's globalConcurrencyLimit), never 20;
//  - lanes never run `just check`, never touch another target or shared config.
//
// Inputs are plain JSON only; no secrets.

const rawLanes = Array.isArray(args.lanes) ? args.lanes : [];

if (rawLanes.length === 0) {
  return { ok: false, error: "bof3-lift-lanes: args.lanes must list at least one { key, target, refs } lane" };
}

// Model ladder. The first model lifts; when it ends non-exact the SAME lane and
// selectors are retried by the next model before any review is spent, so a reviewer
// only sees the rung that the whole ladder failed to close. `args.models` overrides
// the order; `args.model` is still accepted as a single-entry ladder.
const DEFAULT_LADDER = ["ninerouter/ds-combo", "ninerouter/gpt-combo"];
const requestedModels = Array.isArray(args.models)
  ? args.models
  : typeof args.model === "string" && args.model.trim()
    ? [args.model]
    : [];
const modelLadder = (
  requestedModels.length ? requestedModels : DEFAULT_LADDER
).map((value) => String(value || "").trim()).filter(Boolean);
if (modelLadder.length === 0) {
  return { ok: false, error: "bof3-lift-lanes: args.models resolved to an empty model ladder" };
}
// A lane's own ladder, and the rung it starts on. A partial has already been lifted
// once by the cheap rung, so it enters at the strongest model: repeating the rung that
// produced the residual spends a child to learn what the report already says.
const laneLadder = (lane) => {
  const models = lane.models.length ? lane.models : modelLadder;
  return { models, start: lane.kind === "partial" ? models.length - 1 : 0 };
};
const laneModel = (lane, index) => {
  const { models } = laneLadder(lane);
  return models[Math.min(index, models.length - 1)];
};
const laneLastModel = (lane) => laneModel(lane, laneLadder(lane).models.length - 1);
const harvestModel = modelLadder[0];

const lanes = rawLanes.map((lane, index) => ({
  key: String(lane.key || `lane-${index + 1}`),
  target: String(lane.target || ""),
  refs: Array.isArray(lane.refs) ? lane.refs.map(String) : [],
  cwd: typeof lane.cwd === "string" && lane.cwd.trim() ? lane.cwd.trim() : "",
  // "partial" marks a selector that already has an authored source and a measured
  // residual. Such a lane starts on the strongest rung instead of re-running the
  // cheap one that already failed it. Anything else is a fresh lift.
  // "fresh" lifts a boundary, "partial" elevates an authored residual to the strong rung,
  // "naming" skips the models entirely: the function is already exact and only its
  // identity is in debt, so the namer is the only stage that can do anything.
  kind: ["partial", "naming"].includes(String(lane.kind || "").trim().toLowerCase())
    ? String(lane.kind).trim().toLowerCase()
    : "fresh",
  // Closed shapes from earlier runs, supplied by the parent. The workflow has no
  // filesystem, so the parent assembles this from prior reports; passing it in is what
  // makes a later run continue instead of rediscovering experiments.
  ledger: Array.isArray(lane.ledger) ? dedupeLedger(lane.ledger) : [],
  // Per-lane ladder override; empty means the batch ladder.
  models: Array.isArray(lane.models)
    ? lane.models.map((value) => String(value || "").trim()).filter(Boolean)
    : [],
  // Optional bounded parent instruction appended to the lift and repair briefs.
  note: typeof lane.note === "string" ? lane.note.trim() : "",
}));

const bad = lanes.filter((lane) => !lane.target || lane.refs.length === 0 ||
  new Set(lane.refs).size !== lane.refs.length);
// One disjoint target per lane is a write-set guarantee, not a style rule: two lanes on
// one target edit the same splat.yaml/symbols.txt/target.toml and one will clobber the
// other. Reject that here, where it costs nothing, rather than after both have written.
const targetCounts = lanes.reduce((counts, lane) => {
  const key = String(lane.target || "").trim().toLowerCase();
  counts.set(key, (counts.get(key) || 0) + 1);
  return counts;
}, new Map());
const sharedTargets = [...targetCounts.entries()].filter(([, count]) => count > 1);
if (sharedTargets.length > 0) {
  return {
    ok: false,
    error: "bof3-lift-lanes: one lane per target; group these selectors into a single lane: " +
      sharedTargets.map(([target, count]) => `${target} (${count} lanes)`).join(", "),
  };
}
if (bad.length > 0) {
  return { ok: false, error: "bof3-lift-lanes: every lane needs a target and at least one ref: " + bad.map((l) => l.key).join(", ") };
}

const resultContract = (lane) => [
  `Return one JSON object (plain or in one json fence): {lane:"${lane.key}",target:"${lane.target}",`,
  `results:[{selector:"<supplied ref>",outcome:"exact|partial|data|no-op|blocked",`,
  `gate:{verdict:"PASS|FAIL"},source_path:"...",source_sha256:"<64 lowercase hex>"}],harness_calls:{used:0}.`,
  `Include exactly one results row per supplied ref, including unchanged exact rows on repair.`,
  `Every authored partial candidate needs source_path and source_sha256 before repair can route.`,
  `The lane result must be plain JSON or one whole-result json fence. Narration before the`,
  `result fence is tolerated by routing, but keep the result fence last and complete.`,
  `Keep the runtime's required acceptance-report in a SEPARATE acceptance-report fence after it;`,
  `never embed that reserved wrapper or its fields in the lane JSON. The runtime validates and`,
  `removes the acceptance fence before routing. Save only lane JSON to your authorized lane report.`,
  `No runtime-bound output file is configured; the workflow retains the response and artifact references.`,
  `Gate PASS must describe the final live gate, not an earlier attempt. Add evidence, naming,`,
  `ordered attempts and lever consumption as other fields, never as selector rows.`,
  `Also return "ledger":[{attempt:N,shape:"<one line>",measured:"<insns/bytes or neutral>",`,
  `disposition:"retained|reverted|superseded"}]. It is the compact history that travels to the`,
  `next rung, to review and to repair, so keep it short and factual - one row per shape you`,
  `actually measured, including the ones you reverted.`,
].join("\n");

// Second and later ladder rungs. Same lane, same selectors, same write set: only the
// model changes, and it starts from the previous rung's measured evidence.
const liftRetryBrief = (lane, index, model, priorText) => [
  `BOF3 lift+naming lane '${lane.key}' — target '${lane.target}' — ladder rung ${index + 1} on '${model}'.`,
  ``,
  `An earlier model lifted these same selectors and ended non-exact. Continue from its`,
  `measured evidence; do not re-derive the function and do not repeat a variant the`,
  `earlier rungs already measured.`,
  ``,
  `Ledger rule. The previous rung's ledger is below: it is knowledge, not a spent budget.`,
  `Every shape listed there is closed; this rung has its own attempt allowance for shapes`,
  `that are not. Read it before your first experiment and name the untried shape you are`,
  `exercising.`,
  ...carriedLedger(lane),
  ``,
  `Registry refs (identical set, nothing added or substituted):`,
  ...lane.refs.map((ref) => `  ${ref}`),
  ``,
  `Write set, levers, gate discipline and the result contract are exactly as a first rung:`,
  `new/edited sources under src/bof3/ for '${lane.target}'; that target's own header if one`,
  `exists; config/targets/'${lane.target}'/{splat.yaml,symbols.txt,target.toml}; your report`,
  `out/lift-batches/${lane.key}-${lane.target.replace(/^emi\//, "").replace(/\//g, "-")}-${String(lane.refs[0] || "").split("@")[1] || "unaddressed"}.md.`,
  ``,
  `Read the earlier rung's report and ledger, reproduce its live diff for the retained`,
  `candidate, and attack the first named residual with one change at a time. Keep the`,
  `owning compiler and flags fixed. A 100% gate PASS ends the lane without review.`,
  ``,
  `Rung authority. You may run clean-C probes that the ledger does not close without`,
  `asking the supervisor, and /tmp is yours for sweep variants and diagnostic dumps.`,
  `Ask only when a probe would need an owning flag or compiler change, a foreign binding`,
  `or source-ownership claim, or a layout/boundary edit. Do not stop to confirm a probe`,
  `that the evidence supports and the ledger leaves open.`,
  `Everything else goes to the reviewer once this ladder is exhausted.`,
  ``,
  `Earlier rung result (its final JSON):`,
  String(priorText || ""),
  resultContract(lane),
  lane.note ? "Parent note for this lane:" : "",
  lane.note || "",
].filter((line, position, all) => !(line === "" && all[position - 1] === "")).join("\n");

// Two shapes that measured the same thing are one closed class, not two experiments.
// Carrying four rows of "42/46; 184->176" spends the closed-set budget on nothing, so
// collapse rows whose measurement is identical and keep the first spelling of each.
const ledgerKey = (row) => {
  // Normalise the measurement to the numbers that identify the emitted class, so
  // "42/46 instructions; 184->176", "42/46; 184->176" and "42/46; 184->176 bytes"
  // are one key. Unit words are noise; hex offsets are not, so they survive.
  const measured = String((row && row.measured) || "")
    .toLowerCase()
    .replace(/\b(?:instructions?|insns?|bytes?|b)\b/g, " ")
    .replace(/\s*;\s*/g, ";")
    .replace(/\s*(->|≥|<=|>=)\s*/g, "$1")
    .replace(/\s+/g, " ")
    .trim();
  if (measured) return `m:${measured}`;
  return `s:${String((row && row.shape) || "").toLowerCase().replace(/\s+/g, " ").trim()
    .replace(/[^a-z0-9 ]/g, "")
    .split(" ").slice(0, 6).join(" ")}`;
};

const dedupeLedger = (rows) => {
  const seen = new Map();
  for (const row of rows) {
    const shape = String((row && row.shape) || "").trim();
    if (!shape) continue;
    const entry = {
      shape,
      measured: String((row && row.measured) || "").trim(),
      disposition: String((row && row.disposition) || "").trim(),
      collapsed: 1,
    };
    const key = ledgerKey(entry);
    const prior = seen.get(key);
    if (!prior) {
      seen.set(key, entry);
      continue;
    }
    prior.collapsed += 1;
    // Prefer the disposition that closes the class most strongly.
    const rank = { retained: 3, superseded: 2, reverted: 1, "": 0 };
    if ((rank[entry.disposition] || 0) > (rank[prior.disposition] || 0)) {
      prior.shape = entry.shape;
      prior.disposition = entry.disposition;
    }
  }
  return [...seen.values()];
};

// Render the carried ledger as closed history the lane must not repeat.
const carriedLedger = (lane) => {
  if (!lane.ledger.length) {
    return [
      `Carried ledger: none supplied. Recover one first - read the target's earlier reports`,
      `under out/lift-batches/ for this selector and treat every shape they measured as`,
      `closed before you author anything.`,
    ];
  }
  const rows = lane.ledger.slice(0, 40).map((row, index) => {
    const verdict = row.disposition ? ` [${row.disposition}]` : "";
    const measured = row.measured ? ` - ${row.measured}` : "";
    const extra = row.collapsed > 1 ? ` (+${row.collapsed - 1} same measurement)` : "";
    return `  ${index + 1}. ${row.shape}${measured}${verdict}${extra}`;
  });
  const attempts = lane.ledger.reduce((sum, row) => sum + (row.collapsed || 1), 0);
  return [
    `Carried ledger: ${lane.ledger.length} closed class(es) from ${attempts} earlier attempt(s).`,
    ...rows,
    `Every shape above is closed: do not author it again, in any spelling, and do not`,
    `re-measure it to confirm. Pick up at the residual those runs left open, say which`,
    `closed shape you are deliberately not repeating if one looks tempting, and append`,
    `your own measurements to this history when you report.`,
  ];
};

// A partial lane's first child. Same selectors and write set, but the target already
// holds authored C with a measured residual, so the strong rung starts from that report
// rather than from a blank first source.
const partialBrief = (lane, model) => [
  `BOF3 partial lift+naming lane '${lane.key}' — target '${lane.target}' — starting on '${model}'.`,
  ``,
  `Each selector below already has an authored source that was measured non-exact, so`,
  `this lane starts on the strongest rung. Do not author a fresh first seed: read the`,
  `retained source, its metadata and the earlier report, reproduce the live diff, and`,
  `attack the first named residual with one change at a time.`,
  ``,
  `Ledger rule. The carried ledger is knowledge, not a spent budget: every shape it lists`,
  `is closed, and this rung has its own attempt allowance on top of it. Read the ledger`,
  `before your first experiment, name the shape you are trying, and never repeat one that`,
  `is already listed as measured. A rung that ends without a new measurement must say why`,
  `no untried shape followed from the evidence.`,
  ...carriedLedger(lane),
  ``,
  `Registry refs (identical set, nothing added or substituted):`,
  ...lane.refs.map((ref) => `  ${ref}`),
  ``,
  `Write set: the existing function sources under src/bof3/ for '${lane.target}'; that`,
  `target's own header if one exists; config/targets/'${lane.target}'/{splat.yaml,symbols.txt,target.toml};`,
  `your report out/lift-batches/${lane.key}-${lane.target.replace(/^emi\//, "").replace(/\//g, "-")}-${String(lane.refs[0] || "").split("@")[1] || "unaddressed"}.md.`,
  `Do NOT touch another target, config/splat.yaml, the index, caches,`,
  `config/symbol-naming-baseline.json, or Git.`,
  ``,
  `Start with this exact command, once, quotes included:`,
  `  bin/harness agent context reverse '${lane.refs[0]}'`,
  `The argument is the full BIN ref above, never a bare target id: the CLI rejects that`,
  `and emits no skill output.`,
  `Pre-authorized: if that call is rejected for a malformed ref (a bare target id is the`,
  `common slip), run it ONCE more with the ref above spelled in full. Two calls maximum;`,
  `the corrected call is the single required prefill and never needs a supervisor round-trip.`,
  `Then read the current source and its @status/@match/@residual block, and take the live`,
  `diff with 'bin/harness lift gate --target ${lane.target} <REF> [<REF> ...]'.`,
  `Keep the owning compiler and flags fixed. The ladder is exhausted after this rung, so`,
  ``,
  `Rung authority. You may run clean-C probes that the ledger does not close without`,
  `asking the supervisor, and /tmp is yours for sweep variants and diagnostic dumps.`,
  `Ask only when a probe would need an owning flag or compiler change, a foreign binding`,
  `or source-ownership claim, or a layout/boundary edit. Do not stop to confirm a probe`,
  `that the evidence supports and the ledger leaves open.`,
  `an exact PASS ends the lane; anything else goes to review with the full ledger.`,
  `Levers, sweep use and the result contract are exactly as a fresh lane:`,
  resultContract(lane),
  lane.note ? "Parent note for this lane:" : "",
  lane.note || "",
].filter((line, position, all) => !(line === "" && all[position - 1] === "")).join("\n");

// Naming transaction for one exact lift. The namer owns identity edits; the native gate
// stays the acceptance gate, so any applied rename must re-gate clean or be restored.
// Naming-debt lane. No model rung is spent: the function already matches, so the only
// open question is its identity.
const namingDebtBrief = (lane, model) => [
  `BOF3 naming-debt lane '${lane.key}' — target '${lane.target}' on '${model}'.`,
  ``,
  `These selectors are already exact and appear in the target's naming debt. No lift is`,
  `wanted: do not re-derive the function, do not change its body, and do not run the lift`,
  `ladder. Resolve the identity only.`,
  ``,
  `Registry refs:`,
  ...lane.refs.map((ref) => `  ${ref}`),
  ``,
  `Write set: the lifted function sources under src/bof3/ for '${lane.target}'; that`,
  `target's own header if one exists; config/targets/'${lane.target}'/{splat.yaml,symbols.txt,target.toml};`,
  `your report out/lift-batches/${lane.key}-${lane.target.replace(/^emi\//, "").replace(/\//g, "-")}-${String(lane.refs[0] || "").split("@")[1] || "unaddressed"}.md.`,
  `Do NOT touch another target, config/splat.yaml, the index, caches,`,
  `config/symbol-naming-baseline.json, or Git.`,
  ``,
  `Steps:`,
  `1. 'bin/harness naming list ${lane.target} --json' once, and read only the rows covering`,
  `   your selectors. A selector with no debt row is a no-op: report it and move on.`,
  `2. Classify each row before any evidence work, because the two kinds verify differently.`,
  `   - Non-exact: a naming lane renames an identity, so it requires a measured exact body.`,
  `     If the selector's live measurement is anything other than exact, do not rename it and`,
  `     do not edit its source: a rename of a partial cannot be verified, and the lane gate`,
  `     then fails on a body nobody touched. Report naming:"blocked" with the live`,
  `     percentage and the original/current byte counts so the parent can route it to a lift.`,
  `   - Authored here: the selector has a lift source in this target ('naming check' or a`,
  `     gate names it). Then rename the whole owned surface - identifier, filename, Splat`,
  `     label, symbols.txt row, target-private header declaration, every target-owned caller`,
  `     spelling - and verify in this target.`,
  `   - Reference row: the selector is a symbol row for a function whose body is authored in`,
  `     another target (typically the exe), and it is named in a shared prototype or macro`,
  `     that every sibling target sees. Do NOT rename it here, not even this target's own`,
  `     references: the shared header, the owning source, each sibling map row and every`,
  `     caller spelling must change together or the tree ends up with two names for one`,
  `     function, which is worse than the debt. Report naming:"deferred" with the fan-out`,
  `     inventory: owning target and its gate result, the shared prototype and macro that`,
  `     spell the old name, and every other target whose map or source still carries it.`,
  `     The parent runs that as one transaction.`,
  `3. Prove the identity from original bytes, callers/consumers and exact siblings. An`,
  `   unevidenced role keeps its address-anchored name; "no-change" is a valid outcome -`,
  `   never invent a name to fill the stage.`,
  `4. Verify once, not per selector: ONE 'bin/harness naming check ${lane.target}' with all selectors`,
  `   --json' and, if you edited anything, ONE 'bin/harness lift gate' covering every`,
  `   selector of the lane, so the target-scoped build stages run once. A gate on a large`,
  `   target takes minutes, so a second one doubles the lane's cost for nothing. An applied`,
  `   rename MUST keep its owning gate at 100% with identical bytes; on regression restore`,
  `   the previous spelling, re-gate, and report naming:"blocked" with restored:true.`,
  ``,
  ...carriedLedger(lane),
  ``,
  `Return one JSON object (plain or in one json fence):`,
  `{lane:"${lane.key}",target:"${lane.target}",selector:"<ref>",naming:"applied|no-change|deferred|blocked",`,
  `gate:{verdict:"PASS|FAIL"},restored:true|false,name:"<final function name>",`,
  `source_path:"...",source_sha256:"<64 lowercase hex>",evidence:"<one sentence>",harness_calls:{used:0}}.`,
  lane.note ? "Parent note for this lane:" : "",
  lane.note || "",
].filter((line, position, all) => !(line === "" && all[position - 1] === "")).join("\n");

const namingBrief = (lane, liftText, liftModel) => [
  `BOF3 naming transaction for lane '${lane.key}' — target '${lane.target}'.`,
  ``,
  `The lift below passed its live gate on model '${liftModel}'. Apply one evidenced naming`,
  `transaction per lifted selector (usually one), following the bof3-naming skill and its`,
  `direct-editing, naming-style and byte-safe-cosmetics references. Nothing else in the`,
  `target may move.`,
  ``,
  `Steps:`,
  `1. 'bin/harness naming list ${lane.target} --json' — inspect the rows that cover the lifted`,
  `   selector only. Read the retained source and its metadata block.`,
  `2. Decide from evidence: current original bytes, callers/consumers, and same-target exact`,
  `   siblings. An unevidenced role keeps its address-anchored name — "no-change" is a valid`,
  `   and expected outcome; never invent a semantic name to fill the stage.`,
  `3. If a rename is justified, update the whole owned surface together: function identifier,`,
  `   filename, Splat label, symbols.txt row, target-private header declaration if one exists,`,
  `   and every target-owned caller spelling. Keep the metadata @source block intact.`,
  `4. Re-prove it: 'bin/harness naming check ${lane.target} <SELECTOR> --json' always.`,
  `   Re-run 'bin/harness lift gate --target ${lane.target} <SELECTOR>' ONLY if you edited`,
  `   an authored source or binding - an applied rename MUST keep the gate at 100% with`,
  `   identical bytes, and a regression means restoring the previous spelling, re-gating`,
  `   and reporting naming:"blocked" with restored:true. A no-change transaction edits`,
  `   nothing, so the lift's own PASS still describes the live source: report`,
  `   gate:{verdict:"PASS"} from that measurement instead of spending another build.`,
  `5. Do not touch config/symbol-naming-baseline.json, other targets, shared headers, the`,
  `   index, caches, or Git.`,
  ``,
  `Return one JSON object (plain or in one json fence):`,
  `{lane:"${lane.key}",target:"${lane.target}",selector:"<ref>",naming:"applied|no-change|blocked",`,
  `gate:{verdict:"PASS|FAIL"},restored:true|false,name:"<final function name>",`,
  `source_path:"...",source_sha256:"<64 lowercase hex>",evidence:"<one sentence>",harness_calls:{used:0}}.`,
  ``,
  `Exact lift to name:`,
  String(liftText || ""),
  lane.note ? "Parent note for this lane:" : "",
  lane.note || "",
].filter((line, position, all) => !(line === "" && all[position - 1] === "")).join("\n");

// End-of-batch knowledge harvest. Exact lanes only: a partial teaches nothing portable.
const harvestBrief = (board) => {
  const exact = board.filter((row) => row.state === "complete" && isExactVerdict(row.verdict));
  const named = exact.filter((row) => String(row.verdict).startsWith("exact-naming-"));
  return [
    `BOF3 lift-batch knowledge harvest. Every lane in this batch has finished; you are the`,
    `only writer now and no function is being lifted.`,
    ``,
    `Task: improve the shared lifting references from this batch's exact results. Use`,
    `$skill-master for the authoring shape, $unslop for plain technical English, and`,
    `$caveman for compaction so an agent reads the same knowledge in fewer tokens.`,
    `Load each by name; they are installed skills, so use the one you are given rather`,
    `than looking for a copy on disk.`,
    ``,
    `Files you may edit (and nothing else):`,
    `  .agents/skills/bof3-re/references/matching-knowledge.md`,
    `  .agents/skills/bof3-re/references/match-loop.md`,
    `  .agents/skills/bof3-re/references/reverse/mission-protocol.md`,
    `  .agents/skills/bof3-re/SKILL.md  (only to point at a reference you changed)`,
    `Never touch src/, config/, include/, workflows, gates, caches, or Git.`,
    ``,
    `A lane that started from an existing partial and closed it on a later rung is the`,
    `most useful evidence here: record which lever closed which residual class, and note`,
    `that it needed the stronger rung. Still keep it generic - residual class and lever,`,
    `never the function, address or batch numbers.`,
    ``,
    `What counts as a lesson worth keeping:`,
    `- it comes from a lane that ended 100% exact in this batch, or from a lever that was`,
    `  measured and either worked or provably does not;`,
    `- it is generic: a condition, a lever and its expected effect that another function can`,
    `  use. Not a function name, not one target's address, not this batch's numbers;`,
    `- it is not already stated. Search the references first; extend an existing rule rather`,
    `  than appending a near-duplicate paragraph.`,
    `Do NOT record anything from a partial lane, a lane error, or an unmeasured proposal.`,
    `A failed experiment is only worth writing when it names the mechanism and the condition`,
    `that made it fail, so a future lane can recognise that condition.`,
    ``,
    `Write at most three additions or rewrites. Prefer one sharp sentence over a section.`,
    `Keep every existing contract, threshold, command and literal exact. Then run:`,
    `  PYTHONPATH=tools/python .venv/bin/python -m pytest -q tools/python/tests/skills`,
    `  bin/harness source docs`,
    `and report both. Do not commit.`,
    ``,
    `Batch board:`,
    JSON.stringify({ exact: exact, naming: named }, null, 2),
    ``,
    `Return one JSON object: {batch:"knowledge-harvest",added:[{file,rule,evidence}],`,
    `rewritten:[{file,rule,why}],rejected:[{lesson,reason}],checks:{skills:"passed|failed",docs:"ok|drift"},`,
    `harness_calls:{used:0}}.`,
  ].join("\n");
};

const liftBrief = (lane) => [
  `BOF3 lift+naming lane '${lane.key}' — target '${lane.target}'.`,
  ``,
  `Registry refs to lift (each already names the exact function):`,
  ...lane.refs.map((ref) => `  ${ref}`),

  ``,
  `Write set: new/edited function sources under src/bof3/ for '${lane.target}'; that target's own header`,
  `if one exists; config/targets/'${lane.target}'/{splat.yaml,symbols.txt,target.toml}; your report`,
  `out/lift-batches/${lane.key}-${lane.target.replace(/^emi\//, "").replace(/\//g, "-")}-${String(lane.refs[0] || "").split("@")[1] || "unaddressed"}.md. Do NOT touch another target, config/splat.yaml, the index, caches,`,
  `config/symbol-naming-baseline.json, or Git.`,
  ``,
  `Selection: only the supplied refs. Report data/no-op/blocked rows instead`,
  `of silently substituting candidates; the parent selects any follow-up lane.`,
  ``,
  `Per selector:`,
  `1. Run this exact command once, without head/tail pipes:`,
  `     bin/harness agent context reverse '${lane.refs[0]}'`,
  `   The argument is the full BIN ref, never the bare target id: the CLI rejects`,
  `   that without emitting any skill output. That rejection is pre-authorized for exactly`,
  `   one corrected retry with the full ref, then reuse its brief and`,
  `   declarations. If output is clipped, read the saved output remainder, not a second prefill. Read`,
  `   out/splat/${lane.target}/asm/ and original bytes at out/binaries/${lane.target}.bin`,
  `   (offset = addr - manifest load_address). Use m2c/m2ctx only for a named missing shape/declaration,`,
  `   not as a mandatory pair; a decompiler guess is not evidence. Search target-owned paths and include/`,
  `   first, then a bounded module/sibling search; never recurse from '.' into output/session trees.`,
  `2. Before editing, classify the range: data-shaped -> analysis data-candidates; fragment/shared`,
  `   epilogue or leading bin -> analysis carve (probe only; parent owns layout); proven byte-identical`,
  `   exact sibling -> lift clone dry-run (apply only within the lane write set). Record decline reasons.`,
  `   A relocation-different sibling is shape evidence, NOT clone eligibility. Otherwise verify ABI,`,
  `   constants and stores from original bytes and seed clean C directly. Do not run ineligible levers.`,
  `   Before editing an existing C candidate, obtain its live normal asm-diff and diagnose first=.`, 
  `3. Choose the evidenced semantic name/module before the first gate; keep unknown roles address-anchored.`,
  `   Write clean C89 with the metadata block (@source/@behavior/@status/@match/@residual) and the #include`,
  `   copied from a sibling lifted source. Wire the splat asm->c flip + block rename, the symbols.txt row,`,
  `   and the target.toml source claim (append to 'sources' when non-empty, else 'support_sources').`,
  `3b. A range proven to be data is closed by promoting the span, not by authoring C. Inside your write`,
  `   set: change that splat entry from 'asm' to the data kind the neighbours use ('textbin' for a`,
  `   pointer run), rename its label to the matching 'D_<ADDRESS>' form, rename the symbols.txt row to`,
  `   the same label, run 'bin/harness source splat' on the target, and confirm 'bin/harness analysis`,
  `   data-candidates' on the target no longer lists the span. Evidence must include no prologue, no`,
  `   'jr $ra', and a consumer that indexes the span as a table base. Report outcome 'data' with those`,
  `   facts and no lift-gate claim.`,
  "4. Prove it with ONE call: bin/harness lift gate --target " + lane.target + " <REF> [<REF> ...]",
  `   covering every selector you wired in this lane at once, so the target-scoped stages`,
  `   (symbols normalize/check, splat) run once for the lane instead of once per selector.`,
  `   It takes one native measurement per selector yielding both the instruction diff and`,
  `   the byte match, and retries recognized races until an unchanged failure repeats. Never call asm-diff/byte-match`,
  `   again for an unchanged candidate; the pre-edit diagnosis above remains required. Explore eligible alternatives with`,
  `   'bin/harness lift sweep <REF> SOURCE variant...'. Keep the owning compiler/flags fixed;`,
  `   flag catalogs and compiler-variant batches are not matching rungs. At a stall, use the RE skill's`,
  `   compiler-source/RTL and bounded-runtime evidence branches. Permute still needs parent authorization.`,
  `   NEVER run just check / check-all / check-unit.`,
  `5. If a boundary is proven data (not a function), say so and stop on it — do not force a C lift.`,
  ``,
  `NAMING CHECK after gate PASS: confirm the evidenced name/module and filename, splat label, map row,`,
  `header declaration (if present) and manifest claim already agree. Do not rename an already-correct`,
  `first seed merely to enter naming mode. If naming needs an edit, update the whole owned surface and`,
  `re-run the gate; any post-gate source/binding edit invalidates that gate. Unevidenced roles stay address-anchored.`,
  `A 100% gate PASS needs no review.`,
  ``,
  `Use 'bin/harness lift sweep' for eligible clean-C alternatives without call or variant quotas.`,
  ``,
  `Rung authority. You may run clean-C probes that the ledger does not close without`,
  `asking the supervisor, and /tmp is yours for sweep variants and diagnostic dumps.`,
  `Ask only when a probe would need an owning flag or compiler change, a foreign binding`,
  `or source-ownership claim, or a layout/boundary edit. Do not stop to confirm a probe`,
  `that the evidence supports and the ledger leaves open.`,
  `Do not use per-variant asm-diff/byte-match loops or local compile/compare harnesses. The sweeper`,
  `reuses native comparison setup; each variant still rewrites/relinks source. Diagnose the first`,
  `mismatch and select shapes supported by evidence. Proven allocation/scheduling exhaustion retains`,
  `the best coherent partial with its smallest missing evidence; do not chase spelling coincidences.`,
  ``,
  `Scope: only the supplied ${lane.refs.length} selectors.`,
  `Record every bin/harness command and outcome when issued, including queries and failures.`,
  `Report the full ordered attempt/rung ledger, cumulative harness_calls:{used}, every variant`,
  `and each lever's used/ineligible/declined status with evidence. Preserve explicit history gaps`,
  `through repairs; gaps require no execution allowance. Repeated experiments require changed evidence`,
  `explaining the expected effect. Restore regressions and retain the best coherent partial for review.`,
  `Parent owns restoration. Continue until acceptance, a genuine blocker or reviewed ladder exhaustion.`,
  `Report per selector: selector, outcome (exact/partial/data/no-op/blocked),`,
  `source path, module directory, the name and its evidence, gate numbers, and any shared-header need you did`,
  `NOT make.`,
  resultContract(lane),
  lane.note ? "Parent note for this lane:" : "",
  lane.note || "",
].join("\n");

const reviewBrief = (lane, liftOutput) => [
  `A BOF3 lane '${lane.key}' (target '${lane.target}') reports a non-exact outcome. You are READ-ONLY.`,
  `Review ONLY the selector(s) that are not exact — do not re-verify an exact one.`,
  `Read the lane report at out/lift-batches/${lane.key}-${lane.target.replace(/^emi\//, "").replace(/\//g, "-")}-${String(lane.refs[0] || "").split("@")[1] || "unaddressed"}.md and the original bytes. For each failing selector,`,
  `propose a concrete C solution only when evidence supports its shape and expected instruction effect.`,
  `If no proposal is justified, return workflow:"blocked" with the evidence gap, empty experiments and`,
  `ladder_exhausted:false. Do not invent a repair identity or call missing evidence exhaustion.`,
  `If a selector is genuinely data, say so and stop. Supply any naming evidence the lane lacked.`,
  `Return JSON with top-level workflow:"accepted"|"repair"|"blocked", lane and target, plus findings.`,
  `For workflow:"repair", include repair:{lever:"<stable eligible kebab-case lever>",`,
  `proposal_sha256:"<64 lowercase hex hash of retained proposed source/shape>",`,
  `evidence_pins:[{path:"<retained evidence path>",sha256:"<64 lowercase hex>"}]}.`,
  `Evidence pins must be nonempty and paths distinct. Retain proposal/evidence in authorized report`,
  `artifacts. Routing checks identity only; parent/native gates authenticate content and eligibility.`,
  `Accepted means the review is dispositioned, not that a partial became exact. Never repair exact rows.`,
  ``,
  `Preserve the full ordered attempt ledger and outcomes. Any proposal must use an untried eligible`,
  `lever or explain changed evidence for a repeat. When no proposal is supported, use the blocked`,
  `outcome above. Record history gaps without quota stops.`,
  `Lane result (including consumption):`,
  String(liftOutput || ""),
  lane.note ? "Parent restrictions also apply to this review:" : "",
  lane.note || "",
  `Review never grants new experiment authority. A proposed lever outside the parent scope`,
  `must return workflow:"blocked" or a reviewed partial disposition, not workflow:"repair".`,
].join("\n");

const repairBrief = (lane, reviewOutput, proposedText) => [
  `Repair round for lane '${lane.key}' (target '${lane.target}'). A reviewer proposed solutions below.`,
  `Fix ONLY authored partial selectors within the original lane write set; leave exact, data, no-op and blocked rows untouched.`,
  `Record every command and outcome, including queries and failures, in cumulative history.`,
  `No call, variant or repair-round allowance is required. Preserve explicit history gaps.`,
  `Read the retained candidate and ledger, obtain its live normal diff, then apply an untried reviewed shape.`,
  `'bin/harness lift gate --target ${lane.target} <FAILING_REF> [...]' proves`,
  `all repaired selectors together. No just check, other targets or shared headers. Restore regressions,`,
  `not the best coherent partial: retain it for review with truthful metadata; parent owns restoration.`,
  ``,
  `Prior result and cumulative consumption:`,
  String(proposedText || ""),
  `Reviewer proposal:`,
  String(reviewOutput || ""),
  ``,
  `Carry forward the full ordered ledger and outcomes. Use eligible 'bin/harness lift sweep'`,
  `alternatives without call/variant quotas. Repeat a lever only with changed evidence explaining its`,
  `expected effect. Keep the owning compiler/flags fixed; use compiler-source/RTL and bounded-runtime`,
  `evidence before more source probes. No flag catalogs or compiler-variant batches. Permute requires`,
  `selector-specific parent approval. No per-variant asm-diff loop or local compile/compare harness.`,
  `Continue until acceptance, a genuine blocker or`,
  `reviewed ladder exhaustion. Retain the best coherent partial with its smallest missing evidence.`,
  resultContract(lane),
  lane.note ? "Parent note for this lane:" : "",
  lane.note || "",
].join("\n");

// Only explicit result objects govern routing; narrative/nested ledger statuses do not.
//
// Children narrate before they report: reviewers write findings and then append the
// routing JSON, and repairs write a ledger before their result rows. Routing must not
// depend on the response being JSON-only, so collect candidate objects in order from
// the whole text, every fenced block, and every balanced `{...}` span, then prefer the
// first object that declares a routing key.
const balancedObjects = (text) => {
  const spans = [];
  let depth = 0;
  let start = -1;
  let quoted = false;
  for (let index = 0; index < text.length; index += 1) {
    const char = text[index];
    if (quoted) {
      if (char === "\\") index += 1;
      else if (char === '"') quoted = false;
      continue;
    }
    if (char === '"') {
      quoted = true;
      continue;
    }
    if (char === "{") {
      if (depth === 0) start = index;
      depth += 1;
      continue;
    }
    if (char === "}" && depth > 0) {
      depth -= 1;
      if (depth === 0 && start >= 0) {
        spans.push(text.slice(start, index + 1));
        start = -1;
      }
    }
  }
  return spans;
};

const jsonObjects = (text) => {
  const value = String(text || "").trim();
  const objects = [];
  const consider = (raw) => {
    if (!raw) return;
    try {
      const parsed = JSON.parse(raw);
      if (parsed && typeof parsed === "object") objects.push(parsed);
    } catch (_) { /* not JSON; keep looking */ }
  };
  consider(value);
  for (const block of value.matchAll(/```[A-Za-z0-9_-]*[ \t]*\r?\n([\s\S]*?)\r?\n```/g)) {
    consider(block[1].trim());
  }
  for (const span of balancedObjects(value)) consider(span);
  return objects;
};

const parseReport = (text) => {
  const objects = jsonObjects(text);
  if (objects.length === 0) return null;
  return objects.find((value) => !Array.isArray(value) &&
    ("workflow" in value || Array.isArray(value.results))) || objects[0];
};

const verdictOf = (text, lane) => {
  const report = parseReport(text);
  if (!report || report.lane !== lane.key || !sameTarget(report.target, lane.target)) return "unknown";
  return ["accepted", "repair", "blocked"].includes(report.workflow) ? report.workflow : "unknown";
};

// Route by explicit content pins. Parent/native gates authenticate the retained artifacts.
const repairIdentity = (candidate, review) => {
  const repair = review?.repair;
  const isHash = (value) => typeof value === "string" && /^[0-9a-f]{64}$/.test(value);
  const isPath = (value) => typeof value === "string" && value.length > 0 &&
    value === value.trim() && !value.includes("\0");
  if (!repair || typeof repair.lever !== "string" ||
      !/^[a-z][a-z0-9]*(?:-[a-z0-9]+)*$/.test(repair.lever) ||
      !isHash(repair.proposal_sha256) || !Array.isArray(repair.evidence_pins) ||
      repair.evidence_pins.length === 0) return null;
  const candidates = candidate.results.filter((row) => row.outcome === "partial");
  if (candidates.length === 0 || candidates.some((row) => !isPath(row.source_path) || !isHash(row.source_sha256))) return null;
  const evidence = repair.evidence_pins;
  if (evidence.some((pin) => !pin || !isPath(pin.path) || !isHash(pin.sha256)) ||
      new Set(evidence.map((pin) => pin.path)).size !== evidence.length) return null;
  return JSON.stringify({
    candidates: candidates.map((row) => [row.selector, row.source_sha256]).sort(),
    lever: repair.lever, proposal_sha256: repair.proposal_sha256,
    evidence: [...new Set(evidence.map((pin) => pin.sha256))].sort(),
  });
};

// Reviewer proposes an evidenced repair; lifter applies it and reviewer rechecks.

// Per-lane chain with isolation: any rejection becomes that lane's own result.
// Bound output files add a human-readable suffix to result.output. Keep routing
// inline and retain the actual reports/references in the aggregate instead.
const reports = [];
const retainReport = (lane, key, result) => {
  reports.push({ lane: lane.key, target: lane.target, key, runId: result.runId,
    output: result.output, outputReference: result.outputReference,
    artifactPaths: result.artifactPaths });
};

// A rung is worth retrying only when the lane produced a measured partial and did not
// classify any selector as data or blocked.
const worthRetrying = (text) => {
  const report = parseReport(text);
  if (!report || !Array.isArray(report.results)) return false;
  const rows = report.results;
  if (rows.some((row) => row.outcome === "data" || row.outcome === "blocked")) return false;
  return rows.some((row) => row.outcome === "partial");
};

// One rung of the model ladder. The first rung seeds the lift; each later rung gets the
// previous rung's report plus the selector set, so it continues from measured evidence
// instead of re-deriving the function. Non-exact rungs hand off; only a ladder that ends
// non-exact reaches review.
const liftRung = (lane, index, priorText, firstPercent) => {
  const ladder = laneLadder(lane);
  const model = laneModel(lane, index);
  const liftKey = `${lane.key}-lift-${index}`;
  const task = priorText
    ? liftRetryBrief(lane, index, model, priorText)
    : index === ladder.start && ladder.start > 0
      ? partialBrief(lane, model)
      : liftBrief(lane);
  return runs
    .run(liftKey, {
      label: `Lift ${lane.key} (${lane.target}) on ${model}`,
      agent: "bof3-lifter",
      model,
      ...(lane.cwd ? { cwd: lane.cwd } : {}),
      output: false,
      task,
    })
    .then(
      (lift) => {
        retainReport(lane, liftKey, lift);
        const text = String((lift && lift.output) || "");
        const state = resultState(lane, text);
        const percent = bestPercent(text);
        const baseline = firstPercent === undefined ? percent : firstPercent;
        const attempt = { text, state, model, rung: index + 1, percent, firstPercent: baseline };
        // Only a measured partial earns another rung. A boundary proven data, or a lane
        // that blocked on evidence it did not have, will answer the same way on any
        // model; spending the second rung there buys nothing.
        if (state === "exact" || state === "invalid" ||
            !worthRetrying(text) || index + 1 >= ladder.models.length) {
          return finishAttempt(lane, attempt);
        }
        return liftRung(lane, index + 1, text, baseline);
      },
      (error) => ({ lane: lane.key, target: lane.target, state: "lane-error", error: String(error).slice(0, 300) }),
    );
};

// Naming debt is resolved by identity alone. The lane runs one namer on the lowest rung
// and reports naming-applied / naming-no-change / naming-blocked; no lift stage is spent,
// because the function already matches and the ladder has nothing to search.
const namingDebtStage = (lane) => {
  const nameKey = `${lane.key}-name`;
  const model = laneModel(lane, 0);
  return runs
    .run(nameKey, {
      label: `Name debt ${lane.key} (${lane.target}) on ${model}`,
      agent: "bof3-namer",
      model,
      ...(lane.cwd ? { cwd: lane.cwd } : {}),
      output: false,
      task: namingDebtBrief(lane, model),
    })
    .then(
      (named) => {
        retainReport(lane, nameKey, named);
        const verdict = namingVerdict(String((named && named.output) || ""));
        const state = ["applied", "no-change", "deferred"].includes(verdict) ? "complete" : "blocked";
        return { lane: lane.key, target: lane.target, state,
          verdict: `naming-${verdict}`, kind: "naming", model };
      },
      (error) => ({ lane: lane.key, target: lane.target, state: "lane-error",
        error: String(error).slice(0, 300), kind: "naming" }),
    );
};

const laneChain = (lane) => (lane.kind === "naming"
  ? namingDebtStage(lane)
  : liftRung(lane, laneLadder(lane).start, "", undefined));

// Fail closed on incomplete membership or contradictory exact evidence. This is
// report routing, not independent native verification; parent gates still own closure.
// Target ids are case-insensitive here for the same reason addresses are: a lane
// echoing "EMI/Battle/Battle/03" names the same target as "emi/battle/battle/03".
const sameTarget = (left, right) =>
  String(left || "").trim().toLowerCase() === String(right || "").trim().toLowerCase();

// --- selector identity ---
// A lane may name the same function as the registry ref it was given, or as the
// canonical `target@0xADDRESS` form, with either hex case. Routing compares the
// resolved identity, not the spelling: same address and a prefix that is either the
// ref's registry prefix or the lane's target id.
const selectorIdentity = (text) => {
  const value = String(text || "").trim();
  const at = value.lastIndexOf("@");
  if (at <= 0) return null;
  const prefix = value.slice(0, at).trim();
  const address = value.slice(at + 1).trim().toLowerCase();
  if (!/^0x[0-9a-f]{8}$/.test(address)) return null;
  return { prefix, address };
};

const laneIdentities = (lane) => lane.refs.map((ref) => selectorIdentity(ref)).filter(Boolean);

const matchesLane = (lane, text, identities) => {
  const value = selectorIdentity(text);
  if (!value) return false;
  const prefix = value.prefix.toLowerCase();
  return identities.some((identity) => identity.address === value.address &&
    (identity.prefix.toLowerCase() === prefix ||
     prefix === String(lane.target).toLowerCase()));
};
// --- end selector identity ---

// Best match percentage a lane reported, so an escalated rung that improved a partial is
// distinguishable from one that merely re-measured it.
const bestPercent = (text) => {
  const report = parseReport(text);
  if (!report || !Array.isArray(report.results)) return null;
  const values = report.results
    .map((row) => (row && (row.match_percent ?? row.gate?.match_percent)))
    .filter((value) => typeof value === "number" && Number.isFinite(value));
  return values.length ? Math.max(...values) : null;
};

const resultState = (lane, text) => {
  const report = parseReport(text);
  if (!report || report.lane !== lane.key || !sameTarget(report.target, lane.target)) return "invalid";
  const rows = report.results;
  if (!Array.isArray(rows) || rows.length !== lane.refs.length) return "invalid";
  const identities = laneIdentities(lane);
  const seen = new Set();
  for (const row of rows) {
    if (!row || !matchesLane(lane, row.selector, identities)) return "invalid";
    const identity = selectorIdentity(row.selector);
    const key = `${identity.address}|${identity.prefix.toLowerCase()}`;
    if (seen.has(key)) return "invalid";
    seen.add(key);
    if (!["exact", "partial", "data", "no-op", "blocked"].includes(row.outcome)) return "invalid";
    if (row.outcome === "exact" && (row.gate?.verdict !== "PASS" ||
        (row.match_percent !== undefined && row.match_percent !== 100))) return "invalid";
  }
  return rows.every((row) => row.outcome === "exact") ? "exact" : "non-exact";
};

// Any lane that ended at 100% is harvest-eligible, whatever path closed it: a first-seed
// exact, a partial closed by a later rung, a repair that reached exact, or an exact lift
// whose naming stage was blocked. The lift evidence is what teaches a lever; the naming
// outcome is bookkeeping.
const isExactVerdict = (verdict) => {
  const value = String(verdict || "");
  return value === "exact-all" || value === "exact-named" || value === "repaired-exact" ||
    value.startsWith("exact-naming-");
};

// A lane reaches review only after every ladder rung ended non-exact.
const finishAttempt = (lane, attempt) => {
  if (attempt.state === "invalid") {
    return Promise.resolve({ lane: lane.key, target: lane.target, state: "blocked",
      verdict: "invalid-result", model: attempt.model, rung: attempt.rung });
  }
  if (attempt.state === "exact") {
    return namingStage(lane, attempt.text, attempt.model, attempt.rung);
  }
  const improved = typeof attempt.percent === "number" &&
    typeof attempt.firstPercent === "number" && attempt.percent > attempt.firstPercent;
  return reviewOnce(lane, attempt.text, 1, new Set()).then((row) => ({
    ...row,
    rung: attempt.rung,
    model: attempt.model,
    ...(improved ? { improved: attempt.percent } : {}),
  }));
};

// Naming runs on an exact lift. The lift is already at 100%, so a naming edit that
// regresses the gate must be restored by the namer; the lane stays complete either way
// and the verdict records whether an identity was applied.
// A model sometimes answers with a JS object literal (unquoted keys) instead of JSON.
// That is a formatting slip, not a failed naming transaction, so recover the three
// fields that decide the verdict rather than downgrading an exact lift.
const looseNaming = (text) => {
  const naming = /"?naming"?\s*:\s*"?(applied|no-change|blocked)"?/.exec(text);
  if (!naming) return null;
  const gate = /gate"?\s*:\s*\{[^}]*verdict"?\s*:\s*"?(PASS|FAIL)/.exec(text);
  const restored = /"?restored"?\s*:\s*(true|false)/.exec(text);
  return {
    naming: naming[1],
    gate: gate ? { verdict: gate[1] } : undefined,
    restored: restored ? restored[1] === "true" : false,
  };
};

const namingVerdict = (text) => {
  const report = parseReport(text) || looseNaming(text);
  if (!report) return "invalid";
  const naming = String(report.naming || "").trim();
  if (!["applied", "no-change", "deferred", "blocked"].includes(naming)) return "invalid";
  if (naming === "applied") {
    const gate = report.gate && report.gate.verdict;
    const restored = report.restored === true;
    if (gate !== "PASS" && !restored) return "regressed";
  }
  return naming;
};

const namingStage = (lane, liftText, liftModel, liftRung) => {
  const nameKey = `${lane.key}-name`;
  return runs
    .run(nameKey, {
      label: `Name ${lane.key} (${lane.target}) after exact lift`,
      agent: "bof3-namer",
      // Naming is cheap: the evidence already exists and it applies one transaction, so
      // it runs on the lowest rung whichever rung produced the exact lift.
      model: laneModel(lane, 0),
      ...(lane.cwd ? { cwd: lane.cwd } : {}),
      output: false,
      task: namingBrief(lane, liftText, liftModel),
    })
    .then(
      (named) => {
        retainReport(lane, nameKey, named);
        const verdict = namingVerdict(String((named && named.output) || ""));
        return {
          lane: lane.key,
          target: lane.target,
          state: "complete",
          verdict: verdict === "applied" ? "exact-named"
            : verdict === "no-change" ? "exact-all"
            : `exact-naming-${verdict}`,
          kind: lane.kind,
          model: liftModel,
          rung: liftRung,
        };
      },
      (error) => ({ lane: lane.key, target: lane.target, state: "complete",
        verdict: "exact-naming-error", error: String(error).slice(0, 300),
        kind: lane.kind, model: liftModel, rung: liftRung }),
    );
};

const reviewOnce = (lane, proposedText, round, triedRepairs) => {
  const reviewKey = `${lane.key}-review-${round}`;
  return runs
    .run(reviewKey, {
      label: `Review ${lane.key} non-exact lift`,
      agent: "bof3-reviewer",
      model: laneLastModel(lane),
      ...(lane.cwd ? { cwd: lane.cwd } : {}),
      output: false,
      task: reviewBrief(lane, proposedText),
    })
    .then(
      (review) => {
        retainReport(lane, reviewKey, review);
        const text = String((review && review.output) || "");
        const verdict = verdictOf(text, lane);
        if (verdict !== "repair") {
          return { lane: lane.key, target: lane.target, state: verdict === "accepted" ? "complete" : "blocked", verdict: verdict };
        }
        const identity = repairIdentity(parseReport(proposedText), parseReport(text));
        if (identity === null) {
          return { lane: lane.key, target: lane.target, state: "blocked", verdict: "invalid-repair-identity" };
        }
        if (triedRepairs.has(identity)) {
          return { lane: lane.key, target: lane.target, state: "blocked", verdict: "repeated-unchanged-repair" };
        }
        triedRepairs.add(identity);
        const repairKey = `${lane.key}-repair-${round}`;
        return runs
          .run(repairKey, {
            label: `Repair ${lane.key} after review`,
            agent: "bof3-lifter",
            model: laneLastModel(lane),
            ...(lane.cwd ? { cwd: lane.cwd } : {}),
            output: false,
            task: repairBrief(lane, text, proposedText),
          })
          .then(
            (repair) => {
              retainReport(lane, repairKey, repair);
              const repairText = String((repair && repair.output) || "");
              const result = resultState(lane, repairText);
              if (result === "invalid") {
                return { lane: lane.key, target: lane.target, state: "blocked", verdict: "invalid-result" };
              }
              if (result === "exact") {
                return { lane: lane.key, target: lane.target, state: "complete",
                  verdict: "repaired-exact", kind: lane.kind, model: laneLastModel(lane) };
              }
              return reviewOnce(lane, repairText, round + 1, triedRepairs);
            },
            (error) => ({ lane: lane.key, target: lane.target, state: "lane-error", error: String(error).slice(0, 300) }),
          );
      },
      (error) => ({ lane: lane.key, target: lane.target, state: "lane-error", error: String(error).slice(0, 300) }),
    );
};

const board = await runs.all(lanes.map((lane) => laneChain(lane)));

// Harvest only from exact results, and only once every lane has closed, so the child
// writes references with no lift in flight. A harvest failure never fails the batch:
// the lifts are already measured and the references are the improvable part.
const exactRows = board.filter((row) => row.state === "complete" && isExactVerdict(row.verdict));
const harvest = exactRows.length === 0
  ? { state: "skipped", reason: "no exact lane in this batch" }
  : await runs
      .run("knowledge-harvest", {
        label: "Harvest batch lessons into bof3-re references",
        agent: "bof3-lifter",
        model: harvestModel,
        output: false,
        task: harvestBrief(board),
      })
      .then(
        (result) => ({ state: "complete", output: String((result && result.output) || "").slice(0, 6000) }),
        (error) => ({ state: "error", error: String(error).slice(0, 300) }),
      );

return {
  lanes: lanes.length,
  complete: board.filter((row) => row.state === "complete").length,
  laneErrors: board.filter((row) => row.state === "lane-error").length,
  models: modelLadder,
  board,
  harvest,
  reports,
};
