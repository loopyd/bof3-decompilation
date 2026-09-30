# Plan authoring and management

Read by branch: [scope](#scope-and-format), [syntax](#canonical-syntax),
[consolidation](#reviewed-consolidation), [recovery](#publication-and-recovery).

## Scope and format

Create canonical Markdown in repository planning directory `docs/plans/` only for
current scoped work; empty directory is valid. Select existing plan by caller's
filename or unambiguous scope. Plans are task inputs, not skill reading dependencies.
Read selected plan completely. Refresh owning evidence before writing/executing:
relevant lift status, source symbols check, doctor or focused tests. Durable game
facts belong in specifications; operative rules belong in skill references.

Use descriptive lowercase kebab-case filename. Include goal, evidence baseline,
numbered dependency-ordered phases, affected paths/concrete changes, validation and
acceptance, blockers, owners and non-goals. Smallest evidence-backed plan; no
speculative abstractions, commits or generated/private artifacts. Update/supersede
existing plan rather than maintain conflicting plans.

Execution request authorizes only selected scope. If no plan exists, create smallest
plan first. Execute incomplete phases in dependency order, refreshing baseline
before each; record accepted completion in canonical plan. Stop for explicit blocker
or ambiguity, never execute unrelated plans. Toolchain plans retain managed-tool
ownership of installation/path/environment/verification; wrappers dispatch only.
Keep installed toolchains, proprietary inputs, builds and generated output out of Git.

## Canonical syntax

`bin/harness plans list` and `bin/harness plans status [plan]` parse Markdown each
invocation. Optional selector is exact direct-child kebab-case `.md` filename.
Omitted status selector requires exactly one plan. Empty inventory is not completion;
malformed plans fail rather than disappear. No session/review JSON required afterward.
Management grants no domain backlog execution authority.

First line `<!-- bof3.plan/v1 -->`:

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

States: `open`, `in-progress`, `blocked`, `done`. Stable IDs match
`[A-Z][A-Z0-9.-]*`; never change with title/order/status. Step belongs to preceding
phase; all five fields exactly once. Dependencies: `none` or comma-space-separated
existing IDs, no cycles. Child inherits phase dependencies/blocker; status shows both.
Plain ID requires accepted referenced scope. `ID@unit` requires acceptance of same
explicitly frozen target/row transaction, not whole campaign; canonical prose must
define unit binding/acceptance. Commands validate references/cycles, never schedule
or compute readiness. Aggregate status changes only on whole-scope acceptance.

Fields may reference shared canonical sections. Blocked needs blocker; done needs
acceptance evidence and no blocker. Done phase cannot contain incomplete children.
Count phases/steps separately. Ignore fenced examples; unclosed fences fail. Prose
retains complete contracts. Explicit `<a id="anchor"></a>` anchors and item IDs
are valid mapping destinations. State edits need reviewed evidence, no state setter.

## Reviewed consolidation

`bin/harness plans consolidate REVIEW.json` previews read-only. Apply requires
`--apply --backup-dir ABSOLUTE_DIRECTORY`, fresh external directory with existing
parent. Options cannot repeat. Review/candidate/source/backup ancestors cannot be
symlinks; source files regular, single-link direct children. No glob, recursive
removal, stdin, implicit merge or database.

Closed `bof3.plan-review/v1` JSON has exactly:

- `schema`, `canonical` existing source filename, `reviewer`, `evidence`;
- `candidate: {path, sha256}` external absolute regular file/exact hash;
- `sources: [{name, sha256}]` complete live directory inventory;
- `mappings: [{source, start, end, label, destination, disposition, evidence}]`
  inclusive original lines/label, surviving ID/anchor, `retained` or
  `completed-with-evidence`, attributed rationale/evidence;
- `reconciliations: [{source, id, destination, evidence}]` reviewed mapping for
  incomplete original structured IDs removed or marked done.

Duplicate JSON keys, unknown fields, nonblank coverage gaps, overlaps, invalid
destinations and stale hashes fail before mutation. Independent reviewer inspects
semantic equivalence, every obligation and completion claim; JSON attribution alone
cannot authenticate review. Retain original filename/hash/line/phase mapping in
canonical Markdown, not only temporary JSON.

## Publication and recovery

Apply copies, fsyncs and rehashes every original byte/mode, candidate, review and
manifest externally before publication. Publish candidate with same-directory
atomic replacement; verify installed bytes; delete exact reviewed sources.
Failure returns nonzero/recovery path. Restore only transaction's expected
bytes/absence; preserve concurrent edits. Crash may leave canonical plus old plans;
recovery bytes survive. Trusted-writer recoverability, not race-free multi-file
atomicity. Only exact sole canonical candidate permits read-only replay; partial
state needs inspection/fresh review. Never overwrite/delete recovery runs.

Explicit wipe follows [skill wipe contract](../SKILL.md#explicit-scope-wipe-only):
named plan scope, verified external recovery first, preserve live links and unrelated
content. No session/out/tmp/media or Git authority follows from plan authorization.
