# Design

## Context

See `proposal.md` — Why. Facts established read-only:

- `SKILL.md` is 49 lines: frontmatter, a 23-row catalog of which 21 rows are ranked `2.`,
  and three contract paragraphs.
- `references/` holds 25 flat files totalling 1274 lines; `scripts/` holds 52 Lua modules
  plus `runtime.sh`; `tests/` holds 55 Lua files; 134 files in total.
- `.pi/agents/psx-emulator.md` is the dispatch spec and instructs the reader to read
  `SKILL.md` and use `runtime.sh`.
- Two constraints bound the work. The skill's own text says *"keep this entrypoint
  broad"*, so breadth of coverage must survive. And `bin/harness docs refs` — the
  automated link sweep — is retired by `retire-docs-transport`, so link confirmation here
  is manual unless it precedes that change.

## Goals / Non-Goals

**Goals:**

- A small layered entrypoint with an on-demand catalog.
- Every ability and its owning reference one hop away.
- Owning documents updated; no behaviour change.

**Non-Goals:**

- Removing or merging abilities.
- Editing any Lua script, `runtime.sh`, or any of the 55 tests.
- Changing the harness `runtime` command or any acceptance rule.
- No new tests, no commit.

## Decisions

### Decision 1: five named layers, each with one owning reference

Layers: **runtime and bounds**, **observe state**, **drive execution**, **capture
devices**, **extend an action**. *Alternative considered:* keeping the existing list and
only relabelling the ranks — rejected, because 21 of 23 rows already share one rank, so
the list is flat by construction rather than by wording.

### Decision 2: the catalog moves rather than shrinks

The 23-action catalog moves into an on-demand reference with layer grouping added, and
keeps every entry. *Alternative considered:* trimming the catalog to the most-used
actions — rejected: it is the only single place mapping actions to references.

### Decision 3: the entrypoint loses size, not breadth

Every capability family stays reachable through some layer, satisfying the skill's own
*"keep this entrypoint broad"* rule. *Alternative considered:* reducing the entrypoint to
a one-mission checklist — rejected as narrowing scope rather than re-layering it.

### Decision 4: references group by layer but keep their file names

Existing references keep their identity (`CDROM.md`, `GPU.md`, …) and gain a layer home,
so 1274 lines of maintained content are not rewritten. *Alternative considered:*
rewriting 25 references into 5 monoliths — rejected: churn without retrieval benefit, and
it would invalidate inbound links wholesale.

### Decision 5: verify with existing checks, and compare sets

Scoped `just check`, `git diff --check`, and a before/after set comparison of actions,
references and scripts prove nothing was lost. *Alternative considered:* adding a
skill-structure test — rejected: this repository does not add regression tests unless
explicitly requested.

## Risks / Trade-offs

- [A grouping pass silently drops an ability] → the "every ability is preserved"
  requirement makes the action, reference and script sets equal to the recorded baseline,
  so a loss fails verification rather than being noticed later.
- [Moving references breaks inbound links] → Decision 4 keeps file names, and every
  inbound link from the entrypoint, agent spec, docs routes and the plan is checked by
  hand.
- [The entrypoint shrinks past usefulness] → Decision 3's breadth scenario is checked
  layer by layer against the baseline catalog.
- [Ordering against `retire-docs-transport`] → link checks are done while the sweep still
  exists, or manually afterwards; this is an explicit task, not an assumption.
