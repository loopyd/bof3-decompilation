# Design

## Context

See `proposal.md` — Why. Facts established read-only:

- The window builder is `reviewed_windows(repo, targets)` in
  `tools/rust/bof3-text/src/cli.rs`; it walks `targets` for `splat.yaml`, reads segments
  through `splat_segments`, keeps `kind == "textbin"`, bounds each by the following entry,
  and derives `(archive, entry, load)` from the sibling `target.toml`.
- The same file holds a hard-coded expectation of three `archive#entry` pairs and fails the
  build when any is absent.
- `config/targets/emi/etc/commu00/00/splat.yaml` has a textbin entry at `14588` bounded by
  `15696` (length 1108) named `T_801F24FC`; its `target.toml` gives
  `disc_id = "BIN/ETC/COMMU00.EMI#0"` and `load_address = 0x801eec00`, so the name is
  consistent with the address.
- The retained `out/text-windows/registry.json` (48,571 windows) already carries that exact
  window, and exactly three `reviewed_textbin` windows overall.
- The record is tracked and **git-clean**, so this is not uncommitted local damage.
- `commu00`'s record nests entries under mapping-style segments with `subsegments:` (5
  top-level entries); the working `scena00` record exposes 48 flat top-level segments.
  `splat_segments` is a lenient line scanner, so its view can differ from a YAML reader's.

## Goals / Non-Goals

**Goals:**

- Restore the reviewed COMMU00 window so the registry rebuilds and stops blocking `just check`.
- Keep the expectation set and every failure condition intact.
- Leave the retained artifact and a fresh build in agreement.

**Non-Goals:**

- No change to the expectation list, the `RETIRED`/`HISTORICAL` guards, or `source symbols check`.
- No new tests, and no registry format change.
- No wider `splat.yaml` reorganisation beyond what the chosen remedy needs.

## Decisions

### Decision 1: pin the reader's actual output before choosing a remedy

The two remedies act on different owners, so the choice must rest on what `splat_segments`
really returns for that file rather than on inference from its shape. *Rejected:* editing the
record immediately because its YAML "looks nested" — the reader's behaviour, not the apparent
nesting, decides.

### Decision 2: prefer the record-side remedy when it is sufficient

If flattening `commu00`'s record to the working sibling convention makes the reader see the
entry, that is the narrower fix: it changes one target record, alters no harness behaviour,
and needs no ownership review. *Alternative:* teaching `splat_segments` to descend into
`subsegments` — correct in principle, but it changes harness behaviour, so it is taken only if
the record-side remedy cannot work (for example because other records depend on the nesting),
and then it takes the `docs/agents/harness.md` ownership note first.

### Decision 3: the registry artifact may be regenerated, and that is called out

If the record's bytes change, the window's `owner.hash` changes and a fresh build can no
longer equal the retained artifact until the artifact is regenerated. Regenerating
`out/text-windows/registry.json` is legitimate disposable-state maintenance, and the proposal
names it; the test's value is preserved because it still compares a fresh build against a
retained artifact that was not written by the test run.

### Decision 4: verify with the existing gate, and confirm the window count

`cargo test --test text_window_registry` is the gate; a registry that silently dropped a
window must not pass, so verification also confirms the registry still holds exactly three
`reviewed_textbin` windows and that the other two are unchanged. *Rejected:* adding a new
test — this repository does not add regression tests unless the operator asks.

## Risks / Trade-offs

- [Flattening the record changes compiled output] → the record's byte ranges are config, not
  generated code; verify with `bin/harness source splat` and the existing source gates after
  the edit rather than assuming.
- [Regenerating the retained registry hides real drift] → regeneration is allowed only after
  the missing window is explained and restored, and the other two reviewed windows are checked
  unchanged.
- [A reader-side change alters behaviour for every target] → taken only under Decision 2's
  condition and with the ownership note.
- [The change appears to "fix the gate" by loosening it] → Decision 4 and the spec's last
  requirement forbid shortening the expected set or downgrading the failure.
