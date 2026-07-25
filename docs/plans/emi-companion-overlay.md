# EMI companion-overlay relation

**Status:** active

## Goal

Record and verify a reviewed static call from one independently owned EMI entry
into a separately bootstrapped companion payload without creating shared symbol,
source, linker, or runtime-service ownership.

## Evidence baseline

- `emi/world00/area030/04` is `BIN/WORLD00/AREA030.EMI#4` at `0x801D0C00`.
- Its reviewed assembly has `0x801E0C28: jal 0x800F500C`.
- `BIN/WORLD00/AREA030.EMI#5` is type-0 RAM at `0x800F5000`, size `0x2250`,
  SHA-256 `9e8a6a98911ad64ea8c22d59ee39ae551f82da004d4e1b73fca8d1ab67ea9145`.
- The static target is offset `0x0C` within #5. This proves neither ABI nor
  concurrent residency, load order, relocation behavior, or shared ownership.

## Phases

1. **Bootstrap companion target** — complete: create the standalone bin-only
   target `emi/world00/area030/05`; review code boundaries and ABI separately.
2. **Manifest/catalog contract** — add a caller-owned immutable static-call
   record; validate its target identity and original `jal` bytes against the
   extracted catalog. Do not modify maps, CMake, matcher linking, or the reverse
   index.
3. **Regression coverage** — use synthetic EMI payloads to prove no relation is
   inferred from duplicate bytes/base and stale identity/call evidence fails.
4. **Readiness gate** — complete: `bin/companion-check TARGET@0xADDRESS`
   reports static-call, companion boundary/map, ABI, and caller declaration
   evidence in one JSON document; it exits nonzero until all gates pass.
5. **Guidance** — document the limited static contract and require lift
   executors and reviewers to stop until boundary, ABI, and declaration evidence
   are reviewed.

## Validation

- `bin/symbols check emi/world00/area030/05`
- `bin/splat emi/world00/area030/05`
- focused `test_emi_catalog.py` and manifest tests
- `ruff check` on touched Python
- `git diff --check`

## Non-goals

No global game-symbol map, `WEAK_SYMBOL_AT` companion binding, cross-overlay
linking, auto-promotion, duplicate ownership merging, reverse-index call edge,
or claim of runtime co-residency. Reconsider `func_801E0C20` only after #5 has
reviewed function boundary, ABI, target-local map ownership, and matching caller
declaration evidence.
