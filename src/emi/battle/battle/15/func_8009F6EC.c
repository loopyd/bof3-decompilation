#include "internal.h"

void func_8009F6EC(void)
{
    volatile u16* flags;
    s16 result;
    u16 flag_value;
    u32 arg0;

    flags = D_801462E8;
    flag_value = *flags;
    D_801EC2EE = 0;
    do {
        *flags = flag_value | 0x2000;
        arg0 = D_80146374;
    } while (0);
    result = func_801DC044(arg0, D_80146394, 0xFFFF);
    D_801463A0[2] = (s16)((s32)result * 2);
    D_80145558 = 5;
}
