#include "bof3/battle/battle03_internal.h"

/* @source 0x801E1B04
 * @behavior Calls the preceding battle handler and stores its result.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void storeLocalReady2Result(void) {
    u8 result;

    result = localReadyOrHelper2();
    D_1F800044->pad_09[2] = result;
}
