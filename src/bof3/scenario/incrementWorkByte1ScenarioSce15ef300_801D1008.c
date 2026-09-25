#include "bof3/bof3.h"

/* @source 0x801D1008
 * @behavior Increments the scratchpad work byte at pointer-slot 0x44 + 0x1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte1ScenarioSce15ef300_801D1008(void) {
  SPAD_PTR_SLOT(u8, 0x44)[1]++;
}
