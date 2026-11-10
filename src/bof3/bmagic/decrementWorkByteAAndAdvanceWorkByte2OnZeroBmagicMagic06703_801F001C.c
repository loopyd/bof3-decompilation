#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F001C
 * @behavior Decrements byte 0xA of the work object reached through the
 *           scratchpad pointer cell at 0x1F800044. If the stored byte is zero,
 *           reloads that pointer cell and increments work byte 2. The pointer
 *           reload preserves possible aliasing by the preceding byte store.
 *           Takes no arguments, returns nothing and uses no stack frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteAAndAdvanceWorkByte2OnZeroBmagicMagic06703_801F001C(void) {
  if (--g_battle_work[0xA] == 0) {
    g_battle_work[2]++;
  }
}
