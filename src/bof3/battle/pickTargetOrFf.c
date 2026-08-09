#include "bof3/battle/battle03_internal.h"

/* @source 0x801E30B8
 * @behavior Returns 0xFF when D_801462F3 is 1; otherwise calls pickRandomUnblockedId with arg0 + 3 as u8.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
u8 pickTargetOrFf(s8 arg0) {
    s32 result;

    if (D_801462F3 == 1) {
        result = 0xFF;
    } else {
        result = pickRandomUnblockedId((u8)(arg0 + 3));
    }

    return result;
}
