#include "bof3/bof3.h"

/* @source 0x801F7530
 * @behavior Advances the scratchpad work object byte at offset 0x01 and loads
 * the constant 0x1000 into the 32-bit counter at offset 0x0C of the same
 * object.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte1SetCounterScenarioScena0500_801F7530(void) {
  u8* work;

  work = SPAD_PTR_SLOT(u8, 0x44);
  work[1]++;
  *(s32*)(work + 0x0c) = 0x1000;
}
