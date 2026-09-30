#include "bof3/battle/battle15_internal.h"

/* @source 0x800AF5FC
 * @behavior Tests a size word against a battle range record. The scratchpad
 *   work record's 0x34/0x38 axis origins are advanced by half the size and
 *   compared unsigned against the range record's own 0x34/0x38 origins; the
 *   scratchpad 0x3E height word plus the size's ninth bit minus the range
 *   record's 0x3E word gives a masked plane distance. Returns 1 only when the
 *   size reaches both axis bounds and its eighth bit reaches that distance,
 *   otherwise 0.
 * @status partial
 * @match 32.14
 * @residual Clean-C partial: 9/28 instructions in place (32.14%), 112/112
 *   bytes, 28 instructions, first difference +0x0004. Retained shape computes
 *   the two axis bounds first and the plane bound last with both shifts
 *   inlined; it reproduces the entry result-temp clear, the two `bnez` to the
 *   shared `jr ra` and the in-place `sltu`/`xori` tail, and matches more
 *   instructions than the earlier named-shift shape (5/28, same 112/28 and
 *   first mismatch) that this replaces. Remaining difference: the result/`&&`
 *   temporary lands in $t1 vs the original's $v0, and the two `srl` are
 *   emitted around the `lui/lw` pair instead of after it. Measured
 *   alternatives: plane computed first with inlined shifts = 4/28 (14.29%,
 *   first +0x0000, 27/28 opcode kinds positionally identical). The bounded
 *   permute found only `do { ... } while (0)` scope wrappers, rejected as
 *   structural regressions (size/instruction-count or schedule shifts; exact
 *   counts not retained). Compiler-variant probes on the earlier 5/28 shape
 *   found no exact candidate (canonical -O2 retained; 2.6.3 does not compile
 *   this TU; 2.95.2 higher but not exact). Smallest missing evidence: a
 *   clean-C shape that leaves the emitted schedule unchanged while moving
 *   these long-lived temporaries. Clean-C byte match and independent review
 *   pending.
 *   No aids: no register pins, clobbers, barriers or empty asm.
 */
u32 func_800AF5FC(BattleRange *range, u32 value) {
    BattleRange *work;
    u32 plane;
    u32 first;
    u32 second;

    work = (BattleRange *)g_battle_work;
    first = work->range_axis_34 + (value >> 1) - range->range_axis_34;
    second = work->range_axis_38 + (value >> 1) - range->range_axis_38;
    plane = work->unk_3E + (value >> 9) - range->unk_3E;
    return value >= first && value >= second && (value >> 8) >= (plane & 0xFFFF);
}
