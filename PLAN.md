# Plan Authoring

Create repository implementation plans in [`docs/plans/`](docs/plans/), one
Markdown file per scoped effort. Existing historical plans live there too.

## Before writing

1. Read this file, [`AGENTS.md`](AGENTS.md), and any relevant active plan under
   `docs/plans/`.
2. Establish current evidence with the owning commands (for example
   `bin/decomp-status`, `bin/symbols check`, `just doctor`, or focused tests).
3. Keep durable runtime or file-format findings in `docs/specs/`, not plans.

## Plan format

Use a descriptive kebab-case filename, such as
`docs/plans/toolchain-unification.md`. Include:

- a concise goal and evidence baseline;
- numbered, dependency-ordered phases;
- affected files and concrete changes per phase;
- validation commands and acceptance criteria;
- explicit blockers, ownership boundaries, and non-goals.

Prefer the smallest evidence-backed plan. Do not prescribe speculative
abstractions, commits, or generated/private artifacts. Update or supersede an
existing plan instead of maintaining conflicting plans.

## Executing plans

When the user asks to execute a plan, scan `docs/plans/` first. Select the
specified plan by filename or unambiguous scope, then execute its incomplete
phases in dependency order. Revalidate and refresh the plan's evidence baseline
before each phase, mark completed work in that plan, and stop for an explicit
blocker or ambiguity. A plan is historical only when it explicitly says so;
otherwise treat its incomplete phases as active. Do not execute unrelated plans
merely because they are present in the directory.

## Toolchain plans

A managed toolchain owns its install, executable path, invocation environment,
and verification. `bin/` wrappers only dispatch through the toolchain contract.
Keep installed toolchains, proprietary inputs, build products, and `out/` out
of commits.
