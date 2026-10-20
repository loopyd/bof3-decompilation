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

// Optional explicit child model for this run (`args.model`), for example
// "ninerouter/ds-combo". Empty keeps each agent's declared model.
const laneModel =
  typeof args.model === "string" && args.model.trim() ? args.model.trim() : "";
const modelOption = laneModel ? { model: laneModel } : {};

const lanes = rawLanes.map((lane, index) => ({
  key: String(lane.key || `lane-${index + 1}`),
  target: String(lane.target || ""),
  refs: Array.isArray(lane.refs) ? lane.refs.map(String) : [],
  // Optional bounded parent instruction appended to the lift and repair briefs.
  note: typeof lane.note === "string" ? lane.note.trim() : "",
}));

const bad = lanes.filter((lane) => !lane.target || lane.refs.length === 0 ||
  new Set(lane.refs).size !== lane.refs.length);
if (bad.length > 0) {
  return { ok: false, error: "bof3-lift-lanes: every lane needs a target and at least one ref: " + bad.map((l) => l.key).join(", ") };
}

const resultContract = (lane) => [
  `Return one JSON object (plain or in one json fence): {lane:"${lane.key}",target:"${lane.target}",`,
  `results:[{selector:"<supplied ref>",outcome:"exact|partial|data|no-op|blocked",`,
  `gate:{verdict:"PASS|FAIL"},source_path:"...",source_sha256:"<64 lowercase hex>"}],harness_calls:{used:0}.`,
  `Include exactly one results row per supplied ref, including unchanged exact rows on repair.`,
  `Every authored partial candidate needs source_path and source_sha256 before repair can route.`,
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
  `Selection: only the supplied refs. Report data/no-op/blocked rows instead`,
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
  "4. Prove it with ONE call: bin/harness lift gate --target " + lane.target + " <REF> [<REF> ...]",
  `   covering every selector you wired in this lane at once, so the target-scoped stages`,
  `   (symbols normalize/check, splat) run once for the lane instead of once per selector.`,
  `   It takes one native measurement per selector yielding both the instruction diff and`,
  `   the byte match, and retries recognized races until an unchanged failure repeats. Never call asm-diff/byte-match`,
  `   again for an unchanged candidate; the pre-edit diagnosis above remains required. Explore eligible alternatives with`,
  `   'bin/harness lift sweep <REF> SOURCE variant...'. No flag-search/permute without`,
  `   parent authorization. NEVER run just check / check-all / check-unit.`,
  `5. If a boundary is proven data (not a function), say so and stop on it — do not force a C lift.`,
  ``,
  `NAMING CHECK after gate PASS: confirm the evidenced name/module and filename, splat label, map row,`,
  `header declaration (if present) and manifest claim already agree. Do not rename an already-correct`,
  `first seed merely to enter naming mode. If naming needs an edit, update the whole owned surface and`,
  `re-run the gate; any post-gate source/binding edit invalidates that gate. Unevidenced roles stay address-anchored.`,
  `A 100% gate PASS needs no review.`,
  ``,
  `Use 'bin/harness lift sweep' for eligible clean-C alternatives without call or variant quotas.`,
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
  `Read the lane report at out/lift-batches/${lane.key}-${lane.target.replace(/^emi\//, "").replace(/\//g, "-")}.md and the original bytes, then propose the smallest`,
  `concrete new C solution for each failing selector: the exact shape, the expected instruction effect, and why.`,
  `If a selector is genuinely data, say so and stop. Supply any naming evidence the lane lacked.`,
  `Return JSON with top-level workflow:"accepted"|"repair"|"blocked", lane and target, plus findings.`,
  `For workflow:"repair", include repair:{lever:"<stable eligible kebab-case lever>",`,
  `proposal_sha256:"<64 lowercase hex hash of retained proposed source/shape>",`,
  `evidence_pins:[{path:"<retained evidence path>",sha256:"<64 lowercase hex>"}]}.`,
  `Evidence pins must be nonempty and paths distinct. Retain proposal/evidence in authorized report`,
  `artifacts. Routing checks identity only; parent/native gates authenticate content and eligibility.`,
  `Accepted means the review is dispositioned, not that a partial became exact. Never repair exact rows.`,
  ``,
  `Preserve the full ordered attempt ledger and outcomes. Propose untried, eligible`,
  `experiments, or explain changed evidence for a repeated lever. Record history gaps without quota stops.`,
  `Lane result (including consumption):`,
  String(liftOutput || ""),
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
  `expected effect. No flag-search/permute without selector-specific parent approval; no per-variant`,
  `asm-diff loop or local compile/compare harness. Continue until acceptance, a genuine blocker or`,
  `reviewed ladder exhaustion. Retain the best coherent partial with its smallest missing evidence.`,
  resultContract(lane),
  lane.note ? "Parent note for this lane:" : "",
  lane.note || "",
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

const laneChain = (lane) => {
  const liftKey = `${lane.key}-lift`;
  return runs
    .run(liftKey, {
      label: `Lift ${lane.key} (${lane.target})`,
      agent: "bof3-lifter",
      ...modelOption,
      output: false,
      task: liftBrief(lane),
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
  return reviewOnce(lane, text, 1, new Set());
};

const reviewOnce = (lane, proposedText, round, triedRepairs) => {
  const reviewKey = `${lane.key}-review-${round}`;
  return runs
    .run(reviewKey, {
      label: `Review ${lane.key} non-exact lift`,
      agent: "bof3-reviewer",
      ...modelOption,
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
            ...modelOption,
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
                return { lane: lane.key, target: lane.target, state: "complete", verdict: "repaired-exact" };
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

return {
  lanes: lanes.length,
  complete: board.filter((row) => row.state === "complete").length,
  laneErrors: board.filter((row) => row.state === "lane-error").length,
  board,
  reports,
};
