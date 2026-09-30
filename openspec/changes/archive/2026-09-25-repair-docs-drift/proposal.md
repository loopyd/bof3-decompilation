# Proposal

## Why

`bin/harness docs refs docs .pi/skills --broken-only` reports 11 broken references
and exits non-zero, so the repository's own documented reference sweep fails.
Nine are stale paths left by the reorganisation of `tools/rust/bof3-audio` from a
flat layout into submodules (`src/midi.rs`, `src/sequence_timeline.rs`,
`src/sequence_midi.rs`, `src/soundfont.rs`, `src/soundfont_read.rs` and
`tests/bank_stopped.rs` no longer exist). The other two are a stale heading
fragment in the request router (`docs/INDEX.md`) and an off-by-one relative path
in a skill reference that resolves outside the repository root. Readers following
these links reach dead ends, and no gate built on the sweep can go green.

All of this drift sits inside uncommitted work: the two reference documents and
the skill reference file are untracked, and the stale fragment is itself an
uncommitted addition to `docs/INDEX.md`.

## What Changes

- Retarget the nine stale `tools/rust/bof3-audio` references in
  `docs/reference/standard-midi-files.md` and `docs/reference/soundfont.md` to the
  modules that now own each claim, resolved from each module's own `//!` header.
- Repair the `docs/INDEX.md` route to the renamed `## Area text` heading in
  `docs/agents/tool-usage.md`.
- Correct the relative path in `.pi/skills/bof3-re/references/LESSONS.md` so it
  resolves inside the repository root.
- Where one referenced file was split across modules, link each claim's owner or
  the owning directory rather than a single file that no longer exists.
- No requirement, evidence claim or meaning changes; only reference targets.

## Capabilities

### New Capabilities
- `docs-reference-integrity`: documentation references under the swept roots
  resolve to targets that exist, fragments name headings that exist, and
  retargeted references name the module that currently owns the claim.

### Modified Capabilities
(none — `openspec/specs/` holds only `openspec-artifact-conventions`, whose
requirements this change does not alter)

## Impact

- `docs/reference/standard-midi-files.md` — 6 reference occurrences.
- `docs/reference/soundfont.md` — 3 reference occurrences.
- `docs/INDEX.md` — 1 route fragment.
- `.pi/skills/bof3-re/references/LESSONS.md` — 1 relative path.
- Verification: `bin/harness docs refs docs .pi/skills --broken-only` reports no
  broken references.
- Untouched: harness code, Rust crate sources, tests, and every other
  pre-existing uncommitted edit in the working tree.
