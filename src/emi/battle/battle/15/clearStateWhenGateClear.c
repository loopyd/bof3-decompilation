#include "internal.h"

// @source 0x8009BA7C
// @behavior Clears local battle selection state when its gate flag is clear.
void clearStateWhenGateClear(void)
{
    if (D_80146329 == 0) {
        D_801462E0 = 5;
        D_8014932E = 0;
        D_801462E1 = 3;
        D_801462E2 = 0;
        D_801462E3 = 0;
        D_801462E4 = 0;
    }
}
