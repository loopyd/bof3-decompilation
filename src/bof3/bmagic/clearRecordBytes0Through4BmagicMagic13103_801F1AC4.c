#include "bof3/bof3.h"

extern u8 *workRecord801F24E0; /* @source 0x801F24E0 @kind data */

/* @source 0x801F1AC4
 * @behavior Clears record bytes 0 through 4, reloading the pointer cell at
 * 0x801F24E0 before each byte store. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearRecordBytes0Through4BmagicMagic13103_801F1AC4(void) {
  workRecord801F24E0[0] = 0;
  workRecord801F24E0[1] = 0;
  workRecord801F24E0[2] = 0;
  workRecord801F24E0[3] = 0;
  workRecord801F24E0[4] = 0;
}
