#include "internal.h"

/**
 * @source 0x8019F310
 * @behavior Initializes movement fields and advances the work state.
 */
void func_8019F310(void)
{
    u8* work;
    u8 flag;

    flag = D_80143F02;
    work = (u8*)g_game_work;
    *(u16*)&work[0x38] = 0;
    *(s16*)&work[0x3A] = (flag & 1) ? -10 : -20;

    work = (u8*)g_game_work;
    *(u32*)&work[0x10] = 0x20000;
    work[0x29] = 3;

    work = (u8*)g_game_work;
    work[9] = 6;

    work = (u8*)g_game_work;
    work[1]++;
}
