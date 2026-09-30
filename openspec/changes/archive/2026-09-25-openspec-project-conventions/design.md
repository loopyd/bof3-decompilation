# Design

## Context

See `proposal.md` — Why. Two properties of OpenSpec 1.13.2 constrain the
approach, both verified in the installed CLI:

- `context` is re-injected into **every** artifact instruction and is capped at
  50 KB (`MAX_CONTEXT_SIZE`), so it is a per-request cost rather than a one-off
  document.
- `rules` is a map of artifact-id to string list, validated against the artifacts
  of every installed schema; an unknown key emits a warning (once per session),
  so a typo otherwise rots silently.

This repository already owns its conventions in `AGENTS.md` and `docs/agents/*.md`,
and `docs/agents/INDEX.md` requires one owner per contract.

## Goals / Non-Goals

**Goals:**

- Make BOF3's non-negotiable constraints reach artifact generation automatically.
- Keep exactly one owner per contract.
- Stay far below the 50 KB context budget.

**Non-Goals:**

- Do not restate or fork `AGENTS.md` / `docs/agents/*.md` content inside
  `openspec/config.yaml`.
- Do not change harness behaviour, the `justfile` recipes, or the `docs/` tree.
- Do not register or target a store; this repository stays repo-local.
- Do not move the decompilation workflow itself into OpenSpec artifacts.

## Decisions

### Decision 1: configure the tool, do not document it

The conventions go in `openspec/config.yaml` rather than a new `docs/` page,
because OpenSpec reads this file itself and injects it. A docs page would depend
on a reader remembering to open it; the config is applied on every artifact
request. *Alternative considered:* a `docs/agents/openspec.md` guide — rejected
as a second owner that nothing enforces.

### Decision 2: `context` points at owners and states only non-negotiables

`context` names the owning documents plus the constraints that must hold even if
those documents are never opened (no unauthorised tests or dependencies;
`out/`, `build/`, `toolchains/` and generated bindings are not reviewed truth;
original bytes outrank analyzer output; no commit/push/release without approval).
Everything else stays with its owner. *Alternative considered:* inlining the
hard contract — rejected because it creates a rival copy that drifts.

### Decision 3: one `rules` entry per schema artifact id

Rules are keyed by exactly `proposal`, `specs`, `design` and `tasks`, the ids the
active `spec-driven` schema defines, so no unknown-key warning can fire. Each
entry carries that artifact's repository obligation, such as tasks stating their
verification and forbidding unauthorised tests/dependencies.

### Decision 4: keep `skip_specs` unset

The schema offers `skip_specs: true` for changes with no behaviour delta.
Artifact-generation output *is* observable through `openspec instructions
--json`, so this change carries a real capability delta instead of opting out.

### Decision 5: operation guidance is advisory, never a gate

`operations.apply.guidance` and `operations.archive.guidance` are additive advice.
Real enforcement stays in the harness (`just check`, `bin/harness docs refs`);
the config must not be presented as a substitute for those gates.

## Risks / Trade-offs

- [Context drifts away from the documents it points at] → `context` references
  owners instead of copying them, and a spec scenario asserts it names
  `AGENTS.md` and `docs/INDEX.md`.
- [A mistyped rule key silently enforces nothing] → use only the four schema
  artifact ids, and verify no unknown-key warning appears.
- [Context grows into a second docs tree] → the 50 KB cap is a hard limit;
  additions are restricted to non-negotiables by Decision 2.
- [Over-promising enforcement] → Decision 5 keeps guidance advisory and leaves
  the real gates with the harness.
