# Matching knowledge

Use within [match-loop](match-loop.md) budget, not extra ladder. Before address or
qualifier changes read [memory API](memory-api.md). Every lever is hypothesis until
owning live native comparison; keep best coherent PRE and full attempt ledger.

Branches: [first diff](#first-diff-decisions), [control flow](#control-flow),
[delay slots](#delay-slots-and-entry-copies), [representation](#representation-and-volatility),
[allocation](#allocator-sensitive-functions), [storage](#storage-and-analysis-context),
[failure/capture](#failure-and-capture).

## First-diff decisions

| Symptom | Evidence and clean-C lever |
| --- | --- |
| Pointer load versus address calculation | Prove pointer versus array declaration |
| Wrong relocation/base folding | Standalone symbol versus reviewed struct field/array |
| `addu $at,index,$at` versus `addu $at,$at,index` | Named extern array can shift expansion to assembler; never pin scratch `$at` |
| Fixed-address constant steals entry copy | Named extern binding can prevent constant-address CSE; verify entire register web |
| Extra sign extension or `andi` | Prove signedness/width/necessary mask; do not guess from decompiler |
| Frame/size mismatch | Correct calls/prototypes, address-taken locals, aggregate copies, lifetime and topology |
| `beq`/`bne`, return web or tail merge | Invert branches, early return versus result local, duplicate branch calls |
| Loop topology | Compare while/do/for/guarded loop/explicit labels; matched goto is not style debt |
| Global/BSS offsets | Audit COMMON/section/alignment/order/padding and following symbols |
| Phantom read-only object | Local static constant versus existing shared extern ownership |

Named extern-array indexing can emit folded `lw table(index)` and assembler
`lui $at; addu $at,$at,index`; fixed-address arithmetic may emit compiler's opposite
operand order. Neither representation universally wins. Known RAM pointer-valued
global needs narrow evidenced local extern, not anonymous macro hiding ownership.

## Control flow

Interpret every delay slot on both paths before changing equal-valued arms; slot
constant may be comparison operand, not selected result. Bounded matrix: ordered
then nested/inverted equality; separate ungrouped cases; duplicated arm stores/
returns; shared locals or labels. Stop at three non-progressing variants, within
smaller sweep/mission ceiling. Locals may change register web. GCC can normalize
chain inversion regardless of spelling; `-fno-thread-jumps` is not a universal fix.

Branch calls `if (cond) f(x+A); else f(x+B);` can merge into one `jal` where ternary
argument does not. Early returns versus initialized result changes `$v0` lifetime.
GCC loop rotation can move invariants: assigning inside loop body can permit late
hoisting/preheader copies; moving before loop can lose them. Keep only measured
semantically correct shape; no spelling churn after proven ceiling.

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
Sole commutative `$at` operand-order residual may survive representation/profile/
permuter. Record compiler-order ceiling; assembler does not canonicalize compiler
encoding. Unauthorized expensive rungs stay untried, not exhausted.

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

## Allocator-sensitive functions

Same-CFG near match plus narrow probe causing spills/frame/size/save-register/
prologue changes indicates allocation sensitivity. Stay inside match-loop ceiling;
this branch adds no attempts. Move intact statements across dependency-safe lifetime
boundaries before expression churn. Classify retained improvement, retained frontier,
reverted neutral/local regression/structural regression. Rank exact bytes, matched
instructions, later first mismatch, fewer hunks, unchanged size/count/frame, then
least source disturbance. Frontier alone is not completion; keep it separately and
combine with best strict improvement only when safe and budgeted.

Unexpected frame/size/spill/multi-block/prologue drift means restore, unless predicted
by original diff. Ledger retains source parent, moved statement/crossed lifetime,
score/first mismatch/hunks/size/count/frame, retained changes and interaction result.
Reopen rejected shape only with changed definition/last use, overlapping call,
interfering saved value/frame/profile/residual block. Equivalent emitted forms share
one optimizer class; retry only when type/profile/lifetime/volatility/equivalence
changes. Chained assignment may need moving intact; reusing dead-looking local
changes lifetime. Unsupported probes/compile failures never prove exhaustion.

Preserve original surviving calculations even if analyzer calls them dead. Caller
`andi v0,v0,0xFF` proves low-byte consumption, not narrow callee ABI; retain evidenced
full-width return and explicit caller cast, verify both. Boolean spelling can alter
narrow-type code. Local representation view may yield memory `lhu` instead of
register `andi`; no generic punning framework.

## Storage and analysis context

Pad/layout checks include real field offsets, size, BSS order, GP reach and following
symbols. Prefer owning explicit padding symbol, not universal macro. COMMON may
reorder by name; `-fcommon`, `-G` and assembler COMMON options are profile/layout
leads only, requiring separate authorization/native gates. Shared strings/tables
need existing extern ownership, not phantom local static `.rodata`.

Separate Splat code/read-only jump-table objects can obscure m2c flow. Inspect label
resolution through owning generated-layout tooling; no handwritten assembly workaround
or unreviewed boundary change. m2ctx needs declarations/types/prototypes/macros/helpers,
not unrelated bodies/static data that shift rodata. GNU assembler GTE spelling may
use exact `.word` only in generated/SDK assembly, never C matching technique.

## Failure and capture

Compile failure is not diff or exhaustion. Compare known same-target function to
separate candidate from workspace/toolchain failure. Preserve last verified diff;
regenerate compile database before authorized profile/permuter probes on new source.
Known sandbox i386 failure uses native-execution route, never C/flag churn.

Record reviewed reusable residual class, decisive clean-C shape, before/after and
eligibility in this skill's references through authorized writer. Function-specific
selectors/scores stay mission evidence; source retains required metadata and rationale
for opaque clean-C shapes, not campaign journal. Partial evidence is not universal
rule. Reviewer proposes wording; approval precedes durable rule promotion.
