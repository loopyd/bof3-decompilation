# Resumable full-project decompilation prompt

Copy the text below into a fresh planning or implementation agent. It
orchestrates repository skills; it does not duplicate their procedures or prove
that any target is complete.

```text
Lead the BOF3 PlayStation reverse-engineering effort in the current repository.
Resume from authoritative current state, not a prior transcript, plan, generated
artifact, analyzer cache, or claimed completion.

Outcome

- Recover all reviewed game code as factual, readable, period-appropriate C89.
- Prioritize the complete path through EMI loading, boot/logo, title/start,
  New Game and Load, character-name entry, game-start transition, and the
  inactivity/demo branch.
- Seek canonical 100% instruction and byte matches. For incomplete work retain
  the best compiling factual reconstruction, measured diff, first actionable
  mismatch, and next verification step.
- Improve durable source, address-traceable symbols, reviewed types, reproducible
  analyzer evidence, specifications, and stable automation as work proceeds.

Resume

1. Read AGENTS.md and CONTEXT.md. Inspect git status, recent focused commits,
   active work, manifests, source, specs, generated evidence, and available
   tools. Preserve user/unrelated changes.
2. Use $bof3-docs to locate repository documentation, $bof3-specs to interpret
   BOF3 payloads/claims, $psx-rizin to maintain analyzer projects/evidence, and
   $decomp-loop to lift/match. Read each selected skill completely and keep its
   responsibility boundary.
3. Derive a live inventory of executables and promoted EMI targets, reviewed
   code/data ranges, match state, unresolved boundaries, PsyQ dependencies, and
   owners of every requested startup behavior. Rank by dependency and outcome.
4. For each selected item record success evidence before editing: exact input
   and hash, load mapping, entry convention, boundary, canonical assembly,
   compiler command, baseline diff, and owning tracked files.

Execution

- Preserve distinct executable/archive/entry/target identities. Original bytes
  and headers outrank metadata; reviewed layouts outrank analyzer guesses.
- Maintain one reproducible analyzer project per independently mapped target.
  Put generated state under out/ and only reviewed deterministic replay/types
  under config/analysis/. Add names, functions, comments, types, and xrefs only
  as supported facts; preserve func_XXXXXXXX/DAT_XXXXXXXX traceability.
- Use call graphs and xrefs to prove the requested startup path. Correlate
  repeated implementations, layouts, callbacks, constants, strings, and PsyQ
  identities without merging target-local addresses or ownership.
- Follow $decomp-loop for every function: pre-diff, inspect evidence, edit the
  smallest factual C89, compile through the real target, post-diff, and use
  bounded permutation when useful. A candidate requires canonical revalidation.
- Omit permuter --seed for fresh exploration. Set and record a seed only to
  reproduce/debug a useful or failed run. Budget workers from current cores and
  load using the decomp-loop headroom rule across all agents.
- At a verified 100% match, prepare one focused change containing only the
  function and required layout/declarations/bindings. Commit or push only with
  explicit authorization.

Delegation

- Delegate independent, bounded targets or evidence lanes. Give each agent exact
  ownership, addresses/files, acceptance commands, output bounds, and mutation
  limits. Avoid shared-file collisions and share one CPU budget.
- Require pre/post evidence, facts versus inferences, checks, changed files, and
  remaining work. Review every result; reassign or close idle/stale agents.
- Repeated patterns are candidates for automation only after their contract is
  stable. Propose configurable dry-run/bounded/JSON tooling and get user
  alignment before materially expanding the harness.

Evidence and output

- Keep hypotheses/raw sessions under out/. Put stable domain facts in the owning
  spec and reusable gotchas in LESSONS.md or the owning skill.
- Bound console output to a summary, first actionable issue, omitted count, and
  artifact path. Classify non-text bytes, invalid strings, escape-heavy values,
  large tables, and analyzer errors; preserve exact bytes as bounded hex/base64
  rather than Unicode escape floods.
- Never hand-edit generated state or commit media, toolchains, build/, or out/.

Completion

- A target is complete only when all reviewed code is compiling C, every
  data/rodata/padding range has ownership, external bindings are correct, and
  the rebuilt payload matches. Report function and whole-target status apart.
- The requested frontend is complete only with dynamic evidence or complete
  reviewed call/xref evidence for every named transition; matched leaves alone
  are insufficient.
- Run focused checks while iterating, then git diff --check, just check, and
  bin/harness doctor --strict when available. Report Done, Evidence, Checks,
  Skipped, Remaining gaps, and Next. Claim project completion only after a
  requirement-by-requirement evidence audit.
```
