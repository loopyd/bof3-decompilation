#include "bof3/battle/battle15_internal.h"

/* @source 0x8009FEF4
 * @behavior Sets battle state 4 and action 0x66, stores that action's modifier
 * in selection-record field +0x04, then halves the signed halfword toward zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
void storeHalvedAction66Modifier(void)
{
  u8 selection;
  u8 battler;
  u8 power;

  D_80146375 = 4;
  selection = D_80146374;
  battler = D_80146394;
  power = D_801CA71B[0x66 * 0x14];
  D_801463C0 = 0x66;
  D_8014639C.record[2] = func_800A2880(selection, battler, power, 0);
  D_8014639C.record[2] /= 2;
}
