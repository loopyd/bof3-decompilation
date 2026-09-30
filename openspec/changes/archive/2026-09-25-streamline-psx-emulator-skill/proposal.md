# Proposal

## Why

`psx-emulator`'s entrypoint advertises 23 actions in a flat "reading order" list in which
**21 of the 23 rows share the same rank (`2.`)** — the ordering carries no information,
and the entrypoint must be read in full before a mission can start. The substance sits
behind it and is never grouped:

| Layer of the skill | Size |
| --- | --- |
| `SKILL.md` entrypoint | 49 lines: frontmatter, a 23-row flat catalog, 3 contract paragraphs |
| `references/` | 25 flat files, 1274 lines (`CDROM.md` 164, `HISTORY.md` 104, `RUNTIME.md` 101) |
| `scripts/` | 52 Lua modules plus `runtime.sh` |
| `tests/` | 55 Lua files |
| `.pi/agents/psx-emulator.md` | dispatch spec pointing at `SKILL.md` |

Nothing is documented as a layer, so an agent cannot load only the part a mission needs.

## What Changes

- Re-layer the entrypoint: `SKILL.md` keeps the mission contract and gains a small set of
  named layers — runtime and bounds, observe state, drive execution, capture devices,
  extend an action — each pointing to one owning reference.
- Move the full 23-action catalog out of `SKILL.md` into an on-demand reference, so the
  top-level surface is small.
- Keep every ability: no action, script, reference or test is removed. The per-device
  references remain the detail layer and keep their file names.
- Group the flat reference set under its owning layer, and update the dispatch owner
  `.pi/agents/psx-emulator.md` plus the docs routes that describe the entrypoint.
- No behaviour change: actions, scripts, bounds and receipts are untouched.

## Capabilities

### New Capabilities
- `psx-emulator-skill-layering`: the shape of the emulator skill's entrypoint — a small
  layered surface, an on-demand action catalog, and one owning reference per layer.

### Modified Capabilities
(none — no existing requirement covers the skill's entrypoint structure)

## Impact

- `.pi/skills/psx-emulator/SKILL.md` (re-layered and smaller); a new on-demand catalog
  reference; layer grouping for the 25 existing references.
- `.pi/agents/psx-emulator.md`; the `docs/INDEX.md` PS1-emulator route and any
  `docs/agents/INDEX.md` text that describes the entrypoint shape.
- Untouched: all 52 action scripts, `runtime.sh`, all 55 tests, the harness `runtime`
  command, and `docs/plans/psx-emulator-capabilities.md` beyond any reference path it
  names.
- The skill's own rule *"keep this entrypoint broad"* is honoured as breadth of coverage:
  the entrypoint gets smaller in size, not narrower in scope.
- Verification uses existing checks (`just check` scoped, `git diff --check`, manual link
  confirmation). No new tests.
