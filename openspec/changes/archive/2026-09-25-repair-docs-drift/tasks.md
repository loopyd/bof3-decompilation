# Tasks

## 1. Retarget the reorganised `bof3-audio` references

- [x] 1.1 In `docs/reference/soundfont.md`, retarget the bank-writer reference to `src/soundfont/mod.rs`, the reader reference to `src/soundfont/reader/mod.rs`, and the packing-tests reference per claim to `tests/bank/` and `tests/voice/tuning.rs` — verify: `bin/harness docs refs docs/reference/soundfont.md --broken-only` reports no broken references.
- [x] 1.2 In `docs/reference/standard-midi-files.md`, retarget SMF parsing and preservation (2 occurrences) to `src/interchange/midi.rs`, execution order to `src/sequence/timeline.rs`, SEP translation to `src/sequence/midi.rs`, and the MIDI tests (2 occurrences) to `tests/interchange/midi.rs` — verify: `bin/harness docs refs docs/reference/standard-midi-files.md --broken-only` reports no broken references.

- [x] 1.3 Follow-up (operator-requested, closes the "A split claim names each owner" scenario for the third claim group): link the synthetic-silence/held-value clause in `docs/reference/soundfont.md` to `tests/soundfont/packing/` — verify: `bin/harness docs refs docs/reference/soundfont.md --broken-only` reports no broken references and the newly named owner holds the claim.

## 2. Repair the router fragment and the escaping path

- [x] 2.1 In `docs/INDEX.md`, change the route fragment `agents/tool-usage.md#area-dialogue-text` to `agents/tool-usage.md#area-text` — verify: `bin/harness docs refs docs/INDEX.md --broken-only` reports no broken references.
- [x] 2.2 In `.pi/skills/bof3-re/references/LESSONS.md`, correct the relative path to `../../../../docs/agents/matching-playbook.md#allocator-sensitive-complex-functions` — verify: `bin/harness docs refs .pi/skills/bof3-re/references/LESSONS.md --broken-only` reports no broken references and no `outside-root`.

## 3. Integration Verification

- [x] 3.1 Verify the documented sweep is clean end to end: `bin/harness docs refs docs .pi/skills --broken-only` reports no broken references and exits zero.
- [x] 3.2 Verify no unrelated content moved: `git diff` and `git status --short` for the four edited files show only reference lines changed, and the pre-existing uncommitted edits under `docs/` are still present.
- [x] 3.3 Verify each retargeted claim still holds for its new owner: read the named module's `//!` header and confirm it supports the sentence it now backs.
