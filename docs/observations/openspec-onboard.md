# openspec-onboard observations

Historical evidence for improving this skill, its references and supporting tools.
The [measurement contract](INDEX.md#performance-measurement-contract) governs
counts and comparisons. No skill or harness change is implemented by this ledger.

## Coverage and provenance

One complete parent-session episode reviewed: 2026-09-25,
`openspec-project-conventions`, records 11–156 of
`.pi/sessions/2026-09-25T23-10-21-878Z_01a0dad5-86b6-74b4-97f0-9382ab99ade4.jsonl`.
All recorded assistant text, reasoning and tool outputs in that range were read;
recorded outputs that were already clipped by the original command remain clipped
evidence. Historical tool calls were inspected, not executed. Expanded onboarding
requests at records 6 and 10 establish attribution; they describe the same episode.

The four artifact stages, apply and archive are phases of this onboarding mission,
not separate invocations of six other skills. Other history remains under review.
Metrics and source hash: `tmp/observation-ingest/onboard-metrics.json`.

## Measured performance

| Phase / records | Tool calls with matching results | Observed seconds |
| --- | --- | --- |
| Preflight and task discovery, 11–60 | 29 | 179.989 |
| Selected-task exploration, 62–80 | 11 | 24.573 |
| Artifact planning, 82–123 | 19 | 169.915 |
| Configuration, repair and checks, 125–151 | 14 | 52.308 |
| Archive and recap, 152–156 | 2 | 8.677 |
| Whole episode, 11–156 | **75** | **524.390** |

The whole span includes user-response intervals; phase spans omit gaps between
phases but planning still includes approvals. These are transcript timestamps,
not summed subprocess durations or active-model time. One episode supplies no
population distribution. All 75 requested calls have matching results. Nested
commands inside one tool call are not additional tool calls.

Model metadata identifies `ds-combo`; no provider substitution is inferred.
Before the measured episode, record 7 contains one failed assistant generation:
a gateway 503 wraps a provider 401 authentication error. It made no tool call.
This startup failure is outside the 75-call/524.390-second implementation cohort;
retain availability failures separately from skill execution and never infer a
documentation-method defect from a failed generation. Credential text is omitted.
Across 66 assistant messages, recorded usage sums to 243,084 input tokens,
7,242,368 cache-read tokens and 49,645 output tokens. Reasoning is separately
reported as 26,703; it is not an additional term in the recorded total, which
equals input + cache-read + output. Recorded total is 7,535,097, including repeated cached context, not
that many distinct source tokens. Zero cost fields do not establish free execution.

Outcome: CLI reported **5/5 tasks**, `all_done`, then archived the change and
created a main spec with four added requirements. Six instruction payloads
contained 1,067 trimmed context characters; four artifact payloads each had three
rules and two operation payloads each had three guidance items, with clean stderr
after repair (record 140). The retained archive/tasks file corroborates the five
checked boxes. This is observed delivery, not independent proof that every authored
constraint or scope claim was correct.

## Iteration audit log

| Episode | Method verdict | Outcome limits | Improvement destination |
| --- | --- | --- | --- |
| `openspec-project-conventions` | Complete cycle with 75 calls; configuration needed repair; discovery spent 29 calls before task selection | Scope proof lacked a complete starting snapshot; final recap misstated a dependency | Proposals below; current instructions unchanged |

## Durable lessons

- **Discovery should follow repository ownership first.** The initial TODO scan
  returned 25 vendor/toolchain/cache hits and required a second scoped scan
  (19–21). The first entry-index read arrived at record 22. By record 24 a real
  nine-link defect was available; investigation continued through record 59 to
  offer alternatives. These calls have differing value: ownership verification
  is useful, repeated exploration before user selection needs a bounded stopping
  rule. No measured time-saving claim follows from this one episode.
- **Capture complete, structured evidence once.** The reference report was
  clipped at record 40, then repeatedly re-run to recover counts and details.
  The two-file scope had nine broken occurrences/seven unique targets; the wider
  docs/skills scope had eleven occurrences/four files (44, 52, 54). Different
  scope and distinct-target counts are not interchangeable.
- **Validate configuration semantics through emitted fields and stderr.** An
  unquoted colon-space in a YAML rule caused OpenSpec to warn and ignore the file;
  all six requested instruction surfaces lacked the intended fields (128).
  One content repair restored them (134–140). Schema-shaped JSON or a nominally
  successful CLI invocation alone would have missed this failure.
- **Separate wrapper failure from product failure.** JavaScript containing a
  shell `cd` failed before the reference check (42). A malformed ternary failed
  before the post-repair verification (138). `validate --change` was unsupported
  (102); positional item selection worked (104–106). The printed `EXIT:0` at
  record 102 was the pipeline status, not proof of validation success. These are
  at least three invocation/script errors among 75 calls, not three product bugs.
- **Check semantic fidelity when compacting policy into context.** The owner
  text at record 79 made `out/` disposable and barred hand edits to `build/`,
  `toolchains/` and generated bindings. The configuration write at record 125
  extended the hand-edit prohibition to `out/`. Successful field injection did
  not detect that broadened rule. References to owners do not excuse an inaccurate
  restatement beside them.
- **A dirty tree needs a baseline, not a late clean-tree assertion.** Task 2.2
  initially required unmodified docs; the later check found 39 dirty doc paths
  (145–147). The operator amended the task using older mtimes, unrelated diff
  content and known write targets. That supports historical attribution, but
  does not replace before/after content hashes. Absence of the word `openspec`
  in a diff also cannot prove absence of unintended edits.
- **Recaps must preserve what the tool actually proved.** Records 96/118 show
  that proposal unlocks specs and design; tasks depends on specs and design.
  Records 123/156 incorrectly said proposal and design unlock in parallel.
  Record 60 changed a count of 460 Python files into a “460-line harness.”
  Record 156 called untracked work “staged-in-worktree.” These are reporting
  corrections, not evidence that the tool returned the wrong state.
- **Planning completion, implementation and archive are separate outcomes.**
  Four artifacts were done before any config implementation (122); five task
  boxes were later done (151); archive then updated four requirements (153).
  The observed `openspec/` tree was untracked. Archive success did not stage or
  commit it, and none of these states proves independent semantic acceptance.

## Improvement proposals

| Measured problem | Proposed directive/reference/tool change | Acceptance measurement |
| --- | --- | --- |
| 29 discovery calls before selection; first scan entirely vendor material | Onboarding guidance: read the repository route first, scope candidate discovery to owned files, and stop once useful choices are evidenced | Compare calls/time to task selection on similar tutorials; each suggestion remains source-backed and existing checks remain intact |
| Three script/CLI invocation errors; clipped reports caused rereads | Operational reference: use native tools, CLI-returned syntax and structured complete captures; preserve command status separately from presentation | Invalid invocations / relevant calls decreases; counts, exit status and source locations remain recoverable |
| Invalid YAML made six instruction surfaces omit configuration | Config example/reference: parse before the six-surface verification, then assert required fields and inspect stderr | Same six field checks pass with clean stderr; intentionally invalid input is reported as missing configuration, never success |
| Five boxes checked despite weak scope proof and a broadened context rule | Review guidance: bind task completion to starting path/content evidence and compare every summarized constraint with its owner | No unsupported “untouched” claim or changed policy meaning; preserve unrelated work without demanding a clean tree |
| Dependency, unit and staging errors in final prose | Reporting guidance: generate factual recap fields from retained tool output and distinguish evidence from inference | Dependency graph, file/token units, archive state and staging status agree with sources |

These are investigation/implementation proposals. No dependency installation,
installed extension change, new tests, or new approval requirement is implied.
