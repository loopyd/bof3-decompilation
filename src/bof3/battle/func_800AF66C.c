#include "bof3/battle/battle15_internal.h"

/* @behavior Tests both unsigned battle-work axis differences against value.
 * @source 0x800AF66C
 * @status partial
 * @match 10.53
 * @residual first mismatch +0x0000: move a3,a1 instead of move t0,a1; result allocated to t0 instead of v0; 76->76 bytes, clean-C match unresolved
 */
u32 func_800AF66C(BattleRange *range, u32 value) {
    u32 half_value;
    BattleRange *work;
    u32 first;
    u32 second;

    half_value = value >> 1;
    work = (BattleRange *)g_battle_work;
    first = work->range_axis_34 + half_value - range->range_axis_34;
    second = work->range_axis_38 + half_value - range->range_axis_38;
    return value >= first && value >= second;
}
