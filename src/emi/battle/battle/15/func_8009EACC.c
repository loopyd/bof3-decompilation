#include "internal.h"

void func_8009EACC(void)
{
    volatile u16* flags;
    s16 result;
    u32 new_var;

    flags = &D_801462E8;
    do {
        *flags |= 0x2000;
        new_var = D_80146374;
    } while (0);
    result = func_801DC044(new_var, D_80146394, 0xFFFF);
    D_801463A0[2] = (result >> 1) + 1;
}
