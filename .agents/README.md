# BOF3 agent configuration

Keep each concern in one layer. `AGENTS.md` is the actual always-loaded project
policy; `.agents/rules/` are concise policy references for clients that support
rule loading and must not contain the only copy of a safety invariant.

- `prompts/` orchestrates complete outcomes across skills and rules.
- `rules/` contains short, always-on repository policy.
- `skills/` contains scoped workflows or domain knowledge loaded on demand.

## Rules

- `rules/decomp.md` — authored C and symbol traceability policy
- `rules/build.md` — target ownership and minimum verification policy

## Prompts

- `prompts/full-project-decompilation.md` — resumable, evidence-driven whole-project orchestration

## Skills

- `bof3-docs` — locate the smallest authoritative repository document
- `bof3-specs` — interpret BOF3 binaries, formats, layouts, and evidence
- `decomp-loop` — lift and exactly match functions or targets
- `psx-rizin` — collect reproducible PSX analyzer snapshots and replay evidence
