#include "bof3/bof3.h"

extern void func_800AEF98(s32 event, s32 value); /* @source 0x800AEF98 */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_8014637C; /* @source 0x8014637C @kind unknown */

/* @source 0x801EF1FC
 * @behavior When byte 0xB of the scratchpad work object is zero, clears bit
 *           0x40 of byte 0 of the record reached through pointer cell
 *           0x8014637C, calls helper 0x800AEF98 with arguments 4 and 0, then
 *           reloads the scratchpad work pointer and increments its byte 1.
 *           Takes no arguments and returns nothing. The helper's role and
 *           record's wider layout remain unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearRecordFlag40AndInvokeHelperWhenWorkByteBZeroThenAdvanceWorkByte1BmagicMagic00303_801EF1FC(void) {
  if (g_battle_work[0xB] == 0) {
    D_8014637C[0] &= 0xBF;
    func_800AEF98(4, 0);
    g_battle_work[1]++;
  }
}
