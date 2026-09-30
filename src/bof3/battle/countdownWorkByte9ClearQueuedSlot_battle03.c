#include "bof3/battle/battle03_internal.h"

/* @source 0x801E6C20
 * @behavior Counts down the scratch work byte at +0x09, then raises flag bit 0x20 of the battle global halfword and clears the queued slot bytes when it expires.
 * @status exact
 * @match 100.00
 * @residual none
 */
void countdownWorkByte9ClearQueuedSlot(void) {
  u16* flags;

  func_8014DAEC();
  if (--D_1F800044->pad_09[0] == 0u) {
    flags = (u16*)&BATTLE_GLOBAL_HALF_62E8;
    *flags |= 0x20u;
    clearQueuedSlotBytes();
  }
}
