# BOF3 documentation index

User-facing documentation map. Agent entry routing lives in
[`AGENTS.md`](../AGENTS.md); operative protocols live in skill references, not this
index. Guides under [`docs/agents/`](agents/INDEX.md) explain tooling and workflows;
[`docs/specs/`](specs/INDEX.md) holds game specifications. Agents may use explicitly
scoped documentation as maintenance/research input, not required runtime reading.

## Request routes


| Request | Read first | Then |
| --- | --- | --- |
| One hand-guided target-qualified lift, match, duplicate normalization, source/map/Splat edit | [Function matching](agents/matching.md) | [Matching playbook](agents/matching-playbook.md), [Memory API](agents/memory-api.md), `$bof3-re` |
| Concrete Rizin analyzer question | [Project context](agents/project-context.md) | `$psx-rizin` |
| PS1 emulator commands, breakpoints or captures | [`$psx-emulator`](../.agents/skills/psx-emulator/SKILL.md) | Harness `runtime`; [action catalog](../.agents/skills/psx-emulator/references/catalog.md) for mission discovery; [emulator integration](agents/pcsx-redux-agentic.md); `psx-rizin` for static interpretation |
| Function/global index conflict or isolated `textbin` probe | [Boundary evidence](agents/boundary-evidence.md) | Tooling route for implementation; matching route before target edits |
| Area dialogue text extraction, editing or repacking | [Dialogue text](specs/formats/dialogue-text.md) | [Tool usage](agents/tool-usage.md#area-text); binary-format evidence only, no runtime acceptance |
| Tooling, Python harness, CLI, tests | [Coding standards](agents/coding-standards.md) | [Harness ownership](agents/harness.md), [Tool usage](agents/tool-usage.md) and owning code/tests |
| Test-unit organization, scoping or optimization | [Tests](agents/coding-standards.md#tests) | `$bof3-test`; [unit map](../.agents/skills/bof3-test/references/unit-map.md); `just check` (scoped) / `just check-all` / `just check-unit` |
| Codex MCP or Pi extension migration | [Codex configuration](agents/codex.md) | Global settings stay in `~/.codex`; project skills below |
| Autonomous lift/naming/type/macro loop or resume | [`$bof3-lift-loop`](../.agents/skills/bof3-lift-loop/SKILL.md) | Active-session mission protocol; parallel independent reads/reviews, serialized writes; no harness model launcher |
| Parent pre-edit native lift diagnosis / retained candidate audit | [Parent lift diagnosis](agents/codex.md#parent-lift-diagnosis) | [Retained audit](agents/codex.md#retained-lift-audit), `$bof3-re`; original pins/cutoff, no model or source edits |
| Scoped worker or independent review mission | [Skill-only operator](agents/codex.md#skill-only-operator) | `$bof3-lift-loop` delegates `$bof3-re` or the selected domain skill through actual session tools; no self-acceptance |
| Macro opportunity indexing, ranking or resolution | [Macro resolution guide](agents/macros.md) | [Explicit cleanup routing](agents/tool-usage.md#cleanup-opportunity-routing), owning code/checks; matching or tooling route when making changes |
| Multi-function C metadata or function consolidation | [Combiner](agents/combiner.md) | Tooling/matching owners; production consolidation remains gated; OpenSpec records remaining work |
| Type representation opportunities and reviewed application | [Tool usage](agents/tool-usage.md#3b-target-analysis-freshness--rebuild--query) | [Cleanup routing](agents/tool-usage.md#cleanup-opportunity-routing), `$bof3-types`; [harness ownership](agents/harness.md) |
| Symbol naming opportunity discovery or assessment | [Naming opportunities](agents/tool-usage.md#symbol-naming-opportunities) | `$bof3-naming`; [harness ownership](agents/harness.md); evidence and identity routes remain separate |
| Function-argument or local-variable naming | [Source identifier naming](agents/tool-usage.md#source-identifier-naming) | `$bof3-naming`; [harness ownership](agents/harness.md); PRE-bound, single-function, native byte-identity gate |
| Change planning or implementation | `openspec list` | `$openspec-propose`, `$openspec-update-change` or `$openspec-apply-change`; refresh live evidence before implementation |
| Skill performance audit, converged directives, bundled harness improvements | [Observation folder](observations/INDEX.md) | The owning skill's ledger; directives stay in `.agents/skills/<skill>/` and its agent instructions |
| Symbol/type/metadata identity maintenance | [Project context](agents/project-context.md) | `$bof3-naming` transaction mode |
| Naming evidence/audit closure | [Naming audit contract](../.agents/skills/bof3-naming/references/naming-audit-v3.md) | `$bof3-naming` audit mode |
| Markdown references, search, context, aggregation, edit, repair or compaction | [Documentation operations](agents/documentation.md) | `$bof3-docs`; [reference inspection](agents/documentation.md#reference-inspection), owning implementation/policy source before edits |
| Runtime, format, target, or data research | [Specifications index](specs/INDEX.md) | Relevant spec and original evidence |
| BOF3JS EU game knowledge, format or behavior leads | [EU knowledgebase](specs/bof3-eu/README.md) | Relevant chapter; verify regional differences against original US evidence |
| Repository overview / contributor onboarding | [README](../README.md) | [Contributing](../CONTRIBUTING.md), [Tool usage](agents/tool-usage.md) |

## Contracts and directives

| Contract | Owner |
| --- | --- |
| Hard repository contract | [Project context](agents/project-context.md#hard-repository-contract) |
| Autonomous execution | [Codex](agents/codex.md#autonomous-execution) |
| Ownership and repository layout | [Project context](agents/project-context.md#ownership) |
| Header layout | [Project context](agents/project-context.md#header-layout) |
| Validation gates | [Harness](agents/harness.md#validation-gates) |
| Source and duplicate rules | [Function matching](agents/matching.md#source-and-duplicate-rules) |
| Documentation placement and names | [Documentation](agents/documentation.md#documentation-placement-and-names) |
| Codex skills and dispatch | [Codex](agents/codex.md#codex-skills) |

## Codex skills


| Skill | Scope |
| --- | --- |
| [`$bof3-lift-loop`](../.agents/skills/bof3-lift-loop/SKILL.md) | instruction-only active-session operator, bounded missions, parallel evidence/review and serialized domain writes |
| [`$bof3-re`](../.agents/skills/bof3-re/SKILL.md) | target-qualified lifting and independent review |
| [`$bof3-macros`](../.agents/skills/bof3-macros/SKILL.md) | macro opportunities, human-value ranking and reviewed resolution |
| [`$bof3-types`](../.agents/skills/bof3-types/SKILL.md) | established C types, representation opportunities and reviewed application |
| [`$bof3-naming`](../.agents/skills/bof3-naming/SKILL.md) | naming opportunities, evidence, audits, source-identifier naming and reviewed identity transactions |
| [`$bof3-test`](../.agents/skills/bof3-test/SKILL.md) | per-module pytest units, scoped runs and reviewed test-suite optimization |
| `$openspec-propose`, `$openspec-update-change`, `$openspec-apply-change` | change planning, revision and implementation |
| [`$psx-rizin`](../.agents/skills/psx-rizin/SKILL.md) | concrete analyzer questions |
| [`$psx-emulator`](../.agents/skills/psx-emulator/SKILL.md) | bounded PCSX-Redux Agentic missions and independent captures |
| [`$bof3-docs`](../.agents/skills/bof3-docs/SKILL.md) | scoped Markdown reference inspection, search, context, aggregation, editing, repair and one-document compaction |

## Documentation map


- [Agent and tooling index](agents/INDEX.md)

- [Agent coding standards](agents/coding-standards.md)
- [Project context and repository map](agents/project-context.md)
- [Function matching](agents/matching.md)
- [Matching playbook](agents/matching-playbook.md)
- [Memory API](agents/memory-api.md)
- [Lessons](agents/lessons.md)
- [Specifications index](specs/INDEX.md)
- [External research references](reference/INDEX.md)
- [AREA016 graphics initialization](specs/runtime/area016-graphics-initialization.md)
- [Macro opportunity indexing and resolution](agents/macros.md)
- [Python harness ownership](agents/harness.md)
- [Documentation operations](agents/documentation.md)
- [Tool usage](agents/tool-usage.md)
- [BOF3JS EU knowledgebase](specs/bof3-eu/README.md) — leads only; EU
  addresses are not reviewed US facts
