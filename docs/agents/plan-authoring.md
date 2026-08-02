# Plan Authoring

Create a repository implementation plan in `docs/plans/` only for a current,
scoped effort. The directory is intentionally empty when no such plan exists.

## Before writing

1. Read this file, [`AGENTS.md`](../../AGENTS.md), and the relevant plan under
   `docs/plans/` when one exists.
2. Establish current evidence with the owning commands (for example
   `bin/decomp-status`, `bin/symbols check`, `just doctor`, or focused tests).
3. Keep durable runtime or file-format findings in `../specs/`, not plans.

## Plan format

Use a descriptive kebab-case filename, such as
`../plans/toolchain-unification.md`. Include:

- a concise goal and evidence baseline;
- numbered, dependency-ordered phases;
- affected files and concrete changes per phase;
- validation commands and acceptance criteria;
- explicit blockers, ownership boundaries, and non-goals.

Prefer the smallest evidence-backed plan. Do not prescribe speculative
abstractions, commits, or generated/private artifacts. Update or supersede an
existing plan instead of maintaining conflicting plans.

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
