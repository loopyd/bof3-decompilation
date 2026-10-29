#include "bof3/battle/battle15_internal.h"

/* @source 0x8009EA8C
 * @behavior Calls func_801DC044 with bytes D_80146374 and D_80146394 and argument
 * 0xFFFF. Arithmetic-shifts its signed low halfword right by 1 and stores it
 * at byte +0x04 through the pointer loaded from D_801463A0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void storeHalvedSelectionResult(void) {
  D_8014639C.record[2] = func_801DC044(D_80146374, D_80146394, 0xFFFF) >> 1;
}
