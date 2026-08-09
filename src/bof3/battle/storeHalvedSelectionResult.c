#include "bof3/battle/battle15_internal.h"

/* @source 0x8009EA8C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
/* @behavior Stores the signed half of the battle selection result in the active record. */
void storeHalvedSelectionResult(void) {
  D_801463A0[2] = func_801DC044(D_80146374, D_80146394, 0xFFFF) >> 1;
}
