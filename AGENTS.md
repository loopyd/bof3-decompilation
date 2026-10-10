# AGENTS.md — BOF3

At the start of every session, invoke `/saber-mode` before task work. Read its
entire `SKILL.md`, including the full Actions and Playbooks routing ladder. If slash
invocation is unavailable, read `~/.agents/skills/saber-mode/SKILL.md` directly.
Follow that ladder and read each triggered leaf skill before applying it; load
branch references as needed. Report an unavailable mode rather than silently skipping it.

Route each request to its BOF3 owner below. Saber mode governs workflow; repository
contracts and task-specific authorization still govern scope and acceptance.
Operative rules live in skills/references. `docs/` is user documentation, not required
agent reading. During authorized maintenance, extract useful knowledge into relevant
references. Caller-selected documents/plans remain task inputs, not standing dependencies.

| Request | Owner |
| --- | --- |
| Lift, match, target/source/map/layout, duplicates | `.pi/skills/bof3-re/SKILL.md` |
| Autonomous campaign or resume | `.pi/skills/bof3-lift-loop/SKILL.md` |
| Naming, identity, source identifiers | `.pi/skills/bof3-naming/SKILL.md` |
| Type representation / macro work | `.pi/skills/bof3-types/SKILL.md` / `.pi/skills/bof3-macros/SKILL.md` |
| Python harness, CLI, skill scripts | `.pi/skills/bof3-re/references/tooling-contract.md` |
| Tests / Markdown | `.pi/skills/bof3-test/SKILL.md` / `.pi/skills/bof3-docs/SKILL.md` |
| Plans | `.pi/skills/plans/SKILL.md` |
| Static PSX evidence / runtime missions | `.pi/skills/psx-rizin/SKILL.md` / `.pi/skills/psx-emulator/SKILL.md` |

Read `.pi/skills/bof3-re/references/target-contract.md` before target work.
Keep reference Markdown lowercase kebab-case. Preserve loader `SKILL.md`, root
instruction filenames and exact commands, schemas, constants and protocol literals.

Work directly from the user's request. Use repository-relative commands without `cd`.
Inspect starting dirty work and preserve unrelated changes. Run applicable existing
checks and report failures or unresolved blockers without claiming completion.
Domain-specific evidence and rollback obligations remain intact.

## Delivery focus

Prioritize the requested shipped result. Use existing local builds, static checks,
and focused runtime runs to verify changed behavior. Keep verification proportional
to the change; repeat checks only after relevant changes, failures, or unresolved
concerns. Finish each deliverable through its authorized commit and push.

Disposable verification supports the deliverable; it is not a separate product.
Do not build bespoke proof/grant frameworks, recursive source inventories, or
design/review campaigns around temporary checking scripts without an explicit user
request. Prefer one small check over layers of generators, controllers and receipts.
Preserve authorization, runtime bounds, rollback obligations and truthful evidence.
These focus rules take precedence over broader skill workflow suggestions.

Do not modify installed Pi extensions or add/install dependencies unless the user explicitly authorizes that specific change. Task execution, troubleshooting, and workflow recovery do not imply authorization.

Do not add regression tests unless the user specifically requests expanded test coverage. Running existing checks does not authorize adding tests.

After project agent or skill Markdown edits, compact the changed text directly without weakening its contracts and run applicable existing checks. Do not restore the retired agent-skill-compaction machinery.
