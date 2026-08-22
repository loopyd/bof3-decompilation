"""Executable lift-loop workflowScript template (renderer-owned).

The lane workflowScript moves here from Markdown: agents get intent and
safety from the lift-loop skill; this constant is the single source of
the rendered script.  Render/verify via ``render-workflow.py``.
"""

TEMPLATE = r"""const SELECTORS = [
  "emi/example/00@0x80123456"
];
if (SELECTORS.length !== 1) throw new Error("one selector required");
const RUN_KEY = "replace-with-unique-wave-id";
const MAX_ATTEMPTS = 20;
const STALL_LIMIT = 3;
const LADDER = ["clean-c", "static-allocation", "compiler-profile", "permuter", "compiler-ceiling"];

const selector = SELECTORS[0];
const target = selector.slice(0, selector.lastIndexOf("@"));
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
const preparedRows = r => {
  const value = json(r);
  if (!Array.isArray(value.prepared_rows)) throw new Error("final output omitted prepared_rows");
  const rows = value.prepared_rows.map(String);
  const pattern = /^(?:(?:exe|emi)\/[A-Za-z0-9_.-]+(?:\/[A-Za-z0-9_.-]+)*@)?(?:function|data):[A-Za-z_][A-Za-z0-9_]*$/;
  if (rows.some(row => !pattern.test(row))) throw new Error("invalid prepared cleanup row IDs");
  if (new Set(rows).size !== rows.length) throw new Error("duplicate prepared cleanup row IDs");
  if (JSON.stringify(rows) !== JSON.stringify([...rows].sort())) throw new Error("prepared cleanup row IDs are not canonically ordered");
  if (rows.some(row => row.includes("@") && !row.startsWith(target + "@"))) throw new Error("prepared cleanup row target mismatch");
  return rows;
};
const experimentKey = value => JSON.stringify([value.lever || "", value.expected_effect || ""]);
const choices = (review, seen) => (Array.isArray(json(review).experiments) ? json(review).experiments : [])
  .filter(value => value.lever && value.expected_effect && !seen.includes(experimentKey(value)));
const checkpoint = (attempt, run, unique = false) => [
  "python3 .pi/skills/bof3-lift-loop/scripts/attempt-checkpoint.py capture",
  "--lane", quote(laneKey), "--selector", quote(selector),
  "--attempt", String(attempt), "--match=" + String(score(run)),
  "--target-scope", quote(target),
  unique ? "--unique --require-improvement" : "",
  files(run).map(quote).join(" ")
].filter(Boolean).join(" ");
const measureTask = "Measure " + selector + " with live asm-diff. Do not edit. Return JSON with status, match_percent, files_changed containing every file this lift may edit, and prepared_rows as an array of [TARGET@]KIND:NAME naming-audit/v3 IDs (empty when none).";
const checkpointTool = "python3 .pi/skills/bof3-lift-loop/scripts/attempt-checkpoint.py";
const restore = checkpointTool + " restore --lane " + quote(laneKey);
const restoreCheckpoint = checkpoint => restore + " --checkpoint " + quote(checkpoint);
const inspectBest = checkpointTool + " best --lane " + quote(laneKey);
const cleanupAttempt = MAX_ATTEMPTS + 1;
const manager = "$(git worktree list --porcelain | awk '/^worktree / {print substr($0,10); exit}')/.pi/skills/bof3-lift-loop/scripts/lane-worktree.py";
const durableEntry = row => JSON.stringify({ selector, lane_key: laneKey, transaction_id: String(row.attempt) + ":" + row.rung + ":" + row.lever, row });
const recordLedger = async row => {
  const recorded = await runs.run("record-ledger-" + lane.attempt + "-" + lane.ledger.length, {
    agent: "bof3-review", task: "Report the host ledger gate only. No edits.",
    gate: "python3 " + manager + " record --key " + quote(laneKey) + " --selector " + quote(selector) + " --entry-json " + quote(durableEntry(row))
  });
  if (!recorded.ok) throw new Error("durable experiment ledger write failed");
};
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
  selector, attempt: 0, bestScore: null, status: "baseline", phase: "ready", queue: [], seen: [], ledger: [], rung: 0, stalledQueues: 0
};
if (!Number.isInteger(lane.rung) || lane.rung < 0 || lane.rung >= LADDER.length) lane.rung = 0;
if (!Number.isInteger(lane.stalledQueues) || lane.stalledQueues < 0) lane.stalledQueues = 0;
if (lane.selector !== selector) throw new Error("mission state selector mismatch");
if (!lane.historyLoaded) {
  const loaded = await runs.run("load-durable-ledger", {
    agent: "bof3-review", task: "Report the host ledger gate only. No edits.",
    gate: "python3 " + manager + " ledger --selector " + quote(selector)
  });
  if (!loaded.ok) throw new Error("durable experiment ledger read failed");
  const history = gateEvidence(loaded).entries || [];
  lane.history = history;
  for (const entry of history) {
    const row = entry.row || {};
    const variants = Array.isArray(row.variants) ? row.variants : [];
    for (const variant of variants) {
      const key = experimentKey(variant);
      if (key !== '["",""]' && !lane.seen.includes(key)) lane.seen.push(key);
    }
  }
  lane.historyLoaded = true;
  await save(lane);
}
if (lane.phase !== "ready") {
  let recoveryCheckpoint;
  let recoveryActual = "restored persisted best checkpoint";
  if (lane.phase === "best-promoting") {
    const pending = lane.pendingAttempt;
    if (!pending || !pending.row || !pending.result || !Array.isArray(pending.consumedQueue)) throw new Error("pending promotion transaction is incomplete");
    const inspected = await runs.run("inspect-promoted-best", {
      agent: "bof3-review", task: "Report the verified host best checkpoint only. No edits.", gate: inspectBest
    });
    if (!inspected.ok) throw new Error("pending best checkpoint verification failed");
    const durable = gateEvidence(inspected).best;
    const durableScore = durable && durable.metric && Number(durable.metric.match_percent);
    const pendingMatches = durable && durable.selector === selector && durable.attempt === pending.checkpointAttempt && Number.isFinite(durableScore) && durableScore === pending.score;
    if (!pendingMatches && Number.isFinite(durableScore) && durableScore > lane.bestScore) throw new Error("durable best checkpoint mismatches pending promotion");
    if (pendingMatches) {
      if (!durable.checkpoint) throw new Error("verified promoted best omitted checkpoint");
      await recordLedger(pending.row);
      const nextLane = JSON.parse(JSON.stringify(lane));
      nextLane.bestCheckpoint = durable.checkpoint;
      nextLane.bestScore = durableScore;
      nextLane.attempt = pending.attempt;
      nextLane.stalledQueues = 0;
      nextLane.ledger.push(pending.row);
      nextLane.seen.push(...pending.consumedQueue.map(experimentKey).filter(key => !nextLane.seen.includes(key)));
      nextLane.queue = [];
      nextLane.phase = "ready";
      nextLane.status = durableScore === 100 ? "exact" : "running";
      delete nextLane.pendingAttempt;
      await save(nextLane);
      lane = nextLane;
      recoveryCheckpoint = lane.bestCheckpoint;
      recoveryActual = "adopted verified durable promoted best checkpoint";
    } else {
      recoveryCheckpoint = lane.bestCheckpoint;
      delete lane.pendingAttempt;
    }
  } else {
    const cleanupPending = lane.phase === "cleanup-pending";
    const cleanupPhase = lane.phase.startsWith("cleanup") || lane.phase.startsWith("consolidation");
    recoveryCheckpoint = cleanupPending ? lane.bestCheckpoint : cleanupPhase ? lane.cleanupCheckpoint : lane.bestCheckpoint;
  }
  if (!recoveryCheckpoint) throw new Error("interrupted lane omitted persisted checkpoint: " + lane.phase);
  const recovered = await runs.run("restore-interrupted", {
    agent: "bof3-review", task: "Report the host restore only. No edits.", gate: restoreCheckpoint(recoveryCheckpoint)
  });
  if (!recovered.ok) throw new Error("interrupted lane restore failure: " + lane.phase);
  if (lane.phase !== "ready") {
    lane.phase = "ready";
    lane.status = lane.bestScore === 100 ? "exact" : "running";
    lane.queue = [];
    lane.ledger.push({ attempt: lane.attempt, score: lane.bestScore, improved: false, lever: "interruption recovery", predicted: "", actual: recoveryActual, variants: [], rung: LADDER[lane.rung] });
    await save(lane);
  }
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
  lane.bestCheckpoint = baselineEvidence.current.checkpoint || baselineEvidence.checkpoint;
  if (!lane.bestCheckpoint) throw new Error("baseline checkpoint omitted exact leaf");
  lane.status = baselineEvidence.current.metric.exact === true ? "exact" : "running";
  const baselineRow = { attempt: 0, score: lane.bestScore, accepted: true, lever: "baseline", predicted: "", actual: "", variants: [], rung: LADDER[lane.rung] };
  lane.ledger.push(baselineRow);
  await recordLedger(baselineRow);
  await save(lane);
}

while (lane.status === "running" && lane.attempt < MAX_ATTEMPTS && lane.bestScore < 100) {
  const attempt = lane.attempt + 1;
  lane.phase = "reverse-" + attempt;
  await save(lane);

  const rung = LADDER[lane.rung];
  const rungTask = {
    "clean-c": "Search the current first mismatch with dependency-safe source shape, types, control flow, declarations, and lifetime changes.",
    "static-allocation": "Stop broad spelling churn. Inspect live register lifetimes, saved-register interference, frame, calls, and residual hunks; test only hypotheses tied to that static allocation evidence.",
    "compiler-profile": "Run bounded bin/flag-search for this selector. Test reported compiler/profile candidates and retain a profile only for a verified exact or coherent net improvement.",
    "permuter": "Run one bounded bin/permute coordinator for this selector (60 second hard cap). Inspect and simplify its best semantic candidates; verify each retained candidate with live asm-diff.",
    "compiler-ceiling": "Confirm prior rung evidence is exhausted. Test only one final evidence-backed residual hypothesis; otherwise return a durable compiler-ceiling diagnosis without speculative edits."
  }[rung];
  const reverse = await runs.run("reverse-" + attempt, {
    agent: "bof3-reverse",
    acceptance: false,
    task: [
      "Move " + selector + " toward a verified 100% byte match; attempt " + attempt + "/" + MAX_ATTEMPTS + ", ladder rung " + rung + ".",
      lane.queue.length ? "Run this complete experiment queue: " + JSON.stringify(lane.queue) : "Diagnose at least three distinct evidence-backed experiments for this rung.",
      rungTask,
      "This is a substantive investigation pass: inspect live diff and source/compiler evidence, run every queued experiment plus related safe variants, and re-run live asm-diff after each compiled C89 variant. Retain the best coherent state.",
      "Preserve semantics. No git, publication, other targets, children, inline assembly, or INCLUDE_ASM.",
      "Return JSON with status, match_percent, files_changed, lever, predicted_effect, actual_effect, residual, variants_tried, and rung. For compiler-profile also return coverage_complete; for permuter return coordinator_runs."
    ].join("\n")
  });
  if (!Number.isFinite(score(reverse))) throw new Error("reverse omitted match_percent");

  const liveScore = score(reverse);
  const result = json(reverse);
  const variants = Array.isArray(result.variants_tried) ? result.variants_tried : [];
  const substantive = rung === "compiler-profile" ? (result.coverage_complete === true || result.profile && result.profile.coverage_complete === true)
    : rung === "permuter" ? (result.coordinator_runs === 1 || result.permuter && result.permuter.coordinator_runs === 1)
    : rung === "compiler-ceiling" ? true
    : variants.length >= 3;
  if (String(result.rung || rung) !== rung || !substantive) throw new Error("executor did not complete active ladder rung: " + rung);
  const acceptedBest = liveScore > lane.bestScore || exact(reverse);
  const consumedQueue = JSON.parse(JSON.stringify(lane.queue));
  const attemptRow = {
    attempt,
    score: liveScore,
    improved: acceptedBest,
    lever: result.lever || (consumedQueue[0] && consumedQueue[0].lever) || "initial",
    predicted: result.predicted_effect || (consumedQueue[0] && consumedQueue[0].expected_effect) || "",
    actual: result.actual_effect || result.residual || "",
    variants,
    rung
  };
  if (acceptedBest) {
    lane.phase = "best-promoting";
    lane.pendingAttempt = { attempt, checkpointAttempt: attempt + 1, score: liveScore, consumedQueue, row: attemptRow, result };
    await save(lane);
    const captured = await runs.run("checkpoint-best-" + attempt, {
      agent: "bof3-review", task: "Report the host best checkpoint gate only. No edits.", gate: checkpoint(attempt + 1, reverse, true)
    });
    if (!captured.ok) throw new Error("best checkpoint capture failed");
    const evidence = gateEvidence(captured);
    const bestCheckpoint = evidence.current && evidence.current.checkpoint || evidence.checkpoint;
    const promotedScore = evidence.current && evidence.current.metric && Number(evidence.current.metric.match_percent);
    if (!evidence.accepted || !bestCheckpoint || promotedScore !== lane.pendingAttempt.score) throw new Error("best checkpoint capture rejected");
    await recordLedger(attemptRow);
    const nextLane = JSON.parse(JSON.stringify(lane));
    nextLane.bestScore = promotedScore;
    nextLane.bestCheckpoint = bestCheckpoint;
    nextLane.stalledQueues = 0;
    nextLane.attempt = attempt;
    nextLane.phase = "ready";
    nextLane.ledger.push(attemptRow);
    nextLane.seen.push(...consumedQueue.map(experimentKey).filter(key => !nextLane.seen.includes(key)));
    nextLane.queue = [];
    delete nextLane.pendingAttempt;
    await save(nextLane);
    lane = nextLane;
  } else {
    lane.stalledQueues++;
    lane.attempt = attempt;
    lane.phase = "ready";
  }
  const improved = acceptedBest;
  if (!acceptedBest) {
    lane.ledger.push(attemptRow);
    await recordLedger(attemptRow);
    lane.queue = [];
  }
  const oneShotRung = rung === "compiler-profile" || rung === "permuter";
  const rungLimit = oneShotRung || rung === "compiler-ceiling" ? 1 : STALL_LIMIT;
  const advanceRung = oneShotRung || (!improved && lane.stalledQueues >= rungLimit);

  if (exact(reverse)) {
    lane.status = "exact";
    await save(lane);
    break;
  }
  if (attempt >= MAX_ATTEMPTS) {
    lane.status = "attempt-limit";
    await save(lane);
    break;
  }

  const attemptResult = lane.ledger[lane.ledger.length - 1];
  let review = await runs.run("review-" + attempt, {
    agent: "bof3-review",
    task: [
      "Review " + selector + " after attempt " + attempt + "/" + MAX_ATTEMPTS + " at ladder rung " + LADDER[lane.rung] + ". No edits.",
      "Perform a substantive evidence pass: load role context, inspect live diff and relevant source/compiler output, and verify the tested effects. Respect the active rung; do not send an exhausted rung back to broad source spelling.",
      "If safe to continue, return needs-fix with at least 3 distinct untried semantics-preserving experiments for the active rung; use evidence first.",
      "Every experiment requires a lever and concrete expected_effect. The three must target materially different source/compiler effects, not superficial spelling variants. Safety/semantic/external blocker returns block.",
      "Latest executor result: " + JSON.stringify(attemptResult),
      "Tried experiment keys: " + JSON.stringify(lane.seen)
    ].join("\n")
  });
  const reviewResult = json(review);
  if (String(reviewResult.verdict || "") === "block") {
    lane.status = "blocked";
    lane.blocker = reviewResult.findings || [];
    await save(lane);
    break;
  }
  if (rung === "compiler-ceiling" && (reviewResult.ladder_exhausted === true || String(reviewResult.verdict || "") === "pass")) {
    lane.status = "ladder-exhausted";
    lane.ceiling = reviewResult.findings || reviewResult.residual || [];
    await save(lane);
    break;
  }
  if (advanceRung && lane.rung < LADDER.length - 1) {
    lane.rung++;
    lane.stalledQueues = 0;
    const advanceRow = { attempt, score: liveScore, improved: false, lever: "ladder advance", predicted: "", actual: rung + " exhausted after review; advancing to " + LADDER[lane.rung], variants: [], rung: LADDER[lane.rung] };
    lane.ledger.push(advanceRow);
    await recordLedger(advanceRow);
    lane.queue = [];
    await save(lane);
    continue;
  }

  let available = choices(review, lane.seen);
  if (available.length < 3) {
    review = await runs.run("review-fallback-" + attempt, {
      agent: "bof3-review",
      task: [
        "Build a queue of at least 3 distinct new semantics-preserving experiments for " + selector + " at ladder rung " + LADDER[lane.rung] + ". No edits.",
        "Inspect live evidence and respect the active rung. Return needs-fix with experiments containing lever and concrete expected_effect.",
        "Do not repeat or submit superficial variants: " + JSON.stringify(lane.seen)
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
  if (available.length < 3) {
    lane.status = "blocked-experiment-queue";
    lane.blocker = ["review did not provide at least 3 distinct experiments"];
    await save(lane);
    break;
  }
  lane.queue = available.slice(0, 3);
  lane.seen.push(...lane.queue.map(experimentKey));
  await save(lane);
}

lane.phase = "final-measure";
await save(lane);
const finalMeasure = await runs.run("final-measure", { agent: "bof3-reverse", task: measureTask });
if (!Number.isFinite(score(finalMeasure))) throw new Error("final measurement omitted match_percent");
lane.finalScore = score(finalMeasure);
const finalReview = await runs.run("final-review", {
  agent: "bof3-review",
  task: "Final review " + selector + ". Verify live score and semantics. No edits. Return JSON with verdict, findings, and prepared_rows as an array of [TARGET@]KIND:NAME naming-audit/v3 IDs (empty when none). Mission lane state: " + JSON.stringify(lane)
});
lane.finalReview = json(finalReview);
const measuredRows = preparedRows(finalMeasure);
const reviewedRows = preparedRows(finalReview);
if (JSON.stringify(measuredRows) !== JSON.stringify(reviewedRows)) throw new Error("final reviewer prepared_rows disagree with final measurement");
const rejected = String(lane.finalReview.verdict || "") === "block";
const exhausted = lane.status === "ladder-exhausted";
if (rejected || (lane.finalScore < lane.bestScore && lane.finalScore < 100) || (lane.finalScore <= lane.ledger[0].score && lane.finalScore < 100)) {
  lane.phase = "final-restore";
  await save(lane);
  const restored = await runs.run("restore-final", {
    agent: "bof3-review", task: "Report the host restore only. No edits.", gate: restoreCheckpoint(lane.bestCheckpoint)
  });
  if (!restored.ok) throw new Error("final checkpoint restore failure");
  lane.status = rejected ? "restored-review-block" : lane.finalScore < lane.bestScore ? "restored-below-best" : exhausted ? "restored-ladder-exhausted" : "restored-no-improvement";
} else if (lane.finalScore === 100) {
  lane.status = "exact";
  lane.bestScore = 100;
} else {
  lane.status = "improved-partial";
  lane.bestScore = lane.finalScore;
}
if (lane.status === "exact" || lane.status === "improved-partial") {
  lane.phase = "cleanup-pending";
  await save(lane);
  const cleanupState = lane.status;
  const retainedScore = lane.bestScore;
  const cleanupRows = measuredRows;
  const target = selector.slice(0, selector.lastIndexOf("@"));
  const cleanupRequest = ["retained-lift", target, selector, cleanupState, ...cleanupRows].join(" ");
  if (cleanupRequest.includes(" repair ") || cleanupRequest.startsWith("repair ")) throw new Error("retained lift must not dispatch generic repair");
  const cleanupCheckpoint = await runs.run("checkpoint-pre-cleanup", {
    agent: "bof3-review",
    task: "Report the host cleanup checkpoint only. No edits.",
    gate: [
      "python3 .pi/skills/bof3-lift-loop/scripts/attempt-checkpoint.py capture",
      "--lane", quote(laneKey), "--selector", quote(selector),
      "--attempt", String(cleanupAttempt), "--replace --paths-only --target-scope", quote(target),
      files(finalMeasure).map(quote).join(" ")
    ].filter(Boolean).join(" ")
  });
  if (!cleanupCheckpoint.ok) throw new Error("pre-cleanup checkpoint failed");
  const cleanupEvidence = gateEvidence(cleanupCheckpoint);
  const cleanupLeaf = cleanupEvidence.checkpoint || cleanupEvidence.current && cleanupEvidence.current.checkpoint;
  if (!cleanupLeaf) throw new Error("pre-cleanup checkpoint omitted exact leaf");
  lane.cleanupCheckpoint = cleanupLeaf;
  lane.phase = "cleanup";
  await save(lane);
  const cleanup = await runs.run("cleanup", {
    agent: "bof3-cleanup",
    task: cleanupRequest
  });
  lane.cleanup = json(cleanup);
  if (!cleanup.ok) {
    lane.phase = "cleanup-restore";
    await save(lane);
    const restored = await runs.run("restore-cleanup-failure", {
      agent: "bof3-review", task: "Report the host restore only. No edits.", gate: restoreCheckpoint(lane.cleanupCheckpoint)
    });
    if (!restored.ok) throw new Error("cleanup failure retained-state restore failed");
    lane.status = "cleanup-blocked-restored";
    lane.bestScore = retainedScore;
    lane.phase = "ready";
    await save(lane);
    return lane;
  }
  const consolidationReview = await runs.run("consolidation-review", {
    agent: "bof3-review",
    task: "Final consolidation review " + selector + " after cleanup. Verify live score, semantics, metadata, and retention authority. No edits."
  });
  lane.consolidationReview = json(consolidationReview);
  const consolidationVerdict = String(lane.consolidationReview.verdict || "");
  const approved = consolidationVerdict === "pass" || (lane.status === "improved-partial" && ["retain-improved-partial", "retain-as-improved-partial"].includes(consolidationVerdict));
  if (!consolidationReview.ok || !approved) {
    lane.phase = "consolidation-restore";
    await save(lane);
    const restored = await runs.run("restore-consolidation-failure", {
      agent: "bof3-review", task: "Report the host restore only. No edits.", gate: restoreCheckpoint(lane.cleanupCheckpoint)
    });
    if (!restored.ok) throw new Error("consolidation failure retained-state restore failed");
    lane.status = "consolidation-blocked-restored";
    lane.bestScore = retainedScore;
  } else lane.status = lane.status === "exact" ? "ready-to-integrate-exact" : "ready-to-integrate-partial";
}
if (lane.status === "ready-to-integrate-exact" || lane.status === "ready-to-integrate-partial") {
  lane.phase = "integrate";
  await save(lane);
  const message = lane.status === "ready-to-integrate-exact"
    ? "feat(decomp): byte-match " + selector.split("@")[1]
    : "feat(decomp): improve partial " + selector.split("@")[1];
  const integration = await runs.run("integrate", {
    agent: "bof3-review",
    task: "Report the host integration gate only. No edits.",
    gate: "python3 \"" + manager + "\" integrate --key " + quote(RUN_KEY) + " --selector " + quote(selector) + " --message " + quote(message)
  });
  if (!integration.ok) throw new Error("automatic parent integration failed; lane preserved for inspection");
  lane.integration = gateEvidence(integration);
  lane.status = lane.integration.integrated === true ? "integrated" : "integration-blocked";
}
lane.phase = "ready";
await save(lane);
return lane;
"""
