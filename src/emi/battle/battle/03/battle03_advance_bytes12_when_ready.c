#include "internal.h"

/* @source 0x801E1938
 * @behavior Increments scratchpad battle-work byte +1 and clears byte +2 when battle03_local_ready_or_helper2 succeeds.
 */
void battle03_advance_bytes12_when_ready(void) {
    if (battle03_local_ready_or_helper2() != 0) {
        ((u8*)D_1F800044)[1]++;
        ((u8*)D_1F800044)[2] = 0;
    }
}
