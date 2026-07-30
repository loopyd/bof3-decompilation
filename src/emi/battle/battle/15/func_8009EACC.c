#include "internal.h"

/* @source 0x8009EACC
 * @behavior sets flag 0x2000, calls func_801DC044 with state values and 0xFFFF,
 * then stores half the result plus one into D_801463A0[2].
 */
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
