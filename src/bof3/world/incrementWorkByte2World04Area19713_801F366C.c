#include "bof3/bof3.h"

/* @source 0x801F366C
 * @behavior Increments the scratchpad work byte at pointer-slot 0x44 + 0x2.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte2World04Area19713_801F366C(void) {
  SPAD_PTR_SLOT(u8, 0x44)[2]++;
}
