# Design

## Context

See `proposal.md` — Why. Facts established during apply, all measured:

- `bin/harness source symbols check` reported **89** findings: 40 naming (function), 21 naming
  (data), 14 unnormalized map, 14 binding/map drift. It exits 2.
- The 61 naming rows reduce to **9 distinct symbols** (3 functions, 6 data), each shared across
  several targets.
- The binding drift is validated against the **composed** symbol set — `symbols.py:96` builds
  `by_address` from `load_target_symbols(root, target, psyq_space=…)` — not the target map alone.
- None of the drifted bindings is carried by the pinned baseline, so a raw map entry would move the
  finding rather than clear it.
- The reviewed naming path (`naming init` → report → `prepare-transaction` → apply → verify) is
  gated by snapshot freshness: `naming init` fails on `stale Rizin snapshot recipe`, and
  `just index` fails on the next stale snapshot. Staleness is a **recipe** mismatch caused by target
  config being edited after the snapshot (measured on `emi/world03/area131/13`: binary hash matches,
  recipe hash differs).
- This change's spec delta originally covered naming and bindings too; the operator split it, so the
  artifact set now covers only the mechanical class.

## Goals / Non-Goals

**Goals:**

- Clear the 14 unnormalized-map findings with the formatter that owns them.
- Prove the formatter is idempotent and that naming and binding rows are untouched.
- Leave the pinned baseline byte-identical.

**Non-Goals:**

- No naming decision, no binding change, no source edit.
- No baseline write, no `just index`, no snapshot sweep beyond the single probe rebuild recorded in
  the tasks and the proposal.
- No change to the 61 naming rows or the 14 binding drifts — those are planned separately.

## Decisions

### Decision 1: normalization is the only truly mechanical class

Formatting a map is deterministic and reversible, and its own tool owns it, so it can be cleared and
verified in isolation. *Rejected:* treating the whole 89 as one mechanical sweep — measurement
showed the other three classes need semantic decisions or a gated pipeline.

### Decision 2: verify by idempotence, not by diff review

The formatter's own contract is that a second pass changes nothing, so verification re-runs it over
the same 14 targets and compares SHA-256s. *Rejected:* reviewing 14 map diffs by hand — it proves
less than idempotence does.

### Decision 3: prove the other classes were not touched

Because naming debt is measured against a baseline, the cheapest proof that normalization did not
disturb it is that the baseline hash is unchanged **and** the naming rows are identical in kind and
count before and after. *Rejected:* asserting "formatting cannot affect naming" — the check's own
output is the evidence.

### Decision 4: the split is recorded in the artifacts, not just in conversation

The naming rows, the binding drifts and the evidence-pipeline precondition are named out of scope in
the proposal, and the follow-up change is named. *Rejected:* quietly leaving the blocked tasks in
this change's task list — that would make an incomplete campaign look like a finished one.

## Risks / Trade-offs

- [Formatting silently rewrites a spelling] → Decision 3's row-count and baseline-hash checks would
  catch it, since a spelling change moves a symbol between raw and semantic accounting.
- [The split looks like scope avoidance] → the proposal and this design name exactly why the other
  classes cannot be cleared here, with the measurement that shows the binding class is
  naming-dependent and the pipeline is gated.
- [The `out/` snapshot the probe rebuilt is mistaken for reviewed truth] → it is recorded in the
  proposal and the tasks as disposable state written by an authorized probe, and its target's
  snapshot is now fresh.
