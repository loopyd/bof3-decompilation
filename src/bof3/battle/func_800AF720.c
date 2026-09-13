#include "bof3/battle/battle15_internal.h"

/* @behavior Checks horizontal and vertical battle-work bounds for a coordinate pair.
 * @source 0x800AF720
 * @status partial
 * @match unavailable
 * @residual requeued after forbidden matching aid removal; clean-C byte match and independent review required
 */
u32 func_800AF720(u32 x, u32 y, u32 size) {

    u32 result;
    u32 value;
    u32 half_value;
    BattleRange *work;
    u32 first;
    u32 second;

    value = size;
    result = 0;
    half_value = value >> 1;
    work = (BattleRange *)g_battle_work;
    first = work->range_axis_34 + half_value - x;
    second = work->range_axis_38 + half_value - y;
    if (value >= first) {
        result = value >= second;
    }
    return result;
}
