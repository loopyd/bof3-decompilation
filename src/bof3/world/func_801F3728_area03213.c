#include "bof3/world/area03213_internal.h"

/* @behavior Arms the scratch cursor record after the 0x801C2438 palette setup
 * call, setting its bytes 4 and 0x5C to 1 and advancing its 0x34 accumulator by
 * the 0xC step, then consumes two units of the shared countdown word at 0x122
 * of the current work record.
 * @source 0x801F3728
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F3728(void) {
  u8* record;
  u8* work;

  func_801C2438();
  D_1F800044[0x5c] = 1;
  record = D_1F800044;
  record[4] = 1;
  work = D_80146250;
  *(s32*)(record + 0x34) += *(s32*)(record + 0x0c);
  *(u16*)(work + 0x122) -= 2;
}
