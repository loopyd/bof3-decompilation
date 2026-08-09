#include "bof3/world/area01613_internal.h"

/* @source 0x801F3460
 * @behavior clears scratch-state offsets 0x09 and 0x0b, then increments 0x01.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void resetAdvanceScratchState(void)
{
  u8** slots;
  u8*  ptr;

  slots = SPAD_PTR_TABLE(u8);
  /* MATCHING_AID: retain v1 as the scratchpad-table reload register. */
  CLOBBER_CALLER_REG(v1);
  slots[0x11][0x09] = 0;
  /* MATCHING_AID: preserve the second independent v1 table reload. */
  CLOBBER_CALLER_REG(v1);
  slots[0x11][0x0b] = 0;
  ptr = slots[0x11];
  ptr[1] += 1;
}
