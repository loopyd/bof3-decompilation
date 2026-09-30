#include "bof3/bof3.h"

/* @source 0x801F6F8C
 * @behavior Advances the scratchpad work object pointed to by pointer slot
 * 0x1F800044 + 0x44: it increments the byte at object offset 0x02 and loads the
 * constant -8 into the 32-bit counter at object offset 0x10 (the field
 * 0x801F6FB0 reads and increments). Takes no arguments and returns nothing; the
 * incremented byte is left in $v1 only because the store needs it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte2SetCounterScenarioScena0700_801F6F8C(void) {
  u8* work;

  work = SPAD_PTR_SLOT(u8, 0x44);
  work[2]++;
  *(s32*)(work + 0x10) = -8;
}
