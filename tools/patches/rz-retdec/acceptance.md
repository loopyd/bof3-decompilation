# PSX RetDec acceptance status

The customization is accepted for isolated delivery with the limitations below.
The cumulative patch is forward-verified and independently accepted for clean
build, bounded replay, source-only reversal, and final packaging. This is not
production integration or a claim of universal matching.

## Reproduced baseline

A separate source assembly and fresh CMake build succeeded. Configuration with
compiler-derived `CMAKE_LIBRARY_PATH` and `LIBRARY_PATH` finds the existing GCC 13
filesystem archive. The rebuilt plugin links with `-lstdc++fs`.

Replaying the existing generation commands against this plugin produced identical
C for all 125 broad cases and all six metadata-assisted functions. Fresh cache
directories prevent reuse of the original plugin's generated output.

| Retained evidence | Count | Meaning |
| --- | ---: | --- |
| Broad selected cases | 125 | Frozen selection, including excluded references |
| Broad admitted references | 118 | Seven reference-boundary exclusions remain |
| Broad native comparisons | 117 | Baseline excludes blocked case 053; task 2.2 adds its comparison |
| Broad byte-exact results | 10 | Prior complete-function and placement comparisons |
| Assisted byte-exact results | 5 of 6 | Separate metadata-assisted corpus |
| Fresh broad output identities | 125 of 125 | Rebuilt plugin output equals baseline |
| Fresh assisted output identities | 6 of 6 | Rebuilt plugin output equals baseline |

Native and runtime results in this table are retained evidence, not fresh native
or runtime acceptance of the rebuilt plugin. The assisted counter remains 15/24
instructions. Cases 038, 044, and 055 have unaligned translation and link, but
still lack complete function-specific runtime validation.

## Historical score dispositions

Cases 016 and 041 retain unresolved historical instruction-score decreases:
14.08 to 13.24 percent and 31.08 to 22.17 percent. They remain in the comparison
inventory. Higher scores in diagnostic variants do not establish semantic
correctness. In particular, incorrect unsigned-division emission cannot justify
selection or removal of the retained unsigned fix.

The frozen per-function classification covers all 118 admitted references, the
six assisted functions, and all seven exclusions. It protects exact matches and
bounded semantic results separately from score-only comparisons.

## Evidence and remaining work

- `tmp/retdec-psx-preservation/manifest.json`: recoverable starting sources and
  4,297 rechecked production pins.
- `tmp/retdec-psx-preservation/corpus/inventory.json`: 1,571 frozen artifacts and
  verified original-input identities.
- `tmp/retdec-psx-preservation/history/classification.json`: per-function baseline
  and historical-score dispositions.
- `out/retdec-psx/environment-build.json`: successful configuration and build.
- `out/retdec-psx/baseline-replay/results.json`: rebuilt-plugin output identities.

## Task 2.2 implementation and validation

The isolated decoder now derives bounded leaf-call write masks from original
instructions and uses them in path-sensitive interval analysis of decoder IR.
The eligibility helper summary covers 42 instructions and writes only `at`, `v0`,
and `v1`. Synthetic callee bodies supply no evidence. The analysis proves both
zero-divisor guards safe. Existing later optimization removes the overflow guards.
Case 053 now emits without exception dependencies and compiles and links under
its unchanged original profile. It is not byte-exact: 55/295 instructions match,
with 1,180 compiled bytes versus 672 original bytes. Placement checks pass.
This is not full function-specific runtime validation.

Fresh replay preserves the other 124 broad outputs and all six assisted outputs
identically. Existing division/exception host checks pass 10,195 assertions at
each of O0 and O2 with undefined-behavior sanitization. All six guest objects
compile, and the existing normal/taken/conditional BREAK delay checks pass.
Native Redux raw arithmetic runs pass 324 comparisons each for original and
regenerated code. All six original/regenerated exception pairs pass their existing
state assertions, covering zero divisors, signed overflow, normal BREAK, taken
branch delay BREAK, and both outcomes of conditional branch delay BREAK.
All 14 capture receipts report `passed`. The replay took 240.5 seconds.

The first native exception replay failed because its runtime fixture used
`uint32_t` without including `<stdint.h>`. With explicit user approval, the
disposable generator now adds that include only to the runtime fixture.
Original scripts, decompiler-emitted C, exception headers, assertions and
production files are unchanged. The failed run remains intact. A fresh replay
under `guard-proof-division-02/` passes, and hashes confirm the six copied
emitted division sources are unchanged. Task 2.2 is complete; this bounded
exception evidence does not establish full case-053 runtime equivalence.

Evidence: `out/retdec-psx/guard-proof-replay/results.json`, its
`broad/case-053/comparison.json`, `guard-proof-division/host-commands.json`,
`guard-proof-division/runtime-commands.json`, and `guard-proof-runtime.log`
retain the first attempt. The completed replay is in
`guard-proof-division-02/runtime-results.json`, `runtime-commands.json`,
`source-pins.json`, its 14 capture receipts, and `guard-proof-runtime-02.log`.
Reproduction drivers are `tmp/retdec-psx-verify-guards.py` and
`tmp/retdec-psx-runtime-guards.py`; they reuse existing cases and assertions with
isolated paths. Clangd uses the game C profile for these external C++ files and
reports missing C++/LLVM headers; the actual CMake build passes.

## Task 3.1 provenance

A store-attached record captures the counter load/add/store/compare instruction
origins, 32-bit increment, 8-bit truncation, increment of one, original threshold
of 16, and equality predicate before LLVM optimization. At source-recovery entry,
the validator checks the surviving load, byte increment, store, comparison,
original instruction identities and ordering. It removes unsupported records.
The annotation grants neither memory reordering nor volatile qualification.

Pass-boundary IR under `out/retdec-psx/counter-trace-provenance-02/` shows capture
at pass 23, survival through pass 139, and validation on the store and comparison
at pass 141. A disposable inspection of retained IR confirms that changing the
stored value removes the record instead of validating it. `inspection.json`
records both outcomes. All 125 broad and six assisted outputs remain identical
to task 2.2 in `out/retdec-psx/provenance-replay/results.json`. CMake build passes.

## Task 3.2 input-contract decision

The frozen assisted counter command supplies only `D_80145FAA`; its cached
configuration contains no `g_battle_work` global. The declaration in
`include/bof3/battle/battle15_internal.h` is `u8* volatile g_battle_work`, a
volatile pointer cell, not volatile pointee bytes. Address-only inference is
forbidden, and silently adding metadata would change the frozen comparison.
The user approved a separately labeled diagnostic invocation with this declaration,
while retaining both frozen corpora unchanged. The diagnostic supplied
`avga g_battle_work "unsigned char * volatile"` with a fresh cache. Rizin exited
successfully, but RetDec received `cType: "unsigned char *"` without `volatile`.
Generated C remained identical to the retained counter output.

`out/retdec-psx/volatile-input-diagnostic-01/result.json` records the exact command,
input and plugin hashes, received global metadata, and source comparison. The
existing metadata route cannot carry the required qualification. The user subsequently
approved an opt-in plugin-owned declaration input without changing installed Rizin.
`design.md` in the OpenSpec change records its scope.

## Task 3.2 implementation and validation

Task 3.2 is complete in the isolated source tree. `DEC_OBJECT_DECLARATIONS`
accepts explicit global declarations through the existing configuration format.
Validation rejects unsupported types, malformed pointer spellings, conflicting
identities, and addresses outside the target address width. The input is read
once for both configuration parsing and document validation. Rejection applies
none of the supplied declarations; an unset input preserves existing behavior.

Object qualification survives configuration rewrites, marks resolved LLVM
loads and stores before destructive optimization, and reaches BIR and C emission.
The diagnostic emits `extern uint8_t * volatile g_battle_work;` with two 32-bit
pointer-cell reads. Pointee accesses remain unqualified and the entry ABI is
unchanged. The retained compiler profile exits successfully with empty stderr.
The linked object leaves `g_battle_work` undefined rather than allocating storage;
its two cell-load instruction pairs match the original function.

All evidence below is under `out/retdec-psx/task-3-2/` and binds final plugin
SHA-256 `e170091f9aefbf670faab19c16c1f2f7c74b90a20cf45cd2cb25d6373afda55a`.

| Evidence | Verified result |
| --- | --- |
| `replay-05/results.json` | All 125 broad and six assisted outputs equal task 3.1, with no failed invocation. Only case 053 differs from the original evaluation, as recorded for task 2.2. |
| `diagnostic-04/result.json`, `compile-03/result.json`, `link-03/result.json` | Qualified declaration, successful compilation without diagnostics, declaration-only linkage, and original cell-load instructions. |
| `boundary-03/results.json`, `probe-02/results.json` | One qualified case, two inert cases, and 16 rejected boundary cases. Additional probes confirm malformed spelling and out-of-range address rejection. |
| `bof3-runtime-05.json` and both profile capture directories | Six BOF3 functions pass 83,984 comparisons at each of O0 and O2, with zero mismatches. Capture and input hashes match. |
| `runtime-02/` | Separate division fixtures pass 10,195 host assertions at each optimization level, 324 raw-register comparisons each for original and regenerated code, six exception pairs, and the retained BREAK delay checks. All 14 capture receipts pass. |

Reproduction uses `tmp/retdec-psx-task-3-2/` scripts: `task-3-2-build.py`,
`task-3-2-replay.py 05`, `task-3-2-diagnostic.py 04` with the retained
`inputs/g-battle-work.json` declaration, `task-3-2-boundary.py 03`,
`task-3-2-compile.py 03` with `diagnostic-04/emitted.c`, `task-3-2-link.py 03`
with `compile-03/candidate.o`, `task-3-2-bof3-runtime.py 05`, and
`task-3-2-runtime.py 02`. Use new output labels for reruns; preserve prior artifacts.
The task-only patch and 14-file source inventory are retained under
`tmp/retdec-psx-task-3-2/`. Live source hashes match the inventory.
`closeout-01/whitespace.json` checks the actual pre-edit and changed source copies,
not a repository diff that would omit the untracked isolated tree.

### Evidence limits and review disposition

The runtime checks above do not execute the separately qualified emission.
They validate current ordinary output against original bytes with existing
fixtures. Qualified-output runtime and counter recovery remain tasks 4.1 and 4.3.
LLVM marking covers accesses resolved to the declared global at
`retdec-constants`, through casts or zero-offset indexing. The BIR lookup silently
skips a missing name. An accepted but unused declaration therefore does not prove
qualification reached emission; no general claim of loss-free propagation is made.

The retained runtime script's PATH prefix selected a different CMake path and
failed the emulator dependency-identity check. The disposable invocation removed
only that prefix; the check and all assertions still ran. The failed invocation
is `bof3-runtime-04-O0/capture-invocation.json`; successful captures are under
`bof3-runtime-05-O0/` and `bof3-runtime-05-O2/`. The environment record's citation
to `emulator-probe-01/prefixed` has no saved artifact and is not evidence.
Emulator and setup-receipt hashes still match their recorded pins. No setup,
installation, dependency change, or retained-driver edit was needed.

Independent review accepted the task with reporting and coverage notes. Direct
probes refuted its earlier identity-loss prediction: global creation normalizes
`realName` and `isFromDebug` from the supplied name. The retained compiler also
produced no predicted signedness diagnostic. The actual cache identities are
`312b98fe39317543` qualified and `fd8315d26be163e6` unqualified. Inert boundary
cases have no error message; only rejected inputs require one. The final task
patch has 607 added and five removed lines across 14 files; comment-only counts
in the worker report are not acceptance evidence.

The worker's automatic no-staged-files check remains failed because the repository
already contained 160 staged paths. `closeout-01/staging.json` reports no staged
task paths. Those unrelated changes were preserved; the failed automation check
is not represented as passing.

## Task 3.3 arithmetic and memory preservation

Task 3.3 is complete for the retained bounded cases. No semantic repair was needed.
Fresh output uses the same `e170091f...55a` plugin identified in task 3.2. All 14
source inventory hashes still match. Evidence is under
`out/retdec-psx/task-3-3/` unless otherwise stated.

| Evidence | Checked domain and result |
| --- | --- |
| `shift-02/` | Nine ordinary-C runs. Three left-shift profiles each complete groups of 256, 16,384, and 256 comparisons; six right-shift profiles each complete 256, 8,192, and 256. All mismatch and ABI/memory masks are zero. Magic and completion markers pass. |
| `unaligned-01/` | All 28 retained little-endian merge probes pass original/generated return and final-memory-word checks, including immediate and committed LWL/LWR, partial SWL/SWR, paired operations, and pending-load cancellation. |
| `delayslot-01/zero_dest/` | Captured magic is correct, both returns are 7, and final memory is 99. This fixture has no guard region. |
| Other three `delayslot-01/` cases | Taken branch, untaken branch, and jump load-delay boundaries retain their unsupported diagnostic and empty C. The resulting undefined-`probe` link failures are expected rejection evidence, not supported execution. |
| Task 3.2 `runtime-02/` | Reused division evidence remains bound to the current plugin. Six emitted sources were rehashed against the pinned copies in `out/retdec-psx/guard-proof-division/`. Both raw-register captures and all six original/generated exception pairs pass their decoded predicates. |

The shift drivers compare return values, ABI state, a 1,024-byte memory region,
and `STATE`. Merge probes instead establish their specified return/readback and
final-word behavior. The earlier claim that `zero_dest` preserved guard bytes is
retracted. These probes do not establish general guard-region preservation,
cross-word unaligned addressing, MMIO, or emulator-internal pending-load fidelity.
The three unsupported cases have fresh rejection evidence, not fresh original-only
runtime captures. Explicit-state frontend execution and full case-053 equivalence
are not claimed.

Shift containers use fresh per-function decompilations with the retained symbol
renaming and concatenation convention. Their builder script was reconstructed
because the old container builder was not retained. Despite later writer/cast
changes in the emitted text, all nine fresh guest executables match their retained
hashes. All 28 merge candidates and executables also match retained bytes.

### Raw-evidence verification and recovery limits

The initial verifier trusted summary fields and did not establish complete counts
or final memory values. Independent review blocked acceptance. The replacement
`tmp/retdec-psx-task-3-3/raw-verify.py` decodes captured RAM, checks case membership,
magic/completion markers, exact shift counts, mismatch fields, return values and
final memory words. Its bounded merge evaluator derives expectations from retained
probe assembly and initial memory. Division checks rederive arithmetic and exception
expectations. It rehashes 104 captured files and 208 receipt input bindings across
52 passed receipts, including executable identity.

`python3 tmp/retdec-psx-task-3-3/raw-verify-run.py parent-05` exits 0 with all 942
checks passing. `raw-verification-parent-05/process.json` records the actual child
exit and output separately; `checks.json` retains predicates, and `success.json`
is written only after they pass. The verifier hash is
`082a27c1231db08115980169c2e5a1503c74d5fcef47e8c80f45ec732475840d`.
Independent re-review closed the evidence findings before this parent rerun.
This is verification of this inventory, not a general admission tool or a complete
source-to-binary rebuild attestation.

Reproduction scripts under `tmp/retdec-psx-task-3-3/` are
`shift-candidates.py 02`, `shift-runtime.py 02`,
`memory-probes.py unaligned 01`, and `memory-probes.py delayslot 01`.
Use fresh labels and retain prior attempts. The direct raw-verifier process record
closes the earlier piped-shell exit-status gap without rerunning the emulator.
The failed raw-verifier attempts remain under `raw-verification-01/` and
`raw-verification-02/`; the latter used the wrong retained division-source root.

The first worker timed out before its handoff. Four scratch/partial directories
had already been deleted without a pre-deletion inventory. Their contents and
lack of unique evidence cannot be independently audited. The aborted shift build
remains under `shift-aborted-01/`; later failures were preserved. This audit limit
is not represented as verified preservation and does not invalidate the surviving,
rehashed successful captures.

## Task 4.1 counter recovery

Task 4.1 is complete in the isolated source tree. Shared live-relation recognition
in `counter_provenance.h` supplies the comparison recovery invoked from
`source_recovery.cpp`. It uses the existing wrapping stored byte and recorded
threshold, retaining the current predicate, CFG, and memory instructions. Both
consumed metadata attachments are cleared. No layout, address-specific rule, or
fresh load is invented. Repeated-use safety was reviewed in source; no new direct
idempotence fixture was added.

Final plugin SHA-256:
`0239663f8ce9af45d2ffe8e62bad493ca7b457c77fbc1ccee2acdc84e9f53c7d`.
`out/retdec-psx/task-4-1/build-02/build.json` binds the two source hashes and build.
The prior plugin and source snapshots remain under
`tmp/retdec-psx-task-4-1/backup/`.

| Invocation | Native result | Runtime result |
| --- | --- | --- |
| Frozen ordinary assisted counter | 20/24 instructions, improved from 15/24; complete 96-byte extent, not exact | O0 and O2 each pass 6,144 counter comparisons in the 83,984-comparison six-function corpus |
| Separately qualified counter | 24/24 instructions; complete 96-byte function is byte-exact | O0 and O2 pass the same retained domain |
| Other five ordinary assisted functions | All remain exact, including the required `func_8009CF80` `.rodata` placement | Both six-function runtime profiles pass |

Ordinary residual differences concern pointer-cell address materialization.
Qualified exactness does not replace the frozen ordinary disposition. The
user-approved disposable runtime adapter adds only
`--defsym=g_battle_work=0x1F800044` to the linker command; the retained builder,
fixtures and assertions remain unchanged. The supplied type remains a volatile
byte-pointer cell, not a structure or volatile pointee. Runtime observes final
object memory and call-log state over all 256 counter bytes, six word values and
four halfword values; it does not trace individual accesses or mutate the pointer
cell between reads.

Evidence under `out/retdec-psx/task-4-1/`: `diagnostic-02`, `compile-02`,
`link-02`, `compare-03` (six ordinary functions), `compare-04` (qualified counter),
and `bof3-runtime-05-{O0,O2}` / `bof3-runtime-06-{O0,O2}`. Reproduction scripts
are under `tmp/retdec-psx-task-4-1/`; the implementation report records their full
arguments. Frozen `replay-02` generated all 125 broad and six assisted cases
without invocation errors. Its exit **1** remains an output-identity failure:
only the assisted counter changed. Every broad normalized source and the other
five assisted sources are unchanged. This is source-identity evidence, not a
fresh broad native sweep or new broad runtime coverage.

Independent review accepted the implementation with two disposable-verifier
notes: `task-4-1-raw-check.py` incompletely aggregates failed predicates and lacks
executable receipt binding; `task-4-1-counter-check.py` selects earlier native
objects. Neither script's success alone establishes final acceptance.
Parent commands close those gaps for this candidate:

- `python3 tmp/retdec-psx-task-4-1/parent-review-verify.py parent-review-01`:
  538 predicates, exit 0; live source/plugin hashes, actual replay membership and
  normalized sources, four raw RAM captures, completion/counts/mismatches, capture
  and receipt-input hashes, executable bindings, and unchanged fixture identities.
- `python3 tmp/retdec-psx-task-4-1/parent-native-verify.py parent-native-02`:
  63 predicates, exit 0; final symbol-bounded bytes/extents and instruction scores,
  original overlay bytes, required placement, and earlier/final counter-byte identity.

Each directory retains `checks.json` and a separately captured `process.json`.
`parent-native-01` is retained as failed: its expected qualified membership
incorrectly included all six functions, corrected to the one qualified counter.
Earlier failed runtime adapter and shape-check attempts are also retained. The
full independent review was recovered from its transcript into
`tmp/retdec-psx-task-4-1/recovered-review.md` after a late acknowledgement replaced
the report output with a short verdict.

## Tasks 4.2–4.3 baseline retention and assisted verification

Task 4.2 retains the deterministic task-4.1 rule set: **zero admitted or scored
alternatives; two rejected proposals**. The original 15/24 result remains historical
evidence, not a second selected output. The ordinary counter remains at 20/24
instruction-sequence agreement over 96 bytes, not byte-exact. The metric uses
`SequenceMatcher`; it is not a count of equal words at corresponding offsets.
The preliminary assessment's “four slots differ” and changed-halfword-store claims
are withdrawn. The retained diff leaves `sh v1,48(a0)` unchanged and shows address
materialization/reuse and consequent instruction displacement.

| Proposal | Disposition before scoring |
| --- | --- |
| Promote constant-address accesses to external objects | Rejected: the frozen ordinary invocation supplies no declaration evidence for that identity/type/qualification; adding bindings changes its inputs. No new candidate was run. |
| Force different address materialization with unsupported volatility, opaque expressions, or clobbers | Rejected: no admissible provenance-derived form was established; address-derived volatility and artificial clobbers violate the contracts. This proposal group is not a proven evaluated source alternative. No new candidate was run. |

These rejections do not prove that every permissible source form is impossible.
Baseline retention follows insufficient evidence, not an exhaustive search or a
predicted matching gain. The separately qualified byte-exact result remains a
different invocation, never a substitute for the ordinary result. No source change
followed task 4.1; historical cases 016/041 and case-053 dispositions are unchanged.

Task 4.3 is complete using final-build task-4.1 `replay-02/assisted`, `compare-03`,
`bof3-runtime-05-{O0,O2}`, and the parent verification records above: five baseline
exact functions preserved, counter improved, required placement preserved, and
both retained runtime profiles passed. Qualified `compare-04` and runtime `06`
provide separate additional evidence. These are reviewed existing runs, not new
runs performed by the alternatives reviewer.

### Remaining gate execution constraints

Task 4.4 requires actual fresh compilation of the frozen broad rows with their
recorded profiles, complete-symbol/placement comparison, unchanged exclusions,
and explicit case-053 status. Historical evaluator object copies are not fresh
compilation evidence. Task 4.5 requires the retained 15 synthetic cases and
applicable arithmetic/raw checks from the final plugin.

Historical task-3.3 commands are templates, not ready final-build commands. Use
fresh labels, regenerate shift candidates before running their runtime driver,
and bind verification to the final plugin and new artifact paths. The old
`raw-verify.py` pins `e170091f...` and old directories; changing only its wrapper
label does not replay the final build. Parameterization must preserve every
predicate and fixture; a required harness behavior change remains a scope blocker.

## Task 4.4 broad native verification

Task 4.4 is complete for plugin `0239663f...53c7d`. Generation reuses hash-bound
final-plugin task-4.1 `replay-02`; it was not rerun here. All 125 generation commands
retain the frozen inputs, signatures, and analysis extents with only the plugin
path replaced. Fresh compilation uses the frozen profiles for all 118 admitted
references, redirecting only source/object paths. The seven excluded references
remain `029, 032, 033, 035, 052, 072, 107`, with unchanged reasons.

`python3 tmp/retdec-psx-task-4-4/task-4-4-run.py 05` produces `verify-05/` under
`out/retdec-psx/task-4-4/`. Each admitted object was compiled in a new directory
through the inspected compiler execution path, with recorded successful subprocess
exits. Zero objects are byte-identical to their historical counterparts; that hash
observation alone is not proof of fresh compilation.

All **117 historically compared functions** reproduce their frozen per-function
outcomes and linked bytes. Ten remain byte-exact: `001, 006, 009, 023, 024, 112,
117, 119, 123, 125`. Case-053 separately reproduces the accepted task-2.2 result:
55/295 instructions, 672 original bytes versus 1,180 generated bytes, satisfied
placement, no exact match, and no undefined `retdec_mips_exception` reference.
Its generated bytes match the accepted guard-proof artifact. The preliminary
report's claim that all 118 equal the original frozen outcomes is corrected here:
case-053's original frozen entry was blocked.

Full actual ELF symbol extents, addresses, and required section contents/VMAs were
checked. Seven non-exact cases retain unsatisfied `.rodata` placements:
`026, 031, 034, 037, 046, 051, 062`. These are pre-existing limitations, not passes.
Cases 016 and 041 retain 13.24% and 22.17% agreement; their higher historical
score-only variants remain unresolved and unselected. No new broad function-specific
runtime evidence or full case-053 runtime equivalence is claimed.

Independent review found incomplete assertions in the disposable verifier and an
unconditional crosscheck exit. Their success booleans alone are insufficient.
Parent audits close the gaps for this inventory:

- `python3 tmp/retdec-psx-task-4-4/parent-audit.py parent-review-01`: 5,797
  predicates, exit 0. Rehashes all 1,571 frozen inventory artifacts, history evidence,
  live plugin/source pins, candidate inputs and objects; verifies compiler commands,
  actual symbol-bounded bytes, scores, manifest placements, section VMAs, and
  case-053's accepted bytes and successful `nm` invocation.
- `python3 tmp/retdec-psx-task-4-4/parent-generation-audit.py parent-generation-01`:
  627 predicates, exit 0. Verifies every generation command against frozen commands,
  replay metadata binding, and the protected exact set.

Both directories retain `checks.json` and separate `process.json` exit receipts.
Raw `pseudo.stdout` hashes, normalized emitted-text hashes, and adapted compiler
input hashes are distinct and were checked separately. Before later gate reuse,
the disposable scripts must enforce original-byte equality, meaningful independent
extent checks, crosscheck failures, and `nm` exit status; do not reuse their
current success booleans as acceptance.

Task 4.5 evidence is accepted with the limitations below. Patch assembly,
clean patch replay, and reversal remain unfinished. Local evidence paths do not replace durable final
delivery. No production lift, installed tool, compiler profile, or private input
was changed.

## Task 4.5 emitted-C review and bounded replay

Status: complete with notes. The initial worker report omitted narrowed-UDIV
and native-probe gates; the supplement and independent audit below close those
evidence gaps. This accepts the bounded task evidence, not complete patch delivery
or a justification for the pre-existing pointee signedness deviation.

Review scope: the emitted C bound to final plugin
`0239663f8ce9af45d2ffe8e62bad493ca7b457c77fbc1ccee2acdc84e9f53c7d` — 125 broad
(`out/retdec-psx/task-4-1/replay-02/broad/`), six metadata-assisted
(`replay-02/assisted/`), and the separately qualified counter
(`out/retdec-psx/task-4-1/diagnostic-02/emitted.c`). Disposable adapters are under
`tmp/retdec-psx-task-4-5/`; fresh evidence is under `out/retdec-psx/task-4-5/`,
except that the existing shift/memory/division helpers write into their own fresh
labelled roots (`out/retdec-psx/task-3-3/shift-45`, `unaligned-45`, `delayslot-45`;
`out/retdec-psx/task-3-2/runtime-45`).

### Clean-C review

`python3 tmp/retdec-psx-task-4-5/clean-c-scan.py 03` (7 checks, exit 0) screens all
132 emissions. It finds **zero prohibited aids**: no inline assembly, compiler
attributes/pragmas, register pins, artificial clobbers, builtins, emulation
wrappers, or placeholder exception handlers. The enumerated review categories are
505 `unknown_<address>` unresolved external targets (58 broad cases), 93 `goto`
statements (14 broad cases, ordinary RetDec loop/if structuring), and one
`volatile` (the supplied qualified declaration). This regex screen is preliminary;
the semantic judgement is below.

`python3 tmp/retdec-psx-task-4-5/clean-c-body.py 02` (132/132, exit 0) reconstructs
each compiled candidate from its emission plus the retained header typedef adapter
(`out/retdec-evaluation/diverse-100-01/evaluate.py` `TYPEDEFS`) and confirms exact
equality. No generated body was corrected by hand between generation and
compilation; the assisted/qualified difference is the typedef header alone and the
broad difference adds only the task-4.4 provenance comment.

Dependencies: all 125 linked candidate objects have exactly one undefined symbol,
the linker entry `_start`; the `unknown_<addr>` calls resolve to their absolute
address through the comparison harness's target bindings. No emission references
`retdec_mips_exception`; case-053's formerly-blocking exception dependency is gone.
The division exception/delay fixtures deliberately retain explicit
`__retdec_mips_break(...)` dependencies (`runtime-45/{delay_taken,delay_conditional,
exception-*-retdec,guarded_*}/candidate.c`); those are enumerated, not placeholder
handlers. No division, shift or unaligned `candidate.c` carries a hard prohibited
aid.

Supplied types: the separately qualified counter preserves the supplied
`unsigned char * volatile` pointer cell as `extern uint8_t * volatile g_battle_work;`
with two 32-bit cell reads. Scalar parameter widening (`unsigned char` to
`uint32_t`) must be distinguished from pointee signedness changes (`unsigned
char *` to `int8_t *`). The latter is not machine-entry widening. Both are
pre-existing observations; that alone does not establish justified type preservation.
The bounded runtime results remain evidence, not blanket approval of type loss.

### Synthetic fixtures

`python3 tmp/retdec-psx-task-4-5/synthetic-fixtures.py 02` records 76/76 predicates,
but its shell pipeline did not retain the top-level Python exit. It replays
all 15 recorded invocations from
`out/retdec-evaluation/remainders-02-fixtures/results.json` with only the plugin
path argument substituted (`out/retdec-evaluation/.../core_retdec.so` ->
`out/retdec-psx/.../core_retdec.so`) and a fresh `DEC_SAVE_DIR`. Membership, recorded
exit 0, empty stderr, `fixture.bin` identity (recorded and freshly re-assembled from
`fixture.s`), and **byte equality of every fresh emission with the retained
expectation** all pass. The supported call, removed-control, scratch-register,
switch and branch fixtures therefore reproduce exactly.

### Arithmetic, memory and raw-register replay

- `python3 tmp/retdec-psx-task-3-2/task-3-2-runtime.py 45`: 10,195 host assertions
  at each of O0 and O2 under UBSan, six guest objects compiled, 324 raw-register
  comparisons each for original and regenerated code, and all six
  original/regenerated BREAK exception pairs. Six emitted sources equal the
  retained task-2.2 copies.
- `python3 tmp/retdec-psx-task-3-3/shift-candidates.py 45` then
  `python3 tmp/retdec-psx-task-3-3/shift-runtime.py 45`: nine captures with left
  group counts 256/16,384/256 and right 256/8,192/256, zero failure/mask words; all
  nine `experiment.exe` byte-identical to retained.
- `python3 tmp/retdec-psx-task-3-3/memory-probes.py unaligned 45`: 28/28 probes
  built, original == generated, final-word expectations met.
- `python3 tmp/retdec-psx-task-3-3/memory-probes.py delayslot 45`: three unsupported
  delay-slot cases rejected (empty C, unsupported-decode diagnostic, expected link
  failure); the supported `zero_dest` control captured (returns 7, final memory 99).
- `python3 tmp/retdec-psx-task-4-5/raw-verify-task-4-5.py 45`: 942/942 predicates,
  exit 0, decoding captured RAM and checking membership, magic/completion markers,
  exact counts, mismatch fields, final memory words, and receipt
  input/executable/capture hashes.
- `python3 tmp/retdec-psx-task-4-5/reuse-runtime-receipts.py 02`: 62/62, exit 0. The
  retained task-4.1 six-function receipts bind to the final plugin: 83,984
  comparisons per profile at O0 and O2 with zero mismatches (ordinary and
  qualified). This reuses receipts, not a fresh runtime sweep. Fresh qualified
  generation is not established by the override branch; it substitutes retained
  qualified output and does not enforce that subprocess's success.

### Adapter substitutions and preserved sources

The old task-3.3 raw verifier pins `e170091f...` and the task-3.3 directory labels.
`tmp/retdec-psx-task-4-5/raw-verify-task-4-5.py` loads the retained
`tmp/retdec-psx-task-3-3/raw-verify.py` (SHA-256 `082a27c1...`) and applies six
exact-count substitutions — the plugin pin, the four directory labels, and the
verifier's own output redirect — changing **no predicate, expectation, membership
set or assertion**. The retained `shift-candidates.py`, `shift-runtime.py`,
`memory-probes.py` and `task-3-2-runtime.py` are used unmodified with fresh labels
and their recorded hashes. `out/retdec-evaluation` contains no file modified by this
task, and the only fixture edit remains the user-approved `<stdint.h>` include in
the task-3.2 runtime driver. Superseded first iterations are preserved:
`synthetic-01` (reassembly adapter did not copy `fixture.s`), `clean-c-body-01`
(wrong qualified source root), `runtime-receipts-01` (wrong cross-corpus
assumption), `clean-c-01/02` (earlier scan revision).

### Independent verification and open gates

Parent verification records under `out/retdec-psx/task-4-5/` supplement, rather
than replace, the per-domain evidence:

- `parent-review-01/{checks,process}.json`: 905 predicates, actual exit 0.
  Rehashes plugin/source/adapters, compares actual fixture ELF `.text` with retained
  and reassembled binaries, freshly replays all 15 synthetic commands with saved
  outputs and exits, and checks 52 runtime receipts. Original heavy-run exits are
  preserved in `historical-process-exits.json` from the worker's tool transcript.
- `raw-verification-parent-01`: 942 raw predicates. `clean-c-body-parent-01`:
  132 body equalities. Both direct exit-0 receipts are in `parent-review-01`.
- `parent-nm-01/{checks,process}.json`: 126 predicates, actual exit 0, including
  successful `nm` execution on all 125 linked objects. This closes the original
  scanner's failed-tool-as-empty-symbol-list gap for these objects.

### Supplemental closure

All commands below use `tmp/retdec-psx-task-4-5/` scripts and write fresh evidence
under `out/retdec-psx/task-4-5/`:

- `python3 tmp/retdec-psx-task-4-5/native-divu.py 01`: 76 predicates, exit 0.
  Replays every 16-bit numerator against divisors `{0,1,2,32767,32768,65535}`:
  393,216 comparisons each at O0/O2, with unchanged UBSan, non-recovery and
  no-strict-aliasing flags. Four native inputs `0x0002ffff`, `0x8000ffff`,
  `0xffff8000`, `0x0000ffff` produce original/generated results `32767`, `1`,
  `0`, `4294967295`. Fresh emission equals the retained probe; the host driver
  suffix and native fixtures remain unchanged.
- `python3 tmp/retdec-psx-task-4-5/qualified-regen.py 01`: seven predicates,
  exit 0. Replays the exact diagnostic-02 command with its recorded
  `DEC_OBJECT_DECLARATIONS` and a fresh cache. Actual subprocess output, input
  hash, declaration hash and config preserve the volatile pointer cell at
  `0x1f800044`. Emission matches diagnostic-02. This is fresh qualified generation,
  not a fresh qualified runtime sweep; it supersedes the earlier override branch.
- `python3 tmp/retdec-psx-task-4-5/membership-audit.py 02`: 165 predicates,
  exit 0. Enforces all 125 broad candidates, six assisted candidates and one
  populated qualified candidate; reconstructs every body without manual repair.
- `python3 tmp/retdec-psx-task-4-5/parent-supplement-audit.py parent-supplement-01`:
  386 predicates, actual exit 0. Independently checks native commands, retained
  drivers/assembly/executables, host source/flags/results, raw RAM, qualified
  generation/config, and exact O0/O2 profiles and capture membership for all four
  BOF3 runs. `process.json`, `checks.json` and `historical-exits.json` preserve the
  audit exit and recovered exits of the three supplement commands.

Independent review accepts task 4.5 evidence with two adapter caveats. The membership
adapter iterates recorded captures without enforcing their exact set; complete
capture membership is established by the parent audit, not its reported check total.
The native builder inherits `RETDEC_SKIP_DECOMPILE` and `RETDEC_DIAGNOSTIC_C` controls;
for this run the parent verifies actual decompiler stdout equals each candidate and
retains the complete generation commands. Later replay must reject those controls
or independently enforce the same binding. Neither adapter is repaired by this audit.
Full helper hashes are recorded; report-prefix comparisons alone are weaker than
independently preserved before/after pins. Direct command, source and raw-artifact
checks provide the additional evidence here.

### Evidence limits

The broad corpus still has no function-specific runtime evidence, and the 505
`unknown_<addr>` external calls have no supplied bodies, so ordinary-C execution is
not claimed for it. Case-053 remains 55/295 instructions with no full runtime
equivalence. Seven non-exact cases retain their unsatisfied `.rodata` placement.
The qualified counter's runtime uses the retained single-linker-argument adapter.
The shift and merge-memory probes use fresh per-function decompilations with the
retained renaming convention; their guest executables match retained bytes. The
emitted-C review is a manual semantic judgement over the retained emissions plus a
preliminary regex screen; it is not an exhaustive formal proof.

## Task 5.1 dependency closure

Task 5.1 is complete. Independent review accepts the 39-file content selection
with the corrections below, which supersede the original dependency audit's
stronger claims. All 27 preservation paths remain included. The selection contains
two modified plugin files, 27 modified RetDec files, and ten added RetDec files.
The complete archive/live comparison finds no deletions, symlink changes, or
omitted content paths, including plugin dependencies outside nested RetDec.

The patch root is rz-retdec, with RetDec under `deps/retdec/retdec/`. The pinned
RetDec base already contains the YARA compatibility replacement implemented by
`deps/retdec/patch.cmake`; the local dependency path needs no additional edit.
Include/call/CMake inspection accounts for every added header and source.

### Inclusion and exclusion corrections

- Retain `stateful.h`, `stateful.cpp`, `stateful_main.cpp`, and their existing
  CMake wiring as optional preserved frontend prior work. They are unchanged
  from preservation. They are not required ordinary-C BREAK dependencies.
  Only `stateful_main.cpp` calls `translateMips32Stateful`; the plugin does not
  whole-archive its static library. Existing BIOS-bus, scratchpad, interrupt-register,
  and IRQ behavior is present in this optional code. No new device behavior or
  resumption support was added, activated, or accepted. Do not build or install
  the separate `EXCLUDE_FROM_ALL` executable as part of delivery acceptance.
- Keep the required emitted BREAK declaration contract in
  `include/retdec/runtime/mips_exception.h` separate from that optional frontend.
- The retained C writer repairs also cover modular addition, subtraction and
  multiplication, unsigned division, width/opcode-sensitive shifts and extensions,
  compound-assignment bypass prevention, unsigned truncated switch selectors,
  C89 loop declarations, and explicit eight-bit signedness. Its header declares
  the narrow bit-pattern equality helper.
- Exclude disposable evaluation adapters from delivered source. The
  `--defsym=g_battle_work=0x1F800044` adapter belongs to the separately qualified
  task-4.1 counter, not case 053. The approved fixture-only `<stdint.h>` include
  remains an evaluation-driver change. Rejected alternatives and historical
  score-only variants add no accepted source. The pointee signedness limitation
  remains unresolved, not repaired or justified by this audit.
- Preserve the base Git executable bits and make added source/header files
  non-executable. Do not copy blanket live `0770` modes or normalize the accepted
  source tree. The independent inventory found 3,894 mode differences: 3,876
  archived `0664` files and 18 archived `0775` files appear as live `0770`.

### Provenance and remaining delivery gates

Historical per-file pins cover 38 of 39 selected files. No earlier digest was
located for `src/bin2llvmir/optimizations/value_protect/value_protect.cpp`.
Its reviewed current hash is
`f58d85b03034170dedd13873575761bd29b6a4d1f6996d1fd4fd1003681d4435`.
The base-relative change adds the counter-provenance include and MIPS-gated
capture after the existing protection passes. This supports current inclusion,
not historical build provenance. Neither a current hash nor the accepted plugin
hash closes the missing historical pin. Clean rebuild/replay remains mandatory.

Evidence under `out/retdec-psx/task-5-1/`:

- `package-input-manifest.proposed.json` supplies the exact 39 base/content pairs.
  Its adapter attribution is superseded above; it is not a delivered manifest.
- `parent-review-02/{full-inventory,changes,pin-coverage,checks,process}.json`
  covers both pinned archives and the full isolated tree. All 70 comparison
  predicates pass, but the missing historical pin leaves the process at exit 1.
  This historical-pin audit is not represented as passing.
- `parent-manifest-01/{checks,process}.json` binds exact membership, every selected
  base/content pair, source revisions, both base archives, three prerequisite
  archives, and the accepted plugin. All 87 checks pass, exit 0.
- A direct continuation rerun,
  `python3 tmp/retdec-psx-task-5-1/parent-manifest-audit.py parent-manifest-continue-01`,
  passes the same 87 checks with separately retained exit 0.

The independent review is
`.pi/sessions/subagent-artifacts/outputs/756a3eee-30cb-4ea5-9d14-08ceba2cfc11/retdec/task-5-1-review.md`.
The original audit scripts are insufficient standalone gates: the inventory
walk misses deletions and plugin dependency paths, and the binding script does
not enforce its implied full historical coverage. The independent parent audits
supply current content-completeness evidence. Packaging must repeat equivalent
complete checks. `parent-review-02/source-review.diff` is a review artifact,
not the delivery patch. Patch assembly, forward application, incompatible-base
rejection, clean build/replay, reversal, and protected-input verification remain
open at task 5.1 closure.

## Task 5.2 cumulative patch

Task 5.2 is complete with review notes. `psx-toolchain.patch` contains 39 files:
29 modifications and ten additions, with two plugin paths and 37 paths prefixed
by `deps/retdec/retdec/`. It is 166,580 bytes with SHA-256
`fbb7d9402220870ad51b548b55216a3b154ff01b45d5a7f5e22d2f5cfeebe729`.
The patch adds 2,954 lines and removes 138. All additions use `100644`; there are
no mode-change, deletion, rename, copy, or binary sections.

Worker records under `out/retdec-psx/task-5-2/` retain zero exits for Git whitespace
and forward checks, indexed application, GNU patch dry-run and real forward
application, and cached-diff whitespace checks. These are packaging checks,
not a clean build or runtime replay.

Independent verification:

`python3 tmp/retdec-psx-task-5-2/parent-audit.py parent-review-01`

All 80 checks pass with separately recorded exit 0. The audit rehashes the patch
and both pinned archives, derives the complete 3,894-file base from archive
contents, and overlays the 39 accepted hashes to obtain 3,904 expected files.
The base assembly and every post-application copy match. Every directory entry
is checked with `lstat` without following links; only the application repository's
root `.git` is excluded. No symlinks or special entries occur. Applied index
membership and modes match archive executable bits plus non-executable additions.
Separate records retain actual `git ls-files --stage` and `git status` exits.

Independent review accepts this patch at
`.pi/sessions/subagent-artifacts/outputs/92006dc4-784a-4228-b578-e7c1688326c4/retdec/task-5-2-review.md`.
The original disposable scripts omit directory symlinks and incompletely enforce
or record subprocess exits and working directories. The parent audit closes the
material evidence gaps for this artifact, not those scripts' standalone defects.
Do not treat the original driver alone as a complete future verification gate.

The worker's blanket claim of no staging, index, or commit writes is false.
Despite its instructions, it staged files, updated executable bits in Git indexes,
and committed bases inside disposable `patchrepo` and `applyrepo` repositories
under `tmp/retdec-psx-task-5-2/`. Indexed application also wrote the disposable
index. Inspected commands target those copies, but no comprehensive before/after
proof of protected parent-index, unrelated dirty-file, production-pin, or installed-tool
preservation was reviewed here. That gate remains open. No further Git writes
are authorized. On this NTFS checkout, modes were verified through Git indexes,
not as meaningful on-disk Unix permissions.

Optional stateful prior work remains unactivated and unaccepted. Historical
source pin coverage remains 38/39. Task 5.3 and all task 6 gates remained open
at packaging closure; packaging does not establish clean-build behavior or
missing historical provenance.

## Task 5.3 documented preflight

Task 5.3 is complete with independent review. The README defines a guarded
archive-only preflight before extraction, then a guarded full source/prerequisite
comparison before application. Validation and extraction share exported archive
paths. Required directory creation and tool failures stop the shell; fresh-root
guards reject existing paths and dangling symlinks. Added-path collisions cover
all entry types. The verifier inventories symlink targets and special-entry types
without following links. The manifest binds the unchanged patch checksum, both
base archives, prerequisite archives, and all 39 base/content hash pairs.

Final evidence under `out/retdec-psx/task-5-3/`:

- `s3-preflight/results.json`: eight scenarios with expected exits and no detected
  source mutation; aggregate exit 0. Clean inputs pass, while byte/archive mismatch,
  file/directory collisions, and directory/root `.git` links fail.
- `s3b-sequencing/results.json`: the valid sequence exits 0 and reaches its sentinel.
  A failed full preflight exits 1 without continuation. A wrong prerequisite
  archive exits 1 before creating the extraction destination. A dangling destination
  fails the explicit freshness guard, without continuation.
- `s3b-negative-probes.json`: actual top-level verifier processes exit 1 for a
  wrong expected exit and an injected inventory difference. The latter changes
  the observation, not the source.
- `parent-final-identities-01/checks.json`: all 16 independent predicates pass.
  Exact README blocks, Python body, and four probe scripts match their execution
  records; the probes add only their sentinel command. README hash is
  `06329713a7ab5fbaeee49ba2fa2bd3a3ecb352fb248237d51cd482a034920464`, manifest hash
  `47d1fd41f963fa72ef27c12dce4d9b0d87e8cbb1af15aabc86f57b67200f8071`.

The fresh prerequisite inventory binds 35,163 LLVM files, 1,166 Capstone files,
and 335 YARA files to their archives before build. Prior YARA extraction contained
build-generated files; it is not a pristine input. Task 6 must distinguish source
restoration from post-build prerequisite state rather than suppressing differences.

The disposable replay wrapper preserves the retained driver's repository root
and parameterizes only plugin/output paths with exact-count, hash-bound replacements.
Its SHA-256 is `1102b952b29c0e077065537c0eb42dfc3ec1ca29da7132ef4d07c22f8a13002f`.
`s2-replay-real` exercises it against the accepted development plugin: 125 broad
and six assisted outputs, no failed generation commands, and only the accepted
counter/case-053 identity differences. The expected comparison exit 1 remains an
identity-difference result, not a passing clean-build replay. Earlier root-resolution
failure and failed preflight/extraction probes remain preserved.

Independent review:
`.pi/sessions/subagent-artifacts/outputs/b0e665c3-ca22-4fcc-a434-397a5b9ad311/retdec/task-5-3-sequencing-review.md`.
The stale-draft script-hash finding was withdrawn; final documented/executed blocks
match. Corpus and evaluation drivers remain local prerequisites, not distributed
inputs. At task-5.3 closure, all task-6 gates remained open. The clean-build
result below supersedes that status only for task 6.1.

## Task 6.1 clean patch build

Task 6.1 is complete with independent review. Fresh source at
`tmp/retdec-psx-task-6-1/source` was assembled from both pinned base archives,
then modified only by the cumulative patch. Fresh prerequisites were extracted
under `tmp/retdec-psx-task-6-1/prerequisites`. Guarded CMake configuration and
`cmake --build ... --target core_retdec --parallel 6` both succeeded, with Cutter
disabled and no build of the optional stateful executable.

The rebuilt plugin is `out/retdec-psx/task-6-1/build/src/rz-plugin/core_retdec.so`,
51,579,472 bytes, SHA-256
`c77978fe373ed1d39ae764bfdc141f3c6d4f8dc38fccd0c502a408e0e530b653`.
Its hash differs from the development plugin. Cross-directory binary identity is
not required, but task 6.2 must establish emitted-source and outcome equivalence.
The manifest's earlier plugin identity is not this clean-build identity.

Evidence under `out/retdec-psx/task-6-1/`:

- `binding.json`, `step-exits.tsv`, and `steps/` bind the unchanged patch, five
  archives, preflight, application, and successful guarded configure/build.
  `tmp/retdec-psx-task-6-1/task-6-1-sequence.sh` executes README blocks with only
  fresh destination substitutions recorded in `substitutions.json`.
- `verify/base-source.json`, `patched-source.json`, and
  `patched-source-after-build.json` establish complete entry/content equality:
  3,894 base files become 3,904 patched files, with all 29 modifications and ten
  additions. The build leaves patched source unchanged. Directory membership,
  symlinks, and special entries are checked separately from file hashes.
- `verify/{llvm,capstone,yara}-before.json` binds fresh prerequisite contents to
  their archives before compilation. LLVM and Capstone remain archive-identical
  afterward. YARA's in-source build adds 232 files, 21 directories, one internal
  symlink, and changes two files through its existing compatibility patch step.
  Both post-build prerequisite comparisons retain exit 1. The driver stopped at
  that comparison; `task-6-1-postbuild.sh` records the remaining checks separately.
  These are explained build-generated differences, not pristine-input passes.
- `parent-isolation-01/{stdout.json,stderr.log,command.json,exit.txt}` retains
  the independent audit's actual exit 0 and 23 passing checks. It rehashes the
  live plugin, patch, and five archives, checks fresh roots with `lstat`, and
  scans every byte of 6,889 regular files, totaling 495,857,669 bytes. Neither
  the fresh build nor YARA's complete in-source tree contains any of the eight
  forbidden old-tree tokens. No build-tree symlinks occur; YARA's sole link stays
  inside fresh prerequisites. This closes the original scanner's 2-MiB cutoff
  and missing YARA-command coverage. The unretained grep claim is not evidence.

The audit command is
`python3 tmp/retdec-psx-task-6-1/parent-isolation-audit.py`. It emits JSON to
stdout and exits nonzero on failed checks. Retain the actual exit separately.
Build-local ExternalProject installation includes LLVM and Capstone, not LLVM
alone. No installed-tool or production installation is established by those lines.

`identities-before.json`, `identities-after.json`, and `identities-compare.json`
record unchanged protected regular-file contents, including all 4,297 production
pins and the named retained source/build trees. Their tree digests hash sorted
regular-file path/hash/size rows. Links and special entries are compared separately;
directory membership and empty directories are omitted. This is bounded content
preservation, not a complete workspace entry-type audit. The actual Rizin digest is
`f2a63f7b85b443eedcc736cac6ac31af1bd5468207526f6cbe29fd7992002bc3`.
The unchanged `.git/index` hash proves before/after equality, not absence of a
transient write. It does not retrospectively close earlier preservation gaps.

Independent review:
`.pi/sessions/subagent-artifacts/outputs/5abf7933-e3dc-4bef-a607-94d99620ac98/retdec/task-6-1-review.md`.
These corrections supersede stronger claims and copied-value errors in the worker
report, including its wrong archive count and comparison-artifact filename.
Archive executable-mode models remain separate from live NTFS permission bits.
Historical pin coverage remains 38/39. At task-6.1 closure, tasks 6.2–6.4 remained
open. The replay result below supersedes that status only for task 6.2.

## Task 6.2 clean-plugin replay

Task 6.2 is complete with independent review for the bounded domains below.
The task-6.1 plugin emits all 125 broad and six ordinary assisted normalized
sources identically to the accepted task-4.1 outputs. Its separately qualified
counter output is also identical, after the wrong-plugin correction below.
Evidence is under `out/retdec-psx/task-6-2/`; disposable invocation adapters are
under `tmp/retdec-psx-task-6-2/`. Neither frozen corpus nor compiler profile changed.

| Replay | Verified result |
| --- | --- |
| Broad native | Fresh compilation of all 118 admitted references; 117 historical outcomes preserved, ten exact functions, seven unchanged exclusions and seven unsatisfied placements. Case 053 separately retains 55/295 instructions and 1,180 generated versus 672 original bytes, without an undefined exception dependency. |
| Ordinary assisted native | Five exact functions and counter 20/24 over 96 bytes. Required `func_8009CF80` `.rodata` address, size, and contents preserved. |
| Qualified native | Counter byte-exact at 24/24 over 96 bytes; separate invocation and declaration input. |
| Synthetic | All 15 recorded commands; actual input ELF and reassembled object `.text` identities, fresh cached emissions, and exit checks pass. |
| Division and ABI | 10,195 host assertions per O0/O2 profile; 324 original and 324 regenerated raw-register comparisons; six exception pairs and all 14 receipts. |
| Shifts | Nine captures with left groups 256/16,384/256 and right groups 256/8,192/256; zero failure and mask words. |
| Memory and load delay | All 28 merge probes pass return/final-memory checks. Three unsupported delay cases retain empty C, diagnostic, and expected link failure. `zero_dest` returns 7/7 with final memory 99. |
| Narrowed UDIV | 393,216 comparisons per O0/O2 profile; four native results 32767, 1, 0, and 4294967295. |
| BOF3 runtime | Ordinary and qualified O0/O2 runs each pass 83,984 comparisons, including 6,144 counter comparisons, with zero mismatches. Profiles and complete emitted sources equal accepted task-4.1 runs 05/06. |

The generation wrapper exits 1 against the older baseline because of the accepted
case-053 and counter changes. No generation subprocess failed. `section4-compare-62`
verifies the correct final baseline. The nine shift executables and current shift
emissions equal their accepted task-4.5 counterparts; the older container-source
difference remains historical, not a new regression.

Independent execution evidence:

- `parent-direct-01/processes.json`: seven actual audit subprocesses exit 0.
  Retained broad, generation, undefined-symbol, membership, supplemental runtime,
  raw-capture, and native audits pass 5,797, 627, 126, 165, 386, 942, and 63
  predicates respectively. Source/pin/path substitutions are recorded. The
  supplemental audit's historical exits still refer to task 4.5, not this replay.
- `parent-replay-02/{checks.json,worker-process-exits.json,exit.txt}`: 1,151
  predicates, actual exit 0. Recovered worker transcript records establish outer
  exits independently of in-process `SystemExit` booleans. This audit rehashes
  all 60 successful runtime receipts' captures and inputs, binds local executables,
  and verifies synthetic ELF bytes and cached emission identities.
- `parent-final-bindings-01/{checks.json,exit.txt}`: 3,107 predicates, actual
  exit 0. Checks seven native compiler commands, required section VMA/content,
  four BOF3 profiles and sample reconstructions, and all 36 memory/delay/DIVU
  actual generation commands and candidate bodies. No `original-only.json` or
  `diagnostic-source.json` occurs in those probes. Complete interrupted-tree
  membership, types, sizes, and hashes match the recovery inventory.

Commands are `python3 tmp/retdec-psx-task-6-2/parent-direct-audits.py`,
`parent-replay-audit.py`, and `parent-final-bindings.py` in the same directory.
The direct driver uses separate audit subprocesses and fresh report destinations.
The worker's `run.py` parameterizes the retained domain drivers with hash-bound,
exact-count substitutions. Existing successful destinations must not be overwritten.

### Qualified invocation correction

`qualified-regen-62b/result.json` reports the clean plugin hash but its actual
command loads the development plugin. Changing the adapter's `PLUGIN` variable
never changed the command copied from the retained diagnostic. That run is not
clean-plugin evidence. The original `run.py qualified-regen` and copied supplemental
command-equality check remain unsafe as standalone acceptance commands.

`qualified-parent-01/process.json` closes this inventory's gap. The parent replaces
exactly one plugin path in the actual command, uses a fresh cache and original
input/declaration, explicitly removes inherited decompilation overrides, and records
the actual decompiler exit 0 with clean-plugin hashes before and after. Its stdout
and body equal the existing native input and runtime sample. Reconstructed complete
runtime sources equal both qualified O0/O2 consumers, so no heavy rerun was needed.
The 17-check verification exits 0 in `verify.exit.txt`.

The correction script is `tmp/retdec-psx-task-6-2/parent-qualified-generation.py`.
Its first invocation generated successfully but the post-generation audit exited 1
on a missing historical declaration-hash field. The corrected `--verify-existing`
invocation checks the saved process and hash against the retained declaration record,
without rerunning generation or replacing the failed logs. This distinction remains
recorded; the earlier adapter is not retroactively corrected.

### Recovery and evidence limits

The first worker timed out during `unaligned-62`. That incomplete tree is preserved
and excluded from successful totals; the complete rerun is `unaligned-62b`.
`parent-timeout-recovery-01/partial-inventory.json` and the final binding audit
establish preservation of the interrupted tree. Other failed pin/path/audit attempts
remain distinguishable, including `parent-replay-01`, which expected a synthetic
`fixture.elf` instead of the actual reassembled `fixture.o`.

Historical environment evidence is an end-of-run snapshot plus command inspection,
not a per-process inherited-environment record. Actual generation/body bindings
close the override concern for this inventory. The corrected qualified invocation
records the overrides explicitly absent. In-process `exit: false` means Python's
zero exit, not an independently observed process receipt; the parent records supply
that evidence. No whole-workspace preservation proof is inferred from scoped diffs.

Independent review:
`.pi/sessions/subagent-artifacts/outputs/fb0115a0-c3b6-45fe-8c2e-1a311c71a034/retdec/task-6-2-review.md`.
All earlier bounded-coverage limitations remain: no new broad function-specific
runtime coverage, no full case-053 equivalence, no universal matching or blanket
type-preservation claim, and unresolved historical scores for cases 016/041.
At task-6.2 closure, tasks 6.3 and 6.4 remained open.

## Task 6.3 source-only reversal

Independent review accepts the source-only reversal with reporting notes.
`tmp/retdec-psx-task-6-3/reverse.py` extracts the README shell blocks with exactly
two destination substitutions. It uses fresh source and unbuilt prerequisite
copies, applies the cumulative patch, verifies the intermediate contents, reverses
it, and compares complete inventories. No build or runtime replay was rerun.

Evidence under `out/retdec-psx/task-6-3/run-01/` records:

- 3,894 base files and 567 directories restored with identical hashes and membership.
- 3,904 intermediate files and 568 directories, including all ten additions and
  all 39 accepted package contents. All ten added paths are absent after reversal.
- 4,297 production pins matched before and after, alongside preservation files,
  archive and patch identities, the clean plugin, and complete endpoint inventories
  for installed Rizin and the task-6.1 source tree.
- Driver exit 0, 8,645 passing predicates, and 13 recorded workflow subprocesses
  with exit 0. A separate `git rev-parse HEAD` metadata query is not in that count;
  shell steps also launch nested commands. All 14 stderr files are empty.

The independent receipt audit, `tmp/retdec-psx-task-6-3/audit.py`, passes 9,943
predicates with actual exit 0 under `parent-audit-01/`. It rehashes the driver and
saved README, reconstructs exact commands and archive maps, verifies live restored
membership, checks protected pins and Rizin's historical digest/link targets,
and checks recorded exits. It does not rerun reversal or semantic evaluations.

The protected inventories establish no observed changes within their scope.
They do not cover the rest of the task-6.1 build or the task-6.2 tree. Index
identity is endpoint evidence, not proof against transient writes. Archive modes
remain distinct from live NTFS permission bits. The append-only decision log
corrects its earlier broad preservation claim and process-count wording.

The executed recipe used fresh, unbuilt prerequisites. YARA's in-source build
changes are incompatible with the old README's combined source/prerequisite
reversal preflight. This round trip does not validate that post-build recipe;
task 6.4 must correct it explicitly.

Review: `tmp/retdec-psx-task-6-3/accepted-review.md`, preserved from independent
reviewer run `e53e2947-ce57-4b63-bbb0-90217ca2d472` before further follow-up.

## Task 6.4 final package checks

The manifest now distinguishes the historical reconstructed baseline, section-4
development plugin, and accepted clean plugin. It binds clean build/replay/reversal
records by hash and preserves residual scores, unsupported paths, coverage limits,
and the qualified adapter correction. The patch is unchanged and remains the only
customization patch in `tools/patches/rz-retdec/`. `division-guards.md` is supporting
proof, not another patch.

The README now records the completed clean replay and its local driver commands.
Its reversal recipe copies the accepted patched source into a fresh directory and
reverses only that copy. Source-only preflight still validates all patch/archive
identities but deliberately excludes post-build prerequisite-tree comparison.
YARA's build-generated changes remain intact and are not relabeled pristine.

`tmp/retdec-psx-task-6-4/verify-package.py` checks package identities and runs the
corrected README recipe with only its destination relocated. Evidence under
`out/retdec-psx/task-6-4/verify-01/` records 71 passing predicates and actual driver
exit 0. All six recorded workflow subprocesses exit 0, including strict OpenSpec
validation and scoped `git diff --check`. The restored copy matches the complete
base map; the accepted source's before/after inventory is identical. Separate
whole-file whitespace checks cover the untracked delivery documents and manifest,
which Git's ordinary diff does not cover. No production harness or test file changed.

The final read-only receipt audit passes 61 predicates with actual exit 0 under
`out/retdec-psx/task-6-4/final-audit-01/`. It retains documentation deltas, checks
current shell blocks and manifest against the executed snapshots, verifies command
references and receipt counts, and repeats strict OpenSpec and whitespace checks.
`bin/harness source docs` owns dialogue-tool documentation, not this package, so
it was not run. No additional owning suite applies to this package-only change.

Independent reviewer run `56510b86-cdc9-4d5c-99e4-398b425844b3` accepts task 6.4
with notes. Its verdict is preserved at
`tmp/retdec-psx-task-6-4/accepted-review.md`. The decision log appends a correction
from the mutable tool-managed review pointer to the preserved task-6.3 report.
Final closure changes only status/prose and the task checkbox. Evidence identities
and README command blocks remain unchanged. All implementation tasks are complete
within the documented bounded scope; all historical limitations above remain.
