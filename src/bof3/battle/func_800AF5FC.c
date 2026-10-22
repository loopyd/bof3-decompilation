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
 *   bytes, 28 instructions, first difference +0x0004. The retained shape
 *   computes the two axis bounds first and the plane bound last with both
 *   shifts inlined; it keeps a single entry clear of the merged result
 *   temporary, both `bnez` to the shared `jr ra` and the `sltu`/`xori` tail,
 *   but that temporary lands in $t1 where the original uses $v0 (hence the
 *   trailing `move v0,t1`), and the two `srl` are emitted around the `lui/lw`
 *   pair instead of after it. Repair round refuted the early-return
 *   hypothesis: with the plane kept last, two `if (value < bound) return 0;`
 *   guards measure 11/28 at 116 bytes (29 instructions), because the shared
 *   `move v0,zero` block then sits between the compare path and `jr ra` and
 *   that path needs an extra `j`; with the plane first the same guards measure
 *   3/28 at 116 bytes. Both add an instruction the original does not have, so
 *   the single-expression body is kept. A statement-order-only move of the
 *   0x3E plane above both axes, with declarations and single-expression return
 *   unchanged, was measured next: 4/28 at 112 bytes with the `value` copy
 *   leaving $t0 (`move a3,a1`), so emission order is not the lever and the
 *   residual is register colouring. Other measured shapes: plane first with a
 *   `half` local 4/28 at 112 bytes; a named-shift body 5/28. Pre-lane
 *   provenance, not this round's evidence: a bounded permute of the superseded
 *   named-shift body found only `do { ... } while (0)` wrappers (rejected as
 *   structural regressions), and compiler-variant probes reported no exact
 *   candidate (canonical -O2 retained; 2.6.3 does not compile this TU; 2.95.2
 *   higher but not exact). Smallest missing evidence: a clean-C shape that
 *   leaves the emitted schedule unchanged while moving these long-lived
 *   temporaries. Clean-C byte match and independent review pending.
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
