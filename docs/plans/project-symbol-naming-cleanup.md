# Project symbol naming cleanup

## Goal and current evidence

Replace address-canonical `func_XXXXXXXX` and `D_XXXXXXXX` names only when target-local evidence satisfies the two-corroborator gate. Preserve every unresolved address name rather than guessing.

Current inventory (2026-08-22): `bin/naming-audit init-all out/reviews/plan-audit-naming` validated 23 target reports containing 1,361 unique target-qualified map rows (530 functions and 831 data symbols). Only `emi/world00/area030/05` is complete; all 1,361 rows in the other 22 targets are initial `blocked` records with explicit evidence gaps and a next bounded command, not evidence-exhausted conclusions. Full `bin/analysis-readiness` reports 1,361 unresolved evidence ceilings, 1,146 required-work items, no proposed transactions, a ready index, and no stale facts. Live source debt is 338 address-named lift files plus 23 reviewed filename-style exceptions. `bin/symbols check` passes against `config/symbol-naming-baseline.json`, whose exact-entry ceiling is 532 raw functions, 833 raw data symbols, 338 raw function files, and 23 exceptions; the live map has reduced that baseline by four rows.

The disposable report set under `out/reviews/plan-audit-naming/` is the campaign work queue, not reviewed truth. Refresh it before resuming work because reverse-index evidence and maps can change.

## Phase 0 — Naming framework and regression gate ✅

The metadata-authoritative source registry, flexible semantic filenames, v3 naming-audit reports, and exact-entry naming-debt gate are implemented. Keep `@source` and `@behavior` as source identity authority; source filename, compiled symbol, and Splat label remain separate identities.

Validation ownership:

- `bin/symbols check` enforces target maps, metadata provenance, and no naming-debt regression.
- `bin/naming-audit init-all OUTPUT` inventories each raw map row exactly once and validates every generated target report before atomically publishing its summary.
- Baseline changes belong in the same reviewed transaction that removes debt or explicitly approves an exception.

## Phase 1 — Evidence audit by target

Process incomplete targets in descending live row count. The current order is:

| Target | Rows |
| --- | ---: |
| `emi/battle/battle/15` | 249 |
| `exe/slus_004_22` | 247 |
| `emi/etc/game/00` | 192 |
| `emi/battle/battle/03` | 190 |
| `emi/world00/area030/04` | 126 |
| `emi/etc/shop/00` | 99 |
| `emi/etc/commu00/00` | 30 |
| `emi/etc/sisyou/00` | 29 |
| `emi/world00/area032/13` | 23 |
| `emi/world00/area008/13` | 20 |
| `emi/world00/area027/13` | 19 |
| `emi/scenario/scena00/00` | 17 |
| `emi/world00/area016/13` | 17 |
| `emi/world00/area024/14` | 17 |
| `emi/scenario/sce10eff/00` | 16 |
| `exe/logo` | 16 |
| `emi/world00/area026/13` | 13 |
| `emi/scenario/scena16/00` | 12 |
| `emi/etc/game/01` | 11 |
| `emi/world00/area028/13` | 9 |
| `emi/etc/bate/03` | 5 |
| `emi/battle/batl_re2/01` | 4 |
| `emi/world00/area030/05` | 0 (complete) |

For each row, execute its generated `required_work` and `ceiling_next_command` through the typed rungs. Replace an initializer row only with a receipt-backed `exhausted` or `proposed` conclusion. Require exact target ownership, selected instructions/access width, storage/layout, local callers or consumers, and an independent corroborator. A decompiler label, duplicate hash, string, address, or one xref is not a semantic name.

Record durable runtime or format findings in `docs/specs/`; keep report and receipt artifacts under ignored `out/`.

## Phase 2 — Serial semantic naming transactions

Apply one target-local proposal at a time after `bin/naming-audit validate TARGET REPORT --transaction KIND:NAME` accepts it:

- preserve address, ABI, width, signedness, volatility, pointer depth, storage, layout, and code shape;
- update the target map, metadata-resolved lift source and `internal.h`, `symbols.c` binding when present, direct same-target references, and reviewed Splat label when owned;
- add function-level `@source`/`@behavior` metadata and `/* @source 0xXXXXXXXX */` provenance for every non-address map symbol;
- rename a lift file only when the complete metadata-resolved transaction justifies it;
- reduce `config/symbol-naming-baseline.json` exact entries in the same reviewed transaction.

Do not edit SDK maps, generated PsyQ bindings, or another target's symbols. Do not share names merely because addresses or bytes coincide.

## Phase 3 — Transaction verification

For every proposed rename run, in this exact order (aligned to the naming
verifier's required `post_apply_receipts` keys in
`tools/python/harness/analysis/naming_audit_v3.py`):

1. before applying (Phase 2 preparation), capture the pre-apply `partial
   baseline` for any pre-existing partial lift in the transaction scope;
2. apply the transaction accepted by `bin/naming-audit validate`;
3. `bin/symbols normalize TARGET --write`;
4. `bin/symbols check TARGET`;
5. `bin/splat TARGET` and `bin/build TARGET` (function transactions only);
6. lift validation for each touched lift, keyed to its pre-transaction status:
   - exact lift: fresh `bin/asm-diff TARGET@0xADDRESS --detail normal` and `bin/byte-match TARGET@0xADDRESS` must both exit `0`;
   - pre-existing partial lift: capture pre-apply and post-apply `bin/asm-diff` and `bin/byte-match` receipts proving unchanged match percentage, sizes, first mismatch, and body/ABI/boundary/compiler settings, plus unchanged source `@status`/`@match`/`@residual`; a nonzero byte-match exit is the expected, valid outcome for a partial and does not fail the transaction;
7. focused harness tests, `git diff --check`, and independent review;
8. record one digest-bound passed receipt per required command under `out/reviews/evidence/` (repo-relative receipt path plus SHA-256 in the row's `post_apply_receipts`): exactly one passed `bin/symbols normalize`, `bin/symbols check`, and `independent review`, plus for function rows `bin/splat`, `bin/build`, and either `bin/asm-diff`/`bin/byte-match` for the selector (exact) or one `partial baseline` (`partial_used` rows);
9. `bin/naming-audit verify TARGET REPORT --transaction KIND:NAME` — the last gate, run only after every required passed receipt above is recorded;
10. refresh index and readiness: recheck `bin/rz-project status TARGET --json` and `bin/rev-query --json status` (run `bin/index --recover` if stale), then `bin/analysis-readiness TARGET`.

Partial validation aligns with the naming verifier's `partial_baseline` rung and receipt: the audit records live percentage, sizes, first mismatch, residual, and original-byte verification, and `bin/naming-audit verify` requires one passed `partial baseline` receipt for every `partial_used` row instead of passed asm-diff/byte-match receipts.

Rollback point: any check in steps 3–8 failing before verify blocks and
reverts the transaction; do not fix it forward.

## Phase 4 — Target and campaign checkpoints

After each target, refresh its report and record renamed symbols, exhausted rows, remaining blocked rows, evidence, checks, and residual risks. Refresh the complete report set before claiming campaign completion.

The plan is complete only when:

- all 23 target reports validate and every live raw function/data map row appears exactly once;
- every row is receipt-backed `exhausted` or has been applied from a validated `proposed` transaction; no initializer-only `blocked` rows remain;
- every accepted rename has two independent target-local corroborators and exact byte evidence where an exact lift is touched, or unchanged partial-baseline receipts where a pre-existing partial lift is touched;
- maps, Splat, declarations, bindings, references, filenames, and metadata agree;
- remaining raw names have reviewed evidence-exhaustion receipts, and address-named lift files have either been semantically renamed or have an explicit unresolved evidence owner;
- repository-wide `bin/symbols check`, focused naming-audit tests, readiness checks, and `git diff --check` pass.

## Blockers, boundaries, and non-goals

Stale or absent reverse evidence, mixed code/data boundaries, absent callers, runtime-populated tables, unknown ABI/storage, or a missing independent corroborator blocks an individual row, not the campaign. “Name all” means audit every raw symbol; it never authorizes cosmetic aliases or unsupported guesses.

No lifting, matching experimentation, behavior refactor, source relocation unrelated to an accepted rename, compiler/toolchain change, SDK-body lift, generated artifact commit, or cross-target ownership change is in scope.
