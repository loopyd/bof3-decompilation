#include "bof3/ui/game00_internal.h"

/**
 * @source 0x801BDE14
 * @behavior Find the first active eligible work record that passes the
 * coordinate/range test, returning its signed slot index or -1.
 * @status partial
 * @match 86.89
 * @residual same-size callee-saved register allocation rotation; clean-C
 * local copies regressed, permuter and compiler/flag searches found no exact
 */
s8 func_801BDE14(s32 x, s32 y, s32 range)
{
  s32 offset;
  s32 i;
  s16 reference;
  struct GameWorkArea* work;

  reference = func_801BDCF8();
  i = 0;
  offset = 0;
  do {
    work = (struct GameWorkArea*)((u8*)D_80146888 + offset);
    if (work != g_game_work &&
        ((struct GameWorkArea*)((u8*)D_80146888 + offset))->flags_00 != 0 &&
        (((struct GameWorkArea*)((u8*)D_80146888 + offset))->unk_07 & 0x80) ==
            0 &&
        func_801BDD58(x, y, reference, range, work)) {
      return (s8)i;
    }
    i++;
    offset += 0x98;
  } while (i < 34);
  return -1;
}
