# Matching knowledge

Follow the [match-loop](match-loop.md) ladder. Before address or
qualifier changes read [memory API](memory-api.md). Every lever is hypothesis until
owning live native comparison; keep best coherent PRE and full attempt ledger.

Branches: [first diff](#first-diff-decisions), [control flow](#control-flow),
[delay slots](#delay-slots-and-entry-copies), [representation](#representation-and-volatility),
[allocation](#allocator-sensitive-functions), [orphan stack slots](#orphan-stack-slots),
[compiler](#compiler-evidence),
[runtime](#runtime-evidence), [storage](#storage-and-analysis-context),
[failure/capture](#failure-and-capture).

## First-diff decisions

| Symptom | Evidence and clean-C lever |
| --- | --- |
| Pointer load versus address calculation | Prove pointer versus array declaration |
| Wrong relocation/base folding | Standalone symbol versus reviewed struct field/array |
| `addu $at,index,$at` versus `addu $at,$at,index` | Named extern array can shift expansion to assembler; never pin scratch `$at` |
| Fixed-address constant steals entry copy | Named extern binding can prevent constant-address CSE; verify entire register web |
| Extra sign extension or `andi` | Prove signedness/width/necessary mask; do not guess from decompiler |
| Frame/size mismatch | Correct calls/prototypes, address-taken locals, aggregate copies, lifetime and topology; trace unused frame bytes through [orphan stack slots](#orphan-stack-slots) |
| `beq`/`bne`, return web or tail merge | Invert branches, early return versus result local, duplicate branch calls |
| Loop topology | Compare while/do/for/guarded loop/explicit labels; matched goto is not style debt |
| Global/BSS offsets | Audit COMMON/section/alignment/order/padding and following symbols |
| Phantom read-only object | Local static constant versus existing shared extern ownership |
| Load uses wrong argument register | Verify callee argument consumption and caller setup before allocator probes |
| Missing read after a store | Recheck pointer reloads, source qualification and aliasing against original instructions |
| Saved-register pair exchanged | Trace real value deaths, later reuse and merged allocator quantities |

Named extern-array indexing can emit folded `lw table(index)` and assembler
`lui $at; addu $at,$at,index`; fixed-address arithmetic may emit compiler's opposite
operand order. Neither representation universally wins. Known RAM pointer-valued
global needs narrow evidenced local extern, not anonymous macro hiding ownership.

## Control flow

Interpret every delay slot on both paths before changing equal-valued arms; slot
constant may be comparison operand, not selected result. Bounded matrix: ordered
then nested/inverted equality; separate ungrouped cases; duplicated arm stores/
returns; shared locals or labels. Advance only when measured evidence rejects
eligible alternatives. Locals may change register web. GCC can normalize
chain inversion regardless of spelling; another spelling alone is not new evidence.

Branch calls `if (cond) f(x+A); else f(x+B);` can merge into one `jal` where ternary
argument does not. Early returns versus initialized result changes `$v0` lifetime.
GCC loop rotation can move invariants: assigning inside loop body can permit late
hoisting/preheader copies; moving before loop can lose them. Keep only measured
semantically correct shape; record negative experiments before trying another shape.

A shared test after two value-producing arms can extend one local across the join.
Test equivalent per-arm checks or pure address calculations while retaining the
common result/return. Preserve memory reads and their order. Duplicated expressions
must not duplicate calls, volatile accesses or other side effects.

For preheader or delay-slot order, test a block-local value initialized at its real
use, or a single-execution `do { operation; } while (0)` around actual work. Such
scopes can alter loop notes, lifetimes and scheduling. They are hypotheses, not
barriers. Reject empty scopes, dummy work, added accesses and semantic distortion.

## Delay slots and entry copies

- Same-size branch/`jal`/`j` difference: identify exact instruction, live inputs/
  outputs, value use after transfer. Try branch inversion, early return, independent
  statement order, one local or pointer hoist.
- Original `move tN,aN`/`move vN,aN`: prove lifetime across call/branch or overlapping
  temporary, then local copy at that lifetime with bounded declaration/first-use order.
- Frame mismatch: solve prototype/width/call/aggregate/address-taking first.
- Relocation/load order: owner, symbol form, offset, pointer-cell volatility and
  cache/reload evidence first.

Preinitialization that fills one slot but damages downstream allocation is rejected.
Sole commutative `$at` operand-order residual may survive representation and
compiler-informed clean-C work. Assembler need not canonicalize operand encoding.
Preserve the unresolved result; opt-in work remains untried, not exhausted.

## Representation and volatility

Volatile store cannot move into jump delay slot. Narrow volatile pointee may cause
`lbu` plus manual sign extension versus original `lb`. Remove unjustified qualifier
only with semantic evidence; reviewed plain view may restore shape. Volatile pointer
cell forces reload without qualifying pointee; use API reference. Never add volatility
as arbitrary scheduler control. Original-proven ordering remains explicit evidence.

Unaligned fields/padding/SDK stack types follow type-application reference. Direct
member indexing may preserve folded offset where local pointer enlarges frame.
Scalar versus aggregate copies, induction counter separate from pointer, temporary
lifetime and per-evaluation reload locals can change allocation. Full comparison,
not one matching instruction, decides retention.

Match local views to existing storage and consumers. A mutable source view over a
mutable byte table can change read-after-write code generation without adding
volatility. `const` does not imply non-aliasing. A pointer-cell reload after a store
may correct an earlier reconstruction rather than preserve PRE under every alias.
Prove original accesses and final linked bytes; make no universal alias guarantee.

Use established halfword/aggregate views when field widths and consumers support
them. Preserve public constness and ABI unless caller/callee evidence supports a
change. Bitfields require target layout and codegen proof, not guessed packing.
Regroup unsigned address sums through a row/base temporary when original addition
order differs; prove value equivalence and preserve all memory events.

## Allocator-sensitive functions

Same-CFG near match plus narrow probe causing spills/frame/size/save-register/
prologue changes indicates allocation sensitivity. Follow the match-loop ledger
and exhaustion gate. Move intact statements across dependency-safe lifetime
boundaries before expression churn. Classify retained improvement, retained frontier,
reverted neutral/local regression/structural regression. Rank exact bytes, matched
instructions, later first mismatch, fewer hunks, unchanged size/count/frame, then
least source disturbance. Frontier alone is not completion; keep it separately and
combine with best strict improvement only with semantic evidence and live comparison.

Unexpected frame/size/spill/multi-block/prologue drift means restore, unless predicted
by original diff. Ledger retains source parent, moved statement/crossed lifetime,
score/first mismatch/hunks/size/count/frame, retained changes and interaction result.
Reopen rejected shape only with changed definition/last use, overlapping call,
interfering saved value/frame/residual block. Equivalent emitted forms share
one optimizer class; retry only when representation, lifetime or supported access
evidence changes. Chained assignments and local reuse change lifetimes.
Unsupported probes/compile failures never prove exhaustion.

Preserve original surviving calculations even if analyzer calls them dead. Caller
`andi v0,v0,0xFF` proves low-byte consumption, not narrow callee ABI; retain evidenced
full-width return and explicit caller cast, verify both. Boolean spelling can alter
narrow-type code. Local representation view may yield memory `lhu` instead of
register `andi`; no generic punning framework.

Trace each local from definition through its final consumer and address escapes.
After its value is consumed, test reusing it for a later real helper result or
calculation with a compatible type. Preserve call order, signed multiplication,
shifts and narrowing. The new result must have a genuine consumer. Never add dummy
uses to inflate liveness. Review the whole allocator output, not one register pair.

A narrowed induction temporary needs a complete reachable-range proof, including
the final endpoint. A higher score at the same size can be a retainable partial
while introducing a different residual. More matched instructions with missing
original bytes is not exactness; inspect conversion, mask, frame and flow changes.

### Orphan stack slots

Use this lever when the body matches but the frame differs, with no emitted body
access to the extra bytes. First exclude ABI, call, aggregate and address-escape
errors. In GCC 2.7.2, a folded address can leave an internal `USE` of a pseudo
whose definition disappeared. Reload may allocate a slot for that pseudo even
though the `USE` emits no instruction.

1. Bind before/after dumps to the source and unchanged compiler settings. Trace
   the address definition, its death note and surviving `USE` through combine,
   allocation and reload. Confirm a stack-slot `USE` and the extra frame bytes;
   a frame-size difference alone does not establish this cause.
2. Reuse one compatible scalar after each value's final consumer, for example
   store address, next store address, then a later real row index. Preserve each
   original reload, store and call. For address/index reuse through `u32`, prove
   the target's pointer width, lossless round-trip, alignment and unsigned offset
   arithmetic. Do not narrow an address or invent an aggregate to steer aliasing.
3. Check whether the later numeric consumer gives the local a hard register.
   An internal `USE` may remain; it must no longer require an orphan stack slot.
   Pointer-only reuse may reduce slots without removing the last one. Typed
   indexing can instead change alias analysis, reloads and the instruction body.
4. Measure through `lift sweep`, then the owning live gate and independent review.
   Require the original frame, complete function bytes and reviewed readonly data
   for exact acceptance. Preserve semantic and memory-event equivalence; smaller
   frames or fewer `USE`s alone are not acceptance evidence.

## Compiler evidence

Before more allocator/scheduler probes, inspect the actual compiler build recipe,
relevant patches and source version. Bind diagnostic dumps to the source image,
effective compiler and unchanged codegen flags. Locate the first differing pass:
parameter expansion, combine, scheduling, local/global allocation or reload.
Debug dumps and assembly-only diagnostics are not linked acceptance evidence.

For GCC 2.7.2, inspect `local-alloc.c` quantity membership, births/deaths, classes,
call crossings and hard-register suggestions. Its `combine_regs` can tie a dying
input to a result and sum reference counts. Ordinary priority uses merged references,
register size and lifetime, with suggested-register allocation first. `flow.c`
weights references by loop depth; even single-execution loop notes can matter.
Individual pseudo counts or declaration order alone do not explain allocation.

Read `expr.c` for promoted-variable conversions and `combine.c`/`loop.c` for known
bits and induction rewrites. Predict which temporary survives and which instruction
changes. Verify that prediction in before/after RTL and a supported live comparison.
Do not infer exact allocator causality from matching scores, debug-register names
or source formulas without accounting for suggestions and interference. Keep flags
fixed; no automatic flag catalogs or compiler-variant batches.

## Runtime evidence

Use `psx-rizin` for original call/access questions and `psx-emulator` for a bounded
behavior question. Define inputs, selected PCs or writes, stop and coverage before
execution. Natural observations and host-directed calls are different evidence.
For controlled calls, verify original overlay/helper residency, code identities,
register/stack setup and every host edit. Compare original and candidate from the
same supplied state. Keep proprietary captures local.

Check maintained receipts and current/staged input hashes. Version scripts before
changing them; an old receipt with a changed original script path is historical
staged-script evidence, not a current-input pass. Exec snapshots are pre-instruction
observations. Compare actual arguments, helper returns, memory events and outputs;
classify code, saved-return stack, timing and device differences separately.
A passed mission, sampled equality or narrower argument range cannot replace full
source-domain reasoning or live byte equality. Runtime can expose wrong behavior
labels; it does not prove an allocator explanation.

## Storage and analysis context

Pad/layout checks include real field offsets, size, BSS order, GP reach and following
symbols. Prefer owning explicit padding symbol, not universal macro. COMMON may
reorder by name. Prove the owning storage declaration and linker placement under
the attached profile; a layout discrepancy grants no flag changes. Shared
strings/tables need existing extern ownership, not phantom local static `.rodata`.

Separate Splat code/read-only jump-table objects can obscure m2c flow. Inspect label
resolution through owning generated-layout tooling; no handwritten assembly workaround
or unreviewed boundary change. m2ctx needs declarations/types/prototypes/macros/helpers,
not unrelated bodies/static data that shift rodata. GNU assembler GTE spelling may
use exact `.word` only in generated/SDK assembly, never C matching technique.

## Failure and capture

Compile failure is not diff or exhaustion. Compare known same-target function to
separate candidate from workspace/toolchain failure. Preserve last verified diff;
regenerate compile database when selected source settings are missing or stale.
Known sandbox i386 failure uses native-execution route, never C/flag churn.

Record reviewed reusable residual class, decisive clean-C shape, before/after and
eligibility in this skill's references through authorized writer. Function-specific
selectors/scores stay mission evidence; source retains required metadata and rationale
for opaque clean-C shapes, not campaign journal. Partial evidence is not universal
rule. Reviewer proposes wording; approval precedes durable rule promotion.
