#include "bof3/battle/battle15_internal.h"

/* @behavior Tests the size word against both scratchpad work-range axis
 *   origins: the 0x34 and 0x38 origins are each advanced by half the size and
 *   reduced by the matching coordinate argument, and the result is 1 only when
 *   the size reaches both bounds.
 * @source 0x800AF720
 * @status partial
 * @match 5.88
 * @residual Clean-C partial: 1/17 instructions, 68/68 bytes, first difference
 *   +0x0000 `move t0,a2` vs `move a3,a2`. The emitted instruction sequence is
 *   positionally identical to the original (move, move-zero, lui, lw, srl, lw,
 *   lw, addu, subu, addu, sltu, bnez, subu, sltu, xori, jr) except that the
 *   original's trailing `nop` is a `move v0,t0` here; only the register
 *   assignment rotates: original (zero=v0, first=v1, work=a2, half=a3,
 *   size=t0) versus this shape (first=v0, work=v1, half=a2, size=a3, zero=t0).
 *   The allocator gives the same cyclic quantity order five registers in the
 *   v0,v1,a2,a3,t0 sequence but starts one position later, so the
 *   compound-condition result temp is allocated last here and first in the
 *   original. Eleven measured clean-C shapes (statement order, declaration
 *   order, `if`/`result` versus `&&`, local count, folded bounds) do not
 *   perturb it, so this is an allocation-order residual rather than an
 *   expression or control-flow difference. Compiler-profile and permuter rungs
 *   are not authorized in this lane. Clean-C byte match and independent review
 *   pending.
 */
u32 func_800AF720(u32 x, u32 y, u32 size) {
    u32 half;
    BattleRange *work;
    u32 first;
    u32 second;

    half = size >> 1;
    work = (BattleRange *)g_battle_work;
    first = work->range_axis_34 + half - x;
    second = work->range_axis_38 + half - y;
    return size >= first && size >= second;
}
