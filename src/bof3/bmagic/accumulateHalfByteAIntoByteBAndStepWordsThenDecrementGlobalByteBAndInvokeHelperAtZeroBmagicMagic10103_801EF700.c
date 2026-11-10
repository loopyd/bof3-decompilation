#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* @source 0x801EF700
 * @behavior Adds half the work record's unsigned byte 0xA to byte 0xB,
 *           then reloads the scratchpad work pointer. Decrements byte 0xA,
 *           subtracts two from word 0x1C and adds the new word to word 0x10.
 *           When the stored byte reaches zero, decrements byte 0xB of the
 *           record pointed to by 0x801EB4E0 and invokes 0x801E5988.
 *           The entry is published in the handler table at payload 0x10F8;
 *           argument and record roles beyond these accesses remain unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void accumulateHalfByteAIntoByteBAndStepWordsThenDecrementGlobalByteBAndInvokeHelperAtZeroBmagicMagic10103_801EF700(void) {
    u8 *record;
    u8 remaining;
    s32 step;
    s32 position;

    g_battle_work[0x0B] += g_battle_work[0x0A] >> 1;
    record = g_battle_work;
    remaining = record[0x0A] - 1;
    step = FIELD_REF(s32, record, 0x1C) - 2;
    position = FIELD_REF(s32, record, 0x10) + step;
    record[0x0A] = remaining;
    FIELD_REF(s32, record, 0x1C) = step;
    FIELD_REF(s32, record, 0x10) = position;
    if (remaining == 0) {
        u8 *shared_record;

        shared_record = D_801EB4E0;
        shared_record[0x0B] -= 1;
        func_801E5988();
    }
}
