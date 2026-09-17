# BOF3 documentation index

Start here for every repository request. Read only the route that applies, then
follow links from the selected document. [`AGENTS.md`](../AGENTS.md) is the
minimal entry contract; this index owns the detailed reading map.

## Hard repository contract

- Work only in this repository. Never commit user media from `inputs/`.
- `out/` is disposable working state, never reviewed truth. Do not hand-edit
  `build/`, `toolchains/`, or generated PsyQ bindings.
- BOF3 binaries load independently. Qualify one function as
  `TARGET@0xADDRESS`; shipped EMI entries use
  `BIN/FAMILY/ARCHIVE.EMI#INDEX@0xADDRESS`.
- Original bytes and PS-X headers outrank analyzer output; verify `t_addr`.
- No stage, commit, push, release, or external mutation without approval.
- Complete assigned scope. Stop only on an evidence-backed blocker and report
  what was tried plus the smallest unblocking action.

## Autonomous execution

The full BOF3 reconstruction goal is driven by `bof3-lift-loop` in the active
session, with standing authorization for parent-owned index/analysis refreshes,
reviews and bounded native compile/compare gates through available session tools.
Do not request separate conversational confirmation for these operations; use the
[reviewed native execution route](agents/codex.md#native-compiler-execution).
No harness Codex CLI invocation, model SDK/API launch, detached runner or Pi fallback.
Review payloads may include relevant private
source, diffs, agent instructions, target metadata and original-byte evidence;
exclude credentials and unrelated user media. The parent selects bounded review
scope and execution limits before launch. Keep original pins, consumed budgets,
independent semantic review and live acceptance gates; never refresh inputs during
a pinned transaction. Preserve frozen evidence before replacing working indexes.
Local feature-completion commits are authorized; do not push or release. Standing
authorization does not override sandbox decisions or authorize dependency installs.

## Request routes

| Request | Read first | Then |
| --- | --- | --- |
| One hand-guided target-qualified lift, match, duplicate normalization, source/map/Splat edit | [Function matching](agents/matching.md) | [Matching playbook](agents/matching-playbook.md), [Memory API](agents/memory-api.md), `$bof3-re` |
| Concrete Rizin analyzer question | [Project context](agents/project-context.md) | `$psx-rizin` |
| Tooling, Python harness, CLI, tests | [Coding standards](agents/coding-standards.md) | [Harness ownership](agents/harness.md), [Tool usage](agents/tool-usage.md) and owning code/tests |
| Codex MCP or Pi extension migration | [Codex configuration](agents/codex.md) | Global settings stay in `~/.codex`; project skills below |
| Autonomous lift/naming/type/macro loop or resume | [`$bof3-lift-loop`](../.codex/skills/bof3-lift-loop/SKILL.md) | Active-session mission protocol; parallel independent reads/reviews, serialized writes; no harness model launcher |
| Parent pre-edit native lift diagnosis / retained candidate audit | [Parent lift diagnosis](agents/codex.md#parent-lift-diagnosis) | [Retained audit](agents/codex.md#retained-lift-audit), `$bof3-re`; original pins/cutoff, no model or source edits |
| Scoped worker or independent review mission | [Skill-only operator](agents/codex.md#skill-only-operator) | `$bof3-lift-loop` delegates `$bof3-re` or the selected domain skill through actual session tools; no self-acceptance |
| Macro opportunity indexing, ranking or resolution | [Macro resolution guide](agents/macros.md) | [Explicit cleanup routing](agents/tool-usage.md#cleanup-opportunity-routing), owning code/checks; matching or tooling route when making changes |
| Multi-function C metadata or function consolidation | [Combiner](agents/combiner.md) | [Active rollout](plans/autonomous-bof3-decompilation.md#multi-function-consolidation); tooling/matching owners; production consolidation remains gated |
| Type representation opportunities and reviewed application | [Tool usage](agents/tool-usage.md#3b-target-analysis-freshness--rebuild--query) | [Cleanup routing](agents/tool-usage.md#cleanup-opportunity-routing), `$bof3-types`; [harness ownership](agents/harness.md) |
| Symbol naming opportunity discovery or assessment | [Naming opportunities](agents/tool-usage.md#symbol-naming-opportunities) | `$bof3-naming`; [harness ownership](agents/harness.md); evidence and identity routes remain separate |
| Plan creation, management or execution | [Plan authoring](agents/plan-authoring.md) | `$plans`; selected file under `plans/`; refresh live evidence |
| Symbol/type/metadata identity maintenance | [Project context](agents/project-context.md) | `$bof3-naming` transaction mode |
| Naming evidence/audit closure | [Naming audit contract](../.codex/skills/bof3-naming/references/NAMING_AUDIT_V3.md) | `$bof3-naming` audit mode |
| Markdown references, search, context, aggregation, edit, repair or compaction | [Documentation operations](agents/documentation.md) | `$bof3-docs`; [reference inspection](agents/documentation.md#reference-inspection), owning implementation/policy source before edits |
| Runtime, format, target, or data research | [Specifications index](specs/INDEX.md) | Relevant spec and original evidence |
| Repository overview / contributor onboarding | [README](../README.md) | [Contributing](../CONTRIBUTING.md), [Tool usage](agents/tool-usage.md) |

## Ownership

| Fact | Owner |
| --- | --- |
| Binary identity and load address | `config/targets/<target>/target.toml` |
| Reviewed layout | `config/targets/<target>/splat.yaml` |
| Shared Splat execution options | `config/splat.yaml` |
| Target-local symbols | `config/targets/<target>/symbols.txt` |
| Shared SDK symbol maps | `config/sdk/psyq-{slus,logo}.txt` |
| Reviewed Rizin annotations | `config/targets/<target>/reviewed.rz` |
| Authored lifts | Metadata-resolved source under `src/bof3/` |
| Macro opportunity policy and tool reference | `docs/agents/macros.md` |

Until the [multi-function rollout](agents/combiner.md) clears registry and native
gates, keep new production lifts one function per C source; inspection and read-only
source/index ownership support attached per-function records. Existing grouped
sources are not grandfathered into native or transaction acceptance. This is a
staged migration, not a permanent ban on
cohesive multi-function files. Lift identity comes from manifest claims,
maps, Splat, and function-level `@source`/`@behavior` metadata—not filenames or
directory ancestry. Maps use sorted `name = 0xADDRESS;` rows with uppercase
addresses. Bind target symbols through plain declarations plus sanctioned
`WEAK_SYMBOL_AT` entries; keep PsyQ external. See the matching and project
context docs for the complete source/symbol contract.

## Codex skills

Project definitions live in `.codex/skills/`; `.agents/skills/` contains relative
discovery symlinks to those same owners, not copied skill bodies. Codex discovers
repository skills through `.agents/skills`; see the
[official skill documentation](https://developers.openai.com/codex/skills/).
Restart the session to refresh discovery after migration. `.pi/agents/` retains
only the lifter, namer, cleaner and reviewer definitions used by bounded harness
prefills; generic roles and the old Pi chain are retired. These files are not a
Codex scheduler or installed extension.

| Skill | Scope |
| --- | --- |
| [`$bof3-lift-loop`](../.codex/skills/bof3-lift-loop/SKILL.md) | instruction-only active-session operator, bounded missions, parallel evidence/review and serialized domain writes |
| [`$bof3-re`](../.codex/skills/bof3-re/SKILL.md) | target-qualified lifting and independent review |
| [`$bof3-macros`](../.codex/skills/bof3-macros/SKILL.md) | macro opportunities, human-value ranking and reviewed resolution |
| [`$bof3-types`](../.codex/skills/bof3-types/SKILL.md) | established C types, representation opportunities and reviewed application |
| [`$bof3-naming`](../.codex/skills/bof3-naming/SKILL.md) | naming opportunities, evidence, audits and reviewed identity transactions |
| [`$plans`](../.codex/skills/plans/SKILL.md) | persistent plan management |
| [`$psx-rizin`](../.codex/skills/psx-rizin/SKILL.md) | concrete analyzer questions |
| [`$bof3-docs`](../.codex/skills/bof3-docs/SKILL.md) | scoped Markdown reference inspection, search, context, aggregation, editing, repair and one-document compaction |

Macro, type, naming and documentation skills dispatch invocation scripts to `bin/`;
policy, parsing and editing remain in the harness owners. Naming, documentation
and Rizin skills allow automatic invocation for relevant work without renewed user
approval. The parent supplies bounded modes and inputs; discovery never expands
mutation authority or bypasses sandbox, evidence, review or rollback gates.

## Validation gates

- `bin/asm-diff` proves instruction equivalence; `bin/byte-match` proves bytes.
- Run `bin/symbols check` after map edits; normalize with
  `bin/symbols normalize [TARGET] --write` when needed.
- `bin/decomp-status [TARGET...]` is the live lift audit.
- Run focused tests for changed behavior and `just check` before handoff when
  practical. List every skipped check and residual risk.
- Tests assert behavior or parsed structure, never literal wording from agent,
  skill, prompt, or workflow Markdown.

## Source and duplicate rules

Write readable C89. `REGISTER_PIN`, direct asm register bindings, `CLOBBER_*`,
`barrier()` and all artificial empty-asm matching barriers are banned, including
aliases and declaration-only/no-op compatibility shims. This user-directed ban
supersedes every earlier pin/clobber exception, including historical plans and
previously exact aided results. Remove the aids and requeue every affected source
for fresh clean-C native matching and independent review; old scores and receipts
remain historical evidence, not current acceptance.

Preserve manifest-owned `WEAK_SYMBOL_AT` address-binding assembly, original
disassembly and provenance. Ordinary `NO_SIBLING_CALLS` compiler attributes remain
allowed in `include/base/compiler.h`, replacing `include/base/barrier.h`.
Other handwritten inline assembly remains banned; `INCLUDE_ASM` still requires
explicit approval and is not a substitute for this clean-C requeue. Opaque clean-C
matching shapes require adjacent rationale, live byte evidence and independent review.

Treat `(analyzer-range SHA-256, size)` as a reuse candidate, not shared
ownership. Match one representative, then independently port and validate a
second member. Promote only a worthwhile common body after two cross-target
members independently byte-match with the same C shape; each target retains its
own wrapper, declarations, map, layout, and validation.

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

## Documentation map

- [Agent and tooling index](agents/INDEX.md)

- [Agent coding standards](agents/coding-standards.md)
- [Project context and repository map](agents/project-context.md)
- [Function matching](agents/matching.md)
- [Matching playbook](agents/matching-playbook.md)
- [Memory API](agents/memory-api.md)
- [Plan authoring](agents/plan-authoring.md)
- [Lessons](agents/lessons.md)
- [Specifications index](specs/INDEX.md)
- [Macro opportunity indexing and resolution](agents/macros.md)
- [Python harness ownership](agents/harness.md)
- [Documentation operations](agents/documentation.md)
- [Tool usage](agents/tool-usage.md)
- [External EU reference](reference/bof3-eu/README.md) — leads only; EU
  addresses are not reviewed US facts

## Header layout

| Directory | Domain |
| --- | --- |
| `include/base/`, `include/memory/` | common types, compiler attributes, fixed RAM and hardware addresses |
| `include/gpu/`, `include/frontend/`, `include/callback/` | rendering, frontend, callbacks |
| `include/loader/`, `include/panel/`, `include/battle/` | loader, panel, battle runtime |
| `include/data/`, `include/media/`, `include/ui/`, `include/game/` | records, media, UI and game templates |

Completed implementation history is in `git log`; current scoped work belongs
under `docs/plans/` only while active.
