# Proposal

## Why

`bin/harness docs` is a seven-mode read-only Markdown transport
(`harness/docs/cli.py` → `documents` / `references` → `markdown` / `anchors`) with a
shell wrapper in the `$bof3-docs` skill. The agent already reads, searches and edits
files natively, so the transport duplicates a capability the harness must keep
maintaining: it is the sole owner of roughly 692 lines of Markdown parsing, and every
change to it needs harness ownership review, tests and documentation.

Crucially, the part the repository's *gates* depend on is not the transport.
`bin/harness source docs` — the drift gate in `just check` — resolves through
`harness/docs/drift.py`, which imports only `claims.py` and `inspection.py`, both
standard-library-only. The transport chain is separable, and `paths.py` has an
unrelated consumer in `harness/context/bof3_cleanup.py`. Retiring the transport
therefore removes maintenance surface without dropping any gate.

## What Changes

- Remove the `docs` domain from `harness/registry.py` (the passthrough to
  `harness.docs.cli`), so `bin/harness docs` no longer exists.
- Delete the transport modules `harness/docs/cli.py`, `documents.py`,
  `references.py`, `markdown.py` and `anchors.py`.
- Keep `harness/docs/drift.py`, `claims.py`, `inspection.py` and `paths.py`, so
  `bin/harness source docs` (drift) and `bin/harness agent context cleanup` keep
  working exactly as they do now.
- Delete the skill wrapper `.pi/skills/bof3-docs/scripts/docs.sh` and rewrite
  `.pi/skills/bof3-docs/SKILL.md` to work with native file tools while preserving
  every existing obligation: check the owning fact before editing, preserve
  unrelated wording, resolve affected links and indexes, run the applicable checks,
  and report gaps. The `## Reference inspection` contract keeps a home in the
  document contract.
- Update the owners that currently document the transport: `docs/agents/documentation.md`,
  `docs/agents/harness.md`, `docs/agents/tool-usage.md`,
  `.pi/skills/bof3-docs/references/DOCUMENTATION_REPAIR.md`, and the plan reference in
  `docs/plans/psx-emulator-capabilities.md`.
- Update the per-domain expectation in
  `tools/python/tests/commands/test_wrapper_bootstrap.py`.

## Capabilities

### New Capabilities
- `harness-docs-surface`: which Markdown capabilities the harness exposes as
  commands, which remain internal library code, and which obligations the
  `$bof3-docs` skill carries without a transport.

### Modified Capabilities
(none — no existing requirement covers the Markdown transport)

## Impact

- Harness: `harness/registry.py` loses one domain; five modules deleted
  (~692 lines); four modules retained (`drift`, `claims`, `inspection`, `paths`).
- Skill: `.pi/skills/bof3-docs/SKILL.md` rewritten; `scripts/docs.sh` deleted.
- Docs: `docs/agents/documentation.md`, `docs/agents/harness.md`,
  `docs/agents/tool-usage.md`, `references/DOCUMENTATION_REPAIR.md`,
  `docs/plans/psx-emulator-capabilities.md`.
- Tests: `tools/python/tests/commands/test_wrapper_bootstrap.py`.
- Gates retained — no gate is dropped or weakened: `source docs` (drift),
  `source symbols check` and `source validate` all remain, so no recorded goal
  boundary is crossed.
- Untouched: `harness/docs/drift.py`, `claims.py`, `inspection.py`, `paths.py`,
  `harness/context/bof3_cleanup.py`, and the Rust crates.
