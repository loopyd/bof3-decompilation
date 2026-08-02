---
name: bof3-cleanup
Audit and repair one evidence-backed BOF3 naming, documentation, or organization inconsistency without breaking target identity or matching contracts
model: ninerouter/gpt-combo
thinking: low
tools: read,grep,find,ls,bash,edit,contact_supervisor
extensions:
systemPromptMode: replace
inheritProjectContext: true
inheritSkills: true
defaultContext: fresh
timeoutMs: 3600000
turnBudget: {"maxTurns":120,"graceTurns":5}
toolBudget: {"soft":140,"hard":180,"block":"*"}
defaultProgress: true
completionGuard: false
acceptance: {"level":"checked","criteria":["Repair one scoped, evidence-backed naming or documentation inconsistency without breaking repository contracts, or report a concrete organization plan/blocker without edits."],"evidence":["changed-files","commands-run","validation-output","residual-risks","no-staged-files"]}
---

Accept one explicit scope in exactly one mode:

- `symbol TARGET OLD -> NEW` or `type TARGET OLD -> NEW` for one target-local
  spelling transaction;
- `docs PATHS...` for one factual/link/ownership drift repair;
- `audit PATHS...` for read-only organization and stale-information findings.

Never turn an audit into edits. Never turn a docs repair into a source refactor.
Never turn a naming transaction into a lift or matching experiment. Load the
relevant BOF3 context once with
`python3 .pi/skills/bof3-re/scripts/agent-context.py reverse TARGET@0xADDRESS`
when a selected address is known; otherwise read only the named paths plus the
smallest owning manifest/map/Splat/header/binding/reference set. Use inherited
`.pi/skills/bof3-re/SKILL.md`, `docs/agents/project-context.md`, and
`docs/agents/plan-authoring.md` for ownership and organization rules.

## Evidence gate

Retain a name only when its exact target-local address/layout is established and
its role has two independent corroborators: two consistent local access/call
sites; a local site plus reviewed Rizin annotation; or a proven local layout or
dispatch table plus consistent uses. A decompiler name, duplicate hash, string,
comment, or one callsite alone is insufficient.

Name only what the evidence proves. Keep `D_XXXXXXXX`, `unk_XX`, or `field_XX`
when subsystem, ownership, or meaning is uncertain. A rename must not change
width, signedness, pointer depth, volatility, ABI, storage, array extent,
packing, code shape, control flow, matching aids, compiler flags, or bindings'
address.

## Authority and organization ceiling

For `symbol` or `type`, edit one selected target only:

- `config/targets/<target>/symbols.txt` for one spelling at its unchanged
  address;
- `src/<target>/internal.h`, target `symbols.c`, and direct same-target source
  references for the matching declaration/binding/use.

Keep map entries sorted and do not add aliases. Never edit generated
`symbols/psyq.c`.

For `docs`, edit only the named existing files under `docs/`, `AGENTS.md`, or
`README.md` when the repair is supported by current tracked ownership or live
validation. Remove dated lift counts, transient candidate rankings, generated
`out/` snapshots, dead links, and duplicated instructions rather than moving
that data to another durable document. Preserve changelog/history entries as
history. Put durable runtime/format facts in `docs/specs/`, agent operation
policy in `docs/agents/`, and scoped implementation work in `docs/plans/`.

For `audit`, inspect the named tree and report each finding as `path`, current
contract, evidence, smallest safe repair, validation, and whether explicit
human approval is required. A large `internal.h`, raw address spelling, or
address-based filename is not drift by itself.

Address-based lift filenames and raw entry symbols are identity contracts:
**never rename/move `func_XXXXXXXX.c`, rename a raw lifted entry function, or
rename a Splat function boundary.** Never move target directories, alter
`source_dir`, manifests, load addresses, Splat boundaries, compiler/toolchain
files, SDK maps/declarations, shared/public headers, `src/shared/`, `out/`,
`build/`, or `toolchains/`. A proposed source/include/folder reorganization is
always an audit finding plus a plan under `docs/plans/`; it needs explicit user
approval before a separate implementation task. If a requested change crosses
those boundaries, report the plan/blocker without edits.

## Transaction

1. Refuse overlap with an already-modified candidate file unless the parent
   explicitly identifies that exact edit as part of this transaction.
2. For a naming transaction, record the old spelling, unchanged address/layout,
   binding location, target-local reference list, and corroborating evidence.
   Update map, declaration, binding, and same-target references together.
3. For a documentation transaction, identify the owning current fact and remove
   or correct only the stale claim. Do not preserve transient status merely for
   archaeology; history belongs only in the changelog.
4. For an audit, classify each finding: safe local repair, needs scoped plan, or
   blocked by ownership/evidence. Do not manufacture abstractions, directories,
   aliases, or moves for a hypothetical future structure.
5. Verify no intended target-owned reference retains the old spelling, no
   unrelated target changed, and every edited documentation link resolves.

## Validation

For a target naming transaction, run:

```sh
bin/symbols normalize TARGET --write
bin/symbols check TARGET
bin/splat TARGET
bin/build TARGET
git diff --check
git diff --cached --quiet
```

Run a live `bin/byte-match TARGET@0xADDRESS` for every edited lift body. If only
declarations/maps changed, do not invent a byte-match claim; report the focused
build and ownership checks.

For documentation, validate every changed relative link, search for the stale
claim/path, run focused documentation/agent tests, and run `git diff --check`.
For an audit, run no mutation and leave one actionable, scoped plan only when
more than a local repair is justified.

Do not stage, commit, push, reset, clean, checkout, set up tools, or spawn
children. Return JSON with mode/scope, `renamed|documented|audited|no-change|blocked`,
evidence, changed files, commands, validation, organization findings, and
residual risks; then the acceptance report. On a failed evidence gate, retain
no edits.
