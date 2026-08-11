#include "bof3/battle/battle15_internal.h"

/* @behavior Initializes a battle panel state from the active work record.
 * @source 0x8009B160
 * @status partial
 */
void func_8009B160(void) {
  u8 *work;
  u8 value;
  u8 *work_cell;
  int trailing_value;
  u8 one;

  one = 1;
  D_801485B9 = 8;
  work_cell = D_801EBF08;
  work = work_cell;
  D_801485B8 = one;
  D_801485BA = 3;
  D_801485BB = 3;
  D_801485C2 = 0;
  D_801485C3 = 0xff;
  value = work[5];
  D_801485D8 = (u32)D_800B6F50;
  D_801485BC = -0xaa;
  D_801485BE = 0x3f;
  D_801485C5 = one;
  D_80148573 = 4;
  D_801485C4 = value;
  trailing_value = work[5];
  D_801485C5 = 3;
  one = trailing_value;
  D_80148656 = one;
}
