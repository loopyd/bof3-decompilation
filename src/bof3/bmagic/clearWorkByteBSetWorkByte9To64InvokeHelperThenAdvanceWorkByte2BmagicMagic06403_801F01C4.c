#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void func_801F03D8(void); /* @source 0x801F03D8 @kind function */

/* @source 0x801F01C4
 * @behavior Clears work byte 0xB, reloads the scratchpad work pointer and sets
 * work byte 9 to 64, calls the target-local helper at 0x801F03D8, then reloads
 * the work pointer again and increments work byte 2. The entry is published
 * in the payload's callback table at offset 0x5C; no argument is consumed.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByteBSetWorkByte9To64InvokeHelperThenAdvanceWorkByte2BmagicMagic06403_801F01C4(void) {
  g_battle_work[0xB] = 0;
  g_battle_work[9] = 64;
  func_801F03D8();
  g_battle_work[2]++;
}
