# Proposal

## Why

`bin/harness source symbols check` reports **89 findings** and is the last stop in `just check`.
Before any can be cleared they must be classified, because they are not one kind of work:

| Class | Findings | What clearing it costs |
| --- | --- | --- |
| unnormalized map | 14 | mechanical — `bin/harness source symbols normalize --write TARGET` |
| naming debt (function) | 40 | 3 shared symbols, needing reviewed naming transactions |
| naming debt (data) | 21 | 6 shared symbols, same path |
| binding/map drift | 14 | 4 files, and **downstream of naming** (measured below) |

The 14 unnormalized maps are the only genuinely mechanical class, so this change covers exactly
that class. Its measured effect: `symbols check` falls from **89 findings to 75**.

**Why the other three classes are not in this change** (operator decision, split):

- The 61 naming rows can only be cleared through the reviewed naming transaction path.
- The 14 binding drifts **cannot** be cleared before it. The check validates each binding against
  the composed symbol set (`load_target_symbols` = target map + shared engine globals + PSX SDK
  space), and none of the drifted bindings is carried by the pinned baseline (checked: 9/9 in
  `area000/13`, 1/1 in `area003/13`, 1/1 in `area014/13`, 10/10 in `area016/13`). Adding a raw
  `func_*`/`D_*` map entry would therefore convert `binding/map drift` into **new naming debt**
  rather than clear it.
- That path is currently **blocked**: `bin/harness naming init` fails on a stale Rizin snapshot
  recipe, and `analysis query` fails with `stale reverse index snapshot …; run just index`, which
  in turn fails on the next stale snapshot. The staleness is a *recipe* mismatch — a target's
  config inputs were edited after its snapshot was taken, while its binary is unchanged (measured
  on `emi/world03/area131/13`: `binary_sha256` matches, `replay_sha256` differs, snapshot 14:55
  vs config 15:43).

Clearing that starts with an evidence-pipeline precondition that is a different workstream from map
formatting, so it is planned as its own change: `refresh-evidence-snapshots-and-clear-naming-debt`.

## What Changes

- Normalize the 14 unnormalized target maps with the formatter that owns them.
- Nothing else: no naming decision, no binding change, no source edit, no baseline change.

## Capabilities

### New Capabilities
- `target-map-normalization`: the repository's contract that every target map is in the canonical
  form its own formatter produces.

### Modified Capabilities
(none — no existing requirement covers target-map formatting)

## Impact

- `config/targets/emi/**/symbols.txt` — the 14 maps.
- `config/symbol-naming-baseline.json` is **unchanged**, and `baseline --write` is not run.
- **One `out/` file was written, and it is recorded rather than hidden:** the precondition probe
  rebuilt `out/reverse/snapshots/emi--bmagic--magic003--03.json` via
  `analysis rz-project analyze` (disposable state; that snapshot is now fresh). No other `out/`,
  `build/` or `toolchains/` file was touched.
- Verification: `bin/harness source symbols check` reports no `unnormalized map`, the formatter is
  idempotent over the same targets, and the naming and binding rows are unchanged.
- Out of scope, planned separately: the 61 naming rows, the 14 binding drifts, and the
  stale-snapshot/index precondition that gates them.
