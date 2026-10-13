# AGENTS.md — BOF3

Route each request to its BOF3 owner below. Repository contracts and task-specific
authorization govern scope and acceptance. Skills and their references own the
operative rules. `docs/` is user documentation, not required agent reading.
During authorized maintenance, extract useful knowledge into relevant references.
Caller-selected documents and change artifacts are task inputs, not standing
dependencies.

| Request | Owner |
| --- | --- |
| Lift, match, target/source/map/layout, duplicates | `.pi/skills/bof3-re/SKILL.md` |
| Autonomous campaign or resume | `.pi/skills/bof3-lift-loop/SKILL.md` |
| Naming, identity, source identifiers | `.pi/skills/bof3-naming/SKILL.md` |
| Type representation / macro work | `.pi/skills/bof3-types/SKILL.md` / `.pi/skills/bof3-macros/SKILL.md` |
| Python harness, CLI, skill scripts | `.pi/skills/bof3-re/references/tooling-contract.md` |
| Tests / Markdown | `.pi/skills/bof3-test/SKILL.md` / `.pi/skills/bof3-docs/SKILL.md` |
| Change planning / implementation | `.agents/skills/openspec-propose/SKILL.md` / `.agents/skills/openspec-apply-change/SKILL.md` |
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

## Documentation

Keep README files, guides, and API references focused on current supported behavior,
setup, usage, contracts, and lasting limitations. Include versions and numeric limits
when readers need them to use the product. Prefer a discovery command to a copied
inventory that changes with each release.

Keep session progress, one-off benchmarks, test counts, delivery SHAs, temporary
paths, and implementation diaries out of user-facing documentation. Put run results
in existing task evidence, disposable reports in ignored `out/`, task status in its
owning OpenSpec change, and delivery details in commit or PR descriptions. Old
audit bundles and disposable checks may be removed when authorized. Preserve
current corpus inputs, requested deliverables and existing release history.

Apply `$unslop` to every prose edit, including README files, guides, and agent
instructions. Use plain words and complete sentences. Preserve exact commands,
schemas, protocol literals, licenses, and technical meaning. Before committing,
remove paragraphs that report the editing session rather than help the reader use
or understand the maintained product. Run applicable existing documentation checks.

Do not modify installed Pi extensions or add or install dependencies without
specific user authorization. Task execution, troubleshooting, and workflow
recovery do not imply authorization.

Do not add regression tests unless the user requests expanded test coverage.
Running existing checks does not authorize adding tests.

After project agent or skill Markdown edits, compact the changed text directly
without weakening its contracts and run applicable existing checks. Do not restore
the retired agent-skill-compaction machinery.
