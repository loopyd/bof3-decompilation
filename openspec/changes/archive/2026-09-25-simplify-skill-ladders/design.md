# Design

## Context

See `proposal.md` — Why. Facts established by read-only inspection:

- Ladder constructs exist in eight files, with `LESSONS.md` already declaring a
  no-duplication rule for lever tables.
- Current sizes: `matching-playbook.md` 583, `tool-usage.md` 1242, `bof3-re/SKILL.md` 87,
  `LESSONS.md` 59, `OBSERVATIONS.md` 135, `MISSION_PROTOCOL.md` 63,
  `REVIEW_CHECKLIST.md` 76, `BYTE_SAFE_COSMETICS.md` 42.
- `.pi/skills` holds 69 Markdown files (~6360 lines) and 11 scripts; the largest files
  are the vendored `openspec-*` skills (~2900 lines), which are installed skill packages
  rather than project guidance.
- Anchors referenced from other documents must keep working, for example
  `matching-playbook.md#allocation-ladder` and
  `matching.md#first-source-and-required-loop`.
- `bin/harness source docs` (drift) compares tool documentation with code and CLI
  evidence. The automated reference sweep (`bin/harness docs refs`) is retired by the
  `retire-docs-transport` change, so link checks here are manual.

## Goals / Non-Goals

**Goals:**

- Make each ladder a compact, ordered, rung-per-line procedure with an evidence gate.
- Give each ladder one home and make the others reference it.
- Reduce the guidance's size without losing a gate, ban, threshold or obligation.

**Non-Goals:**

- No change to acceptance rules, thresholds, evidence requirements or verification gates.
- No harness code, Rust crate or test changes.
- No edits to the vendored `openspec-*` skills.
- No commit.

## Decisions

### Decision 1: simplify ladders first, compact prose second

Compaction runs against the final ladder text. *Rejected:* compacting first — it would
rewrite prose twice, and the second pass would re-introduce the duplication the first
removed.

### Decision 2: a rung is one line — action plus evidence gate

Each rung states the change to make and the evidence that accepts it. *Rejected:*
keeping narrative explanations inside rungs — that is the duplication being removed; the
explanation belongs with the lever table's owner.

### Decision 3: one home per ladder, references elsewhere

`matching-playbook.md` keeps the lever tables; `OBSERVATIONS.md` keeps its rung-cost
policy; skills link to them. This enforces the rule `LESSONS.md` already states rather
than inventing a new one.

### Decision 4: preserve a grep-checkable gate vocabulary

Preservation is verified by tokens, not asserted: `ladder_exhausted`, `INCLUDE_ASM`,
`REGISTER_PIN`, `CLOBBER_`, barrier, `byte-match`, `@source`, `@behavior`. *Rejected:*
relying on review alone to notice a dropped ban — silent contract loss is exactly the
failure mode of compaction.

### Decision 5: exclude the vendored `openspec-*` skills

They are installed skill packages (~2900 of the 6360 skill lines) and editing them needs
separate authorization, so they stay untouched and their size is not part of the
measured reduction.

### Decision 6: verify with existing checks, add no tests

`bin/harness source docs` (drift), the scoped `just check`, and `git diff --check` cover
this change. *Rejected:* adding a guidance-lint test — this repository does not add
regression tests unless the operator explicitly requests them.

## Risks / Trade-offs

- [A ban or threshold is silently lost while compacting] → Decision 4's token list plus
  the spec scenario that requires each token to remain in the file that stated it.
- [Owners diverge again after the change] → Decision 3 makes one home explicit.
- [Moving a heading breaks a link from another document] → heading anchors referenced
  elsewhere are preserved by name; verify each by hand because the automated sweep is
  retired by `retire-docs-transport`.
- [Editing the in-flight skill relocation] → the change scopes edits to the named
  guidance files and records the touch; the rest of the relocation is untouched.
- [Reduction is claimed but not measured] → the spec requires before/after line counts
  to be recorded for the ladder-bearing files.
