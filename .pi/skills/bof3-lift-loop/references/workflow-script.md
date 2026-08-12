# Lift-loop workflowScript

This is the deterministic lane workflow. The parent renders and verifies it, then submits its exact contents directly in a parent-managed lane worktree. It owns retries, cleanup, host gates, rollback, and final review; no model copies orchestration.

Set `SELECTORS` to exactly one selector and `RUN_KEY` to its unique lane ID via `render-workflow.py`. Launch each script independently with `cwd` set to its `lane-worktree.py` worktree, workflow `worktree:false`, and a unique absolute `sessionDir`. Nested children share that cwd. Never combine lanes in one outer `runs.all`, which aliases their session path.

```js
const SELECTORS = [
  "emi/example/00@0x80123456"
];
if (SELECTORS.length !== 1) throw new Error("inner lane workflow requires exactly one selector");
const RUN_KEY = "replace-with-unique-wave-id";
const MAX_ATTEMPTS = 10;

const targetOf = s => s.split("@")[0];
const keyOf = s => s.replace(/[^A-Za-z0-9]+/g, "-").replace(/^-|-$/g, "").toLowerCase();
const textOf = r => String(r && r.output || "");
const jsonOf = r => {
  if (r && r.structuredOutput && typeof r.structuredOutput === "object") return r.structuredOutput;
  const text = textOf(r).replace(/^\s*```json\s*/, "").replace(/```\s*$/, "");
  const start = text.indexOf("{");
  if (start < 0) return {};
  try { return JSON.parse(text.slice(start)); } catch (_) {
    let depth = 0, quoted = false, escaped = false;
    for (let i = start; i < text.length; i++) {
      const c = text[i];
      if (quoted) {
        if (escaped) escaped = false;
        else if (c === "\\") escaped = true;
        else if (c === '"') quoted = false;
      } else if (c === '"') quoted = true;
      else if (c === "{") depth++;
      else if (c === "}" && --depth === 0) {
        try { return JSON.parse(text.slice(start, i + 1)); } catch (_) { return {}; }
      }
    }
    return {};
  }
};
const exactOf = r => {
  const j = jsonOf(r);
  return j.status === "exact" || Number(j.match_percent) === 100;
};
const verdictOf = r => String(jsonOf(r).verdict || "");
const experimentsOf = r => Array.isArray(jsonOf(r).experiments) ? jsonOf(r).experiments : [];
const findingsOf = r => Array.isArray(jsonOf(r).findings) ? jsonOf(r).findings : [];
const filesOf = r => {
  const j = jsonOf(r);
  return Array.isArray(j.files_changed) ? j.files_changed :
    Array.isArray(j.changedFiles) ? j.changedFiles : [];
};
const scoreOf = r => {
  const value = Number(jsonOf(r).match_percent);
  return Number.isFinite(value) ? value : NaN;
};
const filesSeenOf = x => [...new Set(x.history.flatMap(h => filesOf(h.executor)))];
const repairableOf = r => jsonOf(r).repairable === true;
const checkpointImprovedOf = r => r && r.ok && jsonOf(r).improved === true;
const experimentKey = e => JSON.stringify([e.lever || "", e.expected_effect || ""]);
const observableExperiment = e => {
  const effect = String(e.expected_effect || "");
  return String(e.lever || "").trim() &&
    /(size|frame|cfg|branch|loop|first mismatch|offset|instruction|register|load|store|delay slot|score)/i.test(effect);
};
const unseenExperiments = x => {
  const seen = new Set(x.history.slice(0, -1).flatMap(h =>
    (h.review && Array.isArray(h.review.result.experiments) ? h.review.result.experiments : [])
      .map(experimentKey)
  ));
  return experimentsOf(x.review).filter(e =>
    observableExperiment(e) &&
    (!seen.has(experimentKey(e)) || String(e.new_evidence || "").trim())
  );
};
const shellQuote = s => "'" + String(s).replace(/'/g, "'\\''") + "'";
const checkpointGate = (x, attempt, requireImprovement, extraFiles = [], requireAtLeast = null) => {
  const files = filesOf(x.executor);
  if (!Number.isFinite(scoreOf(x.executor))) return "false";
  return [
    "python3 .pi/skills/bof3-lift-loop/scripts/attempt-checkpoint.py capture",
    "--lane", shellQuote(RUN_KEY + "-" + keyOf(x.selector)), "--selector", shellQuote(x.selector),
    "--attempt", String(attempt), "--match=" + String(scoreOf(x.executor)),
    requireImprovement ? "--require-improvement --soft-no-improvement" : "",
    requireAtLeast == null ? "" : "--require-at-least " + String(requireAtLeast),
    [...new Set([...filesSeenOf(x), ...extraFiles])].map(shellQuote).join(" ")
  ].filter(Boolean).join(" ");
};
const restoreGate = x => "python3 .pi/skills/bof3-lift-loop/scripts/attempt-checkpoint.py restore --lane " + shellQuote(RUN_KEY + "-" + keyOf(x.selector));
const handoffOf = r => ({ result: jsonOf(r), report: textOf(r) });
const recordOf = (attempt, executor, review) => ({
  attempt,
  executor: handoffOf(executor),
  review: review ? handoffOf(review) : null
});
const historyOf = x => JSON.stringify(x.history);
const reverseTask = s => [
  "Lift " + s + ".",
  "Run agent-context reverse. Own only this target/function files.",
  "Follow the bounded clean-C ladder; preserve the best coherent candidate.",
  "No git, publication, other targets, or children. Return protocol JSON and rung ledger."
].join("\n");
const reviewTask = x => [
  "Review " + x.selector + " from fresh repository evidence.",
  "Run agent-context review; inspect live diff, owned changes, semantics/types/ABI/ownership.",
  "Use the complete ordered attempt ledger below. Do not repeat a tested lever unless new evidence explains why its expected effect differs.",
  "Exact: pass or evidence-backed block. For every non-exact candidate, after auditing the prescribed ladder, explicitly ask: What other experiments could we try that are not already in the ladder or ledger?",
  "Use live mismatch/source/compiler/nearby-function/target evidence. Return needs-fix with 1-3 safe concrete untried experiments; pass+ladder_exhausted only after documenting why open discovery found none.",
  "Each experiment predicts an observable size/frame/CFG/first-mismatch or named instruction/register/memory effect. Novel playbook-external experiments are allowed; unsupported speculation is not.",
  "After no progress, ask again using the new evidence; propose only untried candidates with a different predicted effect.",
  "Every block must return repairable:true only for concrete source/metadata/binding fixes this executor may make; return repairable:false for rejected semantics/types, invalid boundary, approval/safety, or external-tool blockers.",
  "No source edits. Identify any decisive reproducible improvement as generic-lever candidate.",
  "Attempt ledger:\n" + historyOf(x),
  "Current executor handoff:\n" + textOf(x.executor)
].join("\n");
const retryTask = (x, actionable) => [
  "Continue " + x.selector + " from the stable repository state left by the prior executor.",
  verdictOf(x.review) === "block"
    ? "Repair only the concrete review blockers below, with live evidence."
    : "Use only the ranked untried review experiments below, one variant at a time.",
  "Read the complete ordered attempt ledger. Do not repeat a lever; record expected versus actual size/CFG/first-mismatch/instruction effect and accept/revert outcome.",
  "The host checkpoints every attempt and restores the best score after unchanged or regressing experiments; do not defeat that state.",
  "Preserve the best legal coherent candidate; obey ten-attempt ceiling.",
  "No git, publication, other targets, or children.",
  "Actionable filtered experiments (the only experiments you may run):\n" + JSON.stringify(actionable),
  "Attempt ledger (history only; repeated experiments here are not actionable):\n" + historyOf(x),
  "Current executor handoff:\n" + textOf(x.executor),
  "Current reviewer handoff:\n" + textOf(x.review)
].join("\n");
const cleanupTask = x => [
  "Cleanup and integrate reviewed " + (exactOf(x.executor) ? "exact" : "retained partial") + " " + x.selector + ".",
  "Perform evidence-backed semantic function naming, source filename/Splat label transaction, target-local symbol imports, declarations, weak bindings, metadata, and owned-file consistency with the rest of the project.",
  exactOf(x.executor)
    ? "Preserve exact bytes and require live asm-diff plus byte-match."
    : "Spelling-only cleanup: preserve body/ABI/address/boundary/compiler settings and @status partial/@match/@residual; require the live score not to regress below " + String(x.bestScore) + ".",
  "Do not invent semantics or broaden target ownership. No git, publication, other targets, or children."
].join("\n");
const gateOf = s => {
  const t = targetOf(s);
  return "bin/asm-diff '" + s + "' >/dev/null && bin/byte-match '" + s +
    "' && bin/symbols check '" + t + "' && bin/splat '" + t + "' && git diff --check";
};
const partialCleanupGate = x => [
  "python3 .pi/skills/bof3-lift-loop/scripts/attempt-checkpoint.py capture",
  "--lane", shellQuote(RUN_KEY + "-" + keyOf(x.selector)),
  "--selector", shellQuote(x.selector), "--attempt", String(x.attempt + 3),
  "--require-at-least", String(x.bestScore), "--no-promote", filesSeenOf(x).map(shellQuote).join(" ")
].join(" ") + " && bin/symbols check " + shellQuote(targetOf(x.selector)) +
  " && bin/splat " + shellQuote(targetOf(x.selector)) + " && git diff --check";
const cleanupPathsGate = (x, attempt) => [
  "python3 .pi/skills/bof3-lift-loop/scripts/attempt-checkpoint.py capture",
  "--lane", shellQuote(RUN_KEY + "-" + keyOf(x.selector)),
  "--selector", shellQuote(x.selector), "--attempt", String(attempt),
  "--paths-only --scan-worktree", filesOf(x.cleanup).map(shellQuote).join(" ")
].join(" ");

let lanes = (await runs.all(SELECTORS.map(s => ({
  key: "reverse-0-" + keyOf(s), agent: "bof3-reverse", task: reverseTask(s)
})))).map((run, i) => ({
  selector: SELECTORS[i], attempt: 1, executor: run, review: null,
  history: [recordOf(1, run, null)], terminal: false,
  bestScore: scoreOf(run), bestExecutor: run
}));

const initialCheckpoints = await runs.all(lanes.map(x => ({
  key: "checkpoint-1-" + keyOf(x.selector), agent: "bof3-review",
  task: "Record the host checkpoint for " + x.selector + ". No edits.",
    gate: checkpointGate(x, 1, false)
})));
lanes.forEach((x, i) => {
  x.checkpoint = initialCheckpoints[i];
  if (!x.checkpoint || !x.checkpoint.ok) x.terminal = true;
});

for (let round = 0; round < MAX_ATTEMPTS; round++) {
  const active = lanes.filter(x => !x.terminal);
  if (!active.length) break;
  const reviews = await runs.all(active.map(x => ({
    key: "review-" + x.attempt + "-" + keyOf(x.selector),
    agent: "bof3-review",
    task: reviewTask(x)
  })));
  active.forEach((x, i) => {
    x.review = reviews[i];
    x.history[x.history.length - 1].review = handoffOf(reviews[i]);
  });

  const retry = active.filter(x => {
    x.actionable = [];
    if (x.attempt >= MAX_ATTEMPTS) return false;
    if (verdictOf(x.review) === "block")
      return repairableOf(x.review) && findingsOf(x.review).length > 0;
    if (verdictOf(x.review) !== "needs-fix") return false;
    x.actionable = unseenExperiments(x);
    return x.actionable.length > 0;
  });
  active.filter(x => !retry.includes(x)).forEach(x => { x.terminal = true; });
  if (!retry.length) break;

  const reruns = await runs.all(retry.map(x => ({
    key: "reverse-" + x.attempt + "-" + keyOf(x.selector),
    agent: "bof3-reverse",
    task: retryTask(x, x.actionable)
  })));
  retry.forEach((x, i) => {
    x.executor = reruns[i];
    x.attempt++;
    x.history.push(recordOf(x.attempt, reruns[i], null));
  });

  const checks = await runs.all(retry.map(x => ({
    key: "checkpoint-" + x.attempt + "-" + keyOf(x.selector), agent: "bof3-review",
    task: "Report the configured host gate result only. improved:false is an expected checkpoint decision, not an agent failure. No edits or extra commands.",
    gate: checkpointGate(x, x.attempt, true)
  })));
  retry.forEach((x, i) => { x.checkpoint = checks[i]; });
  const rejected = retry.filter(x => !checkpointImprovedOf(x.checkpoint));
  if (rejected.length) {
    const restores = await runs.all(rejected.map(x => ({
      key: "restore-best-" + x.attempt + "-" + keyOf(x.selector), agent: "bof3-review",
      task: "Restore the mechanically checkpointed best state for " + x.selector + ". No authored edits.",
      gate: restoreGate(x)
    })));
    rejected.forEach((x, i) => {
      x.restore = restores[i];
      x.history[x.history.length - 1].checkpoint = {
        accepted: false, restored: Boolean(restores[i] && restores[i].ok)
      };
      x.executor = x.bestExecutor;
      // A successful restore re-enters the normal review loop, where open-ended
      // discovery uses the failed experiment as new evidence.
      x.terminal = !restores[i] || !restores[i].ok;
    });
  }
  retry.filter(x => checkpointImprovedOf(x.checkpoint)).forEach(x => {
    x.restore = null;
    x.bestScore = scoreOf(x.executor);
    x.bestExecutor = x.executor;
    x.history[x.history.length - 1].checkpoint = { accepted: true, restored: false };
  });
}

const retained = lanes.filter(x => x.terminal && verdictOf(x.review) === "pass" &&
  (exactOf(x.executor) || jsonOf(x.review).ladder_exhausted === true));
if (retained.length) {
  const precleanup = await runs.all(retained.map(x => ({
    key: "precleanup-gate-" + keyOf(x.selector),
    agent: "bof3-review",
    task: "Attest the reviewed retained-state gate for " + x.selector + ". No edits.",
    gate: exactOf(x.executor) ? gateOf(x.selector) : checkpointGate(x, x.attempt + 1, false, [], x.bestScore)
  })));
  retained.forEach((x, i) => { x.precleanup = precleanup[i]; });
  const gated = retained.filter(x => x.precleanup && x.precleanup.ok);

  const cleanupCheckpoints = await runs.all(gated.map(x => ({
    key: "checkpoint-precleanup-" + keyOf(x.selector), agent: "bof3-review",
    task: "Checkpoint the reviewed retained state before cleanup for " + x.selector + ". No edits.",
    gate: checkpointGate(x, x.attempt + 2, false)
  })));
  gated.forEach((x, i) => { x.cleanupCheckpoint = cleanupCheckpoints[i]; });
  const cleanupReady = gated.filter(x => x.cleanupCheckpoint && x.cleanupCheckpoint.ok);
  const cleanups = await runs.all(cleanupReady.map(x => ({
    key: "cleanup-" + keyOf(x.selector),
    agent: "bof3-cleanup",
    task: cleanupTask(x),
    gate: exactOf(x.executor) ? gateOf(x.selector) : partialCleanupGate(x)
  })));
  cleanupReady.forEach((x, i) => { x.cleanup = cleanups[i]; });
  const failedCleanup = cleanupReady.filter(x => !x.cleanup || !x.cleanup.ok);
  if (failedCleanup.length) {
    const cleanupPathRecords = await runs.all(failedCleanup.map(x => ({
      key: "record-cleanup-paths-" + keyOf(x.selector), agent: "bof3-review",
      task: "Record cleanup-touched paths before rollback for " + x.selector + ". No authored edits.",
      gate: cleanupPathsGate(x, x.attempt + 4)
    })));
    failedCleanup.forEach((x, i) => { x.cleanupPathRecord = cleanupPathRecords[i]; });
    const restorableCleanup = failedCleanup.filter(x => x.cleanupPathRecord && x.cleanupPathRecord.ok);
    const cleanupRestores = await runs.all(restorableCleanup.map(x => ({
      key: "restore-precleanup-" + keyOf(x.selector), agent: "bof3-review",
      task: "Restore the reviewed retained pre-cleanup checkpoint for " + x.selector + ". No authored edits.",
      gate: restoreGate(x)
    })));
    restorableCleanup.forEach((x, i) => { x.cleanupRestore = cleanupRestores[i]; });
  }

  const cleaned = cleanupReady.filter(x => x.cleanup && x.cleanup.ok);
  const finals = await runs.all(cleaned.map(x => ({
    key: "final-review-" + keyOf(x.selector),
    agent: "bof3-review",
    task: [
      "Final retained-state post-cleanup review " + x.selector + ".",
      "Verify semantic naming, source/Splat/map/declaration/binding integration, ownership, metadata, old-spelling absence, and cleanup gate evidence. Exact must remain byte-exact; partial must retain or improve its reviewed live score and partial metadata.",
      "No edits. Cleanup handoff:\n" + textOf(x.cleanup)
    ].join("\n")
  })));
  cleaned.forEach((x, i) => { x.finalReview = finals[i]; });
  const rejectedFinal = cleaned.filter(x => verdictOf(x.finalReview) !== "pass");
  if (rejectedFinal.length) {
    const finalPathRecords = await runs.all(rejectedFinal.map(x => ({
      key: "record-final-rejected-paths-" + keyOf(x.selector), agent: "bof3-review",
      task: "Record final-review-rejected cleanup paths before rollback for " + x.selector + ". No authored edits.",
      gate: cleanupPathsGate(x, x.attempt + 5)
    })));
    rejectedFinal.forEach((x, i) => { x.finalPathRecord = finalPathRecords[i]; });
    const finalRestorable = rejectedFinal.filter(x => x.finalPathRecord && x.finalPathRecord.ok);
    const finalRestores = await runs.all(finalRestorable.map(x => ({
      key: "restore-final-rejected-" + keyOf(x.selector), agent: "bof3-review",
      task: "Restore the reviewed retained pre-cleanup checkpoint after final-review rejection for " + x.selector + ". No authored edits.",
      gate: restoreGate(x)
    })));
    finalRestorable.forEach((x, i) => { x.finalCleanupRestore = finalRestores[i]; });
  }
}

return lanes.map(x => ({
  selector: x.selector,
  attempts: x.attempt,
  attemptLedger: x.history,
  bestScore: x.bestScore,
  checkpointPassed: Boolean(x.checkpoint && x.checkpoint.ok),
  bestStateRestored: Boolean(x.restore && x.restore.ok),
  executorRunId: x.executor && x.executor.runId,
  executor: jsonOf(x.executor),
  review: jsonOf(x.review),
  precleanupGatePassed: Boolean(x.precleanup && x.precleanup.ok),
  cleanup: jsonOf(x.cleanup),
  cleanupRollbackPassed: Boolean((x.cleanupRestore && x.cleanupRestore.ok) || (x.finalCleanupRestore && x.finalCleanupRestore.ok)),
  cleanupRollbackFailed: Boolean(
    (x.cleanup && !x.cleanup.ok && (!x.cleanupRestore || !x.cleanupRestore.ok)) ||
    (x.finalReview && verdictOf(x.finalReview) !== "pass" && (!x.finalCleanupRestore || !x.finalCleanupRestore.ok))
  ),
  finalReview: jsonOf(x.finalReview),
  integrateExact: exactOf(x.executor) && verdictOf(x.review) === "pass" && verdictOf(x.finalReview) === "pass" && Boolean(x.cleanup && x.cleanup.ok),
  retainPartial: !exactOf(x.executor) && verdictOf(x.review) === "pass" && jsonOf(x.review).ladder_exhausted === true && verdictOf(x.finalReview) === "pass" && Boolean(x.cleanup && x.cleanup.ok),
  genericLeverCandidates: [jsonOf(x.review).lesson, jsonOf(x.finalReview).lesson].filter(Boolean)
}));
```

Launch the verified fenced script directly with `cwd` equal to the sibling worktree `../.bof3-lift-worktrees/WAVE-LANE`, workflow `worktree:false`, and a unique absolute `sessionDir`. Do not wrap it in `bof3-lane` or another `runs.run`. Export afterward with `lane-worktree.py export --key WAVE-LANE --selector SELECTOR`; its manifest and binary patch are authoritative. Do not reconstruct patches from child text.

Each lane keeps a complete ordered JSON attempt ledger (executor result, review,
experiment effects, retained/reverted outcome) and passes it to every fresh executor
and reviewer. After the ladder and each failure, review asks what other untried
experiment live evidence suggests; continue until exact, none remains, or ten attempts. This preserves accumulated reasoning without reusing a mutation session.
`needs-fix` retries ranked untried experiments; `block` retries only when review explicitly returns
`repairable: true` with concrete findings. Missing/false repairability is
terminal, including rejected semantics/types, invalid boundary, approval/safety,
or external-tool blockers. Repairable lanes loop through reverse → review for at
most five retries after the first attempt.
Do not use retained-child `resume` here: a resumed mutation child may detach and
return its receipt before editing finishes, allowing the next reviewer to race
an unstable worktree. A normal `runs.run` resolves only after that attempt
finishes.

The script deliberately returns decisions instead of committing. Parent checks
unexpected/overlapping paths, records only generic reusable levers, integrates in
queue order, commits/pushes under authorization, refreshes edited snapshots, and
rebuilds the index once per wave.
