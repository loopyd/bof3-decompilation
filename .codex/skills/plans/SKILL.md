---
name: plans
description: Inspect, reconcile, and consolidate persistent repository plans with reviewed status and recovery safeguards. Use when asked to manage plans, resume planned work, or wipe explicitly named plan files.
---

# Plans

Read [plan authoring](../../../docs/agents/plan-authoring.md) and the selected
plan completely. Canonical Markdown alone owns persistent tasks; reports and
command output are evidence, not task authority.

| Request | Action |
| --- | --- |
| Inventory | `bin/plans list` |
| Resume | `bin/plans status [plan]`; refresh owning evidence before executing authorized ready steps |
| Consolidate | Assess every source line; prepare external candidate and complete hash-bound mapping; independent semantic review; preview; explicit safe apply |
| State correction | Reviewed Markdown edit with attributed evidence; never infer completion from prose PASS, elapsed time, implementation presence, or transport success |

Keep stable IDs, owners, dependencies, blockers, acceptance, provenance and all
incomplete obligations. Historical tests are historical; independent review is a
separate gate. Command validation proves structure/freshness, not semantic parity.
Do not execute a domain backlog merely to manage its plan. Target-qualified
lift/map/layout/duplicate work still requires `$bof3-re` and live evidence.

Before deleting superseded plans, verify external byte/hash/mode recovery copies;
untracked originals may not exist in Git. Require independent candidate/mapping
review before `consolidate --apply`; bind exact reviewed candidate/source hashes.
On stale inputs, failed writes/deletes, or interrupted publication: stop, inspect
the named recovery directory and current files, preserve concurrent edits, obtain
fresh review; never blindly replay or overwrite recovery evidence.

## Explicit-scope wipe only

Require fresh user authorization naming each exact direct-child plan filename.
Validate scope/ancestors as consolidation does, preserve verified external bytes
and hashes first, delete only those named plan files, then repair live references.
No wildcard/recursive wipe, global artifacts, sessions, receipts, `out/`, media,
`/tmp` evidence, recovery directories, or Git writes. Wipe does not complete tasks.
No extra state database, scheduler, aliases, wipe command, or state setter.
