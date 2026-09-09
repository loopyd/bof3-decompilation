# Plan Authoring

Create a repository implementation plan in `docs/plans/` only for a current,
scoped effort. The directory is intentionally empty when no such plan exists.

## Before writing

1. Read this file, [`AGENTS.md`](../../AGENTS.md), and the relevant plan under
   `docs/plans/` when one exists.
2. Establish current evidence with the owning commands (`bin/decomp-status`,
   `bin/symbols check`, `just doctor`, focused tests).
3. Keep durable runtime or file-format findings in `../specs/`, not plans.

## Plan format

Use a descriptive kebab-case filename, such as
`../plans/toolchain-unification.md`. Include:

- a concise goal and evidence baseline;
- numbered, dependency-ordered phases;
- affected files and concrete changes per phase;
- validation commands and acceptance criteria;
- explicit blockers, ownership boundaries, and non-goals.

Prefer the smallest evidence-backed plan; no speculative abstractions,
commits, or generated/private artifacts. Update or supersede an existing plan
instead of maintaining conflicting plans.

## Executing plans

When the user asks to execute a plan, select the specified existing plan by
filename or unambiguous scope. If none exists, create the smallest plan first.
Execute incomplete phases in dependency order, refresh the evidence baseline
before each phase, mark completed work in that plan, and stop for an explicit
blocker or ambiguity. Do not execute unrelated plans merely because they are
present in the directory.

## Toolchain plans

A managed toolchain owns its install, executable path, invocation environment,
and verification. `bin/` wrappers only dispatch through the toolchain contract.
Keep installed toolchains, proprietary inputs, build products, and `out/` out
of commits.

## Persistent plans management

`$plans` owns semantic reconciliation. `bin/plans list` and
`bin/plans status [plan]` parse canonical Markdown on each invocation; no session
or review JSON is needed afterward. The optional selector is an exact direct-child
kebab-case `.md` filename. Empty inventory is not completion; omitted status
selection requires exactly one plan. Malformed plans fail, never disappear.
Plan management does not authorize executing the domain backlog.

### bof3.plan/v1

The first line is `<!-- bof3.plan/v1 -->`. Phases and steps use these delimiters:

```markdown
<!-- bof3.plan/v1 -->
# Program
## 1. [A1] (open) Phase title
- Owner: parent
- Depends: none
- Blocker: none
- Evidence: none
- Acceptance: attributed independent review and owning checks
1. [A1.01] (open) Step title
- Owner: worker
- Depends: none
- Blocker: none
- Evidence: none
- Acceptance: runnable owning check
```

States are `open`, `in-progress`, `blocked`, `done`; IDs match
`[A-Z][A-Z0-9.-]*` and never change with title/order/status. Each step belongs to
the preceding phase. Every item has all five fields exactly once; dependencies
are `none` or comma-space-separated existing IDs, without cycles. A child also
inherits its phase's dependencies and blocker; status displays both. Plain ID
edges require accepted completion of the referenced scope. `ID@unit` instead
requires accepted evidence for the same explicitly frozen target/row transaction,
not completion of that ID's entire campaign. A plan using `@unit` must define its
unit binding and acceptance in canonical prose; commands validate references and
cycles but never compute readiness or schedule work. Aggregate states change only
when the declared whole scope is accepted. Fields can
reference shared canonical sections. Blocked items require a blocker; done items
require recorded acceptance evidence and no blocker, and done phases cannot have
incomplete children. Phase/step counts are separate. Fenced examples are ignored;
unclosed fences fail. Ordinary prose remains available for complete contracts.
Explicit `<a id="anchor"></a>` anchors are valid mapping destinations alongside
item IDs. State edits require reviewed evidence, not a state-setter command.

### Reviewed consolidation

`bin/plans consolidate REVIEW.json` is read-only preview. Apply requires
`--apply --backup-dir ABSOLUTE_DIRECTORY`, a fresh external directory whose parent
exists. Single options cannot repeat. Review/candidate/source/backup ancestors
must not be symlinks; source files must be regular, single-link direct children.
No glob, recursive removal, stdin, implicit merge, or database.

Closed JSON schema `bof3.plan-review/v1` has exactly:

- `schema`, `canonical` (existing source filename), `reviewer`, `evidence`;
- `candidate: {path, sha256}`: external absolute regular file and exact hash;
- `sources: [{name, sha256}]`: complete live directory inventory;
- `mappings: [{source, start, end, label, destination, disposition, evidence}]`:
  inclusive original line ranges, original label, surviving ID/explicit anchor,
  `retained` or `completed-with-evidence`, and attributed rationale/evidence;
- `reconciliations: [{source, id, destination, evidence}]`: explicit reviewed
  mapping for original incomplete structured IDs removed or marked done.

Duplicate JSON keys, unknown fields, gaps in nonblank line coverage, overlaps,
invalid destinations and stale hashes fail before mutation. Independent reviewers
must inspect semantic equivalence, every obligation and completion claim; JSON
attribution alone cannot authenticate review. Keep permanent original filename,
hash, line/phase mapping in canonical Markdown, not solely temporary JSON.

Apply copies/fsyncs/rehashes every original byte/mode, candidate and review plus
manifest externally before publication. Candidate publication is same-directory
atomic replacement, then installed-byte verification, then exact reviewed source
deletions. Failure returns nonzero with recovery path; restoration touches only
this transaction's expected bytes/absence, preserving concurrent edits. A crash
may leave canonical plus old plans; recovery bytes survive. This is one trusted
writer's recoverable transaction, not race-free multi-file atomicity. Only the
exact sole canonical candidate permits read-only replay; partial state needs
inspection and fresh review. Never delete or overwrite recovery runs.

Wipe guidance belongs to the skill: require explicit named plan scope and verified
external recovery first. Authorization never extends to session/out/tmp/media
artifacts or Git state. Preserve live links and unrelated dirty content.
