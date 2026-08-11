# Lift-loop workflowScript

Use one `subagent` call per wave. The script owns executor/reviewer retries,
exact-only cleanup, host gates, and final review; the parent still owns queue
selection, generic playbook edits, integration, commits, pushes, and snapshot/index
refresh.

Replace only `SELECTORS`. Keep selectors target-distinct. Launch with repository
`cwd`, `async: true`, and no mutation-worker turn/tool budget.

```js
const SELECTORS = [
  "emi/example/00@0x80123456"
];
const MAX_ATTEMPTS = 6;

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
const reverseTask = s => [
  "Lift " + s + ".",
  "Run agent-context reverse. Own only this target/function files.",
  "Follow the bounded clean-C ladder; preserve the best coherent candidate.",
  "No git, publication, other targets, or children. Return protocol JSON and rung ledger."
].join("\n");
const reviewTask = (s, prior) => [
  "Review " + s + " from fresh repository evidence.",
  "Run agent-context review; inspect live diff, owned changes, semantics/types/ABI/ownership.",
  "Exact: pass or evidence-backed block. Non-exact: pass+exhausted or needs-fix with 1-3 ranked untried experiments.",
  "No source edits. Identify any decisive reproducible improvement as generic-lever candidate.",
  "Executor handoff:\n" + textOf(prior)
].join("\n");
const retryTask = (s, review) => [
  "Retry " + s + " using only the ranked review experiments below, one variant at a time.",
  "Preserve the best legal coherent candidate; obey six-attempt ceiling.",
  "No git, publication, other targets, or children.\n" + textOf(review)
].join("\n");
const cleanupTask = s => [
  "Cleanup reviewed exact " + s + ".",
  "Perform evidence-backed semantic function/symbol/source filename naming, relocation/binding normalization, metadata and owned-file audit.",
  "Preserve exact bytes. No git, publication, other targets, or children."
].join("\n");
const gateOf = s => {
  const t = targetOf(s);
  return "bin/asm-diff '" + s + "' >/dev/null && bin/byte-match '" + s +
    "' && bin/symbols check '" + t + "' && bin/splat '" + t + "' && git diff --check";
};

let lanes = (await runs.all(SELECTORS.map(s => ({
  key: "reverse-0-" + keyOf(s), agent: "bof3-reverse", task: reverseTask(s)
})))).map((run, i) => ({ selector: SELECTORS[i], attempt: 1, executor: run, terminal: false }));

for (let round = 0; round < MAX_ATTEMPTS; round++) {
  const active = lanes.filter(x => !x.terminal);
  if (!active.length) break;
  const reviews = await runs.all(active.map(x => ({
    key: "review-" + x.attempt + "-" + keyOf(x.selector),
    agent: "bof3-review",
    task: reviewTask(x.selector, x.executor)
  })));
  active.forEach((x, i) => { x.review = reviews[i]; });

  const retry = active.filter(x =>
    verdictOf(x.review) === "needs-fix" &&
    experimentsOf(x.review).length > 0 &&
    x.attempt < MAX_ATTEMPTS
  );
  active.filter(x => !retry.includes(x)).forEach(x => { x.terminal = true; });
  if (!retry.length) break;

  const reruns = await runs.all(retry.map(x => ({
    key: "reverse-" + x.attempt + "-" + keyOf(x.selector),
    resume: x.executor.runId,
    task: retryTask(x.selector, x.review)
  })));
  retry.forEach((x, i) => { x.executor = reruns[i]; x.attempt++; });
}

const exact = lanes.filter(x => x.terminal && exactOf(x.executor) && verdictOf(x.review) === "pass");
if (exact.length) {
  const precleanup = await runs.all(exact.map(x => ({
    key: "precleanup-gate-" + keyOf(x.selector),
    agent: "bof3-review",
    task: "Attest only the host exactness gate for " + x.selector + ". No edits.",
    gate: gateOf(x.selector)
  })));
  exact.forEach((x, i) => { x.precleanup = precleanup[i]; });
  const gated = exact.filter(x => x.precleanup && x.precleanup.ok);

  const cleanups = await runs.all(gated.map(x => ({
    key: "cleanup-" + keyOf(x.selector),
    agent: "bof3-cleanup",
    task: cleanupTask(x.selector),
    gate: gateOf(x.selector)
  })));
  gated.forEach((x, i) => { x.cleanup = cleanups[i]; });

  const cleaned = gated.filter(x => x.cleanup && x.cleanup.ok);
  const finals = await runs.all(cleaned.map(x => ({
    key: "final-review-" + keyOf(x.selector),
    agent: "bof3-review",
    task: [
      "Final post-cleanup review " + x.selector + ".",
      "Verify semantic naming, ownership, metadata, old-spelling absence, exact bytes, and cleanup gate evidence.",
      "No edits. Cleanup handoff:\n" + textOf(x.cleanup)
    ].join("\n")
  })));
  cleaned.forEach((x, i) => { x.finalReview = finals[i]; });
}

return lanes.map(x => ({
  selector: x.selector,
  attempts: x.attempt,
  executorRunId: x.executor && x.executor.runId,
  executor: jsonOf(x.executor),
  review: jsonOf(x.review),
  precleanupGatePassed: Boolean(x.precleanup && x.precleanup.ok),
  cleanup: jsonOf(x.cleanup),
  finalReview: jsonOf(x.finalReview),
  integrateExact: exactOf(x.executor) && verdictOf(x.review) === "pass" && verdictOf(x.finalReview) === "pass",
  retainPartial: !exactOf(x.executor) && verdictOf(x.review) === "pass" && jsonOf(x.review).ladder_exhausted === true,
  genericLeverCandidates: [jsonOf(x.review).lesson, jsonOf(x.finalReview).lesson].filter(Boolean)
}));
```

Invocation shell:

```text
subagent({
  workflowScript: <script above>,
  cwd: "/absolute/repository/path",
  async: true,
  timeoutMs: 2400000,
  artifacts: true
})
```

The script deliberately returns decisions instead of committing. Parent checks
unexpected/overlapping paths, records only generic reusable levers, integrates in
queue order, commits/pushes under authorization, refreshes edited snapshots, and
rebuilds the index once per wave.
