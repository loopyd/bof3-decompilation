# Lift-loop workflowScript

Render with `scripts/render-workflow.py`; run the fenced script in the lane worktree.

```js
const SELECTORS = [
  "emi/example/00@0x80123456"
];
if (SELECTORS.length !== 1) throw new Error("one selector required");
const RUN_KEY = "replace-with-unique-wave-id";
const MAX_ATTEMPTS = 20;

const selector = SELECTORS[0];
const key = selector.replace(/[^A-Za-z0-9]+/g, "-").replace(/^-|-$/g, "").toLowerCase();
const text = r => String(r && r.output || "");
const json = r => {
  if (r && r.structuredOutput && typeof r.structuredOutput === "object") return r.structuredOutput;
  const s = text(r).replace(/^\s*```json\s*/, "").replace(/```\s*$/, "");
  const start = s.indexOf("{");
  if (start < 0) return {};
  try { return JSON.parse(s.slice(start)); } catch (_) {}
  let depth = 0, quote = false, escape = false;
  for (let i = start; i < s.length; i++) {
    const c = s[i];
    if (quote) {
      if (escape) escape = false;
      else if (c === "\\") escape = true;
      else if (c === '"') quote = false;
    } else if (c === '"') quote = true;
    else if (c === "{") depth++;
    else if (c === "}" && --depth === 0) {
      try { return JSON.parse(s.slice(start, i + 1)); } catch (_) { return {}; }
    }
  }
  return {};
};
const score = r => Number(json(r).match_percent);
const exact = r => json(r).status === "exact" || score(r) === 100;
const verdict = r => String(json(r).verdict || "");
const quote = s => "'" + String(s).replace(/'/g, "'\\''") + "'";
const files = r => {
  const j = json(r);
  return Array.isArray(j.files_changed) ? j.files_changed : Array.isArray(j.changedFiles) ? j.changedFiles : [];
};
const experimentKey = e => JSON.stringify([e.lever || "", e.expected_effect || ""]);
const experiments = (review, ledger) => {
  const seen = new Set(ledger.flatMap(x => (x.experiments || []).map(experimentKey)));
  return (Array.isArray(json(review).experiments) ? json(review).experiments : []).filter(e =>
    e.lever && e.expected_effect && !seen.has(experimentKey(e))
  );
};
const lane = RUN_KEY + "-" + key;
const checkpoint = (attempt, run, requireImprovement) => [
  "python3 .pi/skills/bof3-lift-loop/scripts/attempt-checkpoint.py capture",
  "--lane", quote(lane), "--selector", quote(selector),
  "--attempt", String(attempt), "--match=" + String(score(run)),
  requireImprovement ? "--require-improvement --soft-no-improvement" : "",
  files(run).map(quote).join(" ")
].filter(Boolean).join(" ");
const restore = "python3 .pi/skills/bof3-lift-loop/scripts/attempt-checkpoint.py restore --lane " + quote(lane);

const ledger = [];
let attempt = 1;
let current = await runs.run("reverse-1", {
  agent: "bof3-reverse",
  task: [
    "Lift " + selector + " toward 100%.",
    "Run agent-context reverse and live asm-diff. Preserve semantics and the best coherent C.",
    "No git, other targets, publication, or children. Return JSON with match_percent, files_changed, and attempt ledger."
  ].join("\n")
});
let best = current;
let bestScore = score(current);
let gate = await runs.run("checkpoint-1", {
  agent: "bof3-review", task: "Report the host gate only. No edits.", gate: checkpoint(1, current, false)
});
if (!gate.ok) throw new Error("initial checkpoint failed");
ledger.push({ attempt, executor: json(current), score: bestScore, accepted: true, experiments: [] });

while (!exact(best) && attempt < MAX_ATTEMPTS) {
  const review = await runs.run("review-" + attempt, {
    agent: "bof3-review",
    task: [
      "Review " + selector + " at attempt " + attempt + "/" + MAX_ATTEMPTS + ". No edits.",
      "If exact return pass. Otherwise return needs-fix with 1-3 untried semantics-preserving experiments.",
      "Use evidence first, then think outside the box. Each experiment needs lever and concrete expected_effect.",
      "Do not stop for idea exhaustion before attempt 20. Safety/semantic/external blocker returns block.",
      "Ledger:\n" + JSON.stringify(ledger),
      "Current:\n" + text(best)
    ].join("\n")
  });
  if (verdict(review) === "block") break;
  const choices = experiments(review, ledger);
  if (verdict(review) !== "needs-fix" || !choices.length) {
    ledger.push({ attempt, review: json(review), error: "review supplied no experiment" });
    break;
  }

  attempt++;
  current = await runs.run("reverse-" + attempt, {
    agent: "bof3-reverse",
    task: [
      "Continue " + selector + " toward 100%; attempt " + attempt + "/" + MAX_ATTEMPTS + ".",
      "Try one supplied experiment, run live asm-diff, and keep it only if coherent and improved.",
      "No git, other targets, publication, or children.",
      "Experiments:\n" + JSON.stringify(choices),
      "Ledger:\n" + JSON.stringify(ledger)
    ].join("\n")
  });
  gate = await runs.run("checkpoint-" + attempt, {
    agent: "bof3-review",
    task: "Report the host gate only; improved:false is normal. No edits.",
    gate: checkpoint(attempt, current, true)
  });
  const outcome = json(gate);
  const improved = gate.ok && outcome.improved === true;
  ledger.push({
    attempt,
    executor: json(current),
    score: score(current),
    accepted: improved,
    experiments: choices
  });
  if (improved) {
    best = current;
    bestScore = score(current);
  } else {
    const restored = await runs.run("restore-" + attempt, {
      agent: "bof3-review", task: "Restore the host checkpoint only. No edits.", gate: restore
    });
    if (!restored.ok) throw new Error("checkpoint restore failed");
    current = best;
  }
}

const finalReview = await runs.run("final-review", {
  agent: "bof3-review",
  task: [
    "Final review " + selector + ". No edits.",
    "Verify live score, semantics, and whether the best state is exact.",
    "Ledger:\n" + JSON.stringify(ledger)
  ].join("\n")
});

return {
  selector,
  attempts: attempt,
  exact: exact(best),
  bestScore,
  executor: json(best),
  finalReview: json(finalReview),
  attemptLedger: ledger
};
```
