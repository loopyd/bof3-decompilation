#include "internal.h"

/* @behavior reports whether one enemy battler's `0xa8` value is large enough for
 * the current average/max threshold pair.
 * @source 0x801DB3E4
 */
u8 func_801DB3E4(u32 arg0, s32 arg1, u32 arg2) {
  u32 value;
  /*
   * MATCHING_AID:
   * Pins the scaled record offset to $v1 and spells the * 0x118 expansion
   * out as shifts/adds. The original runs the sll/addu/subu chain in $v1
   * against the unscaled index in $a0 and reuses $v1 for the halfword load;
   * unpinned gcc puts the chain in $v0 (asm-diff first=+0x000c, chain v0 vs
   * v1). The do-while anchor keeps the $a1 mask scheduled first. Clean-C
   * levers (early return, if/else, temporaries, pointer hoist, do-while
   * anchor) and two 60s permuter runs left this lone register residual.
   * Remove when gcc's allocator choice is reproduced by source shape; the
   * immediately following bin/byte-match was exact.
   */
  REGISTER_PIN(u32, offset, "v1");

  arg1 &= 0xffff;
  do {
    arg0 = (arg0 & 0xffu) - 3u;
    offset = arg0 << 3;
    offset += arg0;
    offset <<= 2;
    offset -= arg0;
    offset <<= 3;
    value = ((Battle03EnemyHalfRecord*)((u8*)D_801EB6D8 + offset))->half_00;
  } while (0);
  return ((arg1 << 1) <= (s32)value) && ((arg2 & 0xffffu) <= value);
}
