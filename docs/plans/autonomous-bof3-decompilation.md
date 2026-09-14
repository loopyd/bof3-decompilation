<!-- bof3.plan/v1 -->
# Autonomous BOF3 lift, naming and cleanup

## Goal and scope

Deliver 100% byte-faithful, human-readable BOF3 source through the instructional
`bof3-lift-loop` skill and its lift, naming, types, macros, review and documentation
missions. Orchestration belongs to the active agent session, not a Python scheduler,
Codex CLI subprocess, standalone model call or Pi fallback. Use project domain
skills; do not copy built-in or home agent profiles. Campaigns retain finite,
explicit queues and budgets without narrowing the whole-game end goal.

Historical cleanup removed `.pi/agents/`; subsequent recovery retained the BOF3
definitions from source reads/mutation history. The roles were
`bof3-lifter`, `bof3-namer`, `bof3-cleaner`, `bof3-reviewer`,
`bof3-lift-batch-coordinator` and `bof3-lift-batch-finalizer`. Git HEAD contains
older `bof3-reverse.md`, `bof3-cleanup.md` and `bof3-review.md`; compare these but
do not assume they are the latest versions. Generic classifier/scout/planner/
worker/reviewer definitions imported from home are not recovery targets.

Historical iterations authorized narrowly targeted integration regression tests;
current `AGENTS.md` requires fresh explicit authorization before adding tests.
Run existing coverage and disposable characterization instead. Generic roles implement/review
tooling only; the four project BOF3 roles still own domain execution. No copying
generic profiles into the project. Installed-extension edits and dependency
installation remain unauthorized. Preserve unrelated dirty work and
original media. Git writes/publication remain separately authorized. Existing
domain validation and transaction rollback stay; no campaign backup framework,
loader collectors, handshakes, mirrored state or custom receipt reconstruction.

## Current clean-C aid-removal supersession

The user's ban supersedes every earlier sanctioned-pin/clobber exception and
aided acceptance claim in this plan, skills and matching advice. Remove
`REGISTER_PIN`, direct asm register bindings, `CLOBBER_*`, `barrier()` and all
artificial empty-asm matching barriers; no declaration-only or no-op shims.
Preserve `WEAK_SYMBOL_AT` address-binding assembly, ordinary `NO_SIBLING_CALLS`
compiler attributes, original disassembly, raw measurements and provenance.
`include/base/compiler.h` replaces `include/base/barrier.h` for compiler attributes;
the [source contract](../INDEX.md#source-and-duplicate-rules) owns current policy.

Parent reports removal from 50 C sources and the compiler-header rename applied.
Initial persistent requeue tags were `@status partial`, `@match unavailable`, and
`@residual requeued ...`, pending independent clean-C review and further matching
where needed. Parent also reports
live enforcement in `tools/python/harness/domain/policy.py`, called at asm-diff
resolve/compare, decomp pre-cache validation and `decomp.missions.inspect_candidate`;
status-cache schema is v3 and 77 focused tests passed. Header references are updated
in `domain.repository_layout` and `macros.index`. These are attributed parent
tooling results, not independent source acceptance. Parent owns source/harness and `.codex`/`.pi`
policy edits; this pass updates existing docs only.

Every affected target-qualified consumer, including shared-template wrappers,
needs fresh native instruction/byte checks and independent review before renewed
acceptance or exact cleanup. Previous scores, cached exact rows, receipts and
review verdicts remain historical; do not rebind them to cleaned source or use
old retention/restoration instructions to keep a banned matching candidate.
This correction closes no phase and changes no stable IDs, frozen pilot membership,
consumed budgets or unrelated obligations. The removal documentation pass performed
no source audit, tests or native validation and granted no acceptance; subsequent
parent-confirmed source acceptance is recorded below.

### Durable clean-C requeue

The 50-entry queue across ten targets now has four clean-C reviewed exact
dispatchers and 46 outstanding entries: six prior MATCH candidates still need
review, and 40 DIFFER entries, including AF66C, still need matching and acceptance.
The table is canonical; paths are relative to `src/bof3/`. PRE values remain
historical: 46 exact and four partial before removal. The bounded continuation
below records the four renewed acceptances and their parent-confirmed gates.
The parent inventory retains each `sha256_before` and complete `progress_before`:
`out/reviews/register-aid-removal-20260913-complete/inventory.json`, SHA-256
`24dd1dc35eeb58192d1b31fb33320ca87f324c4bdc8a83b43f8462b7a41c0b0a`.
Membership below was checked against that JSON; game source was not inspected.
Do not overwrite its PRE evidence or apply historical scores to cleaned C.

At the initial removal measurement, parent completed native compilation/comparison
for all 50: 10 MATCH, 40 DIFFER, zero tool errors. `verification.json` beside the
inventory binds each selector, source, return code and log; its SHA-256 is
`22697c8a28352a4d65a2fcc49dfb9619510750b1f3d545d5782680be604253fb`.
Its 50 source/selector pairs and 10/40 return-code split match this queue.
Parent reported `bin/index --recover --timeout 180` succeeded at removal: all 50
sources had requeue metadata, but only 49 affected functions were indexed as partial.
`emi/scenario/scena16/00@0x801F6E30` (`seedRouteEnterState3_scena16.c`) is absent
from `snapshot.functions`; the index builder iterates analyzer functions only.
Its manifest-backed queue row remains explicit below with native result DIFFER.
Do not fabricate analyzer functions or treat index omission as completed work.
At that removal checkpoint all 50 retained `partial` / `unavailable` tags;
MATCH rows were measured candidates, not accepted sources. Indexed coverage was
49 partial plus one explicit manifest-backed queue entry, not 50 indexed functions.
That documentation pass inspected the JSON, not native logs or source semantics;
subsequent review/measurement updates are attributed separately below.
Parent confirms index refresh after the frozen source phase and successful
naming/type/macro queries. Parent's rebuilt SQL count is exactly four exact,
45 partial and one missing, retained in
`out/reviews/clean-c-requeue-20260913/index-counts.json`. This worker read that
receipt, not a native audit; the original 49-partial observation remains historical.

| Target-qualified selector | Source under `src/bof3/` | Historical PRE status / match | Native result / current review disposition |
| --- | --- | --- | --- |
| `exe/slus_004_22@0x8015DF18` | `audio/dispatchSoundCue.c` | exact / 100.00 | DIFFER |
| `emi/battle/battle/03@0x801E6724` | `battle/dispatchByte1PairFlagged.c` | exact / 100.00 | DIFFER |
| `emi/battle/battle/15@0x800AE014` | `battle/dispatchLocalHandlerPair.c` | exact / 100.00 | DIFFER |
| `emi/battle/battle/15@0x800A83F8` | `battle/dispatchWorkByte1Pair.c` | exact / 100.00 | DIFFER |
| `emi/battle/battle/15@0x800AD26C` | `battle/dispatchWorkTable6994.c` | exact / 100.00 | Clean-C reviewed exact; final MATCH, receipt below |
| `emi/battle/battle/15@0x800AD69C` | `battle/dispatchWorkTable69a0.c` | exact / 100.00 | Clean-C reviewed exact; final MATCH, receipt below |
| `emi/battle/battle/15@0x800AD9CC` | `battle/dispatchWorkTable69ac.c` | exact / 100.00 | Clean-C reviewed exact; final MATCH, receipt below |
| `emi/battle/battle/15@0x800ADCC4` | `battle/dispatchWorkTable69b8.c` | exact / 100.00 | Clean-C reviewed exact; final MATCH, receipt below |
| `emi/battle/battle/03@0x801DB3E4` | `battle/enemyBattlerMeetsThresholds.c` | exact / 100.00 | DIFFER |
| `emi/battle/battle/15@0x8009DC6C` | `battle/func_8009DC6C.c` | exact / 100.00 | MATCH |
| `emi/battle/battle/15@0x800A0E68` | `battle/func_800A0E68.c` | exact / 100.00 | MATCH |
| `emi/battle/battle/15@0x800A3F28` | `battle/func_800A3F28.c` | partial / 88.57 | DIFFER |
| `emi/battle/battle/15@0x800A8360` | `battle/func_800A8360.c` | partial / 94.87 | DIFFER |
| `emi/battle/battle/15@0x800AAEBC` | `battle/func_800AAEBC.c` | exact / 100.00 | DIFFER |
| `emi/battle/battle/15@0x800AF66C` | `battle/func_800AF66C.c` | exact / 100.00 | Reviewed clean-C partial; DIFFER, 2/19, 76 bytes |
| `emi/battle/battle/15@0x800AF720` | `battle/func_800AF720.c` | exact / 100.00 | DIFFER |
| `emi/battle/battle/03@0x801DC73C` | `battle/func_801DC73C.c` | exact / 100.00 | DIFFER |
| `emi/battle/battle/03@0x801DCD50` | `battle/func_801DCD50.c` | exact / 100.00 | DIFFER |
| `emi/battle/battle/03@0x801E60DC` | `battle/func_801E60DC.c` | exact / 100.00 | DIFFER |
| `emi/battle/battle/03@0x801DEBC4` | `battle/queueScriptEvent.c` | exact / 100.00 | DIFFER |
| `exe/slus_004_22@0x8014B3C4` | `boot/displayBootExceptionDump.c` | exact / 100.00 | DIFFER |
| `exe/slus_004_22@0x80162230` | `io/emiCdReadyCallback.c` | exact / 100.00 | DIFFER |
| `exe/slus_004_22@0x80162618` | `io/recordEmiDispatchHandler.c` | exact / 100.00 | DIFFER |
| `exe/slus_004_22@0x80162790` | `io/selectNextEmiEntry.c` | exact / 100.00 | DIFFER |
| `emi/scenario/scena00/00@0x801FC7D0` | `scenario/dispatchRecordCallbackByByte7A.c` | exact / 100.00 | DIFFER |
| `emi/scenario/scena16/00@0x801F8358` | `scenario/dispatchRecordCallback_scena16.c` | exact / 100.00 | DIFFER |
| `emi/scenario/scena00/00@0x801FC8FC` | `scenario/resetEffectBank_801FC8FC.c` | exact / 100.00 | DIFFER |
| `emi/scenario/scena16/00@0x801F84AC` | `scenario/resetEffectBank_scena16.c` | exact / 100.00 | DIFFER |
| `emi/scenario/scena16/00@0x801F6E30` | `scenario/seedRouteEnterState3_scena16.c` | exact / 100.00 | DIFFER |
| `emi/etc/game/01@0x801D0D5C` | `ui/armFadeDelay_game01_801D0D5C.c` | exact / 100.00 | MATCH |
| `emi/etc/game/00@0x801C57F4` | `ui/dispatchScenarioSubstate.c` | exact / 100.00 | DIFFER |
| `emi/etc/game/00@0x801975E4` | `ui/dispatchSubstate1.c` | exact / 100.00 | DIFFER |
| `emi/etc/game/00@0x80197A24` | `ui/dispatchSubstate2.c` | exact / 100.00 | DIFFER |
| `emi/etc/game/00@0x80198234` | `ui/dispatchSubstate3.c` | exact / 100.00 | DIFFER |
| `emi/etc/game/00@0x801984AC` | `ui/dispatchSubstate4.c` | exact / 100.00 | DIFFER |
| `emi/etc/game/00@0x80198744` | `ui/dispatchSubstate5.c` | exact / 100.00 | DIFFER |
| `emi/etc/game/00@0x80198904` | `ui/dispatchSubstate6.c` | exact / 100.00 | DIFFER |
| `emi/etc/game/00@0x80198AC4` | `ui/dispatchSubstate7.c` | exact / 100.00 | DIFFER |
| `emi/etc/game/00@0x801BDB7C` | `ui/findModeFreeSlot.c` | exact / 100.00 | DIFFER |
| `emi/etc/game/01@0x801D0E54` | `ui/finishSelection_game01_801D0E54.c` | exact / 100.00 | DIFFER |
| `emi/etc/game/00@0x801ADC98` | `ui/func_801ADC98.c` | exact / 100.00 | DIFFER |
| `emi/etc/shop/00@0x801DAB90` | `ui/func_801DAB90.c` | partial / 59.29 | DIFFER |
| `emi/etc/game/01@0x801D0F00` | `ui/handleMenuInput_game01_801D0F00.c` | exact / 100.00 | DIFFER |
| `emi/etc/shop/00@0x801E2650` | `ui/initializeShopUiState.c` | exact / 100.00 | MATCH |
| `emi/etc/game/00@0x801970EC` | `ui/resetSelectionState.c` | exact / 100.00 | DIFFER |
| `emi/etc/shop/00@0x801E2D1C` | `ui/retreatPanelField6To62.c` | exact / 100.00 | DIFFER |
| `emi/etc/game/01@0x801D18F8` | `ui/updateBanner_game01_801D18F8.c` | exact / 100.00 | DIFFER |
| `emi/etc/game/01@0x801D1B00` | `ui/updateWindows_game01_801D1B00.c` | exact / 100.00 | MATCH |
| `emi/world00/area008/13@0x801F3D88` | `world/drawTexturedFrame.c` | partial / 96.17 | DIFFER |
| `emi/world00/area016/13@0x801F3460` | `world/resetAdvanceScratchState.c` | exact / 100.00 | MATCH |

### Bounded clean-C continuation — 2026-09-13

Parent mission `out/reviews/clean-c-requeue-20260913/mission.md` starts from
`52d46b6c` and owns the four battle15 dispatchers above plus
`emi/battle/battle/15@0x800AF66C`. Original work cutoff is 03:23 UTC, cleanup
hard stop 03:25 UTC: at most nine C variants, three profile probes, one 60-second
permuter pass, one independent dispatcher review and one range review, each with
one repair. This record does not reset consumption, extend a cutoff or replace
the historical frozen-five pilot; its receipts, full-goal scope and phase states
remain unchanged. Parent owns all five sources and serialized native/generated
writes; this worker owns only this plan and the range-predicate specification.

Parent confirms independent source-semantics passes for `800AD26C`, `800AD69C`,
`800AD9CC` and `800ADCC4`, plus the explicit `table.handlers` member repair in
place of the whole-aggregate cast. All four candidate native gates passed;
after final metadata (`@status exact`, `@match 100`, `@residual none`), all four
`bin/byte-match` gates exited zero with MATCH. Parent accepts these four clean-C
sources. Receipts in the mission directory are `800AD26C-final.log`,
`800AD69C-final.log`, `800AD9CC-final.log`, `800ADCC4-final.log` and
`final-pins.txt` (SHA-256
`4390621283088106117af5462444f1805ac8c66eb8471bcc7ec0c3ac8a6906e0`).
This worker read the logs/pins; review and gate execution are parent-attributed.
Source review does not accept full types, macros or naming stages.

Six AF66C variants consumed attempts without improving the retained
clean-C shape: direct boolean 2/19 (76 bytes), early return 5/20 (80 bytes),
declaration order 2/19, signed result 2/19 and direct parameter/no alias 2/19
(the latter three each 76 bytes). Retained `variant-1.json` through
`variant-5.json` corroborate those five measurements. Parent confirms variant six
is the retained direct value parameter plus direct conjunction: `variant-6-final.json`
and `af66c-final-{diff,byte}.log` record 2/19 (10.53%), 76→76 bytes,
asm/byte exits 1 (DIFF/DIFFER). Parent confirms independent partial-review PASS
for source SHA-256
`6b5abf5f4f8b5efd1acbde25482e12b053aeaa9c41beec585f158d4d49678c6d`,
equal to the live source and `final-pins.txt`; the 19-instruction/76-byte, 2/19
residual is correctly bound. Metadata remains partial at 10.53, with no exact
claim or exhausted-ladder acceptance. Six of nine C attempts
are consumed; the remaining allowance is not a new mission. The prior 52-profile
search and 300-second permuter exhausted only their old-shape allowances and were not rerun;
no pins or implicit replacement budgets. Further clean-C experiments need a
distinct bounded scope after this mission; timeout alone is not ladder exhaustion.

Naming remains a separate deferred future audit of
`emi/battle/battle/15@function:func_800AF66C`. Retained `naming-lead.json` has
fingerprint `v1:0f0d1562a8345c3b2fb55cb8f054e4be7d7446a9abb4f39d30d8348141dc4f74`;
the live map still hashes to
`b4e1143924c872248f6e2d76b2f26cf849f6a7cd77ac68c1f512f9b20287bfd7`.
This preserves the framework lead, not rename acceptance or full-target
`complete:true`. Type/macro pool observations likewise grant no application.
After refresh, parent began only read-only macro ranking for battle15:
instruction floor 12, pool size three, top one, `require_source:true`, pool pin
`v1:9038c1cc736f79d5fdf3b933562bce28e070372f970784a7b9c7ec4b46e08872`.
`macro-ranking-request.json` and `macro-ranking-result.json` in the mission
directory retain these proposed dispositions:

| Candidate | Parent ranking / remaining gate |
| --- | --- |
| `assembly_block:366ad828049fbe14`, 32 instructions, exactly the four accepted dispatchers | try; validated read-only ranked selection for review |
| `assembly_block:cb455b0d8fb5016a`, 28 instructions, six five-entry dispatchers | defer; type/selector contracts and baselines unresolved |
| `assembly_block:e0385e3255d66393`, 24 instructions, ten three-entry dispatchers | reject on human value; only two useful C statements |

These are ranking proposals, not macro extraction, semantic acceptance or a new
frozen pipeline queue. Formal rank validation succeeded for
`out/reviews/evidence/clean-c-dispatch-ranking-20260913.json`: one selected for
review, not application acceptance. Receipt
`out/reviews/clean-c-requeue-20260913/macro-ranking-validation.json` retains digest
`v1:9c8ccfbe3c3f30bb820014b94aa4af50c9b3a8b893098b2680b87e1bc64f76a2`;
names are unchanged and naming audit remains open. All work retains the original
03:23 UTC cutoff and consumed budgets. Forty-six requeue entries remain
outstanding, including reviewed partial AF66C. Runtime measurements live in the
[range-predicate note](../specs/runtime/battle-range-predicates.md#clean-c-requeue-measurement).

<a id="reviewed-private-header-creation--2026-09-13-acceptance-pending"></a>
### Reviewed private-header creation — 2026-09-13 (tooling committed; application blocked)

Parent mission `out/reviews/private-header-creation-20260913/mission.md` starts
from `20030baa`, with work cutoff 04:05 UTC and cleanup hard stop 04:08 UTC.
It permits one implementation pass, existing-check iteration, one independent
review and one repair; consumed allowances never reset. Parent owns all code and
serialized application/native work. This documentation worker owns only this plan
and `docs/agents/macros.md`; initial work was bounded to eight minutes and
04:05 UTC. Parent subsequently authorizes verification-only documentary cleanup
through the unchanged 04:08 UTC hard stop, without renewed implementation budget.
Those original bounds are consumed. Parent confirms overdue formatting-only
cleanup completed on continuation, with no new implementation trials, and local
feature commit `c9f884b6` and documentation commit `6ed5dd60` are complete.
This closes the tooling checkpoint;
macro application acceptance remains pending.

The real-game lead remains `assembly_block:366ad828049fbe14`: floor 12, pool
three, top N one, exactly the four accepted battle15 dispatchers above. It is
selected for review and still **not applied**. The existing monolithic header
requires unrelated consumers, including partials and support bindings; do not
narrow that coverage, create dummy headers or manually extract the macro.

The prerequisite blocker was existing-only reviewed owner admission. Common
recovery already supports absent PRE, and `preflight_existing_replacements`
skips absent paths; neither needs an absence workaround. Current parent-owned
`domain/headers.py`, `macros/creation.py`, `macros/review.py` and
`macros/transactions.py` introduce the explicit v2 local-template contract
described in the [macro guide](../agents/macros.md#reviewed-private-header-creation).
It pins one new private header's complete contents and complete manifest PRE/POST
text, permitting only its appended header claim. The sole absent owner has a
null fingerprint, its parent directory must exist, and ordinary live manifest
loading stays strict. Reuse staged no-overwrite publication and identity-safe
owned rollback, preserving concurrent work and restoring proven PRE absence.

Acceptance must cover planned-POST literal-include consumers, exact affected
source scope, cross-target reachability and new-identifier collisions, alongside
the existing semantic, ranking and exact-wrapper gates. V1 remains existing-only;
shared/type creation is not added. Missing PRE membership in aggregate history
is not evidence of absence and cannot be reconstructed from current files.
Parent reports the updated existing atomic macro test passed in 1.22 seconds:
v2 creation failure restores header absence and original manifest/source; success
publishes, passes `verify_application` and rejects a wrong attestation digest.
The 165 existing targeted macro/manifest-claim/file/domain tests passed before
the last repair; the final rerun begun around 04:04 UTC passed all 165 in 63.16s,
retained in `out/reviews/private-header-creation-20260913/tests-final.txt` and read
by this worker. The atomic lifecycle test passed again after repair in 0.94s.
Test execution remains parent-attributed; this worker ran no tests.

Parent reports independent review initially found two P1s: normalized planned
include aliases and an invalid `include_support` keyword, plus structural PRE/
target checks. One repair resolves the reported findings; read-only review probes
now pass canonical relative aliases, support-source coverage, correct targets,
null PRE and configuration PRE-hash checks. They confirm the exact POST shape
used by retained revalidation rejects ordinary run, while missing historical
membership still fails closed. Real-repository dry admission and relative-include
POST scope checks passed across the catalog for the four dispatchers, publishing
no files or macros. Parent confirms scoped independent review passed; final tests,
formatting-only closure and the local commit are now complete.

Guarded retained-history verification checks the original `None` PRE. Generic
revalidation uses POST as its adopted PRE and `structural_manifest` without
rederivation, rather than rerunning creation admission; original absence evidence
is not rewritten. No fresh full-acceptance or native revalidation case is proven.
Resolved review probes do not establish full acceptance. Later live native gates,
independent application review and parent confirmation remain mandatory. No game macro
application or new game header exists at this handoff. Whole-game scope, phase
states, frozen-five membership, historical proofs and every unfinished domain
obligation remain intact; tooling closure grants no macro acceptance.

The original source-application stage was
`out/reviews/battle-dispatch-macro-20260913/mission.md`, with work cutoff
2026-09-13 04:45 UTC and cleanup hard stop 04:48 UTC. Its original allowance was one initial
extraction and one repair, with distinct pre-application and post-application
reviewers, each limited to one bounded clarification. Parent confirms explicit
index refresh followed by validation of the unchanged original ranking pin
`v1:9c8ccfbe3c3f30bb820014b94aa4af50c9b3a8b893098b2680b87e1bc64f76a2`;
`ranking-validation.json` in that mission directory retains one selected lead.
The original candidate, floor 12, pool three, top N one and four-member scope
remain unchanged; validation is not reranking or application acceptance.

Parent alone owns the four `src/bof3/battle/dispatchWorkTable69*.c` dispatchers
listed above, one new `include/bof3/battle/battle15_dispatch_internal.h`, and
`config/targets/emi/battle/battle/15/target.toml` for its header claim. No other
header/map/layout/type/flag changes are authorized. The prepared transaction ran
once as `battle-dispatch-macro-20260913-01`; parent reports session `83761`
exited 2 before source publication. Retained `runtime.json` binds the run and
original cutoff; `run.log` reports
`error: transaction path is unsafe: .agents/skills/bof3-docs`.
Parent verified all six PRE hashes unchanged and the proposed header and application
absent.
At that checkpoint application was blocked and parked: no retry, refunded attempt or budget reset.
Preserve original pins, consumption and now-expired 04:45/04:48 UTC bounds. Failure is
neither macro acceptance nor semantic exhaustion. Owner-controlled application and
guarded rollback, all target/native gates, independent POST semantic review,
parent-bound reviewed envelope and live final verification remain required.
No macro is accepted yet; failure grants no new harness implementation budget.
This stage neither resets the consumed 04:05/04:08 bounds nor promotes any other
plan phase or changes the original frozen pilot and full reconstruction goal.

<a id="active-tracked-symlink-tooling-prerequisite--2026-09-13"></a>
### Reviewed tracked-symlink tooling checkpoint — 2026-09-13 (gitlink blocker remains)

This separate prerequisite retains its frozen 2026-09-13 05:00 UTC work cutoff
and 05:03 UTC cleanup hard stop: one implementation pass, one review-driven repair,
one independent reviewer and one bounded clarification. One implementation pass
is complete; zero review repairs were used. Actual independent reviewer
`01a09912-f4dd-7b03-a226-1de8703f4c2d` returned PASS with no blocking findings,
retained in `out/reviews/evidence/workspace-symlink-review-20260913.md` and read by
this writer. The scoped implementation/review checkpoint closes with the validation
limitation below; no allowance resets. Parent completed local checkpoint
`99180d69`; the separate rejected submodule candidate below does not supersede it.

`common/links.py` captures literal link bytes and identity without following the
referent, preserving no-follow ancestor checks. Safeguards v2 distinguishes files
from links and retains historical v1 reading. Both macro/type publication paths
check captured links; target/type/identity drift rejects, unrelated links are never
restored and symlink-owned mutation remains refused. The durable contract lives in
[harness ownership](../agents/harness.md#policy-versus-mechanism). Existing macro/type
atomic cases now use tracked-link fixtures; the test inventory is unchanged.
No new regression tests, game source/header/target edits or source acceptance belong
to this stage.

Parent-confirmed checks (probe and final-suite logs read by this writer):

- `/tmp/bof3-symlink-probe.txt`: six disposable characterization categories PASS,
  including literal directory/dangling/non-UTF8 links, mutation refusal, drift and
  concurrent-link preservation, read races and legacy regular-file guards.
- Initial transaction run: 186 passed in 154.97s, before final fixture updates;
  focused final two atomic cases: two passed in 5.14s.
- Final handle `7729` is terminal. `/tmp/bof3-symlink-final-tests.txt` reports
  **1 failed, 196 passed in 155.82s**: 186 transaction cases and ten other DRY cases
  pass. Existing `test_harness_dry.py::test_required_untracked_file_modes_are_normalized`
  fails because `git hash-object -w` for preserved untracked
  `tmp/default-g4-fix-before/check-live-receipts.mjs` cannot write a Git object under
  the sandbox (exit 128). This remains unresolved, not an implementation repair or
  an all-green check. Parent will neither broaden permissions nor mutate/stage
  unrelated `tmp/` content to make it pass. Scoped Ruff and whitespace pass.

The real-repository probe clears all eight canonical skill links, then fails on
tracked gitlink directories. Parent confirms all ten; read-only `git ls-files
--stage` corroborates mode `160000`: `third_party/asm-differ`,
`third_party/decomp-permuter`, `third_party/m2c`, `third_party/maspsx`,
`third_party/references/bof3-data-doc`, `third_party/references/vast-violence`,
`third_party/rizin`, `third_party/spimdisasm`, `third_party/splat`, and
`toolchains/psx_psyq_signatures`. No submodule skip, dereference or repair belongs
to this stage. Complete workspace backup and live macro readiness remain blocked;
this review grants no further implementation scope.

Repair completion cannot resume the parked application or rebind old evidence.
Retain original pins and consumption; changed tooling may stale transaction inputs
and requires explicit future scope and fresh applicable validation. The 100%
reconstruction goal remains active: four native/reviewed clean-C dispatch matches
are committed in `20030baa`, 46 requeue entries remain, and no macro acceptance
or aggregate phase completion follows. Stable IDs, phase states and frozen-five
obligations are unchanged.

### Tracked-submodule candidate — 2026-09-13 (rejected and restored)

Distinct mission `out/reviews/workspace-submodules-20260913/mission.md` retains
its original 05:35 UTC work cutoff and 05:38 UTC cleanup hard stop. One
implementation pass is consumed; zero of one review-driven repair passes began.
Design scout Peirce `01a09921-5e6a-7ac2-9db8-b398013fb3a0` completed and closed.
Independent reviewer Ptolemy `01a0992c-3999-7a03-9849-d263464182ae` returned
**needs repair before acceptance**. Parent received its completed bounded
clarification, including the fifth finding below. Retained `clarification.md` now
confirms parent closed the actual reviewer handle before interruption. No acceptance
or allowance reset follows.

The retained `review.md` and parent-relayed completed reviewer clarification
establish five findings against the candidate:

- P1: root Git status traverses unvalidated submodules before protection; a
  disposable probe executed a configured fsmonitor hook.
- P1: Git follows mutable refs, configuration includes and excludes outside the
  confined descriptors; an external loose-ref symlink left the snapshot identical.
- P2: gitfile retargeting can bind old Git-directory/HEAD facts to a new marker.
- P2: an initially empty uninitialized module lacks a final namespace/marker
  recheck; an injected file escaped verification.
- P1: low-level `capture_recovery` and `apply_changes` with `workspace=None` and
  `index=None` both reached dependency `prepare_image`. Reviewer probe
  `/tmp/bof3-submodule-review-lowlevel.py` intercepted before image preparation;
  bytes stayed unchanged. This is reviewer-confirmed evidence, not a parent rerun.

Parent retained exact `candidate.patch`, `candidate-submodules.py` and
`candidate-hashes.txt` in that mission directory, verified candidate bytes, then
restored only twelve tracked tooling/test files to `99180d69` and removed only
the candidate's new `common/submodules.py`. This writer read the review and
confirmed Git lists only the three original tracked dirty files and unrelated
`tmp/` before these documentation edits. No partial repair or configuration
whitelist began. Retained candidate files are disposable evidence, not reviewed
truth or live implementation; the [harness contract](../agents/harness.md#policy-versus-mechanism)
at that restoration checkpoint used safeguards v2 and rejected tracked gitlink
directories; the separate successor below does not accept this rejected candidate.

Parent-confirmed **historical candidate checks**, not restored-code acceptance:
three focused checks passed in 13.40s; the full selected suite passed 196 cases
with one deselection in 177.03s. The previously failing Git-object-writing DRY
case remains unresolved; no permission expansion or unrelated-file mutation
bypassed it. Six disposable characterization categories passed after two fixture
corrections. Real workspace backup captured ten top-level modules and initialized
`vast-violence/randomtools` nesting in 11.79s; full guard round trip took 46.95s.
Existing atomic fixtures gained embedded submodules without new test inventory.
These positives do not resolve the independent findings or accept the candidate.

After restoration, parent confirms the three focused existing checks are terminal:
**3 passed in 5.74s**, `/tmp/bof3-submodule-restored-tests.txt`, read by this writer.
This validates the selected restored-code checks, not the rejected candidate.

The next prerequisite identified at that checkpoint was separately bounded,
independently reviewed design: isolate
Git query metadata immutably and compose a dependency baseline without recursive
unvalidated root status. Preserve actual staged/dirty facts; neither declaring
every module dirty nor skipping dependencies is acceptable. Bind marker selection
and final uninitialized namespace observations. Reviewer advice, **not implemented**:
use confined bounded captures and independent metadata copies, never hardlinks;
control configuration/environment to exclude hooks, includes and fetch; bound
required refs, index and objects. Compose nonrecursive root status with correct
HEAD/index/module dirty facts, retaining worktree attribute/ignore semantics.
Enforce gitlink mutation boundaries before images unconditionally, independent
of optional workspace/index guards. These are future design obligations, not proof
that the restored code or retained candidate enforces them.

The real macro stays parked after its single pre-publication failure: parent
reverified all six PRE states unchanged and proposed header/application absent.
No native build, index refresh, macro retry or source acceptance occurred. Preserve
the expired 04:45/04:48 bounds, original pins and consumption, four accepted clean-C
matches, 46 outstanding requeue entries, all frozen pilot bindings and phase states.
The full reconstruction goal remains active; this checkpoint closes no domain work.

That documentation-only pass directly compacted the changed guide text; 46 existing
plan tests passed in 2.72s. Plan parsing, 63 local references and whitespace checks
pass. Broader suites and native gates were not rerun for these three documents.

<a id="submodule-isolation-successor--2026-09-13-acceptance-pending"></a>
### Submodule isolation successor — 2026-09-13 (supported prerequisite accepted)

Distinct mission `out/reviews/workspace-isolation-20260913/mission.md` starts from
`f2751f58`; the user explicitly prioritizes completing submodule preservation before
C. Work cutoff is 07:10 UTC, cleanup hard stop 07:13 UTC: one architectural
implementation pass, one independent review and at most two bounded repair passes,
plus one parallel read-only scout. The implementation and both repairs are consumed;
no implementation/review allowance remains. Scout Goodall `01a0994a-41ec-7023-ad43-55430d61a59b` completed
and closed. Independent reviewer Pasteur `01a0995b-ebcf-7901-9fd0-e90663dac982`
returned needs repair for six gaps: MIDX external paths, live dependency reads by
root status despite suppression, corrupt-index rollback failure, artifact paths
inside gitlinks, packed replacement refs and late nested drift. Repair-one
verification retained its 06:55 UTC cutoff and 06:57 cleanup. At the initial
sidecar's 06:49 final-revision instruction, parent supplied independent findings:
the six effects closed, but unbound rollback, direct receipt/attestation artifact
writes and false-clean empty unignored embedded repositories remained. Acceptance
was withheld at that checkpoint; repair two subsequently addressed all three.

Final verification retained its 07:07 UTC cutoff and 07:09 cleanup. Pasteur's
`final-review.md` in the mission directory, read by this terminal-accounting writer,
reports **PASS with no confirmed open findings**. Parent accepts only the code
frozen at 06:55:54 UTC and bound by `final-code.sha256`, within the support ceiling
below. Prior rejection, its five findings and consumed 05:35/05:38 bounds remain
historical; this acceptance neither revives them nor completes an aggregate phase.

The [harness candidate contract](../agents/harness.md#submodule-isolation-candidate)
owns noun-module responsibilities, independent metadata copies, sanitized scratch
configuration/status inputs, supported normalization and rejected layouts/settings,
resource ceilings, literal-link versus linked-control behavior, opaque ignored
embedded repositories and observational module locks. Repair one extends immutable
mirrors to root status/backup. Optional MIDX/commit-graph files remain observed but
uncopied/uninterpreted, with both accelerators disabled; the real root has a commit
graph. Packed replacement refs reject. Artifact guards precede mkdir and leases;
bound-image rollback remains independent of a concurrently corrupted index, and
final raw recursive observations address late nested drift. `workspace`, `safeguards`,
`recovery` and direct consumers compose real staged/dirty and nested dependency
facts; unconditional gitlink exclusion precedes images even without optional
guards. Safeguard v3 embeds submodule snapshot v2; historical safeguard v1/v2
readers and enclosing recovery v3 remain distinct. Source rechecks and dependency
nonrestoration are mandatory; no atomic whole-workspace guarantee follows.
Repair two guards unbound rollback separately from bound-image restoration,
guards direct receipt/attestation destinations and rejects empty unignored embedded
roots independently of descendant selection.

The accepted deployment retains non-atomic observations and does not exclude manual
concurrency; root Git metadata is trusted. Arbitrary Git-layout compatibility and
arbitrary index-extension confinement remain unassessed. No index-extension escape
was demonstrated and no projection hardening is claimed. The inspected real root
and ten root-module indexes have checksum-valid v2 TREE extensions only; this
inventory establishes no support or confinement for other formats.

Parent reports these results, separating pre-repair and repaired evidence from
historical rejected-candidate checks:

- Three initial existing atomic cases passed in 41.06s. Initial clean capture of
  ten root modules plus initialized nested randomtools observed 685,427,642 bytes,
  63,537 inventory charges and 99 queries; this was capture-only evidence.
- Pre-repair root round trip passed in 120.68s: 2,144 backup entries, ten root modules
  plus nesting, with exact index, guards and workspace state unchanged.
- Earlier selected suite: 196 passed, one deselected in 255.89s, but it started
  before the latest hardening and does not validate the frozen candidate. Parent
  reports its frozen rerun ended with 192 passed, four corrupt-index rollback
  failures and one deselection. Single-case reproduction at
  `/tmp/bof3-isolation-index-rollback.txt` ended with one failure in 7.08s, read by
  this writer. These failures preceded repair one. The excluded existing
  untracked-mode root Git-object-writing test
  remains a separate known unrelated failure; no permissions were expanded or
  unrelated content changed to bypass it.
- Five prior adverse findings plus additional probes passed in 16.15s; separate
  staged/dirty/symlink/absent-status probes passed. These are parent-attributed
  gates, not independent review or application acceptance.
- Repair one: real root round trip passed in 172.18s; all six finding probes
  passed, with zero observed external-read events. Repaired status semantics pass;
  12 rollback/publication cases passed in 199.91s, resolving the four corrupt-index
  regressions in those cases. Parent confirms 195 behavioral cases passed in
  433.74s; the 450-line ceiling failure is historical after the user's explicit
  increase to 600 and parent edits to the existing DRY test and coding standards.
  At that checkpoint no post-policy complete-suite rerun was supplied; policy change
  alone established neither source acceptance nor closure of the three residuals.
- A broader application suite against older code observed a separate stale
  `_target` test reference. Parent reproduced the single latest-code case in
  17.00s and confirms baseline `f2751f58` already lacks `macros.transactions._target`
  used by `test_application_review.py:320`. This unrelated test/facade is not repaired
  here; its failure is neither concealed nor attributed to submodule acceptance.
- Final independent checks: adverse probes passed in 16.69s, including ignored
  ancestors, newline names, selected descendants and unchanged-file checks; two
  existing corrupt-index rollback cases passed in 23.86s; the authorized 600-line
  ceiling check passed in 0.06s (parent's earlier focused check: 0.05s). The original
  six probes passed again, including zero external MIDX/config/ref accesses,
  packed-replacement/artifact rejection and late nested-drift detection. No test
  inventory was added; parent owns the authorized existing-test/standards edits.
- Final parent suite `83201` terminated with exit 0: **196 passed, one known
  untracked-mode case deselected in 469.58s**. That exclusion remains unresolved.
  Final real-root round trip `30681` terminated with exit 0, **PASS in 172.88s**:
  2,144 entries, ten root modules plus nested dependencies. Earlier capture `58789`
  rejected spimdisasm drift; its exact cause remains unproven.
- Broader suite `44396` terminated with exit 1: **147 passed, one pre-existing
  `_target` failure in 3,423.51s**. It began before final repair and is historical,
  not final-revision authority. Final review also records passing Ruff/format and
  whitespace, with matching code fingerprints after review.

After the repair-one real round trip terminated, parent froze code and granted the initial sidecar
the serialized slot for this plan, `docs/agents/harness.md` and
`docs/agents/macros.md` only. The writer read all 3,121 original plan lines before
editing. One initial compact documentation edit and one final evidence revision
after parent supplies review results are allowed; cutoff 06:50 UTC, cleanup 06:52.
Parent subsequently permits the final revision at 06:49 with honest pending status
if no review arrives; parent supplied the repair findings during this final revision.
That consumed its reserved allowance without accepting the candidate. Its final edit
completed at 06:50:36, a recorded deviation from the unchanged 06:50 work cutoff;
46 plan tests and 64-reference checks passed before 06:52, then its handle closed.
The parent's separately authorized 600-line policy/test edits remain outside either
sidecar's three-file ownership. Initial documentation validation passed 46
existing plan tests in 2.62s, plan parsing, 64 references and whitespace checks;
these remain historical checks.

The distinct terminal-accounting sidecar read all 3,217 current plan lines and both
guides while read-only, then received the parent's sole serialized authored-write
slot after final outcomes and narrow acceptance. It owns one compact revision to
these same three documents only, with work cutoff 07:10 UTC and cleanup 07:12,
inside the unchanged parent 07:10/07:13 bounds. Required handoff checks are existing
plan tests/parsing, references for changed Markdown plus documentation indexes and
scoped whitespace; their actual outcomes accompany slot release. This is accounting,
not another implementation/review budget or permission to resume the goal.
No agents, source/worktree/dependency writes, native builds, index refreshes or
macro retries belong to this sidecar. Preserve `.pi/settings.json`, both dirty
battle sources and unrelated `tmp/`.

At that tooling checkpoint C and macros remained parked; the real macro's one pre-publication failure, six
unchanged PRE states, absent header/application, original pins and expired
04:45/04:48 bounds remain. Four accepted clean-C dispatchers and 46 outstanding
requeue entries, stable IDs/states, frozen-five membership and all unfinished
domain obligations survive. Parent then confirmed the whole reconstruction goal was
**paused**; the explicit continuation below supersedes that historical pause. Aggregate plan states remain
unchanged. This narrow tooling acceptance closes no phase, grants no source/macro
acceptance and launches no successor.

<a id="battle-dispatch-macro-reopening--2026-09-13"></a>
### Explicit macro reopening — 2026-09-13 (filesystem blocked)

The user explicitly resumed the full goal and reopened the existing
`out/reviews/battle-dispatch-macro-20260913/mission.md` after supported submodule
prerequisite commit `452498ad`. The same `assembly_block:366ad828049fbe14`, floor
12, pool three, top one, four dispatchers and six authored paths remain frozen.
Original candidate/ranking/proposal/changes pins and the failed `-01` launch under
expired 04:45/04:48 UTC bounds remain history, without refund or rebinding.
New bounds are 08:10 UTC work / 08:15 cleanup: one resumed owner entry, one PRE
reconfirmation, one distinct POST review and one bounded clarification each; at
most one review-driven proposal repair before launch only, never automatic retry.

Parent readiness confirms all six original PRE states, unchanged proposal/changes,
the original ranking pin with one selected lead and adopted baseline
`v1:5ff94dfa4e811c12c321a64bbfa295df531e60fb45b485b61e2c6bf5fff321c3`.
Current preparation is retained separately as
`out/reviews/evidence/battle-dispatch-manifest-reopen-20260913.json`; original
proofs are not overwritten. Four fresh parent native PRE checks each pass 32/32
instructions and 128/128 bytes. Independent Kierkegaard
`01a099a1-aad9-7bd3-8d2a-fe926626382c` reconfirms all eight guards, parameter
mapping, complete four-consumer coverage and human value in
`out/reviews/evidence/battle-dispatch-pre-review-reopen-20260913.md`.
No proposal repair was needed; that handle is closed. This is PRE approval only.

`reopen-run.log` in the mission directory retains a wrong-option argument-parsing
failure before owner entry; parent checked CLI help/source and unchanged PRE.
It is a separate failed invocation, not another owner entry or refunded attempt.
Actual owner session `28025`, run `battle-dispatch-macro-20260913-02`, subsequently
terminated exit 2 around 07:34 UTC. `reopen-owner-run.log` reports
`prepared POST changed: config/targets/emi/battle/battle/15/target.toml`.
The sole resumed owner entry is consumed; no retry or POST review followed.

Independent Erdos `01a099b5-a9d8-7b20-85f0-99f56fb1c558` reproduced the cause;
its closed review is retained in the mission's `reopen-failure-review.md`, read by
this writer. On actual `fuseblk`, same-leaf `RENAME_NOREPLACE` returns `EEXIST`,
falsely passing the capability probe; distinct same-directory and cross-directory
moves to absent destinations return `EINVAL`. The `/tmp` ext4 control succeeds,
proving that invocation only, not checkout portability. `atomic_write` falls back
to hard links and retains its temporary alias: correct proposed bytes have two
links, so the prepared-image single-link guard correctly rejects them.

Failure occurs while `capture_recovery` prepares its first image, before source
publication or a recovery record. Empty outer rollback maps do not establish
restoration of applied source. Failed-stage forensics remain; no new recovery
record, owner command receipts, application or header exists, and no POST acceptance
is claimed. All six PRE hashes/absence states remain exact. Parent's fresh
after-failure baseline session `75882` exits zero with the identical adopted digest
above. Earlier native PRE receipts remain PRE evidence only.

At that failure checkpoint the macro awaited an explicit filesystem decision. Fixing the false
positive preflight only improves diagnostics; installation/quarantine/restoration
still needed unsupported moves. No link-guard relaxation, replacing rename,
check-then-unlink workaround or fixture portability claim was accepted. Parent
requested a persistent ext4 working copy under home preserving the original
checkout, dirty work and submodules; authorization was pending and no copy/move
occurred. Reported readiness is approximately 11 GiB excluding `out/build/.venv/tmp`
and home ext4 with 1.1 TiB free, not permission. Any authorized destination still
needed actual capability testing and reconciliation of root-bound evidence.
The user subsequently refused migration and authorized only the in-place
cooperative publication correction below; this historical request is closed.

The full reconstruction goal remains **active and incomplete**, not globally
blocked by this mission. Game01 is parked with zero C/native/application attempts
and no source edits; its two candidates remain unreviewed. All 50 requeue entries,
four accepted clean-C sources, 46 outstanding entries, stable IDs/states, frozen
five and full naming/type/macro/combiner obligations survive. AF66C and all earlier
tooling allowances remain expired/consumed as recorded. Future macro acceptance
still requires owner-controlled application/recovery, all native/target gates,
distinct POST review, a parent-bound envelope and live final verification.

This sidecar read all 3,262 prior plan lines before its sole serialized final edit
to this plan and `docs/agents/macros.md`. Existing plan tests/parsing, changed-doc
and index references, and scoped whitespace checks accompany slot release; no
source/native/index/Git writes, agents, new tests or migration belong to this edit.
The unchanged 08:10/08:15 bounds grant no replacement implementation allowance.

### In-place NTFS publication — 2026-09-13 (reviewed tooling checkpoint)

Distinct mission `out/reviews/ntfs-publication-20260913/mission.md` starts from
`9331af91`. The user refuses migration and explicitly permits the relevant
publication-guard relaxation to replace retained-hard-link publication in place.
Original work cutoff is 08:45 UTC, cleanup hard stop 08:50: one implementation
pass and one review-driven repair, with independent design advice and final
review/recheck. The implementation and single review-driven repair are consumed;
no allowance resets. Parent froze repaired code at 08:25 UTC. Bernoulli
`01a099c3-0c38-7180-befb-75d57f41728f` returned final **ACCEPT**, observed through
the parent's actual wait. Parent accepts only the six repaired files bound by
`repaired-code.sha256` in the mission directory, independently reviewed and matched
by this writer. Final checks below resolve the filesystem tooling blocker, not
macro application or any aggregate phase.

Parent owns exactly `tools/python/harness/common/{rename,files,git,images}.py`
and adaptations to existing `tools/python/tests/test_transaction_files.py` and
`tools/python/tests/test_application_revalidation.py`.
No new test inventory, dependencies, home/mount migration, settings or sandbox/
approval changes belong to this scope. The [publication contract](../agents/harness.md#file-publication)
owns strict native-first publication and the unsupported-filesystem cooperative
reservation/check/rename fallback. Source and prepared images retain one link;
the retained-alias return field and false-positive same-leaf capability probe
are removed. Atomic creation, quarantine/restoration, images and every Git-index
move use the central publisher. Shared `describe_locations` supplies files/Git
failure diagnostics, including adjacent restoration catches, without asserting
no mutation or retained source after a post-move error. `safe_unlink` classifies movement by device/inode,
mode, link count, size and mtime, excluding rename-sensitive ctime.

The explicit relaxation admits external check/rename and check/cleanup races,
visible empty reservations and crash leftovers. It provides no atomic CAS against
noncooperating writers. Native collision behavior, confined paths, source identity,
writer exclusion, submodule protections and guarded owned rollback remain required;
detected foreign names are preserved. Filesystem compatibility does not broaden
workspace/Git restoration authority or make recovery automatic.

Initial independent review required three repairs: cleanup could remove a captured
nonempty or multiply linked reservation; Git post-move fsync errors falsely claimed
retained source; an existing revalidation case still patched the retired publisher
and message. The single repair requires an initially regular, empty, single-link
reservation plus unchanged descriptor/path identity before cleanup, shares honest
location diagnostics across files/Git catches, and migrates only the existing test
seam/message, preserving behavioral assertions. Zero length alone never authorizes
deleting crash residue. Bernoulli confirms all three findings closed and all six
fingerprints matched: both altered-reservation reproductions preserve destinations,
five Git post-move reproductions report uncertainty/locations without descriptor
leaks, and 25 live-NTFS cases pass with the native-only case deselected. Its
whitespace check passes; these are parent-relayed independent results.

Validation distinguishes initial and repaired code:

- Initial transaction cases: **26 passed in 2.71s**. Owner/DRY session `52362`
  terminated exit 0: **170 passed, one deselected in 510.45s**; 196 focused passes
  in total. The known untracked-mode Git-object-writing exclusion remains unresolved.
- Initial actual NTFS probe `18073` completed; repeat `73849` was subsequently
  confirmed terminal exit 0. This writer read all **12 PASS lines** in
  `/tmp/ntfs-publication-probe.txt`: alias-free creation; original inode/mode
  preservation through replacement/quarantine/restoration; identity-bound apply/
  rollback including new-file absence; cancellation after first publication with
  owned PRE restoration; Git-index restoration; success, collision, pre-error,
  foreign-reservation, post-error and cancellation cases; fixture removal.
  Two earlier disposable fixture-only API corrections (restore keyword and staging
  key) changed no production code and are not separate successful probes.
- Final repaired owner/DRY session `25098` terminated exit 0 at 08:35:47 UTC:
  **170 passed, one deselected in 478.71s**. Final targeted session `66706`
  terminated exit 0 around 08:39: **32 passed, 40 deselected in 350.61s**: 26
  transaction-file cases and six existing type/macro direct/symlink/hardlink
  late-output collision variants. This writer read
  `/tmp/ntfs-publication-repaired-owners.txt` and
  `/tmp/ntfs-publication-targeted-revalidation.txt`. These **202 final focused
  passes** are distinct from the initial 196; the known owner/DRY exclusion remains.
- Parent intentionally interrupted broader 72-case revalidation/file session
  `99580` with Ctrl-C within the original budget; terminal exit 130 at 08:33.
  Partial passing progress is neither a completed 72-pass run nor a reproduced
  test failure. The narrowed 32-case check adds no coverage or implementation
  allowance; full-suite and native game-owner validation are not claimed.
- Final repaired actual NTFS probe `37834` terminated exit 0 with all 12 PASS
  lines, parent-confirmed. These disposable filesystem/transaction probes prove
  their bounded cases, not native BOF3 compiler or macro application acceptance.

This sidecar read all 3,337 prior plan lines and both guides, checked their retained
hashes and received the parent's sole serialized documentation-write slot while
code remained frozen. Its initial compact revision to this plan,
`docs/agents/harness.md` and `docs/agents/macros.md` landed before the repair
findings; the writer immediately released the slot for parent repair. Read-only
plan session `64126` then passed 46 tests in 2.54s; parsing, 146 local references
and scoped whitespace passed. After independent acceptance and actual final test
outcomes, parent granted one final compact accounting revision to these same
three files. Existing plan tests/parsing, changed-document/index references and
scoped whitespace are rerun at handoff within the original 08:45/08:50 bounds. No
agent launch, source/native/index/Git write or new test belongs to the sidecar;
preserve `.pi/settings.json`, both dirty battle sources and unrelated `tmp/`.

No macro is applied or accepted; this stage includes no C or native compiler work.
Any subsequent macro owner entry needs separately recorded finite scope, fresh
readiness and review. Failed `-01` and `-02`, original pins, consumed/expired
04:45/04:48 and 08:10/08:15 bounds remain intact, without automatic retry. The
full goal stays **active and incomplete**: all 50 requeue entries, four accepted
clean-C sources, 46 outstanding entries, frozen five, stable IDs/states, prior
history/budgets and all naming/type/macro/combiner obligations survive. This
reviewed tooling checkpoint completes no aggregate phase.

### Transaction validation performance — 2026-09-13 (reviewed tooling checkpoint)

The user redirected validation to identifying and fixing extreme bottlenecks,
starting from accepted NTFS checkpoint `227f7394`. This separate tooling mission
retains its **17:45 UTC work / 17:50 cleanup** bounds; the serialized docs sidecar
has **17:40 / 17:45** bounds. One measured implementation pass is consumed; Boole's
actual final review at 16:50 accepted the exact three frozen fingerprints without
findings, so no repair pass was consumed. Reviewer:
`01a09ba1-4e35-74b3-9163-dd1850d7f926`. Only
`tools/python/harness/common/{children,process,repositories}.py` changed in the
implementation; no tracked test inventory, C, metadata, configuration, dependency
or Git-policy change belongs to this scope.

The [harness contract](../agents/harness.md) retains subreaper ownership, cleanup
ACK, absolute deadlines, output bounds, environment rejection, live HEAD/index/
worktree checks, confinement, submodule exclusion, leases, guarded rollback and
the accepted cooperative single-link NTFS publisher. Libc pidfd readiness removes
the normal direct-child polling delay, with the old polling fallback on unsupported
or denied capability. Configuration parsing is skipped only for an unchanged
post-query recapture of live controls, identities and hashes, never initial
validation or through a stale cross-transaction cache.

Identical existing
`test_shared_application_pre.py::test_shared_pre_from_two_fresh_private_owners[type]`
ran under `python -m cProfile -m pytest` before and after, without assertion edits.
Sessions `75698` and `32218` both terminated exit 0 (one passed, one deselected):

| Measurement | Before | After |
| --- | ---: | ---: |
| Test elapsed seconds | 498.45 | 181.38 |
| Bounded subprocess calls / cumulative seconds | 5,011 / 444.973 | 4,765 / 138.301 |
| Configuration parses / cumulative seconds | 738 / 64.323 | 492 / 15.570 |
| Fresh `read_git` calls | 3,310 | 3,310 |
| `collect_entries` boundaries / cumulative seconds | 435 / 236.589 | 435 / 74.466 |
| Guarded Git median milliseconds, 20 samples | 83.006 | 27.809 |
| Owned `/bin/true` median milliseconds, 20 samples | 77.587 | 28.775 |

This profiled workload is **2.748x faster, 63.6% lower elapsed time**, not a
whole-pipeline speed claim; nested cumulative times are not additive. All 246
removed subprocesses are redundant configuration parses. Parent non-profiled
shared-PRE type/macro cases separately measured 487.58/483.25s before versus
152.46/149.07s after. Remaining overhead includes 3,310 fresh Git calls and 435
capture boundaries; future batching needs equivalent live-boundary proofs, not
unsafe caching across writes, callbacks, native gates or transactions.

Parent terminal accounting covers the original **1,912 unique existing nodes**,
with no duplicate completed nodes or exclusions:

| Stage / actual handle | Terminal result |
| --- | --- |
| Broad 69 modules, `5016` | exit 1: 1,565 passed, 13 failed, 2 skipped, 30 setup errors; 412.01s |
| Private owners, `76959` | exit 0: 24 passed; 442.25s |
| Six owner files, `16793` | original 1,500s cap, exit 124 at 17:14: 226 passed, one failed |
| Only remaining 51 owner nodes, `65952` | exit 0: 51 passed; 77.89s, separate 600s stage within original mission bounds |

The parent matched 227 named completed owner results to the collected prefix;
the unnamed interrupted XML entry is not a pass. The continuation includes that
interrupted case but replays no completed case and does not reset the original
timeout. Raw unique totals are **1,866 passed, 14 failed, two skipped and 30 setup
errors**, not a full-suite PASS. Baseline broad validation was 1,566 passed,
12 failed, two skipped and 30 setup errors in 1,078.85s. Its additional optimized
raw failure was live `asm-diff` rebuilding a formerly cached object and hitting
SIGSYS 159. Approved native classification `19621` terminated exit 1 with 21
passed and one stale local-`result` assertion in 3.31s: five compiler-dependent
raw failures clear there, without erasing their original results. Nine
unresolved failures and 30 fixture setup errors remain, classified as legacy
test/API debt (including the missing `transactions._target` macro-review helper),
missing claims, wrapper expectations and the protected real `.git` write; none
was attributed to this optimization or repaired in this scope.

Fresh approved native all-source audit `56113` terminated exit 0: **784 exact,
136 partial, zero invalid**, without C/config edits. Four existing managed-process
tests passed in 0.42s; independent review passed 24 helper/lifecycle and 14
configuration-drift probes, including failure withholding ACK. Real NTFS repeat
`90530` terminated exit 0 with all probes passing and its disposable fixture
removed. Parent full Ruff, focused formatting and whitespace checks passed.
The earlier full `just check` handle `15603` was explicitly interrupted on user
redirection at 16:37, exit 130, not passed. Existing symbol debt remains
`emi/battle/battle/03:D_801EB4F0`; these checks establish no whole-pipeline acceptance.

Disposable provenance is under
`out/reviews/transaction-performance-20260913/`: `mission.md`, `review.md`,
`implementation.sha256`, `before.txt`, `after.txt` and the now-populated
`tests.json`. The sidecar read its matching final totals; parent verified exact
node-set equality after correcting XML class-method identity formatting, without
changing test outcomes. The docs sidecar read the complete
3,441-line selected plan before its single compact revision to this plan and
`docs/agents/harness.md`; existing plan/parser/reference and preservation checks
are reported at handoff. All earlier failed macro bounds, histories, structured
states, the full 50-entry requeue (four accepted, 46 outstanding), frozen five and
remaining naming/type/macro/combiner obligations remain unchanged. The macro is
still unapplied and requires separate later readiness; the full goal remains
**active and unfinished**. This accepted performance checkpoint completes no
aggregate phase and grants no unrelated repair or macro retry.

### Existing-fixture repair — 2026-09-13 (reviewed pytest checkpoint)

The user's separate request to resolve the nine remaining pytest failures and
30 fixture setup errors starts from `160f86f3` at **17:29 UTC**, with original
**19:00 work / 19:05 cleanup** bounds: up to three diagnosis/fix cycles per root
cause, one independent review and one correction. This does not renew any earlier
mission allowance. Boole `01a09ba1-4e35-74b3-9163-dd1850d7f926` accepted the exact
seven frozen test/fixture hashes with no confirmed weakening or requested review
correction. Eight reviewer-selected existing tests and isolated Git/token probes
passed; AST comparison found no added or removed test functions.

Parent changes only `tools/python/tests/naming_synthetic_fixture.py` and existing
`test_{application_review,harness_dry,naming_evidence_root,scratchpad,
toolchain_perf_budgets,wrapper_bootstrap}.py`. Repairs explicitly declare
`support_sources` before synthetic index/evidence construction; use canonical
`IndexWorker`, `native.OUTPUT_BUDGET` and `macro_owners.resolve_target` owners;
isolate Git object/index writes in temporary storage while preserving real-root
inventory/mode assertions; and update wrapper inventory/bootstrap expectations.
Scratchpad coverage uses deterministic temporary C through the real SDK
preprocessor, proves the header-name collision and retains referenced declarations
while removing irrelevant context. Token-aware regexes correct macro-expansion
spacing assumptions. No compatibility facade, production-gate relaxation, compiler
substitution, C/config change or source/macro transaction belongs to this repair.

All seven final disjoint groups reached actual terminal **exit 0**, parent-reported:

| Group | Final result | Seconds |
| --- | --- | ---: |
| Broad | 1,504 passed, two skipped | 417.46 |
| Application | 58 passed | 925.05 |
| Review/private | 84 passed | 819.38 |
| Macros | 60 passed | 124.97 |
| Types | 100 passed | 146.00 |
| Approved native | 55 passed | 3.63 |
| Wrappers | 50 passed | 2.90 |

Parent's exact XML/current-collection comparison accounts for **1,913 unique
nodes: 1,911 passed, two skipped, zero failures or errors**, without duplicates
or exclusions. All original 1,912 nodes remain; the current `agent-run` wrapper
adds one case through existing parametrization, not a new test function. Both
existing forensic skips lack disposable inputs. Initial focused terminal-fixture
validation passed 37 cases; the first native group had 54 passes and one whitespace
assertion failure. Its `native-before.*` evidence remains historical and is not
double-counted after the final 55-pass rerun. The nine pytest failures and all
30 setup errors close at this checkpoint, not retroactively in prior runs.

Parent fresh native source validation reports **784 exact, 136 partial, zero
invalid**; full Ruff, changed-file formatting and whitespace pass. Separately,
`bin/symbols check` still exits **2** for `emi/battle/battle/03:D_801EB4F0`, debt
introduced in `604b567c`. Do not waive this baseline, invent a semantic name or
claim composite `just check` is green; identity repair needs its own evidenced
authorization. Pytest closure is not whole-game or aggregate-phase acceptance.

The sidecar read all 3,536 prior plan lines, then the mission's `mission.md`,
`review.md` and `tests.json` under `out/reviews/fixture-repair-20260913/`, and
matched all seven `implementation.sha256` pins. Its sole serialized plan edit
retains **18:05 work / 18:10 cleanup** bounds; existing plan/parser/reference and
preservation checks accompany handoff. No other authored sidecar file or Git
write is authorized. Preserve `.pi/settings.json`, both dirty battle sources,
unrelated `tmp/`, all prior history/budgets, structured states, the 50-entry requeue
(four accepted, 46 outstanding), frozen five and every remaining domain obligation.
The macro remains unapplied, with no retry or backlog reset; the full goal remains
**active and unfinished**.

### Reviewed DATA proposals — 2026-09-13 (tooling accepted; production unapplied)

The user explicitly authorized missing DATA proposal support from `deb3d5cf`,
separate from production identity application. Mission
`out/reviews/data-proposals-20260913/mission.md` retains its **18:38 UTC start,
20:00 work / 20:05 cleanup** bounds: up to three implementation/validation cycles
per root cause, one independent findings pass and one correction review. Parent
owns preparation/data guards; Curie `01a09c10-a6ad-7291-9fa3-efe624e7d5a0` supplied
four postapply/review owners through a serialized slot. Final scope is nine files
under `tools/python/harness/naming/`: new `data.py` plus `audit.py`, `context.py`,
`proposal.py`, `inputs.py`, `application.py`, `reports.py`, `review.py` and `cli.py`.
No tests, dependencies, production C/config changes or identity transaction were
added; local feature commit remains parent-owned and pending at this checkpoint.

The [native lifecycle contract](../agents/tool-usage.md#frozen-naming-postapply-lifecycle)
now documents human-reviewed DATA candidate preparation with frozen-initializer
report CAS and distinct `proposed-data-transaction/v1` provenance. FUNCTION wire
format/`KIND`, single-source restrictions and automatic `conclude` typed-exhaustion
admission remain unchanged; the unsupported proposed-analyzer seam is not widened.
Complete owned PRE/POST hashes, modes and canonical storage bind spelling-only
changes without file moves, representation/qualifier/extent edits or normalization
byte changes. Parent reports 223 pinned owned inputs for the real candidate;
snapshot coverage cannot narrow to renamed files. Every exact direct consumer's
source/name/address/size determines its own asm-diff and byte-match gates after
four base gates; the data address is never treated as a function.

Independent Maxwell `01a09c19-8f13-7132-a709-8325627090e0` accepted the corrected
DATA core: four initial findings and two remaining variants closed within the
review cycle; 16 probes yielded 14 expected rejections, two positive controls and
zero unexpected outcomes. Splicing/rename commutation and spliced collision checks
prevent joined-identifier omission; plain externs exclude wrappers/initializers,
bindings require standalone literal-address `WEAK_SYMBOL_AT`, and call arguments
(including indirect calls), keyword macros, cross-owner/partial/runtime and
consumerless scopes reject. A subsequent disposable multi-function-C preparation
probe exposed the old bulk metadata adapter's whole-file source-tag assumption.
The root repair uses `collect_lift_metadata` per address with one file read,
preserving FUNCTION's single-source guard. Public DATA preparation/preservation
then passed; Maxwell accepted eight focused adapter checks covering legacy and
two-record inputs, scoped progress errors, structural/missing/unreadable metadata
and FUNCTION refusal. All nine final `implementation.sha256` pins match.

Five disjoint existing-inventory groups reached actual terminal **exit 0**:

| Group | Final result | Seconds |
| --- | --- | ---: |
| Broad | 1,554 passed, two existing forensic skips | 430.60 |
| Application | 58 passed | 925.79 |
| Review/private | 84 passed | 820.80 |
| Owners | 160 passed | 254.53 |
| Approved native | 55 passed | 3.56 |

Parent's exact XML/current-collection comparison and populated `tests.json`
account for the unchanged **1,913 unique nodes: 1,911 passed, two skipped, zero
failures/errors**. Both skips lack existing disposable forensic inputs. After the
last DATA guard and metadata repairs, all naming/harness-DRY checks separately
passed **653 cases, two skipped in 37.04s**; this overlapping rerun is not added
to the inventory total. Earlier focused checks likewise are not extra coverage.

The public producer → gates → parent review → public verify disposable probe
passed two-consumer/eight-gate and negative cases using **injected execution**;
explicit rollback restored PRE, frozen report and index. Separate actual native
PRE/POST temporary copies retained an 84-byte function, every allocated ELF
section and normalized relocation identity unchanged, with live C untouched.
`native-rehearsal.json` retains that proof; a failed optional Python ELF import
was followed by existing toolchain `readelf`, not a dependency installation.
Fresh parent all-source validation reports **784 exact, 136 partial, zero invalid**.
These bounded proofs do not establish a production DATA rename or whole-pipeline
acceptance.

The authorized battle03 report regeneration archived its predecessor and produced
191 structurally valid rows, not semantic closure. `D_801EB4F0` remains raw;
`battleDispatchSlots` is still an unapplied candidate requiring a separate
evidenced production transaction. The symbol gate remains failed and composite
`just check` is **not green**; no baseline waiver or invented name follows.
The serialized docs sidecar updates only this plan and `docs/agents/tool-usage.md`
by 19:40, with existing plan/parser/reference checks at handoff. Preserve every
prior plan byte, state, budget, the 50-entry requeue (four accepted, 46 outstanding),
frozen five, unrelated dirty work and all remaining domain obligations. The macro
remains unapplied with no retry or queue reset; the full goal stays **active and
unfinished**.

### Production naming and report generations — 2026-09-13

Mission `naming-production-20260913` starts at **19:31 UTC** from `abb2f1df`,
separate from the historical DATA-tooling mission above. Its original **20:25 work
/ 20:35 cleanup** bounds remain unchanged: at most two production transactions,
one source application and one bounded repair each. Read-only queue scout Hubble
`01a09c40-b0dc-7d23-8a47-27bab5266365` completed by 19:41; no children were used.
Parent serializes runtime, source, native refresh and documentation writers.

**First DATA transaction accepted:** parent public
`out/reviews/naming-production-20260913/data-verify.json` returns `applied:true`,
one row, target `emi/battle/battle/03`, transaction `data:D_801EB4F0`.
The spelling is now `battleDispatchSlots`, with six real native gates and distinct
independent POST review accepted. This supersedes only the preceding checkpoint's
unapplied-candidate/symbol-debt status, not its historical results. Parent reports
fresh independent DATA re-review accepted at **20:16:04 UTC** after the runtime
repair: all 223 physical PRE copies, 1,803 input-closure identities, six real gates
and current index/readiness checked. The reviewer finished four seconds beyond
its requested cleanup node, within the original 20:25 work cap; no limit reset or
old-bundle rebinding follows. Composite `just check` session `83024`, started 19:42,
failed at 20:25:39; its exact terminal status is recorded below.

Two autonomy gaps were diagnosed and repaired within this mission. First,
`inputs._SHAPES` omitted the `role` emitted by indexed command collection; the
repair admits only the existing four enumerated roles and rejects invalid roles,
without weakening the closed schema. Second, accepted DATA left a prepared row
that ordinary reconciliation refused as extra inventory. The new explicit
[report-generation lifecycle](../agents/tool-usage.md#accepted-naming-report-generations)
previews and CAS-publishes a successor while retaining immutable predecessor
proof; it is implemented and independently accepted, not merely a deferred design.

Mill released the runtime writer at **20:11:34 UTC**. Hubble's original review
rejected incomplete embedded proof membership (P1) and blocking lock acquisition
(P2) before 20:10. One authorized repair recheck, cutoff **20:20**, closed both:
shared `collect_evidence_states` now derives complete proof membership and embedded
bindings, including physical PRE copies; report acquisition uses `LOCK_NB` and
deadline checks. Independent memory-only probe session `45091`, chunk `8e4ccc`,
passed complete/restored controls, rejected 21 omission/missing/rebased-evidence
cases over seven artifact classes, and passed three contention/deadline controls.
Fresh tool chunk `f5834d` matched all eight runtime release pins, including the
unchanged role repair. Final review **ACCEPT** is retained in
`out/reviews/naming-production-20260913/checkpoint-review.md`, SHA-256
`29394fcf083c259488ceb98d9d12cf7b2062a0ba4e41c26dbbb8638422d56a01`;
the original rejection remains historical. These consumed repairs reset no budget.

Parent reports **291 passed, two skipped**, Ruff/diff passing at writer release,
then a **653 passed, two skipped** naming rerun in 39.43 seconds. Its disposable
public DATA→FUNCTION two-generation probe passes preview/CAS/replay, stale pins,
rejected verification, corrupt backups, interrupted activation/orphan retention,
changed-orphan refusal and advancing-draft preservation. Native execution in that
fixture is explicitly **injected**, not production proof. The fresh actual native
gates and independent re-review above remain distinct from this fixture.

**Production report checkpoint published at 20:23:04 UTC:** parent apply session
`13398` exited zero after fresh review ingestion/public verification and preview.
Sidecar tool chunk `ec3bbe` read
`out/reviews/naming-production-20260913/data-finalize-apply.json`:
`disposition:published`, `current_acceptance:true`, `production_complete:false`,
190 rows, `complete:false`. The retained preview plan SHA-256 is
`8439fe09b5ee7f1d4d50f4c1872e1319ac5d1b0afac30096cb961c880fd63198`;
the active report is
`out/reviews/plan-audit-naming/emi__battle__battle__03.generation-9dd5be5b1747750372c25977b65582da63a210aaf6dcf2ce356dd4d7527aa231.json`,
SHA-256 `4dfd7c899344a7fc2b3bd5cae961e3800b0f00c30ff15813eeb00549b015239c`.
Parent confirms resolver and exact retained 190-row equality checks; the preview
retains 246 proof artifacts. S3.7 accepts this actual DATA activation together
with the separately labeled two-generation fixture, not a second production name.

The conditional second candidate `emi/battle/battle/03@0x801DE804` is canonical and
unflagged but remains unapplied. Parent started a bounded 75-second
`--instructions` collection against the new active report, previously blocked on
the accepted predecessor; collection does not authorize terminalization, source
application or report edits. Before 20:25, sidecar chunk `a5602c` read the completed
`next-function-collection.json`: no errors, one executed/one resumed command,
one completed row, `terminalized:false`, 2.574 seconds, manifest SHA-256
`582eb139aaf32ceafa737e82a199fdbb368c2ea447209a4ffab8e769ba456aee`.
Parent confirms collection exit zero with no proposal or source edit. This is
next-transaction evidence only, not another accepted name. Composite `just check`
terminated at **20:25:39**: **two failed, 1,909 passed, two skipped**, 2,537.71 seconds.
Failures are `test_wrapper_bootstrap::test_safe_path_blocks_caller_cwd_python_packages`
and `test_wrapper_bootstrap::test_agent_context_system_fallback_and_invocation_modes`.
Cause isolated during cleanup: campaign's module-level history import reached
`common.inputs` → `domain.layout` → `yaml`, breaking the intentionally stdlib-only
system-Python worker fallback. Parent moved only the five history imports inside
`if history is not None`; no validation, fallback safety, dependency installation
or C source change. This is a separate library-load cleanup fix after production
activation, not another source attempt or budget reset. The original **20:35
cleanup cutoff** remains fixed.

Hubble independently **ACCEPTS** only new campaign SHA-256
`90647ecf34d3353af03d68f65a6cd7eb2db3054fdf90a75053e0bdc82c4f6fae`.
Old bytes remain in `out/reviews/naming-production-20260913/campaign-before-bootstrap.py`,
SHA-256 `9bf7a7b550eefdd92e4a654a25cc2950f8cf45fe208abedd3e0aac0658addc4c`.
Tool chunk `e3f5d4` verified the exact retained delta and system Python `-S` worker
exit zero. Independent read-only probe session `49493`, terminal chunk `a39e7c`,
proved AST identity except import placement, legacy resolution without loading
history/YAML, fail-closed dependency failure whenever history is present, and
actual published-history resolution at the unchanged successor hash above.
Historical DATA receipts, native proof, 20:23 activation and original runtime
review artifact remain unchanged; this new acceptance does not rebase them.

Parent reports the two-generation/interruption/corrupt-backup public fixture and
Ruff check/format passing after cleanup. Focused wrapper/naming/dry-run union
session `70156` completed at **20:28:16**: **703 passed, two skipped**, 42.31 seconds,
including both previously failing wrapper cases. Combined test accounting is
1,911 passed/two skipped across the initial run and focused recovery, **not a
second full `just check` invocation**. All `tools/python` Ruff and symbol checks
pass per parent; final all-sources-native stage `55743` completed at **20:31:26**,
exit zero: lifts **exact=784, partial=136, invalid=0**.
`runtime-final.sha256` records the cleanup pins while the original
`runtime.sha256` remains unchanged. Retain the initial full-run failure record;
focused recovery alone is not complete phase validation.

Outstanding inventory after the first DATA rename is **1,361 raw names: 530
functions and 831 data across 23 targets**; battle03 has **190: 86 functions and
104 data**. The scout baseline was 1,362 raw names versus 1,363 report rows
(1,361 blocked, one exhausted, one historical proposed row). Do not conflate row
counts with semantic debt or accepted no-ops. Old battle15 summary drift remains:
249 summarized versus 250 report rows, with the historical accepted function row
still retained; unfiltered validation first rejects `func_8009DC6C` metadata.
Battle03 preflight's **176 safe metadata repairs and two review-required** entries
are actionable legacy metadata debt, not parser drift: noncanonical residual
wording and missing match/residual fields remain independently of the selected
DATA/function. No bulk repairs were authorized or performed by this sidecar.

Preserve all frozen obligations: the five-entry pilot remains blocked with zero
new accepted no-ops; local `D_80096994` structural exhaustion is not semantic
acceptance; the two `1F800044` type controls and exact macro group
`8e1ad03b4ba92303` remain open. Distinct dispatch macro `366ad828049fbe14` remains
unapplied with its two consumed failures and expired limits. The clean-C queue
still has four accepted and **46 outstanding** of 50; old matching/permuter and
macro budgets are not restarted. `S3.2`, S4 and consolidation obligations remain
unfinished. Selected-row acceptance does not replace unfiltered full-target
`complete:true`, separate identity approval or the whole-game completion gates.

<a id="successor-function-action--2026-09-13-checkpoint-accepted-commit-pending"></a>
### Successor FUNCTION action — 2026-09-13 (checkpoint committed)

Parent's observed action start is **20:37:16 UTC**, after accepted DATA/report
checkpoint `d2bbb57c`. Mission `out/reviews/naming-function-20260913/mission.md`
retains **21:00 work / 21:05 cleanup** cutoffs. The preceding production mission's
20:25/20:35 bounds are expired and closed; its one-row collection took 2.573579s,
with `terminalized:false` and no FUNCTION proposal/application attempt consumed.
This distinct stage queues only `emi/battle/battle/03@0x801DE804` for
`deactivateBattleDispatchSlots`: one proposal/application and one bounded repair,
without renewing any previous allowance.

Gauss accepts the selected semantics and corrected proposal in this mission's
`semantic-review.md`, rechecked at 20:45 before its original 20:47 review cutoff.
The one wording repair is consumed: byte 2 remains an additional cleared byte
of unknown role, and the callee initializes its own counter rather than depending
on incoming argument values. Corrected candidate SHA-256 is
`d753669cb8376c81bb9e611297de2c87cc10ad966786083d54c4f87e4fa10e1a`;
the rejected candidate and original receipts remain historical evidence.
This is proposal acceptance, not application acceptance.

Parent baseline receipts establish 21/21 instructions and 84/84 bytes exact.
The sole source application, including required source relocation, has occurred;
parent reports all six native FUNCTION gates pass. The sidecar read
`baseline-bytes.json` and the `gates.json` bundle pointer; execution and six-gate
success remain parent-attributed. Parent confirms index recovery and fresh
readiness pass, plus **673 existing naming, harness-DRY and target-manifest tests
passed, two skipped in 38.86s**. The configured-compiler target audit's populated
`target-status.json` records **179 exact, 38 partial, zero invalid; 217 lifts**,
with no coverage error. Its original exec handle was lost at compaction; this
artifact establishes reported counts, not an observed terminal exit. No full-suite
or composite `just check` pass is claimed for this source-only identity step.
Gauss's actual `application-review.md` records **POST ACCEPT** at 20:54, before
the 20:56 handoff; SHA-256
`75687f13a767d6d5fc23e9a1c17c31bd9ac0764c6a47a402ced21ee5d3741fc1`.
The sidecar read that review: it verifies exact spelling/relocation, physical PRE,
six gate receipts, linked 84-byte equality, fresh readiness and all 246 historical
DATA proof artifacts. Parent confirms review ingestion and public verification
both exited zero; the sidecar read `verify.json`: `applied:true`, one row.
Finalizer preview and apply launched at 20:59 before the work cutoff. Parent
observed apply handle `54978` terminate exit zero at **21:00:45**, within the
unchanged cleanup allowance. The sidecar read `finalize-apply.json`:
`disposition:published`, `current_acceptance:true`, `production_complete:false`,
**189 rows, complete:false**. Preview retains 29 new proof artifacts; plan pin
`7f158ec4dddcd9398c2fce25f8b34da24225527cccbf26754d16814912239eff`
binds successor generation
`7420fd6a09eb4382f04f3154a886aed6edbd9842dfb1535bfa904d4c0be7c22f`,
SHA-256 `fbcc3fc81f45c96655ebcaddbaa0eb90b2411384d44ab63b7c4810d60ad33b88`.
This accepts the selected FUNCTION/report checkpoint, not full-target closure.
Local commit `e6f3a076` completed at **21:02:48 UTC**, closing this action after
**25 minutes 32 seconds**. No further source application or repair is authorized.
Uncertain state still stops for parent inspection, never replay.

Parent's final live inventory across 23 targets is **1,360 raw names: 529
functions and 831 data**; battle03 has **189: 85 functions and 104 data**.
Battle15 still retains 250 report rows versus 249 live names, with stale
`func_800A3638`; its 249-row summary is accurate. This reproduces existing S4.4
debt, not drift introduced here, and remains unrepaired. Parent reports no other
inventory discrepancies; selected-target closure is still incomplete.

The new user requirement records wall-clock start for every action. This action
keeps its observed 20:37:16 start without reset. Parent's
`out/reviews/naming-function-20260913/timing.json` records the commit, stop time
and elapsed duration above; its `assessment.md` records the completed post-commit
assessment. Both were read by this sidecar. Further transaction work stopped;
the assessment identifies reusable naming snapshot production and explicit
parent-review packaging to replace per-action authored Python scripts. Gauss's proposed owner
reuse is `inputs.frozen` / `transaction_paths`, `common.inventory.capture_file`
and `history.read_snapshot_states`; `common.recovery` for macro/type transactions
is not a drop-in replacement. Assess packaging in naming `review.py` with explicit
parent-supplied acceptance, preservation and run IDs, leaving downstream
`review.ingest` unchanged. Never derive approval by parsing ACCEPT prose;
semantic review and approval remain external active-session responsibilities.
These were post-commit assessment leads; the distinct tooling action below owns
their subsequent implementation and budget.
It must follow [modular standards](../agents/coding-standards.md):
concern-owned noun modules, verb-led operations, thin CLI adapters and existing
shared mechanisms. The assessment alone grants no implementation, dependency,
generic scheduler,
Codex CLI/model invocation or renewed source budget.

For current naming scope, the reviewed DATA support and `d2bbb57c` supersede
historical FUNCTION-only postapply descriptions, including S3.4's old evidence.
The [bounded FUNCTION/DATA lifecycle](../agents/tool-usage.md#frozen-naming-postapply-lifecycle)
retains distinct provenance and concern-specific restrictions; types/macros are
not admitted through it. Individually approved selected transactions may proceed
through their own proposal, native gates, independent review and public verify
without claiming full-target closure. Unfiltered full-target `complete:true`
and separate identity approval still gate production campaign advancement;
selected acceptance or report activation cannot satisfy S3.2/S4 or whole-game
completion. Historical proofs and full-report blockers are not rewritten.

The initial documentation slot released at 20:58:22 with 46 plan tests, parser,
references and whitespace passing. Parent authorized this final outcome-only
plan revision and checks within the unchanged **21:05 cleanup** cutoff, without
renewed source work. The sidecar performs no source/runtime, build, index or Git
writes and launches no children. Preserve `.pi/settings.json`
without inspection, both unrelated dirty battle sources and `tmp/`. The immutable
191-row predecessor proof and 190-row successor provenance survive; the latter
is the stage's input, not a claimed final row count. Frozen-five membership and
blockers, linked type controls, both macro obligations and their consumed limits,
the 50-entry clean-C queue (four accepted, 46 outstanding), stable IDs/states,
S3.2/S4/consolidation and all other unfinished obligations remain unchanged.

<a id="naming-preparation-tooling--2026-09-13-tooling-accepted-commit-pending"></a>
### Naming preparation tooling — 2026-09-13 (checkpoint committed)

The separate parent action starts **21:03:46 UTC** from `e6f3a076`, with unchanged
**21:35 work / 21:40 cleanup** cutoffs: one implementation and up to two repair
rounds. First implementation and both repairs are consumed; independent tooling
review accepts. Commit `2d1d125b` completed at **21:20:39 UTC**, elapsed **16m53s**,
as recorded in `out/reviews/naming-preparation-20260913/timing.json` and
`assessment.md`, read by this sidecar. No allowance or cutoff is renewed.
Parent owns `naming/snapshot.py`, `naming/review.py` and thin `naming/cli.py` adapters;
the [harness](../agents/harness.md#naming-preparation) and
[CLI lifecycle](../agents/tool-usage.md#frozen-naming-postapply-lifecycle) own details.

`snapshot` captures physical PRE for exact prepared FUNCTION/DATA scope, retaining
bytes/modes/absence and actual-index/report/reviewed pins with manifest-last
publication. `prepare-review` binds an explicit closed parent decision to pinned
report/gates/review/snapshot evidence; it derives mechanical digests only and feeds
existing ingestion/public verification. Both use cooperative source/report locks;
neither mutates source, infers approval or grants unattended restoration.

Independent review found source/deadline drift could escape during final
publication. Repair round 1 adds snapshot copy checks before manifest publication
and complete live-input/index/report/deadline rechecks afterward; review packaging
revalidates bundle/source/execution pins, attestation, parent-decision/report/gates
pins and deadline after publication. Failures retain artifacts, including manifests,
without validity or restoration authority. Copies cover selected PRE paths only;
existing state checks conservatively hash the native build closure.
Retained `out/reviews/naming-preparation-20260913/repair-source.log` and
`repair-deadline.log` show the snapshot reviewer's source/deadline reproductions
now reject as intended. Reviewer then confirmed valid JSON could substitute an
alternative reviewer ID or snapshot manifest after publication. Final repair 2
requires both published byte SHA-256 values to match their originally encoded
payloads, followed by lease/deadline checks; all prior live-input checks remain.
Both repair rounds are consumed.

Parent's consolidated final repair-2 validation reports **749 passed, two skipped in
47.24s** (`out/reviews/naming-preparation-20260913/tests-final.log`); this reruns
the focused checks, not an additional cumulative count. Retained
`out/reviews/naming-preparation-20260913/probe.py` and `probe.log`, inspected by this
sidecar, exercise actual snapshot/prepare-review CLI handlers and existing
ingestion/verification APIs for FUNCTION and DATA with synthetic native/index
fixtures. Positive controls, exact PRE/absence, stale pins, applied-PRE refusal and
seven decision rejections pass per log; this is no new production rename or native
acceptance. Final logs inspected by this sidecar confirm both lifecycle probes pass
on the repository NTFS filesystem (`probe-final.log`), still with synthetic
native/index fixtures, and whole-tools Ruff passes (`ruff-final.log`). Parent
confirms production report history resolves. No whole-suite or production native
rerun is claimed.
Independent review at **21:17:16 UTC** records **ACCEPT** in
`out/reviews/naming-preparation-20260913/independent-review-repair2.json`
(SHA-256 `1d362d1e82c7b9fdbd59e64a238b75f2283960b1b5f1302d6c8191d0f1b2c1a7`).
This sidecar read the artifact and verified all five exact runtime/owner-guide
pins; accepted guide bytes remain unchanged. All **12/12** adversarial publication
controls reject and retain outputs; no open review blockers remain. The review's
then-pending parent reruns are evidenced by the final logs above. Acceptance is
limited to tooling at those pins, not this plan, source/native/name acceptance,
restoration authority, report finalization or retries. The probes use synthetic
native/build-closure/index fixtures; cooperative checks do not exclude manual
writers or authenticate remote actors. Later runtime/guide changes require renewed
review or explicit reviewer rebinding. The commit/timing outcome is recorded above.

The parent grants this sidecar only the plan and both linked guides, targeting
writer release by **21:18 UTC** within the original cutoffs. Existing plan tests,
parsing, scoped references and whitespace checks accompany handoff. The parent
subsequently recorded commit/stop/elapsed results in `timing.json`. Preserve all
prior history, budgets, stable IDs and
states, frozen-five membership/blockers, linked type controls, both macro queues,
the 50-entry clean-C queue (four accepted, 46 outstanding) and unrelated dirty work.
Battle15's stale `func_800A3638` remains S4.4 debt; full-target `complete:true`,
S3.2/S4/consolidation and whole-game obligations remain unfinished.

**Production-resume preflight — committed.** The
new action retains its **21:21:48 UTC** start from `2d1d125b`, **21:50 work /
21:55 cleanup** cutoffs and one implementation/up to two repairs; the first
implementation is consumed. Before repair, parent confirmed fresh battle03
readiness with **189 rows: 85 functions, 104 data**. Actual
`bin/agent-context cleanup audit-target emi/battle/battle/03` then failed because
campaign history imported `common.inputs` and domain claims/layout requiring
`yaml` under the wrapper's stdlib-only `-S` bootstrap.

Parent's import-boundary repair moves domain imports into `common.inputs.input_state`;
`naming.proposal` keeps its context type under `TYPE_CHECKING` and imports live
context helpers inside `validate_proposal`. Saved-proposal history remains
stdlib-only; live validation guards remain required. This sidecar inspected the
import-only delta. Parent reports the unchanged `-S` wrapper command now exits
zero in **1.32s**; handle `47414` completed with **749 passed, two skipped in
46.99s**, retained in `/tmp/naming-bootstrap-tests.log` and read by this sidecar.
Parent reports whole-tools Ruff passes. Pure `-S` verification of historical
prepared `func_801DE804` through `require_provenance`/`validate_authored_digests`,
then active-history resolution, passes without importing `yaml`, `naming.context`
or `domain.manifests`. Independent **ACCEPT** at **21:28:24 UTC** is retained in
`out/reviews/naming-bootstrap-20260913/review.json`, SHA-256
`1263010a3f8e766ab1446cba86fe02bec6f20b5d31c94d3c3dd454d9d2481bdf`.
This sidecar read the record and verified its hash and both live runtime pins.
No blockers remain: the reviewer passed the exact live wrapper, **37** retained
disposable `-S` helper/history checks (expanding the initial 21) covering
FUNCTION/DATA proposals, exhaustion and corruption, and **44 existing tests**
(75 deselected); non-import ASTs equal `2d1d125b`. Acceptance covers only the two
pinned runtime files, not documentation or source/name/native acceptance.
Parent's read-only old-HEAD/current `input_state` comparison yields identical
**1,804 path states**, taking 1.555/1.510s; this is parity evidence, not a speed
benchmark. Commit `8319fbcd` closed this action at **21:31:36 UTC**, elapsed
**9m48s**, recorded in `out/reviews/naming-bootstrap-20260913/timing.json` and
`assessment.md`; this sidecar read the timing record. That docs slot covered only
this plan and a brief harness note; its checks grant no source acceptance.

At that bootstrap checkpoint no semantic name was chosen or rename applied. Battle03
`func_801DE560`, `func_801DE60C` and `func_801DE858` remain partial and must not
route as exact; `func_801DE9A8` is only an exact 112-byte scout with two indexed
callers and no accepted semantic name. All source goals, queues, historical proofs,
consumed budgets and phase states remain unchanged; no native/index/Git writes or
children belong to this sidecar. Existing docs checks accompany handoff.

### Battle03 name-copy identity — 2026-09-13 (rolled back; tooling fixed)

Parent resumes one candidate and at most one accepted identity application for
`emi/battle/battle/03@0x801DE9A8` from `8319fbcd`. The initial clock observation
was lost to output overflow; **21:35:49 UTC** is only the first recovered clock,
so later elapsed time from it is a lower bound. Work/cleanup cutoffs remain
**234800 / 235400 monotonic**; no earlier budget is reset or replaced.

Parent reports semantic acceptance of `copyLocalBattlerNameToTextSlot0` through
candidate `5afcc889` and final semantic verdict `5ccf3998`. Original resident
callee bytes prove bounded NUL-terminated copying (maximum five), corroborated by
START's Ryu/Nina/etc. name initialization, local copying and the resident text-slot
consumer. This contradicts the old script/event `@behavior`. A parent draft
misread compressed output and was corrected using independent original-byte
evidence; no native corruption was established. The original aligned-JAL scan
finds **five sites versus two indexed**; the reviewer covers the extra three raw
callers through candidate `optional_work`, without inventing reviewed boundaries.
Indexed incoming-call coverage is not complete. The collector correctly refuses
unreviewed caller `emi/battle/battle/03@0x801D54F8`; parent's minimal
`naming/instructions.py` patch adds its selector to the existing refusal only.
Indexed collection succeeds; the reviewed-byte gate is unchanged.

One consumed spelling-only application followed a first-line patch-verification
failure that made zero edits: parent verified every physical PRE before applying,
without taking a replacement PRE. Prepared report SHA-256 is
`71710dca41c4efa7b0d72870004458b271c10449a7d30c9f46c50cc6339c7273`.
Public PRE is `out/reviews/evidence/postapply-5ae74bc0ee71b2468ccf2e8e7a46b025/snapshot.json`;
native receipts are `out/reviews/evidence/postapply-4f63e2362de972c2f5a1d364f522f400/gates.json`.
This sidecar read all six passed gate results: symbols normalize/check, Splat,
build, asm-diff **28/28** and byte-match **112/112** for the selected helper only.
Parent confirmed POST index recovery and fresh `ready:true`, plus **164 existing
tests passed in 27.36s**; the earlier 64-test subset overlaps and is not added to
that count. No tests were added.

Gödel `01a09cc5-afb0-78d3-9ae0-a426c37b0aa4` issued the sole POST verdict
**BLOCKED**, `accepted:false`, `repairable:false`, in
`out/reviews/evidence/naming-template-20260913/post-review.json`, SHA-256
`16b4d17c0de5a2374da6f0ece13c1e365f74b4ba04b7a07ff1b73ab4aec4b431`.
This sidecar read and hash-verified it. Preservation and selected native checks
pass; no source defect or failed native comparison was observed. Two blockers:

- The parent's handoff transcribed the semantic-review digest suffix incorrectly:
  supplied `5ccf3998ef11d9d781f213ce027ecc18da0cf62b88e6a04a3db9da029a546bbc`,
  actual `5ccf399855f7221b2071d314f1dec76e6ec70ac6c68a02722b602bba1a7028cc`.
  This sidecar verified the actual artifact and correct candidate digest; original
  artifacts remain unchanged. Recording the discrepancy is not silent pin rebinding
  or acceptance of the mismatched handoff.
- At rejection, `naming/application.py` generated FUNCTION asm/byte checks only for
  the selected helper. Changed C callers `emi/battle/battle/03@0x801D54F8`, `@0x801D5658` and
  `@0x801D590C` also require live comparisons; target compilation does not prove
  their byte equality. The six passing gates therefore do not close affected scope.

Parent completed exact physical-PRE rollback; this sidecar verified all nine
physical paths against snapshot bytes/modes/absence after its earlier POST-only
observation. Names are restored. Parent confirms rollback build/index/readiness
complete and battle03 **189 = 85 functions + 104 data**, `complete:false`, versus
global **1360 = 529 functions + 831 data**, with prepared report SHA unchanged.
No full-report completion, finalization or
historical-row deletion occurred.
This sidecar read `rollback-readiness.json` and all six `rollback-*-{asm,bytes}.json`
records under `out/reviews/naming-template-20260913`: the three PRE callers pass
asm/byte checks at **352, 340, 340 bytes**, respectively. These prove restored PRE,
not the rejected POST. No successor was published or production rename accepted.
The one application remains consumed: no new PRE, source retry or acceptance.

Parent implemented the bounded coverage-root fix within the original **234800**
work / **235400** cleanup cutoffs. `application.collect_function_checks` derives
affected C callers from frozen `source_locations`, live pinned exact metadata and
resolver identity; producer and validator share `plan(root, target, row)`, requiring
**12 gates** for this scope once caller metadata is valid, versus the original six.
Evidence checks bind each caller selector/name/source, positive aligned original
size and exact current bytes; no raw boundary is promoted. This sidecar inspected the implementation
delta. Final existing coverage passes **164 tests in 27.47s**, retained in
`/tmp/naming-template-final-tests.log`; this sidecar read the result. This rerun
supersedes the earlier run for current tooling, not a cumulative test count.
Lovelace `01a09ccf-bbc2-7872-aa29-362663535a8c` independently **ACCEPT_TOOLING**
in `out/reviews/naming-template-20260913/coverage-review`, SHA-256
`d9e07982b305d5ba1bfc102f9e79c04af95cba82837f93365982bca427fae448`.
This sidecar read/hash-verified the artifact and all three live implementation pins.
Independent validation passed **146 existing tests, two skipped**, Ruff/format,
fixture coverage and 96 negative payload cases; overlap with the parent's 164 is
unknown, so counts are not summed. Actual PRE asm-full/byte payloads validate
under canonical-metadata fixtures; normal-detail asm correctly rejects missing
fields. The snapshot refusal probe mocks surrounding setup, not the real caller
metadata or collector, and grants no live publication. This accepts coverage and
early eligibility tooling only, not source/POST acceptance, metadata repair or
retry authority. Original review/work cutoffs remain unchanged. Existing docs
checks accompany handoff; parent owns final stop/commit timing.

Strict preflight exposes a separate production blocker in all three restored
callers: their `@status exact` / `@match 100` accompanies
`@residual none; live audit is instruction- and byte-exact.` Canonical
`parse_progress_tags` requires literal `none`, so `lift_lifecycle` is `invalid`
despite the **352/340/340-byte** native matches. Parent preserves source PRE and
does not soften parsing or edit these tags. `collect_function_checks` now reports
the invalid caller path; public snapshot preparation runs it before physical-PRE
publication, rejecting before identity mutation. This sidecar inspected both
changes. Parent's retained `coverage-probe.json` in the wave directory, read by
this sidecar, reports the final **99 passing assertions**: original 94, four
selected-size checks rejecting 116 against frozen 112 bytes, and one DATA-plan
comparison identical to HEAD. The 12-gate plan exists only
under an in-memory canonical-metadata fixture; real PRE rejects malformed caller
progress before publication. Production C hashes stay unchanged. These disposable
checks establish neither production readiness nor a new source attempt.
Separate evidence-backed metadata repair must precede any future rename involving
these callers. The present case remains blocked by invalid metadata, its consumed
application and the recorded parent pin-transcription failure; report counts stay
**189/1360**, with no completion decrease.

At that checkpoint caller metadata and stale `@behavior` repair remained pending;
the next separately scoped action below authorizes these without identity retry.
Accepted harness coverage remains required; raw-JAL-versus-indexed coverage
diagnostics remain explicit work. Preserve frozen queues, proofs, budgets and states: the
three neighboring partials, **46 outstanding clean-C entries**, linked type and
both macro obligations, battle15's stale history, full-target `complete:true`,
S3.2/S4/consolidation and whole-game goals are not closed or refreshed by this row.

### Battle03 metadata correction — 2026-09-13 (accepted)

Prior action committed locally as `4d5fc2dd`, stopping **22:18:14 UTC**:
**42m25s is only a lower bound** from the first recovered clock, not a recovered
initial start. Timing/assessment live in `out/reviews/naming-template-20260913/`.
The new metadata-only action starts **22:19:42 UTC**, with unchanged work/cleanup
cutoffs **235100 / 235700 monotonic**. Its four-source before-images protect this
metadata action; they are not a new PRE for the spent naming application.

Parent applied only comment corrections, confirmed by this sidecar's diff read:
`emi/battle/battle/03@0x801D590C` (`advanceLocalFlag20Countdown`), `@0x801D5658`
(`advanceLocalFlag40Countdown`) and `@0x801D54F8` now have literal `@residual none`;
`@0x801DE9A8` corrects `@behavior` to copying the selected local battler's name into
text substitution slot 0, at most five source bytes followed by NUL, from the
retained original proof. Names, bodies, ABI, addresses and compiler settings stay
unchanged; no symbol rename or source retry is authorized.

The initial unsorted `--rows` request rejected before edits. Parent verified all
four before-image hashes, then passed the same three sorted repair rows to the
existing CLI. All six selected live asm/byte gates passed. CLI exit 1 reports
remaining target-wide findings—**two blocked, 173 repairable**—not selected repair
failure or permission to expand scope. Parent reports all **eight fresh POST
asm/byte gates** pass at **352/340/340/112 bytes** for `801D54F8`, `801D5658`,
`801D590C`, `801DE9A8`, respectively, plus symbol checking. All four strict progress
records parse valid; complete sources equal only the approved progress/behavior
comment replacements. Actual production `collect_function_checks` returns four
functions and `plan` requires 12 gates without mocks. Its selected name/destination
remain the historical proposal, not an applied identity or new PRE.

Parent ran one batch index recovery after type-state comment-hash staleness;
`out/reviews/naming-metadata-20260913/final-readiness.json`, read by this sidecar,
reports `ready:true` and `index_ready:true`. Existing preflight tests passed
**30 in 0.08s**, read from `/tmp/naming-metadata-tests.log`.

Plato `01a09cdf-1ce0-7451-b4fa-ebb91a72a75b` issued **PASS** at **22:27:31 UTC**,
within the ten-minute review and original outer bounds. Artifact
`out/reviews/naming-metadata-20260913/review.json` has SHA-256
`7a605a879175b5ee9ff51c407df55a784b36147e59a9e255b9bb900da4975278`;
this sidecar read it and verified its hash and all four live source pins. The
review confirms exact comment-only deltas, strict progress, eight native artifacts,
frozen ownership/index/report pins, original semantics and fresh readiness, with
no findings. Parent accepts **only these four metadata corrections**. Reviewer
inspected parent native evidence without rerunning compilation; unrelated dirty
exclusions remain parent-attested. No review/attempt budget resets.

This plan-only handoff runs existing plan/reference/whitespace checks. The failed
identity application's consumed allowance and rejected POST remain historical;
no new naming PRE, retry, successor, finalization or historical-row deletion follows.
Metadata acceptance closes no broader phase or whole-game goal. Preserve battle03
**189 = 85 functions + 104 data**, global **1360 = 529 functions + 831 data**,
all other queues/budgets, the 46 clean-C obligations and all unfinished phases.

### Area027 naming scout — 2026-09-13 (deferred; semantic budget exhausted)

Mission `out/reviews/naming-area027-20260913/mission.md` starts **22:31:56 UTC**
after metadata commit `35763b5e`. Its sole selector is
`emi/world00/area027/13@0x801F3650`, a reviewed **64-byte** helper absent from prior
frozen/consumed queues. Bounds remain one semantic candidate plus one repair,
one conditional identity application with zero forward fixes/retries, fifteen
minutes semantic review, ten minutes POST review, and **236800 / 237400 monotonic**
work/cleanup cutoffs. This separate candidate replaces no prior queue member.

Faraday `01a09cea-21c1-7533-b8f2-0cc0b80915a2` initially supported
`setScenarioProgress5Or6FromSelection` at historical candidate `3b717733…`.
Schema-authoring defects consumed review time and the single repair before any
mutation. Final candidate SHA-256 is
`b91badbeb10d377113e136a2a2eed330c8ea357a22c5e1210edca4eaa2c794d1`;
verification finished **22:51:37 UTC**, after the original **22:51:32** ceiling.
The definitive decision is **blocked**, not late acceptance, in
`out/reviews/evidence/naming-area027-20260913/semantic-decision.json`, SHA-256
`60a03b2759c3dce83bb52b30f6e177448c923212854f5004b23e726ddb932114`.
This sidecar read the mission/decision and verified both final hashes. Earlier
semantic evidence does not bind the repaired pin; no retry, extension or rebinding.

Zero source/report mutations or identity applications occurred. The source remains
`func_801F3650`; the 64-byte exact baseline is not naming acceptance. This sidecar
verified source preservation and the unchanged 19-row report SHA-256
`e650abab8da0faf832c5f90fe61dbfc4d36a62b6023762794a5ccab498fa29bb`.
Parent implemented `audit.prepare_transaction(check_only=...)` and CLI `--check`:
the same full-validation/lock path retains deadline/report-byte guards but skips
report publication. Existing preparation already fully validated before publishing;
no surviving-proposal invalidation bug is claimed. `facts.py` now reports allowed
corroborator classes and grouping advice. This sidecar inspected the code delta.
Parent's production check exits zero with `checked:true`, `prepared:false`, and
unchanged report SHA above; the semantic decision remains blocked. Parent reports
**642 existing naming tests passed, two skipped**, **60 focused tests passed** and
**42 documentation references valid**; overlap is not added into a combined count.
Pascal's initial **FAIL R1** remains preserved: the final report read could cross
the deadline before returning `checked:true`. The single bounded repair computes
the result/hash first, then checks the deadline immediately before return under
the lock. Final **PASS** completed **23:05:20 UTC**, before the unchanged
**23:07:40** review ceiling, in
`out/reviews/naming-preflight-20260913/final-review.json`, SHA-256
`d9db1a2ebb041b4c7babd2a5fca7da52ce330cf422ffc4b57de6387c3fd79e38`.
This sidecar read the verdict and verified its hash, all five code/guide pins and
the preserved initial review. Independent checks passed **173 existing tests and
16 disposable probes**; parent reports **60 post-repair tests and 16 rerun probes**
passed, without summing overlapping runs. Acceptance is tooling-only: point-in-time
mechanical checks reserve no state and do not accept candidate semantics.

Parent reports whole-tools Ruff and symbols pass. Full `just check` timed out
after **300s**, exit **124** at **23:04:15 UTC**, with 35 progress dots and no
assertion failure shown; it is not a full pass. Source aggregate audit handle
`35843` entered `decomp.build_report`/native compilation and was cancelled by
actual Ctrl-C, confirmed exit **130**. Source audit is unverified, with no retry,
claimed native outcome or unsupported compiler-failure diagnosis. Authored
source/report pins remain intact. Final plan-only cleanup is bounded by
**23:08:20 UTC**; original work cutoff **236800** is unchanged. These checks neither
retry semantic review nor grant source/name acceptance. Preserve battle03's rejected
prepared row, its spent application, battle15's stale history, all prior queues,
budgets and **1360 = 529 functions + 831 data** global counts. No successor,
full-report completion or broader phase closure follows. This plan-only slot runs
existing checks; no staging or commit belongs to the sidecar.

### Supervisor finalization performance — 2026-09-13 (tooling accepted)

Action starts **23:08:44 UTC** after `91a2263a`; work/cleanup remain
**23:33:44 / 23:38:44**, with this plan-only slot ending **23:29:00**.
One focused fix and one bounded repair are consumed: `process.py` flushes available
Python streams and calls `os._exit` only after `_main` finishes cleanup/FD closure.
Initial FAIL R1 (absent streams causing false failure) is preserved; the None guard
fixes it while genuine flush errors still fail. No ownership/deadline/ACK guard,
Git query, validator or cache policy is weakened. Parent accepts tooling only.
Lagrange's [final review](../../out/reviews/validation-profile-20260913/final-review.json)
passed **23:25:33**, before the unchanged **23:26** review ceiling; SHA-256
`0a860fb485f3ff42c0c277dcc5ee9de5a4425ad1364b57b2a351eb145a45a5d6`.
This sidecar read/hash-verified it and the live process/guide pins.
The [measurement summary](../../out/reviews/validation-profile-20260913/summary.json)
owns methods/limits: 216 commands consume 6.264s of 8.608s; 163 Git calls take
4.690s (66 index-path, 44 HEAD, 44 entry, nine root queries). Final exact-code
`-m` pair passes at **8.246699→7.818148s (~5.20%, one pair)**; pre-repair `-m`
means improve **2.80%**, high variance/two pairs; controlled `-c` improves **6.13%**.
The unmatched **8.608→8.800s** profile remains unimproved. Cold startup/repeated Git
queries still dominate; no whole-suite, throughput guarantee or bottleneck closure.
Independent 20 existing checks, 21 semantic probes and closed-stream/flush-error
cases pass; parent 20 post-repair checks, earlier 172 naming and four owner
integrations pass, without summing overlap. Existing plan checks accompany handoff.
No source/report/index acceptance, naming retry or queue/phase change: preserve
**1360 = 529 functions + 831 data**, prior blocked/deferred budgets and all debts.

### Commu00 naming prerequisite — 2026-09-13 (prerequisite accepted; naming deferred)

[Mission](../../out/reviews/naming-commu-20260913/mission.md) starts **23:28:56 UTC**
after `d79db07a`, selecting only `emi/etc/commu00/00@0x801F0E1C` (**172 bytes**),
with no prior frozen-queue overlap. Effective work cutoff remains **240500 monotonic
(~00:11:27 UTC)**, stricter than **2026-09-14 00:13:56** wall work; cleanup is
**00:23:56**. Retain the mission's one candidate/one semantic repair, one conditional
identity application, 15-minute semantic/10-minute POST reviews and no forward retry.
Parent's native asm/byte baseline is **172/172**; naming has **zero candidates,
reviews, applications or renames**, deferred at the prerequisite, not accepted.

Original collection failed because installed Rizin **1.0.0 cc06c1d** treats `tl` as
type listing; both old type-link commands failed and their definitions were absent.
Emitted `pdj` bytes do not make that operation successful. Original 30-row report
SHA-256 `e7f9409089e96e8fcd0698fc29fd3bbafc74ff5d36707e8d4a75cb46cd651ba2`
and failed-namespace manifest SHA-256
`1fc30225a405c66c51e6acfa619410ee5e16631d0626f186885bae5f3b14e8a5`
remain unchanged, checked by this sidecar; original evidence is not rebound.

Separate authorized prerequisite repair changes only commu00 `reviewed.rz` to
`td` structs plus `avga` arrays: Gift **20×4** at `0x801EEC48`, Explore **48×2** at
`0x801F2618`. Parent verifies original archive load **0x801EEC00**, size **0x3D50**,
offset **0x800**, byte-equal extracted/normalized payloads. Real Rizin probes match
original instructions for selected/Gift-consumer/Explore-consumer **172/516/684**
bytes and exact table extents **80/96**. Collection/execution now retain full failed
operation context/raw stderr; deliberate bad-command evidence stays failed without
a journal commit. This sidecar inspected those deltas and the harness note.
Existing **156 tests passed in 57.12s**; full naming run `90491` exited zero with
**642 passed, two skipped in 32.53s**, read from the mission's respective logs.
Parent reports scoped Ruff passes and harness references **19 valid, zero broken**;
overlapping test runs are not summed. Schrödinger
`01a09d24-f9cc-7240-9183-de8982ce8483` issued **PASS** at **23:48:23 UTC**, before
the unchanged **23:55:41** ceiling, with no findings requiring repair. Parent accepts
only the four pinned prerequisite files. This sidecar read the
[review](../../out/reviews/naming-commu-20260913/prerequisite-review.json), verified
SHA-256 `e9e5a9cca9a7b1b9f85f635f30085bec3a4b457b14070d664b658ba9bc9156f1`
and matched live files to the reviewer's starting pins. Independent checks cover
real type fields/array extents/original bytes, **13 existing tests**, seven injected
attribution/cleanup/deadline cases and full **>8 KiB** raw/stderr retention. Parent's
full-collector disposable failure probe passed after fixing missing fixture
`required_work`, not a product repair or production collection rerun.

Residual limits remain: `SemanticResult.output` is not retained, so lifecycle-only
summary diagnostics can be absent; pre-existing timeout paths may supply empty
raw/stderr. Full forensic JSON is not bounded by the lifecycle's 8 KiB summary cap.
Control-flow probes are not native process-cleanup proof; no full-suite or fresh
POST compiler acceptance is claimed. At that checkpoint naming had zero candidates/reviews/
applications; prerequisite acceptance grants no semantic approval.

Changed recipe left snapshot `fresh:false`; prerequisite acceptance performed no
index refresh or campaign rebinding. Subsequent continuation requires separately
safe snapshot/index refresh and explicit provenance
recovery preserving the old report/failures, never a silent rerun. GAME's **eight**
obsolete `tl` entries remain outside scope. Preserve all prior queues, consumed
budgets, **1360 = 529 functions + 831 data**, and broader unfinished goals. This
plan-only slot grants no commit, source acceptance or renewed naming mission.

**Same-queue continuation — POST deadline failed; exact PRE restored.** Prerequisite
commit `85a0a1f1` stopped **23:50:19 UTC** (21m23s); continuation starts **23:51:07**,
retaining original **240500** work / **00:23:56** cleanup bounds and the failed
collector. Parent preserved prior index/snapshot and original report CAS/failure
evidence before one safe refresh; no physical backup of the original initializer
report is claimed. Only the later prepared physical PRE exists. Stale commu00
analysis rebuilt, then index; readiness is true.
Fresh manual owner receipts in a distinct namespace cover selected/caller/initializer/
dispatch/consumer **172/796/484/56/76 bytes**; no production collector rerun occurred.
Original initializer report CAS stayed unchanged through successful public `--check`.
Socrates `01a09d33-817f-7c51-9712-0d92d738fc2f` passed the original candidate
`configureScratchTaskForVariant` at **2026-09-14 00:00:15 UTC**, before **00:02:30**;
parent pin-verified and separately accepted in the mission's `semantic-acceptance.json`.

One candidate, zero repairs and **one identity application** are consumed. Parent
verifies five logical roles equal physical PRE with only raw-name replacement and
unchanged modes. Public PRE `postapply-2b3696a44015a189c7101858727a0f80/snapshot.json`
hashes to `73fe85d810acb01dff4ba077a2f00a27ea65d85a1dac7afd30591c7b6808d2ba`;
six passed gates in `postapply-02c7bd4ad461664f1b8fe0bf48eb9fdb/gates.json` hash to
`9833c80ec3fe560facb012cfbae9da017d84ad6cf24b7d665e1427e324b9a339`
(both under `out/reviews/evidence/`). This sidecar verified hashes and gate results:
**43/43 instructions, 172/172 bytes**. Prepared report SHA-256 is
`d61cc2c3c4809553183fa0695bee9ab3c235e988e2ac4973110c77ffa206fa44`.
An unsupported-deadline-flag invocation rejected before gates; one syntax correction
used the same `bind_deadline(240500)` launcher, not a source retry. Parent confirms
one POST index recovery and `ready:true` before rejection. Boyle
`01a09d3c-1a3b-7080-84cf-47d34a2eb133` missed the original **00:08:45** POST ceiling:
at **00:09:03 UTC**, parent stat-verified no canonical review artifact while the
reviewer remained active, then interrupted and required an honest timing failure.
The subsequent [FAIL](../../out/reviews/evidence/naming-commu-continuation-20260913/post-review.json)
was reported **00:09:40.821818 UTC**, SHA-256
`eebc4e603b86264a7da1425fd2ad69fc7a0c255a5ad7ab39cebe734ca822ed27`.
This sidecar read the timing verdict and verified its hash. All technical checks,
including 1,805 final-state file pins and independent ELF extraction of the original
172 bytes, passed; these and semantic approval are not timely POST acceptance.
No source defect was found. Parent compressed the POST review budget after the
prerequisite work; insufficient allocation/handoff and missed publication caused
failure, not semantic uncertainty or compiler failure. Reviewer is closed; no late PASS.

Parent reversed the five identity roles and verified all six physical PRE states,
frozen report/recipe pins and raw Git index. This sidecar independently confirms
all six paths' bytes/modes/absence and unchanged prepared-report SHA above:
`func_801F0E1C.c` is restored and the new source is absent. Rollback index recovery
`31842` exited zero with readiness true at **00:09:53 UTC**; native build `29785`
exited zero at **00:10:18 UTC**. This sidecar read rollback
readiness (`ready:true`, `index_ready:true`) and confirms `build/cmake/build.ninja`
contains the old source and excludes the new source. Restoration is complete.
No `prepare-review`, public `verify` or finalization executed; the prepared report
stays immutable. Accounting is **one candidate, zero semantic repairs, one spent
application, zero accepted names**. No automatic retry, new naming PRE or pin
rebinding follows. Semantic proof and exercised rollback are progress, not debt
closure: global **1360** and all prior queues/budgets remain unchanged. Existing
plan/reference/whitespace checks accompany this rollback checkpoint; any local
recording commit is not feature/naming completion or broader phase acceptance.

### Naming deadline/admission tooling — 2026-09-14 (tooling accepted)

Separate tooling action starts **00:16:10 UTC** after rollback checkpoint `b859b849`;
work/cleanup remain **00:56:10 / 01:06:10**, with 15-minute independent review
reserved and one repair consumed. Dirac `01a09d4d-9ce2-7ed1-9498-e56caaa8889b`
completed final repair checks **00:34:56**, before the unchanged **00:39:29** ceiling.
Four Python owners add original `--work-deadline` binding to nine public naming
lifecycle nodes via `common.cli`/`common.deadlines`: single finite value, stricter
inherited cutoff, cooperative entry/late-return checks and context restoration.
Snapshot's optional positive `--reserve-seconds` requires a bound original cutoff,
checked at entry, before output, before publication and before return. Schemas
stay unchanged; a late rejected PRE may remain on disk without acceptance. Reserve
is admission evidence, not guaranteed completion, clock reset or retry authority.
Initial **FAIL** at **00:31:28** remains preserved: legacy outer broken-pipe success
bypassed the opted deadline (R1), and the overview omitted the nine nodes (R2).
One bounded repair catches EPIPE inside the bound context before the final deadline
check; legacy/unexpired EPIPE still returns zero. The overview is corrected.
Final [PASS](../../out/reviews/naming-admission-20260914/final-review.json), SHA-256
`ca0012bcb9e57137f2385c8b214e6d7620fcb7a8f9b0a74bef2365f8397e607b`, accepts only
seven exact code/guide pins. This sidecar read/hash-verified both reviews and all
live pins. Independent 25 probe groups, real OS EPIPE and four nested/legacy cases
pass. Parent's post-repair **663 tests passed, two skipped in 37.39s**, **16 probe
groups**, **four actual EPIPE cases** and **43 valid docs references** supersede its
earlier 40.20s run without summing overlap. No regression tests, dependencies or
production mutations were added. This plan/operator text and profile are outside
the tooling review; no semantic, source or phase acceptance follows.

Read-only public `prepare-transaction --check` on spent Commu's prepared report
(handle `3064`) exited zero, `checked:true`, `prepared:false`, preserving `d61cc2…`.
The [profile](../../out/reviews/naming-admission-20260914/profile.json) and sibling
`prepare.prof` retain **53.3787s** profiled wall: four `transaction_scope` calls
48.079s, four `cross_target_files` 47.074s, 98 `local_include_files` 46.630s;
211,331 resolves, 1,719,996 lstats (20.399s), 517,377 stats (11.750s). Cumulative
times overlap and include profiler overhead; they are not summed. Live
`readiness._cross_target_files` calls `local_include_files(root, files)` inside each
other-target iteration on an accumulating list. Gathering all seeds before one
include traversal is a concrete separate optimization lead, not implemented here
or a measured speedup/new naming attempt. No source/report/index mutation or pin
rebinding occurred. Keep **1360**
debt, full-goal scope and all Commu/battle03/Area027 spent/deferred limits unchanged.

## Current skill-only operator correction

The user's latest architecture decision supersedes all executable Pi/native-Codex
launch and scheduler proposals below. Historical runs, failure evidence and accepted
domain primitives remain history, not permission to revive retired transports.
Remove harness code that discovers, configures or launches Codex, including native
writer/reviewer commands and their transport-only helpers. Keep local deterministic
diagnosis, audit, evidence, transactions, native compiler gates and guarded recovery.
The uncommitted auto-review child-configuration experiment is superseded too.

`bof3-lift-loop` owns the instruction-only operator: select a finite evidence-bound
queue; issue scoped missions to the other skills through tools available in the
active session; inspect actual results; review independently; apply/verify through
domain owners; account every outcome; continue until the original budget ends.
Do not shell out to Codex, invoke a model SDK, create detached agents or substitute
synthetic identities for unavailable active-session delegation. Missing host
delegation blocks parallel execution/independent review, not safe local inspection.

Parallelize independent read-only evidence and review missions. Serialize all writes
in this shared checkout, including shared headers/maps, target refreshes and domain
transactions. Disjoint selectors alone do not prove disjoint effects. Future writer
parallelism requires genuinely isolated, validated workspaces and explicit ownership;
never clean this dirty checkout by discarding unrelated work to manufacture isolation.

Mission inputs bind selector/opportunity and full membership, owning skill/mode,
allowed paths, adopted baseline/evidence pins, predecessor results, original limits,
acceptance gates and rollback/stop conditions. Results bind actual session/tool
handles, changed content, checks, reviewer provenance and accepted/no-op/repair/
blocked/deferred disposition. Durable transaction receipts remain owner evidence;
Markdown and live session handles own orchestration, not a new state database.
Resume preserves original consumption and first verifies real handles/current state.
Never infer termination, acceptance or permission from stale records or a free lease.

Before source application, reserve realistic time for remaining native gates,
index recovery, independent POST review and finalization, plus margin; cleanup has
its own separate bound. Never compress review to fit an expiring work clock.
Autonomous naming passes the original `--work-deadline` to public lifecycle nodes
and a positive snapshot `--reserve-seconds` covering those remaining phases.
Recheck the remaining budget immediately before editing; insufficient time defers
before application. Reserve checks promise neither completion nor a fresh allowance.

Index refreshes and active-session reviews have standing authorization; no repetitive
user prompts. Preserve frozen proofs before refresh and never invalidate an active
mission's inputs. Local feature commits are authorized; no push. Source fidelity,
independent review, naming `complete:true`, type layout and macro human-value/use-site
gates, same-envelope obligations and all frozen-five blockers remain unchanged.
Historical S3.4/S3.5 acceptance does not prove this new operator's end-to-end behavior.
S3.6 owns removal/skill implementation; S3.2 still owes a real bounded skill sequence;
S4–S9 retain useful campaign, naming, type, macro and whole-game acceptance.

## Skill-only continuation checkpoint

Feature commit `53ba86e8` removes the model launchers and installs the instructional
operator. Two concurrent read-only skill scouts completed before their shared
original cutoff `1215665.660447284`, one assessment each, zero repairs or writes.
Both real handles were observed completed and closed. Before/after index SHA-256
was `6ab87c008e4cd085ef6cd893986a3c86cfa6e91f6ffd0190193ef8c83a0b8d04`;
plan SHA-256 was `7f4f424b91537070f1968eb7e9f88029e147df6c830ca26ab1d0a0400f510f21`.
These are bounded scouting results, not live domain or S3.2 acceptance:

- Types, `01a0896a-d1ce-7f73-86ac-e6860fc9a55c`: the frozen storage and aggregate
  leads at battle15 `1F800044` remain one linked blocked concern. Original accesses
  distinguish a four-byte pointer cell from changing pointees; the existing
  `BattleWork` declaration does not prove universal pointee identity or extent.
  Next inspect the two pointer-assignment branches and restoration in
  `emi/battle/battle/15@800A36F0`, resolving its extra source indirection against
  original cell loads/stores before proposing a layout.
- Macros, `01a0896a-d1ff-7a41-856e-dff9bf102a7c`: frozen exact group
  `8e1ad03b4ba92303` retains all ten members and their original 64-byte hash.
  `PANEL_ADVANCE_X` already implements the useful operation; fourteen lexical
  consumers include four limit-17 uses. Cross-target private proofs, all semantic
  guards and complete consumer acceptance remain missing. Next consider a scoped
  metadata correction plus native/independent checks for frozen game00 `801996FC`,
  not another extraction. Narrow index/source inspection was not live owner
  descriptor validation or permission to widen the transaction.
- Parent control: current manifest/source/map still bind battle15 `800A3638` to
  `passesBytePairGate`; no rename replay or fresh terminal acceptance. Unfiltered
  canonical naming validation with its retained evidence root exits 2 on the
  historical `func_800A3638` binding scope. Local data remains blocked, not a no-op;
  no report initialization, reconciliation or receipt rebinding occurred.

Broader pytest and whole-source validation were deliberately interrupted after
roughly eight and seven minutes; both process terminations were observed. Pytest
also reported an interruption-time fixture teardown KeyError. Neither run passed;
focused checks remain the feature evidence. No full native audit, recovery/resume,
full-target naming closure or production skill sequence is claimed.

The subsequent game00 `801996FC` metadata pass corrected its behavior description
and canonical `@residual none`, with the same description in Splat; no macro body,
type, symbol, boundary or instruction changed. Pre/post native comparisons were
16/16 instructions and 64 bytes exact; post byte-match, target symbols and Splat
checks passed. Independent active-session reviewer
`01a08972-8826-78c0-b97c-26466d09cff3` accepted metadata semantics only and was closed.
Parent retained the prior working index/snapshot and original/native proof in
`out/reviews/game00-metadata-5oI1Mf/` before rebuilding the index. Frozen pilot
archives and unrelated dirty identities remain unchanged. This advances one
cross-target wrapper prerequisite, not reviewed private/shared macro acceptance.

The baseline compiler failure is an execution-capability issue: the installed
static i386 GCC receives SIGSYS/159 even for `--version` in this host's command
sandbox, while reviewed native execution completes the comparisons. Skills now
route the known host's baseline gates through the reviewed parent-native tool call
before compiling, retaining the inner read-only native-gate sandbox and original
bounds. No Codex policy/rule weakening, automatic self-elevation, compiler swap or
installation occurred. Native approval is still required; the host restriction
itself is not removed. Skill validators, 41 existing context/cleanup checks and
19 documentation references pass.

Pointer-slot continuation (reviewed partial, not lift/type acceptance): the
single-selector battle15 `800A36F0` mission retained evidence and both trials in
`out/reviews/pointer-slot-mqohxl/`, under unchanged cutoff `1216748.861511238`.
The preserved before-source equals HEAD at mission start. Baseline was 44/177
(24.86%), 704 bytes against original 708. Trial one repaired byte-vs-pointer-cell
access and measured 51/177 (28.81%), 656 bytes; reviewer
`01a0897b-aee6-7620-b52c-108a5f9b013b` accepted only that narrow correction.
Trial two also restored the full enemy index for the call/installed pointer and
the volatile status reload used as the second call argument, preserving the cached
return value. It measures 27/177 (15.25%), 708 bytes; reviewer
`01a0897d-1a8b-7e50-9580-9b631d379089` explicitly recommends retaining the coherent
semantic corrections despite the lower positional score. Both handles completed
and closed before cutoff; final byte-match exits 1 (DIFFER), never accepted/exact.
Symbols pass; no shared type, pointee layout, macro proof or naming closure is
inferred. This bounded two-trial stop is not exhaustion. Remaining work includes
clean call bindings/address representations and register/code-generation
differences, with a separately bounded mission and independent review required.

Call-binding follow-up for battle15 `800A36F0` remains evidence-blocked: its
manifest declares no companion ownership. Current owner queries locate `80196718`
in game00 and `801DDAB4` in battle03 (other images also contain that address).
These static leads do not establish battle15 residency or ABI; no foreign binding
or prototype was introduced. The retained pointer/status corrections are unchanged.

Status pipeline continuation reuses a validated manifest catalog only in the
post-build, read-only resolution loop; comparisons and fallback builds still load
fresh ownership. Independent reviewer `01a08983-ec3d-7cf2-bcbc-07239455dc2a`
found no issues and was closed. Controlled battle03 resolution of all 217 functions
fell from 31.286s to 3.851s, with identical serialized outputs and catalog loads
reduced from 217 to one. Profiles are `/tmp/bof3-resolution-{before,after}.prof`.
The full uncached native check completed in 44.359s; all 217 function records equal
the retained earlier report: 185 exact, 32 partial, zero invalid, no coverage error.
Current report: `out/reviews/status-catalog-after.json`; full profile:
`/tmp/bof3-status-catalog-after.prof`. This is not a controlled whole-command timing
comparison against the older run. Existing checks: 68 status/source tests plus
12 owner/decomposition tests pass; one pre-existing fixed-mode test is deselected.
No new tests, persistent cache or claim/ownership policy changes were added.

Frozen-panel wrapper continuation corrected only the six shop00 source metadata
records at `801E31C4`, `801E3774`, `801E3BF8`, `801E3D4C`, `801E438C`, and
`801E4540`; macro bodies, types, maps and Splat remain unchanged. Before/after
native checks pass for every selector: 16 instructions, 64 bytes exact, matching
the frozen original hash. Independent read-only reviewers
`01a08988-7cb6-7c53-af29-1af6e2102393` and
`01a08988-7cf8-7d43-8012-ae8ef3e611f4` each accepted three metadata-only scopes;
both handles completed and closed within the original `1217661.25391831` cutoff.
Symbols pass; the snapshot remains fresh. Evidence and prior index are retained
in `out/reviews/shop-panels-c0OrwK/`; frozen archives and unrelated dirty identities
are unchanged.

After parent index refresh, live macro owner inspection retains exactly the same
ten members of `exact_group:8e1ad03b4ba92303`, now all indexed exact. Observation
fingerprint `v1:71651687c009d6580d72ef482c10420bc4dcb1d681c6c21af0ee7ba563a7c682`
is a new descriptor observation, not a replacement ranking or rewritten private
proof. Resolution remains blocked with zero safe applications: private/shared
proofs, semantic guards and complete consumer coverage still apply. The existing
macro already factors these wrappers; no extra extraction or shared acceptance
is claimed by correcting their metadata.

## Current Codex continuation

The user's shared-macro relaxation now permits two or more declared targets,
with one independently accepted exact private wrapper per target. New macro
manifests retain a catalog-derived participation limit; old histories retain two,
and revalidation cannot enlarge original capture scope. Type policy is unchanged.
Every proven wrapper must be an affected function with both POST native checks,
even if lexical coverage finds no definitions. Common PRE, complete consumers,
distinct executions, independent reviews and historical replay remain gates.
Independent reviewer `01a08994-74a6-78b1-85e2-df1cb8b0d12e` identified the wrapper
omission; `01a08996-9f5c-7721-9f54-677d87f564d3` confirmed its fix. Both are closed.
The disposable three-target integration and current-code replay passed; evidence
is `/tmp/bof3-three-macro-probe-20260910/RESULTS.md`. Native Ninja and synthetic
BOF3 gates/reviews prove protocol behavior, not source acceptance. Existing checks
pass: 60 macro transactions, 12 shared PRE/decomposition (one pre-existing mode
check deselected), 46 plans, plus the 122-check macro/history/review run. No frozen proof was expanded or rewritten, and
the panel group's live private/shared acceptance obligations remain unfinished.

The bounded battler predicate mission retained an independently reviewed partial
at `emi/battle/battle/03@0x801DB3A0`. It replaces the absolute address macro with
the existing target-local `Battle03LocalHalfRecord` array view. Original and
retained code each read ordinary RAM once; this does not license removing volatile
qualification elsewhere. Native comparison improved from 8/20 instructions,
80 bytes, to 11/18 instructions (61.11%), 72 bytes; original size is 68. Final
instruction and byte gates remain DIFFER, symbols pass, and metadata stays partial.
Reviewer `01a089a5-937f-7780-b4a3-9bf2afa7a794` accepted coherent partial retention,
not exactness or ladder exhaustion, and is closed. All four source trials are
consumed; do not restart that mission or extend its `1219657.660698305` cutoff.
Evidence, original pins and final native summary are retained under
`out/reviews/battler-threshold-bvcl3lvf/`. Untried signed-comparison/early-return
and single-result forms remain future proposals, not authorized extra trials.

Macro readiness scout `01a089a0-dedc-7332-9273-f2c6766c197e` completed and closed:
all ten frozen panel wrappers already use `PANEL_ADVANCE_X(..., 320)`; four other
consumers use limit 17. No useful private extraction was found. Do not unshare and
re-extract merely to obtain transaction receipts. The next tooling obligation is
a separately pinned, independently reviewed **existing-abstraction disposition**,
with unchanged-state verification, semantic/use-site evidence and native gates.
It must distinguish reviewed existing work from new safe applications, reject
drift, and preserve all consumers and original participation/history constraints.
Current empty/identical edits reject, revalidation needs a prior reviewed envelope,
and accounting has no existing-work disposition. Thus the frozen macro remains
blocked, not accepted no-op; three-target capability alone does not close it.

The existing-abstraction tooling gap now has a separate prepare/check/review/verify
lifecycle and explicit accounting; see the [macro guide](../agents/macros.md#existing-abstractions).
Unchanged audits freeze all candidate members, selected definitions and every
direct/transitive consumer function. Exact native evidence, unchanged captured
build/source/index/tooling/workspace, externally pinned proofs and independent
review remain required. Existing dispositions count separately from applications
and cannot enter legacy private/shared promotion or revalidation. The frozen
adapter accepts a distinct macro-only `existing` route without changing original
pilot/freshness pins; this is not live closure or permission to rewrite history.

Current-code parent validation exercised four candidate members across four targets
plus a fifth-target consumer, 20 owner-derived gates and 42 expected adversarial
rejections. The real preparation/CLI/context/receipt/review/accounting code and
CMake/Ninja ran; BOF3 byte claims and reviewer identities were synthetic. Evidence:
`/tmp/bof3-existing-probe-20260910/parent-final.log`. The parent `timeout 180s` check
finished with exit zero. The preceding delegated replay exceeded its original
420-second bound; that worker was closed and its late result was not accepted as
a bounded mission. It was not resumed or given replacement budget.
Independent code reviews identified and resolved unchecked ranking inputs,
per-gate opportunity proof capture, occupied output preflight, function-level
consumer coverage and adopted-entry metadata preservation. Final reviewer
`01a089b8-a6dc-7380-8ec3-599a016d663a` found no remaining critical issue in the
last two fixes; all review handles are closed. Existing checks pass (134 combined,
74 final accounting/structure, 70 macro/structure and 46 plans; one known fixed-mode
check deselected where applicable), as do Ruff and documentation references.
No production macro disposition exists yet. Next: prepare the selected targets'
native build before freezing fresh audit inputs, prove every actual consumer,
obtain independent live semantic review, then verify the separate disposition.

Live panel audit prerequisites exposed four additional limit-17 consumers with
invalid progress metadata. Their bodies already match all 16 instructions/64 bytes.
Canonical residual tags and accurate add-32/clamp-17 behavior now replace the
invalid/incorrect comments; only game00's corresponding Splat behavior annotation
also changes. Original boundaries, bindings and all C bodies are unchanged. Native
before/after comparisons, byte matches and all three symbol checks pass. Reviewer
`01a089be-920e-7ef3-a2eb-77d535781485` independently passed this metadata scope and
the existing macro's semantics across all 14 consumers; final macro acceptance
still requires the separate native audit and review, not this prerequisite review.
The refreshed index has the unchanged ten-member candidate fingerprint and all
14 exact consumers; the game00 snapshot remains fresh. The three target builds
are warmed before freezing. Stage inputs and prior working index are retained in
`out/reviews/panel-existing-du5098he/`; the original audit cutoff is
`1222016.415020909`, one preparation and one native inspection, with two reviews
total. The metadata dependency has its own consumed one-pass/one-review bound.
The native inspection subsequently completed with tool session 13686 exit zero,
34 passing gates, `checked:true`, `accepted:false` and zero safe applications.
Inspection pin: `v1:0db6cf6cc3f8fe384554f1d6f9c1861c577391b92021d491c988c9d24b60d607`.
Final reviewer `01a089c9-c554-7f23-9a28-d9af66332163` reported PENDING: parent external
pin confirmation did not arrive within its original 220-second bound. That actual
handle is closed; both reviews and the single preparation/inspection are consumed.
No acceptance envelope or final owner acceptance exists. Preserve `terminal.md`
in the mission folder; subsequent tooling/plan edits make this historical evidence,
not fresh closure. Do not extend this mission or rewrite original frozen-five pins.

### Multi-function consolidation

The user's new scope adds cohesive multi-function C files, with leading metadata
for every implementation, and a ranked **combiner** cleanup node. The canonical
[combiner contract](../agents/combiner.md) owns methods, human-value filtering and
safety. This does not add entries to or replace the frozen five, restart their
budgets, close existing macro acceptance, or require a manufactured extraction.
Queue C1 alongside unfinished S3.2; production consolidation waits for C1's owner,
native and transaction gates. S4.1 only invokes combiner where applicable; S4.2
must include its reviewed result or explicit blocked/deferred/no-op accounting.

Current inspection finds one-address source scans, source-keyed status caching,
whole-file progress parsing, unique-source macro association, a Splat bijection,
and native `.text` prefix comparison. Compiler overrides are currently source/object
keyed in `config/compiler/object-flags.cmake`; do not invent supported comment flags.
These are migration seams, not license to relax validation. The panel example is
14 wrappers across battle15/game00/shop00 with repeated compiled symbols: first
evaluate target-local groups, never concatenate all 14 into one linked object.
No source consolidation or map/layout/compiler migration has occurred yet.
Read-only scout `01a089ce-f27b-7612-8b68-475ebbb785f7` completed and closed within
its original 240-second bound. It recommends evaluating per-function generated
compilation units rather than whole-object relinking for noncontiguous panel ranges.
That is an architecture lead, not validated code; C1.3 must choose and prove the
approach, including shared state/linkage, compiler settings and execution closure.

Retained history follows. The skill-only correction above governs current execution;
old transport commands, permissions and NOT RUN/readiness claims are historical.

The latest discovery slice adds reviewed within-function assembly blocks under the
[macro specification](../agents/macros.md#assembly-blocks). An explicit experimental
floor of eight instructions yields 1,568 global four-use groups, 675 involving
battle/15; the largest displayed lead spans 67 instructions at four sites. Reports
are `out/reviews/codex-macro-blocks-battle15.json`,
`out/reviews/codex-macro-block-account.json`, and
`out/reviews/codex-macro-block-description.json`. Account replay and descriptor
fingerprints agree; 222 existing checks, exhaustive small-sequence/disposable
preparation probes, and independent code/docs review pass. No lifted source,
index, frozen membership, or proof changed. Human-value evidence is required for
block artifacts; AI reranking, bounded top-N execution, and actual source acceptance
remain unfinished. The experiment does not establish a project-wide size default.

The latest macro consumer-coverage correction closes the independently reproduced
literal-include omissions; its current contract lives in
[macros.md](../agents/macros.md). Six disposable omission probes now reject while
covered scopes pass, and their dependency sets agree with native C89 preprocessing.
Default dependency lists remain unchanged for all 977 manifest source/support
inputs. All 343 scoped source/macro/type/naming checks pass; reviewer
`01a084c1-85d3-7b01-8298-e63590cd5e6a` accepted the final code/docs correction.
No lifted source, index, or frozen proof changed. This closes the reviewed tooling
gap, not macro acceptance, ranking-loop requirements, or a campaign phase.

Macro-tooling continuation: reverse-index v13 repairs unique target-owned
source/function associations for absolute and relative paths; ambiguous and
generated owners remain unlinked. Native rebuild retains all 6,580 macro uses and
links 1,010; an in-memory comparison preserves every other use fact. Read-only
`bin/macro-audit describe ID [--target TARGET]` exposes member sources, existing
file-level lexical macro context, and review gates without narrowing the global
candidate fingerprint. The pilot report
`out/reviews/codex-macro-resolution-panel.json` retains ten members (three focused,
seven external) and identifies `PANEL_ADVANCE_X`; global/focused/account
fingerprints agree. All five opportunity kinds pass native inspection probes;
238 existing macro/index/status/plan checks pass. Independent tooling review
accepted the multiline-context correction. No source or frozen proof changed;
semantic guards, independent private exact proofs, and shared promotion remain
unresolved. This advances discovery/inspection, not candidate acceptance or phase
completion.

The subsequent impact lookup follows global indexed consumers and target-qualified
definition-body dependencies, including cycles, without claiming preprocessor
binding or expanding write scope. `bin/macro-audit impact DEFINITION_ID` and the
descriptor's impact section expose fourteen `PANEL_ADVANCE_X` uses across three
targets: ten with limit 320, four with limit 17. This is wider than both the focused
pilot and its exact group. Evidence is retained in
`out/reviews/codex-macro-impact-panel.json` and
`out/reviews/codex-macro-resolution-panel-impact.json`; the candidate fingerprint
and frozen membership are unchanged. All 176 existing scoped checks and disposable
transitive/cycle/target-isolation probes pass; independent tooling review found no
blockers. Real macro resolution still owes use-site/semantic review, native private
proofs, and safe coverage of external consumers before any shared change. The
impact report remains incomplete lexical evidence, never an `all_use_sites` proof.

The current user goal requires the lifting, evidence-based naming, indexing and
type/macro cleanup pipeline to work in Codex and ultimately produce byte-faithful,
readable source across the codebase. Codex's available tools now execute the
repository commands and independent reviews; this supersedes the earlier Pi-only
execution assumption, not domain ownership, review, rollback or acceptance gates.
Historical Pi recovery is not proof of Codex recovery. Keep the frozen five-entry
pilot and all broader unfinished obligations; subprocess probes alone cannot
complete S3.5, S3.2, S4 or whole-game reconstruction.

This continuation repairs reproduced tooling failures: workspace inspection's
optional Git-index refresh rejected otherwise valid type/macro transactions;
duplicate mission selection queried a removed `hash` column instead of the
reviewed hash/size identity; owner-group cancellation killed the cleanup supervisor;
native-check interruption lacked cleanup; partial index responses and blocked
writes could exceed their deadlines. No source, identity, type or macro promotion
follows from these repairs. Existing checks and disposable process probes provide
implementation evidence; independent review and full validation remain separate.

Codex reviewer `01a08459-58dc-7ac3-86db-08057e5bbbec` independently accepted this
tooling slice, including the existing environment-fixture adaptation; no tests
were added. All 1,910 collected cases have outcomes across split runs: 1,908
passed and two skipped. The Git-object mode check and five historical-compiler
checks passed separately with approved sandbox escalation. Final naming/process/
root checks passed 68 cases; lint, format, maps and plan checks passed. The selected
`passesBytePairGate` still matches 20 instructions and 80 bytes. The initial source
audit was interrupted (exit 130); the performance follow-up below resolves it.

The performance follow-up reuses proven manifests/SDK space within each operation
and filters impossible whitespace-delimited tokens before CMake's unchanged
missing-source regex. Every manifest load still resolves and hashes each claim;
comparison reloads ownership after building. Reviewer
`01a08476-b44e-7be3-9e0b-3595295e57d1` accepted the retained changes and typed-error
repair. Parent-directory caching was rejected for a symlink race and removed;
its interrupted exploratory profile is not validation evidence.

The 20-lift GAME01 audit fell from 25.834s to 13.063s after manifest reuse
(102 to 42 loads). Complete CLI profiles then measured 13.158s before and 11.610s
after CMake scan filtering; all reports are identical, with native build products
warm and status summaries bypassed for the repeats. The scan retained all 977
paths while falling from 1.080s to 0.249s. Warm full-report profiles remain about
2.5s; no meaningful warm-cache speedup is claimed. Profiles and JSON live under
`out/reviews/codex-decomp-status-*`; `docs/agents/tool-usage.md` gives reproduction commands.

Final uncached `decomp-status` independently recomputed all 920 lifts across 23
targets: 817 exact, 103 partial, zero invalid, with complete JSON equality to
`out/reviews/codex-source-audit.json`. The final report is
`out/reviews/codex-source-audit-optimized.json`. Existing checks again cover all
1,910 cases: 1,908 passed across split runs, two skipped for missing disposable
forensic inputs. Five sandbox-sensitive compiler failures passed on approved
retries; the Git-object mode check and compiler-pipeline group passed separately.
Lint, formatting, maps and live `passesBytePairGate` instruction/byte checks pass.
No tests, source lifts, symbol maps or frozen pilot evidence were added or changed
by this performance follow-up; audit totals do not establish whole-game completion.

The subsequent frozen battle15 member cleanup corrects only comments in
`func_800B2218.c`, `func_800B22AC.c` and `func_800B250C.c`: canonical
`@residual none` replaces an invalid exact-status residual, and original-backed
behavior replaces UNKNOWN. All three add 32 modulo 65536 to panel x, interpret
the result as signed, then clamp to 320 and clear state only above 320.
Executable source is unchanged. Parent and independent BOF3 reviewer
`01a08494-61ba-7353-9aa8-aaf82c0984f3` verified each 16-instruction/64-byte match;
parent compiler failures passed with approved sandbox escalation. Existing
source/index/macro checks passed 139 cases; whitespace checks pass. No installed
formatter was available; no dependencies were installed.

All 23 snapshots remained fresh. After source acceptance, parent ran `bin/index`
once: all three lifecycle rows now read exact, and the selected
`exact_group:8e1ad03b4ba92303` query clears its registry-level no-exact-member
blocker. `out/reviews/codex-panel-macro-after-index.json` still reports blocked
source-shape/independent-member/semantic acceptance. Seven cross-target members
remain report-only. Frozen report and pilot-input hashes are unchanged; no names,
types, shared implementations or historical receipts were promoted or rebound.
This supersedes historical invalid-status statements for these three only, not
the frozen macro obligation, full-target naming or any phase acceptance.

Retain the review's limits: initialization uses separate bounded send/read
allowances, noisy startup can exhaust a write deadline, and the naming evidence
collector's native timeouts discard partial output. Actual Codex cancellation/resume, escaped-session descendants and
cancellation-time transaction rollback remain unproven. Existing coverage-only
inspection rejects nested Splat metadata and reports unmanifested STR files;
analysis readiness does not establish coverage or semantic naming completion.
Next: finish native recovery before claiming unattended
operation, then discharge the frozen live naming/type/macro and broader source
obligations through their existing owners. No phase status changes here.

The latest Codex integration follow-up moves macro/type/naming query parsing and
adapters into each domain's `cli.py`; `rev-query` now composes those owners.
Existing parser contracts and 176 focused checks passed. All 93 domain/common
modules satisfy formatting and the 450-line ceiling; formatting preserved ASTs.
Four `dispatchWorkTable69*` wrappers received only canonical exact-status residual
metadata. Independent review and live native checks confirm 32 instructions and
128 bytes each; the refreshed index recognizes all four as exact. The refreshed
macro ranking in `out/reviews/evidence/codex-macro-dispatch-ranking.json` has pin
`v1:60c90358ce826d520543986e1986ed33d78bf7f6193be267e346e793b82c9966`:
one defer, four rejections, zero selected. A generic macro must not hide the
local `REGISTER_PIN`; no extraction or type declaration was accepted.

That dispatch checkpoint is historical: its aided exact results and permission
to retain the local pin are superseded by the clean-C aid-removal correction above.
Keep its measurements and ranking pin; requeue the affected wrappers for fresh
clean-C matching and review.

Application/revalidation runtime preparation now shares naming postapply's
bounded process owner with macro/type gates: 120 seconds and 2 MiB of native
output per command, failed timeout/overflow receipts, and owned-tree cleanup.
Injected-runner APIs and receipt schemas remain unchanged. The existing gate
suite passed 287 checks during implementation; direct native probes cover text
compatibility and clipped-UTF8 timeout/overflow behavior after review fixes.
This does not establish actual Codex cancellation-time transaction rollback.

The **broader pipeline remains the acceptance target**:
- **Recovery/S3.5:** the user authorized the bounded `/tmp`-only rehearsal.
  Native cancellation exposed the missing durable recovery mapping described
  below; this is still an implementation gap, not an authorization blocker.
- **Naming/S3:** caller/role/session-bound terminal reservation and the frozen
  live naming obligations remain unfinished; inventory is not semantic closure.
- **Types/macros/S7–S8:** reviewed private-to-shared application, independent
  live consumer proof and the macro bounded-attempt accounting remain unfinished.
- **Lifting/finalization/S4–S9:** the gated lift-to-name-to-cleanup sequence,
  pause/resume/batching and broader coverage still require end-to-end evidence.
No phase is promoted by this integration work. Frozen reports, historical
receipts and original parent approvals are not rebound to changed tooling.

The authorized rehearsal used `/tmp/bof3-codex-recovery-prhacsqm`, one writer at
a time, within the 08:13:41–08:23:41 UTC window. Native Codex tool cancellation
ended sessions 79316 and 17461 without executing Python exception rollback:
the fixture header remained POST, no application was published, and child
heartbeat activity stopped. Reusing the stale PRE manifest rejected before edits.
Explicit parent recovery through the production rollback/quarantine primitives
restored PRE bytes and recorded modes; displaced POST evidence was retained.
A fresh invocation then passed all four fixture-native gates and integrity replay
with external pin `v1:9a2b08a09e0bc74def3ffa549b5207bfd56523d1314d1fac932b3961873ccfe7`.
`out/rehearsal/{cancellation-observation,recovery,result}.json` under that fixture
retain the observations. Live repository status, index and frozen report stayed
unchanged. The fixture used no mocked production callables, but its binary/native
outputs are synthetic, the application is legacy/non-context-bearing, and no
Git-index recovery, parent source acceptance or Codex agent-session resume is
proven. No phase is complete on this evidence.

The first implementation layer now persists root/manifest/run-bound PRE images,
mode/inode facts, intended POST and reserved quarantine destinations before moving
source files. Capture failure aborts before source writes; new directory links
are synced and post-rename mode drift rejects. This is recovery evidence, not
restoration authority or authenticated proof that a writer has terminated.

A follow-up bounded fixture at `/tmp/bof3-codex-recovery-ecmduidg` completed within
08:33:55–08:43:55 UTC. Recovery record pin
`v1:f33a5977df86fea2874d2c1e284a05bddb6ed2363870df3e3c803fd11b5053cf`
was observed before native cancellation of session 64316. The mapping survived;
explicit parent verification/recovery restored the original inode, bytes and
mode. A fresh invocation passed four synthetic gates and integrity replay at
`v1:3c3027f2b075c4661d737e3c7026d23fa00ba980081e8bb86fb473a476bf9f43`.
The fixture's `out/rehearsal/journal-recovery.json` retains the results. No source
acceptance, Git-index recovery, automatic recovery or agent-session resume is
proven; original live inputs remain unchanged.

Read-only recovery inspection is now exposed by both owner CLIs with an external
recovery digest pin. It validates recovery record/root/owner bindings and PRE
material, reports current source/quarantine/publication observations without
source images, and explicitly leaves manifest re-derivation, POST inode binding,
writer termination/exclusion and workspace/Git verification unproven. This is an
inspection checkpoint, not automatic recovery or pipeline acceptance.

The inspection checkpoint passes 210 existing file/domain/history/shared-PRE/CLI
and harness checks plus the separately escalated file-mode check. Disposable
POST/PRE, CLI, read-only and malformed-record probes pass; independent review
accepted the strict root-identity correction. Ruff, 128 local documentation
links/anchors and all eight migrated skill metadata checks pass. The broader
suite and guarded restoration are not completed by these results. The live index
and frozen naming report remain unchanged, and plan phase states are unchanged.

The user authorizes checkpoint commits after feature completion, without pushing,
then continued goal work. Local commit `88d6925b` contains only the seven reviewed
panel/dispatch metadata corrections; fresh native asm-diff and byte-match pass
for every member. Other source/header/map changes and the broader tooling refactor
remained uncommitted at that checkpoint. Review of the deleted
`test_agent_context.py` identified active compatibility, read-only, bounds and
transport coverage alongside obsolete Pi protocol checks.
Do not silently restore that deleted file or count its missing coverage as passed.
The user subsequently approved a tooling/docs/skills staging scope explicitly
preserving the existing `test_agent_context.py` deletion. That resolves the
checkpoint disposition without restoring the file; applicable lost coverage
remains a follow-up. Local settings, media, generated evidence and unreviewed
source/header/maps stay outside the tooling checkpoint.
Retained proofs are not rebound to the new checkout state.

Next implementation: explicit, restartable owner recovery requiring writer
termination/exclusion, rejecting unexplained drift and reconciling application
publication. Owned-file recovery must not claim coverage for unrelated workspace
or Git-index mutations without durable backing and corresponding validation.
Macro/type application and check-only revalidation now hold a common nonblocking
writer lease across canonical manifest re-derivation, source application, native
gates, publication and exception rollback. Root/path/inode drift and nested
writers reject; the private persistent lock is never removed on release. Naming
report-set locks and nonparticipating commands remain separate. This closes the
cooperating-owner exclusion gap, not guarded restoration, cross-domain campaign
scheduling or prior-worker termination. Native handles remain the termination
evidence; stored PIDs alone are insufficient across tool PID namespaces.

The lease rehearsal at `/tmp/bof3-codex-recovery-2eb_jrgx` ran within the bounded
09:11:21–09:21:21 UTC window and closed at 09:14:26 UTC. Native session 31625
held the lease while type, macro and revalidation contenders rejected. The parent
retained recovery pin
`v1:f0697e02fa67b85d2efbad6af2c06f8cbf59653d272067430802990f13f97f16`
before native cancellation (terminal exit 1), confirmed the child heartbeat
stopped, reacquired the lease and explicitly restored the original inode, bytes
and mode. A fresh invocation passed four synthetic gates and integrity replay at
`v1:05d56f3c8f8b333edee8c220575dce95b858d52085681f1b2e2ede1edbe1e1ce`.
The fixture's `out/rehearsal/lease-recovery.json` retains the result. No automatic
restoration, BOF3 source acceptance, Git/workspace recovery or Codex agent-session
resume is proven. The live reverse index and frozen naming report are unchanged.

Lease validation passes 260 existing file/domain/history/review/shared-PRE/harness
checks, 56 revalidation/CLI checks and the separately escalated file-mode check.
After the final per-primitive guards, 96 overlapping boundary checks pass again.
Twelve disposable rejection probes cover native contention, owner death,
close-on-exec behavior, persistent inode, substitution and unsafe file types.
Injected lease-loss probes preserve unexpected POST or missing-source state and
the original PRE quarantine rather than continuing publication/restoration.
Independent code/docs review reports no blockers; Ruff, decomposition, whitespace,
98 documentation links/anchors and unchanged plan-phase checks pass. Full
`just check` and broader pipeline acceptance are not claimed.

Recovery v2 now stages POST images in fresh directories before any source
mutation and binds exact hash/device/inode/mode plus staging and reserved
PRE/POST quarantine names. Verified no-replace installation preserves the prepared
inode; rollback rejects same-content substitutions. New-file mode binds the
actual umask result. `common/directory.py` owns descriptor traversal with direct
verb-named imports; `common/images.py` owns staging/installation. Existing fault
injections move to the installation owner without changing their assertions.
Historical v1 inspection stays content-only and is not upgraded.

Independent review found that subsequent Git-backed workspace rollback rewrote
already-restored owned files. It now skips byte-identical PRE paths, preserving
their restored identity/mode. A full-owner failing-gate Git fixture with explicit
baseline adoption verifies this, including reserved POST retention and absent
application publication. This does not establish durable unrelated-workspace or
Git-index recovery; those remain separate requirements.

The v2 native rehearsal at `/tmp/bof3-codex-recovery-jb9069uf` ran within the
09:32:49–09:42:49 UTC window and closed at 09:38:03 UTC. The parent retained pin
`v1:e5f5b7a364adbc35ff163c4b7d2c4a769f6d6671f16ee635ceed4cdb64f21de0`
before cancelling session 60787 (terminal exit 1). With the writer stopped and
lease reacquired, a deliberate same-content/mode clone was reported as drifted
and preserved by refused rollback. Restoring the known POST inode then allowed
explicit parent recovery of original PRE inode/mode/bytes, with POST at its
reserved quarantine. A fresh invocation passed four synthetic gates and integrity
replay at
`v1:c24d20f53f7b94aa76988495c67038b078bdba6332ff278b99b1c8a64c1b57e9`.
The fixture retains `out/rehearsal/v2-recovery.json`. Disposable probes also verify
new-file umask/identity, capture-before-source-mutation, staged-clone rejection and
unchanged historical v1 inspection. No native BOF3 source acceptance or Codex
agent-session resume is claimed; the live index and frozen report are unchanged.

Recovery v3 now reuses the owner's captured workspace/index snapshots to retain
private untouched PRE images/metadata and exact index bytes/path/state alongside
v2 source identity evidence. Guard inspection reports scoped additions, removals,
content/metadata drift and index agreement/locks without exposing archived bytes.
This inventory follows tracked/unignored workspace-backup policy and excludes
generated-artifact roots, not every filesystem path. Non-Git or unsupplied
snapshots are explicitly unavailable; v1/v2 records are not silently upgraded.
Durable backing and observational guards close prerequisites but do not grant
restoration authority or establish atomic, complete workspace/Git verification.

Remaining automatic recovery must independently establish prior-worker
termination, reconcile publication and implement guarded restoration using the
retained source/workspace/index backing. Neither snapshots nor a newly acquired
lease supplies those authority and lifecycle facts.

The v3 Git-backed rehearsal at `/tmp/bof3-codex-recovery-sizl4nt_` ran within
09:59:51–10:09:51 UTC and closed at 10:02:48 UTC. Session 38144 reached a native
gate with source POST installed and matching workspace/index guards. The parent
retained pin
`v1:e14a62937c7675737a8d1713c49345f36d3cbdd3197a508afa6d8f076dc8c2f8`
before cancellation (terminal exit 1). Guard matching and archived bytes survived;
controlled untouched-content drift, an added file and an index lock were detected.
Explicit parent recovery under the reacquired lease restored owned PRE, and a
fresh invocation passed four synthetic gates plus integrity replay at
`v1:a5481ea4252e40daf6e664c5570deff2b8c18a1893005ed9bcf7a720a5973dce`.
The fixture retains `out/rehearsal/v3-recovery.json`; no automated restoration,
native BOF3 source acceptance or Codex agent-session resume is proven.

A separate native Git probe detected staging of an owned edit even though that
path is excluded from untouched-file comparison. Reconstructing an index snapshot
from archived bytes lets the existing primitive restore index bytes/mode/ownership,
but its new inode remains distinct from the original exact guard. Future recovery
must record this verified identity transition, not rewrite the original record or
pretend its guard still matches. Exact index backing is not a complete Git-metadata
backup. The live reverse index and frozen naming report remain unchanged.

V3 validation passes 196 existing owner/file/harness checks and 120 existing
history/review/shared-PRE/revalidation checks. Independent malformed-image and
scope/redaction probes pass; no new persistent tests were added. Archival reuses
the owner's existing reads, but increases storage: the observed live inventory
contained 2,068 files and about 14.85 MB of PRE data, roughly 19.8 MB in base64
before JSON/metadata overhead. This is a measured cost, not a new size limit or a
claim that the full pipeline is complete.

The v2 checkpoint passes 196 existing owner/file/harness checks, 120 existing
history/review/shared-PRE/revalidation checks and the escalated file-mode check.
After final verb-name migration, 36 overlapping file/decomposition/import checks
and the Git-backed full-owner rollback probe pass again. Independent review
accepts the inode-preservation correction and documented scope. Ruff, formatting,
98 local links/anchors, whitespace and unchanged phase-state checks pass. No new
regression tests or dependencies were added; broader acceptance and context
coverage remain open. No additional commit or push at that v2 checkpoint.

The accumulated harness/domain/recovery checkpoint is committed locally as
`5dd2ab20`; no push occurred. The subsequent source checkpoint accepts
`passesBytePairGate` at `emi/battle/battle/15@0x800A3638` (20 instructions,
80 bytes) with matching `s32` definition/prototype, byte arguments and reviewed
`BattleLocalWork` fields at `0x86/0x87`, preserving size `0x140`. Reviewer
`01a084ec-947a-7a72-ad93-604fa3c331f9` independently verified final native
instructions/bytes and header/map/layout scope. This return spelling is a
conservative reconstruction, not proof of the original declared width.

Explicit caller-side `(u8)` conversions restore `resetSelectionApplyInput`
at `@0x800A41D8` and `querySelectionApplyInput` at `@0x800A4238` in the same
target to 24/24 instructions (96 bytes) and 23/23 (92 bytes). The unchanged
callee declaration remains `s32`; reviewer
`01a084ec-94e4-7473-a0d0-f0c691222ff7` independently accepted both byte matches
and semantics. The [signedness lesson](../agents/matching-playbook.md#signedness)
records the distinction between caller conversion and callee declaration.

The user specifically authorized installing clang-format; version 23.1.0 is now
local to `.venv`. All three candidate C files pass formatting and `bin/promote`;
target symbol checks and Splat regeneration pass. Existing header-wide formatter
violations reproduce at HEAD and remain outside this source change. Source,
header and map edits can stale analysis/cleanup evidence; no frozen index, naming
report, ranking or receipt is rebound. Whole-target naming, macro acceptance,
automatic recovery and the broader pipeline remain unfinished; no phase advances
from these three source validations.

Source/manifest and plan/harness checks pass 118 existing cases, including the
Git-object file-mode probe rerun with sandbox escalation. Plan parsing and
whitespace pass; full `just check` and a new global source audit were not run.

The next recovery slice fixes a reproduced replay failure: a second source
rollback rejected the already-restored original PRE. Identity-bound rollback now
checks complete backup/image coverage and consistent PRE/POST locations before
each path's transition; already-restored PRE is a no-op and an interrupted missing
destination resumes from its original quarantine. The shared
[recovery contract](../agents/harness.md#policy-versus-mechanism) retains authority,
non-atomic observation, publication, workspace/Git and durability limitations.

Native tool session 8495 exercised this primitive under `/tmp` at
13:45:18–13:45:28 UTC. Fixture `/tmp/bof3-rollback-native-cq79wiuz` paused after
the first original file was restored, the second POST was quarantined but PRE
not restored, and a newly created third file remained POST. The externally
retained recovery pin is
`v1:e171d13e41653a134ee85ce9536eb8e497c8d8c530211eef9bab2ac19fdbd8ca`
for `out/reviews/evidence/type-recovery-414edffa5e90a9c437a89459ec8f0ca1.json`.
Ctrl-C produced terminal exit 1. Explicit parent resume under a fresh writer
lease restored original bytes/modes/inodes and new-file absence; repeated rollback
passed without replacing restored inodes. This is actual tool cancellation during
disposable source restoration, not native BOF3 gate acceptance, whole-owner
automatic recovery or an agent-session resume protocol. Frozen live index and
naming-report hashes remain unchanged.

Eight disposable cases additionally verify equal/empty PRE/POST bytes, corrupt or
substituted PRE, mode/link drift, missing POST and incomplete scope; refused states
preserve all observed bytes/inodes/modes/links. Independent reviewer
`01a084ec-947a-7a72-ad93-604fa3c331f9` accepted the bounded implementation and
320 mock-only state combinations. Existing owner/file checks pass 186 cases;
plan/harness checks pass 56 and the escalated Git-object mode probe passes.
Reviewer `01a084ec-94e4-7473-a0d0-f0c691222ff7` independently verified the retained
pin, original PRE identities/bytes/modes, absent new file and all three exact
quarantined POST images, without claiming to have witnessed cancellation timing.
The Git-backed full-owner failure probe passes at
`/tmp/bof3-codex-recovery-kh4lhswi`; its first invocation lacked the disposable
fixture's test-import path and was corrected without repository changes.
No persistent tests or dependencies were added. Guarded recovery and the bounded
loop remain unfinished; no phase advances from this prerequisite.

Guarded source-only parent recovery is now implemented by `type-audit recover`
and `macro-audit recover`; the [canonical contract](../agents/harness.md#guarded-source-recovery)
owns its closed authorization schema, mandatory lease, matching v3 guards,
publication refusal and replay checks. The CLI authenticates neither actor IDs nor
tool output: the supervising parent attests stopped writers from actual native
evidence, retains an independent review, and pins that authorization separately.
Recovered PRE is not source acceptance or permission for automatic retry.

The native cancellation fixture `/tmp/bof3-codex-recovery-f4paaqpl` started at
13:59:33 UTC; tool session 77110 reached retained source POST, then Ctrl-C returned
terminal exit 1. Its bounded pause had no child writer. Record pin, retained before
cancellation: `v1:fef66b14b0f0f57eba1122c48f5db1fdf363aa2f27aaec98f147fe4e4a88df13`
at `out/reviews/evidence/type-recovery-79a484dcbccf87d8e256078a8df3bb01.json`.
Reviewer `01a084ec-947a-7a72-ad93-604fa3c331f9` independently inspected POST,
original PRE quarantine, guards and absent publication, approving only fixture
source restoration subject to fresh CLI checks; cancellation timing remained
parent-attested. Parent authority pin:
`v1:edf4ae747f1a3a09be232455e3974d707408c1e4d28f8c5d540f13f4e2963e58`.
The actual type CLI restored `include/test.h` to original inode 5382625, mode
0664 and bytes; replay passed at 14:01:35 UTC with completion pin
`v1:22d94f60428d5af523371a1d975b5ecd87a73067724bd62fe232926d21c634f8`.
This fixture uses real capture/restoration and Git guards with a synthetic
manifest, not BOF3 native-gate or whole-owner campaign acceptance.
The same reviewer accepted the final PRE/POST identities, guard agreement,
absent publication and all three pins; termination remains parent-attested.

Independent macro CLI restoration/replay passed in
`/tmp/bof3-independent-macro-recover-8o41662x` under explicitly synthetic authority,
pin `v1:950ee0beeed884691570490b0a6edb4df2556950daf1ec9bf693e1cb5bc7c9ec`;
reviewer `01a084ec-94e4-7473-a0d0-f0c691222ff7` verified original PRE, retained POST,
unchanged receipt bytes/inode/timestamps, guards and absent publication. Initial
type CLI/replay under synthetic authority also passed in
`/tmp/bof3-codex-recovery-g_2drq7l` at
`v1:7bf48bcc2b9360f92842e1eddfa3aca4fcf488010939cc0b3e10ed58815fa9a6`.

Review-driven corrections now reclassify every owned image before each transition
and complete PRE after receipt publication, revalidate authority at completion,
check private receipt metadata and reject equal-valued float/bool binding fields.
Twenty-one mock adversarial scenarios, four independent binding-type rejection
probes, historical v1/v2/v3 inspection comparisons and 243 existing owner/file/
plan/harness cases pass. No persistent tests or dependencies were added. Full
`just check` and a new global source audit were not run. Published-application
reconciliation, workspace/Git restoration, durable automatic/session recovery and
the bounded ranked execution loop remain unfinished; no phase advances here.

Request-bound `type-audit resume` and `macro-audit resume` now use the existing
live owner verifiers, not a second reconciliation receipt or scheduler. The
[shared contract](../agents/harness.md#request-bound-resume) binds the original
manifest/request, captured implementation run and complete application. Valid
unreviewed POST yields `needs-review`; an independently pinned accepted envelope
for that same application yields `skip-accepted`. Invalid supplied acceptance
rejects rather than downgrading. Neither disposition applies work or resets budgets.

Full-owner type CLI fixture `/tmp/bof3-codex-recovery-uzbwpbhm` executed four
synthetic native gates, including real CMake/Ninja, with captured execution context.
At 14:10:40 UTC session 43002 started; it published application pin
`v1:e99a908fd3b319d1268b8f7ab121af77de4be271f9e1eee862932de074fbc895`, then paused
before review. Parent retained that pin before Ctrl-C (terminal exit 1). Original
manifest pin is `v1:ad3fad3ff6918c9ac2f63faed6ab031d61db6cd9232465c238960726cea24730`;
run is `postpublication-fixture-implementation`. Fresh resume returned
`needs-review`, with complete proof pin
`v1:1260a156c820858b4f8c086e14d958d14dc4b3c9287793f352bd18633785b8a6`.

Reviewer `01a084ec-947a-7a72-ad93-604fa3c331f9` independently verified that
application and baseline for fixture-only acceptance. Existing parent review
published envelope pin
`v1:05a828d58f2c76475ef3a8174005546e243a56d81133ce9ce7447fa8c9865569`; session
5346 paused after acceptance at 14:16:16 UTC, then Ctrl-C returned terminal exit 1.
Fresh request-bound resume returned `skip-accepted`. A disposable preservation
probe verified 16 source/index/evidence files retained bytes, inode, mode, link
count and timestamps, with no duplicate application or receipts.
The same reviewer independently confirmed final acceptance, `skip-accepted`, both
pins and preservation, accepting only the scoped native continuation cells.

Execution-context checking was not relaxed. Plain invocation initially failed;
diagnostics isolated only `environment_digest`. Parent replay required the original
compound PTY launch and `PYTHONDONTWRITEBYTECODE=1`. Independent verifier replay
also explicitly reproduced the original `CODEX_THREAD_ID`/`CODEX_SESSION_ID`
runtime-input values (`01a08454-8f36-7af3-b72d-d98bb3daa9f5`), while retaining the
actual independent reviewer ID in acceptance evidence. No proof, guard or actual
Codex session identity changed. A prior disposable full-owner attempt correctly
rejected unignored generated CMake files; the next fresh fixture supplied its
missing `.gitignore`, not a relaxed unrelated-workspace guard.

Independent callback/CLI probes cover both owners/dispositions, ten pre-verifier
rejection cases and eight verifier failures. Existing checks pass 217 cases,
including the escalated file-mode probe; Ruff, formatting, whitespace and plan
parsing pass. No persistent tests or dependencies were added; frozen live index
and naming-report pins remain unchanged. These native fixture cells do not prove
BOF3 semantic acceptance, whole S3.5 or the durable bounded queue/budget loop.
Full `just check` and a new global source audit were not run.

Native hard-stop investigation exposed a real lifecycle defect: killing the owner
terminated its direct child but left a `setsid` grandchild running. The observer
used pidfds acquired for its own spawned processes in the same namespace, not
stored cross-namespace PIDs or heartbeat silence. Before-fix evidence at
`/tmp/bof3-escaped-child-1qpqddua/result.json` distinguishes failed supervisor
cleanup from the observer's subsequent explicit survivor cleanup.

Linux supervision now uses a fresh subreaper, sole-waiter descendant reaping and
a private completion ACK. Missing ACK or cleanup timeout raises
`ProcessCleanupError`, retaining the still-cleaning guardian and stopping type/
macro restoration and naming finalization. Non-Linux rejects before spawn; no
process-group-only fallback claims equivalent safety. The
[shared lifecycle contract](../agents/harness.md#policy-versus-mechanism) owns details
and preserves the separate lease/quiescence and parent recovery obligations.

Parent native hard-stop, normal exit and timeout controls pass at
`/tmp/bof3-escaped-child-{j5e4ovv4,cjyqoi3g,ona6zr5c}/result.json` with owner exits
-9, 7 and 124. All observed pidfds were terminal before observer cleanup. Retained
before/after evidence is pinned by `/tmp/bof3-escaped-child-reviewed.sha256`, SHA-256
`a175fa2f91f0d509ade1fee5f3b8e8458ca170ee5af3efa93872e0fed56530f2`.
Python lacked pidfd bindings, but an actual x86-64 kernel preflight passed;
architecture-specific ctypes syscalls stay in the disposable probe, not production.
These are observer-triggered native SIGKILL/timeout cells, not a claim that the
Codex stop endpoint itself supplies pidfds.

Reviewer `01a084ec-94e4-7473-a0d0-f0c691222ff7` independently repeated all three
native cases at `/tmp/bof3-escaped-child-{mxkoqh3o,44806hdf,jhf3usd3}`, accepting
those lifecycle cells only. It also verified missing-ACK refusal across handle
APIs, guardian retention, descriptor cleanup, exit status preservation and naming
fail-stop behavior. Reviewer `01a084ec-947a-7a72-ad93-604fa3c331f9` verified four
Git-backed type/macro owner probes: uncertain cleanup preserves POST and original
quarantines with no rollback/publication; ordinary failure still restores PRE.
Their runners were injected, not native descendant evidence. An additional
first-install failure probe at `/tmp/bof3-first-install-failure-0m9zp7w6` disproved
a suspected backup/record coverage mismatch; no fix was needed.

Existing checks pass 326 cases (owner/revalidation, process/naming, harness/plans
and escalated file-mode check). Ruff, formatting and whitespace pass; no persistent
tests or dependencies were added, and frozen live index/report pins remain intact.
Native forward partial-mutation recovery, original-budget deadline expiry and the
resumed bounded queue/budget workflow remain unproven; S3.5 stays in-progress and
S3.2 stays blocked. Full `just check` and a new global source audit were not run.

### Budget and failure rehearsal checkpoint

The read-only budget validator and absolute native deadline contract now live in
[harness.md](../agents/harness.md#bounded-continuation). This is not a scheduler:
the parent retains the latest externally pinned consumption position, durably
pre-debits work, checks owner freshness and establishes quiescence separately.
Neither fresh invocation nor repair resets the original allowance.

Native forward partial-application interruption used
`/tmp/bof3-codex-recovery-n0_bam7g`, session 56479 (terminal exit 1), after the
first real install and before the second source mutation or any gate. Recovery pin
`v1:ad478dd1591fd07aa0fa124c02ae152ccc0b67286685bdffb3a3b6a0bb3076dc`
bound header POST/original PRE quarantine and untouched C PRE/staged POST.
Reviewer `01a084ec-947a-7a72-ad93-604fa3c331f9` checked the actual states and
approved fixture-only recovery. Actual owner CLI recovery with parent authority
`v1:4b47990cc54b14fb7a65a192328ac16e54757c3c15898424fbbcec26d807a7bc`
produced completion
`v1:f18dbb6bfbd7cf7b7791ed84419312593ccd7b06c72c17669c97de0f12861631`.
Both original PRE inodes/modes returned; both POST images and unchanged guards
remain. No gate, application, acceptance or retry authority follows.

Original-budget expiry used `/tmp/bof3-budget-deadline-wzu9yjr_`, native session
90788 (exit 0), one read-only dispatch, zero repairs and a 12-second original
deadline. Externally retained budget
`v1:c8bf8ef5813dcf66d2e5911c00eb4de4b0778e98a4c6422125bba00fc6c76868`
and sequence-1 consumption
`v1:231e101d142132428de7be2652def3b024df738638c8ae36eb17beed2d8fb319`
were pinned before dispatch. Startup delay consumed time; the child stopped at
the original deadline, with confirmed descendant cleanup and protected source,
dirty file and Git index unchanged. A fresh invocation using the same pins and
sequence reported zero time/launches and dispatched nothing. Independent reviewer
`01a084ec-94e4-7473-a0d0-f0c691222ff7` repeated this native cell in
`/tmp/bof3-budget-deadline-cvrsn_yl`, including an unchanged inventory on resume.
The approximately 32-ms cleanup tail grants no work extension.

Native fixture tool failure in `/tmp/bof3-codex-recovery-nulu3b5r` stopped after
Splat exit 7 with one failed receipt and no application. Structured verification
failure in `/tmp/bof3-codex-recovery-pphg7i4b` passed Splat/build, then rejected
exit-zero asm-diff JSON `null`; byte-match did not run. Both owner CLIs exited 2
and restored owned PRE while preserving captured unrelated source/dirty/index.
These synthetic gate fixtures are not BOF3 source acceptance.
The earlier `/tmp/bof3-codex-recovery-8in6gwp8` negative control was invalid:
Splat diagnostic-looking output with exit zero satisfies its documented opaque
exit-status contract. Its published application and POST remain untouched; it
is not failed-verification evidence. No diagnostic blacklist was added.

These observations supersede the preceding unproven claims only for forward
partial recovery and original-deadline/exhausted fresh-invocation cells. S3.5
remains in-progress, S3.2 blocked and the broader pipeline unfinished. Independent
concurrent-editor probes confirmed a remaining P1: type/macro automatic workspace
rollback can overwrite unrelated changed bytes and quarantine unrelated new files.
The next scoped repair must preserve unexplained concurrent work, restore only
proven owned paths and inspect automatic index restoration for the same defect.
Uncertain-cleanup stop state across controller resume also remains unfinished.

Independent reviewers accepted the bounded code/docs and native failure cells.
Existing process/owner/revalidation/harness/plan checks passed 276 cases plus the
separately escalated file-mode check. Ruff, formatting, whitespace, plan status
and 75 local documentation links/anchors pass; frozen live index/report pins
remain unchanged. No persistent tests or dependencies were added. Full `just check`
and a new global source audit were not run; this is only the scoped checkpoint.

### Concurrent-work preservation repair

Budget/deadline checkpoint `de9d8229` is committed locally without pushing.
Independent editor probes in `/tmp/bof3-concurrent-type-32_cfy6g` and
`/tmp/bof3-concurrent-macro-wmbneevt`, plus staging probes in
`/tmp/bof3-staging-type-6jtr162y` and `/tmp/bof3-staging-macro-p1s1gbc1`, confirmed
that previous automatic workspace/index restoration erased unowned work.
Type/macro exception handlers now restore only captured identity-bound owned
source images. They never automatically restore the workspace or Git index.
External drift or inspection failure stops for parent review after safe owned
rollback; unreadable or foreign untouched paths cannot suppress that restoration.
Original raw index and all-PRE metadata safeguards are checked separately from
durable v3 recovery inventory. Non-Git checks remain weaker; observations are not
atomic and no lease excludes manual editors. See the
[owning recovery contract](../agents/harness.md) for guarantees and exclusions.

Existing destructive-expectation tests now require preserved external bytes and
staging while retaining drift detection and owned-PRE restoration checks; no new
test cases were added. The focused 21-case failure/publication suite passes.
Broader owner/revalidation/harness/plan checks passed 272 cases; the previously
run file-mode check also passes. Ruff, formatting, whitespace, plan status and
98 local documentation links/anchors pass; frozen live index/report pins remain
unchanged. Independent code/docs review found no blocker. Disposable patched
type/macro editor, staging and metadata probes passed separately from native work.

Actual native preservation used `/tmp/bof3-codex-recovery-4i_rt0f3`, terminal tool
chunk `741b18` (exit 0), with one counted writer dispatch, zero repairs and a
60-second absolute original deadline. Parent control
`v1:d04fd9bc7feb08277ce12cdb812dd3c209a6c3f9646c3078bff88a6072bc9cb0`
was pinned before launch. The real Splat fixture gate spawned and joined an
editor that changed an unrelated file and mode, created another file and staged
the edited file. Splat exited 7, owner CLI exited 2 for preserved external drift,
only the owned original header PRE returned, and all editor-post bytes, identities,
modes, links, mtime/ctime and raw index bytes remained exact. Unchanged C remained
exact too. One failed receipt and no application/later gate were observed; the
bounded dispatch finished in 0.691 seconds. Reviewer
`01a084ec-947a-7a72-ad93-604fa3c331f9` independently accepted the pins, restored
PRE/retained POST, editor-post state, receipts and absence of later gates or
application. Native tool termination remains parent-reported. V3 guard drift is
exactly the preserved editor work/staging, not a falsely clean recovery state.

Invalid setup controls remain untouched: `_cdbppj6` rejected an unsupported CLI
option, and `1nh5aaiu` rejected a manifest whose gate changed after preparation;
neither supplies successful gate evidence. The earlier explanation attributing a
setup failure to float-valued digest validation is withdrawn: that cause and its
pre-dispatch timing were not established by retained evidence. Do not treat the
rejected fixture's control file as a complete immutable attempt history. None
supplies acceptance or a reset of the separately pinned successful attempt.
This repair does not close
uncertain-cleanup resume, the bounded controller, S3.5 or the pipeline.

Next bounded cell: retain original budget/latest consumption pins and a real
`ProcessCleanupError` transcript, then use a fresh read-only `check_budget` plus
owner `inspect-recovery`. Budget remaining or a free lease must not override the
parent's cleanup-unconfirmed stop disposition. No new debit, writer, revalidation
or recovery is allowed until independent native termination review authorizes
the existing guarded recovery path. This proves parent-controlled refusal, not
automatic enforcement: arbitrary dispatch currently has no durable cleanup hold.
Do not introduce a generic controller merely to demonstrate this cell.

### Cleanup-unconfirmed fresh continuation

Concurrent-work preservation is committed as `bcee2854`, without pushing.
The next native fixture `/tmp/bof3-codex-recovery-xv5bph8t` used session 78045
(terminal exit 0), one counted owner dispatch, zero repairs, 60 seconds of original
work allowance and a separate 10-second cleanup tail. Watchdog and read-only
observers are supervision, not additional owner attempts. Original budget pin:
`v1:2e32522700247ddbaabab57470ec16515e164e300873578df3845a2c6ea0dcef`;
pre-debited sequence-1 pin:
`v1:975a7bfb04e4948c9c6154a2a647785ab6c9aeb532cb11b9d655f08c504d5e26`.

An independent watchdog started before dispatch. The native gate overflowed its
output bound while its guardian was deliberately stopped; actual production CLI
exit 2 reported `ProcessCleanupError`, leaving owned POST/original PRE quarantine
and no application or ordinary command receipt. Same-namespace observer pidfds
confirmed the guardian and gate remained alive. Retained failure pin:
`v1:1de3b38eb48e33d11e6b7877841b3488cdfddf9115d0aad60da8563703c00d93`;
recovery `out/reviews/evidence/type-recovery-81b5e96dda90d633a6e6130298e694ad.json`
has pin `v1:3e609d4f61164ff312af539102d90083e7b65194314488357e38b1975f03f20f`.

A fresh OS-process invocation used those original pins for `check_budget` and
actual owner `inspect-recovery`. It reported parent-controlled stop despite one
launch and 57.28 seconds remaining and an available lease. All 64 inventoried
file states stayed identical; no debit, writer, gate, revalidation, recovery or
application followed. The original observer confirmed both processes still alive
around inspection, then released the watchdog for cleanup. All observed CLI,
guardian and gate pidfds became terminal, their children were reaped and watchdog
exit was zero. POST/backing remain deliberately retained; no source recovery or
acceptance is asserted. Artifacts are `out/rehearsal/{budget,consumption-0,
consumption-1,failure,read-only-result,terminal,watchdog-result}.json` in that root.

An earlier separately bounded fixture `/tmp/bof3-codex-recovery-fns8p9ag` produced
the same real cleanup uncertainty, then reached its original deadline. Its
watchdog stopped/reaped observed processes with a 57-ms cleanup tail. A separate
tool invocation could not open saved PIDs from another PID namespace; its corrected
read-only attempt then rejected the exhausted original time allowance. Neither
failure caused another owner dispatch or reset; original pins remain unchanged.
The successful cell keeps native liveness in its original observer, never infers
it from stored PIDs, heartbeat silence or a free lease.

Reviewer `01a084ec-947a-7a72-ad93-604fa3c331f9` accepts the scoped cell inspection
and semantic recording. The disposable watchdog's early-error/handle-handoff path
was not proven; successful observed cleanup does not establish that path.
This is a parent-controlled fresh invocation, not Codex agent-session revival or
an automatically enforced persistent cleanup hold. S3.2 still owns the bounded
execution loop.

### Whole S3.5 acceptance

Reviewer `01a084ec-94e4-7473-a0d0-f0c691222ff7` independently audited the complete
failure/recovery matrix, checked the retained final-cell pins and image identities,
and explicitly accepted **whole S3.5** after Bohr's scoped cell review. Parent
accepts that verdict, not an inference from accumulated test passes. Historical
native observations remain historical evidence; they grant no current acceptance
to tooling-stale applications. Historical Pi preapply/session evidence is not
relabeled Codex session revival. The last cell proves fresh OS invocation and
parent refusal, not automatic persistent exclusion; the disposable watchdog's
unproven early-error handoff remains outside reuse approval.

This supersedes earlier pending/open whole-S3.5 claims only. S3.5 becomes done and
S3.2 in-progress; S3 remains in-progress, and S4 plus frozen live/full-target naming
obligations remain unchanged. The next dependency-ready work is one small bounded
Codex sequence using existing concern owners, native tools, frozen membership,
original budgets, independent stage reviews and explicit parent stop/recovery.
No generic controller is a prerequisite retroactively added to S3.5. S3.2 is not
complete until that bounded sequence is independently demonstrated; production
still cannot bypass unfiltered naming `complete:true` or separate identity approval.

Current documentation checks: 46 existing plan cases, plan status, whitespace and
98 local links/anchors pass. No repository Python, skill/agent Markdown, source,
index, report, dependency or global configuration changed in this milestone.
Frozen live index/report pins remain unchanged; broad Python/source audits were
not rerun for this evidence/documentation-only continuation.

## Proposed roles and sequence

Type/macro `run` now accepts an inherited absolute monotonic work deadline;
native gates, including partial-baseline preflight, no longer silently receive a
fresh full allowance beyond it. Forward edits and publication have cooperative
checks; original owned-image rollback uses the parent's separately retained cleanup
tail. `ProcessCleanupError` still stops speculative restoration. This is run-only,
not revalidation/naming collection or a completed S3.2 scheduler.
The native type fixture `/tmp/bof3-codex-recovery-eduu8e40` used one launch, zero
repairs, an 8-second work cutoff and fixed 10-second cleanup tail. It observed POST,
then owner exit 2, exact PRE bytes/inode/mode and raw index preservation, both
observed gate/child pidfds terminal and no application publication in 8.143740s.
`out/rehearsal/deadline.json` SHA-256 is
`4f95989fe4f7acba234b5c25369c3a3a0a2458800eabd71e976f041109fbbe1e`;
`deadline-result.json` SHA-256 is
`c1dd9a144812124b9d90719886d5c98b337c918ba01d27b5e16a3df8ae5041ea`.
These are fixture observations, not a new campaign-budget schema or BOF3 acceptance.
Both owners' disposable API probes cover timely success, gate expiry and retained
late publication after PRE rollback, plus inherited preflight limits and scope
reset. The earlier `fzglhs7f` control failed observer setup and CLI error normalization;
its observed PRE restoration is not accepted native-termination evidence. Independent
review accepts the scoped integration; whole S3.2 remains in progress.
Validation: 365 distinct existing checks passed across type/macro transactions,
shared application/history/revalidation, process ownership, harness structure,
file modes and plans. No regression cases were added. Native/API fixtures are
disposable characterization, not production source or naming acceptance.

The follow-up extends original cutoffs to type/macro `revalidate --deadline` and
naming `--work-deadline`, preserving naming's relative `--deadline` cap. Binding
precedes writer exclusion; naming session ownership cleans all acquired clients,
preserves cleanup uncertainty and skips forward finalization after expiry.
Revalidation never gains restoration authority. Partial writes and late verifiable
check-only receipts remain evidence, not budget acceptance or permission to advance.
Disposable API logs `/tmp/bof3-revalidation-deadline-api-final.log` (eight cases)
and `/tmp/bof3-naming-cutoff-api-final.log` (five cases) passed timely execution,
gate/commit/finalization expiry, retained publication and cleanup priority cases.
Native naming fixtures `/tmp/bof3-naming-work-index-e4tu8i_j`,
`/tmp/bof3-naming-work-byte-81l35hmh` and `/tmp/bof3-naming-work-rizin-gvx4y362`
each used one launch, zero repairs, five-second work and ten-second cleanup limits.
Their `deadline.json` / `result.json` retain terminal original process pidfds,
no expired finalization and unchanged report/index. Bohr accepted this scope.
Native check-only type fixture `/tmp/bof3-codex-recovery-0h1rjbgg` used one launch,
zero repairs, eight-second work and ten-second cleanup: exit 2 at 8.050233s,
original gate/child pidfds terminal, source bytes/inode/mode and raw index unchanged,
no new publication or later gate. Its synthetic review prerequisite is not BOF3
acceptance. `out/rehearsal/deadline.json` SHA-256:
`6f7aeca9f13b5e67927c1dd7e9753b67cf2f7a49a3d124c12f1c330a715cc11f`;
`out/rehearsal/result.json` SHA-256:
`f7e6e9c21dc6223937098c9887714b3cbf792d8d86117460f62c33cfc9768a25`.
Herschel accepted scoped code/docs parity and the retained revalidation evidence.
Validation passed 513 distinct existing checks: naming/revalidation 115,
owner/shared application, process, structure, skills/context, wrappers and plans
397, plus the separately escalated file-mode check. Ruff, formatting and whitespace
pass; frozen live index/report hashes remain unchanged. No coverage added. The
whole native sequence, original budget receipt and live owner queue remain
unfinished; S3.2 stays in progress with frozen queue and source gates unchanged.

### Integrated native sequence checkpoint

After local deadline feature commit `2118f979`, the fixed disposable driver
`/tmp/bof3-s32-sequence.py` joins actual owner subprocesses and native Codex review
handoffs under one original budget at `/tmp/bof3-codex-recovery-4duf1zqh`.
Prelaunch review closed request-pin drift, cleanup short-circuit, helper binding,
preparation timeout and watchdog handoff gaps before freezing any work allowance.
The two earlier setup-only roots `8n5vlba9` and `t4e7s9yj` never received a budget
or owner dispatch; they are not successful native runs or refunded attempts.

The four-entry fixture queue has 12 native-dispatch slots, zero repairs, two
separately accounted Codex review handoffs, 1,200 seconds of original work and a
fixed ten-second cleanup tail. Budget pin:
`v1:cb5fd493a7359c3ac6e5153f9c820cbfa0e457f6e601f7addefdcd996f328931`.
Eight durably pre-debited native dispatches reach high-water sequence 8:
`v1:eaf3c8a5f07f665740aabb4ca737fb4062b8a629aceb34eee8197ad9f4c4c625`.
Actual type apply → needs-review → independent Herschel review → parent review /
final verification → fresh skip-accepted passes without duplicate application or
new gates on skip. First envelope pin:
`v1:26fc9ac3e607b574488e4cf82d1b1d9aca4984e7cbd5a7f20d7f9b34bb5b5bc3`.

The subsequent macro account reports a real stale-input blocker. A separately
counted, original-cutoff-bounded fixture preparation refreshes only its disposable
index and creates a new candidate artifact, preserving the earlier proof. A second
actual type application produces cleanup uncertainty, retained POST/backing and
no application. Fresh process inspection matches guards and refuses advancement
while original gate/guardian pidfds are still nonterminal, despite four launches
remaining. Naming is deferred. Independent pre-exec watchdog supervision then
confirms CLI/gate/guardian termination and child reaping, with no cleanup errors.
No source restoration or retry follows; first acceptance is historical after the
declared later transition, not current source acceptance. Failure pin:
`v1:9fd5368b14f845140e956b889582e38fa0d52836df835c6e9a26649c5f92057b`.

Retained `out/rehearsal/{budget,consumption-0..8,native-*,independent-review,
parent,accepted-checkpoint,blocked,failure,read-only-result,terminal,watchdog-result,
sequence-result}` artifacts distinguish observed progress, blocked work and deferred
successors. Independent reviewer `01a084ec-947a-7a72-ad93-604fa3c331f9` accepted
this integrated fixture scope in `out/rehearsal/sequence-review.md`, SHA-256
`89319d13510632e58fc13a4477f5bf01b1ea4fc00888d5921d19206c14e36cc1`.
The observer's terminal checkpoint is 360.585262 seconds after original start;
both native Codex reviews finish before the original work cutoff. Review attributes
pidfd observations to the original supervisors, not reopened saved PIDs, and does
not establish every watchdog-loss/early-error path or a persistent enforced hold.
Validation: native script syntax/Ruff, 46 existing plan checks and whitespace pass;
no production code or persistent tests changed. This is not a durable production
dispatcher, named lift/naming/cleanup success path or live BOF3 semantic acceptance.
S3.2 and S4 remain unfinished; frozen production pins are unchanged.

### Native review transport rollout

The reviewer transport (`bin/agent-run`, `context.dispatch/codex/journal`
and the `bof3-re` dispatch helper) is scoped to explicit read-only review. Its initial
code review, private-artifact/hold/event probes and 320 distinct existing checks
passed before the native attempt. Shared `run_bounded` now supports selector-driven
stdin and retained-output/spawn callbacks without blocking on an initial prompt
write. Callback `ProcessCleanupError` and `TimeoutExpired` also require cleanup,
retain exception identity when cleanup succeeds, and yield to cleanup uncertainty.
Bohr accepted that correction; four real-process callback probes retain terminal
original supervisor pidfds in `/tmp/bof3-callback-error-probe.log`.
After that correction, 174 existing process/type/macro/revalidation checks and
46 plan checks pass; no persistent regression tests were added.

The separately approved read-only native invocation stopped **before any Codex
process or dispatch debit**: the actual repository is `fuseblk`, and its empty
writer lock reports `0770` after `fchmod(0600)`. The then-strict mode guard rejected
it. This is historical failure evidence, not a requirement to enforce Unix modes
on NTFS; that attempt established no native AI/authenticated review.
The failed attempt is retained unchanged under `out/reviews/native-codex-review-v1`:
original 900-second work / ten-second cleanup allowance, one dispatch, zero repairs,
budget pin `v1:55146dfe5be00b15b1da88db0c4bc1142704ee1b9d95ab934298bf1827a4d53b`,
initial high-water pin
`v1:98046a50a1993eeee6c92a20b6b01f22aab84a8b5678acd1910cc723d1d58602`.
`outer-result.json` SHA-256:
`a37924dd1b91ebe2820b1f1eaf74b4652d6d4403b5baaff9b1730ab641d4397f`.
No model was retried, allowance reset, source restored or production queue repinned.

The user's subsequent NTFS decision supersedes the proposed POSIX-private state
store and dual-lock workaround: remove fixed `0600`/`0700` enforcement from leases,
dispatch streams, staging and recovery records; delete the unused storage modules.
Keep ownership, regular-file/single-link, root/path/inode, exclusive creation,
locking, evidence pins and rollback checks. Preserve captured source modes, native
read-only capability policy and global credential hygiene. Filesystem-native modes
do not promise confidentiality. Never unlink locks, move recovery across filesystems,
reset an allowance or silently retry. Continue transport validation without claiming
production acceptance; S3.2/S4 and live report-provenance repair remain open.

Post-change NTFS characterization passes `0770` lease/journal creation, native
flock contention, hard-link rejection, persistent-inode reacquisition and exclusive
journal publication (`/tmp/bof3-ntfs-lease-journal-probe.py`). A separate synthetic
recovery attempt reaches POST preparation but stops at its single-link guard:
unsupported cross-name no-replace rename falls back to a retained hard link
(`nlink=2`). Same-path `EEXIST` preflight is insufficient support evidence. Retain
`out/reviews/bof3-codex-recovery-o62e76l2` unchanged; no source was installed or
restored. This independent filesystem capability gap remains open: do not remove
link/identity safeguards or claim that mode-policy removal completes NTFS recovery.

The separately approved post-fix native check completed in 59.493424 seconds:
`out/reviews/native-codex-review-ntfs-0f1e317f573ee50d`, original 600-second work /
ten-second cleanup allowance, one dispatch, zero repairs. Budget pin:
`v1:6fdd9c8320ccf6cf608a2a002103f9babf48961acfd64aee91408a0fb027e418`;
initial pin `v1:d4e4d4ce5c90ad4136455a9002aeb03beccd7180886695f2904347b29553d34b`;
sequence-one pin `v1:04580416012c0460a8d0dc4d9bddaa7ee5bce6bb24741aa8f1aaa959f13f51ec`;
receipt pin `v1:de9b4eb23a31dac89efbf47f611e0966ad63c60af65db9f96ed533eab467081d`.
Dispatch evidence is under `out/reviews/dispatch/<budget hash>/1`; native thread:
`01a0881e-717f-72f0-a562-6bcad0219767`. The original failed allowance is unchanged.

Independent reviewer `01a084ec-94e4-7473-a0d0-f0c691222ff7` accepted the narrow
permission-policy and read-only transport scope: eight artifact hashes, queue and
consumption bindings, terminal outer exit zero, completed native turn, no failure
marker and all 383 then-adopted baseline entries verified. Seventeen events include
six completed read-only diagnostic commands. The proposal reports no limited-scope
findings, but unavailable byte/build gates and unverified assembly freshness block
source acceptance. Global-configuration preservation is parent-attested, not proven
by before/after hashes in this bundle. This does not complete NTFS recovery, the
named-role scheduler, S3.2/S4 or any live BOF3 acceptance gate.

Validation: 430 existing checks pass, including 110 transaction-file and application
review/transition checks; the Git-object file-mode check initially
failed only because `.git` was sandbox-read-only, then passed with approval using
its separate index. Full Python Ruff, changed-module formatting, whitespace,
disposable event/hold probes and frozen production index/report hashes pass.
No persistent tests were added. Subsequent documentation/commit changes make the
transport baseline historical; do not reuse it as a fresh source-acceptance pin.

Documentation operations now share `bof3-docs` and `harness.docs`, replacing the
repair-only skill and separate context-builder profile. Context/search/aggregate
are read-only; edit/repair/one-document compact prepare full pinned inputs for
reviewed agent edits, never automatic rewrites. Legacy docs cleanup keeps its
narrow scope. Independent review closed runtime-path exclusions; 38 disposable
native probes preserve the protected index/report/sources. This documentation
feature does not complete S3.2 or alter its frozen queues and authority gates.

The symbol-naming routing prerequisite now follows the same concern ownership:
`harness.naming.opportunities` owns target-local read-only leads and legacy
inventory projection. `bin/naming-audit opportunities` / `describe-opportunity`
bind exact spelling and mapped address to one map-byte fingerprint; inventory
no longer scans every target or authored-source filename. One explicit-only
`bof3-naming` skill replaces the separate evidence and identity skills, retaining
opportunity, audit and transaction authority as distinct modes with selective
references. Existing canonical inputs are unchanged; frozen old skill bindings
reject rather than silently migrate. Type representation remains in `types`.
Native read-only scouting enumerated 249 battle15 raw leads, not accepted naming
decisions or additions to the frozen queue; exact ID/pin description and all six
legacy naming prefills succeeded. Disposable suffix/address, map-drift and stale
binding probes passed, preserving the raw Git index, frozen reverse index/report
and unrelated dirty sources. The merged audit prefill stays under 16 KiB.
Existing checks: 210 passed plus the separate file-mode case; no new tests.
Herschel independently accepted the feature and consolidated layout after 58
existing checks and native/rejection probes; Bohr found no lost audit requirements.
This does not close target naming evidence or S3.2's bounded native sequence;
the macro/type stale-index handoff below remains unchanged.

S3.2 routing prerequisite: native cleanup prefills rejected both macro and type
opportunity requests because the structured router and cleaner admitted only
identity, naming and documentation skills. Explicit `macro-opportunity TARGET ID`
and `type-opportunity TARGET ID` now select their existing concern skills and
retain one opaque owner ID. They query or mutate nothing, infer no ranking inputs,
and grant no transaction authority. Existing type-spelling identity requests stay
unchanged. Canonical re-derivation checks internal coherence, not original caller
authenticity; the parent retains bindings and owners validate candidate freshness,
membership and separate approval. Policy and invocation details live in
[harness routing](../agents/harness.md#policy-versus-mechanism) and
[usage](../agents/tool-usage.md#cleanup-opportunity-routing), with macro specifics in macros.md.

Actual prefills for frozen `exact_group:8e1ad03b4ba92303` and the battle15
`@1F800044:storage` lead now exit zero, each loading only its selected body and
preserving the supplied ID. Disposable probes reject malformed tokens and forged
inconsistent fields without body reads, preserve the old identity route, and
verify unchanged raw index/frozen report/live reverse-index bytes. Existing cleanup,
context, wrapper, skill-status and harness checks pass 100 cases plus the separately
escalated file-mode case. Both changed skills validate; changed Markdown is directly
compacted. No persistent tests or dependencies were added.

The subsequent actual macro/type owner queries both exit 2 for stale battle15
type inputs in the reverse index. Context transport is not candidate acceptance:
no index rebuild, frozen-proof update, source edit, ranking or application follows.
Reviewer `01a084ec-94e4-7473-a0d0-f0c691222ff7` independently accepts this routing
prerequisite, including 40 disposable routing/transport/coherence probes and the
code/docs boundaries. Whole S3.2, current production freshness and the bounded
native sequence remain unfinished. An additional 46 existing plan checks pass;
125 local links/anchors, Ruff and formatting pass. This is neither a successful
live opportunity resolution nor a completed loop.

`select TARGET@ADDRESS → bof3-lifter → bof3-reviewer → bof3-namer →
bof3-reviewer → bof3-cleaner → bof3-reviewer → record result → next function`

| Project role | Responsibility | Boundary |
| --- | --- | --- |
| bof3-lifter | Reconstruct readable C and obtain an evidence-backed match | One target-qualified function; no speculative identity promotion |
| bof3-namer | Gather/corroborate naming evidence and propose identities | Evidence preparation does not authorize application |
| bof3-cleaner | Apply reviewed canonical identity transactions; byte-safe cleanup | Apply once, validate and roll back on failure; semantic changes return to lifting |
| bof3-reviewer | Inspect actual changes and independently run existing checks | Inspection/check tools, no source edits; reused for final acceptance |

Recover the BOF3 reviewer's actual check-running capability; do not inherit the
built-in reviewer's tool limitations. Document allowed validation side effects.
Review fresh evidence, not a writer's PASS assertion. Naming or cleanup may be a
justified no-op; never manufacture changes to satisfy a stage.

Retain only these four core profiles unless a retained definition proves another
role necessary. Remove coordinator/finalizer wrappers that merely relay calls;
move any unique target freshness or acceptance obligation into its owning skill
or final review stage. The active session may parallelize independent reads/reviews;
one shared-checkout writer and explicit selector/consumer ownership remain required.

Agent files should contain role, tools/model, input, skill route, output and stop
conditions. Skills own domain procedures. Put shared rules in one owning place,
not every prompt. Use the instructional `bof3-lift-loop` mission protocol with a
finite queue and bounded repair rounds. No harness model launcher, generic framework
or separate scheduler. Repairable findings return to the owning stage; failed/unknown
execution stops for recovery, never silently replays mutations. Record blocked
items honestly and proceed to independent items only within the approved scope.

## Autonomy and cleanup acceptance

- Audit live entrypoints, role names, removed-script links and stale plan anchors
  (including `docs/INDEX.md`'s missing `#N1`). Repair only references owned by
  this implementation; historical records are not active instructions.
- Preserve the starting dirty work. Use one writer in this checkout; require a
  clean source checkout before any managed worktree allocation. Never reset,
  stage or commit unrelated changes to satisfy a launch prerequisite.
- Select a finite queue from live target-qualified evidence; exclude accepted,
  duplicate and already-owned work. Record selector, baseline and stage outcome
  in native run artifacts, not a second campaign database.
- Require explicit structured accepted/no-op/repair/blocked outcomes bound to
  the selector and actual changed content. Missing, malformed, stale or failed
  results stop advancement; writer self-approval cannot satisfy review.
- Reviewer acceptance authorizes only the evidenced identity transaction within
  the approved scope, not speculative names or new privileges. No-op naming and
  cleanup require reasons. Limit repair rounds and total runtime/spawns.
- Resume only after inspecting native run state and current content; never
  blindly repeat identity application. Infrastructure failure stops the loop;
  domain-blocked items may yield to independent approved items. No silent CLI
  fallback or permission widening. Native recovery must be demonstrated, not
  inferred from a successful happy path.
- Compact changed project agent/skill Markdown without weakening its contracts;
  remove duplicate instructions, not unique evidence gates. Delete temporary
  outputs only when proven owned by this implementation and no longer needed;
  preserve earlier recovery copies, session evidence and unrelated `tmp/` work.
- Handoff one documented launch/resume path with finite defaults, supported
  inputs, stop conditions and limitations. Run existing relevant checks and
  independent review; report unexercised paths as NOT RUN. Full autonomy means
  unattended work inside approved boundaries, not unbounded execution.

Recovery exposed two required scope corrections: `audit-target TARGET` is a
target-wide audit, not a function-only naming request; reuse validated target
reports rather than auditing an entire target afresh for every function. The
former reviewer's lesson-edit exception moves to a scoped writer transaction;
review itself must remain no-edit. The previously deleted compaction skill is
absent despite its stale session advertisement; do not silently restore retired
machinery or claim that unavailable skill ran. The owner explicitly approved
replacing that obsolete requirement with direct Markdown compaction and existing
checks; `AGENTS.md` now records the replacement. This was the S1.1 unblock; its later acceptance is recorded below. Current
AGENTS.md and the plans skill were read: direct compaction is the policy, the
retired skill is physically absent, and no separate compaction tool was run.

These requirements extend S1–S4 acceptance; implementation and demonstration
must satisfy them before the corresponding phase is marked done.

## 1. [S1] (done) Recover and streamline project BOF3 agents
- Owner: parent
- Depends: none
- Blocker: none
- Evidence: latest BOF3 role names retained in native session history; older BOF3 definitions available in Git HEAD.
- Acceptance: latest source bodies identified, four minimal project-owned profiles proposed, no imported generic or built-in profiles.

1. [S1.1] (done) Compare retained definitions and preserve only unique domain responsibilities.
- Owner: parent
- Depends: none
- Blocker: none
- Evidence: recovered complete read results from native sessions: 2026-09-07 ef728133/run-0 (lifter/reviewer/coordinator/finalizer), 2026-09-06 0397af18/run-0 (namer/cleaner); later namer read eac7279d/run-0 matches SHA-256 21e12fd3d88a593cbdb9f1b893ac8ab388cdacb1c2736397b58243760089f4c5; Git HEAD bof3-review.md inspected as older baseline.
- Acceptance: restore and simplify bof3-lifter, bof3-namer, bof3-cleaner and bof3-reviewer under .pi/agents after approval; retain reviewer check execution without source-edit authority; remove redundant coordinator/finalizer profiles and stale orchestration requirements without losing domain checks.

## 2. [S2] (done) Streamline the project skills around those roles
- Owner: parent with bof3-reviewer
- Depends: S1
- Blocker: none
- Evidence: six project skills remain; their actual instructions require a focused overlap/reference audit.
- Acceptance: concise entrypoints, explicit routes, one owner per rule, valid links and existing relevant checks; no weakening byte or naming evidence.

1. [S2.1] (done) Align lift, naming-evidence and identity-maintenance contracts.
- Owner: parent
- Depends: none
- Blocker: none
- Evidence: .codex/skills/bof3-re and the unified .codex/skills/bof3-naming opportunity/audit/transaction contracts.
- Acceptance: consistent target-qualified inputs/results; separate naming proposal, approval and application; remove duplicated prompt/orchestration text; retain byte-match and rollback requirements.

2. [S2.2] (done) Reconcile supporting skills and project instructions.
- Owner: parent
- Depends: S2.1
- Blocker: none
- Evidence: .codex/skills/psx-rizin, unified .codex/skills/bof3-docs, .codex/skills/plans, AGENTS.md and docs/INDEX.md.
- Acceptance: preserve explicit psx-rizin opt-in and documentation-only scope; eliminate stale imported-agent/default-loop references; no home/package skill edits or global skill rewrite.

## 3. [S3] (in-progress) Prove machine integration, native recovery and one bounded loop
- Owner: parent
- Depends: S2
- Blocker: none
- Evidence: accepted owner/handler/accounting slices and whole S3.4 below; whole S3.5 accepted by 01a084ec-94e4-7473-a0d0-f0c691222ff7 after complete matrix audit and final native cell review. Bounded loop execution remains NOT RUN.
- Acceptance: independently accepted S3.4 machine integration, S3.5 actual native recovery and S3.2 bounded loop; project bof3-* profiles, finite budgets and no custom infrastructure. Fail-closed blocked handling proves machinery, not S4 live closure; separation contract below governs.

1. [S3.1] (done) Exercise one direct target-qualified handoff before automation.
- Owner: parent and BOF3 roles
- Depends: none
- Blocker: none
- Evidence: workflow 3a519b7f lifted emi/battle/battle/15@0x800A3638; reviewer 5e97e6aa accepted 20/20 instructions and all 80 bytes with no matching aids. Parent refreshed battle15 snapshot and rebuilt the index. Naming workflow f58d0343 and independent reviewer 459539e5 reproduced inventory failure (249 report rows versus 250 live rows), canonical report SHA-256 39fbb754e5b8a0f3b7e30a92bf611cc7689768d9be60c9e54e1b5f5a8e6d2856. Audit remains blocked, not no-op; cleanup unproven. Existing initialize(root,target) can generate current blocked rows in memory, but neither CLI initializer merges preserved rows.
- Acceptance: restored role discovery/tool availability verified; lift/naming/cleanup handoffs need no invented permissions or duplicate skill instructions; blockers attributed to actual domain evidence.

2. [S3.3] (done) Freeze indexed pilot coverage and owner routing.
- Owner: worker with independent reviewer
- Depends: S3.1
- Blocker: none
- Evidence: independent S3.3 review-0.md accepted the declared-global repair, all five frozen payloads and fingerprints, exact macro membership and 294 checks; checkpoint below. Historical selected FUNCTION verify is not rebound.
- Acceptance: first implementation slice: characterize existing index/query behavior in test_reverse_index.py, test_type_index.py and test_macro_opportunities.py; minimally repair owning index/query omissions only if reproduced. Freeze the five pilot entries below with current IDs, fingerprints, owners and overlap exclusions in native run inputs; all five classes route or explicitly block, never disappear. Reviewer independently runs focused tests and inspects inventory equality; no target/report writes.

3. [S3.4] (done) Accept concern-owned machine integration and frozen accounting.
- Owner: worker with independent reviewer; parent owns scope acceptance
- Depends: S3.3
- Blocker: none
- Evidence: whole S3.4 independently accepted by 2dc9dec6 (715 passed, 2 skipped), attributed in the superseding acceptance record below. S3.4 checkpoint below records independently accepted duplicate-key rejection and the independently accepted native-gate prerequisite repair and accepted execution-context/publication prerequisite and accepted parent-envelope slice; accepted reviewed shared-input migration; accepted no-drift check-only revalidation including B1; accepted fresh no-drift parent acceptance; accepted internal historical validation prerequisite; accepted same-target exact private transition; accepted distinct-target common-PRE integration; accepted shared PRE/POST integration; accepted current-byte owner CLI transport (b295c173); accepted bounded terminal CLI/API (e2920f90, 605 passed/two skipped); accepted all-handler fixture sequence (11fcb490) and bounded frozen-five accounting (efb1b118), not native execution or live closure; frozen local-data control BLOCKED (d740aabe), not accepted exhaustion/no-op. Postapply was FUNCTION-only at that checkpoint; reviewed bounded DATA support and its accepted production transaction are recorded above, without rebinding old proofs. Existing type/macro digest pins do not establish independent final review.
- Acceptance: independent reviewer decides whole machine readiness from accepted owner, all-handler and frozen-accounting coverage: proposal versus approval, dirty-baseline adoption, scope/storage/layout preservation, gate rejection, digest-bound final verification and all-five accounting. Supported fixture positives must be labeled synthetic; unsupported/live-blocked routes fail closed. Historical Pi recovery belongs to S3.5; live evidence/application/full-target closure moves to S4.3/S4.4 below. Current selected FUNCTION/DATA transactions retain distinct bounded admission; never route types/macros through naming postapply or invent a framework; no writer self-approval.

4. [S3.5] (done) Prove failure, rollback and resume before unattended execution.
- Owner: worker with independent reviewer and parent
- Depends: S3.4
- Blocker: none
- Evidence: whole S3.5 accepted by 01a084ec-94e4-7473-a0d0-f0c691222ff7 after complete failure-matrix audit and 01a084ec-947a-7a72-ad93-604fa3c331f9's final cleanup-unconfirmed cell review; parent accepts the superseding Whole S3.5 acceptance record above. Original deadline, no duplicate apply, owned recovery, concurrent preservation and fresh parent-controlled refusal are evidenced; no automatic dispatch hold or Codex session revival is claimed.
- Acceptance: run the failure matrix below against owner transactions and the small native workflow; inspect actual run state/tool outputs, not prompt text or exit zero alone. Reviewer verifies rollback bytes/modes/absence, unchanged unrelated dirty work/index and no duplicate apply; parent owns freshness recovery and attestation. Report unavailable native capabilities as blockers, not mocked acceptance.

5. [S3.2] (in-progress) Demonstrate the skill-only sequence with bounded repair and recovery.
- Owner: parent
- Depends: S3.6
- Blocker: none
- Evidence: S3.1 handoffs, efb1b118 accounting and whole S3.5 accepted; the skill-only continuation checkpoint records two real parallel read-only domain scouts and parent skip/blocked controls after 53ba86e8. The full bounded sequence remains unfinished. Full-target naming blocks production, not controlled fixtures or read-only blocked accounting. Parent freshness/attestation checkpoints remain explicit and cannot be delegated implicitly.
- Acceptance: active-session mission delegation, existing applicable checks and independently reviewed finite skill execution with fixed queue/budget. Parallel read/review results must retain actual handles and disjoint scope; shared-checkout writes serialize. No Codex CLI/model subprocess or Pi fallback. May account blocked entries, never advance failed/unknown mutation or claim live closure from fixtures. Production campaign advancement requires unfiltered full-target naming complete:true and separate identity approval; individually approved selected transactions retain their own gates without satisfying that closure. Final review binds actual selector/change. Accepted work skips; uncertain apply stops for recovery; no budget reset on resume.

6. [S3.6] (done) Retire harness Codex launchers and establish the skill operator.
- Owner: parent with independent active-session reviewer
- Depends: S3.5
- Blocker: none
- Evidence: parent accepts this removal/skill scope after independent active-session reviewers 01a08963-3714-7410-82ec-ff6a93c7a4c5 (code) and 01a08963-3763-7133-99dc-35bd6c959bdd (protocol) found no blockers; two documentation corrections are applied. Native CLI transports are removed; deterministic tools and the discoverable bof3-lift-loop remain. Existing focused checks: 151 passed, one pre-existing fixed-mode assertion deselected; Ruff/import/parser/skill checks pass, 119 references resolve without broken/unchecked entries. Native no-edit diagnosis skill-local-check-8b67f3d46ee6 is byte-exact at 100%, source_accepted:false and model_dispatched:false, digest v1:08dd97c2ce60130fd13edda6dcdb7aacc2cd0c8baa56d457ea22c97b36e230a1. Its original 180-second window expired before retained audit; a disposable probe confirms rejection before output, without reset. Frozen evidence and four unrelated dirty file identities are preserved. Broader suite/source validation is pending; symbols check flags existing emi/battle/battle/03:D_801EB4F0 raw-data naming debt from the preceding source feature. Parallel read-only reviews prove capability, not full S3.2 execution, native audit acceptance or whole-game closure.
- Acceptance: no harness Codex discovery/configuration/process launch or detached model dispatch; deterministic diagnosis/audit/transaction/recovery tools remain usable. One discoverable bof3-lift-loop skill owns mission inputs/results, parallel-read/serialized-write policy, original budgets, recovery and domain handoffs. Update live instructions/paths, run existing checks and demonstrate available active-session capability without inventing delegation or marking S3.2 complete.

7. [S3.7] (done) Preserve accepted naming proof across active report generations.
- Owner: parent with naming owner and independent active-session reviewer
- Depends: S3.6
- Blocker: none
- Evidence: production naming checkpoint above; final runtime pins independently accepted by Hubble after one P1/P2 repair recheck. Fresh real gates, independent DATA re-review, ingestion and public verification accepted; actual DATA checkpoint published at 20:23:04, session 13398 exit zero, exact 190-row successor/resolver checks pass. Separate two-generation FUNCTION fixture native execution remains injected; no second production name accepted.
- Acceptance: exact reviewed preview/CAS successor activated after fresh public verification; immutable predecessor and complete embedded proof retained, active-generation resolution and subsequent proposal demonstrated without historical replay overwrite. Preserve unrelated summary drift, original budgets and all incomplete naming obligations; tooling or selected-row success is not full-target closure.

## 4. [S4] (open) Demonstrate useful autonomy and close accepted scope
- Owner: parent with bof3-reviewer
- Depends: S3
- Blocker: none
- Evidence: none
- Acceptance: real target-qualified results, existing owning checks and independent review; no claim of whole-game coverage from samples.

1. [S4.1] (open) Complete one function through lift, naming and cleanup.
- Owner: bof3-lifter, bof3-namer, bof3-cleaner and bof3-reviewer
- Depends: none
- Blocker: none
- Evidence: historical passesBytePairGate selected-row success accepted by 94f37f72 in workflow 93ed23ed; current tooling closure is stale per efb1b118. Remains open under S3 dependency; fresh gates/review without rename replay and automated final cleanup/review are required.
- Acceptance: instruction/byte checks as applicable; validated identity application or justified no-op; cleanup preserves final bytes; final reviewer reports actual residual risks.

2. [S4.2] (open) Run a small approved queue and measure intervention cost.
- Owner: parent
- Depends: S4.1, S4.3, S4.4, C1.5
- Blocker: none
- Evidence: finite pilot and closure matrix below; no queue execution accepted yet
- Acceptance: finite queue/budget, native stop/resume without duplicate writes, honest completed/blocked/deferred outcomes; assess elapsed effort and parent interventions before scaling.

3. [S4.3] (blocked) Close the frozen five live domain obligations.
- Owner: parent and BOF3 evidence/application roles with independent reviewer
- Depends: none
- Blocker: all five frozen entries remain blocked; linked storage/layout same-envelope closure is incompatible with current concern rules; exact private/shared domain evidence is absent.
- Evidence: efb1b118 accepted accounting: accepted 0, noop 0, blocked 5, historical_skip 1 (overlapping), linked pair blocked; campaign_complete false and production_complete false.
- Acceptance: discharge the S3.4 live obligations explicitly mapped below through current concern-owned evidence, separate approval/application, native gates and independent final verification. No speculative names, receipt rebinding or fixture substitution. Resolve linked storage/layout schema design only with explicit approval and independent review; preserve concern isolation, serialization and both IDs. Every frozen entry must be accepted or independently justified no-op/exhaustion for finite closure; blocked/deferred remains unfinished.

4. [S4.4] (blocked) Restore and satisfy full-target naming production closure.
- Owner: parent with bof3-namer and bof3-reviewer
- Depends: none
- Blocker: unfiltered report rejects func_800A3638 binding_locations: missing=[] invented=['config/targets/emi/battle/battle/15/symbols.txt']; full-target complete:true is absent.
- Evidence: efb1b118 actual unfiltered verifier failure; selected FUNCTION history and frozen accounting do not override it.
- Acceptance: separately authorize evidence-preserving report/provenance recovery if needed; obtain successful unfiltered full-target complete:true and separate identity approval before production campaign advancement. Individually approved selected transactions do not complete this obligation. Never narrow the report gate, rewrite historical receipts or treat selected acceptance as target completion.

## 5. [C1] (in-progress) Support per-function metadata and reviewed source consolidation
- Owner: parent with domain/build/cleanup owners and independent reviewer
- Depends: S3.6
- Blocker: none
- Evidence: latest user request; panel audit exposes 14 coherent template uses across three independently loaded targets; migration seams recorded above.
- Acceptance: complete per-function identity/native/consumer support, reviewed bounded combiner methods and human-value decisions, recoverable transactions and independently verified target-local pilot; no pipeline or source acceptance inferred from inspection.

1. [C1.1] (done) Establish attached metadata records and read-only inspection.
- Owner: domain and combiner owners
- Depends: none
- Blocker: none
- Evidence: domain/functions.py and combiner/inspection.py expose address-selected records and lexical ranges; bounded independent release review 01a089de-ca01-7261-9efd-7a82f332d533 accepts this slice only, retained at out/reviews/combiner-foundation-c1-20260910/review.md. All 147 relevant checks pass (one known Git-writing mode check deselected); wrapper checks 48 pass with pre-existing HEAD inventory omission bin/agent-run left reported. Thirty disposable negative cases, mixed records and all 14 panel records pass; Ruff and 119 documentation references pass. Earlier reviewer findings fixed; actual handles closed, no native or source acceptance claimed.
- Acceptance: ordinary C89 bodies and body-emitting invocation records isolate metadata/progress; ambiguous, duplicate, nested or unsupported inputs reject; existing checks, disposable characterizations and independent review. Inspection never proves macro expansion or enables production combination.

2. [C1.2] (in-progress) Migrate source claims, layout, index and all metadata consumers.
- Owner: domain, analysis, decomp, naming, types and macros owners
- Depends: C1.1
- Blocker: none
- Evidence: source/claim enumeration, complete boundary address sets, scoped lifecycle and direct macro occurrences implemented; duplicate owners reject. Native resolve/compare/status, unscoped metadata repair, filename-changing naming facts and macro/type PRE/proposed C guards remain restrictive. Layout, function-qualified cache, naming editing and type/macro consumer coverage remain unmigrated. The exposed battle15 duplicate was resolved by reviewed original/native source cleanup, not by weakening indexing; the v15 index now publishes and validates. Its repair evidence and unchanged setter metadata obligation are recorded below.
- Acceptance: exact target/address-to-record relation, many boundaries per source with complete coverage and duplicate rejection; metadata edits touch only selected records; status cache keys and lexical use links remain function-qualified, include-level consumers remain conservative; no first-record fallback or unrelated lifecycle overwrite.

C1.2 source/index slice has a scoped clean final code review from
`01a089f8-f794-7a63-802d-aaaf58d25f9d`; both earlier naming guard findings are fixed.
Evidence is `out/reviews/c1-source-ownership-IPs4j27l/checkpoint.md`. Initial index
rebuild exited 2 for the existing duplicate and preserved v14 bytes unchanged;
repository metadata preflight failed for the same collision. Parent check groups
364/241/104/56 pass with overlap and the known Git-writing mode check omitted;
disposable grouped-index/guard probes and 69 references pass. Unchanged game00
`0x801996FC` still passes 16/16 instructions and 64-byte native smoke under its
original 180-second cap. These are not production consolidation acceptance.
C1.3 may develop from C1.1 in parallel with remaining consumers, avoiding a circular
dependency on the then-unpublishable index; C1.4 still requires both complete.

The publication blocker is now resolved by a separate, bounded source cleanup:
`out/reviews/entry-ownership-9bvgvfnl/terminal.json`. Independent original/native
evidence justified removing the unused incorrect alias only; first C body and
canonical setter/map/layout/claims remain unchanged. Four native checks pass
(5/5 instructions, 20 bytes; 12/12, 48 bytes), symbol validation passes, and the
v15 index published and validates. All 23 targets scan 920 unique function claims;
repository metadata/index preflight passes. Prereviewer
`01a08a20-d01c-77a1-b0f9-10633da9a245` and final reviewer
`01a08a29-c15c-7c32-8e1f-eb74ef6007fb` completed and closed within their original
bounds. One source edit and one index publication; no layout promotion or reset
of earlier stage budgets. The setter remains natively exact but metadata-invalid
because its pre-existing noncanonical residual was deliberately preserved; queue
that separate metadata audit. Full C1.2 consumer migration remains unfinished.

3. [C1.3] (in-progress) Prove multi-function compiler, build and native comparison correctness.
- Owner: build, toolchain and matching owners
- Depends: C1.1
- Blocker: none
- Evidence: bin/cc now partitions one complete maspsx translation by attached records and reviewed ownership; scratch two-panel GCC/native proof passes. Path-keyed profile preservation, full producer freshness and native match/_asm_link.py text-prefix extraction still need migration; catalog/default GCC selection remains authoritative.
- Acceptance: independently select whole-object placement or deterministic per-function compilation projections; preserve every effective compiler/flag profile or reject, no duplicated shared state or unbound generated inputs. Prove per-symbol original placement/relocation/size/instruction/byte checks including noncontiguous functions, calls and data sections; all-member native positive and negative probes. Scoped reviewed execution only; no sandbox changes or installs.

Read-only scout `01a08a05-dac7-77c0-9588-81539af9a405` completed and closed within
180 seconds, favoring post-maspsx section placement for compatible profiles over
duplicating C translation units. The inactive `build/sections.py` and
`match/placement.py` kernel preserves one C unit, partitions complete assembler
envelopes and inspects actual linked symbols/ranges. Disposable native characterization
covers five noncontiguous functions, calls/tails/branches/function pointers, shared
initialized data and one BSS allocation; 13 negative cases pass. Existing 71 checks
pass with the known Git-writing mode check omitted. A fresh compiler characterization
retained at `out/reviews/c1-native-sections-5_4evx1e/terminal.md` correctly rejected
the old alias's 56 bytes against its reviewed 48-byte range; no grouped acceptance.
Kernel reviewer `01a08a1c-afac-7202-bc74-6d8b3a3653e4` completed within 300 seconds
and withheld acceptance. All four findings are now fixed: reachable MIPS-I flow
tracks ordinary entries separately from delay slots; private snapshot linking
validates before confined no-replace publication; unsupported text subsections/
attributes reject; supervised native calls share the caller's original deadline
and output bound. `match.flow` and `match.execution` own those checks. Failed work
retains scratch, especially with unconfirmed descendant cleanup; script/ELF
publication is not an atomic pair and never replaces concurrent artifacts.
Follow-up reviewer `01a08a3f-e26b-70f3-8c59-6b20675748ac` found no actionable
inactive-kernel findings within the original 240 seconds and was closed. Parent
evidence `out/reviews/c1-native-safety-0sdngfjo/result.json` records 18 safety
rejects, including real MIPS branch-to-delay-slot fallthrough, both publication
races, timeout cleanup and retained unconfirmed scratch. The five-function native
probe and 13 negatives pass again; 34 existing checks pass with the known
Git-writing mode check omitted. No GCC rerun or production consolidation occurred.
At that checkpoint the compiler, matcher and grouped-source guards were unchanged. Compatible
profile/compiler integration, shared-data proof, full member coverage, instruction
selection, cache identities and producer/placement freshness remain required;
kernel acceptance does not complete C1.3 or authorize C1.4/C1.5.

The next producer slice wires `build.translation` into `bin/cc` after its single
whole-unit maspsx pass. Ungrouped input is unchanged; grouped records require one
explicit lift owner and complete map/Splat-resolved assembler membership. CMake
now invalidates objects for adapter/partitioner/ownership inputs. Reviewer
`01a08a49-8d22-7ea1-a90b-ab21692ba6fe` found a separate-comment compiler-annotation
bypass; scanning grouped comment lexemes fixes it. Follow-up
`01a08a4d-05b6-7f83-90c4-760a09f42859` accepts the repair after 44 in-memory checks
within 90 seconds; both reviewers are closed within their original bounds.
Parent evidence `out/reviews/c1-grouped-producer-60skk_tr/` preserves one configured
GCC compilation of a scratch game00 panel pair: each matches 16/16 instructions
and all 64 original bytes at its noncontiguous address. Post-repair replay uses
retained compiler assembly under the original 600-second cutoff, produces an
identical object and keeps both matches; no second GCC run or production source
edit. Scratch CMake generation verifies new dependencies. Existing groups of
4 wrapper, 34 build/compiler/DRY and 99 source/preflight/plan checks pass; the
Git-writing mode check and unrelated live asm-diff wrapper check are omitted.

Scout `01a08a46-8908-72e3-8508-fc2505601ee7` confirmed there are no implemented
per-function compiler tags: current authority is project defaults plus path-keyed
object flags/compiler IDs. Preserve every member's PRE path, ordered effective
arguments, compiler identity and configuration pins before consolidation; reject
incompatibility or missing history, then verify the destination reproduces that
profile. Unsupported annotations reject, not silently fall back. This validator
and profile migration remain required. Full include/toolchain freshness, late
private-header dependency registration, function-qualified caches and production
grouped matching/consumer gates also remain open. The broad new CMake ownership
dependencies favor correctness; later narrow them without losing invalidation.
This producer slice does not complete C1.3 or authorize C1.4/C1.5.

4. [C1.4] (open) Add combiner discovery, ranked assessment and reviewed transactions.
- Owner: combiner owner with independent human-value reviewer
- Depends: C1.2, C1.3
- Blocker: none
- Evidence: planned macro-family, functional-subcategory, shared-state/type and helper/call-cluster methods in combiner guide; no ranker or mutation CLI implemented yet.
- Acceptance: one noun-module owner and concern skill for opportunities/ranking/audit/transactions/editing/CLI; explicit cleanup combiner node and thin invocation script; finite pinned membership/method/limits, overlap handling and configurable top N; would-a-human-do-this decisions before mutation. PRE/POST claims/maps/compiler/consumer closure, all-member gates, distinct final acceptance and owned deletion/move cancellation rollback preserve unrelated work and original budgets.

5. [C1.5] (open) Integrate a bounded panel consolidation pilot and update source contracts.
- Owner: parent with bof3-re and combiner reviewers
- Depends: C1.4
- Blocker: none
- Evidence: existing panel wrappers are leads, not accepted consolidation or an expanded frozen macro queue.
- Acceptance: independently justify target-local advance-panel files or retain explicit rejection/defer; prove every original function unchanged before/after, safe rollback and no duplicate application; refresh safe-checkpoint index/status, docs/skills/source references and both indexes. Replace temporary single-function restrictions only after supporting owners pass; integrate result accounting into S3.2/S4 without marking whole-game completion.

## Milestone separation and superseding obligation map

The user explicitly authorized separating machine integration/recovery/loop from
live campaign closure. This section supersedes earlier **current-readiness** claims
that live S3.4 closure/full-target naming must precede any S3.5 rehearsal or S3.2
machine loop; historical findings and their evidence are retained, not rewritten.
No existing ID or done status changes. S3.4 remains in-progress for an independent
whole-machine verdict; this writer does not infer readiness from slice acceptance.
S3.5 still depends on accepted S3.4, S3.2 on accepted S3.5, and S4 on accepted S3.
No unit edge or implicit launch permission bypasses those dependencies.

| Old obligation / ID | Retained or new owner ID | Acceptance preserved / attribution |
| --- | --- | --- |
| S3.4 owner gates, all-handler integration, five-entry accounting | S3.4 | 11fcb490 accepts handlers only, not subprocess/Pi or every positive replay placement; efb1b118 accepts the read-only frozen consumer only. Reviewer must decide combined machine readiness; fail-closed handling is admissible, not live success. |
| S3.4 current FUNCTION closure | S4.3; S4.1 retains end-to-end function demonstration | Fresh owning gates/review without rename replay or rebinding; historical FUNCTION skip is informational and overlaps blocked, not current acceptance. |
| S3.4 data terminal acceptance | S4.3 | D_80096994 remains blocked: capability integrity/historical exhausted row is not accepted semantic exhaustion/no-op. Retain name, report and unresolved findings; no peripheral research merely to account blocked. |
| S3.4 fixed-RAM/storage and aggregate evidence/application | S4.3 | Owners/access/base/extent/role and layout/alignment/padding/semantics remain unproven. Same-envelope positive requirement cannot currently satisfy concern isolation: deferred live/schema design obligation, not erased or weakened into different-envelope acceptance. |
| S3.4 real type/macro private members and useful sharing | S4.3 | Two independently exact private members, corroborated equal contracts and useful common body still required. Frozen three invalid macro lifts are not exact; seven external observations remain report-only. |
| S3.2/S3.4 full-target production naming prerequisite | S4.4; S3.2 retains enforcement | Exact binding failure above remains; production still needs unfiltered complete:true plus separate identity approval. A machine loop may stop/account it, never advance through it. |
| S3.4 closure-matrix recovery; S3.5 failure matrix | S3.5 | Actual native stop/cancel/timeout, rollback and state-inspected resume still mandatory; fixtures cannot replace native evidence. |
| S3.2 bounded orchestration; S4.2 useful finite campaign | S3.2 / S4.2 respectively | Machine loop may account blocked and simulate controlled fixture stages; S4.2 owes live accepted/no-op closure and intervention measurement, not five-blocked success. |

The integration/closure matrix below is read with this mapping: S3.4 owns machine
routing, supported owner fixtures and fail-closed accounting; S3.5 owns actual
native recovery; S4.3/S4.4 own live positives and production closure. All five
blockers, linked pair and full-report failure survive. The frozen consumer's
unchanged-envelope ceiling needs a separately reviewed fresh baseline for later
mutations; do not silently replace the five or fabricate current positive proofs.
Finite ceilings remain five entries, one writer, two repairs/entry, 60 spawns and
two hours, with stricter native timeouts and no reset on resume. Machine completion
never implies full finite campaign or whole-game completion.

Latest independent accounting acceptance is
`.pi/sessions/subagent-artifacts/outputs/efb1b118-5597-4a95-a40e-991f244688c6/review-frozen-accounting.md`:
161 checks and actual five-blocked accounting preserving 435 byte/mode states.
It supersedes only the pending accounting verdict below; its whole-S3.4 blocker
was against the former combined scope, not approval of this new machine scope.
The all-handler attribution remains 11fcb490 as recorded below. No native runtime
has been exercised by this plan revision; full source-audit timeout is not a pass.

After independent plan/machine review, smallest S3.5 rehearsal is one frozen
selector, one writer, a short parent-frozen budget and **actual native stop before
apply**, then inspection of native run state, owned bytes/modes/absence and
unrelated/index preservation before a nonmutating resume/skip decision. This is
only the first failure-matrix cell, not S3.5 acceptance. Later cells still require
actual cancellation during mutation/checks, after apply before review, and after
acceptance, plus timeout/tool/partial-write/verification failure, owned rollback
and no duplicate apply. Any mutating rehearsal needs separately scoped safe owned
inputs and authorization; controlled fixtures label synthetic domain evidence.
Verify current native runs.run, stop/cancel and state/resume protocol/tool exposure
first (historical 0.66.0 discovery is not current runtime evidence), four project
role discovery/check tools, one-writer ownership, frozen baseline/budget and parent
freshness/attestation/recovery availability. Unavailable native tools block rather
than trigger CLI fallback, installs or extension edits. Worktree allocation still
requires a clean source checkout; no unauthorized Git writes to clean this one.

## S3.4 whole-machine acceptance (superseding status record)

Parent-authorized recording of independent reviewer `2dc9dec6`'s **whole S3.4
acceptance**, not an inference from slice passes:
`.pi/sessions/subagent-artifacts/outputs/2dc9dec6-1d6e-4561-a518-fa9ad96db172/whole-machine-s34-review.md`.
The combined review ran **715 passed, 2 skipped** and real five-blocked accounting;
synthetic owner/handler/positive-consumer fixtures prove the machine boundary,
not live domain closure or native recovery. Before this status-only edit, the
reviewed plan SHA-256 was verified as
`bd7dd15e04a952cd5978598b88ffbdeb4c5cb228a469fe3036ebe93cd6505d0f`
and the sorted 220-file harness manifest SHA-256 as
`6c0aebe1780db49ecb31c5d5efae9bf146b2d8a4dd64961f5c38304d800bc82d`
(sorted `tools/python/harness/**/*.py`, each `SHA256  repository-relative-path\n`).

Only S3.4 becomes done. This supersedes earlier pending whole-machine/readiness
claims, including the separation map's pending verdict, not historical evidence
or obligations. S3.5 remains open with its S3.4 dependency now satisfied; native
capability, baseline, budget and authorization prerequisites still precede any
first pre-apply interruption cell. No fixture preparation or native launch occurs
here. S3 stays in-progress, S3.2 blocked, S4 open and S4.3/S4.4 blocked unchanged;
all live five-entry, linked same-envelope, full-target naming, full native failure
matrix and source-audit limitations remain. Machine acceptance is neither launch
permission nor campaign/production completion.

## S3.5 preapply cell (accepted bounded scope; superseding status record)

Parent accepted independent reviewer `9342ff6b`, finalized by retained continuation
`41d69a9a`, at 2026-09-09T00:22:09.448Z:
`.pi/sessions/subagent-artifacts/outputs/9342ff6b-5a71-40de-98f0-5ff48cebe0fe/review-native-preapply.md`.
Only preapply native interruption, retained read-only recovery/skip and captured
preservation close. S3.5 becomes in-progress, not done; this supersedes earlier
NOT RUN/open claims for this cell only. S3.2 stays blocked; S4 stays unchanged.

Workflow `1f1751d3` remains failed/paused-after-interrupt, not relabeled success.
Original child `a0d25e00` printed the no-transaction checkpoint then its bash tool
returned `Command aborted` (00:13:43.752Z–00:13:52.999Z). Paused state and observed
runner closure establish resumable termination, not task success. Recovery
`43b8ea26` appended to the same persisted session: exactly one checkpoint execution
across 16 tool calls; zero applications/repairs, no historical FUNCTION replay.
Its runner closure and canonical-session lease release were observed.

Independent captures under `/tmp/bof3-native-preapply-eb2b18c2/` compared 26,030
states and all nine Git evidence files with zero drift, including union membership
and five owner-root enumerations. Preserved main/HEAD, raw index, empty staged set,
starting dirty status/binary diffs, captured Git-operation absence, plan/pilot,
historical FUNCTION absence/current renamed source and recursive report/index roots.
This is sampled preservation, not continuous monitoring: arbitrary unseeded ignored
paths, symlink referent contents, all metadata, transient restored changes and active
native artifacts are not globally covered. No rollback/restoration or independent
OS process-tree sweep occurred. Remaining native matrix: interruption during
mutation/checks, after apply/before review and after acceptance; timeout/tool failure,
partial-write/verification failure, owned rollback and no duplicate application.

Original reviewer `9342ff6b` failed with `Subagent timed out after 180000ms`;
its incomplete inspection was not acceptance. Recovery took 137.536s, exceeding the
requested 120s target; its status has no deadline. Original three-launch/600s
rehearsal allowance is exhausted (deadline 2026-09-09T00:22:36.394Z); the separately
authorized five-minute reviewer continuation did not reset it. Original campaign
budget remains unknown. No further native launch or domain authority follows.

Read-only timeout investigation traced actual parent calls and installed native
0.66.0 owners: `43b8ea26`'s resume supplied 120s only in message text, no API
`timeoutMs`; direct revival forwards only an explicit timeout/internal deadline,
not the stored source deadline or normal launch default. Its absent status field
therefore accompanies an absent run-deadline timer, not just serialization loss.
`41d69a9a` supplied API `timeoutMs:300000`; status and recovery descriptor retain
absolute deadline 1788913575510, with 117.179s completion before expiry. Native
launch passes that deadline to the runner's real timeout/abort path. This proves
configured propagation plus early completion, not expiry enforcement on resume.
No repository-owned native caller defect was reproduced; no code/helper is needed.
Use explicit native `timeoutMs`, never task text as the timer or a fresh relative
allowance as an implicit ledger reset. Investigation/evidence and the concrete
parent-executable next timeout cell (NOT RUN), including absolute budget requirements:
`.pi/sessions/subagent-artifacts/outputs/db9bd640-9c16-401d-a08f-4e6224e049c7/record-and-advance-recovery.md`.
That proposed cell requires separate bounded native execution authorization; this
continuation permits reconciliation/development only. Full source-audit timeout
remains unresolved, not passed; independent review of this recording is required.

## S3.5 retained-timeout cell (accepted bounded scope)

Independent review `12b72400` accepted retained-resume timer expiry during an
active tool, captured preservation and observed runner closure/lease release:
`.pi/sessions/subagent-artifacts/outputs/12b72400-9be4-4049-a2f4-b63f5ed52f9d/review-retained-timeout.md`.
Actual run `974d6e09-8987-4d5d-a651-84ab6a4dec36` supplied API
`timeoutMs:120000`; native timeout occurred 1 ms after deadline 1788914432546,
followed by `Command aborted` and closure 4,060 ms after timeout. The run remains
failed/timedOut/acceptance rejected, not successful. Review verified 26,030 states
and ten Git/inventory files unchanged, raw index
`558a9432955f1286a0dd70eff53c283039c6bfc905b00a182322ce47eb540e74`,
empty staged paths, absent new timeout output and preserved prior report.

This supersedes NOT RUN/expiry-unproven claims only for this retained-timeout cell.
S3.5 stays in-progress; S3.2 blocked and S4 unchanged. Rollback, during-check,
after-apply/before-review, after-acceptance and independent hard-stop/process-tree
cells remain unfinished. Old rehearsal budgets remain exhausted and original
campaign allowance unknown; this acceptance resets neither and grants no launch
authority. Current separately scoped disposable failure demonstrations are
synthetic machine evidence, not live source/report/index fault authority.

## Refreshed selected-row baseline and finite pilot

Selected `emi/battle/battle/15@0x800A3638` is now `passesBytePairGate`,
APPLIED and independently accepted by reviewer `94f37f72` in workflow
`93ed23ed`. Parent actually completed postapply-review and final verify (exit 0,
`applied:true`, `rows:1`). This plan-only refresh repeated **read-only verify**
with the retained bundle and explicit evidence root; it returned the same result.
Frozen report `out/reviews/plan-audit-naming/emi__battle__battle__15.json` retains
SHA-256 `bc6c9cabcb1dfea5cbf3746598cf7e966b58742fe92876cf4e4d53cbf5f2a500`.
Evidence root is `out/reviews/evidence-access-selection-regeneration`; bundle is
`postapply-ae216471b00c2e8f9e615c11a4cc3d29/reviewed-postapply-809d3534b4a012ce2f5c40c4c85e15e5.json`
under that root; parent attestation is root-level `parent-review-d8be8adb.json`.
No gates, report, source, target or index regeneration occurred in this refresh.
Future tooling edits can invalidate this closure: retain the historical success,
then obtain fresh gates/review rather than rewriting or rebinding old receipts.

This resolves the historical missing proposal/application blocker for **one
function**, not full-target audit, automated loop, type/macro acceptance or S4.
The older 249/250-row failures and hashes below remain historical evidence;
current inventory after rename need not equal the frozen preapply report.

Pilot selection below comes from live `rev-query` type-candidates,
macro-opportunities and variables queries plus the verified function binding.
These are five work entries, not five promised mutations. S3.3 freezes current
fingerprints and full membership before execution; new entries or replacement
candidates require parent approval. Already accepted entries are read-only controls,
not repeat applications. Cross-target observations do not expand write authority.

| Pilot | Live identity / owner route | Current outcome / exact next evidence |
| --- | --- | --- |
| Function | `emi/battle/battle/15@0x800A3638`, frozen `function:func_800A3638`; naming prepare/postapply/verify | Accepted control above; prove skip/replay detection and final loop accounting without renaming again. |
| Local data | `emi/battle/battle/15@0x80096994`, `data:D_80096994`; naming evidence/validation | BLOCKED per independent review d740aabe below: retained capability validates, semantic exhaustion/no-op is not accepted. Preserve historical exhausted report row/name; parent may scope read-only handler review through bof3-reviewer, not row dispatch to bof3-namer. |
| Shared fixed RAM | `emi/battle/battle/15@1F800044:storage`; type candidate, mapped `g_battle_work = 0x1F800044` | Width 4 lead; base/extent/semantics unresolved. `variables g_battle_work` returns no rows despite map/type presence: S3.3 must establish whether intentional query scope or coverage gap and route fixed RAM explicitly. Corroborate real owners/accesses before any shared change. |
| Struct/layout | `emi/battle/battle/15@1F800044:aggregate_region`; type-audit | Indexed blocked lead: extent/alignment/padding/semantic role unproven. Shares address with storage entry: one serialized concern transaction, linked accounting, not two conflicting applications. Two independent observations required; no inferred struct identity. |
| Macro opportunity | `exact_group:8e1ad03b4ba92303`, target scope `emi/battle/battle/15`; macro-audit | Members `@800b2218`, `@800b22ac`, `@800b250c` were invalid at freeze; the reviewed Codex metadata cleanup above establishes exact bytes and indexed status. Macro acceptance remains BLOCKED: cross-target private/shared proof and semantic/source-shape gates are unclosed. Membership is frozen below; no shared promotion follows. |

Initial implementation was **S3.3**, not a production rename: worker adds
narrow index/routing characterizations and only reproduced owner fixes; reviewer
checks actual queries and tests. Follow with S3.4, S3.5, S3.2, S4.1, S4.2, one
writer/reviewer iteration per slice. Parent launches these iterations; BOF3 roles
retain all domain skill gates. No worker/reviewer profiles are installed/copied.

## S3.3 accepted implementation checkpoint

Independent reviewer `30a418da` accepted the prerequisite plan-only refresh in
`.pi/sessions/subagent-artifacts/outputs/04f0918b-70b7-4249-afca-a4b77958d75e/review-autonomy-plan.md`;
46 plan checks and read-only selected FUNCTION verification passed there. This
accepts the plan, not S3.3 runtime or subsequent transactions.

Worker run `e423967f` reproduced `variables g_battle_work = []`: symbol insertion
classified every non-`D_` map name as a function despite the target-owned global
declaration. The repair uses existing `header_claim` global type usages, rejects
function-entry/prototype collisions, preserves unproven fallback and target
isolation, and bumps the disposable index schema to v12. No domain identity or
storage authority changed. Parent ran `bin/index --recover` (exit 0); subsequent
live queries return `g_battle_work` as data. `describe` still reports unknown
storage authority outside the payload: indexing is not fixed-RAM ownership proof.
Battle15 snapshot is fresh; binary SHA-256 remains
`77d963c56e3ba6b1619323e3007c25d70300e855ac9f4b088ddce2604f92e140`.

Frozen native inputs:
`.pi/sessions/subagent-artifacts/outputs/e423967f-e0f5-4e3c-9718-62a74c8de235/pilot-input.json`,
SHA-256 `bb68a07c5fe271005251d11634b6ed4496a28418d961193304ad39f00719c218`.
Exactly five entries retain owner routes, source/binary/index fingerprints,
blocked/control outcomes and storage/layout serialization. These are review
inputs, not launch or application approval. Function remains a historical accepted
skip control; local-data exhaustion needs receipt revalidation. Both type concerns
remain blocked. Tooling changed: old final-state bundle is historical only; fresh
owning gates/review are required, never receipt rebinding.

Macro membership was equal before/after recovery: the selected three battle15
members in the pilot table are a target-filtered subset of ten reviewed-byte
members, not a missing-member defect. The other seven are report-only:
`emi/etc/game/00@801996fc` and `emi/etc/shop/00@801e31c4`, `@801e3774`,
`@801e3bf8`, `@801e3d4c`, `@801e438c`, `@801e4540`. All ten have invalid lift
status; reviewed SHA-256 is
`b5a0227129c99631166270ac80b2bfc769b0c330181a0be7b9ca7e97adcaee78`,
size 64. No macro query repair or cross-target write expansion was needed.

Focused reverse/type/macro opportunity/index and harness decomposition checks:
116 passed; 46 plan tests and plan status passed; scoped Ruff and `git diff --check`
passed. Bounded `just check` timed out at 110 seconds during pytest (past 81%, no
failure shown); full-suite/lint/maps/source-audit completion is not claimed.
Tests cover refresh inventory
equality, stale headers, classification/collision/fallback/isolation, distinct
blocked fixed-RAM concerns and complete target-filtered/global macro membership.
Independent review now accepts S3.3 as recorded below; S3.4 is the next slice. Native recovery/application and full-target
naming closure were not exercised and remain later gates.

Independent read-only reviewer accepted S3.3 in
`.pi/sessions/subagent-artifacts/outputs/e423967f-e0f5-4e3c-9718-62a74c8de235/review-0.md`:
174 routing/index/plan/harness tests and 120 owner tests passed (294 total).
All five frozen payloads, 437 source fingerprints, three binary/snapshot pairs,
index/report hashes and ten-member macro inventory were independently equal.
No source/report/index mutation or fresh historical FUNCTION closure was claimed.
This acceptance does not complete S3, full-target naming or native recovery.

## S3.4 accepted trust-boundary sub-slice

Supervisor approved limiting this iteration to duplicate-key rejection at existing
reviewed type/macro JSON boundaries. Their previous last-value parsing accepted
contradictory top-level and nested reviews while the final parsed digest remained
valid. Reuse the naming parser's existing `unique_object`, moved to `harness/io.py`
with direct importer updates to avoid an analysis-to-naming dependency. Apply it
to type candidates/private proofs and macro opportunities/exact proofs. No schema,
valid-input policy, domain identity, transaction authority or full-target gate
changes. Ordinary `read_json` and other parsing boundaries are unchanged.

Eight added parameterized owner cases plus one extended shared-macro proof case
cover duplicate schema/reviewer/verdict/semantic-guard/concern rejection; prepare
rejections preserve all fixture file bytes. Last-value parser substitution makes
all nine cases fail to reject; strict parsing passes the 128 owner checks. An additional
194 naming/plan/harness checks passed, two skipped; scoped Ruff and plan status
passed. `just check` timed out after 110 seconds at 81% pytest, no failure shown;
full-suite/lint/maps/source-audit completion is not claimed. These
are fixture integration results, not native pilot application or independent
review. Snapshot/index/report and historical proof artifacts remain untouched.

Independent read-only review accepted this duplicate-key sub-slice in
`.pi/sessions/subagent-artifacts/outputs/e423967f-e0f5-4e3c-9718-62a74c8de235/review-1.md`:
439 tests passed, two skipped; scoped lint/format and five-class live payload and
fingerprint comparisons passed. No blockers or domain mutations. This accepts
only the prior repair, not S3.4 final review or native recovery.

S3.4 remains in progress. Next after this sub-slice's independent acceptance:
add concern-owned final review ingestion and verification, keeping the order
`execute native gates → distinct reviewer inspects → parent attests → final verify`.
The approved local trust model requires explicit parent attribution of distinct
implementation/reviewer run IDs and retained reviewer artifact, bound to the exact
request/application/final state and native gates. Existing writer-created digest
pins remain integrity evidence only, never relabeled independent acceptance.
Use owner-specific review input/envelope additions; no generic framework or new
credentials. Unsupported local-data/shared closure stays fail-closed and storage/
layout stays serialized. Remaining all-class coverage, fresh native domain gates,
parent attestation and S3.5 cancellation/recovery are NOT RUN; no live promotion
or stale receipt rebinding is authorized by this repair.

## S3.4 accepted native-gate prerequisite

While preparing parent-review ingestion, the worker traced exact type/macro
checks to the native asm-diff/byte-match payload owners: exit zero alone accepted
warning, malformed and missing-status JSON. Supervisor approved repairing this
shared owner seam first rather than layering final authority on insufficient
gates. Both exact gates now require their closed native schema, positive exact
and byte-match booleans, selected source/function/address, target-owned binary,
equal positive sizes and coherent asm counts/no mismatch. Splat/build retain their
plain-output exit contract; non-exact transactions retain pinned partial evidence.
Strict duplicate parsing applies to native JSON and retained command receipts.
Exact stdout remains complete and separate from retained stderr, so receipt replay
cannot mistake diagnostics or a truncated JSON tail for the command payload.

Targeted type/macro fixture integrations cover exit-zero rejection, rollback and
failed receipts, plus successful receipt replay with large stderr and rejection of
rehash-consistent warning receipts. These are not live BOF3 gates or final review.
No request/application authority, shared promotion, full-target naming gate,
source/map/type/macro identity, index or native pilot artifact changed. New owner
parent envelopes/context capture are NOT IMPLEMENTED in this coherent prerequisite
slice; old application pins remain integrity-only. Validation: 225 owner/naming/plan/harness checks and 117 routing/index checks
passed; scoped Ruff, plan status and diff whitespace passed. All 437 frozen
source fingerprints, three binary/snapshot pairs and index/report hashes remain
equal; staged paths are empty. `just check` timed out at 110 seconds during pytest
(past 53%, no failure shown); full-suite/lint/maps/source-audit completion is not
claimed. Evidence is in
`.pi/sessions/subagent-artifacts/outputs/e423967f-e0f5-4e3c-9718-62a74c8de235/implement-2.md`;
independent review accepted this prerequisite in
`.pi/sessions/subagent-artifacts/outputs/e423967f-e0f5-4e3c-9718-62a74c8de235/review-2.md`: 342 checks passed; no blockers. This accepts native payload/receipt integrity only, not S3.4 completion.

Next ready work remains S3.4: approved owner-specific review/final-verify APIs,
caller-retained parent-envelope digest, explicit distinct implementation/reviewer
runs and supervising parent attestation, retained reviewer bytes, native execution
context across gates/publication (inputs, modes/absence, tooling/environment and
index), and shared-promotion rejection of legacy integrity-only pins. Preserve
`gates → reviewer → parent → final verify`; unsupported data/shared pilot closure
stays blocked. No S3.5 or live application is authorized by this prerequisite.

## S3.4 execution-context slice (implementation pending independent review)

Type/macro `run --implementation-run-id` now binds an owner-derived closure
before edits, around each native check and application publication. Macro reuses
the type-owned runtime; naming delegates its existing closure to that same input
owner without changing FUNCTION scope. Context retains request/manifest/adopted
baseline, sources/headers/config/binary/tooling/catalog presence, modes, index,
resolved native tools/environment digest, explicit edits and native receipt/build
transitions. Ninja is the supported build ceiling; configured variants must
already exist and environment toolchain overrides reject. Legacy omitted-ID run
and verify remain integrity-only; context itself is not independent review.

Mode capture reproduced an existing application defect: quarantine followed by
new-file publication reset nondefault permissions. Supervisor approved forwarding
the original mode through validated optional `atomic_write(mode=...)`; omitted
mode/new-file behavior stays unchanged. Owner fixtures check successful and failed
applications preserve mode 0750, native drift stops subsequent gates, publication
and replay drift reject, and malformed context/duplicate CLI JSON reject.
No live application, report/index recovery, identity promotion or historical proof
rebinding occurred. Tooling changes invalidate old proof closure. Current slice
is NOT independently accepted; S3.4 remains in progress and S3.5 stays unready.
Validation: 251 owner/naming/plan/harness and 117 routing/index checks passed;
26 new targeted cases are included. A single 600-second `just check` completed
1653 tests (two skipped), repository Ruff and maps, then timed out during the
source audit; audit completion is not claimed. All 1124 frozen source/header/
config/report/reverse-index hashes remained equal. Raw Git index bytes changed
with no staged paths (stat refresh is suspected, not proven); no staging, reset
or raw-index restoration was attempted. Scoped format/lint and plan status passed.

Independent read-only review blocked this context slice in
`.pi/sessions/subagent-artifacts/outputs/0e915fbc-2892-41c2-9738-ca2720278f57/owner-review-0.md`:
368 focused checks passed, but native Splat launcher/runtime and Ninja-executed
VerifyGlobs inputs were omitted, and context rejection preempted Git-index and
owned-source rollback. Repair and independently re-review this same slice before
parent-envelope ingestion. No execution-context acceptance or S3.5 readiness is
claimed; the earlier raw-index discrepancy remains historical and unresolved.

Independent re-review remained blocked in
`.pi/sessions/subagent-artifacts/outputs/0e915fbc-2892-41c2-9738-ca2720278f57/owner-review-1.md`:
380 checks passed and launcher/build/index rollback repairs were accepted, but
CLI application publication still validated context outside owned-source rollback.
The publication-boundary repair remains pending independent review; no context
slice acceptance, parent-envelope readiness or S3.5 completion is claimed.

Next after that independent acceptance: owner parent envelopes with distinct actual reviewer/implementation IDs,
parent identity and retained accepted reviewer bytes, externally pinned final
verification, then shared-promotion migration rejecting legacy integrity pins.
Native pilot/recovery and unsupported data/shared closure remain NOT RUN.

## Integration test and closure matrix

Use existing owners: `analysis/index_build.py`, `index_validation.py`,
`rev_queries.py`, `type_index.py`, `type_transactions.py`,
`type_transaction_checks.py`, `macro_index.py`, `macro_opportunities.py`,
`macro_accounting.py`, `macro_transactions.py`, and `naming/prepare.py`,
`postapply.py`, `postapply_review.py`. Read actual owning tests before adding
coverage; no generic framework, new database, dependency, or scheduler.

| Coverage | Required positive and rejection evidence |
| --- | --- |
| Inventory/routing, all five classes | Target-qualified selectors, source/binary fingerprints, complete finite membership, ownership and overlap exclusions survive index refresh; missing/stale rows fail closed. Indexed opportunities are not approvals. Cross-target same-address/byte coincidences do not merge owners. |
| Function/local data/shared variables | Evidence → authored proposal or justified exhaustion/no-op → independent approval → scoped application → native gates → separate review → final verification → accounting. Test unchanged function body/ABI/range; data address/storage/width, all declarations/references and fixed-RAM ownership. A blocked evidence ceiling never becomes no-op just to advance. |
| Struct/types | Existing type-audit account/validate-account/prepare/run/verify: widths, signedness, field offsets, extent, padding, alignment and semantics need corroboration; conflicting consumers reject. Shared promotion requires two independently verified private applications with identical contracts, no target-address-bearing shared contract. |
| Macros | Existing macro-audit account/validate-account/prepare/run/verify: every selected opportunity ID once; automatic safe application count stays zero. Check evaluation count, side effects, precedence, lvalues, integer promotions, aliasing, volatile/control flow; shared template requires independently exact members and explicit owners. Generated/helper exclusions survive routing. |
| Trust/freshness, every owner | Missing/duplicate/unknown JSON fields, stale report/row/source/binary/index, malformed receipts, wrong root/digest/selector, replay and writer self-approval reject before writes. Failed mandatory gates and exit-zero actionable warnings never advance. Approvals bind current changed content, not author assertions. |
| Native recovery | Cancel/stop before apply, during mutation/checks, after apply before review and after acceptance; timeout/tool failure, partial write and verification failure retain evidence. Restore only transaction-owned bytes/modes/absence from adopted baseline, preserving concurrent/unrelated changes. Inspect native state and current hashes before resume; accepted work is skipped, uncertain application stops for parent/cleaner recovery, never blindly reapplied. |
| Final accounting | Each frozen entry has accepted application, independently justified no-op/exhaustion, or explicit blocked/deferred owner/reason/next action. Preserve cross-entry dependencies; missing/duplicate entries and unsupported routes fail campaign completion. Requery freshness after sanctioned parent recovery and independently verify final applications with trusted retained digests. |

Focused validation: existing `test_reverse_index.py`, `test_type_index.py`,
`test_macro_index.py`, `test_macro_opportunities.py`, `test_type_transactions.py`,
`test_macro_transactions.py`, `test_macro_accounting.py`, `test_naming_audit_check.py`
and relevant naming lifecycle tests, then `just check` when practical. Tests
assert behavior/parsed structures, not Markdown strings. Fixtures may demonstrate
rejection/recovery but cannot substitute for actual native stop/resume and live
owner-gate evidence. List NOT RUN paths explicitly. Plan-only checks are
`bin/plans status autonomous-bof3-decompilation.md` and existing `test_plans.py`.

Proposed finite launch ceilings: five entries above, one writer, two repair rounds
per entry, 60 total spawns, two hours campaign wall time; preserve stricter native
command timeouts. Parent freezes these limits with queue fingerprints before
launch. On exhaustion stop with remaining obligations; no implicit budget reset
on resume. Record elapsed time, stage attempts, cancellations, blocked/deferred
counts and each parent intervention/reason (refresh, attestation, recovery,
permissions) in native artifacts. No claim of zero-intervention autonomy while
parent checkpoints remain required. Worktrees require a clean source checkout;
this dirty checkout cannot be made clean via unauthorized Git writes.

Integration acceptance means all five classes have checked routing, transaction,
recovery and accounting behavior with independent review. **Full finite campaign
closure additionally requires every entry accepted or independently justified
no-op/exhaustion, with no blocked/deferred entry silently counted done.** A
blocked pilot can prove fail-closed integration, not campaign completion. Neither
level implies target-wide semantic completeness: retain the existing full-target
`complete:true` naming gate unchanged; no blocked naming gate can advance a
production target audit. S4 stays open until its declared scope is accepted.

## Current prerequisite implementation

Infrastructure scripts accepted by reviewer `891ecdeb`: snapshot status global
JSON flag repaired, bootstrap import lint repaired without suppression, live
snapshot/mission JSON checks and two existing tests passed.

Reviewer `8e026bce` corrected the route analysis: `prepare-transaction` already
supports authored proposed FUNCTION rows independently of exact-capability
`conclude`. The implemented `--candidate JSON --expected-sha256 HASH` extension
replaces exactly one blocked initializer in memory before readiness, provenance,
full validation and atomic publication; `--evidence-root` retains explicit receipt
context. Independent reviewer `3761d476` verified success, preservation and fault
paths; duplicate-key repair accepted by `8e86d774` with production fixture replay.
Seventy existing checks passed locally. That was the pre-application baseline; the selected FUNCTION has since been prepared, applied and independently accepted as recorded below.
Hashes and reviewer labels are integrity/coordination evidence, not authentication;
parent-controlled semantic review and separate identity application remain required.

## Full-autonomy readiness blocker (historical finding, corrected above)

Independent reviewer `5d19eb7d` (workflow `6b5af217`) inspected current domain
implementation and native pi-subagents 0.66.0. Review:
`.pi/sessions/subagent-artifacts/outputs/6b5af217-1585-4f62-9e10-103e7f91fba8/loop-contract-check.md`.
`terminalize_report` is a no-op; the importer rejects conclusions without trusted
capabilities; current capabilities exclude functions and semantic proposals.
Thus `function:func_800A3638` cannot close through the existing importer. A
fail-closed workflow shell is possible, but cannot fulfill S4 under the retained
full-target `complete:true` gate. Do not narrow that gate or mark collection as
semantic completion.

Semantic prerequisite review `0f3efd96` (workflow `94ba66ba`) examined the
complete selected body and all three caller sites against original bytes. Caller
use supports a suppressing predicate for requested mask updates; a narrow
descriptive name remains plausible. Exhaustion is not justified. Consequently,
do not implement the suggested exhaustion-only function pilot merely to fit the
current importer. The missing route must support independently reviewed semantic
proposals, or this function must remain blocked without changing its identity.
Evidence: `.pi/sessions/subagent-artifacts/outputs/94ba66ba-941e-4833-831a-879cee08ea7a/function-semantic-prerequisite.md`.

Historical required scope decision (selected FUNCTION resolved below; general
classes still need owner-specific integration): design a trusted, evidence-backed function/general
conclusion route with independent approval, current report/row binding and
serialized import/revalidation. Expert JSON alone is not authority. Retain
existing receipt validation and evidence-preserving recovery. Define explicit
ownership for post-lift snapshot/index refresh without granting raw scripts host
execution or silently widening a role. This historical dependency record authorized no tests, dependencies or installed
extension edits; only the current targeted test authorization above supersedes it.

## Reconciliation recovery evidence

Reviewer `3721cee2` (workflow `a2e7d74f`) accepted the exact battle15 additive
operation. Parent verified all five reviewed code hashes and report hash, then
ran `bin/naming-audit reconcile emi/battle/battle/15 --apply`. Only missing
`function:func_800A3638` was appended; all 249 prior ordered rows, top-level
values, mode 0770 and 23 other report-set files were preserved. Report SHA-256
is `42451b711ee48eff8cc35641345d27f2cdd519a63341d2854454de7bdb1558af`.
Full validation passes with 250 rows, `complete:false`; repeated preview is a
no-op. This resolves inventory staleness, not naming completion. Prepared
whole-report proposal receipts may still reject additive changes as stale;
reconciliation never rewrites receipts to bypass validation.

One-row closure: reviewer `42ea1461` accepted corrected exhaustion envelope for
`data:D_80096994`; parent imported its exact JSON via `conclude` with the explicit
regeneration evidence root. Only that row changed; all 72 evidence files stayed
hash-identical. Report digest became
`ca8b82bdc1e48933632483b890bacc69ea40efaa72222e385ace20ec36a5dd85`.
Plain full validation rejected absolute receipts without root context. Parent
added explicit `validate --evidence-root` using existing canonical-root/context
helpers; live full validation now passes with that root, 250 rows, complete:false.
112 existing tests passed, 2 skipped; independent validation-context review is
pending. This is one exhausted row, not semantic renaming or target completion.

Direct handoff acceptance: cleanup reviewer `2aad3a20` (workflow `003d2e1c`)
accepted justified no-op for `emi/battle/battle/15@0x800A3638`: live 20/20
instructions and 80 bytes, symbols/Splat checks passed, 2,008 authored files
unchanged. Naming collection/root-generation integration was accepted by
`14c95adb` (workflow `e10aad04`); canonical report remains one exhausted and
249 blocked. Together with the independently accepted lift these complete
S3.1's role/handoff demonstration, not S4's end-to-end naming acceptance.
S3.2 must preserve this distinction: collection progress cannot satisfy a full
naming gate or silently advance the production loop through blocked naming.

## Accepted implementation evidence

S1/S1.1 and S2/S2.1/S2.2 accepted by independent project `bof3-reviewer`
run `317d0a41-fe6b-407b-a61b-7eb6101d0875`, workflow `9222db22`.
Review: `.pi/sessions/subagent-artifacts/outputs/9222db22-4148-4989-b0c3-8ff7cf1ead7f/contracts-recheck.md`.
167 existing tests and both skill-script checks passed; emitted context and
native 0.66.0 discovery verified; 30 scoped Markdown files had valid references.
Acceptance applies to these contracts, not S3/S4 runtime behavior. Initial S3
candidate: `emi/battle/battle/15@0x800A3638`, subject to fresh mission checks.

The owner-requested performance detour is complete: capture-local source lookup
reduced profiled battle15 capture from 5.14s to 1.23s with identical snapshot JSON.
Reusing existing manifest/PsyQ inputs and batch source lookup reduced profiled
index rebuild from 555s to 10.0s; actual `bin/index` took 4.89s. All table contents
were equal and 105 existing focused tests passed. No new cache, dependency or
regression tests; these timings do not accept S3/S4 autonomy.

## Historical scope is not completion

This proposal replaces the former program at the owner's request. Its domain
work is not marked complete by deletion: F4 identity reconciliation, F5 duplicate
configured claims, denominator/LOGO interpretation and other retained findings
remain leads to reverify when relevant. Do not turn every historical lead into a
prerequisite for a single lift. Native sessions and original data are unchanged.
The previous program's verified plan-only recovery copy remains at
`/tmp/bof3-retired-plan-0axpva0h/autonomous-bof3-decompilation.md`, SHA-256
`dbe8c70630872f8e110274c5481c68fbd007f948b8f471c57d590569bb428982`;
it is history, not a workflow dependency.

## S3.4 parent-envelope slice (accepted bounded scope)

Independent reviewer accepted the execution-context/mode/publication prerequisite
in `.pi/sessions/subagent-artifacts/outputs/0e915fbc-2892-41c2-9738-ca2720278f57/owner-review-2.md`:
390 checks passed; no blockers. This supersedes the pending context verdicts
above, not historical index uncertainty or full S3.4 acceptance.

Owner `type_application_review.py` and `macro_application_review.py` now provide
`review_application`/`verify_reviewed_application`, with CLI `review` and
`final-verify`. Closed parent envelopes explicitly accept actual distinct parent,
implementation and reviewer runs, retained reviewer path/hash, exact original
application/manifest/request/context/post-state/native receipts and adopted
baseline preservation. Replay uses the externally retained envelope digest,
original owner integrity verification, current closure and unrelated adopted
workspace checks; omitted-ID legacy applications cannot become independent
acceptance. No prose-derived acceptance, credentials, new database or gate rerun.
Fixture integrations exercise actual owner preparation/application/receipts,
CMake/Ninja generation, ingestion and replay; exact BOF3 payloads remain synthetic.

Independent review accepted this bounded slice in
`.pi/sessions/subagent-artifacts/outputs/4d41995f-5a49-4cc6-b403-a7e53e13f5ec/final-review-check-0.md`:
329 tests passed in 205s; scoped lint/format, plan/diff and raw-index checks passed,
no blockers. Shared migration and full S3.4 were not accepted. S3.4 remains in progress; unsupported data/shared closure, live native
pilot gates and S3.5 cancellation/resume remain NOT RUN. No live application,
report/index recovery, historical proof rebinding, installs or checkout Git writes.

Validation for this slice: 327 parent/owner/naming/plan/harness checks passed;
a subsequent parent/routing/index run passed 173 checks (56 parent cases,
117 existing routing/index cases; two retained-receipt cases added after the
first run). Scoped Ruff/format, plan status and diff whitespace passed. Raw
checkout index remains `558a9432955f1286a0dd70eff53c283039c6bfc905b00a182322ce47eb540e74`,
with no staged paths. Full `just check`/source audit was not repeated: prior
`validate_sources` timeout remains an evidence limitation, not a pass.


## S3.4 reviewed shared-input migration (accepted bounded scope)

Type/macro shared preparation now requires closed `expected_envelope_digest`
pins and owner final verification of retained private envelopes; legacy integrity
pins reject. Existing target/private/exact-wrapper/contract/dependency guards
remain. Standalone integrity verification is unchanged. Shared parent review and
final verification explicitly reject until the owner transition below exists;
positive preparation is not shared acceptance.

Inspection found that current closure includes all source/header/config inputs,
Ninja state and unrelated adopted workspace: a later private mutation invalidates
the first private envelope. Existing `run` requires real changed paths and has no
check-only revalidation operation. Shared replay validates stored manifests but
has no historical prerequisite validator. Supervisor approved this fail-closed
bounded migration rather than weakening closure or manufacturing no-op proofs.
Tests use real temporary private transactions/envelopes to reach the existing
shared input guards, then execute a later private mutation and reject the original
pin without rebinding. Both owners reject shared parent acceptance before CLI
publication. Existing consumer tests retain uniqueness/contract/path/exact guards;
their stubbed positives are not two-private/shared execution acceptance.

Next dependency-ready S3.4 slice: add genuine owner check-only revalidation of
already-applied private results at one finalized pre-shared state. Freeze both
fresh accepted envelopes/external pins and common pre-state at shared preparation.
Shared execution must own the explicit pre/post delta, rollback, new native gates
and independent parent review. Final shared replay must validate retained private
prerequisite integrity, nested evidence and common historical PRE-state, plus the
current shared POST-state, rejecting all unrelated drift. Standalone private final
verification remains unchanged; no historical rebinding, inferred acceptance,
generic snapshot graph or new database. Implement and independently review that
transition before removing the shared acceptance guard. Positive mutating
sharing, unsupported data/shared pilot closure, live gates and S3.5 remain NOT RUN.

Validation: 333 owner/parent/naming/plan/harness checks passed in 213.24s.
Initial aggregate had 332 passes and one module-ceiling failure (452 lines);
relocating the lazy review import into the proof owner restored the 450-line
ceiling, then the complete bounded aggregate passed. Scoped Ruff/format, plan
status and diff whitespace passed; no staged paths. Raw index SHA-256 remains
`558a9432955f1286a0dd70eff53c283039c6bfc905b00a182322ce47eb540e74`.
No live source/map/promotion, report/index recovery, historical rebinding,
installs or checkout Git writes. Full `just check`/`validate_sources` was not
repeated; its known timeout is not a pass. Independent review is still required.


## S3.4 no-drift check-only revalidation (accepted bounded scope)

Independent read-only review accepted the preceding shared-input migration and
fail-closed guard in
`.pi/sessions/subagent-artifacts/outputs/4d41995f-5a49-4cc6-b403-a7e53e13f5ec/final-review-check-1.md`:
333 checks passed in 213.42s; no blockers. This supersedes that slice's pending
verdict above, not full S3.4 or shared execution acceptance.

Supervisor approved a bounded no-drift prerequisite after inspection: owner
`revalidate_application` requires a still-current independently reviewed private
envelope, external digest, explicit current baseline adoption and new execution
ID distinct from its original parent/reviewer/implementation. It executes new
native gates without application writes, retains separate owner revalidation
schema/receipts/context, and provides externally pinned `verify_revalidation`.
Original envelope, application and receipts remain unchanged. The result says
`checked`, never `applied` or independently `accepted`; shared consumers and
standalone final verification are unchanged. No CLI or new parent-acceptance
surface is added at this layer.

The no-drift ceiling also rejects build-graph regeneration that invalidates the
original envelope; an unchanged native Ninja build succeeds. Unexpected gate
mutations stop before further checks/publication and remain for explicit parent
recovery: this check-only operation has no source mutation/rollback authority.
Fixture tests use real private applications and actual Ninja execution, with
synthetic BOF3 exact-match payloads; they are not live pilot evidence.

Next dependency-ready layer after independent review: validate exact intervening
private-transaction deltas backed by retained independently reviewed envelopes
against original/current states, not blanket workspace adoption. Then issue fresh
independent revalidation acceptance/common PRE-state pins before explicit shared
PRE-to-POST execution/rollback/new parent review and historical prerequisite replay.
The shared guard stays until that transition is accepted. Mutating two-private
closure, unsupported data/shared pilots, live native gates and S3.5 remain blocked
or NOT RUN; S3.4 stays in progress. No historical proof rebinding, source/map
promotion, report/index recovery, installs or checkout Git writes occurred.

Validation: 347 owner/revalidation/parent/naming/plan/harness checks passed in
236.79s (14 new revalidation cases); scoped Ruff/format, plan status and diff
whitespace passed. Initial positive fixtures regenerated CMake and correctly
rejected stale original build closure (12 passed, two failed); using native Ninja
against the already-finalized graph passed all 14 in 27.32s. Initial test-fixture
import lint was corrected without suppression. No staged paths; raw index remains
`558a9432955f1286a0dd70eff53c283039c6bfc905b00a182322ce47eb540e74`.
Full `just check`/`validate_sources` was not repeated; known timeout is not a pass.


Independent read-only review accepted this bounded no-drift slice including B1 in
`.pi/sessions/subagent-artifacts/outputs/3f96bc49-9797-463f-bbfe-63f6a2e903a9/review-revalidation-0.md`:
264 checks passed in 281.72s; no blockers. Existing output/prerequisite aliases
reject before gates; native exclusive publication rejects late creation without
clobbering receipts, applications or envelopes. Original bytes/modes and replay
survive. This supersedes the pending verdict above, not full S3.4 acceptance.

Independent review accepted fresh parent acceptance of still-current no-drift
revalidation in
`.pi/sessions/subagent-artifacts/outputs/3f96bc49-9797-463f-bbfe-63f6a2e903a9/review-revalidation-1.md`:
all 273 collected focused cases have passing evidence across an interrupted
290-second aggregate and nine-case completion; lint/format, plan, whitespace and
unchanged raw index passed. No blockers; this accepts only that bounded layer.

## S3.4 internal historical validation (pending independent review)

Supervisor approved a mechanical prerequisite before the reviewed-delta producer:
separate retained private application/context integrity from live post-state,
execution-input/build and unrelated-workspace comparisons. Both standalone owner
verification paths still perform those live comparisons. The internal historical
validator requires the original external envelope pin, parent/reviewer identities
and retained artifact, original application/manifest/attestation/native receipts,
context transitions and retained manifest evidence. It neither rebuilds old context
from current files nor returns current validity, accepted drift or sharing authority.
Native receipt semantics still use existing owner resolution; target configuration
or path-identity drift can conservatively reject. Shared acceptance guard remains.

Two owner integrations use real temporary transactions/native Ninja and synthetic
BOF3 payloads. Historical integrity survives unrelated/source/build drift while
standalone final verification rejects; every retained manifest artifact, receipt,
attestation and reviewer file is tampered individually and rejects. No live domain
mutation, report/index recovery, installs or Git writes. This slice is pending
independent review, not S3.4 completion.

Remaining: exact independently reviewed intervening-private deltas and continuity,
genuine new native checks for both results at one common PRE-state, fresh parent
acceptance/external pins, then explicit shared PRE/POST execution/rollback/new
review and historical prerequisite replay. No blanket adoption authority, no-op
application fabrication, proof rebinding or removal of the shared guard.
Validation: new two-case test passed; 332-case aggregate timed out at 290 seconds
with 260 passing progress results and no failure printed. Bounded completion runs
passed 10 owner-context/publication cases and 64 accounting/plan/harness cases
(including overlap). Full `just check`/known source-audit timeout was not repeated.


## S3.4 exact sequential private transition (accepted bounded scope)

Independent reviewer accepted the internal historical prerequisite in
`.pi/sessions/subagent-artifacts/outputs/3f96bc49-9797-463f-bbfe-63f6a2e903a9/review-revalidation-2.md`:
210 focused cases passed; no blockers. This supersedes its pending verdict above,
not common-PRE, shared execution or S3.4 acceptance.

Both owner revalidation APIs now retain ordered externally pinned, independently
reviewed private envelopes and validate exact POST/PRE/POST/current continuity.
Nested history remains prerequisite integrity, not present acceptance. Only owned
reviewed edits and receipt-owned native build transitions explain drift; input,
workspace, index, mode and build gaps reject despite adoption. Original owned
post-state is preserved. Existing no-drift and standalone verification are unchanged.
Ten sequential fixture cases exercise real owner preparation/application, native
CMake/Ninja and synthetic BOF3 payloads: positive two-private fresh checks/parent
acceptance at a common state, plus independently accepted second transactions
with unevidenced workspace/input/build/mode gaps that reject before new gates.
Shared guard remains; shared historical PRE/current POST replay, live pilots and
S3.5 remain NOT RUN. This implementation is pending independent review; S3.4 stays
in progress. No live source/map/shared promotion, index recovery, installs or
checkout Git writes occurred.

Validation: 10 sequential cases passed in 32.25s; 50 history/revalidation cases
(including two sequential positives) passed in 109.44s; 98 owner/context/review
cases passed in 109.35s; 64 accounting/plan/harness cases passed in 15.74s.
Scoped Ruff/format, plan status and diff whitespace passed; staged paths remain
empty and raw index remains `558a9432955f1286a0dd70eff53c283039c6bfc905b00a182322ce47eb540e74`.
Full `just check`/known oversized source audit was not repeated. The positive
sequence uses nonoverlapping private headers within one target, not two distinct
targets; differing target-specific input membership conservatively rejects.
Cross-target common-PRE and shared acceptance remain unproven, not silently
excluded from closure or counted complete.


## S3.4 distinct-target common-PRE integration (accepted bounded scope)

Independent reviewer accepted the preceding same-owner sequential transition in
`.pi/sessions/subagent-artifacts/outputs/88cf8e3a-6fc1-46fb-8d6d-9960b290ffd9/review-private-transition-0.md`:
220 checks passed, no blockers; this supersedes that slice's pending verdict only.

Distinct-target execution exposed missing historical binary/Splat/reviewed membership.
Supervisor approved explicit finite `participating_targets` capture on type/macro
Python `run_transaction` only: canonical sorted unique one/two-target set including
mutation targets, frozen before original execution and inherited unchanged by fresh
checks. Existing manifest owners derive each participant's closure; no write/check
authority expands, naming/default capture stays unchanged, and old incomplete or
mismatched histories reject. No historical reconstruction, exclusions or rebinding.

Both owner fixtures now execute two distinct private target transactions, retain
externally pinned synthetic parent envelopes, run distinct fresh native Ninja
checks at one common state, freshly parent-review both and replay nested evidence.
BOF3 exact payloads and reviewer identities remain explicitly synthetic. Missing/
mismatched scope, invalid sets, revalidation scope rebinding and participant input
drift reject alongside prior gap cases. Shared historical PRE/current POST execution,
rollback and prerequisite replay remain unimplemented; shared guard stays. S3.4 is
in progress and this slice requires independent review; no live promotion, index
recovery, installs/extensions or checkout Git writes occurred.

Validation: final sequential/accounting/plan/harness group 88 passed in 93.94s;
history/revalidation/parent group 108 passed in 184.21s; selected owner gates 64
passed in 55.17s. Scoped Ruff/format, plan status and diff whitespace passed.
Initial fixture-only failure came from Git's same-second DB stat cache after raw
index restoration; explicit fixture DB timestamp separation fixed it without
changing runtime checks. Staged paths remain empty; raw index SHA-256 remains
`558a9432955f1286a0dd70eff53c283039c6bfc905b00a182322ce47eb540e74`.
Full `just check`/known oversized source audit intentionally NOT RUN.


## S3.4 shared PRE integration (accepted bounded scope)

Independent reviewer accepted distinct-target common-PRE in
`.pi/sessions/subagent-artifacts/outputs/88cf8e3a-6fc1-46fb-8d6d-9960b290ffd9/review-private-transition-1.md`:
356 checks passed, no blockers. This supersedes that slice's pending verdict only.

Inspection found shared consumers lacked reviewed-revalidation ingestion and
retained revalidation replay still requires private-current equality. Supervisor
approved integrating shared PRE first, keeping shared parent/final acceptance
blocked. Both owner preparations now consume externally pinned fresh accepted
revalidations, validate all nested current prerequisites and existing contracts,
exact wrappers and ownership, require two distinct fresh executions/targets with
identical captured participants, and freeze common native state/build/workspace
plus full prerequisites in the manifest. Run rederives this binding and compares
captured PRE before mutation/gates. Capture scope remains separate from authority.
Legacy reviewed-application preparation remains preparation-only.

Canonical absolute reviewer artifacts exposed a repository-input seed mismatch.
Supervisor approved schema-aware parent validation before excluding only those
reviewer references from repo-relative seeds; arbitrary absolute evidence still
rejects. Nested reviewer evidence remains mandatory at preparation and run entry.
Two real sequential owner fixtures exercise fresh native Ninja checks, shared
preparation, PRE/pin/evidence/drift rejection, actual failed-gate rollback and
successful bounded shared mutation with parent acceptance still rejected. BOF3
payloads/registry and reviewer identities are synthetic, not live domain approval.

Remaining boundary: retained historical revalidation integrity/common PRE replay,
new shared POST/context final validation, distinct fresh shared parent acceptance
and external envelope pin, full nested tamper/drift/rollback matrix and independent
review. No impossible private-current comparison may replace historical validation;
no original proof rewriting. S3.4 remains in progress; S3.5/live pilots remain unready.
No live promotion, report/index recovery, installs/extensions or checkout Git writes.

Validation: 28 shared/sequential/history checks, 106 revalidation/parent checks,
160 owner checks and 66 plan/harness/accounting/shared checks passed in complete
runs below 300 seconds each (358 distinct cases; shared cases overlap). Final
expanded shared cases passed again (2 in 25.65s). Scoped Ruff/format, plan status
and diff whitespace passed; staged paths are empty and raw index remains
`558a9432955f1286a0dd70eff53c283039c6bfc905b00a182322ce47eb540e74`.
Initial fixture candidate/registry setup and recursive reviewer-node handling
failures were repaired; an existing stub proof regression was fixed by preserving
verified target output. Full `just check`/known oversized source audit NOT RUN.


## S3.4 shared POST integration (accepted bounded scope)

Independent review accepted only shared PRE in
`.pi/sessions/subagent-artifacts/outputs/1007202e-7ecf-48f8-8ff9-ec09d8acc2cc/review-shared-transition-0.md`:
358 distinct checks passed in four complete groups below 300 seconds; no blockers.
This supersedes PRE's pending verdict only, not full S3.4 acceptance.

Both owners now replay retained private/revalidation histories against their frozen
common PRE, then validate the shared authorized delta and new current POST/context.
The existing parent review/final replay accepts only the fresh two-target branch,
with new shared execution/reviewer identities, retained fresh reviewer artifact and
external shared envelope pin. Legacy preparation-only branches remain guarded.
Standalone private final verification still rejects intended subsequent drift;
no old proof is rewritten or reconstructed from current files. Existing preparation
contracts, macro exact wrappers, address/ownership restrictions and capture-only
participation remain unchanged. No generic framework or CLI surface was added.

Expanded two-owner integration checks exercise all shared native gate failures,
publication failure and rollback bytes/modes/index, positive publication/new parent
acceptance/read-only replay, every retained nested artifact, historical record
mutation, wrong pins, reused reviewers and unrelated/source/build/mode drift.
Fixtures use actual temporary owner applications and native CMake/Ninja with
synthetic BOF3 exact payloads/registry/reviews and a shared header comment edit;
these are not domain approval or a demonstrated useful extraction. Full S3.4,
live pilots and S3.5 cancellation/resume remain incomplete. Independent acceptance of this bounded branch is recorded below.

Validation: 2 expanded shared checks (55.34s), 26 sequential/history (80.10s),
106 revalidation/parent (182.18s), 160 owner (112.38s) passed in complete bounded
groups. Initial fixture output-path rejection and an inert added receipt field
were corrected to use the canonical evidence directory and actual receipt status.
No live source/map/promotion, report/index recovery, installs/extensions or checkout
Git writes. Full `just check`/known oversized source audit intentionally NOT RUN.
Final plan/harness/accounting group: 64 passed (16.16s); 358 distinct cases across
all groups. Scoped Ruff/format, plan status and diff whitespace passed. Staged
paths remain empty; raw index remains
`558a9432955f1286a0dd70eff53c283039c6bfc905b00a182322ce47eb540e74`.


## S3.4 owner transport and obligation audit (accepted bounded transport)

Independent review accepted shared POST in
`.pi/sessions/subagent-artifacts/outputs/1007202e-7ecf-48f8-8ff9-ec09d8acc2cc/review-shared-transition-1.md`:
358 distinct checks passed in complete bounded groups; no blockers. This supersedes
POST's pending verdict only. Synthetic BOF3 payloads and comment edits prove owner
integration, not useful extraction or live domain acceptance.

| Remaining obligation | Existing evidence / smallest next action and owner |
| --- | --- |
| Function | Historical selected-row acceptance/skip control; BOF3 reviewer must obtain fresh naming postapply gates/review before present closure, never reapply or rebind. Full-target `complete:true` remains required. |
| Local data | Frozen `data:D_80096994` control is BLOCKED per d740aabe below; capability integrity is not semantic exhaustion/no-op. Parent owns bounded read-only bof3-reviewer follow-up; historical report row stays unchanged and FUNCTION-only `naming/postapply*.py` still rejects data application. |
| Fixed RAM/storage and aggregate | Indexed blocked `1F800044` concerns, not ownership/layout proof. BOF3 namer/lifter must corroborate owners/access width/base/extent and layout/semantics before serialized `type-audit` preparation; storage and aggregate remain linked, not competing writes. |
| Types/macros shared positive | Accepted two-private/fresh-check/shared PRE/POST tooling fixtures; BOF3 lanes still owe real corroborated contracts, two private native exact results and useful common body before live shared review. Macro pilot members remain invalid lifts, not exact extraction evidence. |
| Accepted/no-op/blocked accounting | Existing owner account validators and frozen five-entry routing are not final campaign acceptance. Parent must bind every entry to current independent acceptance, justified exhaustion/no-op or explicit blocked owner/reason/next action; blocked entries cannot count done. |
| API/CLI | Existing participant/revalidation owner transport independently accepted at current bytes by b295c173; no before-image provenance or all-CLI shared campaign acceptance. |

Both commands now expose `run --participating-targets` and owner revalidation,
integrity replay, parent review and final replay commands. One private command
helper shares only parsing/transport, not orchestration, state or authority.
Ordered intervening JSON retains full envelopes/external pins; strict duplicate
parsing and native output publication stay owner-controlled. No new domain route,
proof rebinding, recovery, installs/extensions or checkout Git writes. S3.4 stays
in progress; S3.5/native cancellation and S4 remain unready. Current checks and
transport limitations are recorded in this run's implementation artifact.

Validation: 8 CLI cases passed (7.47s); 160 transaction cases passed in the
166-case CLI/owner run (120.70s); 28 shared/history/sequential cases passed
(137.61s); 106 revalidation/parent cases passed (181.36s); 64 plan/harness/accounting
cases passed (14.76s): 366 distinct cases with complete groups below 300s.
Scoped Ruff/format, plan status and diff whitespace passed. Revalidation and fresh
review output collisions preserve existing bytes via native exclusive publication.
Initial fixture absolute-output and exception-class expectations were corrected;
no owner validation was weakened. Raw index remains
`558a9432955f1286a0dd70eff53c283039c6bfc905b00a182322ce47eb540e74`, staged paths
empty. Full `just check`/known source-audit timeout and live/native cancellation
were NOT RUN.

Independent reviewer b295c173 accepted only current-byte CLI transport in
`.pi/sessions/subagent-artifacts/outputs/b295c173-77ad-4bae-b602-3ba902620990/cli-transport-independent-review.md`:
366 existing tests and 26 supplemental assertions passed; no tooling blockers.
No preserved before-images or reconstruction equivalence were verified. Synthetic
owner fixtures are not live domain or all-CLI shared campaign acceptance; S3.4
and all unfinished obligations above remain open in their existing states.

## Frozen data control (BLOCKED; plan update pending semantic review)

Independent read-only review d740aabe in
`.pi/sessions/subagent-artifacts/outputs/d740aabe-6583-40f5-990d-23d7553538eb/data-control-independent-review.md`
validated retained capability integrity and original data/consumer bytes, not
semantic exhaustion or current no-op completion. Handler behavior and independent
corroboration remain unproven; preserve the historical exhausted row and raw name.
Selected transaction CLI rejection reflects proposal-only routing; full-report
validation fails on separate historical FUNCTION binding state, not a new data
defect. No report/receipt repair or rebinding is authorized.

Parent may scope read-only `bof3-reviewer` review of the three retained handler
ranges to establish selector/state effects or a concrete static ceiling. Row-level
`bof3-namer` dispatch is unsupported: it accepts only `audit-target TARGET`;
reuse the existing reviewer route, without widening skills or launching collection.
The review's selector-offset/consumer-end specification correction remains a
separate writer obligation, not authorized here. BLOCKED accounting cannot satisfy
finite campaign closure or full-target `complete:true`. This plan-only recording
still requires independent semantic review; neither domain nor S3/S4 completion
is claimed.

## Bounded index ceiling (tooling pending independent review)

Domain review 707dab39 (`data-list-producer-review.md` in its retained output
directory) accepted conditional uniqueness preservation only, not a global
invariant or independent access to `D_80096994`; the data pilot remains BLOCKED.
No further peripheral gameplay investigation is required for its identity gate.

Owning `domain/mips.py::data_references` deliberately recognizes direct LUI/%lo
pairs within 12 instructions, not materialized-pointer chains or dynamic indexed
bases. Original `800A5424`, `800A5434` and `800A5590` omissions reproduce this
ceiling; exact `801463C4` xrefs remain empty, correctly without guessed aliases.
The fixed table-consumer owner separately proves all three loads in its reviewed
body; the index reports only its table-address materialization at `800AD278`.
Neither establishes a complete inventory of consumers. No propagation defect in
supported behavior was reproduced; runtime extraction/capabilities are unchanged.

The empty-access analyzer already leaves selected access open. Its allowlisted
positive structural capability can still say `exhausted` with a second independent
consumer explicitly missing: this is an unresolved terminal-accounting integration
obligation, not semantic ladder exhaustion/current no-op acceptance. Parent approved
characterization/docs only. Next: independently review this ceiling, then parent
scope the naming conclusion/accounting owner contract to distinguish structural
capability from reviewed terminal acceptance. Keep this pilot blocked, historical
report/receipts untouched, and the full-target `complete:true` gate unchanged;
blocked outcomes prove fail-closed routing, not finite campaign closure.

Validation: 433 reverse-index/facts/table-consumer tests and 148 plan/harness/
conclusion tests passed (two skipped), in groups under 300 seconds. Initial new
query-fixture foreign-key/row-factory setup failures were corrected; no runtime
check changed. Original image/table/consumer assertions passed. No live index or
snapshot regeneration, source/map/report promotion, installs/extensions or Git
writes. Full `just check`/source audit and native cancellation were NOT RUN.

## S3.4 terminal-accounting boundary (accepted bounded scope)

Independent review a4f82975 accepted the bounded index-ceiling characterization in
`.pi/sessions/subagent-artifacts/outputs/a4f82975-a4ff-455a-b8ce-59e8f21e5d0d/review-index-evidence-limit.md`:
581 checks passed, two skipped; no extractor repair required. This supersedes only
that slice's pending verdict. Structural single-consumer exhaustion is not semantic
ladder exhaustion; `data:D_80096994` remains BLOCKED with historical bytes unchanged.

Parent approved a bounded read-only `naming-audit terminal-verify` CLI/API as the
future loop's selected-row acceptance consumer; no existing autonomous consumer
was available to wire. It reuses naming row/capability/receipt and local review
primitives, current owner-derived scope/tooling, external parent digest and separate
retained preparation/semantic reviewer artifacts. Three distinct actual native run
identities are locally parent-attributed; old collection journals contain no run ID.
False ladders, unresolved executable leads, contrary evidence and stale bindings
reject. Evidence-backed static ceiling rationale must address the missing naming
fact; missing corroboration alone cannot prove exhaustion. Structural validation
and historical report/capabilities remain unchanged. Full-report blockers are
reported separately and `complete:true` remains additional mandatory production
acceptance, never replaced by a selected row. No current BOF3 attestation is issued.

Targeted synthetic owner/CLI tests cover accepted reviewed ceiling/replay, structural
only, open leads, contrary reviews, stale pins/row/report/tooling/source/evidence,
self-review, missing artifacts and separate fullreport failure. Synthetic attribution
is not live domain review. Independent review e2920f90 accepted this CLI/API
boundary (605 passed, two skipped):
`.pi/sessions/subagent-artifacts/outputs/e2920f90-54da-4c4c-80de-4c1a3d10466c/review-terminal-accounting.md`.
This does not accept loop accounting or complete S3.4. S3.5/native recovery, S3.2 and S4 remain unready.
No source/map/report/index recovery, receipts, installs/extensions or Git writes.

Validation: 237 naming/conclusion/CLI/plan/harness checks passed, two skipped;
392 terminal/facts/consumer/root/bulk checks passed (30 terminal cases, overlapping
first group). Scoped Ruff/format, plan status and diff whitespace passed. Raw index
remains `558a9432955f1286a0dd70eff53c283039c6bfc905b00a182322ce47eb540e74`,
staged paths empty; historical report retains `bc6c9cabcb1dfea5cbf3746598cf7e966b58742fe92876cf4e4d53cbf5f2a500`.
Full `just check`/known oversized source audit and live/native cancellation NOT RUN.

## S3.4 all-handler fixture sequence (accepted bounded scope)

Two new type/macro cases reuse the existing distinct-target private/common-PRE/
shared fixture and its rejection matrix through actual argparse/CLI handlers:
prepare → participating run → review/final verify for each private application,
ordered fresh revalidation → integrity replay → new parent review/final replay,
then shared prepare/run/review/final verify. Only native-gate runner injection is
substituted; actual CMake/Ninja runs, BOF3 payloads and parent/reviewer identities
are explicitly synthetic. No transport defect reproduced; runtime is unchanged.
This validates the existing supported-class obligation, not a new plan dependency.
Validation: 36 CLI/private/shared cases passed (200.06s); 128 terminal/plan/harness/
agent/skill checks passed (22.46s), plus both skill-script checks. Structural-only
terminal rejection remains covered without issuing a live data attestation.

Current AGENTS requires direct compaction, forbids restoring retired machinery;
`.pi/skills/agent-skill-compaction/SKILL.md` is absent. S1/S2 review 9222db22 above
records accepted compact contracts, 30-file reference inspection and checks; this
run edits no agent/skill Markdown and reran applicable existing checks.

Independent review 11fcb490 accepted this bounded all-handler sequence (164
checks plus both skill-script checks):
`.pi/sessions/subagent-artifacts/outputs/11fcb490-323d-468e-bb89-e31621ec7604/review-remaining-integration-gates.md`.
Handler reachability is not subprocess/Pi execution or positive private replay
at every lifecycle point. All-five accounting acceptance remains distinct. Native S3.5 stop/recovery is NOT RUN and still depends on
accepted S3.4. Live S3.4 prerequisites are unchanged: fresh FUNCTION gates/review,
BLOCKED local-data control, corroborated serialized fixed-RAM/layout ownership,
and real exact private type/macro members/useful shared body. No peripheral data
research, live recovery or prerequisite reclassification is authorized here.
S3.2 cannot encode the production loop before S3.5 acceptance and the retained
full-target naming `complete:true` gate; S4 remains open.

Next: independently review the bounded accounting slice below, then obtain an
explicit whole-S3.4 acceptance/blocker verdict, not infer completion from fixture
passes. Only after whole-S3.4 acceptance, scope an actual native
finite rehearsal: one writer, frozen selectors/baseline/budgets, stop before apply,
during checks, after apply/before review and after acceptance; inspect real native
state, owned bytes/modes/absence and unrelated/index preservation before resume;
accepted work skips, uncertain apply stops for parent recovery. Fixtures do not
prove native cancellation, and no such rehearsal was launched here. Full
`just check`/known oversized source audit remains NOT RUN, not passed.

## S3.4 frozen-five accounting (pending independent review)

Authorized design ea766bcf now has one read-only Python consumer,
`analysis/frozen_queue_accounting.py::account_frozen_queue`, and focused tests.
It requires the exact externally pinned S3.3 five, unchanged source/binary/snapshot/
index/report bytes before and after, current owner queries and direct verifier
calls. Closed proof references retain owner digest spellings and manifest identity;
both naming evidence/receipt contexts restore on success or failure. Storage/layout
positive closure requires the same verified envelope explicitly covering both IDs;
current type concern rules cannot produce that pair, so no positive live closure
is claimed. Macro owner uses `candidate_id`, global-account fingerprint and
repo-path owners; only the frozen three affected functions are permitted.

Live read-only accounting returns five accounted, zero accepted/noop, five blocked,
one overlapping historical FUNCTION skip and one blocked linked pair. Unfiltered
full-report validation independently reports the existing FUNCTION binding-location
blocker; report/receipts remain unchanged. Campaign is incomplete and bounded
`production_complete` is always false. Parent/BOF3 owners still owe fresh FUNCTION
checks/review without reapplication, accepted data ceiling, corroborated fixed-RAM/
layout authority and real exact private members/useful sharing. S3.4 stays in
progress; S3.5, S3.2 and S4 cannot advance from this summary.

Validation: 121 consumer/accounting/plan/harness checks and 40 terminal/owner-CLI
checks passed in complete groups below 300s. Synthetic consumer positives prove
routing/binding only; actual malformed owner proofs reject. One intermediate
relative-path check exposed RHS-before-key evaluation during hash capture; explicit
path validation now precedes hashing, and the entire group passed. Scoped Ruff/
format, plan status and whitespace checks passed. No live application, receipt
publication/rebinding, recovery, installs/extensions or checkout Git writes.
Full `just check`/known oversized source audit and native cancellation NOT RUN.
