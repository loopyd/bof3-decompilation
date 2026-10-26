#include "bof3/battle/battle03_internal.h"

/* @behavior reports whether one local/enemy battler slot is unavailable, either
 * because it is inactive or because its corresponding `0x4000` flag is set.
 * @source 0x801DB524
 * @status exact
 * @match 100.00
 * @residual none
 * The zero-initialized `blocked` result keeps the original's direct `return 1`
 * branch and its shared zero-return fall-through; a constant `return 0` tail
 * folds into an `sltu` value and loses those instructions.
 */
u8 isBattlerInactiveOrFlag4000(u8 arg0) {
  u16 flags;
  u8  blocked;

  blocked = 0u;
  if (arg0 < 3u) {
    if ((D_80145E90[arg0].flags_00 & 1u) == 0u) {
      return 1u;
    }
    flags = D_80145E90[arg0].unk_80 & 0x4000u;
  } else {
    arg0 = (arg0 - 3u) & 0xffu;
    if ((D_801EB630[arg0].unk_00 & 1u) == 0u) {
      return 1u;
    }
    flags = D_801EB630[arg0].unk_82 & 0x4000u;
  }

  if (flags != 0u) {
    return 1u;
  }
  return blocked;
}
