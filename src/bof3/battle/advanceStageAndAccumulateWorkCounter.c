#include "bof3/battle/battle15_internal.h"

/* @source 0x800AE640
 * @behavior increments battle work byte +0x0A while it is below 8; then
 * increments work byte +0x09, adds 2 to signed counter +0x10 and accumulates
 * that counter into halfword +0x30; when byte +0x09 reaches 0x0C it resets
 * counter +0x10 to -6 and increments work byte +0x01.
 * @status partial
 * @match 55.88
 * @residual first mismatch +0x0000: work pointer in a1 and +0x09 counter in a0 instead of the original a0/a1; every one of the 15 differing instructions is that single register swap with no other difference, 136->136 bytes, clean-C match unresolved
 */
void advanceStageAndAccumulateWorkCounter(void)
{
    BattleWork *work;
    s32 value;
    u8 counter;
    u16 total;

    work = (BattleWork *)g_battle_work;
    if (work->unk_09[1] < 8) {
        work->unk_09[1] = work->unk_09[1] + 1;
        work = (BattleWork *)g_battle_work;
    }
    counter = work->unk_09[0] + 1;
    value = work->unk_10 + 2;
    total = work->unk_30 + value;
    work->unk_09[0] = counter;
    work->unk_10 = value;
    work->unk_30 = total;
    if (counter == 0x0C) {
        u8 *current;

        current = g_battle_work;
        ((BattleWork *)current)->unk_10 = -6;
        current[1] = current[1] + 1;
    }
}
