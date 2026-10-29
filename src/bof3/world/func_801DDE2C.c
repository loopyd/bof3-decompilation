#include "bof3/world/area03004_internal.h"

/* @source 0x801DDE2C
 * @behavior Updates D_801E3210 to D_801E2720 + 20 * D_80145026 when that
 * index byte is nonzero, and D_801E320C to D_801E2748 + 10 * D_80145028
 * when that index byte is nonzero. Leaves each cursor unchanged for a
 * zero index.
 */
void func_801DDE2C(void)
{
    u8 index;

    index = D_80145026;
    if (index != 0) {
        D_801E3210 = &D_801E2720[index * 20];
    }

    index = D_80145028;
    if (index != 0) {
        D_801E320C = (s8*)&D_801E2748[index * 10];
    }
}
