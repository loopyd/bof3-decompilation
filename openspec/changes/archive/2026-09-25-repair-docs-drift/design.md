# Design

## Context

See `proposal.md` — Why. Facts that shape the approach, all verified read-only
through the harness and the crate:

- `bin/harness docs refs` classifies each reference and reports `broken: true`
  with a `status` of `missing-target`, `missing-fragment` or `outside-root`; it
  exits non-zero while any remain. It is the existing detector for this defect.
- `tools/rust/bof3-audio` is no longer flat: `src/` holds 17 submodule
  directories and `tests/` holds 18, and the flat files named by the docs are gone.
- Every file this change edits is uncommitted work: the two reference documents
  and the skill reference file are untracked, and the stale `#area-dialogue-text`
  fragment is itself an uncommitted addition to `docs/INDEX.md`.

## Goals / Non-Goals

**Goals:**

- Make the documented reference sweep exit zero without weakening it.
- Point each reference at the module that actually owns the claim it supports.
- Preserve every pre-existing uncommitted edit in the files touched.

**Non-Goals:**

- No prose rewrite beyond what a truthful link requires, and no change to any
  requirement, evidence claim or conclusion.
- No harness code, gate, Rust source or test changes.
- Do not attempt the 39 pre-existing uncommitted `docs/` edits, and do not commit
  anything.

## Decisions

### Decision 1: verify with the existing sweep, add no test

`bin/harness docs refs docs .pi/skills --broken-only` already detects, classifies
and gates exactly this defect. *Alternative considered:* a pytest unit asserting
link resolution — rejected: it duplicates a harness capability that already owns
the contract, and this repository does not add regression tests unless the
operator explicitly requests them.

### Decision 2: retarget from each module's own doc comment

Targets were chosen from the owning module's `//!` header rather than from
intuition: `src/interchange/midi.rs` ("Standard MIDI File framing and
preservation"), `src/sequence/timeline.rs` ("Bounded execution-order SEP
traversal"), `src/sequence/midi.rs` ("Finite SEP execution-order translation"),
`src/soundfont/mod.rs` ("Authored SF2 banks with mono PCM samples") and
`src/soundfont/reader/mod.rs` ("Preservation-aware SF2 structure and PCM
reader"). *Alternative considered:* pointing all nine at the crate root —
rejected: it satisfies the checker while destroying the reference's value.

### Decision 3: a split claim links each owner

`tests/bank_stopped.rs` no longer exists and its claims are now spread across
`tests/bank/packing.rs` (unchanged VH/VB reuse), `tests/bank/extraction.rs`
(complete edited-fixture EMI through CLI extraction/repacking) and
`tests/voice/tuning.rs` (zero-step "stopped" tuning rejection). One link cannot be
truthful for all of them, so the reference is retargeted per claim. *Alternative
considered:* linking only the first module and leaving the rest unqualified —
rejected: it silently misattributes the stopped-tone claim.

### Decision 4: repair the fragment, keep the route

`docs/INDEX.md`'s route entry is correct and useful; only its fragment is stale,
because `## Area text` replaced the `Area dialogue text` heading. *Alternative
considered:* dropping the fragment to leave a plain file link — rejected: it
would lose a deep link that still has a destination.

## Risks / Trade-offs

- [Editing files that already contain uncommitted work] → change only the
  reference lines, then confirm with `git diff` and `git status --short` that no
  other line moved and no pre-existing edit was reverted.
- [A retargeted link satisfies the checker but misattributes its claim] →
  Decision 2 requires each target to be evidenced by the module's own doc header.
- [The swept roots are narrower than the whole documentation tree] → the spec and
  its scenarios are scoped to the swept roots (`docs`, `.pi/skills`) rather than
  claiming repository-wide reference integrity.
