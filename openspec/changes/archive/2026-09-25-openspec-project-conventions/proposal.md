# Proposal

## Why

`openspec/config.yaml` declares only `schema: spec-driven`; every other key is
commented-out example text. OpenSpec therefore injects no project context and no
per-artifact rules into artifact prompts, so every proposal, spec, design and
tasks document authored in this repository starts with zero knowledge of BOF3's
hard contract. The surviving examples describe a TypeScript/React e-commerce
stack, which is actively wrong for a MIPS matching-decompilation repository.

Verified baseline: `openspec instructions proposal --change
openspec-project-conventions --json` returns no `context` field and
`"rules": undefined`.

## What Changes

- Activate `context:` as a short, high-signal description of this repository that
  **points at the owning documents** (`AGENTS.md`, `docs/INDEX.md`,
  `docs/agents/project-context.md` § Hard repository contract) rather than
  duplicating them, honouring the one-owner-per-contract rule in
  `docs/agents/INDEX.md`.
- Activate `rules:` for `proposal`, `specs`, `design` and `tasks` — the four
  artifact ids the `spec-driven` schema defines.
- Activate `operations.apply.guidance` and `operations.archive.guidance`.
- Delete the commented-out example blocks, which are the file's only current
  content and misdescribe this project.
- Keep `schema: spec-driven` unchanged and leave `store:` unset; this repository
  remains repo-local (`openspec store list --json` reports no stores).

## Capabilities

### New Capabilities
- `openspec-artifact-conventions`: this repository's OpenSpec artifact-generation
  contract — every artifact instruction emitted here carries BOF3's hard contract
  plus the per-artifact authoring rules, and apply/archive carry their own
  guidance.

### Modified Capabilities
(none — `openspec/specs/` currently holds no capability specs)

## Impact

- `openspec/config.yaml` — the only file this change edits.
- Downstream, with no code change: the JSON payload of `openspec instructions
  <artifact> --change <name> --json` for every future change in this repository
  gains `context` and `rules`.
- Untouched: harness code (`tools/python`, `bin/`), tests, the `docs/` tree,
  `AGENTS.md`, and the pre-existing dirty worktree.
