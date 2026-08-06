#include "internal.h"

/* @source 0x801E1938
 * @behavior Increments scratchpad battle-work byte +1 and clears byte +2 when localReadyOrHelper2 succeeds.
 */
void advanceBytes12WhenReady(void) {
    if (localReadyOrHelper2() != 0) {
        ((u8*)D_1F800044)[1]++;
        ((u8*)D_1F800044)[2] = 0;
    }
}
