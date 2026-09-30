# Document contract

## Scope

Caller names authored repository-relative Markdown; no implicit whole-tree scan or
edit. Within `.pi/`, only authored agents/skills qualify. Discovery symlinks carry
no copied bodies. Exclude generated output, builds, toolchains, inputs, scratch,
sessions, ledgers and caches as authored truth. Other root files require explicit
scope. Read access grants no edit authority. Preserve concurrent and dirty work.

Skills obtain operative rules from their entry and bundled references, not user
documentation or named plans. During authorized skill maintenance, extract durable
knowledge from supplied documentation into relevant references; preserve evidence
limits. Documentation is source material, not a runtime reading dependency.

## Reference inspection

1. Inspect named files; walk named directories in sorted Markdown order without
   following symlinks/special files. Empty directory yields empty inventory.
2. Report containing file, one-based line/column, kind, raw destination and status
   for each link. Resolve relative paths from containing Markdown directory;
   leading `/` means repository root. Decode URL escapes/entities; ignore query
   tokens for filesystem lookup. Traversal may not escape repository. Directories
   are valid targets.
3. Check target existence and fragments. ATX/setext slugs use lowercase,
   punctuation removal, space-to-hyphen conversion and duplicate suffixes. Accept
   explicit HTML IDs/anchor names; confirm renderer-specific anchors manually.
   Read linked authored Markdown only for needed anchors.
4. Include inline links/images, linked images, balanced parentheses, angle
   destinations/titles, reference definitions, full/collapsed/defined-shortcut
   references, angle autolinks and HTML `href`/`src`. Exclude fenced/indented code,
   code spans and HTML comments. Inspect undefined explicit pairs before repair;
   bracketed byte notation may be literal. Bare paths/URLs, wiki links and
   footnotes are not inferred.
5. Count references, broken and unchecked targets separately. Broken links fail.
   Report external URLs without fetching or declaring them healthy. Unchecked
   means no proof. Inspection never rewrites links or grants repair authority.

## Placement and compaction

User-facing game knowledge belongs in specifications, user-facing tooling guides
in agent documentation, scoped work in caller-selected planning files, external
research with provenance and authority limits. Operative skill instructions and
reusable knowledge belong in the relevant skill's `references/`.

Use lowercase kebab-case Markdown reference filenames and directories. Keep loader
entry `SKILL.md`, navigation exceptions `INDEX.md`/`README.md`, externally sourced
provenance names and exact code/schema literals unchanged. Rename only within
approved scope; update all live path consumers and affected indexes together.

Read entire document, find disputed facts' implementation/configuration/policy
owner, then compact redundancy without losing rules, gates, exceptions, owners,
stable IDs, unresolved work, history, provenance, commands or needed examples.
Keep mandatory steps visible; disclose branch details behind explicit read-before
links. No-op beats information loss. Byte reduction is not semantic parity.

Validate edited links/anchors, affected indexes and applicable existing checks;
run scoped `git diff --check`. Record paths, changes, failures and remaining gaps.
No semantic source approval, plan transition, deletion or source refactor follows.
Plan lifecycle belongs to `$plans` with caller-supplied inputs. No retired Markdown
transport or agent-skill-compaction machinery. `bin/harness source docs` reports
drift only; native tools own Markdown work.

## Measurement

Return mission/tool identity, tool count, wall time, method, validation, rework and
accepted output. Separate provider startup failure from document quality. Compare
complete contracts, not retained keywords or line counts. Search explicit live-owner
inventory; capped output never proves absence. Keep historical evidence distinct.
Caller may archive measurements; skill execution does not read observation ledgers.
