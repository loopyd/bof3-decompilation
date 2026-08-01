#include "internal.h"

/* @source 0x801E949C
 * @behavior Calls func_801D8450 with D_801462F4 when D_80144955 is zero.
 */
void func_801E949C(void)
{
    if (D_80144955 == 0) {
        func_801D8450(D_801462F4);
    }
}
