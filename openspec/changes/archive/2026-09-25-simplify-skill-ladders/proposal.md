# Proposal

## Why

The repository encodes its acceptance procedure as *ladders* — ordered rungs with an
evidence gate at each step. They work, but they have grown: the same ladder vocabulary
is repeated across document owners and skill references, and each copy carries prose
another copy already states. `LESSONS.md` declares the very rule being broken —
*"the lever tables live in `OBSERVATIONS.md` and `docs/agents/matching-playbook.md`.
No prose duplication of those."*

The ladder constructs are spread across two homes:

| Ladder construct | Owner |
| --- | --- |
| `## Allocation ladder` and clean-C/allocation ladder references | `docs/agents/matching-playbook.md` |
| Terminal ladder, clean-C-only rung rules | `.pi/skills/bof3-re/SKILL.md`, `references/REVERSE/MISSION_PROTOCOL.md` |
| Lift levels (Level 1..N gate tables) | `.pi/skills/bof3-re/references/LESSONS.md` |
| Default/terminal rung policy, opt-in expensive rungs | `.pi/skills/bof3-re/references/OBSERVATIONS.md` |
| `## Exhaustion gate`, `ladder_exhausted` conditions | `.pi/skills/bof3-re/references/REVIEW/REVIEW_CHECKLIST.md` |
| `## Safe ladder`, `## Guarded ladder` | `.pi/skills/bof3-naming/references/BYTE_SAFE_COSMETICS.md` |
| Typed naming rungs and `ladder_exhausted` reporting | `docs/agents/tool-usage.md` |

Because the ladders are restated rather than referenced, every change to one copy risks
contradicting the others, and agents spend context re-reading the same rules.

## What Changes

- **First, simplify the ladders in place.** Each named ladder becomes a compact ordered
  rung list: one line per rung, its action, and the evidence gate that promotes or
  rejects it. Duplicated narrative is removed, and where a lever table has an owner the
  ladder links to it.
- **Then compact the surrounding skill prose**, including the guidance that restates
  harness command sequences, done directly and without weakening any contract per the
  repository rule for agent/skill Markdown.
- Order matters: compaction follows simplification so prose is reduced once against the
  final ladder text instead of being rewritten twice.

## Capabilities

### New Capabilities
- `skill-ladder-guidance`: the shape and integrity of the ladder guidance — every rung,
  gate, ban and obligation survives simplification, and each ladder has one home the
  others reference.

### Modified Capabilities
(none — no existing requirement covers the structure of skill guidance)

## Impact

- Ladder owners simplified: `docs/agents/matching-playbook.md`,
  `.pi/skills/bof3-re/SKILL.md`, `references/LESSONS.md`, `references/OBSERVATIONS.md`,
  `references/REVERSE/MISSION_PROTOCOL.md`, `references/REVIEW/REVIEW_CHECKLIST.md`,
  `.pi/skills/bof3-naming/references/BYTE_SAFE_COSMETICS.md`, `docs/agents/tool-usage.md`.
- Prose compacted in the same files plus the skills that restate harness command
  sequences: `NAMING_AUDIT_V3.md`, `IDENTITY_TRANSACTIONS.md`, `SOURCE_IDENTIFIERS.md`,
  `SOURCE_RELOCATION.md`, `SHARING_NONMATCHES.md`.
- Scope excludes the vendored `openspec-*` skills: they are installed skill packages,
  not project guidance, and editing them needs separate authorization.
- No gate, ban, threshold, obligation or acceptance rule is removed — only wording and
  duplication. Verification uses the applicable existing checks (`bin/harness source
  docs`, scoped `just check`, `git diff --check`), not new tests.
- Untouched: harness code, Rust crates, and every non-guidance document.
