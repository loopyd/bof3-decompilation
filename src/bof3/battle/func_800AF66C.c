#include "bof3/battle/battle15_internal.h"

/* @behavior Checks horizontal and vertical battle-work bounds for arg1.
 * @source 0x800AF66C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
u32 func_800AF66C(BattleRange *range, u32 arg1) {
    /*
     * MATCHING_AID:
     * The original initializes its result in `v0`; clean-C lifetime/order,
     * compiler-profile, and permuter attempts left that allocator residual.
     * This local pin produces the original entry allocation and the immediately
     * following live byte match is exact. Remove if a clean-C shape supersedes it.
     */
    REGISTER_PIN(u32, result, "v0");
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
