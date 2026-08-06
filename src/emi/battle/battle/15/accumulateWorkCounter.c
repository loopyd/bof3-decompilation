#include "internal.h"

/* @source 0x800AE6C8
 * @behavior increments BattleWork byte `0x09`, adds 2 to its `0x10` counter,
 * then accumulates that counter into `0x30`; increments byte `0x01` when `0x09` reaches 0x10.
 */
void accumulateWorkCounter(void)
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
