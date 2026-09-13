#include "bof3/battle/battle15_internal.h"

/* @behavior Checks horizontal and vertical battle-work bounds for arg1.
 * @source 0x800AF66C
 * @status partial
 * @match unavailable
 * @residual requeued after forbidden matching aid removal; clean-C byte match and independent review required
 */
u32 func_800AF66C(BattleRange *range, u32 arg1) {

    u32 result;
    u32 value;
    u32 half_value;
    BattleRange *work;
    u32 first;
    u32 second;

    value = arg1;
    result = 0;
    half_value = value >> 1;
    work = (BattleRange *)g_battle_work;
    first = work->range_axis_34 + half_value - range->range_axis_34;
    second = work->range_axis_38 + half_value - range->range_axis_38;
    if (value >= first) {
        result = value >= second;
    }
    return result;
}
