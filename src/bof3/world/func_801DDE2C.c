#include "bof3/world/area03004_internal.h"

/* @source 0x801DDE2C */
/* @behavior Select nonzero indexed records for two area030 work cursors. */
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
