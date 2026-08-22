# Documentation repair

## Scope and authority

The caller must name existing files under `docs/`, `AGENTS.md`, or `README.md`. Find the owning implementation, configuration, specification, policy, or plan before editing; generated `out/` state and analyzer prose are not authority. Correct or delete dated lift counts, transient rankings, disposable snapshots, duplicate claims, and dead links. Preserve history and changelogs.

Place durable runtime/format facts in `docs/specs/`, agent policy in `docs/agents/`, and scoped work in `docs/plans/`. Do not relocate files, refactor source, invent policy, execute a plan, or turn a docs repair into naming, lifting, matching, or analysis work.

## Procedure

1. Refuse overlap with a modified candidate unless the parent explicitly named that edit.
2. Recursively inspect the caller-named paths and resolve each disputed claim to its direct owner.
3. Change only the stale claim or broken link. Preserve unrelated wording and valid historical context.
4. Resolve every edited link, grep for the stale claim, run focused documentation tests when present, and run `git diff --check`.
5. Report path, owning contract, evidence, smallest repair, validation, and any item requiring human approval. If authority is missing or conflicting, make no speculative edit and name the smallest unblocking decision.
