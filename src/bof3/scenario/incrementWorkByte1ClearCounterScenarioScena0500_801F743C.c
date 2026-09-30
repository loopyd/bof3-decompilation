#include "bof3/bof3.h"

/* @source 0x801F743C
 * @behavior Advances the scratchpad work object byte at offset 0x01 and clears
 * the 32-bit counter at offset 0x0C of the same object.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte1ClearCounterScenarioScena0500_801F743C(void) {
  u8* work;

  work = SPAD_PTR_SLOT(u8, 0x44);
  work[1]++;
  *(s32*)(work + 0x0c) = 0;
}
