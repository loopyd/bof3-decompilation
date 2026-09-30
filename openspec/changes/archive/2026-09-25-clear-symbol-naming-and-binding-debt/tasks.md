# Tasks

Scope note (operator decision, split): this change covers **only** the mechanical class — the 14
unnormalized maps. The 61 naming rows, the 14 binding drifts and the stale-snapshot/index
precondition that gates them are planned as `refresh-evidence-snapshots-and-clear-naming-debt`.

## 1. Capture the debt and the baseline

- [x] 1.1 Record the check as it stood: run `bin/harness source symbols check` and record its exit code plus the class counts — verify: 89 findings with 40 function, 21 data, 14 unnormalized-map and 14 binding/map-drift rows, exit 2. RESULT: exactly that; rows at `/tmp/debt-rows-clean.txt`, record at `/tmp/clear-debt-baseline.json`.
- [x] 1.2 Record which maps are unnormalized and which naming decisions exist behind the naming rows — verify: 14 map paths recorded, and the 61 naming rows reduce to 9 distinct symbols. RESULT: 14 targets recorded; 3 function symbols (×22/×11/×7) and 6 data symbols (×11/×4/×2/×2/×1/×1) = 9 decisions.
- [x] 1.3 Record the pinned baseline and the pre-change identity state — verify: the baseline hash and identity results are retained for the end-state comparison. RESULT: baseline sha256 `afe50b04b1445d83…`; `lift status --detail minimal --json` over the 59 naming-row targets aggregates to **exact 206, invalid 0, partial 0**; repo-wide `source validate` = `valid=2071 invalid=0`.

## 2. Normalize the target maps

- [x] 2.1 Normalize the 14 unnormalized maps with `bin/harness source symbols normalize --write TARGET` for each recorded target — verify: `check` reports no `unnormalized map`, and re-running the formatter over the same targets produces no further change. RESULT: all 14 normalized (`emi/world00/{area007,area015,area023}/13`, `emi/world01/area049/14`, `emi/world02/{area077,area080,area081,area100}/*`, `emi/world03/{area119,area134,area148}/13`, `emi/world04/{area167,area188,area196}/13`); `check` fell 89 → 75 with **0** unnormalized-map rows; a second pass left their SHA-256s unchanged.

## 3. Verification

- [x] 3.1 Verify no unnormalized map remains: run `bin/harness source symbols check` and count the class — verify: `unnormalized map` rows = 0.
- [x] 3.2 Verify the formatter is idempotent over the same 14 targets — verify: re-running it leaves the maps' SHA-256s identical (checked; unchanged).
- [x] 3.3 Verify no naming debt was created or cleared: compare the naming and drift row counts before and after — verify: naming (function) 40 → 40, naming (data) 21 → 21, binding/map drift 14 → 14 (total 89 → 75, the difference being exactly the 14 formatting rows).
- [x] 3.4 Verify the baseline did not absorb anything: compare `config/symbol-naming-baseline.json` against task 1.3's hash — verify: byte-identical (`afe50b04b1445d83…`) and no `baseline --write` run.
- [x] 3.5 Record the touched-file set, including the one `out/` write: list the changed maps and state the probe's snapshot rebuild — verify: only the 14 maps differ under `config/targets`, and `out/reverse/snapshots/emi--bmagic--magic003--03.json` is the single `out/` file written (disposable state, now fresh), with no `build/` or `toolchains/` write.
