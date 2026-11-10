#include "bof3/bof3.h"

extern void func_801E5988(void); /* @source 0x801E5988 @kind function */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF6C4
 * @behavior Last entry of the five-entry table at 0x801F0D14, indexed by
 * the scratchpad work object's byte 1 through the dispatcher at 0x801EF56C.
 * Decrements work byte 9 modulo 256 and tests the truncated result. When it
 * reaches zero, decrements byte 0xB of the shared work record reached through
 * 0x801EB4E0, then calls the helper at 0x801E5988. Takes no arguments and
 * returns nothing. The scratchpad pointer is read before the 0x18-byte frame
 * is created; the shared-record store occupies the call delay slot.
 * The 92-byte original span equals magic131/03 at 0x801F02B0, proven by a
 * payload cmp; this body follows that sibling's statement order.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic08203_801EF6C4(void) {
  if (--g_battle_work[0x09] == 0) {
    D_801EB4E0[0x0B] -= 1;
    func_801E5988();
  }
}
