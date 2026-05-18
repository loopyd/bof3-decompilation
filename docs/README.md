# Docs

This tree holds the human documentation for `rebof3-simple`.

This repo is still settling after the tooling/layout migration. The maintained
surfaces are `bin/` plus `tools/python/`.

Start here:

1. `docs/SETUP.md`
2. `docs/REPO_LAYOUT.md`
3. `docs/DECOMP_WORKFLOW.md`
4. `docs/TROUBLESHOOTING.md`
5. `docs/plan.md`
6. `docs/specs/status.md`

Use `docs/specs/` for stable reverse-engineering knowledge. Keep workflow guidance in the top-level docs files above instead of mixing it into the specs tree.

Some specs still cite legacy generated paths as historical evidence. When
describing the active repo layout or workflows, prefer the top-level docs and
current paths under `out/`.

## Migration Direction

`rebof3-simple` is the active workspace. The sibling legacy `rebof3` tree may
be useful as reference material, but it does not define compatibility
requirements for this repo.

The intended organic layout is:

- `tools/python/rebof3/core/`: small reusable primitives such as paths,
  process execution, task models, and pipeline helpers
- `tools/python/rebof3/tasks/`: generic task builders that can be reused by
  more than one pipeline
- `tools/python/rebof3/setup/tasks/` and workflow-specific packages: concrete
  tasks that do one piece of work
- `tools/python/rebof3/pipelines/` and `tools/python/rebof3/setup/pipelines/`:
  ordered task flows for setup, extraction, inventory, Ghidra bootstrap, and
  matching work
- `bin/`: stable human-facing command wrappers over the Python implementation

Use `bin/pipeline --list` to inspect the composable pipeline surface. Current
high-level recipes include `setup-open`, `extract-assets`, `inventory-refresh`,
`ghidra-bootstrap`, `ghidra-ready`, `decomp-ready`, `build-ready`, `match-loop`,
and `harness-ready`. Use `bin/pipeline <name> --plan` before
running a pipeline when changing task order or adding new tasks.

Ghidra, decompilation helpers, PsyQ staging, extracted disc data, inventory
artifacts, and matching tools are all part of the full reverse-engineering
project. Doctor profiles validate different phases of that project; they do
not turn those dependencies into optional extras.

The active migration status lives in `docs/plan.md`. Keep it concise and
task-oriented so a human can inspect, edit, and extend it without reading a
generated changelog.

## Source Of Truth

- `docs/specs/`
  - stable human-maintained reverse-engineering knowledge
- `out/`
  - generated extraction, inventory, planning, and review artifacts
- `bof3/`
  - recovered or reimplemented PSX-first source
- `docs/DECOMP_WORKFLOW.md`
  - one-function compile and asm-diff workflow
- `bin/`
  - maintained command surface
- `tools/python/`
  - repo-owned maintained CLI and setup implementation
- `docs/plan.md`
  - living migration plan and current implementation status
- `third_party/`
  - vendored external tools
