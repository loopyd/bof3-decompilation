---
name: bof3-docs
description: Locate the smallest authoritative BOF3 repository document. Use for documentation lookup, repository orientation, setup instructions, workflow references, specifications, generated-artifact ownership, or troubleshooting routes; use a domain skill to interpret the retrieved facts.
---

# BOF3 project documentation

Load only the documents needed for the question. Prefer an index first when the
owning concept is unclear; do not preload the documentation tree.

## Routing workflow

1. Classify the question with the table below.
2. Read the primary route. Follow one linked concept when it owns the answer.
3. If no route fits, search headings and frontmatter descriptions with
   `rg -n '<term>' docs CONTEXT.md README.md`, then open only the best matches.

## Routes

| Need | Read first | Add only when needed |
| --- | --- | --- |
| Documentation map | `docs/README.md` | — |
| Repository quick start | `README.md` | `docs/setup.md` |
| Binary, archive, EMI-entry, or target identity | `CONTEXT.md` | `docs/specs/runtime/runtime-layout.md` |
| Setup, disc input, toolchains, PsyQ SDK | `docs/setup.md` | `docs/tools.md` |
| Build, extraction, doctor, diff, or analysis failure | `docs/troubleshooting.md` | The workflow page named by the failure |
| Classify, promote, inspect, or lift code | `docs/reverse-engineering.md` | `docs/specs/programs/targets.md` |
| Match a lifted function | `docs/matching.md` | `docs/tools.md` |
| Tool roles and evidence authority | `docs/tools.md` | `docs/artifacts.md` |
| Generated versus tracked output ownership | `docs/artifacts.md` | `AGENTS.md` |
| Verified reverse-engineering knowledge | `docs/specs/index.md` | The owning section index below |
| EMI, graphics, STR, or XA format | `docs/specs/formats/index.md` | The linked format concept |
| Runtime loading, EMI dispatch, frontend flow, or memory layout | `docs/specs/runtime/index.md` | The linked runtime concept |
| Executables and confirmed overlays | `docs/specs/programs/index.md` | `docs/specs/programs/targets.md` |
| Archive families, ownership, or duplicate data | `docs/specs/archives/index.md` | The linked archive concept |
| IDs, encodings, records, tables, characters, equipment, or area data | `docs/specs/data/index.md` | The linked domain concept |
| Field-semantic confidence or unresolved data work | `docs/specs/data/schema-ledger.md` | The owning data concept |
| Discovery or verification procedure | `docs/specs/methods/index.md` | The linked method concept |
| Algorithm-level recovered behavior | `docs/specs/pseudocode.md` | The owning format/runtime/data spec |

## Reading discipline

- Prefer the nearest `index.md` over enumerating its directory; indexes carry
  current concept descriptions and links.
- Read long ledgers or pseudocode selectively by heading or `rg`, not in full,
  unless the task spans the whole document.
- Follow direct Markdown links instead of guessing filenames.
- Report the documents and evidence actually inspected; state skipped checks.
