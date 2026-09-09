# AGENTS.md — BOF3

Start every request at [`docs/INDEX.md`](docs/INDEX.md) and follow its
request-oriented reading map. That index owns the canonical reading order,
repository contracts, skills, evidence gates, validation, and
planning guidance; do not preload unrelated documentation.

Work directly from the user's request. Use repository-relative commands without `cd`.
Inspect starting dirty work and preserve unrelated changes. Run applicable existing
checks and report failures or unresolved blockers without claiming completion.
Domain-specific evidence and rollback obligations remain intact.

Do not modify installed Pi extensions or add/install dependencies unless the user explicitly authorizes that specific change. Task execution, troubleshooting, and workflow recovery do not imply authorization.

Do not add regression tests unless the user specifically requests expanded test coverage. Running existing checks does not authorize adding tests.

After project agent or skill Markdown edits, compact the changed text directly without weakening its contracts and run applicable existing checks. Do not restore the retired agent-skill-compaction machinery.
