#include "bof3/bof3.h"

extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF3F4
 * @behavior Decrements work byte 9 and, when it reaches zero, copies byte 4
 *           from the record at pointer cell 0x801EB4E0 into the work object.
 *           Copies record words 0x34 and 0x38, adds 0xC00000 to record word
 *           0x3C, clears work byte 9 and stores the adjusted word at 0x3C.
 *           Reloads the work cell to set byte 0xA to 0x3C, then reloads it
 *           again to increment byte 2. Leaf callback at table word 0x801F048C;
 *           takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenCopyRecordByte4AndTripletAddC00000ClearByte9SetATo3CAndAdvanceWorkByte2BmagicMagic07603_801EF3F4(void) {
    u32 *source;
    u32 *work;
    u32 value;

    if (--g_battle_work[9] == 0) {
        g_battle_work[4] = D_801EB4E0[4];
        source = (u32 *)D_801EB4E0;
        work = (u32 *)g_battle_work;
        work[13] = source[13];
        work[14] = source[14];
        value = source[15];
        ((u8 *)work)[9] = 0;
        work[15] = value + 0xC00000;
        g_battle_work[0xA] = 0x3C;
        g_battle_work[2]++;
    }
}
