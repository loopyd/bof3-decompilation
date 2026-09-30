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
//       ],
//       maxSelectors: 2,               // per lane, when the lane also uses `lift next`
//       repairRounds: 1                // bounded lift<->review repair cycles
//     },
//     globalConcurrencyLimit: 8
//   })
//
// `refs` are registry references (`DISC_ID#INDEX@0xADDRESS`); every lookup is done
// through harness commands (`bin/harness lift m2c|gate`), which resolve the ref
// themselves. `target` is the manifest target id used by the gate's second
// argument and by the lane's write set.
//
// Lessons encoded here (all measured in batches 001-005):
//  - one target per lane, disjoint target-local write sets;
//  - a `lift gate` PASS is exact, so NO review is spent on an exact selector;
//  - a non-exact selector goes to a reviewer, then to a bounded repair round;
//  - per-lane failure isolation: a lane timeout/error becomes that lane's result
//    and never rejects the batch;
//  - bounded concurrency (the caller's globalConcurrencyLimit), never 20;
//  - lanes never run `just check`, never touch another target or shared config.
//
// Inputs are plain JSON only; no secrets.

const rawLanes = Array.isArray(args.lanes) ? args.lanes : [];
const maxSelectors = Number.isInteger(args.maxSelectors) ? args.maxSelectors : 2;
const repairRounds = Number.isInteger(args.repairRounds) ? args.repairRounds : 1;
const harnessCallLimit = 12; // cumulative writer calls across lift and every repair
const laneTimeoutMs = Number.isInteger(args.laneTimeoutMs) ? args.laneTimeoutMs : 3600000;
const reviewTimeoutMs = Number.isInteger(args.reviewTimeoutMs) ? args.reviewTimeoutMs : 2400000;
// A repair round is a bounded second attempt, not a fresh campaign: the tool budget
// and short clock stop a hard selector from eating the batch (batch-007 lane-1 spent
// 18+ min re-sweeping a known clean-C ceiling).
const repairTimeoutMs = Number.isInteger(args.repairTimeoutMs) ? args.repairTimeoutMs : 900000;

if (rawLanes.length === 0) {
  return { ok: false, error: "bof3-lift-lanes: args.lanes must list at least one { key, target, refs } lane" };
}
if (rawLanes.length > 32) {
  return { ok: false, error: "bof3-lift-lanes: at most 32 lanes per invocation" };
}

const lanes = rawLanes.map((lane, index) => ({
  key: String(lane.key || `lane-${index + 1}`),
  target: String(lane.target || ""),
  refs: Array.isArray(lane.refs) ? lane.refs.map(String) : [],
}));

const bad = lanes.filter((lane) => !lane.target || lane.refs.length === 0 ||
  lane.refs.length > maxSelectors || new Set(lane.refs).size !== lane.refs.length);
if (bad.length > 0) {
  return { ok: false, error: "bof3-lift-lanes: every lane needs a target and at least one ref: " + bad.map((l) => l.key).join(", ") };
}

const resultContract = (lane) => [
  `Return one JSON object (plain or in one json fence): {lane:"${lane.key}",target:"${lane.target}",`,
  `results:[{selector:"<supplied ref>",outcome:"exact|partial|data|no-op|blocked",`,
  `gate:{verdict:"PASS|FAIL"},source_path:"..."}],harness_calls:{limit:${harnessCallLimit},used:0,remaining:${harnessCallLimit}}.`,
  `Include exactly one results row per supplied ref, including unchanged exact rows on repair.`,
  `The lane result must be plain JSON or one whole-result json fence, without surrounding prose.`,
  `Keep the runtime's required acceptance-report in a SEPARATE acceptance-report fence after it;`,
  `never embed that reserved wrapper or its fields in the lane JSON. The runtime validates and`,
  `removes the acceptance fence before routing. Save only lane JSON to your authorized lane report.`,
  `No runtime-bound output file is configured; the workflow retains the response and artifact references.`,
  `Gate PASS must describe the final live gate, not an earlier attempt. Add evidence, naming,`,
  `ordered attempts and lever consumption as other fields, never as selector rows.`,
].join("\n");

const liftBrief = (lane) => [
  `BOF3 lift+naming lane '${lane.key}' — target '${lane.target}'.`,
  ``,
  `Registry refs to lift (each already names the exact function):`,
  ...lane.refs.map((ref) => `  ${ref}`),

  ``,
  `Write set: new/edited function sources under src/bof3/ for '${lane.target}'; that target's own header`,
  `if one exists; config/targets/'${lane.target}'/{splat.yaml,symbols.txt,target.toml}; your report`,
  `out/lift-batches/${lane.key}-${lane.target.replace(/^emi\//, "").replace(/\//g, "-")}.md. Do NOT touch another target, config/splat.yaml, the index, caches,`,
  `config/symbol-naming-baseline.json, or Git.`,
  ``,
  `Selection: only the supplied refs (max ${maxSelectors}). Report data/no-op/blocked rows instead`,
  `of silently substituting candidates; the parent selects any follow-up lane.`,
  ``,
  `Per selector:`,
  `1. Run 'bin/harness agent context reverse <REF>' once, without head/tail pipes; reuse its brief and`,
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
  "4. Prove it with ONE call: export BOF3_GATE_RETRIES=10; bin/harness lift gate --target " + lane.target + " <REF> [<REF> ...]",
  `   covering every selector you wired in this lane at once, so the target-scoped stages`,
  `   (symbols normalize/check, splat) run once for the lane instead of once per selector.`,
  `   It takes one native measurement per selector yielding both the instruction diff and`,
  `   the byte match, and retries shared-build-tree races. Never call asm-diff/byte-match`,
  `   again for an unchanged candidate; the pre-edit diagnosis above remains required. Explore alternatives with one`,
  `   'bin/harness lift sweep <REF> SOURCE variant...' call. No flag-search/permute without`,
  `   parent authorization. NEVER run just check / check-all / check-unit.`,
  `5. If a boundary is proven data (not a function), say so and stop on it — do not force a C lift.`,
  ``,
  `NAMING CHECK after gate PASS: confirm the evidenced name/module and filename, splat label, map row,`,
  `header declaration (if present) and manifest claim already agree. Do not rename an already-correct`,
  `first seed merely to enter naming mode. If naming needs an edit, update the whole owned surface and`,
  `re-run the gate; any post-gate source/binding edit invalidates that gate. Unevidenced roles stay address-anchored.`,
  `A 100% gate PASS needs no review.`,
  ``,
  `PROBE CEILING: at most ONE 'bin/harness lift sweep' with <=4 variants for the whole lane, and never a`,
  `per-variant asm-diff/byte-match loop. Do NOT write local compile/compare harnesses (no python or shell`,
  `loops that compile candidates). The sweeper reuses native comparison setup, but each variant still`,
  `rewrites/relinks source. Diagnose the first mismatch and choose only applicable clean-C shapes. If`,
  `the residual is an allocation/scheduling class, record the selector partial with its smallest missing`,
  `evidence and stop - do not chase it.`,
  ``,
  `Bounds: at most ${maxSelectors} selectors and ${harnessCallLimit} cumulative writer harness invocations`,
  `across the lift and all repairs. Count EVERY bin/harness invocation: help/examples, read-only queries,`,
  `failed calls, prefill, diagnosis and gates; multiple commands in one shell call count separately.`,
  `Record each command and cumulative count when issued; no read-only or failure exemption.`,
  `Reserve diagnosis and final validation`,
  `before spending calls on experiments; if they cannot fit, stop review-pending. Restore regressions,`,
  `but retain the best coherent non-exact candidate and owned facts for review; parent owns restoration.`,
  `Report the ordered attempt/rung ledger, harness calls {limit:${harnessCallLimit},used,remaining},`,
  `sweep/variant consumption and each lever's`,
  `used/ineligible/declined status with evidence. Bounds carry through repair; a new child is not a reset.`,
  `Report per selector: selector, outcome (exact/partial/data/no-op/blocked),`,
  `source path, module directory, the name and its evidence, gate numbers, and any shared-header need you did`,
  `NOT make.`,
  resultContract(lane),
].join("\n");

const reviewBrief = (lane, liftOutput) => [
  `A BOF3 lane '${lane.key}' (target '${lane.target}') reports a non-exact outcome. You are READ-ONLY.`,
  `Review ONLY the selector(s) that are not exact — do not re-verify an exact one.`,
  `Read the lane report at out/lift-batches/${lane.key}-${lane.target.replace(/^emi\//, "").replace(/\//g, "-")}.md and the original bytes, then propose the smallest`,
  `concrete new C solution for each failing selector: the exact shape, the expected instruction effect, and why.`,
  `If a selector is genuinely data, say so and stop. Supply any naming evidence the lane lacked.`,
  `Return JSON with top-level workflow:"accepted"|"repair"|"blocked", lane and target, plus findings.`,
  `Accepted means the review is dispositioned, not that a partial became exact. Never repair exact rows.`,
  ``,
  `Preserve the prior ordered attempt ledger and sweep consumption. Propose only untried, eligible`,
  `experiments within remaining bounds; missing ledger/budget evidence blocks repair, never resets it.`,
  `Lane result (including consumption):`,
  String(liftOutput || ""),
].join("\n");

const repairBrief = (lane, reviewOutput, proposedText) => [
  `Repair round for lane '${lane.key}' (target '${lane.target}'). A reviewer proposed solutions below.`,
  `Fix ONLY failing selectors within the original lane write set; leave exact named selectors untouched.`,
  `Original immutable writer harness-call ceiling: ${harnessCallLimit} across lift and ALL repairs,`,
  `counting EVERY bin/harness invocation, including help/examples, read-only queries and failed calls;`,
  `multiple commands in one shell call count separately. Record each command/count when issued.`,
  `Derive remaining=${harnessCallLimit}-prior cumulative used; missing`,
  `or inconsistent consumption blocks experiments. If remaining cannot cover required diagnosis and`,
  `validation, stop review-pending without edits. Report {limit:${harnessCallLimit},used,remaining} cumulatively.`,
  `Read the retained candidate and ledger, obtain its live normal diff, then apply an untried reviewed shape.`,
  `'bin/harness lift gate --target ${lane.target} <FAILING_REF> [...]' with BOF3_GATE_RETRIES=10 proves`,
  `all repaired selectors together. No just check, other targets or shared headers. Restore regressions,`,
  `not the best coherent partial: retain it for review with truthful metadata; parent owns restoration.`,
  ``,
  `Prior result and cumulative consumption:`,
  String(proposedText || ""),
  `Reviewer proposal:`,
  String(reviewOutput || ""),
  ``,
  `Carry forward the full ordered ledger and consumption in your result. Missing budget evidence blocks`,
  `experiments. At most ONE 'bin/harness lift sweep' with <=4 variants across lift AND all repairs; if`,
  `already consumed, do not sweep again. No flag-search/permute without selector-specific parent approval; no per-variant`,
  `asm-diff loop, and no local compile/compare harness. If the residual class is allocation or scheduling,`,
  `keep the best coherent candidate, record the partial with its smallest missing evidence, and stop.`,
  resultContract(lane),
].join("\n");

// Only explicit result objects govern routing; narrative/nested ledger statuses do not.
const parseReport = (text) => {
  const value = String(text || "").trim();
  try { return JSON.parse(value); } catch (_) { /* allow one explicit JSON fence */ }
  const fence = /^```json\s*\n([\s\S]*?)\n```$/.exec(value);
  if (!fence) return null;
  try { return JSON.parse(fence[1]); } catch (_) { return null; }
};

const verdictOf = (text, lane) => {
  const report = parseReport(text);
  if (!report || report.lane !== lane.key || report.target !== lane.target) return "unknown";
  return ["accepted", "repair", "blocked"].includes(report.workflow) ? report.workflow : "unknown";
};

// One repair round: reviewer proposes, lifter retries, reviewer re-checks.

// Per-lane chain with isolation: any rejection becomes that lane's own result.
// Bound output files add a human-readable suffix to result.output. Keep routing
// inline and retain the actual reports/references in the aggregate instead.
const reports = [];
const retainReport = (lane, key, result) => {
  reports.push({ lane: lane.key, target: lane.target, key, runId: result.runId,
    output: result.output, outputReference: result.outputReference,
    artifactPaths: result.artifactPaths });
};

const laneChain = (lane) => {
  const liftKey = `${lane.key}-lift`;
  return runs
    .run(liftKey, {
      label: `Lift ${lane.key} (${lane.target})`,
      agent: "bof3-lifter",
      output: false,
      task: liftBrief(lane),
      timeoutMs: laneTimeoutMs,
    })
    .then(
      (lift) => {
        retainReport(lane, liftKey, lift);
        return finishLane(lane, lift);
      },
      (error) => ({ lane: lane.key, target: lane.target, state: "lane-error", error: String(error).slice(0, 300) }),
    );
};

// Fail closed on incomplete membership or contradictory exact evidence. This is
// report routing, not independent native verification; parent gates still own closure.
const resultState = (lane, text) => {
  const report = parseReport(text);
  if (!report || report.lane !== lane.key || report.target !== lane.target) return "invalid";
  const rows = report.results;
  if (!Array.isArray(rows) || rows.length !== lane.refs.length) return "invalid";
  const seen = new Set();
  for (const row of rows) {
    if (!row || !lane.refs.includes(row.selector) || seen.has(row.selector)) return "invalid";
    seen.add(row.selector);
    if (!["exact", "partial", "data", "no-op", "blocked"].includes(row.outcome)) return "invalid";
    if (row.outcome === "exact" && (row.gate?.verdict !== "PASS" ||
        (row.match_percent !== undefined && row.match_percent !== 100))) return "invalid";
  }
  return rows.every((row) => row.outcome === "exact") ? "exact" : "non-exact";
};

const finishLane = (lane, lift) => {
  const text = String((lift && lift.output) || "");
  const result = resultState(lane, text);
  if (result === "invalid") {
    return Promise.resolve({ lane: lane.key, target: lane.target, state: "blocked", verdict: "invalid-result" });
  }
  if (result === "exact") {
    return Promise.resolve({ lane: lane.key, target: lane.target, state: "complete", verdict: "exact-all" });
  }
  if (repairRounds < 1) {
    return Promise.resolve({ lane: lane.key, target: lane.target, state: "blocked", verdict: "unreviewed" });
  }
  return reviewOnce(lane, text, 1);
};

const reviewOnce = (lane, proposedText, round) => {
  const reviewKey = `${lane.key}-review-${round}`;
  return runs
    .run(reviewKey, {
      label: `Review ${lane.key} non-exact lift`,
      agent: "bof3-reviewer",
      output: false,
      task: reviewBrief(lane, proposedText),
      timeoutMs: reviewTimeoutMs,
    })
    .then(
      (review) => {
        retainReport(lane, reviewKey, review);
        const text = String((review && review.output) || "");
        const verdict = verdictOf(text, lane);
        if (verdict !== "repair" || round > repairRounds) {
          return { lane: lane.key, target: lane.target, state: verdict === "accepted" ? "complete" : "blocked", verdict: verdict };
        }
        const repairKey = `${lane.key}-repair-${round}`;
        return runs
          .run(repairKey, {
            label: `Repair ${lane.key} after review`,
            agent: "bof3-lifter",
            output: false,
            task: repairBrief(lane, text, proposedText),
            timeoutMs: repairTimeoutMs,
            toolBudget: { soft: 24, hard: 40 },
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
                return { lane: lane.key, target: lane.target, state: "complete", verdict: "repaired-exact" };
              }
              return reviewOnce(lane, repairText, round + 1);
            },
            (error) => ({ lane: lane.key, target: lane.target, state: "lane-error", error: String(error).slice(0, 300) }),
          );
      },
      (error) => ({ lane: lane.key, target: lane.target, state: "lane-error", error: String(error).slice(0, 300) }),
    );
};

const board = await runs.all(lanes.map((lane) => laneChain(lane)));

return {
  lanes: lanes.length,
  complete: board.filter((row) => row.state === "complete").length,
  laneErrors: board.filter((row) => row.state === "lane-error").length,
  board,
  reports,
};
