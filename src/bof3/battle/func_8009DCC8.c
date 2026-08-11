#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x8009DCC8
 * @behavior Applies the indexed battle modifier and stores it in work field 4.
 * @status partial
 * @match 68.18
 * @residual prologue/global-load scheduling and index register differ
 */
void func_8009DCC8(void)
{
  u16 index;
  u8 battler_index;
  u8 base_value;
  s16 result;

  index = D_801463C0;
  battler_index = D_80146374;
  base_value = D_80146394;
  result = func_800A2880(battler_index, base_value,
                         D_801CA71B[index * 0x14], 0);
  D_801463A0[2] = result;
}
