# Documentation operations

`bof3-docs` owns explicitly scoped Markdown reference inspection, search, editing,
repair and compaction, performed with the native read, search and edit tools — the
harness exposes no Markdown transport. Read [docs/INDEX.md](../INDEX.md) first.
Domain facts, plan authority and source acceptance remain with their owners.

## Working scope

Work on existing canonical repository-relative authored `.md` files that the parent
names explicitly; there is no implicit whole-repository scan and no edit by default.
Authored Markdown is what counts as input: `out/`, `build/`, `toolchains/`, `inputs/`,
`tmp/`, `sessions/`, `ledger/`, caches and disposable state are not authored truth,
and within `.pi/` only authored `.pi/agents/` and `.pi/skills/` Markdown is eligible.
`.codex/` and `.agents/` hold only relative discovery symlinks to `.pi/skills/`
owners, never authored bodies. Other authored Markdown, including root and agent
files, is available only when explicitly named; read access is not edit approval.
Preserve unrelated dirty work, and never overwrite a concurrent change.

## Reference inspection

Inspect each named document; for a named directory, walk its Markdown in sorted order
without following symlinks or special files, and treat an empty directory as an empty
inventory. For every link report its containing document, one-based line and column,
kind, raw destination and status. Relative destinations resolve from the **containing
Markdown directory**, not the shell directory; a leading `/PATH` is
repository-root-relative. Parent traversal may stay within the repository, never
escape it. Decode URL escaping and entities; a query token does not change filesystem
lookup. Directories are valid targets.

Check each local target's existence, and each fragment against the target's headings:
ATX and setext headings use lowercase, punctuation-stripped, space-to-hyphen slugs
with duplicate suffixes, and explicit HTML IDs and anchor names are also accepted.
Renderer-specific anchors require manual confirmation. Linked authored Markdown may be
read solely for its anchors. Track the reference count, the broken count and the
unchecked count: broken references fail the inspection, while external URLs are
reported but never fetched and never declared healthy. Unchecked targets prove nothing.

Inline links and images (including linked images, balanced parentheses, angle
destinations and optional titles), reference definitions and full/collapsed/defined-
shortcut references, angle autolinks and HTML `href`/`src` are in scope. Fenced and
indented code, code spans and HTML comments are excluded. Undefined explicit reference
pairs are candidates only: bracketed byte notation can be literal prose, so inspect it
before proposing a repair. Bare paths, code-path mentions, bare URLs, wiki links and
footnotes are not inferred; this is a source inspection, not a complete Markdown
renderer.

Use the locations you found to prepare an authorized repair, then re-run the same
inspection and check the affected indexes. Inspection never rewrites links and grants
no edit authority. Automated link checking retired with the harness Markdown
transport, so link hygiene is the skill's responsibility now.

## Editing and compaction

Find each disputed fact's implementation, configuration, specification or policy
owner before changing it. Preserve historical context; analyzer prose and
disposable outputs are not reviewed truth. Follow [placement and naming](documentation.md#documentation-placement-and-names):
game facts in `docs/specs/`, agent/tooling contracts in `docs/agents/`, scoped work in `docs/plans/`.

Compact by removing redundant wording and linking to a single authority, not by
deleting requirements, evidence gates, exceptions, dependencies, owners, stable
IDs, unresolved work, provenance, commands, links or necessary examples. Read the
whole document first. A smaller byte count alone proves nothing; a justified
no-op is preferable to information loss. Resolve affected links and index entries,
run applicable existing checks and `git diff --check`, and report actual edits,
validation and outstanding questions. Agent/skill Markdown must be compacted
directly without weakening contracts.

Plan consolidation, state transitions, deletion and recovery still follow
[`plans`](../../.pi/skills/plans/SKILL.md). Ordinary Markdown editing does not
authorize those operations, source refactors, invented policy or domain approval.

## Ownership

`$bof3-docs` owns explicitly scoped Markdown work; `harness.context` owns role and
target-qualified context profiles; shared confined reads stay in `harness.common`.
The surviving harness command in this area is the documentation-drift check
(`bin/harness source docs`), which compares tool documentation against code and CLI
evidence — it reports drift, and never rewrites a document.

`bin/harness agent context cleanup docs PATHS...` remains the narrow route limited to
files under `docs/`, `README.md` and `AGENTS.md`; broader requests use the explicit
scope named above. Frozen selections of retired skills reject rather than silently
rebind. No compatibility skill, copied body, generic scheduler or document database is
introduced. Markdown navigation indexes remain the reading maps; installed Pi
extensions and historical logs are not modified.

## Documentation placement and names


| Directory | Owns |
| --- | --- |
| `docs/agents/` | Agent/session policy, harness and CLI contracts, matching and evidence workflows |
| `docs/specs/` | BOF3 game knowledge: runtime behavior, binary formats, layouts and target-specific evidence |
| `docs/plans/` | Active scoped work, dependencies and acceptance status |
| `docs/reference/` | External research and leads, with provenance and authority limits |

Use lowercase kebab-case Markdown filenames; `INDEX.md` and `README.md` are
navigation exceptions. Preserve externally sourced filenames where provenance
requires them. Update both owning and root indexes plus all path references when
moving a document. Split mixed tooling/game topics rather than duplicating owners.
