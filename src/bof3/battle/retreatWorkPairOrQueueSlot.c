#include "bof3/battle/battle03_internal.h"

void func_800AAC88(s16 arg0, u8 arg1); /* @source 0x800AAC88 */

/* @source 0x801E05EC
 * @behavior While scratch work byte +0x0A is nonzero, decrements it, subtracts the delta pair +0x0C/+0x10 from position pair +0x34/+0x38 and runs localReadyOrHelper2. Otherwise calls func_800AAC88 with local halfword +0x11C and scratch byte +0x05 when local byte +0x120 has bit 0x10, or queues a mode-1 slot with byte +0x07 set to 3, byte +0x27 cleared and its work pointer set to scratch, then advances scratch byte +0x03.
 * @status exact
 * @match 100.00
 * @residual none
 */
void retreatWorkPairOrQueueSlot(void)
{
    Battle03LocalWork* scratch;
    Battle03LocalWork* work;
    Battle03LocalWork* queued_work;
    u32 slot;

    scratch = D_1F800044;
    if (scratch->pad_0a[0] == 0) {
        work = D_80146250;
        if (work->unk_120 & 0x10) {
            func_800AAC88((s16)work->unk_11c, scratch->unk_05);
        } else {
            slot = (u8)func_801E590C(0u, 1u);
            queued_work = D_1F800044;
            D_801EC330[slot].pad_07[0] = 3;
            FIELD_REF(u8, &D_801EC330[slot], 0x27) = 0;
            D_801EC330[slot].ptr_74 = (u32)queued_work;
        }
        D_1F800044->unk_03++;
    } else {
        scratch->unk_34 -= scratch->unk_0c;
        scratch->unk_38 -= scratch->unk_10;
        scratch->pad_0a[0]--;
        localReadyOrHelper2();
    }
}
