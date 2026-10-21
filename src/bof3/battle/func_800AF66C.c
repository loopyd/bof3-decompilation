#include "bof3/battle/battle15_internal.h"

/* @behavior Tests both unsigned battle-work axis differences against value:
 *   the scratchpad work record's 0x34 and 0x38 axis origins are each advanced
 *   by half the value and reduced by the range record's matching origin, and
 *   the function returns 1 only when the value reaches both bounds.
 * @source 0x800AF66C
 * @status partial
 * @match 21.05
 * @residual Register allocation rotates the value, result and axis temporaries; both bodies are 76 bytes.
 */
u32 func_800AF66C(BattleRange *range, u32 value) {
    u32 half_value;
    BattleRange *work;
    u32 first;
    u32 second;

    do {
        half_value = value >> 1;
    } while (0);
    work = (BattleRange *)g_battle_work;
    first = work->range_axis_34 + half_value - range->range_axis_34;
    second = work->range_axis_38 + half_value - range->range_axis_38;
    return value >= first && value >= second;
}
