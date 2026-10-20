#include "bof3/battle/battle15_internal.h"

/* @behavior Initializes a battle panel state from the active work record.
 * @source 0x8009B160
 * @status partial
 * @match 83.72
 * @residual same-CFG allocator/scheduling residual from +0x0000 (live
 *   byte-match DIFFER): the instruction sequence matches but the two short
 *   constants swap registers -- original holds 1 in $a0 and 3 in $v1, this
 *   shape holds 1 in $v1 and 3 in $a0. Rejected variants: literal 1/3 without
 *   a constant local and a named three = 3 local both measure 34/43 (worse
 *   than 36/43). The 52-profile canonical flag catalog returns no exact match
 *   (best 83.72).
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
