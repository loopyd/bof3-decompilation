#include "internal.h"

/* @source 0x801E1938
 * @behavior Increments scratchpad battle-work byte +1 and clears byte +2 when func_801DEE4C succeeds.
 */
void func_801E1938(void) {
    if (func_801DEE4C() != 0) {
        ((u8*)D_1F800044)[1]++;
        ((u8*)D_1F800044)[2] = 0;
    }
}
