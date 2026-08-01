#include "internal.h"

/* @source 0x801E1B04
 * @behavior Calls the preceding battle handler and stores its result.
 */
void func_801E1B04(void) {
    u8 result;

    result = func_801DEE4C();
    D_1F800044->pad_09[2] = result;
}
