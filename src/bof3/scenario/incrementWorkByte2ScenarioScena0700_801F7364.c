#include "bof3/bof3.h"

/* @source 0x801F7364
 * @behavior Increments the scratchpad work byte at pointer-slot 0x44 + 0x2.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte2ScenarioScena0700_801F7364(void) {
  SPAD_PTR_SLOT(u8, 0x44)[2]++;
}
