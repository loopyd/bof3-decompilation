#include "bof3/bof3.h"

extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F1CCC
 * @behavior Copies words at offsets 0x34, 0x38 and 0x3C from the record
 * published at 0x801EB4E0 to the scratchpad work record at 0x1F800044.
 * Sets work byte 9 to 0x10 and work word 0x14 to 0x100, then reloads the
 * scratchpad pointer and increments byte 2. Takes no arguments and returns
 * nothing. The word view preserves the original word accesses and pointer
 * reload, as in the exact MAGIC041 triplet-copy sibling.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyWorkTripletSetByte9To10AndWord14To100ThenAdvanceWorkByte2BmagicMagic15103_801F1CCC(void) {
    s32 *source;
    s32 *work;

    source = (s32 *)D_801EB4E0;
    work = (s32 *)g_battle_work;
    work[13] = source[13];
    work[14] = source[14];
    work[15] = source[15];
    ((u8 *)work)[9] = 0x10;
    work[5] = 0x100;
    g_battle_work[2]++;
}
