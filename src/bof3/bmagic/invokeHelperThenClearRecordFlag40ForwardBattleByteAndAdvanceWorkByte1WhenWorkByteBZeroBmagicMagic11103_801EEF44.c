#include "bof3/bof3.h"

extern void func_800AEF98(s32 event, s32 value); /* @source 0x800AEF98 */
extern void func_800A9820(u8 value); /* @source 0x800A9820 */

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* @source 0x801EEF44
 * @behavior Handler at entry 1 of the table at 0x801EEC04. When byte 0xB
 *           of the scratchpad work object is zero, calls helper 0x800AEF98
 *           with arguments 4 and 0, clears bit 0x40 of byte 0 of the record
 *           reached through 0x801EB4E0, forwards the unsigned battle byte at
 *           0x80146384 to helper 0x800A9820, then reloads the scratchpad work
 *           pointer and increments its byte 1. Takes no arguments and returns
 *           nothing. Helper roles beyond these observed calls remain unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperThenClearRecordFlag40ForwardBattleByteAndAdvanceWorkByte1WhenWorkByteBZeroBmagicMagic11103_801EEF44(void) {
  if (g_battle_work[0xB] == 0) {
    func_800AEF98(4, 0);
    D_801EB4E0[0] &= 0xBF;
    func_800A9820(D_80146384);
    g_battle_work[1]++;
  }
}
