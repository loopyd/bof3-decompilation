#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 D_80146374; /* @source 0x80146374 @kind data */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind data */
extern u8 D_801EB630[]; /* @source 0x801EB630 @kind data */
extern u8 D_801EC330[]; /* @source 0x801EC330 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */
extern u32 func_801E590C(u32 mode0, u32 mode1); /* @source 0x801E590C @kind function */

typedef struct Magic00303QueuedRecordHead {
    u32 words[29];
} Magic00303QueuedRecordHead;

/* @source 0x801EFEC8
 * @behavior Claims four queued slots through 0x801E590C(1, 1). On each pass,
 *           reads selector 0x80146374 and copies the selected battler record's
 *           0x74-byte head into the allocated 0x78-byte slot. Selectors below
 *           3 use 0x80145E90 with stride 0x140; others use 0x801EB630 with
 *           stride 0x118 and index selector minus 3. Publishes the scratchpad
 *           work pointer in slot word 0x74, sets slot bytes 6, 5 and 1 to 1,
 *           clears byte 2, sets byte B to the ordinal and byte 9 to
 *           (ordinal + 1) * 4 + 1, then reloads the work pointer and increments
 *           work byte B. Does not check the allocator's 0xFF failure result.
 *           Takes no arguments and returns nothing. Field roles stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueFourActingBattlerSlotsCopyRecordHeadStampOrdinalAndStepPublishWorkIncrementByteBBmagicMagic00303_801EFEC8(void) {
    u8 ordinal;
    u32 slot;
    u8 selector;
    u32 offset;
    u8 *entry;
    u8 *counter;

    for (ordinal = 0; ordinal < 4; ordinal++) {
        slot = func_801E590C(1, 1);
        selector = D_80146374;
        if (selector < 3) {
            Magic00303QueuedRecordHead *src =
                (Magic00303QueuedRecordHead *)&D_80145E90[selector * 0x140];
            *(Magic00303QueuedRecordHead *)&D_801EC330[(u8)slot * 0x78] = *src;
        } else {
            Magic00303QueuedRecordHead *src =
                (Magic00303QueuedRecordHead *)&D_801EB630[(selector - 3) * 0x118];
            *(Magic00303QueuedRecordHead *)&D_801EC330[(u8)slot * 0x78] = *src;
        }
        offset = (u8)slot * 0x78;
        *(u8 **)&D_801EC3A4[offset] = g_battle_work;
        entry = &D_801EC330[offset];
        entry[6] = 1;
        entry[5] = 1;
        entry[1] = 1;
        entry[2] = 0;
        entry[0xB] = ordinal;
        entry[9] = ((u8)ordinal + 1) * 4 + 1;
        counter = g_battle_work;
        counter[0xB]++;
    }
}
