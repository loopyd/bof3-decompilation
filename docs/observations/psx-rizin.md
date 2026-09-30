# psx-rizin observations

Per-skill half of the central observation folder ([index](INDEX.md)). The skill carries
**directives**; this file carries the **measurements** behind them. Read it before designing a
mission for this skill, and after each mission add one audit row and converge the finding into a
directive in `.pi/skills/psx-rizin/` (or its agent instructions) - prose alone is not convergence.

## Measured performance

No dedicated `psx-rizin` invocation cohort has yet been established. The following
is a **Rizin-operation case study inside naming/tooling missions**, not three
proven invocations of this skill. Scope and role must remain explicit when these
results guide analyzer references or harness improvements.

| Phase / source run | Calls | Lifecycle seconds | Outcome |
| --- | --- | --- | --- |
| Prompt diagnosis `7271abd9…` (worker) | 4 | 128.483 | Located live command-loop interactivity; no production fix in this phase |
| Handshake implementation `bb5dcc80…` (worker) | 11 | 832.052 | Captures recovered; canonical evidence still refused |
| Independent review `360af7c5…` (reviewer) | 83 | 680.253 | Reproduced byte controls, budget behavior and remaining canonical blocker |

These totals include surrounding naming/tooling work. They are not per-query
latency. Source: complete matching reports and transcripts in
`.pi/sessions/subagent-artifacts`, joined to each run's lifecycle `status.json`
under `/tmp/pi-subagents-uid-1000/async-subagent-runs`; retained scalar snapshots
and hashes are in `tmp/observation-ingest/runtime-metrics.json`.

The diagnosis timed out after **5.05 seconds** with only an **82-byte START
marker**. The same full command, after changing the setting in the live loop,
returned **221,901 bytes in 0.049 seconds**, matching the retained standalone
control. Implementation measurements separated handshake from capture:

| Capture | Set/read-back seconds | Capture seconds | Output bytes |
| --- | --- | --- | --- |
| Battle owner | 0.46954 | 0.00348 | 11,588 |
| Shop owner | 0.29526 | 0.04344 | 221,901 |

This two-command sample includes engine readiness in the initial handshake.
It cannot establish a steady-state latency distribution. Independent review
re-captured the shop result in **2.194 seconds end-to-end** with the same hash;
do not compare that measure directly with capture-only time.

## Parent diagnostic cost and comparison limits

The [apply parent episode](openspec-apply-change.md#capability-implementation-capture-diagnosis-timeout-and-recovery),
records 1399–1455, incurred **29 calls / 496.850 seconds** before the resumed
diagnosis above returned. This is shared orchestration cost, not a dedicated
`psx-rizin` invocation or query-latency sample. Its standalone 2.09-second capture
and 134.157954-second failed canonical run used different measurement scopes;
the apparent ratio is not a proved transport speedup. The parent also mistook
`-c` configuration on a malloc buffer for live-loop state and prohibited the
proposed setting before comparing equivalent modes. Two invalid probe forms and
clipped config/output searches added friction without establishing that prohibition.
Use this measured failure case to evaluate D13-style mode-aware guidance: count
invalid comparisons and corrective handoffs per comparable diagnosis, while
requiring complete output, identical bytes and preserved deadlines for acceptance.

### Boundary inference after transport recovery

Parent records 1456–1508, measured in the
[apply ledger](openspec-apply-change.md#capability-implementation-handshake-supervision-and-premature-range-conclusion),
add **23 calls / 491.850 seconds** of shared supervision. They show why the reviewed
4,392-byte extent must remain distinct from the analyzer's 164-byte estimate. The
parent read only forty-five lines of a diagnostic JSON, treated the smaller size
as the true boundary and prescribed shortening; the first invalid record at
`0x801E51E8` was insufficient evidence to classify the entire remainder as data.
It also wrote `0x801E52AC` as the requested end, whereas the artifact's decimal end
and `0x801E5144 + 4392` both yield **`0x801E626C`**. A schema pass did not check this
arithmetic or authorize a boundary change. Evaluate analyzer-reference improvements
against this concrete failure: retain competing extents and their authorities,
calculate addresses mechanically, and require original evidence before promotion.

## Editorial provenance and source gaps

The [editorial audit](bof3-docs.md#rizin-and-type-editorial-child-review) fully
reviews both ledger rewrites: seven calls/274.734 summed lifecycle seconds, with
no analyzer execution. The retained twenty legacy bullets remain summaries of
mixed prompts/reports; their original missions are still pending. In particular,
the merged 14/35 missing-start count and nine interior/two delay-slot starts do
not establish a shared population or an analyzer error rate.

The first rewrite also dropped two qualitative leads: a caller's byte mask alone
does not establish a callee's return declaration, and contiguity guaranteed by a
construction rule is not independent evidence that every start is reachable.
Their underlying examples need source reconciliation before becoming operational
directives. Retain them as evidence questions rather than discarding them for
lacking a numerical delta. Reference improvements should distinguish authority,
population and independently tested properties before interpreting analyzer counts.

## Iteration audit log

| Iteration | Mission | Tools | Duration | Method verdict | Converged into |
| --- | --- | --- | --- | --- | --- |
| 2026-09-26 history review | Diagnosis, repair and independent analyzer review | 4 / 11 / 83 | 128.483 / 832.052 / 680.253 s | Transport recovered; semantic acceptance remained blocked | Operational lessons and proposals below; no new source changes |

Add one row per mission. A row that changes nothing is itself a finding.

## Converged directives

None recorded yet. Directives for this skill live in `.pi/skills/psx-rizin/SKILL.md` and its references;
record the pointer here when one is added.

## Analyzer-specific evidence rules (absorbed from the legacy `LESSONS.md`)

Cherry-picked from `LESSONS.md` Level 5 — this is the ledger for the skill that owns Rizin work.

- **Rizin data exclusion**: declare it with `Cd <size> @ <addr>` in the target's `reviewed.rz`.
  `af-` does not survive replay because `aa` re-creates it, so an exclusion that is not written into
  `reviewed.rz` is not durable evidence.

## Bundled harness improvements

| Measured evidence | Proposed owner/change | Acceptance measurement |
| --- | --- | --- |
| Startup probe said noninteractive; live command loop was interactive | Rizin operation reference: reproduce configuration in the actual invocation mode and record framing diagnostics | Same command returns complete, byte-identical output in the live loop; no confirmation consumes command/frame input |
| Set/read-back dominates capture-only latency but costs under half a second in two controls | Harness profiling: separate startup, verified configuration, capture and validation costs before optimizing transport | Any optimization retains per-command verified state, identical bytes, refusal on unknown/true responses and the same total deadline |
| 290/1,098 captured records violate 4-byte instruction validation | Boundary-analysis guidance: distinguish transport success from reviewed code/data extent | Classify the reviewed range from original evidence; no truncation to the analyzer's 164-byte guess and no weakened size/byte check |

## Important observations

Historical findings, including the source-joined operation study above; current
skill contracts still govern new work.

- An interactive confirmation consumed the leading `e` of the following `echo`
  frame marker, producing `Command 'cho' does not exist`. Reading faster in Python
  could not fix output withheld by the child. Probe the child mode before blaming
  the parent read loop or enlarging timeouts (diagnosis `7271abd9…`).
- Successful capture is not valid instruction evidence. The recovered shop range
  stayed 4,392 reviewed bytes/1,098 records; 290 records had size 1. Both writer
  and reviewer observed canonical refusal with zero committed rows, so neither
  handshake success nor a smaller analyzer extent justified declaring acceptance
  or shortening the boundary (`bb5dcc80…`, `360af7c5…`).
- Independent budget measurement spent 0.5 seconds on each handshake leg, then
  killed the evidence operation at **2.06 seconds** under a shared 2-second budget.
  Per-leg budget renewal would have changed the contract. Read-back `true` or
  empty produced refusal after SET/READ with no evidence command sent (reviewer
  `360af7c5…`; these were manual probes, not existing regression coverage).

- Indirect dispatch through progressHandlerTable4 at 0x801F269C, indexed by signed fairyProgress[0], produced a 15-instruction leaf with no analyzer edges.
- emi/battle/battle/15@0x800A8360 retained an equal-size 31/38 (81.58%) candidate with D_80148570 address-materialization/base-lifetime differences; GCC 2.8.1 peaked at 92.11%. Compiler/permuter searches remained non-exact or failed; stale reverse indexing blocked focused Rizin analysis.
- emi/battle/battle/15@0x80097FE8 retained an unpinned 32/49 (65.31%) candidate, 192→196 bytes, with entry allocation a0 versus v1 and scheduling differences. Removing volatile regressed it; extern binding improved relocation without exactness.
- emi/etc/shop/00@0x801DAB90 restored 182/339 (53.69%) with a pointer-cell extern/binding and an assembly-proven 8-byte, four-u16 primitive advanced by 8 bytes. Rizin established a five-argument ABI and stack argument/copy evidence; Splat edits made subsequent queries stale.
- emi/etc/game/00@0x801ADC98 measured 18/25 (72%), 92→100 bytes, with subu register/lifetime, move, sign-extension and branch-scheduling differences. Compiler comparisons peaked at 72% for canonical/2.8.x, 48% for 2.6.3 and 34.62% for 2.95.2; none was exact.
- emi/world00/area030/04@0x801DAE3C measured 16/19 (84.21%), 72→76 bytes; at +0xC the original bnez delay slot held li v0,64, while the candidate used nop and loaded the constant after a scratch load.
- emi/etc/shop/00@0x801E2724 measured 21/39 (53.85%) and 25/39, both 152→156 bytes; the latter delayed the entry panel-base lui. The leaf boundary was 0x801E2724..0x801E27BC, with one caller at 0x801E23A4 and no callees. A fresh focused snapshot still encountered an interactive Rizin endianness failure.
- emi/etc/game/00@0x8019AAFC measured 21/24 at an equal 96 bytes; the first mismatch at +0x10 was load/store scheduling and register allocation.
- emi/battle/battle/15@0x800AF720 was reported as normalized 16/17, but live comparison measured 1/17 (5.88%) at an equal 68 bytes: entry move t0,a2 became move t0,zero, with broader allocation differences.
- emi/battle/battle/15@0x8009B160 regressed from 42/43 (97.67%) to 35/43 (81.40%) at an equal 172 bytes when entry allocation changed from li a0,1/work pointer a1 to li a2,1/work pointer a0. The final 42/43 candidate retained one REGISTER_PIN(a0) and a +0x90 load-order mismatch; compiler/permuter searches remained non-exact and stale indexing blocked focused Rizin analysis.
- emi/etc/game/00@0x8019625C measured 86/174 (49.43%), 704→668 bytes, after a signed-bound correction; the first mismatch at +0x14 was original ori s1,s1,0x7800 versus candidate addiu.
- emi/world00/area027/13@0x801F3650 used a grouped switch emitting slti/beqz/bgez and measured 10/16, 64→60 bytes; control-flow shape and size remained non-exact.
- emi/etc/game/00@0x801B2D00 measured 12/21 (57.14%) at an equal 84 bytes using a separate s32, versus 52.38% with direct u8. At +0x14, original sltu v0,v1,a0 became andi a1,v1,0xff; an if/else variant shrank to 80 bytes and was reverted. Fresh Rizin evidence found one caller, 0x801BEDD0, and no calls.
- An exe/logo audit inventoried 7/7 target-local raw functions, all unresolved; rz-project open failed before mandatory original-byte/caller/callee analysis, so no rename passed its evidence gate.
- All 249/249 Battle 15 rows remained initializer-blocked despite 127 having digest-bound collected evidence and seven having reviewed exact capabilities. Zero conclusions were enabled because 23 unrelated access records remained unavailable; the analyzer required zero open and zero unavailable evidence.
- emi/scenario/scena00/00@0x801F85A8 was reported exact at 100% without matching aids after carving a 416-byte function from header_tail_after_801F82B4.
- func_8009F814 was counted by the analyzer as nine instructions/36 bytes, but its actual boundary contained ten instructions/40 bytes. Disassembly and sibling func_8009F7EC_battle15.c showed the same wrapper shape with kind immediate 0x3 instead of 0x1.
- emi/scenario/scena00/00@0x801F8BCC had a Rizin-confirmed boundary of 0x801F8BCC..0x801F8D6F, 105 instructions/0x1A4 bytes, with the next prologue at 0x801F8D70. A 102/105 baseline yielded two exact shape-sweep variants and a passing lift gate.
- For the target containing 0x801FA7D8, all 16 analyzer functions were claimed and next-lift returned no candidate. The claimed partial measured 422/485 (87.01%) with exact size, table-entry shifts of [5]+4 and [9]-4, and a frame 16 bytes larger.
- One legacy excerpt reports no Rizin functions at 14 of 35 declared starts. Another reports that replacing analyzer start 0 preserved counts but produced nine interior starts and two delay-slot starts. Their population relationship is unverified; neither declaration counts nor the merged bullet establish independent boundary corroboration.
