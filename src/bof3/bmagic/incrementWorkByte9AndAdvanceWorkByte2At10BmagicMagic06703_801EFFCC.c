#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EFFCC
 * @behavior Increments work byte 9 through the scratchpad pointer cell at
 *           0x1F800044 and tests the truncated stored byte against 0x10.
 *           On equality, reloads the pointer cell and increments work byte 2.
 *           The reload preserves possible aliasing by the preceding store.
 *           Takes no arguments, returns nothing and uses no stack frame.
 *           The payload publishes this handler at 0x801F04DC in the byte-1
 *           dispatch table based at 0x801F04B8; no direct jal reaches it.
 *           A payload cmp proved this 80-byte span identical to
 *           emi/bmagic/magic088/03@0x801EF59C, whose C shape is used here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9AndAdvanceWorkByte2At10BmagicMagic06703_801EFFCC(void) {
  u8 *work;
  u8 counter;

  work = g_battle_work;
  counter = work[9] + 1;
  work[9] = counter;
  if (counter == 0x10) {
    g_battle_work[2]++;
  }
}
