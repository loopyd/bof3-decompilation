#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x8009EECC
 * @behavior Applies the battle modifier for the current battler index and base
 * value with element flag 0x14 and stores the signed halfword result in work
 * field 4 of the record pointed to by D_801463A0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8009EECC(void)
{
  u8 battler_index;
  u8 base_value;
  s16 result;

  battler_index = D_80146374;
  base_value = D_80146394;
  result = func_800A2880(battler_index, base_value, 0x14, 0);
  D_801463A0[2] = result;
}
