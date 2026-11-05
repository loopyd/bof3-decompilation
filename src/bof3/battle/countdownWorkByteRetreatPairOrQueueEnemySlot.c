#include "bof3/battle/battle03_internal.h"

void func_800AAC88(s16 arg0, u8 arg1); /* @source 0x800AAC88 */

/* @source 0x801E45DC
 * @behavior Decrements scratch byte +0x0A; on expiry calls func_800AAC88 with
 * enemy halfword +0xF8 and scratch byte +5 when enemy byte +0xFC has bit 0x10,
 * otherwise queues a mode-1 slot with byte +7 set to 3, byte +0x27 cleared and
 * its work pointer set to scratch, then advances scratch byte +3. Always
 * subtracts the delta pair +0x0C/+0x10 from scratch position +0x34/+0x38 and
 * runs enemyReadyOrHelper1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void countdownWorkByteRetreatPairOrQueueEnemySlot(void)
{
    Battle03LocalWork* work;
    Battle03LocalWork* queued_work;
    u32 slot;

    if (--D_1F800044->pad_0a[0] == 0) {
        if (battleCurrentEnemyWorkState->unk_fc & 0x10) {
            func_800AAC88((s16)battleCurrentEnemyWorkState->unk_f8,
                         D_1F800044->unk_05);
        } else {
            slot = (u8)func_801E590C(0u, 1u);
            queued_work = D_1F800044;
            D_801EC330[slot].pad_07[0] = 3;
            FIELD_REF(u8, &D_801EC330[slot], 0x27) = 0;
            D_801EC330[slot].ptr_74 = (u32)queued_work;
        }
        D_1F800044->unk_03++;
    }
    work = D_1F800044;
    work->unk_34 -= work->unk_0c;
    work->unk_38 -= work->unk_10;
    enemyReadyOrHelper1();
}
