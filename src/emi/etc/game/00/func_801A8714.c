#include "internal.h"

/* @behavior stores 0xF0 to D_80146864 and returns zero.
 * @source 0x801A8714
 */
s32 func_801A8714(void) {
    D_80146864 = 0xF0;
    return 0;
}
