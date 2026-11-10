#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 D_80146384; /* @source 0x80146384 @kind data */
extern void func_800A9610(u8 target, s32 flags); /* @source 0x800A9610 */

/* @source 0x801EEE0C
 * @behavior Decrements work byte 9 through the scratchpad pointer cell at
 * 0x1F800044. When the stored byte reaches zero, passes the shared target byte
 * at 0x80146384 and flag 0x10 to func_800A9610, which raises that flag on the
 * selected battlers. Then reloads the work pointer and increments work byte 1.
 * Takes no arguments and returns nothing. Payload cmp proved this 104-byte
 * span identical to emi/bmagic/magic220/03 at 0x801EEEB0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9RaiseBattleTargetFlag10AndAdvanceWorkByte1OnZeroBmagicMagic13003_801EEE0C(void) {
    if (--g_battle_work[9] == 0) {
        func_800A9610(D_80146384, 0x10);
        g_battle_work[1]++;
    }
}
