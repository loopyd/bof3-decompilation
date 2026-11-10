#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F055C
 * @behavior Decrements unsigned work byte 9 through the scratchpad pointer
 *           cell at 0x1F800044. On zero, copies byte 8 and words 0x34 and
 *           0x38 from the record published at 0x801EB4E0, reads its word
 *           0x3C, sets work word 0x14 to 0x60000 and byte 9 to 8, then
 *           stores the record word minus 0x03000000 into work word 0x3C.
 *           Reloads the work cell to set byte 0xA to 0x10 and again to
 *           advance byte 2. The handler word at 0x801F0C84 names this
 *           argument-free, frame-free leaf; it returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenCopyRecordFieldsSeedWord14AndRearmBytes9AThenAdvanceWorkByte2BmagicMagic22603_801F055C(void) {
    u32 *source;
    u32 *work;
    u32 value;

    if (--g_battle_work[9] == 0) {
        g_battle_work[8] = D_801EB4E0[8];
        source = (u32 *)D_801EB4E0;
        work = (u32 *)g_battle_work;
        work[13] = source[13];
        work[14] = source[14];
        value = source[15];
        work[5] = 0x60000;
        ((u8 *)work)[9] = 8;
        work[15] = value - 0x03000000;
        g_battle_work[0xA] = 0x10;
        g_battle_work[2]++;
    }
}
