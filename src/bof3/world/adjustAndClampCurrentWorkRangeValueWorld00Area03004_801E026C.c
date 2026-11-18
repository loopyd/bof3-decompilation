#include "bof3/world/area03004_internal.h"

/* @behavior advances the signed range position at +0x8E of the current work
 * record D_80146884 by delta and clamps the result into [0, +0x8C] of that
 * record.
 * @source 0x801E026C
 * @status exact
 * @match 100.00
 * @residual none
 */
void adjustAndClampCurrentWorkRangeValueWorld00Area03004(s8 delta) {
    Area030WorkRecord* range;
    s16 value;
    s16 limit;
    s16 result;

    D_80146884->value_8E += delta;
    range = D_80146884;
    value = range->value_8E;
    if (value >= 0) {
        limit = range->limit_8C;
        result = value;
        if (limit < value) {
            result = limit;
        }
    } else {
        result = 0;
    }
    range->value_8E = result;
}
