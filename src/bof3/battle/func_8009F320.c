#include "bof3/battle/battle15_internal.h"

/* @source 0x8009F320
 * @behavior Runs selection pass func_800A3F28, then sets flag 0x200 in the
 * word indexed by D_80146394: the 0x140-stride local array at D_80145FB8
 * when D_80146374 is below three, otherwise the 0x118-stride reserve array
 * at D_801EB734 using index minus three.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8009F320(void) {
  func_800A3F28();
  if (*(u8 *)&D_80146374 < 3) {
    u32 local_index;

    local_index = *(u8 *)&D_80146394;
    D_80145FB8[local_index].flags_00 |= 0x200;
  } else {
    u32 reserve_index;

    reserve_index = *(u8 *)&D_80146394 - 3;
    D_801EB734[reserve_index].word_00 |= 0x200;
  }
}
