#include "bof3/battle/battle03_internal.h"

/* @behavior reports whether one battler slot is minimally eligible for the ranking
 * path, using only the active bit and the base local/enemy rejection masks.
 * @source 0x801DB2F8
 * @status exact
 * @match 100.00
 * @residual none
 * The zero-initialized `eligible` result keeps the original's direct `return 1`
 * branch and its shared zero-return fall-through; a constant `return 0` tail
 * folds into an `sltu` value and loses those instructions.
 */
u8 func_801DB2F8(u32 arg0) {
  u16 flags;
  u8  index;
  u8  eligible;

  eligible = 0u;
  index = (u8)arg0;
  if (index < 3u) {
    if ((D_80145E90[index].flags_00 & 1u) == 0u) {
      return 0u;
    }
    flags = D_80145E90[index].unk_80 & 0x4944u;
  } else {
    index = (u8)(arg0 - 3u);
    if ((D_801EB630[index].unk_00 & 1u) == 0u) {
      return 0u;
    }
    flags = D_801EB630[index].unk_82 & 0x4144u;
  }

  if (flags == 0u) {
    return 1u;
  }
  return eligible;
}
