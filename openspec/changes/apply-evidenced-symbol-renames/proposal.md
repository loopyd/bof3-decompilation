# Proposal

## Why

The reviewed naming transaction path cannot resolve **any** of the 61 raw-spelling rows or the 14
binding drifts, because no row can reach `rung_status: proposed` and only a `proposed` row can be
prepared. This is a **harness capability gap**, verified independently against four code gates and
the repository's own tests — not an evidence shortage, and not something better evidence can fix. This
change records the verified gap and the backlog it blocks, so the renaming work stops being scheduled
against a path that cannot run it.

## What Changes

- **Record the verified capability gap**, with file-and-line evidence rather than assertion:
  `_reject_unsupported_semantic_facts` (`tools/python/harness/naming/validation.py`) rejects every
  `proposed`/`exhausted` row without a runner-recomputed capability; `_validated_capability`
  (`naming/equivalence.py`) returns `None` for `kind != "data"`, so **function proposals are
  unreachable by construction**, and for data it requires `rung_status == "exhausted"`, so a
  **`proposed` data row is refused too**; `validate_terminal_capability` blocks the unfiltered
  full-report gate; and **no code path emits positive `selected_call` or `owner_body` facts**, so two
  required imported-function rungs can never close.
- **Record the live observations** that confirm the reading: the trusted import refuses a function row
  with `error: receipt lacks runner-produced owner_resolution facts` while its bound payload holds only
  plain `owner` receipts with no `bof3.naming-evidence-facts/v1` facts (`derived.json` → `facts: []`,
  `conclusion_enabled: false`); and `naming evidence` **skips** committed-checkpoint rows
  (`completed 0, skipped 10`), so collection cannot advance them either.
- **Record the blocked backlog** this leaves: **75 findings — 61 rows (40 function, 21 data) over 9
  symbols and 59 targets, plus 14 `binding/map drift`s**.
- **Withdraw the renaming tasks.** They are not narrowed or deferred by preference: no authored
  evidence of any quality can satisfy the gates, so nothing in this change can implement them. The
  resolution and binding-composition requirements move to the change that can satisfy them.
- **Hand off the unblocker** to `enable-function-naming-conclusions`, and state plainly that it is a
  harness behaviour change and therefore needs the ownership review `docs/agents/harness.md` requires.
- Keep `config/symbol-naming-baseline.json` byte-unchanged.

## Capabilities

### New Capabilities
(none)

### Modified Capabilities
- `symbol-naming-integrity`: the resolution requirement added by this change is replaced by one that
  records the verified capability gap and the blocked backlog. The evidence-freshness requirement and
  the reviewed-path invariant are unaffected and continue to hold.

## Impact

- `out/reviews/**` evidence only: the 26 debt reports, the collector's receipts, and the refused
  conclusion-input attempt
  (`out/reviews/debt-func_801E5988-bmagic-magic004-03.conclusion-input-attempt.json`).
- **No `config/targets/**` map, manifest or `src/bof3/support/*_psyq.c` is touched, and no identity is
  applied** — the gates forbid it, and the only attempt made was a refused import.
- `config/symbol-naming-baseline.json` is **unchanged**; `baseline --write` is never run.
- Verification: the four gates are re-read and their file/line evidence recorded; the repository's own
  naming-conclusion tests are run and their assertions recorded; `bin/harness source symbols check`
  still reports 75 findings, and that result is recorded as-is rather than presented as progress.
