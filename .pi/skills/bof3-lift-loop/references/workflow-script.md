# Lift-loop workflowScript

Render with `scripts/render-workflow.py`; launch it with a durable mission in the lane worktree.

```js
const SELECTORS = [
  "emi/example/00@0x80123456"
];
if (SELECTORS.length !== 1) throw new Error("one selector required");
const RUN_KEY = "replace-with-unique-wave-id";
const MAX_ATTEMPTS = 20;

const selector = SELECTORS[0];
const laneKey = RUN_KEY + "-" + selector.replace(/[^A-Za-z0-9]+/g, "-").replace(/^-|-$/g, "").toLowerCase();
const text = r => String(r && r.output || "");
const json = r => {
  if (r && r.structuredOutput && typeof r.structuredOutput === "object") return r.structuredOutput;
  const input = text(r).replace(/^\s*```json\s*/, "").replace(/```\s*$/, "");
  const start = input.indexOf("{");
  if (start < 0) return {};
  try { return JSON.parse(input.slice(start)); } catch (_) {}
  let depth = 0, quoted = false, escaped = false;
  for (let i = start; i < input.length; i++) {
    const c = input[i];
    if (quoted) {
      if (escaped) escaped = false;
      else if (c === "\\") escaped = true;
      else if (c === '"') quoted = false;
    } else if (c === '"') quoted = true;
    else if (c === "{") depth++;
    else if (c === "}" && --depth === 0) {
      try { return JSON.parse(input.slice(start, i + 1)); } catch (_) { return {}; }
    }
  }
  return {};
};
const score = r => Number(json(r).match_percent);
const exact = r => json(r).status === "exact" || score(r) === 100;
const quote = s => "'" + String(s).replace(/'/g, "'\\''") + "'";
const files = r => {
  const value = json(r);
  return Array.isArray(value.files_changed) ? value.files_changed : Array.isArray(value.changedFiles) ? value.changedFiles : [];
};
const experimentKey = value => JSON.stringify([value.lever || "", value.expected_effect || ""]);
const choices = (review, seen) => (Array.isArray(json(review).experiments) ? json(review).experiments : [])
  .filter(value => value.lever && value.expected_effect && !seen.includes(experimentKey(value)));
const checkpoint = (attempt, run) => [
  "python3 .pi/skills/bof3-lift-loop/scripts/attempt-checkpoint.py capture",
  "--lane", quote(laneKey), "--selector", quote(selector),
  "--attempt", String(attempt), "--match=" + String(score(run)),
  files(run).map(quote).join(" ")
].filter(Boolean).join(" ");
const measureTask = "Measure " + selector + " with live asm-diff. Do not edit. Return JSON with status, match_percent, and files_changed containing every file this lift may edit.";
const restore = "python3 .pi/skills/bof3-lift-loop/scripts/attempt-checkpoint.py restore --lane " + quote(laneKey);
const gateEvidence = run => {
  const rows = run && Array.isArray(run.results) ? run.results : [];
  const verify = rows.flatMap(row => row.acceptance && Array.isArray(row.acceptance.verifyRuns) ? row.acceptance.verifyRuns : []);
  const output = verify.length && verify[verify.length - 1].stdout;
  if (!output) throw new Error("host gate omitted checkpoint evidence");
  return JSON.parse(output);
};
const save = async lane => {
  await state.set("lane", lane);
  await state.set("nextReadyAction", lane.status === "running"
    ? "Continue " + selector + " at attempt " + (lane.attempt + 1) + "/" + MAX_ATTEMPTS
    : "Inspect final lane status: " + lane.status);
};

let lane = await state.get("lane") || {
  selector, attempt: 0, bestScore: null, status: "baseline", phase: "ready", next: null, seen: [], ledger: []
};
if (lane.selector !== selector) throw new Error("mission state selector mismatch");
if (lane.phase !== "ready") {
  const recovered = await runs.run("restore-interrupted", {
    agent: "bof3-review", task: "Report the host restore only. No edits.", gate: restore
  });
  if (!recovered.ok) throw new Error("interrupted lane restore failure: " + lane.phase);
  lane.phase = "ready";
  lane.status = "running";
  lane.bestScore = lane.ledger.length ? lane.ledger[0].score : lane.bestScore;
  lane.next = null;
  lane.ledger.push({ attempt: lane.attempt, score: lane.bestScore, improved: false, lever: "interruption recovery", predicted: "", actual: "restored baseline checkpoint" });
  await save(lane);
}

if (lane.status === "baseline") {
  const baseline = await runs.run("baseline", { agent: "bof3-reverse", task: measureTask });
  if (!Number.isFinite(score(baseline))) throw new Error("baseline omitted match_percent");
  const captured = await runs.run("checkpoint-baseline", {
    agent: "bof3-review", task: "Report the host gate only. No edits.", gate: checkpoint(1, baseline)
  });
  if (!captured.ok) throw new Error("baseline checkpoint failed");
  const baselineEvidence = gateEvidence(captured);
  if (!baselineEvidence.accepted) throw new Error("baseline checkpoint rejected");
  lane.bestScore = baselineEvidence.current.metric.match_percent;
  lane.status = baselineEvidence.current.metric.exact === true ? "exact" : "running";
  lane.ledger.push({ attempt: 0, score: lane.bestScore, accepted: true, lever: "baseline", predicted: "", actual: "" });
  await save(lane);
}

while (lane.status === "running" && lane.attempt < MAX_ATTEMPTS && lane.bestScore < 100) {
  const attempt = lane.attempt + 1;
  lane.phase = "reverse-" + attempt;
  await save(lane);

  const reverse = await runs.run("reverse-" + attempt, {
    agent: "bof3-reverse",
    task: [
      "Move " + selector + " toward a verified 100% byte match; attempt " + attempt + "/" + MAX_ATTEMPTS + ".",
      lane.next ? "Start with this experiment: " + JSON.stringify(lane.next) : "Diagnose the best clean-C experiment.",
      "This is a substantive investigation pass, not one edit: inspect live diff and source/compiler evidence, then try up to three related safe variants before returning. Re-run live asm-diff after each variant; retain the best coherent state even when it is not yet better than the mission baseline.",
      "Use the matching ladder and targeted static evidence when relevant. Do not stop merely because the first variant fails or revert to the mission baseline; later attempts build on this worktree state.",
      "Preserve semantics. No git, publication, other targets, or children.",
      "Return JSON with status, match_percent, files_changed, lever, predicted_effect, actual_effect, residual, and a compact variants_tried ledger."
    ].join("\n")
  });
  if (!Number.isFinite(score(reverse))) throw new Error("reverse omitted match_percent");

  const liveScore = score(reverse);
  const improved = liveScore > lane.bestScore;
  if (improved) lane.bestScore = liveScore;
  lane.attempt = attempt;
  lane.phase = "ready";
  lane.ledger.push({
    attempt,
    score: liveScore,
    improved,
    lever: json(reverse).lever || (lane.next && lane.next.lever) || "initial",
    predicted: json(reverse).predicted_effect || (lane.next && lane.next.expected_effect) || "",
    actual: json(reverse).actual_effect || json(reverse).residual || "",
    variants: json(reverse).variants_tried || []
  });
  lane.next = null;

  if (exact(reverse)) {
    lane.bestScore = 100;
    lane.status = "exact";
    await save(lane);
    break;
  }
  if (attempt >= MAX_ATTEMPTS) {
    lane.status = "attempt-limit";
    await save(lane);
    break;
  }

  let review = await runs.run("review-" + attempt, {
    agent: "bof3-review",
    task: [
      "Review " + selector + " after attempt " + attempt + "/" + MAX_ATTEMPTS + ". No edits.",
      "Perform a substantive evidence pass: load role context, inspect the live asm-diff and relevant source/compiler output, and check whether the executor's variants actually tested their predicted effects.",
      "If safe to continue, return needs-fix with 1-3 untried semantics-preserving experiments; use evidence first, then think outside the box.",
      "Each experiment requires a lever and concrete expected_effect. Do not recycle a superficial spelling variant. Safety/semantic/external blocker returns block.",
      "Latest result: " + JSON.stringify(lane.ledger[lane.ledger.length - 1]),
      "Tried experiment keys: " + JSON.stringify(lane.seen)
    ].join("\n")
  });
  if (String(json(review).verdict || "") === "block") {
    lane.status = "blocked";
    lane.blocker = json(review).findings || [];
    await save(lane);
    break;
  }

  let available = choices(review, lane.seen);
  if (!available.length) {
    review = await runs.run("review-fallback-" + attempt, {
      agent: "bof3-review",
      task: [
        "Propose one new semantics-preserving C-shape experiment for " + selector + ". No edits.",
        "Evidence-backed is preferred but not required. Return needs-fix with lever and concrete expected_effect.",
        "Do not repeat: " + JSON.stringify(lane.seen)
      ].join("\n")
    });
    available = choices(review, lane.seen);
  }
  if (String(json(review).verdict || "") === "block") {
    lane.status = "blocked";
    lane.blocker = json(review).findings || [];
    await save(lane);
    break;
  }
  if (!available.length) {
    lane.next = { lever: "independent clean-C shape exploration", expected_effect: "change instruction/register scheduling while preserving semantics" };
  } else lane.next = available[0];
  lane.seen.push(experimentKey(lane.next));
  await save(lane);
}

lane.phase = "final-measure";
await save(lane);
const finalMeasure = await runs.run("final-measure", { agent: "bof3-reverse", task: measureTask });
if (!Number.isFinite(score(finalMeasure))) throw new Error("final measurement omitted match_percent");
lane.finalScore = score(finalMeasure);
const finalReview = await runs.run("final-review", {
  agent: "bof3-review",
  task: "Final review " + selector + ". Verify live score and semantics. No edits. Mission lane state: " + JSON.stringify(lane)
});
lane.finalReview = json(finalReview);
const rejected = String(lane.finalReview.verdict || "") === "block";
if (rejected || (lane.finalScore <= lane.ledger[0].score && lane.finalScore < 100)) {
  lane.phase = "final-restore";
  await save(lane);
  const restored = await runs.run("restore-final", {
    agent: "bof3-review", task: "Report the host restore only. No edits.", gate: restore
  });
  if (!restored.ok) throw new Error("final checkpoint restore failure");
  lane.status = rejected ? "restored-review-block" : "restored-no-improvement";
  lane.bestScore = lane.ledger[0].score;
} else if (lane.finalScore === 100) {
  lane.status = "exact";
  lane.bestScore = 100;
} else {
  lane.status = "improved-partial";
  lane.bestScore = lane.finalScore;
}
lane.phase = "ready";
await save(lane);
return lane;
```
