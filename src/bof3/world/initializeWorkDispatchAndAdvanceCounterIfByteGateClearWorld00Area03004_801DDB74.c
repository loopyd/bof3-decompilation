#include "bof3/world/area03004_internal.h"

/* @behavior When the guard byte is clear, advances the area state and
 * initializes scratch-record bytes 2 and 3.
 * @source 0x801DDB74
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializeWorkDispatchAndAdvanceCounterIfByteGateClearWorld00Area03004(void)
{
    u8* counter;

    if (D_80149332 == 0) {
        counter = &D_8014403D;
        *counter = *counter + 1;
        D_1F800044[2] = 1;
        D_1F800044[3] = 0;
    }
}
