#include "internal.h"

void func_800AE6C8(void)
{
    s32 value;
    u8 counter;
    u16 total;
    BattleWork *work;

    work = (BattleWork *)g_battle_work;
    counter = work->unk_09[0] + 1;
    value = work->unk_10 + 2;
    total = work->unk_30 + value;
    work->unk_09[0] = counter;
    work->unk_10 = value;
    work->unk_30 = total;
    if (counter == 0x10) {
        g_battle_work[1]++;
    }
}
