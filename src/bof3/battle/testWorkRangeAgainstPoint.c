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
u32 testWorkRangeAgainstPoint(u32 x, u32 y, u32 size) {
    u32 halfSize;
    BattleRange *work;
    u32 axis34Bound;
    u32 axis38Bound;

    halfSize = size >> 1;
    work = (BattleRange *)g_battle_work;
    axis34Bound = work->range_axis_34 + halfSize - x;
    axis38Bound = work->range_axis_38 + halfSize - y;
    if (size >= axis34Bound && size >= axis38Bound) {
        return 1;
    }
    return 0;
}
