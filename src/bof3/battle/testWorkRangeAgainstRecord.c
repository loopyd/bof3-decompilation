#include "bof3/battle/battle15_internal.h"

/* @behavior Tests both unsigned battle-work axis differences against value:
 *   the scratchpad work record's 0x34 and 0x38 axis origins are each advanced
 *   by half the value and reduced by the range record's matching origin, and
 *   the function returns 1 only when the value reaches both bounds.
 * @source 0x800AF66C
 * @status exact
 * @match 100.00
 * @residual none
 */
u32 testWorkRangeAgainstRecord(BattleRange *range, u32 size) {
    u32 halfSize;
    BattleRange *work;
    u32 axis34Bound;
    u32 axis38Bound;

    halfSize = size >> 1;
    work = (BattleRange *)g_battle_work;
    axis34Bound = work->range_axis_34 + halfSize - range->range_axis_34;
    axis38Bound = work->range_axis_38 + halfSize - range->range_axis_38;
    if (size >= axis34Bound && size >= axis38Bound) {
        return 1;
    }
    return 0;
}
