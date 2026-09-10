# Documentation operations

`bof3-docs` owns explicitly scoped Markdown references, search, context, aggregation,
editing, repair and compaction. It replaces the documentation-repair skill; context is a
subcommand, not a separate context-builder role. Read [docs/INDEX.md](../INDEX.md)
first. Domain facts, plan authority and source acceptance remain with their owners.

## Commands

```sh
bin/docs refs docs/agents/macros.md
bin/docs refs docs .codex/skills --broken-only --max-bytes 262144
bin/docs context docs/agents/macros.md --start-line 1 --lines 60
bin/docs search --ignore-case --limit 20 'transaction' docs/agents/harness.md
bin/docs aggregate docs/agents/harness.md docs/agents/coding-standards.md --max-bytes 65536
bin/docs edit docs/INDEX.md
bin/docs repair docs/agents/tool-usage.md --max-bytes 262144
bin/docs compact docs/agents/documentation.md
bin/docs compact docs/agents/documentation.md --expected-sha256 RETAINED_SHA256
```

All commands are **read-only**. Edit, repair and compact prepare complete document
snapshots for the skill to act on; they neither invoke a model nor rewrite files.
`compact` takes exactly one target. The agent performs an explicitly authorized
edit with `apply_patch`, then checks semantic preservation and links. No successful
preparation is reported as completed compaction. This is not the retired automatic
agent-skill-compaction machinery, a background trigger or a compaction dependency.

The wrapper `sh .codex/skills/bof3-docs/scripts/docs.sh ...` forwards arguments
unchanged to `bin/docs`. `--root PATH` is the shared global flag, before the
subcommand. The CLI never chooses a larger scope or writes an output file itself.

## Scope and output

Supply existing canonical repository-relative `.md` files, not globs, symlinks
or duplicate paths. Only `refs` also accepts directories. Caller order is retained. The CLI excludes
`out/`, `build/`, `toolchains/`, `inputs/`, `tmp/`, `.git/`, `.venv/`, `.agents/`,
`sessions/`, `ledger/`, `.pi-subagents/`, `.cache/`, `.uv-cache/`, `dist/`, `assets/`
and root `session-*.md` artifacts. Within `.pi/`, only authored `.pi/agents/`
Markdown is eligible; within `.codex/`, only `.codex/skills/` is eligible. This
excludes runtime subagents, caches, retired skills and sessions rather than treating
their Markdown as authored input. Use canonical `.codex/skills/` definitions,
not their discovery aliases. Other authored Markdown, including root and agent
files, is available only when explicitly named; read access is not edit approval.

JSON uses `bof3.docs/v1`, names its `operation` and canonical absolute `root`, and reports
`write_authorized:false`. Each document has its path, SHA-256 of original bytes,
original byte count and total line count. Context includes one-based start/end
lines, excerpt text and `partial`; an empty document has range 1–0. Other document
modes except search/refs include full text. Snapshots are independent non-atomic observations, not
authenticated scope, semantic authority or an atomic multi-file transaction.

Context defaults to the first 80 lines per file; positive `--start-line` and
`--lines` select another range. It does not interpret headings or recursively
load linked documents. Search is literal, case-sensitive unless `--ignore-case`,
and counts matching lines, not substring occurrences. Its positive `--limit`
defaults to 20 across the ordered scope; `matching_lines` and `truncated` disclose
omitted matches. Search metadata retains full-file hashes without full bodies.
For more complex searches, use `rg` on the same explicitly named scope.

Every command caps serialized UTF-8 stdout at `--max-bytes`, default 16,384;
accepted bounds are 512 through 1,048,576. Oversized output fails before printing,
never silently truncates an edit/compaction input. Narrow context/search or
explicitly raise the bound. `aggregate` collects complete documents with separate
provenance; it does not deduplicate, merge or delete them. A semantic consolidation
requires named source/destination authority and appropriate owner review.

Edit, repair and compact accept `--expected-sha256` for one file. Retain the
original external pin; mismatched or malformed pins fail. Without a pin, the
result is current scouting/preparation only. Before editing, independently check
the full current document against the retained hash and stop on drift; the CLI
does not lock files or make subsequent edits atomic. Preserve unrelated dirty work.

## Reference inspection

`refs FILE_OR_DIRECTORY...` recursively inventories Markdown in each named directory,
sorting children and deduplicating overlapping scopes. There is no implicit root
scan. Excluded paths and discovered symlinks/special files are listed in `skipped`,
never followed; explicitly naming them rejects. Empty directories return an empty
inventory. Other commands retain file-only scope.

Each `references` row reports its containing `path`, one-based `line`/`column`,
`kind`, raw `destination`, and `status`. Local rows include `resolved_path`, decoded
`fragment` and `query`; reference-style links also name their label and definition
line. Relative destinations resolve from the **containing Markdown directory**,
not the shell directory; `/PATH` means repository-root-relative. Parent traversal
may stay within the repository, never escape it. URL escaping and entities are
decoded; queries do not change filesystem lookup. Directories are valid targets.

The scanner handles inline links/images (including linked images, balanced
parentheses, angle destinations and optional titles), reference definitions and
full/collapsed/defined-shortcut references, angle autolinks, and HTML `href`/`src`.
Fenced/indented code, code spans and HTML comments are excluded. Undefined explicit
reference pairs are reported as `undefined-reference` candidates: bracketed byte
notation can be literal prose, so inspect it before proposing a repair. Bare paths,
code-path mentions, bare URLs, wiki links, footnotes and renderer extensions are not inferred;
this is a source scanner, not a complete Markdown renderer.

Local existence and authored Markdown fragments are checked. ATX/setext headings
use lowercase, punctuation-stripped, space-to-hyphen slugs with duplicate suffixes;
explicit HTML IDs and anchor names are also accepted. Renderer-specific anchors
require manual confirmation. Linked authored Markdown may be read solely for its
anchors, with an invocation-local cache; its outbound links are not added to scope.
Other fragments, unreadable targets and symlinks remain explicitly unchecked.
External URLs are displayed as `external`, never fetched or declared healthy.

`reference_count`, `status_counts`, `broken_count` and `unchecked_count` cover the
whole inventory. `--broken-only` filters rows, not counts or document hashes; use
the unfiltered view to inspect unchecked targets. Exit **1** means broken references
or undefined-reference candidates, **2** means scope/output/argument failure, and
**0** means no detected breakage—not verification of external or unchecked targets.
Output retains the shared byte cap and fails before printing rather than silently
omitting references. Narrow the directory or raise the cap explicitly.

Use row locations to prepare an authorized `apply_patch` repair, then rerun the same
scope and check affected indexes. `refs` never rewrites links or grants edit authority.

## Editing and compaction

Find each disputed fact's implementation, configuration, specification or policy
owner before changing it. Preserve historical context; analyzer prose and
disposable outputs are not reviewed truth. Follow [placement and naming](../INDEX.md#documentation-placement-and-names):
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
[`plans`](../../.codex/skills/plans/SKILL.md). Ordinary Markdown editing does not
authorize those operations, source refactors, invented policy or domain approval.

## Migration and ownership

`bin/agent-context cleanup docs PATHS...` now selects `bof3-docs` and its repair
reference. Its existing narrow scope—files under `docs/`, `README.md`, `AGENTS.md`—
is unchanged; broader requests use the explicit docs command/skill scope.
Frozen selections of the retired skill reject, rather than silently rebind.
`bin/agent-context context-builder` is retired; use `bin/docs context PATHS...`.
Other role and target-qualified context profiles remain in `harness.context`.

`harness.docs.cli` owns command parsing; `docs.documents` owns snapshots, context,
literal search and edit preparation; `docs.paths` owns file/directory scope and the
legacy cleanup contract. `docs.references` owns resolution and reporting,
`docs.markdown` link extraction and `docs.anchors` fragment inventory.
Shared confined reads stay in `harness.common`.
No compatibility skill, copied body, generic scheduler or document database is
introduced. Markdown navigation indexes remain the reading maps; installed Pi
extensions and historical logs are not modified.
