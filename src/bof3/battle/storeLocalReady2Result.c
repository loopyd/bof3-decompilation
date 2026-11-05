#include "bof3/battle/battle03_internal.h"

/* @source 0x801E1B04
 * @behavior Calls the preceding battle handler and stores its result.
 * @status exact
 * @match 100.00
 * @residual none
 */
void storeLocalReady2Result(void) {
    u8 result;

    result = localReadyOrHelper2();
    D_1F800044->pad_0a[1] = result;
}
