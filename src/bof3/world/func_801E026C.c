#include "bof3/world/area03004_internal.h"

/* @behavior advances the signed range position at +0x8E of the current work
 * record D_80146884 by arg0 and clamps the result into [0, +0x8C] of that
 * record.
 * @source 0x801E026C
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E026C(s8 arg0) {
    Area030Range* range;
    s16 value;
    s16 limit;
    s16 result;

    D_80146884->value_8E += arg0;
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
