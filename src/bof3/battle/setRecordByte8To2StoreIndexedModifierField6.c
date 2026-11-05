#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x800A1AD0
 * @behavior Sets selection record byte +0x08 to 2, applies the indexed battle
 *   modifier with element flag 1, and stores its result in record field +0x06.
 * @status partial
 * @match 88.89
 * @residual Constant scheduling and an unfilled jal delay slot add one instruction.
 */
void setRecordByte8To2StoreIndexedModifierField6(void)
{
  u32 index;
  u8 selection_index;
  u8 battler_index;
  s16 result;

  ((u8 *)D_8014639C.record)[8] = 2;
  index = D_801463C0;
  selection_index = D_80146374;
  battler_index = D_80146394;
  result = func_800A2880(selection_index, battler_index,
                        D_801CA71B[index * 0x14], 1);
  D_8014639C.record[3] = result;
}
