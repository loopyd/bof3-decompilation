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

## Request routes

| Request | Read first | Then |
| --- | --- | --- |
| One hand-guided target-qualified lift, match, duplicate normalization, source/map/Splat edit | [Function matching](agents/matching.md) | [Matching playbook](agents/matching-playbook.md), [Memory API](agents/memory-api.md), `$bof3-re` |
| Generic analyzer work explicitly requesting Rizin | [Project context](agents/project-context.md) | `$psx-rizin` |
| Tooling, Python harness, CLI, tests | [Coding standards](agents/CODING_STANDARDS.md) | [Harness ownership](specs/HARNESS.md), [Tool usage](usage.md) and owning code/tests |
| Codex MCP or Pi extension migration | [Codex configuration](specs/CODEX.md) | Global settings stay in `~/.codex`; project skills below |
| Macro opportunity indexing, ranking or resolution | [Macro specification](specs/MACROS.md) | [Explicit cleanup routing](usage.md#cleanup-opportunity-routing), owning code/checks; matching or tooling route when making changes |
| Type representation opportunities and reviewed application | [Tool usage](usage.md#3b-target-analysis-freshness--rebuild--query) | [Cleanup routing](usage.md#cleanup-opportunity-routing), `$bof3-types`; [harness ownership](specs/HARNESS.md) |
| Symbol naming opportunity discovery or assessment | [Naming opportunities](usage.md#symbol-naming-opportunities) | `$bof3-naming`; [harness ownership](specs/HARNESS.md); evidence and identity routes remain separate |
| Plan creation, management or execution | [Plan authoring](agents/plan-authoring.md) | `$plans`; selected file under `plans/`; refresh live evidence |
| Symbol/type/metadata identity maintenance | [Project context](agents/project-context.md) | Explicit `$bof3-naming` transaction mode only |
| Naming evidence/audit closure | [Naming audit contract](../.codex/skills/bof3-naming/references/NAMING_AUDIT_V3.md) | Explicit `$bof3-naming` audit mode only |
| Documentation-only repair | Owning implementation/policy source | `$repo-documentation-repair` |
| Runtime, format, target, or data research | [Specifications index](specs/INDEX.md) | Relevant spec and original evidence |
| Repository overview / contributor onboarding | [README](../README.md) | [Contributing](../CONTRIBUTING.md), [Tool usage](usage.md) |


## Ownership

| Fact | Owner |
| --- | --- |
| Binary identity and load address | `config/targets/<target>/target.toml` |
| Reviewed layout | `config/targets/<target>/splat.yaml` |
| Target-local symbols | `config/targets/<target>/symbols.txt` |
| Shared SDK symbol maps | `config/sdk/psyq-{slus,logo}.txt` |
| Reviewed Rizin annotations | `config/targets/<target>/reviewed.rz` |
| Authored lifts | Metadata-resolved source under `src/bof3/` |
| Macro opportunity policy and tool reference | `docs/specs/MACROS.md` |

Keep one C source per lifted function. Lift identity comes from manifest claims,
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
| [`$bof3-re`](../.codex/skills/bof3-re/SKILL.md) | target-qualified lifting and independent review |
| [`$bof3-macros`](../.codex/skills/bof3-macros/SKILL.md) | macro opportunities, human-value ranking and reviewed resolution |
| [`$bof3-types`](../.codex/skills/bof3-types/SKILL.md) | established C types, representation opportunities and reviewed application |
| [`$bof3-naming`](../.codex/skills/bof3-naming/SKILL.md) | explicitly routed naming opportunities, evidence, audits and reviewed identity transactions |
| [`$plans`](../.codex/skills/plans/SKILL.md) | persistent plan management |
| [`$psx-rizin`](../.codex/skills/psx-rizin/SKILL.md) | explicitly requested analyzer workflow |
| [`$repo-documentation-repair`](../.codex/skills/repo-documentation-repair/SKILL.md) | explicitly scoped documentation repair |

Macro, type and naming skills include invocation scripts that dispatch to `bin/`;
policy, parsing and editing remain in the harness owners. Explicit-only routing is
preserved in skill metadata; discovery never expands mutation authority.

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

Write readable C89. Inline assembly is banned except the sanctioned helpers in
`include/base/barrier.h`: `barrier()`/`CLOBBER_*`, bounded approved
`REGISTER_PIN`, and manifest-owned `WEAK_SYMBOL_AT`. A retained matching aid
requires adjacent rationale, live byte-match evidence, and independent review.
Direct numeric register spelling and `INCLUDE_ASM` require explicit approval.

Treat `(analyzer-range SHA-256, size)` as a reuse candidate, not shared
ownership. Match one representative, then independently port and validate a
second member. Promote only a worthwhile common body after two cross-target
members independently byte-match with the same C shape; each target retains its
own wrapper, declarations, map, layout, and validation.

## Documentation map

- [Agent coding standards](agents/CODING_STANDARDS.md)
- [Project context and repository map](agents/project-context.md)
- [Function matching](agents/matching.md)
- [Matching playbook](agents/matching-playbook.md)
- [Memory API](agents/memory-api.md)
- [Plan authoring](agents/plan-authoring.md)
- [Lessons](agents/lessons.md)
- [Specifications index](specs/INDEX.md)
- [Macro opportunity indexing and resolution](specs/MACROS.md)
- [Python harness ownership](specs/HARNESS.md)
- [Tool usage](usage.md)
- [External EU reference](reference/bof3-eu/README.md) — leads only; EU
  addresses are not reviewed US facts

## Header layout

| Directory | Domain |
| --- | --- |
| `include/base/`, `include/memory/` | common types, barriers, fixed RAM and hardware addresses |
| `include/gpu/`, `include/frontend/`, `include/callback/` | rendering, frontend, callbacks |
| `include/loader/`, `include/panel/`, `include/battle/` | loader, panel, battle runtime |
| `include/data/`, `include/media/`, `include/ui/`, `include/game/` | records, media, UI and game templates |

Completed implementation history is in `git log`; current scoped work belongs
under `docs/plans/` only while active.
