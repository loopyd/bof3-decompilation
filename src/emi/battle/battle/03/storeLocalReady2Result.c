#include "internal.h"

/* @source 0x801E1B04
 * @behavior Calls the preceding battle handler and stores its result.
 */
void storeLocalReady2Result(void) {
    u8 result;

    result = localReadyOrHelper2();
    D_1F800044->pad_09[2] = result;
}
