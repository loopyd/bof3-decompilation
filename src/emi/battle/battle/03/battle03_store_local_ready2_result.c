#include "internal.h"

/* @source 0x801E1B04
 * @behavior Calls the preceding battle handler and stores its result.
 */
void battle03_store_local_ready2_result(void) {
    u8 result;

    result = battle03_local_ready_or_helper2();
    D_1F800044->pad_09[2] = result;
}
