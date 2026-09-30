# Design

## Context

See `proposal.md` — Why. Facts verified read-only:

- The `docs` domain in `harness/registry.py` is one passthrough action to
  `harness.docs.cli`; the canonical argv table `harness/common/commands.py:37` also
  carries `"docs": ("docs",)`, and `tests/commands/test_wrapper_bootstrap.py` mirrors
  that table in `PYTHON_POLICY`.
- Import graph inside `harness/docs/`: `cli` → `documents` + `references`;
  `references` → `anchors` + `documents` + `markdown` + `paths`; `anchors` → `markdown`;
  `documents` → `paths`. The drift path is separate: `drift` → `claims` (+
  `inspection`), `inspection` → `claims`, and `claims`/`markdown` are stdlib-only.
- `paths.py` is imported outside the transport by `harness/context/bof3_cleanup.py`.
- `bin/harness source docs` resolves `harness.commands.docs.drift` →
  `harness.docs.drift.collect_drift`.
- `tests/commands/test_harness_dry.py` locks package-initializer import edges for
  `harness.docs` and `harness.commands.docs`; both initializers stay, so it is
  unaffected by deleting their siblings.
- `bin/harness source docs` currently reports **0** findings while 11 broken
  references exist, so drift detection and reference inspection are disjoint.

## Goals / Non-Goals

**Goals:**

- Remove the transport and its maintenance surface.
- Keep every gate and every surviving library consumer working unchanged.
- Leave no document or skill instructing a retired command.

**Non-Goals:**

- No gate dropped or weakened; no change to `drift`, `claims`, `inspection` or
  `paths` behaviour.
- No change to `harness agent context cleanup`.
- No Rust crate changes, no new tests, no commit.

## Decisions

### Decision 1: delete the transport chain, keep the drift pair and `paths.py`

Delete `cli.py`, `documents.py`, `references.py`, `markdown.py`, `anchors.py`
(~692 lines). Keep `drift.py`, `claims.py`, `inspection.py` (the `source docs`
gate) and `paths.py` (used by `bof3_cleanup`). *Alternatives:* (a) delete the whole
`harness/docs/` package — rejected, it would delete the drift gate that `just check`
runs, which the recorded goal pool forbids; (b) relocate drift under
`harness/commands/docs/` — rejected as churn, since `paths.py` still needs a package
home for `bof3_cleanup` either way.

### Decision 2: the skill keeps its obligations and loses its transport

Rewrite `SKILL.md` around native read/search/edit while preserving every obligation:
owner check before editing, unrelated-wording preservation, affected link/index
resolution, applicable checks, gap reporting, the repair contract reference and the
explicit-scope rule. *Rejected:* keeping the mode table with a "command retired" note
— that would keep instructing the retired surface.

### Decision 3: retire the domain on all three surfaces together

`registry.py`, `common/commands.py` and the `PYTHON_POLICY` expectation in
`test_wrapper_bootstrap.py` must agree; leaving one behind makes the command surface
inconsistent and the bootstrap test wrong.

### Decision 4: sequence this change after `repair-docs-drift`

`repair-docs-drift` verifies itself with `bin/harness docs refs --broken-only`. That
command is removed here, so the repair must be applied and verified first; otherwise
its gate disappears mid-change. This ordering is a task in the plan, not an
assumption.

### Decision 5: automated reference inspection is retired, and that loss is recorded

`refs` lives only in the transport, and `source docs` does **not** detect broken
references (0 drift findings against 11 broken references, evidenced above).
Retiring the transport therefore ends automated link checking; link hygiene becomes
the agent's responsibility under the rewritten skill. *Rejected (available if
wanted):* folding reference inspection into the surviving `source docs` drift check —
it would keep link hygiene gated, but it contradicts the requested removal and keeps
`references`/`markdown`/`anchors` alive, so it is recorded here rather than assumed.

## Risks / Trade-offs

- [Deleting a module another owner imports] → `paths.py` and the drift pair are
  retained deliberately; verify with an import check after deletion.
- [The documentation rewrite silently drops an obligation] → the spec scenario names
  each obligation, and `## Reference inspection` is preserved because
  `docs/INDEX.md` links to that heading.
- [An agent-facing instruction still names the removed command] → Decision 5's grep
  plus the surviving drift gate.
- [Editing the in-flight skill relocation] → the change scopes skill edits to
  `.pi/skills/bof3-docs/` only and leaves the rest of the relocation untouched.
- [No automated broken-link detector after this change] → accepted per Decision 5 and
  recorded in the proposal's impact; reintroducing it under `source docs` remains an
  option for a later change.
