#include "bof3/battle/battle15_internal.h"

/* @behavior Tests the size word against both scratchpad work-range axis
 *   origins: the 0x34 and 0x38 origins are each advanced by half the size and
 *   reduced by the matching coordinate argument, and the result is 1 only when
 *   the size reaches both bounds.
 * @source 0x800AF720
 * @status exact
 * @match 100.00
 * @residual none
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
    if (size >= first && size >= second) {
        return 1;
    }
    return 0;
}
