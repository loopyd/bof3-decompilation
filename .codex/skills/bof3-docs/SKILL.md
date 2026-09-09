---
name: bof3-docs
description: Search, assemble context, aggregate, edit, repair or compact explicitly scoped BOF3 Markdown. Own documentation operations without granting source edits, policy invention, plan execution or semantic acceptance.
---

# BOF3 documentation

Start at [docs/INDEX.md](../../../docs/INDEX.md), then the
[documentation command contract](../../../docs/specs/DOCS.md). Use one explicit
mode and caller-named paths; do not scan or edit the whole repository by default.

| Mode | Action |
|---|---|
| `context` | Read numbered excerpts with full-file hashes; partial excerpts are not complete authority |
| `search` | Find literal claims and links within the named Markdown scope |
| `aggregate` | Collect complete documents with provenance; merging or deleting originals needs separate explicit scope |
| `edit` | Make the requested Markdown change after checking its owning facts and current input hashes |
| `repair` | Apply the [repair contract](references/DOCUMENTATION_REPAIR.md); preserve unrelated wording and history |
| `compact PATH` | Read one complete document, then remove redundancy without weakening its contracts |

`bin/docs MODE ...` and `sh .codex/skills/bof3-docs/scripts/docs.sh MODE ...`
are read-only inspection/preparation tools. Edit/repair/compact return full pinned
inputs, not completed rewrites. Retain their hashes, check for drift immediately
before `apply_patch`, and edit only the user-authorized paths. A stale pin stops;
never silently repin or overwrite concurrent work. CLI output grants no writes.

For compaction, preserve every requirement, evidence gate, exception, dependency,
owner, stable ID, unresolved obligation, provenance, command, link and necessary
example. Prefer concise wording and one authoritative statement with links over
duplicate paragraphs. Do not turn historical or blocked work into completion.
If meaningful reduction would lose information, report a justified no-op.

Plan consolidation/status/recovery still requires `$plans`; domain facts stay
with their source owners. Explicit Markdown scope does not authorize semantic
source edits, policy changes or deletion. Resolve edited links/anchors and affected
indexes, run applicable existing checks and `git diff --check`, and report actual
changes and gaps. Compact changed agent/skill text directly; never restore the
retired automatic agent-skill-compaction machinery.
