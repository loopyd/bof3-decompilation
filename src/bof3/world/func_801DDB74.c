#include "bof3/world/area03004_internal.h"

/* @behavior When the guard byte is clear, advances the area state and
 * initializes scratch-record bytes 2 and 3.
 * @source 0x801DDB74
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801DDB74(void)
{
    u8* state;

    if (D_80149332 == 0) {
        state = &D_8014403D;
        *state = *state + 1;
        D_1F800044[2] = 1;
        D_1F800044[3] = 0;
    }
}
