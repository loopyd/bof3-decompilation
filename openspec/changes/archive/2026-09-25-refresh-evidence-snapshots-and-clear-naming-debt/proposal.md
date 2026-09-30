# Proposal

## Why

61 naming rows and 14 binding drifts remain from `bin/harness source symbols check` — the previous
change cleared only the 14 formatting rows (89 → 75). Neither class can be cleared by editing maps:

- The 14 binding drifts are validated against the **composed** symbol set (`load_target_symbols` =
  target map + shared engine globals + PSX SDK space), and **none** of the drifted bindings is
  carried by the pinned baseline (measured: 9/9 in `area000/13`, 1/1 in `area003/13`, 1/1 in
  `area014/13`, 10/10 in `area016/13`). A raw `func_*`/`D_*` map entry would therefore convert
  `binding/map drift` into **new naming debt** instead of clearing it.
- The 61 naming rows require the reviewed naming transaction path.

**A pilot measured what that path still requires, and this change records the result rather than
assuming it:** `bin/harness naming prepare-transaction` refuses any row whose `rung_status` is not
`proposed`. Measured across all 26 debt reports, **1004 rows are `blocked` and 0 are `proposed`** —
so no rename can be prepared, let alone applied, until a row is fully proposed. The two row classes
also differ in kind: function rows carry containment evidence (`analysis query --json owners`
returns `payload_contained: 1`), while data rows carry none (`describe` reports `contained=false`,
`present_in_binary=false`, `authority=[]`).

**That path is blocked by an evidence precondition, which this change owns first:**

```
bin/harness naming init <target> out/reviews/…   → error: stale Rizin snapshot recipe: …magic003--03.json
   (after refreshing that one snapshot)          → error: stale reverse index snapshot …; run just index
just index                                       → error: stale Rizin snapshot recipe: …world03--area131--13.json
bin/harness analysis query status                → same failure as naming init
```

Staleness is a **recipe** mismatch, not bytes: on `emi/world03/area131/13` the snapshot's
`binary_sha256` matches the current binary while its `replay_sha256` differs, because the target's
manifest/splat/map were edited at 15:43 after the snapshot was taken at 14:55. A 25-snapshot sample
of the first targets was 100 % fresh, so staleness is specific, not global.

Facts that make the naming decisions tractable:

- The 61 rows are **9 symbols**: 3 functions (×22/×11/×7) and 6 data (×11/×4/×2/×2/×1/×1).
- **5 of the 9 already carry a reviewed name in another target**: `func_801E5988` →
  `clearQueuedSlotBytes`, `func_80196070` → `clearWorkFlags`, `D_8014932A` → `counter2` (×2) and
  `selectionCounter` (×1), `D_801448EB` → `uiMode`, `D_80149328` → `counter1`.
- **4 have no name anywhere**: `func_8014D978`, `D_8014933B`, `D_80146384`, `D_80146865`.
- `func_801E5988` is a **shared resident-engine function**: `emi/bmagic/magic004/03` loads at
  `0x801eec00` (above that address) while `emi/battle/battle/03` loads at `0x801d0c00` and covers
  it — the 22 raw entries are overlay references to another target's code.
- `D_8014932A` has **three names for one address** (×2 `counter2`, ×1 `selectionCounter`, ×1 raw) —
  a divergence to resolve rather than inherit.

## What Changes

- **Precondition first, and it is this change's work:** enumerate the stale Rizin snapshots, refresh
  each with `bin/harness analysis rz-project analyze TARGET`, then make `just index` succeed so the
  naming pipeline and analysis queries work again.
- **Record the gate that still blocks the debt.** The reviewed path has a precondition this change
  sets out and measures: `prepare-transaction` refuses a row that is not `proposed` (measured: 1004
  `blocked`, 0 `proposed`). The gate, the two row classes and their evidence are recorded as facts.
- **Defer the renames themselves to a follow-on change.** The nine symbol decisions and the four
  `psyq_source` regenerations move there, and not arbitrarily: the data rows are out-of-payload (no
  owner symbol, no storage authority, no initializer), and a spelling carried by another overlay is a
  shared fixed-RAM promotion rather than an audit-target identity — so renaming is not the operation
  available for them here. The follow-on change starts from this change's recorded gate and
  disposition.
- Leave `config/symbol-naming-baseline.json` byte-unchanged and keep every touched target
  byte-identical.

## Capabilities

### New Capabilities
- `symbol-naming-integrity`: the repository's symbol-naming and binding integrity contract — the
  reviewed transaction path, its per-kind ownership evidence, and the evidence pipeline freshness the
  contract is proved through. This change restores that pipeline and records the gate that still
  blocks the remaining debt; the renames follow in their own change.

### Modified Capabilities
(none — no existing requirement covers naming integrity or the evidence pipeline)

## Impact

- `out/reverse/snapshots/*.json` (refreshed for stale targets) and the reverse index (rebuilt) —
  disposable state that the precondition owns.
- `out/reviews/**` report artifacts — the naming workflow's normal inputs, the pilot's evidence.
- No `config/targets/**` map or manifest and no `src/bof3/support/*_psyq.c` is touched: the renames
  and that regeneration belong to the follow-on change.
- `config/symbol-naming-baseline.json` is **unchanged**; `baseline --write` is never run.
- Verification: `bin/harness source symbols check` still reports its debt and that result is recorded
  as-is; the baseline is byte-identical; no touched target loses byte identity; the scoped gate's
  stopping point is recorded where it actually stops.
