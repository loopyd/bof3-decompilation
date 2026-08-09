#include "bof3/battle/battle03_internal.h"

/* @source 0x801E1938
 * @behavior Increments scratchpad battle-work byte +1 and clears byte +2 when localReadyOrHelper2 succeeds.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void advanceBytes12WhenReady(void) {
    if (localReadyOrHelper2() != 0) {
        ((u8*)D_1F800044)[1]++;
        ((u8*)D_1F800044)[2] = 0;
    }
}
